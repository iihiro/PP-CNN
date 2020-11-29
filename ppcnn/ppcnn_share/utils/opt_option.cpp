#include <ppcnn_share/utils/define.h>
#include <ppcnn_share/utils/opt_option.hpp>

OptOption::OptOption(const std::shared_ptr<seal::SEALContext>& context,
                     const std::shared_ptr<seal::PublicKey>& pubkey,
                     const std::shared_ptr<seal::RelinKeys>& relinkey,
                     const EOptLevel opt_level,
                     const EActivation act)
    : enable_fuse_layers(false), 
      enable_optimize_activation(false),
      enable_optimize_pooling(false),
      should_multiply_coeff(false),
      should_multiply_pool(false),
      activation(act),
      highest_deg_coeff(0.0f),
      current_pooling_mul_factor(0.0f),
      consumed_level(0),
      relin_keys(relinkey),
      encryptor(new seal::Encryptor(context, *pubkey)),
      evaluator(new seal::Evaluator(context)),
      encoder(new seal::CKKSEncoder(context))
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

    size_t slot_count  = encoder->slot_count();
    double scale_param = pow(2.0, INTERMEDIATE_PRIMES_BIT_SIZE);
}
