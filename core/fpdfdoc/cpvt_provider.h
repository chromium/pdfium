// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPVT_PROVIDER_H_
#define CORE_FPDFDOC_CPVT_PROVIDER_H_

#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/include/fpdfdoc/fpdf_vt.h"

class CPVT_Provider : public IPDF_VariableText::Provider {
 public:
  CPVT_Provider(IPVT_FontMap* pFontMap);
  ~CPVT_Provider() override;

  // IPDF_VariableText::Provider
  int32_t GetCharWidth(int32_t nFontIndex,
                       uint16_t word,
                       int32_t nWordStyle) override;
  int32_t GetTypeAscent(int32_t nFontIndex) override;
  int32_t GetTypeDescent(int32_t nFontIndex) override;
  int32_t GetWordFontIndex(uint16_t word,
                           int32_t charset,
                           int32_t nFontIndex) override;
  FX_BOOL IsLatinWord(uint16_t word) override;
  int32_t GetDefaultFontIndex() override;

 private:
  IPVT_FontMap* m_pFontMap;
};

#endif  // CORE_FPDFDOC_CPVT_PROVIDER_H_
