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
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

// Own
#include "rlib/remote/reader.h"

// StdLib
#include <algorithm>
#include <cmath>
#include <experimental/optional>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

std::experimental::optional< rlib::common::sample > rlib::remote::reader::
    read_from_buffer(char* buffer, size_t length)
{
    size_t bytes_read = 0;
    // Read remote_id
    int64_t remote_id = *(reinterpret_cast< int64_t* >(buffer));
    if (remote_id != this->remote_id()) {
        return {};
    }
    buffer += sizeof(int64_t);
    bytes_read += sizeof(int64_t);

    // Read time in nanoseconds
    int64_t time_in_ns = *(reinterpret_cast< int64_t* >(buffer));
    buffer += sizeof(int64_t);
    bytes_read += sizeof(int64_t);

    // Read number of values
    int64_t num_of_values = *(reinterpret_cast< int64_t* >(buffer));
    buffer += sizeof(int64_t);
    bytes_read += sizeof(int64_t);

    rlib::common::sample sample;
    {
        sample.time = double(time_in_ns) / double(1000L * 1000L * 1000L);
    }
    while (bytes_read < length && num_of_values > 0) {
        double value = *(reinterpret_cast< double* >(buffer));
        buffer += sizeof(double);
        bytes_read += sizeof(double);
        sample.values.push_back(value);
        --num_of_values;
    }
    if (num_of_values != 0) {
        return {};
    }
    if (length < bytes_read) {
        return {};
    }
    return { sample };
}

rlib::remote::reader::reader(std::string const& filename, size_t sensors,
    uint16_t port, int64_t remote_id)
    : m_filename{ filename }
    , m_port{ port }
    , m_remote_id{ remote_id }
    , m_data(filename, std::ios::in | std::ios::out | std::ios::app)
    , m_length{ 0 }
    , m_sensors{ sensors }
    , m_io_service{}
    , m_socket(m_io_service,
          boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), m_port))
{
    size_t buffer_length = (sizeof(int64_t) * 3) + (sizeof(double) * sensors);
    char buffer[ buffer_length * 2 ];
    // Read Samples data
    auto tellp = this->m_data.tellp();
    this->m_data.seekg(std::ios_base::beg);
    while (!this->m_data.eof()) {
        this->m_data.read(buffer, std::streamsize(buffer_length));
        auto sample = this->read_from_buffer(buffer, buffer_length);
        if (sample && sample->values.size() == this->m_sensors) {
            this->m_samples.push_back(*sample);
            this->m_length = std::max(this->m_length, sample->time);
        }
    }
    std::sort(this->m_samples.begin(), this->m_samples.end(),
        [](auto& a, auto& b) { return a.time < b.time; });

    this->m_data.clear();
    this->m_data.seekg(std::ios_base::end);
    this->m_data.seekp(tellp);

    this->receive();

    this->m_io_service_thread =
        std::thread([&]() { this->m_io_service.run(); });
}

std::string rlib::remote::reader::filename()
{
    return this->m_filename;
}

std::vector< rlib::common::sensor > rlib::remote::reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (size_t i = 0; i < this->m_sensors; ++i) {
        auto sensor = rlib::common::sensor();
        {
            sensor.name =
                std::to_string(this->remote_id()) + "_" + std::to_string(i);
        }
        sensors.push_back(std::move(sensor));
    }
    return sensors;
}

std::vector< rlib::common::sample > rlib::remote::reader::samples(
    double begin, double end)
{
    std::lock_guard< std::mutex > guard(this->m_samples_mutex);
    auto first = std::find_if(this->m_samples.begin(), this->m_samples.end(),
        [begin](auto& p) { return p.time >= begin; });
    auto last = std::find_if(this->m_samples.begin(), this->m_samples.end(),
        [end](auto& p) { return p.time > end; });
    return std::vector< rlib::common::sample >(first, last);
}
std::vector< rlib::common::event_data > rlib::remote::reader::events(
    double begin, double end)
{
    auto first = std::find_if(this->m_events.begin(), this->m_events.end(),
        [begin](auto& p) { return p.time >= begin; });
    auto last = std::find_if(this->m_events.begin(), this->m_events.end(),
        [end](auto& p) { return p.time > end; });
    return std::vector< rlib::common::event_data >(first, last);
}

double rlib::remote::reader::length()
{
    return this->m_length;
}

uint16_t rlib::remote::reader::port()
{
    return this->m_port;
}

int64_t rlib::remote::reader::remote_id()
{
    return this->m_remote_id;
}

void rlib::remote::reader::receive()
{
    this->m_socket.async_receive(
        boost::asio::buffer(this->m_msg_buffer, this->BUFFER_SIZE),
        [this](boost::system::error_code ec, std::size_t bytes_recvd) {
            if (!ec && bytes_recvd >= (3 * sizeof(int64_t) + sizeof(double))) {
                // Received data!
                char* buffer = this->m_msg_buffer;
                auto sample = this->read_from_buffer(buffer, bytes_recvd);
                if (sample && sample->values.size() == this->m_sensors) {
                    this->m_data.write(this->m_msg_buffer,
                        std::streamsize(3 * sizeof(int64_t) +
                                        sizeof(double) * this->m_sensors));
                    this->m_data.flush();
                    std::lock_guard< std::mutex > guard(this->m_samples_mutex);
                    this->m_samples.push_back(*sample);
                    this->m_length = std::max(this->m_length, sample->time);
                    std::sort(this->m_samples.begin(), this->m_samples.end(),
                        [](auto& a, auto& b) { return a.time < b.time; });
                }
            }
            this->receive();
        });
}
