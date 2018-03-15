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
#include "rlib/common/sample.h"

// StdLib
#include <algorithm>

rlib::common::sample::sample(double _time, std::vector< double > _values)
{
    this->time = _time;
    this->values = _values;
}

rlib::common::sample rlib::common::sample::operator+(
    const rlib::common::sample& other)
{
    rlib::common::sample r = *this;
    {
        r += other;
    }
    return r;
}

rlib::common::sample rlib::common::sample::operator-(
    const rlib::common::sample& other)
{
    rlib::common::sample r = *this;
    {
        r -= other;
    }
    return r;
}

rlib::common::sample rlib::common::sample::operator+(const double& other)
{
    rlib::common::sample r = *this;
    {
        r += other;
    }
    return r;
}

rlib::common::sample rlib::common::sample::operator-(const double& other)
{
    rlib::common::sample r = *this;
    {
        r -= other;
    }
    return r;
}

rlib::common::sample rlib::common::sample::operator/(const double other)
{
    rlib::common::sample r = *this;
    {
        r /= other;
    }
    return r;
}

rlib::common::sample rlib::common::sample::operator*(const double other)
{
    rlib::common::sample r = *this;
    {
        r *= other;
    }
    return r;
}

rlib::common::sample& rlib::common::sample::operator+=(
    const rlib::common::sample& other)
{
    this->time += other.time;
    for (size_t i = 0; i < this->values.size(); ++i) {
        if (i < other.values.size()) {
            this->values[ i ] += other.values[ i ];
        }
    }
    for (size_t i = this->values.size(); i < other.values.size(); ++i) {
        this->values.push_back(other.values[ i ]);
    }
    return *this;
}

rlib::common::sample& rlib::common::sample::operator-=(
    const rlib::common::sample& other)
{
    this->time -= other.time;
    for (size_t i = 0; i < this->values.size(); ++i) {
        if (i < other.values.size()) {
            this->values[ i ] -= other.values[ i ];
        }
    }
    for (size_t i = this->values.size(); i < other.values.size(); ++i) {
        this->values.push_back(other.values[ i ]);
    }
    return *this;
}

rlib::common::sample& rlib::common::sample::operator+=(const double& other)
{
    this->time += other;
    for (size_t i = 0; i < this->values.size(); ++i) {
        this->values[ i ] += other;
    }
    return *this;
}

rlib::common::sample& rlib::common::sample::operator-=(const double& other)
{
    this->time -= other;
    for (size_t i = 0; i < this->values.size(); ++i) {
        this->values[ i ] -= other;
    }
    return *this;
}

rlib::common::sample& rlib::common::sample::operator/=(const double other)
{
    this->time /= other;
    for (size_t i = 0; i < this->values.size(); ++i) {
        this->values[ i ] /= other;
    }
    return *this;
}

rlib::common::sample& rlib::common::sample::operator*=(const double other)
{
    this->time *= other;
    for (size_t i = 0; i < this->values.size(); ++i) {
        this->values[ i ] *= other;
    }
    return *this;
}

bool rlib::common::sample::operator==(const rlib::common::sample& rhs)
{
    if (this->time > rhs.time || this->time < rhs.time) {
        return false;
    }
    if (this->values.size() != rhs.values.size()) {
        return false;
    }
    for (size_t i = 0; i < this->values.size(); ++i) {
        if (this->values[ i ] > rhs.values[ i ] ||
            this->values[ i ] < rhs.values[ i ]) {
            return false;
        }
    }
    return true;
}

bool rlib::common::sample::operator!=(const rlib::common::sample& rhs)
{
    return !((*this) == rhs);
}