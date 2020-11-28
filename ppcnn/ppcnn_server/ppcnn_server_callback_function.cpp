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

#include <iostream>
#include <cstring>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <ppcnn_share/ppcnn_packet.hpp>
#include <ppcnn_share/ppcnn_plaindata.hpp>
#include <ppcnn_share/ppcnn_encdata.hpp>
#include <ppcnn_share/ppcnn_cli2srvparam.hpp>
#include <ppcnn_share/ppcnn_srv2cliparam.hpp>
#include <ppcnn_share/ppcnn_seal_utility.hpp>
#include <ppcnn_server/ppcnn_server_callback_function.hpp>
#include <ppcnn_server/ppcnn_server_callback_param.hpp>
#include <ppcnn_server/ppcnn_server_query.hpp>
#include <ppcnn_server/ppcnn_server_result.hpp>
#include <ppcnn_server/ppcnn_server_calcmanager.hpp>
#include <ppcnn_server/ppcnn_server_keycontainer.hpp>
#include <ppcnn_server/ppcnn_server_state.hpp>

#include <seal/seal.h>

#define ENABLE_LOCAL_DEBUG

namespace ppcnn_server
{

// CallbackFunction for Encryption keys
DEFUN_DATA(CallbackFunctionEncryptionKeys)
{
    STDSC_LOG_INFO("Received encryption keys. (current state : %s)",
                   state.current_state_str().c_str());
    
    DEF_CDATA_ON_ALL(ppcnn_server::CommonCallbackParam);
    //auto& calc_manager = cdata_a->calc_manager_;
    auto& key_container = cdata_a->key_container_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    ppcnn_share::PlainData<ppcnn_share::C2SEnckeyParam> rplaindata;
    rplaindata.load(rstream);
    const auto param = rplaindata.data();
    STDSC_LOG_DEBUG("param: key_id: %d", param.key_id);

    // Load encryption parameter using dummy binary stream.
    // ** seal::EncryptionParameters::load() requires a binary stream. **
#if 1
    seal::EncryptionParameters enc_params(seal::scheme_type::CKKS);
    ppcnn_share::seal_utility::read_from_binary_stream(
        rstream, rbuffstream.data(), param.enc_params_stream_sz, enc_params);
#else
    seal::EncryptionParameters enc_params(seal::scheme_type::CKKS);
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + param.enc_params_stream_sz);

        std::istringstream iss(s, std::istringstream::binary);
        auto loaded_sz = enc_params.load(iss);
        STDSC_LOG_DEBUG("Loaded %lu bytes to stream.", loaded_sz);

        // update stream position
        rstream.seekg(loaded_sz, std::ios_base::cur);
    }
#endif
#if defined ENABLE_LOCAL_DEBUG
    ppcnn_share::seal_utility::write_to_file("params.txt", enc_params);
#endif

    auto context = seal::SEALContext::Create(enc_params);

#if 1
    seal::PublicKey pubkey;
    ppcnn_share::seal_utility::read_from_binary_stream(
        rstream, rbuffstream.data(), param.pubkey_stream_sz, enc_params, pubkey);
#else
    seal::PublicKey pubkey;
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + param.pubkey_stream_sz);

        std::istringstream iss(s, std::istringstream::binary);
        auto loaded_sz = pubkey.load(context, iss);
        STDSC_LOG_DEBUG("Loaded %lu bytes to stream.", loaded_sz);

        // update stream position
        rstream.seekg(loaded_sz, std::ios_base::cur);
    }
    //pubkey.load(context, rstream);
#endif
#if defined ENABLE_LOCAL_DEBUG
    ppcnn_share::seal_utility::write_to_file("pubkey.txt", pubkey);
#endif

#if 1
        seal::RelinKeys relinkey;
    ppcnn_share::seal_utility::read_from_binary_stream(
        rstream, rbuffstream.data(), param.relinkey_stream_sz, enc_params, relinkey);
#else
    seal::RelinKeys relinkey;
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + param.relinkey_stream_sz);

        std::istringstream iss(s, std::istringstream::binary);
        auto loaded_sz = relinkey.load(context, iss);
        STDSC_LOG_DEBUG("Loaded %lu bytes to stream.", loaded_sz);

        // update stream position
        rstream.seekg(loaded_sz, std::ios_base::cur);
    }
#endif
#if defined ENABLE_LOCAL_DEBUG
    ppcnn_share::seal_utility::write_to_file("relinkey.txt", relinkey);
#endif


    key_container.register_keys(param.key_id,
                                enc_params, pubkey, relinkey);

    const auto& e = key_container.get_params(param.key_id);
}
    
// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionQuery)
{
    STDSC_LOG_INFO("Received query. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(ppcnn_server::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    // load plaindata (param)
    ppcnn_share::PlainData<ppcnn_share::C2SQueryParam> rplaindata;
    rplaindata.load(rstream);
    const auto& c2s_param = rplaindata.data();
    STDSC_LOG_DEBUG("c2s_param: img_info: {%s}, "
                    "enc_params_stream_sz: %lu, "
                    "enc_inputs_stream_sz: %lu\n",
                    c2s_param.img_info.to_string().c_str(),
                    c2s_param.enc_params_stream_sz,
                    c2s_param.enc_inputs_stream_sz);


    // Load encryption parameter using dummy binary stream.
    // ** seal::EncryptionParameters::load() requires a binary stream. **
    seal::EncryptionParameters params(seal::scheme_type::CKKS);
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + c2s_param.enc_params_stream_sz);

        std::istringstream iss(s, std::istringstream::binary);
        auto loaded_sz = params.load(iss);
        STDSC_LOG_DEBUG("Loaded %lu bytes to stream.", loaded_sz);

        // update stream position
        rstream.seekg(loaded_sz, std::ios_base::cur);
    }

    // Load encryption parameter using dummy binary stream.
    // ** seal::Ciphertext::load() requires a binary stream. **
    ppcnn_share::EncData enc_inputs(params);
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + c2s_param.enc_inputs_stream_sz);
        std::istringstream iss(s, std::istringstream::binary);
        enc_inputs.load(iss);

        // update stream position
        rstream.seekg(c2s_param.enc_inputs_stream_sz, std::ios_base::cur);
    }

#if defined ENABLE_LOCAL_DEBUG
    //ppcnn_share::seal_utility::write_to_file("enc_inputs.txt", enc_inputs.data());
    printf("enc_inputs.size(): %ld\n", enc_inputs.vdata().size());
    for (size_t i=0; i<enc_inputs.vdata().size(); ++i) {
        std::ostringstream oss;
        oss << "enc_inputs-" << i;
        ppcnn_share::seal_utility::write_to_file(oss.str(), enc_inputs.vdata()[i]);
    }
#endif

    Query query(c2s_param.img_info, enc_inputs.vdata());
    int32_t query_id = calc_manager.push_query(query);

    ppcnn_share::PlainData<int32_t> splaindata;
    splaindata.push(query_id);

    auto sz = splaindata.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save(sstream);

    STDSC_LOG_INFO("Sending query ack. (query ID: %d)", query_id);
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(ppcnn_share::kControlCodeDataQueryID, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventQuery);
}

// CallbackFunction for Result Request
DEFUN_UPDOWNLOAD(CallbackFunctionResultRequest)
{
    STDSC_LOG_INFO("Received result request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(ppcnn_server::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    // load plaindata (param)
    ppcnn_share::PlainData<int32_t> rplaindata;
    rplaindata.load(rstream);
    const auto query_id = rplaindata.vdata()[0];
    const auto enc_params_stream_sz = rplaindata.vdata()[1];

    // Load encryption parameter using dummy binary stream.
    // ** seal::EncryptionParameters::load() requires a binary stream. **
    seal::EncryptionParameters params(seal::scheme_type::CKKS);
    {
        auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
        std::string s(p, p + enc_params_stream_sz);

        std::istringstream iss(s, std::istringstream::binary);
        auto loaded_sz = params.load(iss);
        STDSC_LOG_DEBUG("Loaded %lu bytes to stream.", loaded_sz);

        // update stream position
        rstream.seekg(loaded_sz, std::ios_base::cur);
    }

    printf("0_1\n");
    Result result;
    calc_manager.pop_result(query_id, result);

    printf("0_2\n");
    ppcnn_share::EncData enc_outputs(params, result.ctxts_.data(), result.ctxts_.size());
#if defined ENABLE_LOCAL_DEBUG
    //ppcnn_share::seal_utility::write_to_file("result.txt", enc_outputs.vdata());
#endif

    printf("1\n");
    ppcnn_share::PlainData<ppcnn_share::Srv2CliParam> splaindata;
    ppcnn_share::Srv2CliParam s2c_param;
    s2c_param.result = result.status_ ? ppcnn_share::kServerCalcResultSuccess : ppcnn_share::kServerCalcResultFailed;
    s2c_param.enc_results_stream_sz = enc_outputs.stream_size();
    splaindata.push(s2c_param);
    
    printf("2\n");
    auto sz = splaindata.stream_size() + enc_outputs.stream_size();
    //auto sz = splaindata.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    printf("3\n");
    splaindata.save(sstream);

    printf("4\n");
    {
        std::ostringstream oss(std::istringstream::binary);
        enc_outputs.save(oss);
    
        auto* p = static_cast<uint8_t*>(sbuffstream.data()) + sstream.tellp();
        std::memcpy(p, oss.str().data(), enc_outputs.stream_size());
        sstream.seekp(enc_outputs.stream_size(), std::ios_base::cur);
    }
    //enc_outputs.save(sstream);

    printf("5\n");
    STDSC_LOG_INFO("Sending result. (query ID: %d)", query_id);
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(ppcnn_share::kControlCodeDataResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventResultRequest);
}

} /* namespace ppcnn_server */
