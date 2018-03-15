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

// Own
#include "rlib/xml/xml_exporter.h"

// StdLib
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <vector>

void rlib::xml::xml_exporter::data_export(
    double begin, double end, std::ostream& output)
{
    this->data_export(begin, end, -1, output);
}

void rlib::xml::xml_exporter::data_export(
    double begin, double end, int_fast32_t r, std::ostream& output)
{
    output << std::setprecision(
        std::numeric_limits< long double >::digits10 + 1);
    std::vector< rlib::common::event_data > events =
        this->reader->events(begin, end);
    std::vector< rlib::common::sample > data;
    if (r > 0) {
        data = this->reader->samples(begin, end, r);
    }
    else {
        data = this->reader->samples(begin, end);
    }

    auto sensors = this->reader->sensors();
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
    output << "<output from=\"" << this->reader->filename() << "\">"
           << std::endl;
    // Sensors
    output << "    <sensors>" << std::endl;
    for (size_t i = 0; i < sensors.size(); ++i) {
        output << "        <sensor id=\"" << i << "\" name=\""
               << sensors.at(i).name << "\" unit=\"" << sensors.at(i).unit
               << "\" />" << std::endl;
    }
    output << "    </sensors>" << std::endl;

    // Data
    output << "    <dataset>" << std::endl;
    for (const auto& datum : data) {
        output << "         <data time=\"" << datum.time << "\">" << std::endl;
        for (size_t i = 0; i < datum.values.size(); ++i) {
            output << "            <value sensor=\"" << i << "\" value=\""
                   << datum.values.at(i) << "\"";
            output << " />" << std::endl;
        }
        output << "</data>" << std::endl;
    }
    output << "    </dataset>" << std::endl;

    // Events
    output << "    <events>" << std::endl;
    for (const auto& event : events) {
        output << "        <event level=\"" << event.event_level << "\" time=\""
               << event.time << "\" origin=\"" << event.origin << "\" >"
               << std::endl;
        output << "            <message>" << event.message << "</message>"
               << std::endl;
        output << "            <data>";
        for (auto c : event.raw_data) {
            for (size_t i = 0; i < sizeof(c) * 2; ++i) {
                output << "0123456789ABCDEF"[ c & 15 ];
                c = c >> 4;
            }
        }
        output << "</data>";
        output << "        </event>";
    }
    output << "   </events>" << std::endl;
    output << "</output>" << std::endl;
}
