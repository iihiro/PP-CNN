/*
 * Copyright 2020 Yamana Laboratory, Waseda University
 * Supported by JST CREST Grant Number JPMJCR1503, Japan.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE‐2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "network.hpp"

using std::string;

#if 0
class NetworkBuilder final {
public:
  static void setup(const string& model_structure_path, const string& model_weights_path) {
    model_structure_path_ = model_structure_path;
    model_weights_path_   = model_weights_path;
  }
  static Network buildNetwork() noexcept(false);

private:
  inline static string model_structure_path_;
  inline static string model_weights_path_;
  NetworkBuilder();
  ~NetworkBuilder();
};

#endif
