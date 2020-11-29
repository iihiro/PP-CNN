#include <unistd.h>
#include <algorithm> // for sort
#include <chrono>
#include <random>
#include <fstream>
#include <sys/types.h>   // for thread id
#include <sys/syscall.h> // for thread id
#include <stdsc/stdsc_log.hpp>
#include <omp.h>
#include <ppcnn_share/ppcnn_utility.hpp>
#include <ppcnn_share/utils/globals.hpp>
#include <ppcnn_share/utils/types.h>
#include <ppcnn_share/utils/define.h>
#include <ppcnn_share/cnn/network_builder.hpp>
#include <ppcnn_share/cnn/picojson.h>
#include <ppcnn_share/cnn/load_model.hpp>
#include <ppcnn_server/ppcnn_server_query.hpp>
#include <ppcnn_server/ppcnn_server_result.hpp>
#include <ppcnn_server/ppcnn_server_calcthread.hpp>
#include <ppcnn_server/ppcnn_server_keycontainer.hpp>
#include <seal/seal.h>

namespace ppcnn_server
{

static Network 
BuildNetwork(const std::string& model_structure_path,
             const std::string& model_weights_path,
             OptOption& option)
{
    printf("1\n");
    Network network;
    //picojson::array layers = loadLayers(model_structure_path_);
    picojson::array layers = loadLayers(model_structure_path);

    printf("2\n");
    //if (gOption.enable_fuse_layers()) {
    if (option.enable_fuse_layers) {
        for (picojson::array::const_iterator it = layers.cbegin(), layers_end = layers.cend(); it != layers_end; ++it) {
            picojson::object layer        = (*it).get<picojson::object>();
            const string layer_class_name = layer["class_name"].get<string>();

            if (it + 1 != layers_end) {
                picojson::object next_layer = (*(it + 1)).get<picojson::object>();
                //network.addLayer(buildLayer(layer, next_layer, layer_class_name, model_weights_path_, it));
                network.addLayer(buildLayer(layer, next_layer, layer_class_name, model_weights_path, it, option));
            } else {
                network.addLayer(buildLayer(layer, layer_class_name, model_weights_path, option));
            }
        }
        return network;
    }

    printf("3\n");
    size_t dbg_cnt=0;
    for (picojson::array::const_iterator it = layers.cbegin(), layers_end = layers.cend(); it != layers_end; ++it) {
        printf("4:%lu\n", dbg_cnt);
        picojson::object layer        = (*it).get<picojson::object>();
        printf("5:%lu\n", dbg_cnt);
        const string layer_class_name = layer["class_name"].get<string>();
        printf("6t:%lu\n", dbg_cnt);
        network.addLayer(buildLayer(layer, layer_class_name, model_weights_path, option));
        printf("7:%lu\n", dbg_cnt);
        dbg_cnt++;
    }
    printf("4\n");
    return network;
}

#define LOGINFO(fmt, ...) STDSC_LOG_INFO("[th:%d,query:%d] " fmt , th_id, query_id, ##__VA_ARGS__)

struct CalcThread::Impl
{
    Impl(QueryQueue& in_queue,
         ResultQueue& out_queue)
        : in_queue_(in_queue),
          out_queue_(out_queue)
    {
    }

    void exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        auto th_id = syscall(SYS_gettid);
        STDSC_LOG_INFO("Launched calcuration thread. (thread ID: %d)", th_id);
        
        while (!args.force_finish) {

            STDSC_LOG_INFO("[th:%d] Try getting query from QueryQueue.", th_id);

            int32_t query_id;
            Query query;
            while (!in_queue_.pop(query_id, query)) {
                usleep(args.retry_interval_msec * 1000);
            }

            const auto dataset_name = std::string(query.params_.dataset);
            const auto model_name  = std::string(query.params_.model);
            const std::string base_model_path      = args.plaintext_experiment_path + dataset_name + "/saved_models/" + model_name;
            const std::string model_structure_path = base_model_path + "_structure.json";
            const std::string model_weights_path   = base_model_path + "_weights.h5";

            if (!ppcnn_share::utility::file_exist(model_structure_path)) {
                std::ostringstream oss;
                oss << "File not fount. (" << model_structure_path << ")";
                STDSC_THROW_FILE(oss.str());
            }
            if (!ppcnn_share::utility::file_exist(model_weights_path)) {
                std::ostringstream oss;
                oss << "File not fount. (" << model_weights_path << ")";
                STDSC_THROW_FILE(oss.str());
            }
            
            bool status = compute(th_id, query_id,
                                  query.params_, 
                                  *(query.enc_keys_p_), 
                                  model_structure_path, 
                                  model_weights_path);
            
            LOGINFO("Get query. (%s)\n", query.params_.to_string().c_str());

            std::vector<seal::Ciphertext> dummy_results;
            Result result(query.key_id_, query_id, status, dummy_results);
            out_queue_.push(query_id, result);

            LOGINFO("Set result of query.\n");
        }
    }

    bool compute(const int32_t th_id,
                 const int32_t query_id,
                 const ppcnn_share::ComputationParams& params,
                 const EncryptionKeys& enc_keys,
                 const std::string& model_structure_path,
                 const std::string& model_weights_path)
    {
        LOGINFO("Start computation.\n");
        bool res = true;
        auto context = seal::SEALContext::Create(*(enc_keys.params));
        
        auto& pubkey     = *(enc_keys.pubkey);
        auto& relin_keys = *(enc_keys.relinkey);
        std::shared_ptr<seal::Evaluator> evaluator(new seal::Evaluator(context));
        std::shared_ptr<seal::CKKSEncoder> encoder(new seal::CKKSEncoder(context));
        
        auto opt_level  = static_cast<EOptLevel>(params.opt_level); 
        auto activation = static_cast<EActivation>(params.activation);
        OptOption option(opt_level, activation, relin_keys, *evaluator, *encoder);

        printf("*2\n");
        auto trained_model_name = std::string(params.model);
        if (option.enable_optimize_activation) {
            if (trained_model_name.find("CKKS-swish_rg4_deg4") != std::string::npos || activation == SWISH_RG4_DEG4) {
                option.highest_deg_coeff = SWISH_RG4_DEG4_COEFFS.front();
            } else if (trained_model_name.find("CKKS-swish_rg6_deg4") != string::npos || activation == SWISH_RG6_DEG4) {
                option.highest_deg_coeff =SWISH_RG6_DEG4_COEFFS.front();
            }
        }
            
        printf("*3\n");
        LOGINFO("Buiding network from trained model...\n");
        BuildNetwork(model_structure_path, model_weights_path, option);
        STDSC_LOG_INFO("Finish buiding.\n");


        return res;
    }

    QueryQueue& in_queue_;
    ResultQueue& out_queue_;
    CalcThreadParam param_;
    std::shared_ptr<stdsc::ThreadException> te_;
};

CalcThread::CalcThread(QueryQueue& in_queue,
                       ResultQueue& out_queue)
    : pimpl_(new Impl(in_queue, out_queue))
{}

void CalcThread::start()
{
    pimpl_->param_.force_finish = false;
    super::start(pimpl_->param_, pimpl_->te_);
}

void CalcThread::stop()
{
    STDSC_LOG_INFO("Stop calculation thread.");
    pimpl_->param_.force_finish = true;
}

void CalcThread::exec(CalcThreadParam& args, std::shared_ptr<stdsc::ThreadException> te) const
{
    pimpl_->exec(args, te);
}

} /* namespace ppcnn_server */
