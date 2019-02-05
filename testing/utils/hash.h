// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_UTILS_HASH_H_
#define TESTING_UTILS_HASH_H_

#include <string>

std::string CryptToBase16(const uint8_t* digest);
std::string GenerateMD5Base16(const uint8_t* data, uint32_t size);

#endif  // TESTING_UTILS_HASH_H_
