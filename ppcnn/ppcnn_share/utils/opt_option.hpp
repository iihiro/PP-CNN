#pragma once

#include <memory>
#include <unistd.h>
#include <ppcnn_share/utils/types.h>
#include <seal/seal.h>

struct OptOption 
{
    OptOption(const std::shared_ptr<seal::SEALContext>& context,
              const std::shared_ptr<seal::PublicKey>& pubkey,
              const std::shared_ptr<seal::RelinKeys>& relinkey,
              const EOptLevel opt_level,
              const EActivation act);
    ~OptOption() = default;

    bool enable_fuse_layers;
    bool enable_optimize_activation;
    bool enable_optimize_pooling;

    bool should_multiply_coeff;
    bool should_multiply_pool;

    EActivation activation;

    float highest_deg_coeff;
    float current_pooling_mul_factor;

    size_t consumed_level;

    std::shared_ptr<seal::RelinKeys> relin_keys;
    std::shared_ptr<seal::Encryptor> encryptor;
    std::shared_ptr<seal::Evaluator> evaluator;
    std::shared_ptr<seal::CKKSEncoder> encoder;
    size_t slot_count;
    double scale_param;
};
