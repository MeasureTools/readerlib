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
#include "rlib/android/meta.h"

// StdLib
#include <iostream>
#include <sstream>
#include <string>

rlib::android::meta::meta(std::string filename)
{
    this->meta_filename = filename;
    this->data_filename = filename.substr(0, filename.size() - 4) + "data";
    this->event_filename = filename.substr(0, filename.size() - 4) + "event";

    boost::property_tree::ptree meta_xml;
    boost::property_tree::read_xml(this->meta_filename, meta_xml);

    this->version =
        meta_xml.get< std::string >("grim.<xmlattr>.version", "0.0");
    this->name = meta_xml.get< std::string >("grim.meta.name");
    this->init = meta_xml.get< uint64_t >("grim.meta.init");

    std::string format_text(meta_xml.get< std::string >("grim.meta.format"));
    std::istringstream format_stream(format_text);
    std::string format_element;
    while (format_stream.good()) {
        std::getline(format_stream, format_element, '|');
        this->format.push_back(format_element);
    }

    for (auto& value : meta_xml.get_child("grim.values")) {
        if (value.first != "value") {
            continue;
        }
        auto value_name =
            value.second.get< std::string >("<xmlattr>.name", "N.N");
        auto value_type =
            value.second.get< std::string >("<xmlattr>.type", "N.N");
        auto value_unit =
            value.second.get< std::string >("<xmlattr>.unit", "N.N");
        this->type[ value_name ] = value_type;
        this->unit[ value_name ] = value_unit;
    }
}
