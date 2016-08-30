// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PAGE_CPDF_TEXTSTATE_H_
#define CORE_FPDFAPI_FPDF_PAGE_CPDF_TEXTSTATE_H_

#include "core/fpdfapi/fpdf_page/include/cpdf_textstatedata.h"
#include "core/fxcrt/include/cfx_count_ref.h"
#include "core/fxcrt/include/fx_basic.h"

class CPDF_Font;

class CPDF_TextState {
 public:
  CPDF_TextState();
  ~CPDF_TextState();

  void Emplace();

  CPDF_Font* GetFont() const;
  void SetFont(CPDF_Font* pFont);

  FX_FLOAT GetFontSize() const;
  void SetFontSize(FX_FLOAT size);

  const FX_FLOAT* GetMatrix() const;
  FX_FLOAT* GetMutableMatrix();

  FX_FLOAT GetCharSpace() const;
  void SetCharSpace(FX_FLOAT sp);

  FX_FLOAT GetWordSpace() const;
  void SetWordSpace(FX_FLOAT sp);

  FX_FLOAT GetFontSizeV() const;
  FX_FLOAT GetFontSizeH() const;
  FX_FLOAT GetBaselineAngle() const;
  FX_FLOAT GetShearAngle() const;

  TextRenderingMode GetTextMode() const;
  void SetTextMode(TextRenderingMode mode);

  const FX_FLOAT* GetCTM() const;
  FX_FLOAT* GetMutableCTM();

 private:
  CFX_CountRef<CPDF_TextStateData> m_Ref;
};

#endif  // CORE_FPDFAPI_FPDF_PAGE_CPDF_TEXTSTATE_H_
