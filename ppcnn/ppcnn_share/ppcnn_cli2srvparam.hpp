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

#ifndef PPCNN_CLI2SRVPARAM_HPP
#define PPCNN_CLI2SRVPARAM_HPP

#include <iostream>

namespace ppcnn_share
{
    
/**
 * @brief This class is used to hold the parameters to compute on encryptor.
 */
struct Cli2SrvParam
{
    int32_t  key_id;
};

std::ostream& operator<<(std::ostream& os, const Cli2SrvParam& param);
std::istream& operator>>(std::istream& is, Cli2SrvParam& param);

} /* namespace ppcnn_share */

#endif /* PPCNN_CLI2SRVPARAM_HPP */
