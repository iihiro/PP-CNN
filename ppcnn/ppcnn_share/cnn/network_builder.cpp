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

#include "network_builder.hpp"
#include <stdexcept>
#include "load_model.hpp"
#include "picojson.h"

#if 0
/**
 * Create CNN
 *
 * @return Network built from pre-trained model
 * @throws std::runtime_error
 */
Network NetworkBuilder::buildNetwork() noexcept(false) {
  Network network;
  picojson::array layers = loadLayers(model_structure_path_);

  if (gOption.enable_fuse_layers()) {
    for (picojson::array::const_iterator it = layers.cbegin(), layers_end = layers.cend(); it != layers_end; ++it) {
      picojson::object layer        = (*it).get<picojson::object>();
      const string layer_class_name = layer["class_name"].get<string>();

      if (it + 1 != layers_end) {
        picojson::object next_layer = (*(it + 1)).get<picojson::object>();
        network.addLayer(buildLayer(layer, next_layer, layer_class_name, model_weights_path_, it));
      } else {
        network.addLayer(buildLayer(layer, layer_class_name, model_weights_path_));
      }
    }
    return network;
  }

  for (picojson::array::const_iterator it = layers.cbegin(), layers_end = layers.cend(); it != layers_end; ++it) {
    picojson::object layer        = (*it).get<picojson::object>();
    const string layer_class_name = layer["class_name"].get<string>();
    network.addLayer(buildLayer(layer, layer_class_name, model_weights_path_));
  }
  return network;
}
#endif
