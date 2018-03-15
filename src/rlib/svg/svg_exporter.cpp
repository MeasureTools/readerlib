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
#include "rlib/svg/svg_exporter.h"

// StdLib
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

// Export data from begin (in seconds) till end (in seconds) to output stream
void rlib::svg::svg_exporter::data_export(
    double begin, double end, std::ostream& output)
{
    this->data_export(begin, end, -1, output);
}

// Export data from begin (in seconds) till end (in seconds) to output stream
// with resolution r
void rlib::svg::svg_exporter::data_export(
    double begin, double end, int_fast32_t r, std::ostream& output)
{
    std::vector< rlib::common::sensor > sensors = this->reader->sensors();
    std::vector< rlib::common::sample > data;
    if (r > 0) {
        data = this->reader->samples(begin, end, r);
    }
    else {
        data = this->reader->samples(begin, end);
    }

    std::srand(static_cast< unsigned int >(data.size() + sensors.size()));

    double yScale = 250;
    double dataBegin = 0.0;
    double dataEnd = 0.0;
    double min = 0.0;
    double max = 0.0;
    for (auto& datum : data) {
        dataBegin = std::fmin(dataBegin, datum.time);
        dataEnd = std::fmax(dataEnd, datum.time);
        min = std::fmin(
            min, *std::min_element(datum.values.begin(), datum.values.end()));
        max = std::fmax(
            max, *std::max_element(datum.values.begin(), datum.values.end()));
    }
    min *= 1.25;
    max *= 1.25;

    double virtualZero = std::max(std::abs(max), std::abs(min)) * yScale;
    int64_t height = int64_t(2.0 * virtualZero);
    // Header
    output << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    output
        << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" height=\""
        << height << "\" width=\"" << std::round(dataEnd) << "\">" << std::endl;
    // Content
    for (size_t sensorIndex = 0; sensorIndex < sensors.size(); ++sensorIndex) {
        output << "<polyline points=\"";
        for (auto& datum : data) {
            double value = datum.values.at(sensorIndex);
            if (value > 0) {
                value = virtualZero - value * yScale;
            }
            else {
                value = virtualZero + value * yScale;
            }
            if (std::isnormal(value)) {
                output << (datum.time) << "," << (value) << " " << std::endl;
            }
        }
        output << "\" style=\"fill:none;stroke:rgb(" << (std::rand() % 255)
               << "," << (std::rand() % 255) << "," << (std::rand() % 255)
               << ");stroke-width:1\" />" << std::endl;
    }
    // Zero Line
    output << "<polyline points=\"" << 0 << "," << virtualZero << " "
           << std::round(dataEnd) << "," << virtualZero
           << "\" style=\"fill:none;stroke:rgb(0,0,0);stroke-width:3\" />"
           << std::endl;
    output << "</svg>" << std::endl;
}
