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

// Qt

// Own
#include "rlib/common/statistic_reader.h"

// StdLib
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <future>
#include <iostream>
#include <numeric>
#include <string>
#include <tuple>

#include <iostream>

void rlib::common::statistic_reader::analyse_data()
{
    auto data = this->_reader->samples(0.0, -1.0);
    if (data.empty()) {
        return;
    }

    std::vector<
        std::future< std::map< rlib::common::statistic_data, long double > > >
        futures;

    for (size_t sensor = 0; sensor < this->_reader->sensors().size();
         ++sensor) {
        futures.emplace_back(std::async(std::launch::deferred, [&, sensor]() {
            std::vector< long double > sensorData;
            {
                sensorData.reserve(data.size());
            }
            for (auto& d : data) {
                sensorData.push_back(
                    static_cast< long double >(d.values[ sensor ]));
            }
            std::sort(sensorData.begin(), sensorData.end());
            long double avg =
                std::accumulate(sensorData.begin(), sensorData.end(), 0.0L) /
                static_cast< long double >(sensorData.size());
            /*long double var = std::transform_reduce(std::execution::par,
               sensorData.begin(), sensorData.end(), 1.0L, [](long double a,
               long double b) { return
               a + b; }, [avg](long double a) { return (a - avg) * (a - avg); })
               / static_cast< long double >(sensorData.size());*/
            long double var = 0.0L;
            {
                for (auto& d : sensorData) {
                    var += (d - avg) * (d - avg);
                }
                var /= static_cast< long double >(sensorData.size());
            }
            long double min = sensorData[ 0 ];
            long double max = sensorData[ sensorData.size() - 1 ];
            long double median;
            if (sensorData.size() % 2 == 0) {
                median = (sensorData[ (sensorData.size() / 2) ] +
                             sensorData[ (sensorData.size() / 2) - 1 ]) /
                         2.0L;
            }
            else {
                median = sensorData[ (sensorData.size() / 2) ];
            }
            std::map< rlib::common::statistic_data, long double > values;
            {
                values[ rlib::common::statistic_data::MIN_VALUE ] = min;
                values[ rlib::common::statistic_data::MAX_VALUE ] = max;
                values[ rlib::common::statistic_data::AVG_VALUE ] = avg;
                values[ rlib::common::statistic_data::MEDIAN_VALUE ] = median;
                values[ rlib::common::statistic_data::VAR_VALUE ] = var;
            }
            return values;
        }));
    }

    for (auto& future : futures) {
        auto returnMap = future.get();
        for (auto tuple : returnMap) {
            rlib::common::statistic_data key = std::get< 0 >(tuple);
            std::experimental::optional< double > opt;
            if (std::isnormal(std::get< 1 >(tuple))) {
                opt = std::experimental::make_optional(
                    static_cast< double >(std::get< 1 >(tuple)));
            }
            this->_analysis_data[ key ].push_back(opt);
        }
    }
}

rlib::common::statistic_reader::statistic_reader(
    std::shared_ptr< reader > rawReader)
{
    this->_reader = rawReader;
    this->analyse_data();
}

std::string rlib::common::statistic_reader::filename()
{
    return this->_reader->filename();
}

std::vector< rlib::common::sensor > rlib::common::statistic_reader::sensors()
{
    return this->_reader->sensors();
}

std::vector< rlib::common::sample > rlib::common::statistic_reader::samples(
    double begin, double end, int_fast32_t r)
{
    return this->_reader->samples(begin, end, r);
}

std::vector< rlib::common::sample > rlib::common::statistic_reader::samples(
    double begin, double end)
{
    return this->_reader->samples(begin, end);
}

std::vector< rlib::common::event_data > rlib::common::statistic_reader::events(
    double begin, double end)
{
    return this->_reader->events(begin, end);
}

std::vector< std::experimental::optional< double > > rlib::common::
    statistic_reader::statistic(rlib::common::statistic_data t)
{
    auto s = this->_reader->statistic(t);
    if (std::accumulate(s.begin(), s.end(), true, [](auto a, auto b) {
            return static_cast< bool >(a) & static_cast< bool >(b);
        })) {
        return s;
    }
    return this->_analysis_data[ t ];
}

double rlib::common::statistic_reader::length()
{
    return this->_reader->length();
}
