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
#include "rlib/common/event_data.h"
#include "rlib/common/reader.h"
#include "rlib/common/sample.h"
#include "rlib/common/sensor.h"

// StdLib
#include <cstdint>
#include <experimental/optional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rlib {
    namespace common {
        class statistic_reader : public reader {
            private:
            std::map< statistic_data,
                std::vector< std::experimental::optional< double > > >
                _analysis_data;

            public:
            std::shared_ptr< reader > _reader;

            private:
            void analyse_data();

            public:
            statistic_reader(std::shared_ptr< reader > reader);
            virtual ~statistic_reader() override = default;

            virtual std::string filename() override final;
            virtual std::vector< sensor > sensors() override final;
            virtual std::vector< common::sample > samples(
                double begin, double end, int_fast32_t r) override final;
            virtual std::vector< common::sample > samples(
                double begin, double end) override final;
            virtual std::vector< event_data > events(
                double begin, double end) override final;
            virtual std::vector< std::experimental::optional< double > >
                statistic(statistic_data t) override final;
            virtual double length() override final;
        };
    }
}
