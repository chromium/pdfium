// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
#define CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_

#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"

enum class BorderStyle { SOLID, DASH, BEVELED, INSET, UNDERLINE };

class CPDF_DefaultAppearance {
 public:
  CPDF_DefaultAppearance() {}
  explicit CPDF_DefaultAppearance(const ByteString& csDA) : m_csDA(csDA) {}
  CPDF_DefaultAppearance(const CPDF_DefaultAppearance& cDA)
      : m_csDA(cDA.m_csDA) {}

  bool HasFont();
  ByteString GetFont(float* fFontSize);

  bool HasColor();
  void GetColor(int& iColorType, float fc[4]);
  void GetColor(FX_ARGB& color, int& iColorType);

  bool FindTagParamFromStartForTesting(CPDF_SimpleParser* parser,
                                       const ByteStringView& token,
                                       int nParams);

 private:
  ByteString m_csDA;
};

#endif  // CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
