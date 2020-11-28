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

#include <unordered_map>
#include <seal/seal.h>
#include <ppcnn_server/ppcnn_server_keycontainer.hpp>


namespace ppcnn_server
{
    struct EncryptionKeys
    {
        EncryptionKeys(const seal::EncryptionParameters& params,
                       const seal::PublicKey& pubkey,
                       const seal::RelinKeys& relinkey)
            : params_(new seal::EncryptionParameters(params)),
              pubkey_(new seal::PublicKey(pubkey)),
              relinkey_(new seal::RelinKeys(relinkey))
        {}

        std::shared_ptr<seal::EncryptionParameters> params_;
        std::shared_ptr<seal::PublicKey> pubkey_;
        std::shared_ptr<seal::RelinKeys> relinkey_;
    };
    
    struct KeyContainer::Impl
    {
        Impl()
        {}

        std::unordered_map<int32_t, EncryptionKeys> keymap_;
    };

    KeyContainer::KeyContainer()
    {}

    void KeyContainer::register_keys(const int32_t key_id,
                                     const seal::EncryptionParameters& params,
                                     const seal::PublicKey& pubkey,
                                     const seal::RelinKeys& relinkey)
    {
        EncryptionKeys enckeys(params, pubkey, relinkey);
        pimpl_->keymap_.emplace(key_id, enckeys);
    }
    
} /* namespace ppcnn_server */
