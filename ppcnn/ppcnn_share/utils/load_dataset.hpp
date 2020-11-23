#pragma once

#include <string>
#include <vector>

using std::string;
using std::vector;

extern const string MNIST_DATASET_PATH;
extern const string CIFAR10_DATASET_PATH;

template <template <typename...> class Container, typename Image>
void normalize(Container<Image>& images);

vector<vector<float>> loadMnistTestImages(const std::string& dataset_dir, const size_t& test_limit);
vector<unsigned char> loadMnistTestLabels(const std::string& dataset_dir, const size_t& test_limit);
vector<vector<float>> loadCifar10TestImages(const std::string& dataset_dir, const size_t& test_limit);
vector<unsigned char> loadCifar10TestLabels(const std::string& dataset_dir, const size_t& test_limit);
