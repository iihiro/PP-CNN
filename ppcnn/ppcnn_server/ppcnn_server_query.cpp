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

#include <ppcnn_share/ppcnn_utility.hpp>
#include <ppcnn_server/ppcnn_server_query.hpp>
#include <seal/seal.h>

namespace ppcnn_server
{
Query::Query(const std::vector<seal::Ciphertext>& ctxts)
{
    ctxts_.resize(ctxts.size());
    std::copy(ctxts.begin(), ctxts.end(), ctxts_.begin());
}

int32_t QueryQueue::push(const Query& data)
{
    auto id = ppcnn_share::utility::gen_uuid();
    super::push(id, data);
    return id;
}

} /* namespace ppcnn_server */