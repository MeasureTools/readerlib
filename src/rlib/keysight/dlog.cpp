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
#include "rlib/keysight/channel.h"
#include "rlib/keysight/dlog.h"

// StdLib
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

rlib::keysight::dlog::dlog(std::string _filename)
{
    this->filename = _filename;

    // Read XML Part of the dlog
    std::string dlogXmlString;
    std::ifstream txtstream(this->filename.c_str());
    std::string line;
    do {
        std::getline(txtstream, line);
        // HOTFIX: Make Invalid xml valid again ...
        if (line.find("<1ua>") != std::string::npos) {
            line.replace(
                line.find("<1ua>"), std::string("<1ua>").size(), "<FIXME_1ua>");
        }
        if (line.find("</1ua>") != std::string::npos) {
            line.replace(line.find("</1ua>"), std::string("</1ua>").size(),
                "</FIXME_1ua>");
        }
        if (line.find("<2ua>") != std::string::npos) {
            line.replace(
                line.find("<2ua>"), std::string("<2ua>").size(), "<FIXME_2ua>");
        }
        if (line.find("</2ua>") != std::string::npos) {
            line.replace(line.find("</2ua>"), std::string("</2ua>").size(),
                "</FIXME_2ua>");
        }
        // HOTFIX: END!
        dlogXmlString += line;
    } while (line.find("</dlog>") == std::string::npos);
    this->data_begin_pos = size_t(txtstream.tellg());

    boost::property_tree::ptree dlog_xml;
    boost::property_tree::read_xml(this->filename, dlog_xml);

    for (auto& c : dlog_xml.get_child("dlog")) {
        // Skip all non channel Elements
        if (c.first != "channel") {
            continue;
        }
        rlib::keysight::channel chan;
        {
            chan.id = c.second.get< int >("<xmlattr>.id", -1);
            chan.senseVolt = c.second.get< int >("sense_volt", 0);
            chan.senseCurr = c.second.get< int >("sense_curr", 0);
        }
        this->channels.push_back(std::move(chan));
    }
    this->sampling_interval = dlog_xml.get< double >("dlog.frame.tint");
}
