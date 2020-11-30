#pragma once

#include <vector>

using std::string;
using std::vector;

extern const string SECRETS_DIR;
extern const string PARAMS_FILE_PATH;
extern const string PK_FILE_PATH;
extern const string SK_FILE_PATH;
extern const string RK_FILE_PATH;
extern const string GK_FILE_PATH;

void setupSealTool();

void encryptImage(const vector<float>& origin_image, 
                  Ciphertext3D& target_image,
                  const double scale_param,
                  seal::Encryptor& encryptor,
                  seal::CKKSEncoder& encoder);
                  
void encryptImages(const vector<vector<float>>& origin_images, 
                   Ciphertext3D& target_packed_images, 
                   const size_t& begin_idx, 
                   const size_t& end_idx,
                   const double scale_param,
                   seal::Encryptor& encryptor,
                   seal::CKKSEncoder& encoder);
