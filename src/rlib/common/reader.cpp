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
#include "rlib/common/reader.h"

// StdLib
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <experimental/optional>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

std::vector< rlib::common::sample > rlib::common::reader::samples(
    double begin, double end, int_fast32_t r)
{
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);

    const double sample_length = r <= 1 ? 1.0 : 1.0 / static_cast< double >(r);
    const size_t precision =
        std::to_string(sample_length).size() - std::string("0.").size();
    const auto data = this->samples(begin, end);

    std::map< size_t, std::map< size_t, std::vector< rlib::common::sample > > >
        sample_partition;
    for (const auto& sample : data) {
        double fract_part, int_part;
        fract_part = std::modf(static_cast< double >(sample.time), &int_part);
        size_t fst_index = size_t(int_part);
        size_t snd_index = size_t(std::round((fract_part / sample_length) *
                                             std::pow(10.0, precision)) /
                                  std::pow(10.0, precision));
        sample_partition[ fst_index ][ snd_index ].push_back(sample);
    }

    // Calculate mean
    std::vector< rlib::common::sample > return_vector;
    for (size_t fst_index = 0; fst_index < size_t(std::ceil(end));
         ++fst_index) {
        for (size_t snd_index = 0; snd_index < size_t(r); ++snd_index) {
            if (!sample_partition[ fst_index ][ snd_index ].empty()) {
                // TODO: Replace accumulate with reduce execution::par
                if (sample_partition[ fst_index ][ snd_index ].size() == 1) {
                    return_vector.push_back(
                        sample_partition[ fst_index ][ snd_index ][ 0 ]);
                }
                else {
                    rlib::common::sample mean = std::accumulate(
                        sample_partition[ fst_index ][ snd_index ].begin(),
                        sample_partition[ fst_index ][ snd_index ].end(),
                        rlib::common::sample());
                    mean /= sample_partition[ fst_index ][ snd_index ].size();

                    return_vector.push_back(mean);
                }

                sample_partition[ fst_index ][ snd_index ].clear();
                sample_partition[ fst_index ][ snd_index ].shrink_to_fit();
            }
        }
    }

    // Fill gaps
    size_t original_size = return_vector.size();
    if (original_size > 0) {
        for (size_t i = 0; i < original_size - 1; ++i) {
            auto diff = return_vector[ i + 1 ] - return_vector[ i ];
            if (std::fabs(diff.time / sample_length) >= 1) {
                auto step = diff * (sample_length / diff.time);
                for (double j = 1; j < std::fabs(sample_length / diff.time);
                     ++j) {
                    return_vector.push_back(return_vector[ i ] + (step * j));
                }
            }
        }
    }

    // TODO: execution::par
    std::sort(return_vector.begin(), return_vector.end(),
        [](auto a, auto b) { return a.time < b.time; });

    return return_vector;
}

rlib::common::sample rlib::common::reader::sample(double time)
{
    double length = this->length();
    double epsilon = 0.001;
    auto data = this->samples(time - epsilon, time + epsilon);
    while (data.empty() || (time - epsilon < 0 && time + epsilon > length)) {
        epsilon *= 2.0;
        data = this->samples(time - epsilon, time + epsilon);
    }
    for (size_t i = 0; i < data.size(); ++i) {
        auto datum = data[ i ];
        if (datum.time > time) {
            if (i == 0) {
                return rlib::common::sample();
            }
            auto prev = data[ i - 1 ];
            double prev_ratio =
                1.0 - std::fabs(prev.time - time) / (datum.time - prev.time);
            rlib::common::sample middle =
                prev * prev_ratio + datum * prev_ratio;
            return middle;
        }
    }
    return rlib::common::sample();
}

rlib::common::sample rlib::common::reader::sample(double time, int_fast32_t r)
{
    double length = this->length();
    double epsilon = 0.001;
    auto data = this->samples(time - epsilon, time + epsilon, r);
    while (data.empty() || (time - epsilon < 0 && time + epsilon > length)) {
        epsilon *= 2;
        data = this->samples(time - epsilon, time + epsilon, r);
    }
    for (size_t i = 0; i < data.size(); i++) {
        auto datum = data[ i ];
        if (datum.time > time) {
            if (i == 0) {
                return rlib::common::sample();
            }
            auto prev = data[ i - 1 ];
            double prev_ratio =
                std::fabs(prev.time - time) / (datum.time - prev.time);
            rlib::common::sample middle =
                prev * prev_ratio + datum * prev_ratio;
            return middle;
        }
    }
    return rlib::common::sample();
}

std::vector< std::experimental::optional< double > > rlib::common::reader::
    statistic(rlib::common::statistic_data t)
{
    std::vector< std::experimental::optional< double > > r(
        this->sensors().size());
    return r;
}
