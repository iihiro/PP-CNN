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
#include <ppcnn_server/ppcnn_server_callback_function.hpp>
#include <ppcnn_server/ppcnn_server_callback_param.hpp>
#include <ppcnn_server/ppcnn_server_query.hpp>
#include <ppcnn_server/ppcnn_server_result.hpp>
#include <ppcnn_server/ppcnn_server_calcmanager.hpp>
#include <ppcnn_server/ppcnn_server_state.hpp>

#include <seal/seal.h>

#define ENABLE_LOCAL_DEBUG

namespace ppcnn_server
{

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
    ppcnn_share::PlainData<ppcnn_share::Cli2SrvParam> rplaindata;
    rplaindata.load_from_stream(rstream);
    const auto& cli2srvparam = rplaindata.data();
    STDSC_LOG_DBG("cli2srvparam: %d", cli2srvparam.dummy);

    // load encryption parameters
    seal::EncryptionParameters params(seal::scheme_type::CKKS);
    params.load(rstream);

    // load encryption inputs
    ppcnn_share::EncData enc_inputs(params);
    enc_inputs.load_from_stream(rstream);
#if defined ENABLE_LOCAL_DEBUG
    ppcnn_share::seal_utility::write_to_file("query.txt", enc_inputs.data());
#endif

    Query query(cli2srvparam.img_info, enc_inputs.vdata());
    int32_t query_id = calc_manager.push_query(query);

    ppcnn_share::PlainData<int32_t> splaindata;
    splaindata.push(query_id);

    auto sz = splaindata.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save_to_stream(sstream);

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
    rplaindata.load_from_stream(rstream);
    const auto query_id = rplaindata.data();

    // load encryption parameters
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    //params = seal::EncryptionParameters::Load(rstream);
    params.load(rstream);

    Result result;
    calc_manager.pop_result(query_id, result);

    ppcnn_share::PlainData<ppcnn_share::Srv2CliParam> splaindata;
    ppcnn_share::Srv2CliParam srv2cliparam;
    srv2cliparam.result = result.status_ ? ppcnn_share::kServerCalcResultSuccess : ppcnn_share::kServerCalcResultFailed;
    splaindata.push(srv2cliparam);
    
    ppcnn_share::EncData enc_outputs(params, result.ctxt_);
#if defined ENABLE_LOCAL_DEBUG
    ppcnn_share::seal_utility::write_to_file("result.txt", enc_outputs.data());
#endif
    
    auto sz = splaindata.stream_size() + enc_outputs.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save_to_stream(sstream);
    enc_outputs.save_to_stream(sstream);

    STDSC_LOG_INFO("Sending result. (query ID: %d)", query_id);
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(ppcnn_share::kControlCodeDataResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventResultRequest);
}

} /* namespace ppcnn_server */
