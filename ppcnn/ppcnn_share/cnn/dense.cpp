#include "dense.hpp"
#include <omp.h>
#include <fstream>
#include <iostream>

using std::cout;
using std::endl;
using std::move;

Dense::Dense(const string& name,
             const size_t& in_units, const size_t& out_units,
             const string& activation,
             const Plaintext2D& plain_weights, const vector<Plaintext>& plain_biases)
    : Layer(name, DENSE),
      in_units_(in_units),
      out_units_(out_units),
      activation_(activation),
      plain_weights_(plain_weights),
      plain_biases_(plain_biases) {
  gConsumedLevel++;
}
Dense::~Dense() {}

void Dense::printInfo() const {
  cout << DENSE_CLASS_NAME << ": " << name() << endl;
}

void Dense::forward(vector<Ciphertext>& input) const {
  cout << "\tForwarding " << name() << "..." << endl;
  cout << "\t  input size: " << input.size() << endl;
  vector<Ciphertext> output(out_units_);

  Ciphertext weighted_unit;
#ifdef _OPENMP
#pragma omp parallel for private(weighted_unit)
#endif
  for (size_t ou = 0; ou < out_units_; ++ou) {
    for (size_t iu = 0; iu < in_units_; ++iu) {
      gTool.evaluator()->multiply_plain(input[iu], plain_weights_[iu][ou], weighted_unit);
      if (iu == 0) {
        output[ou] = weighted_unit;
      } else {
        gTool.evaluator()->add_inplace(output[ou], weighted_unit);
      }
    }
    gTool.evaluator()->rescale_to_next_inplace(output[ou]);
    output[ou].scale() = gTool.scale_param();
    gTool.evaluator()->add_plain_inplace(output[ou], plain_biases_[ou]);
  }

  input.resize(out_units_);
#ifdef __DEBUG__
  Plaintext plain;
  vector<double> vec_tmp;
  std::ofstream debug_file;
  debug_file.open(DEBUG_FILE_PATH, std::ios::app);
  debug_file << "In " << name() << ":" << endl;
#endif
#ifdef _OPENMP
#pragma omp parallel for
#endif
  for (size_t ou = 0; ou < out_units_; ++ou) {
    input[ou] = move(output[ou]);
#ifdef __DEBUG__
    // if (omp_get_thread_num() == 10) {
    //   gTool.decryptor()->decrypt(input[ou], plain);
    //   gTool.encoder()->decode(plain, vec_tmp);
    //   debug_file << "\toutput[" << ou << "]: " << vec_tmp[0] << ", " << vec_tmp[1] << ", " << vec_tmp[2] << endl;
    // }
    gTool.decryptor()->decrypt(input[ou], plain);
    gTool.encoder()->decode(plain, vec_tmp);
    debug_file << "\toutput[" << ou << "]: " << vec_tmp[0] << ", " << vec_tmp[1] << ", " << vec_tmp[2] << endl;
#endif
  }
}
