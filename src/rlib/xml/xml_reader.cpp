/**
 * Copyright (c) 2017, Daniel "Dadie" Korner
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

// Ext
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Own
#include "rlib/common/reader.h"
#include "rlib/common/sample.h"
#include "rlib/common/sensor.h"
#include "rlib/xml/xml_reader.h"

// StdLib
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <vector>

rlib::xml::xml_reader::xml_reader(std::string filename)
{
    this->_filename = filename;

    boost::property_tree::ptree xml;
    boost::property_tree::read_xml(this->_filename, xml);

    std::vector< int > sensor_ids;
    for (auto& sensor : xml.get_child("output.sensors")) {
        if (sensor.first != "sensor") {
            continue;
        }
        auto sensor_id = sensor.second.get< int >("<xmlattr>.id", -1);
        auto name = sensor.second.get< std::string >("<xmlattr>.name", "N.N.");
        auto unit = sensor.second.get< std::string >("<xmlattr>.unit", "");
        this->_sensors.emplace_back(name, unit);
        sensor_ids.push_back(sensor_id);
    }

    for (auto& data : xml.get_child("output.dataset")) {
        if (data.first != "data") {
            continue;
        }
        rlib::common::sample datum;
        {
            datum.time = data.second.get< double >("<xmlattr>.time", -1.0);
        }
        std::map< int, double > values;
        {
            for (auto& value : data.second.get_child("")) {
                if (value.first != "value") {
                    continue;
                }
                auto sensor_id = value.second.get< int >("<xmlattr>.sensor");
                auto sensor_value =
                    value.second.get< double >("<xmlattr>.value");
                values[ sensor_id ] = sensor_value;
            }
        }
        for (auto sensor_id : sensor_ids) {
            if (values.count(sensor_id) < 1) {
                values[ sensor_id ] =
                    std::numeric_limits< double >::quiet_NaN();
            }
            datum.values.push_back(values[ sensor_id ]);
        }
        if (datum.values.size() != this->_sensors.size()) {
            continue;
        }
        this->_data.push_back(std::move(datum));
    }

    for (auto& event : xml.get_child("output.events")) {
        if (event.first != "event") {
            continue;
        }

        rlib::common::event_data e;
        {
            e.event_level = static_cast< rlib::common::event_data_level >(
                event.second.get< int64_t >("<xmlattr>.level"));
            e.time = event.second.get< double >("<xmlattr>.time");
            e.origin = event.second.get< int >("<xmlattr>.origin");
            e.message = event.second.get< std::string >("message", "");

            auto raw_data = event.second.get< std::string >("data", "");
            if (!raw_data.empty()) {
                std::string chars = "0123456789ABCDEF";
                for (size_t i = 0; i + 1 < raw_data.size(); i += 2) {
                    std::byte b =
                        static_cast< std::byte >(chars.find(raw_data[ i ]))
                        << 4;
                    b |=
                        static_cast< std::byte >(chars.find(raw_data[ i + 1 ]));
                    e.raw_data.push_back(static_cast< unsigned char >(b));
                }
            }
        }
        this->_events.push_back(std::move(e));
    }
}

std::string rlib::xml::xml_reader::filename()
{
    return this->_filename;
}

std::vector< rlib::common::sensor > rlib::xml::xml_reader::sensors()
{
    return this->_sensors;
}

std::vector< rlib::common::sample > rlib::xml::xml_reader::samples(
    double begin, double end)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0) {
        end = this->length();
    }
    std::vector< rlib::common::sample > returnData;
    for (auto& d : this->_data) {
        if (d.time < begin) {
            continue;
        }
        if (d.time > end) {
            break;
        }
        returnData.push_back(d);
    }
    return returnData;
}

std::vector< rlib::common::event_data > rlib::xml::xml_reader::events(
    double begin, double end)
{

    return this->_events;
}

double rlib::xml::xml_reader::length()
{
    double length = 0.0;
    if (!this->_data.empty()) {
        length = this->_data.at(this->_data.size() - 1).time;
    }
    return length;
}
