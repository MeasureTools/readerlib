/**
 * Copyright (c) 2016-2017, Daniel "Dadie" Korner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Neither the source code nor the binary may be used for any military use.
 *
 * THIS SOFTWARE IS PROVIDED BY Daniel "Dadie" Korner ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Daniel "Dadie" Korner BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

// Own
#include "rlib/csv/csv_reader.h"

// StdLib
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

rlib::csv::csv_reader::csv_reader(std::string filename)
    : _filename(filename)
{
    this->_async_loader = std::async(std::launch::async, [&]() {
        std::ifstream csv_file(this->filename());
        std::string line;
        if (csv_file.good()) {
            // Read first line to generate the Sensors
            std::getline(csv_file, line);
            std::istringstream value_stream(line);
            std::string value;
            // skip time
            std::getline(value_stream, value, ',');
            // values
            while (getline(value_stream, value, ',')) {
                rlib::common::sensor sensor;
                {
                    // Get Name (and Unit)
                    auto unit_begin = value.find_last_of("(");
                    sensor.name = value.substr(0, unit_begin - 1);
                    sensor.unit = value.substr(
                        unit_begin + 1, value.size() - unit_begin - 2);
                    sensor.sampling_interval = -1;
                }
                this->_sensors.push_back(sensor);
            }
        }
        this->_async_loader_sensor_finished = true;
        while (csv_file.good()) {
            std::getline(csv_file, line);
            std::istringstream value_stream(line);
            std::string value;
            rlib::common::sample datum;
            {
                // time
                std::getline(value_stream, value, ',');
                datum.time = std::stod(value);
                // values
                while (std::getline(value_stream, value, ',')) {
                    double v = double(std::stold(value));
                    // Note: if v is NaN v != v is true
                    if (v != v) {
                        if (!value.empty() && value[ 0 ] == '-') {
                            v = double(-NAN);
                        }
                    }
                    else {
                        size_t precision =
                            value.size() - value.find_first_of('.');
                        v = std::round(v * std::pow(10.0, precision)) /
                            std::pow(10.0, precision);
                    }
                    datum.values.push_back(v);
                }
            }
            this->_data.push_back(datum);
        }
        this->_async_loader_finished = true;
    });
}

std::string rlib::csv::csv_reader::filename()
{
    return this->_filename;
}

std::vector< rlib::common::sensor > rlib::csv::csv_reader::sensors()
{
    if (!this->_async_loader_sensor_finished) {
        this->_async_loader.wait();
    }
    return this->_sensors;
}

std::vector< rlib::common::sample > rlib::csv::csv_reader::samples(
    double begin, double end)
{
    if (!this->_async_loader_finished) {
        this->_async_loader.wait();
    }
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    std::vector< rlib::common::sample > return_data;
    for (auto& d : this->_data) {
        if (d.time < begin) {
            continue;
        }
        if (d.time > end) {
            break;
        }
        return_data.push_back(d);
    }
    return return_data;
}

std::vector< rlib::common::event_data > rlib::csv::csv_reader::events(
    double begin, double end)
{
    return std::vector< rlib::common::event_data >();
}

double rlib::csv::csv_reader::length()
{
    if (!this->_async_loader_finished) {
        this->_async_loader.wait();
    }
    double length = 0.0;
    if (!this->_data.empty()) {
        length = this->_data.at(this->_data.size() - 1).time;
    }
    return length;
}
