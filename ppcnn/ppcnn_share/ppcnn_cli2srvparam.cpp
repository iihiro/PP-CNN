/*
 * Copyright 2018 Yamana Laboratory, Waseda University
 * Supported by JST CREST Grant Number JPMJCR1503, Japan.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE‐2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ppcnn_share/ppcnn_cli2srvparam.hpp>

namespace ppcnn_share
{

std::ostream& operator<<(std::ostream& os, const Cli2SrvParam& param)
{
    os << param.img_info.width       << std::endl;
    os << param.img_info.height      << std::endl;
    os << param.img_info.channels    << std::endl;
    os << param.img_info.labels      << std::endl;
    os << param.enc_params_stream_sz << std::endl;
    os << param.enc_inputs_stream_sz << std::endl;
    return os;
}

std::istream& operator>>(std::istream& is, Cli2SrvParam& param)
{
    is >> param.img_info.width;
    is >> param.img_info.height;
    is >> param.img_info.channels;
    is >> param.img_info.labels;
    is >> param.enc_params_stream_sz;
    is >> param.enc_inputs_stream_sz;
    return is;
}
    
} /* namespace ppcnn_share */
