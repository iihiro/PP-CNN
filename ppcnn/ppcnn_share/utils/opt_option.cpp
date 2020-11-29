#include <ppcnn_share/utils/define.h>
#include <ppcnn_share/utils/opt_option.hpp>

OptOption::OptOption(const EOptLevel opt_level,
                     const EActivation act,
                     seal::RelinKeys& _relin_keys,
                     seal::Evaluator& _evaluator,
                     seal::CKKSEncoder& _encoder)
    : enable_fuse_layers(false), 
      enable_optimize_activation(false),
      enable_optimize_pooling(false),
      should_multiply_coeff(false),
      should_multiply_pool(false),
      activation(act),
      highest_deg_coeff(0.0f),
      current_pooling_mul_factor(0.0f),
      consumed_level(0),
      relin_keys(_relin_keys),
      evaluator(_evaluator),
      encoder(_encoder)
{
    switch (opt_level) {
    case FUSE_LAYERS:
        enable_fuse_layers = true;
        break;
    case OPT_ACTIVATION:
        enable_optimize_activation = true;
        break;
    case OPT_POOLING:
        enable_optimize_pooling = true;
        break;
    case ALL_OPT:
        enable_fuse_layers = true;
        enable_optimize_activation = true;
        enable_optimize_pooling = true;
        break;
    case NO_OPT:
    default:
        break;
    }

    slot_count  = encoder.slot_count();
    scale_param = pow(2.0, INTERMEDIATE_PRIMES_BIT_SIZE);
}
