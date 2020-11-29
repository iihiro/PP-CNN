#include <cmath>
#include <fstream>
#include <memory>

#include <ppcnn_share/utils/define.h>
#include <ppcnn_share/utils/helper_functions.hpp>

using namespace seal;
using std::cout;
using std::endl;
using std::fill;
using std::ifstream;
using std::pow;
using std::shared_ptr;
using std::size_t;

const string SECRETS_DIR      = "../secrets/";
const string PARAMS_FILE_PATH = SECRETS_DIR + "params.bin";
const string PK_FILE_PATH     = SECRETS_DIR + "pk.bin";
const string SK_FILE_PATH     = SECRETS_DIR + "sk.bin";
const string RK_FILE_PATH     = SECRETS_DIR + "rk.bin";
const string GK_FILE_PATH     = SECRETS_DIR + "gk.bin";

inline void loadParams(EncryptionParameters& params) {
  ifstream params_ifs(PARAMS_FILE_PATH);
  params.load(params_ifs);
  params_ifs.close();
}
inline void loadPublicKey(PublicKey& public_key, shared_ptr<SEALContext>& context) {
  ifstream pk_ifs(PK_FILE_PATH);
  public_key.load(context, pk_ifs);
  pk_ifs.close();
}
inline void loadSecretKey(SecretKey& secret_key, shared_ptr<SEALContext>& context) {
  ifstream sk_ifs(SK_FILE_PATH);
  secret_key.load(context, sk_ifs);
  sk_ifs.close();
}
inline void loadRelinKeys(RelinKeys& relin_keys, shared_ptr<SEALContext>& context) {
  ifstream rk_ifs(RK_FILE_PATH);
  relin_keys.load(context, rk_ifs);
  rk_ifs.close();
}

#if 0
/* Setup Homomorphic Encryption tool using SEAL */
void setupSealTool() {
  EncryptionParameters params(scheme_type::CKKS);
  PublicKey public_key;
  SecretKey secret_key;
  RelinKeys* relin_keys = new RelinKeys();

  cout << "Loading secrets from " << SECRETS_DIR << endl;

  loadParams(params);
  auto context = SEALContext::Create(params);
  loadPublicKey(public_key, context);
  loadSecretKey(secret_key, context);
  loadRelinKeys(*relin_keys, context);

  cout << "Finish loading!\n"
       << endl;

  // Create encryptor, decryptor, evaluator, encoder
  Encryptor* encryptor = new Encryptor(context, public_key);
  Decryptor* decryptor = new Decryptor(context, secret_key);
  Evaluator* evaluator = new Evaluator(context);
  CKKSEncoder* encoder = new CKKSEncoder(context);
  size_t slot_count    = encoder->slot_count();
  double scale_param   = pow(2.0, INTERMEDIATE_PRIMES_BIT_SIZE);

  gTool.setRelinKeys(relin_keys);
  gTool.setEncryptor(encryptor);
  gTool.setDecryptor(decryptor);
  gTool.setEvaluator(evaluator);
  gTool.setEncoder(encoder);
  gTool.setSlotCount(slot_count);
  gTool.setScaleParam(scale_param);
}
#endif

#if 0
/* Setup optimization settings */
void setupOptimizationOption(const EOptLevel& optimization_level) {
  switch (optimization_level) {
    case NO_OPT:
      break;
    case FUSE_LAYERS:
      gOption.setEnableFuseLayers(true);
      break;
    case OPT_ACTIVATION:
      gOption.setEnableOptimizeActivation(true);
      break;
    case OPT_POOLING:
      gOption.setEnableOptimizePooling(true);
      break;
    case ALL_OPT:
      gOption.setEnableFuseLayers(true);
      gOption.setEnableOptimizeActivation(true);
      gOption.setEnableOptimizePooling(true);
      break;
  }
}
#endif

/* Encrypt single image using SEAL encryptor */
void encryptImage(const vector<float>& origin_image, 
                  Ciphertext3D& target_image,
                  const double scale_param,
                  seal::Encryptor& encryptor,
                  seal::CKKSEncoder& encoder)
{
    const size_t rows               = target_image.shape()[0];
    const size_t cols               = target_image.shape()[1];
    const size_t channels           = target_image.shape()[2];
    const size_t pixels_per_channel = rows * cols;
    
    for (size_t ch = 0; ch < channels; ++ch) {
        for (size_t row = 0; row < rows; ++row) {
            for (size_t col = 0; col < cols; ++col) {
                Plaintext plaintext_pixel;
                encoder.encode(origin_image[ch * pixels_per_channel + row * cols + col], scale_param, plaintext_pixel);
                encryptor.encrypt(plaintext_pixel, target_image[row][col][ch]);
            }
        }
    }
}

/* Encrypt images packed by slots using SEAL encryptor */
void encryptImages(const vector<vector<float>>& origin_images, 
                   Ciphertext3D& target_packed_images, 
                   const size_t& begin_idx, 
                   const size_t& end_idx,
                   const double scale_param,
                   seal::Encryptor& encryptor,
                   seal::CKKSEncoder& encoder) {
    const size_t slot_count         = encoder.slot_count();
    const size_t rows               = target_packed_images.shape()[0];
    const size_t cols               = target_packed_images.shape()[1];
    const size_t channels           = target_packed_images.shape()[2];
    const size_t pixels_per_channel = rows * cols;
    vector<double> pixels_in_slots(slot_count, 0);
    Plaintext plaintext_packed_pixels;

    if (size_t target_images_count = end_idx - begin_idx; target_images_count < slot_count) {  // fill slots if size of target images < slot_count
        size_t origin_images_size = origin_images.size();
        size_t fill_images_count  = slot_count - target_images_count;
        for (size_t ch = 0; ch < channels; ++ch) {
            for (size_t row = 0; row < rows; ++row) {
                for (size_t col = 0; col < cols; ++col) {
                    for (size_t idx = begin_idx, counter = 0, pos = ch * pixels_per_channel + row * cols + col; idx < end_idx + fill_images_count; ++idx) {
                        pixels_in_slots[counter++] = origin_images[idx % origin_images_size][pos];
                    }
                    encoder.encode(pixels_in_slots, scale_param, plaintext_packed_pixels);
                    encryptor.encrypt(plaintext_packed_pixels, target_packed_images[row][col][ch]);
                    fill(pixels_in_slots.begin(), pixels_in_slots.end(), 0);
                }
            }
        }
    } else {
        for (size_t ch = 0; ch < channels; ++ch) {
            for (size_t row = 0; row < rows; ++row) {
                for (size_t col = 0; col < cols; ++col) {
                    for (size_t idx = begin_idx, counter = 0, pos = ch * pixels_per_channel + row * cols + col; idx < end_idx; ++idx) {
                        pixels_in_slots[counter++] = origin_images[idx][pos];
                    }
                    encoder.encode(pixels_in_slots, scale_param, plaintext_packed_pixels);
                    encryptor.encrypt(plaintext_packed_pixels, target_packed_images[row][col][ch]);
                    fill(pixels_in_slots.begin(), pixels_in_slots.end(), 0);
                }
            }
        }
    }
}
