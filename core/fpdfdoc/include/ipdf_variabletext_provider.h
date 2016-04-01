// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_INCLUDE_IPDF_VARIABLETEXT_PROVIDER_H_
#define CORE_FPDFDOC_INCLUDE_IPDF_VARIABLETEXT_PROVIDER_H_

#include "core/fxcrt/include/fx_system.h"

class IPDF_VariableText_Provider {
 public:
  virtual ~IPDF_VariableText_Provider();

  virtual int32_t GetCharWidth(int32_t nFontIndex,
                               uint16_t word,
                               int32_t nWordStyle) = 0;
  virtual int32_t GetTypeAscent(int32_t nFontIndex) = 0;
  virtual int32_t GetTypeDescent(int32_t nFontIndex) = 0;
  virtual int32_t GetWordFontIndex(uint16_t word,
                                   int32_t charset,
                                   int32_t nFontIndex) = 0;
  virtual int32_t GetDefaultFontIndex() = 0;
  virtual FX_BOOL IsLatinWord(uint16_t word) = 0;
};

#endif  // CORE_FPDFDOC_INCLUDE_IPDF_VARIABLETEXT_PROVIDER_H_
