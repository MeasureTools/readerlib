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
#include "rlib/android/meta_reader.h"

// StdLib
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <utility>

void rlib::android::meta_reader::preload_events()
{
    std::ifstream text_stream(this->_meta->event_filename);
    std::string line;
    while (text_stream.good() && !text_stream.eof()) {
        std::getline(text_stream, line);
        if (line.empty()) {
            continue;
        }
        std::istringstream event_format_stream(line);
        std::string event_format_element;
        std::getline(event_format_stream, event_format_element, ':');
        double time = std::stod(event_format_element) / 1000.0;
        if (!event_format_stream.good()) {
            continue;
        }
        std::string text = event_format_stream.str();

        rlib::common::event_data event;
        {
            event.time = time;
            event.origin = -1;
            event.message = text;
            event.event_level = rlib::common::event_data_level::VERBOS;
        }
        this->_events.push_back(event);
    }
    return;
}

rlib::android::meta_reader::meta_reader(std::string filename)
{
    this->_meta = std::make_unique< meta >(filename);
    this->preload_events();
}

std::string rlib::android::meta_reader::filename()
{
    return this->_meta->meta_filename;
}

std::vector< rlib::common::sensor > rlib::android::meta_reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (auto name : this->_meta->format) {
        if (name == "time") {
            continue;
        }
        rlib::common::sensor sensor;
        {
            sensor.name = name;
            sensor.unit = this->_meta->unit[ name ];
            sensor.sampling_interval = -1;
        }
        sensors.push_back(sensor);
    }
    return sensors;
}

std::vector< rlib::common::sample > rlib::android::meta_reader::samples(
    double begin, double end)
{
    std::vector< rlib::common::sample > data_vector;
    if (begin < 0.0) {
        begin = 0.0;
    }
    if (end < 0.0) {
        end = this->length();
    }
    end = fmin(this->length(), end);

    std::ifstream data_stream(this->_meta->data_filename, std::ios::binary);
    while (data_stream.good()) {
        rlib::common::sample datum;
        for (auto name : this->_meta->format) {
            if (name == "time") {
                uint64_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                if (this->_meta->unit[ name ] == "ns") {
                    datum.time = double(raw_value) / (1000.0 * 1000.0 * 1000.0);
                }
                if (this->_meta->unit[ name ] == "µs") {
                    datum.time = double(raw_value) / (1000.0 * 1000.0);
                }
                if (this->_meta->unit[ name ] == "ms") {
                    datum.time = double(raw_value) / (1000.0);
                }
                if (this->_meta->unit[ name ] == "s") {
                    datum.time = double(raw_value);
                }
                continue;
            }
            std::string type = this->_meta->type[ name ];
            double value;
            if (type == "i8") {
                int8_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "i16") {
                int16_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "i32") {
                int32_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "i64") {
                int64_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "u8") {
                uint8_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "u16") {
                uint16_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "u32") {
                uint32_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "u64") {
                uint64_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "f32") {
                float raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            if (type == "f64") {
                double raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                value = double(raw_value);
            }
            datum.values.push_back(value);
        }
        if (datum.time > end) {
            break;
        }
        if (begin <= datum.time && datum.time <= end) {
            data_vector.push_back(datum);
        }
    }

    return data_vector;
}

std::vector< rlib::common::event_data > rlib::android::meta_reader::events(
    double begin, double end)
{
    std::vector< rlib::common::event_data > eventVector;
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    for (auto& event : this->_events) {
        if (event.time > begin && event.time < end) {
            eventVector.push_back(event);
        }
    }
    return eventVector;
}

double rlib::android::meta_reader::length()
{
    std::ifstream data_stream(this->_meta->data_filename, std::ios::binary);
    double length = 0.0;
    while (data_stream.good() && !data_stream.eof()) {
        for (auto name : this->_meta->format) {
            if (name == "time") {
                uint64_t raw_value;
                data_stream.read(
                    reinterpret_cast< char* >(&raw_value), sizeof(raw_value));
                if (this->_meta->unit[ name ] == "ns") {
                    length = double(raw_value) / double(1000 * 1000 * 1000);
                }
                else if (this->_meta->unit[ name ] == "µs") {
                    length = double(raw_value) / double(1000 * 1000);
                }
                else if (this->_meta->unit[ name ] == "ms") {
                    length = double(raw_value) / double(1000);
                }
                else if (this->_meta->unit[ name ] == "s") {
                    length = double(raw_value);
                }
                continue;
            }
            std::string type = this->_meta->type[ name ];

            if (type == "i8" || type == "u8") {
                data_stream.ignore(sizeof(uint8_t));
            }
            else if (type == "i16" || type == "u16") {
                data_stream.ignore(sizeof(uint16_t));
            }
            else if (type == "i32" || type == "u32" || type == "f32") {
                data_stream.ignore(sizeof(uint32_t));
            }
            else if (type == "i64" || type == "u64" || type == "f64") {
                data_stream.ignore(sizeof(uint64_t));
            }
        }
    }
    return length;
}
