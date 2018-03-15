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
#include "rlib/grim/grim_reader.h"

// StdLib
#include <cmath>
#include <iostream>
#include <string>
#include <utility>

rlib::grim::grim_reader::grim_reader(std::string filename)
    : _grim(filename)
{
}

std::string rlib::grim::grim_reader::filename()
{
    return this->_grim.filename;
}

std::vector< rlib::common::sensor > rlib::grim::grim_reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (size_t i = 0; i < this->_grim.sensor_name.size(); ++i) {
        rlib::common::sensor sensor;
        {
            sensor.name = this->_grim.sensor_name.at(i);
            sensor.unit = this->_grim.sensor_unit.at(i);
            sensor.sampling_interval = this->_grim.interval;
        }
        sensors.push_back(sensor);
    }
    return sensors;
}

std::vector< rlib::common::sample > rlib::grim::grim_reader::samples(
    double begin, double end)
{
    std::vector< rlib::common::sample > returnData;
    std::vector< std::vector< rlib::common::sample > > vectors;
    size_t maxSize = 0;
    for (auto& pair : this->_grim.src) {
        int_fast32_t r = int_fast32_t(1.0 / this->_grim.interval);
        auto vector = pair.second->samples(begin, end, r);
        maxSize = std::max(vector.size(), maxSize);
        vectors.insert(vectors.begin() + pair.first, vector);
    }
    for (size_t i = 0; i < maxSize; i++) {
        std::vector< rlib::common::sample > data;
        for (auto& vector : vectors) {
            if (vector.size() > i) {
                data.push_back(vector[ i ]);
            }
            else {
                auto stup = rlib::common::sample();
                {
                    stup.time = -1.0;
                }
                data.push_back(stup);
            }
        }
        rlib::common::sample datum;
        for (auto& vector : vectors) {
            if (vector.size() > i) {
                datum.time = std::fmax(vector.at(i).time, datum.time);
            }
        }

        for (auto func : this->_grim.sensor_function) {
            datum.values.push_back(func(data));
        }
        returnData.push_back(datum);
    }
    return returnData;
}

std::vector< rlib::common::event_data > rlib::grim::grim_reader::events(
    double begin, double end)
{
    return std::vector< rlib::common::event_data >();
}

double rlib::grim::grim_reader::length()
{
    double length = 0.0;
    for (auto& pair : this->_grim.src) {
        length = std::fmax(pair.second->length(), length);
    }
    return length;
}
