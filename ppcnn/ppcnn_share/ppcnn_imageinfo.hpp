/*
 * Copyright 2018 Yamana Laboratory, Waseda University
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

#ifndef PPCNN_IMAGEINFO_HPP
#define PPCNN_IMAGEINFO_HPP

#include <string>
#include <sstream>

namespace ppcnn_share
{

/**
 * @brief This class is used to hold the image info.
 */
struct ImageInfo
{
    size_t width;
    size_t height;
    size_t channels;
    size_t labels;

    std::string to_string() const {
        std::ostringstream oss;
        oss << width    << ", "
            << height   << ", "
            << channels << ", "
            << labels;
        return oss.str();
    }
};

} /* namespace ppcnn_share */

#endif /* PPCNN_IMAGEINFO_HPP */
