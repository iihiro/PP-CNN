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
#include <ppcnn_share/ppcnn_imageinfo.hpp>
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

    void register_enckeys(const int32_t key_id,
                          const seal::PublicKey& pubkey,
                          const seal::RelinKeys& relinkey)
    {
        ppcnn_share::PlainData<ppcnn_share::C2SEnckeyParam> splaindata;
        ppcnn_share::C2SEnckeyParam c2s_param;
        c2s_param.key_id               = key_id;
        c2s_param.enc_params_stream_sz = ppcnn_share::seal_utility::stream_size(enc_params_);
        c2s_param.pubkey_stream_sz     = ppcnn_share::seal_utility::stream_size(pubkey);
        c2s_param.relinkey_stream_sz   = ppcnn_share::seal_utility::stream_size(relinkey);
        splaindata.push(c2s_param);
        
        auto sz = (splaindata.stream_size() 
                   + c2s_param.enc_params_stream_sz
                   + c2s_param.pubkey_stream_sz
                   + c2s_param.relinkey_stream_sz);
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save(stream);

#if 1
        ppcnn_share::seal_utility::write_to_binary_stream(stream, sbuffstream.data(), enc_params_);
#else
        // Save encryption parameter using dummy binary stream.
        // ** seal::EncryptionParameters::save() requires a binary stream. **
        {
            std::ostringstream oss(std::istringstream::binary);
            auto saved_sz = enc_params_.save(oss);
            STDSC_LOG_DEBUG("Saved %lu bytes to stream.", saved_sz);

            auto* p = static_cast<uint8_t*>(sbuffstream.data()) + stream.tellp();
            std::memcpy(p, oss.str().data(), c2s_param.enc_params_stream_sz);
            stream.seekp(saved_sz, std::ios_base::cur);
        }
#endif

#if 1
        ppcnn_share::seal_utility::write_to_binary_stream(stream, sbuffstream.data(), pubkey);
#else
        {
            std::ostringstream oss(std::istringstream::binary);
            auto saved_sz = pubkey.save(oss);
            STDSC_LOG_DEBUG("Saved %lu bytes to stream.", saved_sz);

            auto* p = static_cast<uint8_t*>(sbuffstream.data()) + stream.tellp();
            //std::memcpy(p, oss.str().data(), c2s_param.pubkey_stream_sz);
            std::memcpy(p, oss.str().data(), saved_sz);
            stream.seekp(saved_sz, std::ios_base::cur);
        }
#endif

        ppcnn_share::seal_utility::write_to_binary_stream(stream, sbuffstream.data(), relinkey);

        stdsc::Buffer* sbuffer = &sbuffstream;
        client_.send_data_blocking(ppcnn_share::kControlCodeDataEncKeys, *sbuffer);
    }

    int32_t send_query(const int32_t key_id,
                       const ppcnn_share::ImageInfo& img_info,
                       const ppcnn_share::EncData& enc_inputs)
    {
        ppcnn_share::PlainData<ppcnn_share::C2SQueryParam> splaindata;
        ppcnn_share::C2SQueryParam c2s_param;
        c2s_param.img_info = img_info;
        c2s_param.key_id   = key_id;
        c2s_param.enc_inputs_stream_sz = enc_inputs.stream_size();
        splaindata.push(c2s_param);

        auto sz = splaindata.stream_size() + c2s_param.enc_inputs_stream_sz;
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save(stream);

        printf("===hoge: enc_inptus_stream_sz: %lu\n", c2s_param.enc_inputs_stream_sz);
        // Save encryption inputs using dummy binary stream.
        // ** seal::Ciphertext::save() requires a binary stream. **
        {
            std::ostringstream oss(std::istringstream::binary);
            enc_inputs.save(oss);

            auto* p = static_cast<uint8_t*>(sbuffstream.data()) + stream.tellp();
            std::memcpy(p, oss.str().data(), c2s_param.enc_inputs_stream_sz);
            stream.seekp(enc_inputs.stream_size(), std::ios_base::cur);
        }
        
        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(ppcnn_share::kControlCodeUpDownloadQuery, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);
        ppcnn_share::PlainData<int32_t> rplaindata;
        rplaindata.load(rstream);

        return rplaindata.data();
    }

    void recv_results(const int32_t query_id, bool& status, ppcnn_share::EncData& enc_results)
    {
        int32_t enc_params_stream_sz = ppcnn_share::seal_utility::stream_size(enc_params_);

        ppcnn_share::PlainData<ppcnn_share::C2SResreqParam> splaindata;
        ppcnn_share::C2SResreqParam c2s_param;
        c2s_param.query_id = query_id;
        splaindata.push(c2s_param);
        
        auto sz = splaindata.stream_size();
        stdsc::BufferStream sbuffstream(sz);
        std::iostream stream(&sbuffstream);

        splaindata.save(stream);

        stdsc::Buffer* sbuffer = &sbuffstream;
        stdsc::Buffer rbuffer;
        client_.send_recv_data_blocking(ppcnn_share::kControlCodeUpDownloadResult, *sbuffer, rbuffer);

        stdsc::BufferStream rbuffstream(rbuffer);
        std::iostream rstream(&rbuffstream);

        ppcnn_share::PlainData<ppcnn_share::Srv2CliParam> rplaindata;
        rplaindata.load(rstream);
        auto& s2c_param = rplaindata.data();
        status = s2c_param.result == ppcnn_share::kServerCalcResultSuccess;

        if (status) {
//            auto* p = static_cast<uint8_t*>(rbuffstream.data()) + rstream.tellg();
//            std::string s(p, p + s2c_param.enc_results_stream_sz);
//            std::istringstream iss(s, std::istringstream::binary);
//            enc_results.load(iss);
//
//            // update stream position
//            rstream.seekg(s2c_param.enc_results_stream_sz, std::ios_base::cur);
//            
//            //enc_result.load(rstream);
//#if defined ENABLE_LOCAL_DEBUG
//            ppcnn_share::seal_utility::write_to_file("result.txt", enc_results.data());
//#endif
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

void Client::register_enckeys(const int32_t key_id,
                              const seal::PublicKey& pubkey,
                              const seal::RelinKeys& relinkey) const
{
    STDSC_LOG_INFO("Regist_Enckeys.");
    pimpl_->register_enckeys(key_id, pubkey, relinkey);
}

int32_t Client::send_query(const int32_t key_id,
                           const ppcnn_share::ImageInfo& img_info, 
                           const ppcnn_share::EncData& enc_inputs) const
{
    STDSC_LOG_INFO("Send query: sending query to computation server.");
    auto query_id = pimpl_->send_query(key_id, img_info, enc_inputs);
    STDSC_LOG_INFO("Send query: received query ID (#%d)", query_id);
    return query_id;
}

int32_t Client::send_query(const int32_t key_id,
                           const ppcnn_share::ImageInfo& img_info,
                           const ppcnn_share::EncData& enc_inputs,
                           cbfunc_t cbfunc,
                           void* cbfunc_args) const
{
    int32_t query_id = pimpl_->send_query(key_id, img_info, enc_inputs);
    STDSC_LOG_INFO("Set callback function for query #%d", query_id);
    set_callback(query_id, cbfunc, cbfunc_args);
    return query_id;
}

void Client::recv_results(const int32_t query_id, bool& status, ppcnn_share::EncData& enc_results) const
{
    STDSC_LOG_INFO("Waiting for query #%d results ...", query_id);
    pimpl_->recv_results(query_id, status, enc_results);
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
