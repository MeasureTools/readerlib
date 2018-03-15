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
#include "rlib/common/cached_reader.h"

// StdLib
#include <cmath>
#include <future>
#include <iostream>
#include <string>

rlib::common::cached_reader::cached_reader(std::shared_ptr< reader > reader)
{
    this->_reader = reader;
    this->_sensor_cache = this->_reader->sensors();
}

std::string rlib::common::cached_reader::filename()
{
    return this->_reader->filename();
}

std::vector< rlib::common::sensor > rlib::common::cached_reader::sensors()
{
    return this->_sensor_cache;
}

std::vector< rlib::common::sample > rlib::common::cached_reader::samples(
    double begin, double end, int_fast32_t r)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0.0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    if (begin >= end) {
        return {};
    }

    std::vector< rlib::common::sample > return_vector;
    int start_chunk = int(std::fmax(0.0, std::floor(begin / CHUNK_SIZE)));
    int end_chunk = int(std::ceil(end / CHUNK_SIZE)) + 1;

    std::vector<
        std::future< std::tuple< int, std::vector< rlib::common::sample > > > >
        futures;
    // Load the chunks parallel (if needed)
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        if (this->_chunked_resolution_sample_cache[ r ].count(chunk) < 1) {
            futures.emplace_back(
                std::async(std::launch::deferred, [&, chunk, r]() {
                    double chunk_begin = double(chunk) * CHUNK_SIZE;
                    double chunk_end = double(chunk + 1) * CHUNK_SIZE;
                    // Load Data from Reader
                    return std::make_tuple(chunk,
                        this->_reader->samples(chunk_begin, chunk_end, r));
                }));
        }
    }
    for (auto& future : futures) {
        auto return_tuple = future.get();
        if (this->_chunked_resolution_sample_cache[ r ].count(
                std::get< 0 >(return_tuple)) < 1) {
            this->_chunked_resolution_sample_cache[ r ][ std::get< 0 >(
                return_tuple) ] = std::get< 1 >(return_tuple);
        }
    }
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        auto data = this->_chunked_resolution_sample_cache[ r ][ chunk ];
        if (!return_vector.empty()) {
            double last_time = return_vector.at(return_vector.size() - 1).time;
            for (auto& datum : data) {
                if (last_time < datum.time && datum.time >= begin &&
                    datum.time <= end) {
                    return_vector.push_back(datum);
                }
            }
        }
        else {
            for (auto& datum : data) {
                return_vector.push_back(datum);
            }
        }
    }
    return return_vector;
}

std::vector< rlib::common::sample > rlib::common::cached_reader::samples(
    double begin, double end)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0.0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    if (begin >= end) {
        return {};
    }

    std::vector< rlib::common::sample > return_vector;
    int start_chunk = int(std::fmax(0.0, std::floor(begin / CHUNK_SIZE)));
    int end_chunk = int(std::ceil(end / CHUNK_SIZE)) + 1;

    std::vector<
        std::future< std::tuple< int, std::vector< rlib::common::sample > > > >
        futures;
    // Load the chunks parallel (if needed)
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        if (this->_chunked_sample_cache.count(chunk) < 1) {
            futures.emplace_back(std::async(std::launch::async, [&, chunk]() {
                double chunk_begin = double(chunk) * CHUNK_SIZE;
                double chunk_end = double(chunk + 1) * CHUNK_SIZE;
                // Load Data from Reader
                return std::make_tuple(
                    chunk, this->_reader->samples(chunk_begin, chunk_end));
            }));
        }
    }
    for (auto& future : futures) {
        auto return_tuple = future.get();
        if (this->_chunked_sample_cache.count(std::get< 0 >(return_tuple)) <
            1) {
            this->_chunked_sample_cache.insert_or_assign(
                std::get< 0 >(return_tuple), std::get< 1 >(return_tuple));
        }
    }
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        auto data = this->_chunked_sample_cache.at(chunk);
        double last_time = -1;
        if (!return_vector.empty()) {
            last_time = return_vector.at(return_vector.size() - 1).time;
        }
        for (auto& datum : data) {
            if (last_time < datum.time && datum.time >= begin &&
                datum.time <= end) {
                return_vector.push_back(datum);
            }
        }
    }
    return return_vector;
}

std::vector< rlib::common::event_data > rlib::common::cached_reader::events(
    double begin, double end)
{
    begin = std::fmax(begin, 0.0);
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    if (begin >= end) {
        return {};
    }

    std::vector< rlib::common::event_data > event_vector;
    int start_chunk = int(std::fmax(0.0, std::floor(begin / EVENT_CHUNK_SIZE)));
    int end_chunk = int(std::ceil(end / EVENT_CHUNK_SIZE));

    std::vector< std::future<
        std::tuple< int, std::vector< rlib::common::event_data > > > >
        futures;
    // Load the chunks parallel (if needed)
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        if (this->_chunked_event_cache.count(chunk) < 1) {
            futures.emplace_back(std::async(std::launch::async, [&, chunk]() {
                double chunk_begin = double(chunk) * EVENT_CHUNK_SIZE;
                double chunk_end = double(chunk + 1) * EVENT_CHUNK_SIZE;
                // Load Data from Reader
                return std::make_tuple(
                    chunk, this->_reader->events(chunk_begin, chunk_end));
            }));
        }
    }
    for (auto& future : futures) {
        auto return_tuple = future.get();
        if (this->_chunked_event_cache.count(std::get< int >(return_tuple)) ==
            0) {
            this->_chunked_event_cache.insert_or_assign(
                std::get< int >(return_tuple),
                std::get< std::vector< rlib::common::event_data > >(
                    return_tuple));
        }
    }
    for (int chunk = start_chunk; chunk < end_chunk; ++chunk) {
        auto events = this->_chunked_event_cache.at(chunk);
        double last_time = -1.0;
        if (!event_vector.empty()) {
            last_time = event_vector.at(event_vector.size() - 1).time;
        }
        for (auto& event : events) {
            if (last_time < event.time && event.time >= begin &&
                event.time <= end) {
                event_vector.push_back(event);
            }
        }
    }
    return event_vector;
}

std::vector< std::experimental::optional< double > > rlib::common::
    cached_reader::statistic(rlib::common::statistic_data t)
{
    if (this->_statistic_cache.count(t) == 0) {
        this->_statistic_cache.insert_or_assign(t, this->_reader->statistic(t));
    }
    return this->_statistic_cache[ t ];
}

double rlib::common::cached_reader::length()
{
    if (!this->_length_cache) {
        this->_length_cache = { this->_reader->length() };
    }
    return this->_length_cache.value();
}

void rlib::common::cached_reader::reset()
{
    this->_chunked_event_cache.clear();
    this->_chunked_sample_cache.clear();
    this->_chunked_resolution_sample_cache.clear();
    this->_length_cache = {};
    this->_sensor_cache = this->_reader->sensors();
    this->_statistic_cache.clear();
}
