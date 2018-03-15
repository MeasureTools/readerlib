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
#include "rlib/powerscale/psi_reader.h"
#include "rlib/powerscale/psi.h"

// StdLib
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

rlib::powerscale::psi_reader::psi_reader(std::string filename)
    : _psi(filename)
{
}

std::string rlib::powerscale::psi_reader::filename()
{
    return this->_psi.filename();
}

std::vector< rlib::common::sensor > rlib::powerscale::psi_reader::sensors()
{
    std::vector< rlib::common::sensor > sensors;
    for (auto& dstream : this->_psi.data_streams()) {
        rlib::common::sensor currentSensor;
        {
            currentSensor.name =
                "Port[" + std::to_string(dstream.port) + "].Current";
            currentSensor.unit = "A";
            currentSensor.date = "---ADDME";
            currentSensor.version = this->_psi.version();
            currentSensor.sampling_interval = this->_psi.update_rate();
            currentSensor.channel = int(dstream.port);
        }
        sensors.push_back(currentSensor);

        rlib::common::sensor voltageSensor;
        {
            voltageSensor.name =
                "Port[" + std::to_string(dstream.port) + "].Voltage";
            voltageSensor.unit = "V";
            voltageSensor.date = "---ADDME";
            voltageSensor.version = this->_psi.version();
            voltageSensor.sampling_interval = this->_psi.update_rate();
            voltageSensor.channel = int(dstream.port);
        }
        sensors.push_back(voltageSensor);
    }
    return sensors;
}

std::vector< rlib::common::sample > rlib::powerscale::psi_reader::samples(
    double begin, double end)
{
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    std::vector< rlib::common::sample > dataVector;
    if (begin >= end) {
        return dataVector;
    }

    size_t valuesPerInterval = this->sensors().size();
    size_t sizePerValue = valuesPerInterval * 8 +
                          this->_psi.data_streams().size() * 2 + 2; // in Byte
    uint64_t begInMicroSeconds = uint64_t(begin * 1000 * 1000);
    uint64_t endInMicroSeconds = uint64_t(end * 1000 * 1000);
    uint64_t samplingRateInMicroSeconds =
        uint64_t(this->_psi.update_rate() * 1000 * 1000);
    uint64_t begSample = begInMicroSeconds / samplingRateInMicroSeconds;
    uint64_t endSample = endInMicroSeconds / samplingRateInMicroSeconds;
    std::vector< psd > affectedPSDs;

    for (auto& psd : this->_psi.psds()) {
        uint64_t psdMinSample = psd.offset / valuesPerInterval;
        uint64_t psdMaxSample =
            (psd.offset + psd.data_count) / valuesPerInterval;
        if (!(psdMaxSample < begSample || psdMinSample > endSample)) {
            affectedPSDs.push_back(psd);
        }
    }
    uint64_t curSample = begSample;
    for (auto& psd : affectedPSDs) {
        std::ifstream dataStream(psd.filename, std::ios::binary);
        {
            dataStream.ignore(
                std::streamsize((curSample - psd.offset) * sizePerValue));
        }
        while (dataStream.good() && curSample < endSample) {
            rlib::common::sample datum;
            // Calulate Time
            datum.time = double(curSample) * this->_psi.update_rate();
            // Add Values
            for (size_t i = 0; i < valuesPerInterval; i++) {
                // Read Values (As each value is a f32, float is used)
                double value = -1;
                dataStream.read(
                    reinterpret_cast< char* >(&value), sizeof(value));
                datum.values.push_back(value);
            }

            // Skip event data
            for (size_t i = 0; i < this->_psi.data_streams().size(); i++) {
                // Skip event data (each datastream can have an event of 2 Byte
                // size)
                dataStream.ignore(sizeof(uint16_t));
            }
            // Skip global event (2 Byte size)
            dataStream.ignore(sizeof(uint16_t));

            dataVector.push_back(datum);
            curSample++;
        }
    }
    return dataVector;
}

