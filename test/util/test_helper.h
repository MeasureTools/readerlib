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

#pragma once

// Own
#include <rlib/common/event.h>
#include <rlib/common/event_data.h>
#include <rlib/common/exporter.h>
#include <rlib/common/reader.h>
#include <rlib/common/sample.h>
#include <rlib/common/synthetic_reader.h>

// StdLib
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

inline std::shared_ptr< rlib::common::reader > gen_syn_reader()
{
    auto succeedingTime = [](double t) { return t + 0.2; };
    auto events = [](double begin, double end) {
        std::vector< rlib::common::event_data > e;
        {
            {
                // Global Event
                rlib::common::event_data event;
                {
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('S');
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('1');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('1');
                    event.origin = -1;
                    event.time = 3.0;
                    event.message = "MSG:TEST1E1";
                    event.event_level = rlib::common::event_data_level::DEBUG;
                }
                e.push_back(event);
            }
            {
                // Sensor 2 Event
                rlib::common::event_data event;
                {
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('S');
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('1');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('2');
                    event.origin = 2;
                    event.time = 4.0;
                    event.message = "MSG:TEST1E2";
                    event.event_level = rlib::common::event_data_level::WARNING;
                }
                e.push_back(event);
            }
            {
                // Sensor 0 Event
                rlib::common::event_data event;
                {
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('S');
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('1');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('3');
                    event.origin = 0;
                    event.time = 9.0;
                    event.message = "MSG:TEST1E3";
                    event.event_level = rlib::common::event_data_level::VERBOS;
                }
                e.push_back(event);
            }
            {
                // Sensor 1 Event
                rlib::common::event_data event;
                {
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('S');
                    event.raw_data.push_back('T');
                    event.raw_data.push_back('1');
                    event.raw_data.push_back('E');
                    event.raw_data.push_back('4');
                    event.origin = 1;
                    event.time = 9.0;
                    event.message = "MSG:TEST1E4";
                    event.event_level = rlib::common::event_data_level::ERROR;
                }
                e.push_back(event);
            }
        }
        return e;
    };
    std::vector< std::function< double(double) > > sensors = { [](double t) {
                                                                  return 0.5;
                                                              },
        [](double t) { return 1.5; }, [](double t) { return 1.5; } };

    return std::make_shared< rlib::common::synthetic_reader >(
        succeedingTime, events, sensors);
}

template < class EXPORTER, int_fast32_t r = -1 >
int test_export_helper(std::shared_ptr< rlib::common::reader > src_reader,
    std::string filename1, std::string filename2)
{
    EXPORTER exporter(src_reader);

    std::ofstream out1(filename1, std::ios::binary);
    if (r > 0) {
        exporter.data_export(0.0, -1, r, out1);
    }
    else {
        exporter.data_export(0.0, -1, out1);
    }
    out1.flush();
    out1.close();
    std::ofstream out2(filename2, std::ios::binary);
    if (r > 0) {
        exporter.data_export(0.0, -1, r, out2);
    }
    else {
        exporter.data_export(0.0, -1, out2);
    }
    out2.flush();
    out2.close();

    std::ifstream in1(filename1, std::ios::binary);
    std::ifstream in2(filename2, std::ios::binary);
    while (in1.good() && in2.good()) {
        auto c1 = in1.get();
        auto c2 = in2.get();
        if (c1 != c2) {
            return EXIT_FAILURE;
        }
    }
    if (!in1.eof() || !in2.eof()) {
        return EXIT_FAILURE;
    }
    if (in1.rdstate() != in2.rdstate()) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

template < class READER, int_fast32_t r = -1 >
int test_reader_helper(std::shared_ptr< rlib::common::reader > src_reader,
    std::string filename1, std::string filename2)
{
    std::shared_ptr< rlib::common::reader > reader1 =
        std::make_shared< READER >(filename1);
    std::shared_ptr< rlib::common::reader > reader2 =
        std::make_shared< READER >(filename2);
    std::vector< rlib::common::sample > src_data, data1, data2;
    // TODO: Compare sensors and events!
    if (r > 0) {
        src_data = src_reader->samples(0.0, -1.0, r);
        data1 = reader1->samples(0.0, -1.0, r);
        data2 = reader2->samples(0.0, -1.0, r);
    }
    else {
        src_data = src_reader->samples(0.0, -1.0);
        data1 = reader1->samples(0.0, -1.0);
        data2 = reader2->samples(0.0, -1.0);
    }
    if (data1.size() != data2.size()) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < data1.size(); ++i) {
        if (data1[ i ] != data2[ i ]) {
            return EXIT_FAILURE;
        }
    }

    if (data1.size() != src_data.size()) {
        return EXIT_FAILURE;
    }
    for (size_t i = 0; i < data1.size(); ++i) {
        if (data1[ i ] != src_data[ i ]) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
