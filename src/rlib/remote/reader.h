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

// Ext
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

// Own
#include "rlib/common/event_data.h"
#include "rlib/common/reader.h"
#include "rlib/common/sample.h"
#include "rlib/common/sensor.h"

// StdLib
#include <cstdint>
#include <experimental/optional>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace rlib {
    namespace remote {
        class reader : public rlib::common::reader {
            private:
            constexpr static size_t BUFFER_SIZE = 1024 * 16; // 16kByte

            std::string m_filename;
            uint16_t m_port;
            int64_t m_remote_id;

            std::fstream m_data;
            double m_length;
            size_t m_sensors;

            std::vector< rlib::common::sample > m_samples;
            std::mutex m_samples_mutex;
            std::vector< rlib::common::event_data > m_events;

            boost::asio::io_service m_io_service;
            boost::asio::ip::udp::socket m_socket;
            std::thread m_io_service_thread;
            char m_msg_buffer[ BUFFER_SIZE ];

            private:
            std::experimental::optional< rlib::common::sample >
                read_from_buffer(char* buffer, size_t length);

            public:
            reader(std::string const& filename, size_t sensors, uint16_t port,
                int64_t remote_id);
            virtual ~reader() override = default;

            virtual std::string filename() override final;
            virtual std::vector< rlib::common::sensor > sensors()
                override final;
            virtual std::vector< rlib::common::sample > samples(
                double begin, double end) override final;
            virtual std::vector< rlib::common::event_data > events(
                double begin, double end) override final;
            virtual double length() override final;

            uint16_t port();
            int64_t remote_id();

            void receive();
        };
    }
}
