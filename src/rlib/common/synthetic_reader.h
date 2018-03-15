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
#include <experimental/optional>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace rlib {
    namespace common {
        class synthetic_reader : public reader {
            private:
            std::string _filename;
            std::function< double(double) > _succeeding_time;
            std::function< std::vector< event_data >(double, double) > _events;
            std::vector< std::function< double(double) > > _sensors;
            double _length;

            public:
            synthetic_reader(std::function< double(double) > succeeding_time,
                std::function< std::vector< event_data >(double, double) >
                    events,
                std::vector< std::function< double(double) > > sensors,
                double length = 10.0);
            virtual ~synthetic_reader() override = default;

            virtual std::string filename() override final;
            virtual std::vector< sensor > sensors() override final;
            virtual std::vector< common::sample > samples(
                double begin, double end) override final;
            virtual std::vector< event_data > events(
                double begin, double end) override final;
            virtual double length() override final;
        };
    }
}
