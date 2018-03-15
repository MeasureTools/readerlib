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

// Ext
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Own
#include "rlib/android/meta_reader.h"
#include "rlib/common/cached_reader.h"
#include "rlib/csv/csv_reader.h"
#include "rlib/grim/grim_data.h"
#include "rlib/grim/grim_reader.h"
#include "rlib/keysight/dlog_reader.h"
#include "rlib/powerscale/psi_reader.h"

// StdLib
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

rlib::grim::grim_data::grim_data(std::string _filename)
{
    this->filename = _filename;

    boost::property_tree::ptree grim_xml;
    boost::property_tree::read_xml(this->filename, grim_xml);

    this->version =
        grim_xml.get< std::string >("grim.<xmlattr>.version", "0.0");

    auto samplerate =
        grim_xml.get< double >("grim.sources.<xmlattr>.samplerate", -1);
    if (samplerate > 0) {
        this->interval = 1.0 / samplerate;
    }
    else {
        this->interval = 0.0;
        if (!this->src.empty()) {
            for (auto pair : this->src) {
                for (auto sensor : pair.second->sensors()) {
                    this->interval =
                        std::fmax(sensor.sampling_interval, this->interval);
                }
            }
        }
    }

    for (auto& source : grim_xml.get_child("grim.sources")) {
        // Skip all non source Elements
        if (source.first != "src") {
            continue;
        }
        auto id = source.second.get< int >("<xmlattr>.id");
        auto file = source.second.get< std::string >("<xmlattr>.file");
        auto ext = file.substr(file.find_last_of("."));

        // endsWith .dlog
        auto suffix = std::string(".dlog");
        if (ext == suffix) {
            // KEYSIGHT
            auto reader = std::make_shared< keysight::dlog_reader >(file);
            this->src.insert_or_assign(id, reader);
        }

        // endsWith .psi
        suffix = std::string(".psi");
        if (ext == suffix) {
            // POWERSCALE
            auto reader = std::make_shared< powerscale::psi_reader >(file);
            this->src.insert_or_assign(id, reader);
        }

        // endsWith .meta
        suffix = std::string(".meta");
        if (ext == suffix) {
            // ANDROID
            auto reader = std::make_shared< android::meta_reader >(file);
            this->src.insert_or_assign(id, reader);
        }

        // endsWith .grim
        suffix = std::string(".grim");
        if (ext == suffix) {
            // grim
            auto reader = std::make_shared< rlib::grim::grim_reader >(file);
            this->src.insert_or_assign(id, reader);
        }

        // endsWith .csv
        suffix = std::string(".csv");
        if (ext == suffix) {
            // CSV
            auto reader = std::make_shared< csv::csv_reader >(file);
            this->src.insert_or_assign(id, reader);
        }
    }

    for (auto& sensor : grim_xml.get_child("grim.sensors")) {
        // Skip all non sensor Elements
        if (sensor.first != "sensor") {
            continue;
        }

        this->sensor_name.push_back(
            sensor.second.get< std::string >("<xmlattr>.name"));
        this->sensor_unit.push_back(
            sensor.second.get< std::string >("<xmlattr>.unit"));

        std::function< double(std::vector< rlib::common::sample >&) > func =
            [](std::vector< rlib::common::sample >& v) { return 0; };
        for (auto& element : sensor.second.get_child("")) {
            if (element.first == "const") {
                double const_value =
                    element.second.get< double >("<xmlattr>.value");
                auto lambda = [const_value](
                                  std::vector< rlib::common::sample >& v) {
                    return const_value;
                };
                func = lambda;
            }
            if (element.first == "plus") {
                if (element.second.get_optional< double >("<xmlattr>.value")) {
                    double const_value =
                        element.second.get< double >("<xmlattr>.value");
                    auto lambda = [func, const_value](
                                      std::vector< rlib::common::sample >& v) {
                        return func(v) + const_value;
                    };
                    func = lambda;
                }
                else {
                    int src_id = element.second.get< int >("<xmlattr>.src");
                    int sensor_id =
                        element.second.get< int >("<xmlattr>.sensor");
                    auto lambda = [func, src_id, sensor_id](
                                      std::vector< rlib::common::sample >& v) {
                        if (v.size() > size_t(src_id) &&
                            v[ size_t(src_id) ].values.size() >
                                size_t(sensor_id)) {
                            double value = v.at(size_t(src_id))
                                               .values.at(size_t(sensor_id));
                            return func(v) + value;
                        }
                        return func(v);
                    };
                    func = lambda;
                }
            }
            if (element.first == "minus") {
                if (element.second.get_optional< double >("<xmlattr>.value")) {
                    double const_value =
                        element.second.get< double >("<xmlattr>.value");
                    auto lambda = [func, const_value](
                                      std::vector< rlib::common::sample >& v) {
                        return func(v) - const_value;
                    };
                    func = lambda;
                }
                else {
                    int src_id = element.second.get< int >("<xmlattr>.src");
                    int sensor_id =
                        element.second.get< int >("<xmlattr>.sensor");
                    auto lambda = [func, src_id, sensor_id](
                                      std::vector< rlib::common::sample >& v) {
                        if (v.size() > size_t(src_id) &&
                            v[ size_t(src_id) ].values.size() >
                                size_t(sensor_id)) {
                            double value = v.at(size_t(src_id))
                                               .values.at(size_t(sensor_id));
                            return func(v) - value;
                        }
                        return func(v);
                    };
                    func = lambda;
                }
            }
            if (element.first == "times") {
                if (element.second.get_optional< double >("<xmlattr>.value")) {
                    double const_value =
                        element.second.get< double >("<xmlattr>.value");
                    auto lambda = [func, const_value](
                                      std::vector< rlib::common::sample >& v) {
                        return func(v) * const_value;
                    };
                    func = lambda;
                }
                else {
                    int src_id = element.second.get< int >("<xmlattr>.src");
                    int sensor_id =
                        element.second.get< int >("<xmlattr>.sensor");
                    auto lambda = [func, src_id, sensor_id](
                                      std::vector< rlib::common::sample >& v) {
                        if (v.size() > size_t(src_id) &&
                            v[ size_t(src_id) ].values.size() >
                                size_t(sensor_id)) {
                            double value = v.at(size_t(src_id))
                                               .values.at(size_t(sensor_id));
                            return func(v) * value;
                        }
                        return func(v);
                    };
                    func = lambda;
                }
            }
            if (element.first == "divide") {
                if (element.second.get_optional< double >("<xmlattr>.value")) {
                    double const_value =
                        element.second.get< double >("<xmlattr>.value");
                    auto lambda = [func, const_value](
                                      std::vector< rlib::common::sample >& v) {
                        return func(v) / const_value;
                    };
                    func = lambda;
                }
                else {
                    int src_id = element.second.get< int >("<xmlattr>.src");
                    int sensor_id =
                        element.second.get< int >("<xmlattr>.sensor");
                    auto lambda = [func, src_id, sensor_id](
                                      std::vector< rlib::common::sample >& v) {
                        if (v.size() > size_t(src_id) &&
                            v[ size_t(src_id) ].values.size() >
                                size_t(sensor_id)) {
                            double value = v.at(size_t(src_id))
                                               .values.at(size_t(sensor_id));
                            return func(v) / value;
                        }
                        return func(v);
                    };
                    func = lambda;
                }
            }
        }
        this->sensor_function.push_back(func);
    }
}
