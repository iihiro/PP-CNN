#ifndef __TYPES_H__
#define __TYPES_H__

#include <boost/multi_array.hpp>
#include <seal/seal.h>

using float2D      = boost::multi_array<float, 2>;
using float4D      = boost::multi_array<float, 4>;
using Plaintext2D  = boost::multi_array<seal::Plaintext, 2>;
using Plaintext3D  = boost::multi_array<seal::Plaintext, 3>;
using Plaintext4D  = boost::multi_array<seal::Plaintext, 4>;
using Ciphertext2D = boost::multi_array<seal::Ciphertext, 2>;
using Ciphertext3D = boost::multi_array<seal::Ciphertext, 3>;
using Ciphertext4D = boost::multi_array<seal::Ciphertext, 4>;

enum EOptLevel {
    NO_OPT         = 0,
    FUSE_LAYERS    = 1,
    OPT_ACTIVATION = 2,
    OPT_POOLING    = 3,
    ALL_OPT        = 4,
};

enum EActivation {
    DEFAULT        = 0,
    SQUARE         = 1,
    SWISH_RG4_DEG4 = 2,
    SWISH_RG6_DEG4 = 3,
    MISH_RG4_DEG4  = 4,
    MISH_RG6_DEG4  = 5,
};

enum ELayerClass {
    CONV2D,
    AVERAGE_POOLING2D,
    ACTIVATION,
    BATCH_NORMALIZATION,
    DENSE,
    FLATTEN,
    GLOBAL_AVERAGE_POOLING2D
};

#endif/*__TYPES_H__*/
