// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_TYPE3CHAR_H_
#define CORE_FPDFAPI_FONT_CPDF_TYPE3CHAR_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_DIBitmap;
class CPDF_ImageObject;

class CPDF_Type3Char {
 public:
  class FormIface {
   public:
    virtual ~FormIface() {}

    virtual void ParseContentForType3Char(CPDF_Type3Char* pChar) = 0;
    virtual bool HasPageObjects() const = 0;
    virtual const CPDF_ImageObject* GetSoleImageOfForm() const = 0;
    virtual CFX_FloatRect CalcBoundingBox() const = 0;
  };

  CPDF_Type3Char();
  ~CPDF_Type3Char();

  static float TextUnitToGlyphUnit(float fTextUnit);
  static void TextUnitRectToGlyphUnitRect(CFX_FloatRect* pRect);

  bool LoadBitmapFromSoleImageOfForm();
  void InitializeFromStreamData(bool bColored, const float* pData);
  void Transform(FormIface* pForm, const CFX_Matrix& matrix);

  RetainPtr<CFX_DIBitmap> GetBitmap();
  const RetainPtr<CFX_DIBitmap>& GetBitmap() const;

  bool colored() const { return m_bColored; }
  uint32_t width() const { return m_Width; }
  const CFX_Matrix& matrix() const { return m_ImageMatrix; }
  const FX_RECT& bbox() const { return m_BBox; }

  const FormIface* form() const { return m_pForm.get(); }
  void SetForm(std::unique_ptr<FormIface> pForm);

 private:
  std::unique_ptr<FormIface> m_pForm;
  RetainPtr<CFX_DIBitmap> m_pBitmap;
  bool m_bColored = false;
  uint32_t m_Width = 0;
  CFX_Matrix m_ImageMatrix;
  FX_RECT m_BBox;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TYPE3CHAR_H_
