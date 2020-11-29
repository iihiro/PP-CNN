#pragma once

#include <memory>
#include <unistd.h>
#include <ppcnn_share/utils/types.h>
#include <seal/seal.h>

struct OptOption 
{
    OptOption(const EOptLevel opt_level,
              const EActivation act,
              seal::RelinKeys& relinkey,
              seal::Evaluator& evaluator,
              seal::CKKSEncoder& encoder);
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

    seal::RelinKeys& relin_keys;
    seal::Evaluator& evaluator;
    seal::CKKSEncoder& encoder;
    size_t slot_count;
    double scale_param;
};

