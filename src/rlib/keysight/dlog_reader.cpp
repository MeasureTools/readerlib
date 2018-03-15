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
#include "rlib/keysight/dlog_reader.h"
#include "rlib/keysight/dlog.h"

// StdLib
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

rlib::keysight::dlog_reader::dlog_reader(std::string filename)
    : _dlog(filename)
{
}

std::string rlib::keysight::dlog_reader::filename()
{
    return this->_dlog.filename;
}

std::vector< rlib::common::sensor > rlib::keysight::dlog_reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (size_t i = this->_dlog.channels.size(); 0 < i; i--) {
        auto chan = this->_dlog.channels[ i - 1 ];
        if (chan.senseCurr) {
            rlib::common::sensor currentSensor;
            {
                currentSensor.name =
                    "Channel[" + std::to_string(chan.id) + "].Current";
                currentSensor.unit = "A";
                currentSensor.date = "---ADDME";
                currentSensor.version = "---ADDME";
                currentSensor.sampling_interval = this->_dlog.sampling_interval;
                currentSensor.channel = chan.id;
            }
            sensors.push_back(currentSensor);
        }
    }
    for (size_t i = this->_dlog.channels.size(); 0 < i; i--) {
        auto chan = this->_dlog.channels[ i - 1 ];
        if (chan.senseVolt) {
            rlib::common::sensor voltageSensor;
            {
                voltageSensor.name =
                    "Channel[" + std::to_string(chan.id) + "].Voltage";
                voltageSensor.unit = "V";
                voltageSensor.date = "---ADDME";
                voltageSensor.version = "---ADDME";
                voltageSensor.sampling_interval = this->_dlog.sampling_interval;
                voltageSensor.channel = chan.id;
            }
            sensors.push_back(voltageSensor);
        }
    }
    return sensors;
}

std::vector< rlib::common::sample > rlib::keysight::dlog_reader::samples(
    double begin, double end)
{
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    std::vector< rlib::common::sample > dataVector;
    if (begin >= end) {
        return dataVector;
    }
    size_t valuesPerInterval = this->sensors().size();
    size_t skipValues =
        size_t(std::round(begin / this->_dlog.sampling_interval));
    size_t begPos =
        size_t(this->_dlog.data_begin_pos +
               (valuesPerInterval * int(sizeof(float)) * skipValues));

    std::ifstream dataStream(this->_dlog.filename, std::ios::binary);
    {
        dataStream.ignore(std::streamsize(begPos));
    }
    size_t pos = begPos;
    while (dataStream.good()) {
        rlib::common::sample datum;
        // Calulate Time
        datum.time = ((pos - begPos) / valuesPerInterval) *
                     this->_dlog.sampling_interval;
        if (datum.time > end) {
            break;
        }
        // Add Values
        for (size_t i = 0; i < valuesPerInterval; i++) {
            // VALUE
            // Read Values (As each value is a f32, float is used)
            float value = -1;
            dataStream.read(reinterpret_cast< char* >(&value), sizeof(value));
            // Big to Little Endian! (UNSAFE)
            char* mem = reinterpret_cast< char* >(&value);
            std::reverse(mem, mem + sizeof(value));
            datum.values.push_back(double(value));
        }
        dataVector.push_back(datum);
        pos += valuesPerInterval * int(sizeof(float));
    }
    return dataVector;
}

std::vector< rlib::common::event_data > rlib::keysight::dlog_reader::events(
    double begin, double end)
{
    return {};
}

double rlib::keysight::dlog_reader::length()
{
    size_t valuesPerInterval = this->sensors().size();
    std::ifstream data(
        this->_dlog.filename, std::ifstream::ate | std::ifstream::binary);
    size_t fileSize = size_t(data.tellg());
    return double((fileSize - this->_dlog.data_begin_pos) / valuesPerInterval) *
           this->_dlog.sampling_interval;
}
