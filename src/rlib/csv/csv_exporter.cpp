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
#include "rlib/csv/csv_exporter.h"

// StdLib
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>

// Export data from begin (in seconds) till end (in seconds) to output stream
void rlib::csv::csv_exporter::data_export(
    double begin, double end, std::ostream& output)
{
    this->data_export(begin, end, -1, output);
}

// Export data from begin (in seconds) till end (in seconds) to output stream
// with resolution r
void rlib::csv::csv_exporter::data_export(
    double begin, double end, int_fast32_t r, std::ostream& output)
{
    output << std::setprecision(
        std::numeric_limits< long double >::digits10 + 1);
    std::vector< rlib::common::sample > data;
    if (r > 0) {
        data = this->reader->samples(begin, end, r);
    }
    else {
        data = this->reader->samples(begin, end);
    }

    // Header
    output << "time";
    for (const auto& sensor : this->reader->sensors()) {
        output << "," << sensor.name << " (" << sensor.unit << ")";
    }
    output << std::endl;
    // Content
    for (const auto& datum : data) {
        output << datum.time;
        for (auto& value : datum.values) {
            output << "," << value;
        }
        output << std::endl;
    }
}
