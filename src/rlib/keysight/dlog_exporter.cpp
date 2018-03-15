/**
 * Copyright (c) 2017, Daniel "Dadie" Korner
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
#include "rlib/keysight/dlog_exporter.h"

// StdLib
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <map>
#include <numeric>
#include <vector>

// Export data from begin (in seconds) till end (in seconds) to output stream
void rlib::keysight::dlog_exporter::data_export(
    double begin, double end, std::ostream& output)
{
    this->data_export(begin, end, -1, output);
}

// Export data from begin (in seconds) till end (in seconds) to output stream
// with resolution r
void rlib::keysight::dlog_exporter::data_export(
    double begin, double end, int_fast32_t r, std::ostream& output)
{
    std::vector< rlib::common::sample > data;
    if (r > 0) {
        data = this->reader->samples(begin, end, r);
    }
    else {
        data = this->reader->samples(begin, end);
    }

    output << "<!-- N6700X dlog settings -->" << std::endl;
    output << "<dlog>" << std::endl;

    std::vector< std::map< std::string, size_t > > channels;

    auto sensors = this->reader->sensors();
    for (size_t i = 0; i < sensors.size(); ++i) {
        if (sensors[ i ].unit != "A" && sensors[ i ].unit != "V") {
            std::cerr << "ERROR Unit " << sensors[ i ].unit
                      << " is not supported in .dlog" << std::endl;
            return;
        }
        if (channels.empty()) {
            channels.push_back(std::map< std::string, size_t >());
            if (sensors[ i ].unit == "A") {
                channels[ 0 ][ "A" ] = i;
            }
            if (sensors[ i ].unit == "V") {
                channels[ 0 ][ "V" ] = i;
            }
        }
        else {
            if (sensors[ i ].unit == "A") {
                if (channels[ channels.size() - 1 ].count("A") > 0) {
                    channels.push_back(std::map< std::string, size_t >());
                    channels[ channels.size() - 1 ][ "A" ] = i;
                }
                else {
                    channels[ channels.size() - 1 ][ "A" ] = i;
                }
            }
            else if (sensors[ i ].unit == "V") {
                if (channels[ channels.size() - 1 ].count("V") > 0) {
                    channels.push_back(std::map< std::string, size_t >());
                    channels[ channels.size() - 1 ][ "V" ] = i;
                }
                else {
                    channels[ channels.size() - 1 ][ "V" ] = i;
                }
            }
        }
    }

    int channelId = 1;
    for (auto channel : channels) {
        output << "        <channel id=\"" << channelId << "\">" << std::endl;
        output << "                <curr_trig_lev>0</curr_trig_lev>"
               << std::endl;
        output << "                <volt_trig_lev>0</volt_trig_lev>"
               << std::endl;
        output << "                <curr_trig_slope>0</curr_trig_slope>"
               << std::endl;
        output << "                <volt_trig_slope>0</volt_trig_slope>"
               << std::endl;
        output << "                <sense_volt>"
               << (channel.count("V") > 0 ? 1 : 0) << "</sense_volt>"
               << std::endl;
        output << "                <sense_curr>"
               << (channel.count("A") > 0 ? 1 : 0) << "</sense_curr>"
               << std::endl;
        output << "                <volt_range>0</volt_range>" << std::endl;
        output << "                <curr_range>0</curr_range>" << std::endl;
        output << "                <curr_auto_range>1</curr_auto_range>"
               << std::endl;
        output << "                <volt_auto_range>1</volt_auto_range>"
               << std::endl;
        output << "                <ident>" << std::endl;
        output << "                        <model>N6785A</model>" << std::endl;
        output << "                        <option>" << std::endl;
        output << "                                <1ua>0</1ua>" << std::endl;
        output << "                                <2ua>0</2ua>" << std::endl;
        output << "                                <lga>0</lga>" << std::endl;
        output << "                                <relay>0</relay>"
               << std::endl;
        output << "                                <reverse>0</reverse>"
               << std::endl;
        output << "                        </option>" << std::endl;
        output << "                </ident>" << std::endl;
        output << "        </channel>" << std::endl;
        ++channelId;
    }

    output << "        <frame>" << std::endl;
    output << "                <sense_minmax>0</sense_minmax>" << std::endl;
    output << "                <trig_source>1</trig_source>" << std::endl;
    output << "                <time>600</time>" << std::endl;
    output << "                <offset>0</offset>" << std::endl;
    output << "                <tint>" << double(1) / double(r) << "</tint>"
           << std::endl;
    output << "                <date>\"Wed Jan 25 17:47:22 2017\"</date>"
           << std::endl;
    output << "        </frame>" << std::endl;

    // This part more or less only contains meta data for the device
    output << "        <gui_chan id=\"1\">" << std::endl;
    output << "                <volt_trace>1</volt_trace>" << std::endl;
    output << "                <curr_trace>1</curr_trace>" << std::endl;
    output << "                <volt_gain>2</volt_gain>" << std::endl;
    output << "                <volt_offset>0</volt_offset>" << std::endl;
    output << "                <curr_gain>0.050000001</curr_gain>" << std::endl;
    output << "                <curr_offset>0</curr_offset>" << std::endl;
    output << "                <power_trace>0</power_trace>" << std::endl;
    output << "                <power_gain>0.050000001</power_gain>"
           << std::endl;
    output << "                <power_offset>0</power_offset>" << std::endl;
    output << "        </gui_chan>" << std::endl;
    output << "        <gui_frame>" << std::endl;
    output << "                <horiz_gain>5</horiz_gain>" << std::endl;
    output << "                <horiz_offset>0</horiz_offset>" << std::endl;
    output << "                <date_time>1</date_time>" << std::endl;
    output << "                "
              "<base_filename>\"External:\\default.dlog\"</base_filename>"
           << std::endl;
    output << "                <marker1_pos>0.0000000</marker1_pos>"
           << std::endl;
    output << "                <marker2_pos>00.000000</marker2_pos>"
           << std::endl;
    output << "        </gui_frame>" << std::endl;
    output << "</dlog>" << std::endl;

    // 5 times Zero Bytes
    output << uint8_t(0) << uint8_t(0) << uint8_t(0) << uint8_t(0)
           << uint8_t(0);

    // Values as 32 Bit floats
    for (const auto& datum : data) {
        for (size_t i = channels.size(); 0 < i; --i) {
            if (channels[ i - 1 ].count("A") > 0) {
                float value = float(datum.values.at(channels[ i - 1 ][ "A" ]));
                // Little to Big Endian! (UNSAFE)
                char* mem = reinterpret_cast< char* >(&value);
                for (size_t j = 0; j < sizeof(value); ++j) {
                    output << *mem;
                    ++mem;
                }
            }
        }
        for (size_t i = channels.size(); 0 < i; --i) {
            if (channels[ i - 1 ].count("V") > 0) {
                float value = float(datum.values.at(channels[ i - 1 ][ "V" ]));
                // Little to Big Endian! (UNSAFE)
                char* mem = reinterpret_cast< char* >(&value);
                for (size_t j = 0; j < sizeof(value); ++j) {
                    output << *mem;
                    ++mem;
                }
            }
        }
    }
}
