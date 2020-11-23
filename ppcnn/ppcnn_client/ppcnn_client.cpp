#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <ppcnn_share/ppcnn_utility.hpp>
#include <ppcnn_share/ppcnn_encdata.hpp>
#include <ppcnn_share/ppcnn_packet.hpp>
#include <ppcnn_share/ppcnn_plaindata.hpp>
#include <ppcnn_share/ppcnn_cli2srvparam.hpp>
#include <ppcnn_share/ppcnn_seal_utility.hpp>
#include <ppcnn_share/ppcnn_srv2cliparam.hpp>
#include <ppcnn_client/ppcnn_client_result_thread.hpp>
#include <ppcnn_client/ppcnn_client.hpp>

namespace ppcnn_client
{

struct ResultCallback
{
    std::shared_ptr<ResultThread> thread;
    ResultThreadParam param;
};
    
struct Client::Impl
{
    Impl(const char* host, const char* port,
         const seal::EncryptionParameters& enc_params)
        : host_(host),
          port_(port),
          enc_params_(enc_params),
          client_()
    {
    }

    ~Impl(void)
    {
        disconnect();
    }

    void connect(const uint32_t retry_interval_usec,
                 const uint32_t timeout_sec)
    {
        client_.connect(host_, port_, retry_interval_usec, timeout_sec);
    }

    void disconnect(void)
    {
        client_.close();
    }

    int32_t send_query(const int32_t key_id, 
                       const ppcnn_share::EncData& enc_inputs)
    {
        ppcnn_share::PlainData<ppcnn_share::Cli2SrvParam> splaindata;
        ppcnn_share::Cli2SrvParam cli2srvparam {key_id};
        splaindata.push(cli2srvparam);

        auto sz = (splaindata.stream_size()
                   + ppcnn_share::seal_utility::stream_size(enc_params_)
                   + enc_inputs.stream_size());
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);
        
        splaindata.save_to_stream(stream);
        //seal::EncryptionParameters::Save(enc_params_, stream);
        enc_params_.save(stream);
        enc_inputs.save_to_stream(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(ppcnn_share::kControlCodeUpDownloadQuery, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        ppcnn_share::PlainData<int32_t> rplaindata;
        rplaindata.load_from_stream(rstream);

        return rplaindata.data();
    }

    void recv_results(const int32_t query_id, bool& status, ppcnn_share::EncData& enc_result)
    {
        ppcnn_share::PlainData<int32_t> splaindata;
        splaindata.push(query_id);

        auto sz = (splaindata.stream_size()
                   + ppcnn_share::seal_utility::stream_size(enc_params_));
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save_to_stream(stream);
        //seal::EncryptionParameters::Save(enc_params_, stream);
        enc_params_.save(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(ppcnn_share::kControlCodeUpDownloadResult, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);

        ppcnn_share::PlainData<ppcnn_share::Srv2CliParam> rplaindata;
        rplaindata.load_from_stream(rstream);
        auto& srv2cliparam = rplaindata.data();
        status = srv2cliparam.result == ppcnn_share::kServerCalcResultSuccess;

        if (status) {
            enc_result.load_from_stream(rstream);
#if defined ENABLE_LOCAL_DEBUG
            ppcnn_share::seal_utility::write_to_file("result.txt", enc_result.data());
#endif
        }
    }

    void wait(const int32_t query_id) const
    {
        if (cbmap_.count(query_id)) {
            auto& rcb = cbmap_.at(query_id);
            rcb.thread->wait();
        }
    }

    const char* host_;
    const char* port_;
    const seal::EncryptionParameters& enc_params_;
    stdsc::Client client_;
    std::unordered_map<int32_t, ResultCallback> cbmap_;
};

Client::Client(const char* host, const char* port,
                   const seal::EncryptionParameters& enc_params)
    : pimpl_(new Impl(host, port, enc_params))
{
}

void Client::connect(const uint32_t retry_interval_usec,
                       const uint32_t timeout_sec)
{
    STDSC_LOG_INFO("Connect to computation server.");
    pimpl_->connect(retry_interval_usec, timeout_sec);
}

void Client::disconnect(void)
{
    STDSC_LOG_INFO("Disconnect from computation server.");
    pimpl_->disconnect();
}

int32_t Client::send_query(const int32_t key_id, const ppcnn_share::EncData& enc_inputs) const
{
    STDSC_LOG_INFO("Send query: sending query to computation server. (key_id: %d)", key_id);
    auto query_id = pimpl_->send_query(key_id, enc_inputs);
    STDSC_LOG_INFO("Send query: received query ID (#%d)", query_id);
    return query_id;
}

int32_t Client::send_query(const int32_t key_id,
                             const ppcnn_share::EncData& enc_inputs,
                             cbfunc_t cbfunc,
                             void* cbfunc_args) const
{
    int32_t query_id = pimpl_->send_query(key_id, enc_inputs);
    STDSC_LOG_INFO("Set callback function for query #%d", query_id);
    set_callback(query_id, cbfunc, cbfunc_args);
    return query_id;
}

void Client::recv_results(const int32_t query_id, bool& status, ppcnn_share::EncData& enc_result) const
{
    STDSC_LOG_INFO("Waiting for query #%d results ...", query_id);
    pimpl_->recv_results(query_id, status, enc_result);
}

void Client::set_callback(const int32_t query_id, cbfunc_t func, void* args) const
{
    ResultCallback rcb;
    rcb.thread = std::make_shared<ResultThread>(*this, pimpl_->enc_params_, func, args);
    rcb.param  = {query_id};
    pimpl_->cbmap_[query_id] = rcb;
    pimpl_->cbmap_[query_id].thread->start(pimpl_->cbmap_[query_id].param);
}

void Client::wait(const int32_t query_id) const
{
    pimpl_->wait(query_id);
}

} /* namespace ppcnn_client */