std::vector< rlib::common::event_data > rlib::powerscale::psi_reader::events(
    double begin, double end)
{
    if (begin < 0) {
        begin = 0;
    }
    if (end < 0) {
        end = this->length();
    }
    end = std::fmin(this->length(), end);
    std::vector< rlib::common::event_data > eventVector;
    if (begin >= end) {
        return eventVector;
    }

    size_t valuesPerInterval = this->sensors().size();
    size_t sizePerValue = valuesPerInterval * 8 +
                          this->_psi.data_streams().size() * 2 + 2; // in Byte
    uint64_t begInMicroSeconds = uint64_t(begin * 1000 * 1000);
    uint64_t endInMicroSeconds = uint64_t(end * 1000 * 1000);
    uint64_t samplingRateInMicroSeconds =
        uint64_t(this->_psi.update_rate() * 1000 * 1000);
    uint64_t begSample = begInMicroSeconds / samplingRateInMicroSeconds;
    uint64_t endSample = endInMicroSeconds / samplingRateInMicroSeconds;

    std::vector< psd > affectedPSDs;

    for (auto& psd : this->_psi.psds()) {
        uint64_t psdMinSample = psd.offset / valuesPerInterval;
        uint64_t psdMaxSample =
            (psd.offset + psd.data_count) / valuesPerInterval;
        if (!(psdMaxSample < begSample || psdMinSample > endSample)) {
            affectedPSDs.push_back(psd);
        }
    }
    uint64_t curSample = begSample;
    for (auto& psd : affectedPSDs) {
        std::ifstream dataStream(psd.filename, std::ios::binary);
        {
            dataStream.ignore(
                std::streamsize((curSample - psd.offset) * sizePerValue));
        }
        while (dataStream.good() && curSample < endSample) {
            // Calulate Time
            double curTime = double(curSample) * this->_psi.update_rate();

            // Skip Values
            for (uint64_t i = 0; i < valuesPerInterval; i++) {
                // Skip Values (As each value is a f32)
                dataStream.ignore(sizeof(uint32_t));
            }

            // Read event data
            for (uint64_t i = 0; i < this->_psi.data_streams().size(); i++) {
                uint16_t rawValue;
                dataStream.read(
                    reinterpret_cast< char* >(&rawValue), sizeof(rawValue));
                if ((rawValue & 0x80) != 0x0) {
                    rlib::common::event_data eventData;
                    {
                        eventData.time = curTime;
                        eventData.origin = int64_t(i);

                        auto valuesBytes =
                            reinterpret_cast< unsigned char* >(&rawValue);
                        eventData.raw_data.push_back(*valuesBytes);
                        eventData.raw_data.push_back(*(++valuesBytes));

                        std::stringstream hexValue;
                        {
                            for (auto x : eventData.raw_data) {
                                hexValue << std::hex << x;
                            }
                        }
                        eventData.message = hexValue.str();
                    }
                    eventVector.push_back(eventData);
                }
            }
            // Read global event (2 Byte size)
            uint16_t rawValue;
            dataStream.read(
                reinterpret_cast< char* >(&rawValue), sizeof(rawValue));
            if ((rawValue & 0x80) != 0x0) {
                rlib::common::event_data eventData;
                {
                    eventData.time = curTime;
                    eventData.origin = -1;

                    auto valuesBytes =
                        reinterpret_cast< unsigned char* >(&rawValue);
                    eventData.raw_data.push_back(*valuesBytes);
                    eventData.raw_data.push_back(*(++valuesBytes));

                    std::stringstream hexValue;
                    {
                        for (auto x : eventData.raw_data) {
                            hexValue << std::hex << x;
                        }
                    }
                    eventData.message = hexValue.str();
                }
                eventVector.push_back(eventData);
            }
            curSample++;
        }
    }
    return eventVector;
}

std::vector< std::experimental::optional< double > > rlib::powerscale::
    psi_reader::statistic(rlib::common::statistic_data t)
{
    std::vector< std::experimental::optional< double > > r;
    for (auto datastream : this->_psi.data_streams()) {
        if (t == rlib::common::statistic_data::MIN_VALUE) {
            r.push_back(datastream.min_current);
            r.push_back(datastream.min_voltage);
        }
        if (t == rlib::common::statistic_data::MAX_VALUE) {
            r.push_back(datastream.max_current);
            r.push_back(datastream.max_voltage);
        }
        if (t == rlib::common::statistic_data::AVG_VALUE) {
            r.push_back({});
            r.push_back({});
        }
    }
    return r;
}

double rlib::powerscale::psi_reader::length()
{
    return this->_psi.update_rate() * double(this->_psi.sampling_count()) /
           double(this->_psi.data_streams().size() * 2);
}
