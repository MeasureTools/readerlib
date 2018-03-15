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
#include "rlib/common/synthetic_reader.h"

// StdLib
#include <experimental/optional>
#include <functional>
#include <limits>
#include <random>
#include <string>
#include <vector>

rlib::common::synthetic_reader::synthetic_reader(
    std::function< double(double) > succeeding_time,
    std::function< std::vector< rlib::common::event_data >(double, double) >
        events,
    std::vector< std::function< double(double) > > sensors, double length)
{
    this->_filename = "SYNTHETIC_READER";
    this->_succeeding_time = succeeding_time;
    this->_events = events;
    this->_sensors = sensors;
    this->_length = length;
}

// Returns the filename of data file (e.g. data.psi, data.dlog)
std::string rlib::common::synthetic_reader::filename()
{
    return this->_filename;
}

// Contains Properties!
std::vector< rlib::common::sensor > rlib::common::synthetic_reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (size_t i = 0; i < this->_sensors.size(); ++i) {

        rlib::common::sensor sensor;
        {
            sensor.name = std::string("SynthSens[") + std::to_string(i) +
                          std::string("]");
        }
        sensors.push_back(sensor);
    }
    return sensors;
}

// Read data from begin (in seconds) till end (in seconds)
std::vector< rlib::common::sample > rlib::common::synthetic_reader::samples(
    double begin, double end)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0) {
        end = this->length();
    }
    std::vector< rlib::common::sample > data;
    for (double current = begin; current < end;
         current = this->_succeeding_time(current)) {
        rlib::common::sample datum;
        {
            datum.time = current;
            for (auto& sensor_function : this->_sensors) {
                datum.values.push_back(sensor_function(current));
            }
        }
        data.push_back(datum);
    }
    return data;
}

// Read Events from begin (in seconds) till end (in seconds)
std::vector< rlib::common::event_data > rlib::common::synthetic_reader::events(
    double begin, double end)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0) {
        end = this->length();
    }
    return this->_events(begin, end);
}

double rlib::common::synthetic_reader::length()
{
    return this->_length;
}
