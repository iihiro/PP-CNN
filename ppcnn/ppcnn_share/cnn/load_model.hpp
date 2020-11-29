#pragma once

#include "layer.hpp"
#include "picojson.h"

picojson::array 
loadLayers(const string& model_structure_path);

Layer* 
buildLayer(picojson::object& layer, 
           const string& layer_class_name, 
           const string& model_weights_path,
           OptOption& option);

Layer* 
buildLayer(picojson::object& layer, 
           picojson::object& next_layer, 
           const string& layer_class_name, 
           const string& model_weight_path, 
           picojson::array::const_iterator& layers_itrator,
           OptOption& option);

Layer* 
buildConv2D(picojson::object& layer_info, 
            const string& model_weights_path,
            OptOption& option);

Layer* 
buildAveragePooling2D(picojson::object& layer_info, 
                      const string& model_weights_path,
                      OptOption& option);

Layer* 
buildBatchNormalization(picojson::object& layer_info, 
                        const string& model_weights_path,
                        OptOption& option);

Layer* 
buildFlatten(picojson::object& layer_info,
             const string& model_weights_path,
             OptOption& option);

Layer* 
buildDense(picojson::object& layer_info, 
           const string& model_weights_path,
           OptOption& option);

Layer* 
buildActivation(picojson::object& layer_info, 
                const string& model_weights_path,
                OptOption& option);

Layer* 
buildGlobalAveragePooling2D(picojson::object& layer_info, 
                            const string& model_weights_path,
                            OptOption& option);

Layer* 
buildConv2DFusedBN(picojson::object& conv2d_layer_info, 
                   picojson::object& bn_layer_info, 
                   const string& model_weights_path,
                   OptOption& option);

Layer* 
buildDenseFusedBN(picojson::object& dense_layer_info, 
                  picojson::object& bn_layer_info, 
                  const string& model_weights_path,
                  OptOption& option);
