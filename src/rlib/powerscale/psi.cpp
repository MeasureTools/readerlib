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
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Own
#include "rlib/powerscale/data_stream.h"
#include "rlib/powerscale/psd.h"
#include "rlib/powerscale/psi.h"

// StdLib
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

rlib::powerscale::psi::psi(std::string filename)
{
    this->_filename = filename;

    boost::property_tree::ptree psi_xml;
    boost::property_tree::read_xml(filename, psi_xml);

    // Get Checksum
    auto psi_checksum =
        psi_xml.get< std::string >("PSI.Checksum.<xmlattr>.Value");
    try {
        this->_checksum =
            boost::lexical_cast< uint32_t >(psi_checksum.c_str() + 2);
    }
    catch (boost::bad_lexical_cast&) {
        throw std::runtime_error(
            "Invalid Checksum Value " + psi_checksum + " in " + filename);
    }

    // Get SamplingRate (and transform it from kHz to Hz)
    this->_sampling_rate = psi_xml.get< uint64_t >(
                               "PSI.Measurement.SamplingRate.<xmlattr>.Value") *
                           1000;

    // Get SamplingCount
    this->_sampling_count = psi_xml.get< uint64_t >(
        "PSI.Measurement.SamplingCount.<xmlattr>.Value");

    // Get all inner DataStreams (Probes)
    for (auto& v : psi_xml.get_child("PSI.DataStream")) {
        // Skip all non DataStream Elements
        if (v.first != "DataStream") {
            continue;
        }
        auto dstream = rlib::powerscale::data_stream();
        {
            double nan = std::numeric_limits< double >::quiet_NaN();
            dstream.number = v.second.get< uint64_t >("<xmlattr>.Id", 0);
            dstream.port = v.second.get< uint64_t >("<xmlattr>.ProbeID", 0);
            dstream.kind = v.second.get< uint64_t >("<xmlattr>.ProbeKind", 0);

            dstream.min_voltage =
                v.second.get< double >("<xmlattr>.voltageMin", nan);
            dstream.max_voltage =
                v.second.get< double >("<xmlattr>.voltageMax", nan);
            dstream.min_current =
                v.second.get< double >("<xmlattr>.currentMin", nan);
            dstream.max_current =
                v.second.get< double >("<xmlattr>.currentMax", nan);
        }
        this->_data_streams.push_back(std::move(dstream));
    }

    // Get all PSDFiles
    for (auto& v : psi_xml.get_child("PSI.PSD")) {
        // Skip all non PSDFile Elements
        if (v.first != "PSDFile") {
            continue;
        }
        psd psd;
        {
            psd.id = v.second.get< int64_t >("<xmlattr>.Id", 0);
            psd.offset = v.second.get< uint64_t >("<xmlattr>.Offset", 0);
            psd.data_count = v.second.get< uint64_t >("<xmlattr>.DataCount", 0);
            psd.event_count =
                v.second.get< uint64_t >("<xmlattr>.EventCount", 0);
            psd.filename = std::string(this->_filename.begin(),
                               this->_filename.begin() +
                                   std::ptrdiff_t(this->_filename.size() - 4)) +
                           "_" + std::to_string(psd.id) + ".psd";
        }
        this->_psds.push_back(std::move(psd));
    }
}

std::string rlib::powerscale::psi::filename()
{
    return this->_filename;
}

std::string rlib::powerscale::psi::version()
{
    return this->_version;
}

uint32_t rlib::powerscale::psi::checksum()
{
    return this->_checksum;
}

uint64_t rlib::powerscale::psi::sampling_rate()
{
    return this->_sampling_rate;
}

double rlib::powerscale::psi::update_rate()
{
    return double(1) / double(this->_sampling_rate);
}

uint64_t rlib::powerscale::psi::sampling_count()
{
    return this->_sampling_count;
}

std::vector< rlib::powerscale::data_stream > rlib::powerscale::psi::
    data_streams()
{
    return this->_data_streams;
}

std::vector< rlib::powerscale::psd > rlib::powerscale::psi::psds()
{
    return this->_psds;
}
