// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
#define CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_

#include "core/fpdfdoc/cpdf_defaultappearance.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"

enum class BorderStyle { SOLID, DASH, BEVELED, INSET, UNDERLINE };
enum class PaintOperation { STROKE, FILL };

class CPDF_DefaultAppearance {
 public:
  CPDF_DefaultAppearance() {}
  explicit CPDF_DefaultAppearance(const ByteString& csDA) : m_csDA(csDA) {}

  CPDF_DefaultAppearance(const CPDF_DefaultAppearance& cDA) {
    m_csDA = cDA.GetStr();
  }

  ByteString GetStr() const { return m_csDA; }

  bool HasFont();
  ByteString GetFontString();
  ByteString GetFont(float* fFontSize);

  bool HasColor(PaintOperation nOperation = PaintOperation::FILL);
  ByteString GetColorString(PaintOperation nOperation = PaintOperation::FILL);
  void GetColor(int& iColorType,
                float fc[4],
                PaintOperation nOperation = PaintOperation::FILL);
  void GetColor(FX_ARGB& color,
                int& iColorType,
                PaintOperation nOperation = PaintOperation::FILL);

  bool HasTextMatrix();
  ByteString GetTextMatrixString();
  CFX_Matrix GetTextMatrix();

 private:
  ByteString m_csDA;
};

#endif  // CORE_FPDFDOC_CPDF_DEFAULTAPPEARANCE_H_
