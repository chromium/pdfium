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
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Form;
class CPDF_RenderContext;
class CPDF_Stream;

class CPDF_Type3Char {
 public:
  CPDF_Type3Char(CPDF_Document* pDocument,
                 CPDF_Dictionary* pPageResources,
                 CPDF_Stream* pFormStream);
  ~CPDF_Type3Char();

  static float TextUnitToGlyphUnit(float fTextUnit);
  static void TextUnitRectToGlyphUnitRect(CFX_FloatRect* pRect);

  bool LoadBitmap(CPDF_RenderContext* pContext);
  void InitializeFromStreamData(bool bColored, const float* pData);
  void Transform(const CFX_Matrix& matrix);
  void ResetForm();
  void ParseContent();
  bool HasPageObjects() const;

  RetainPtr<CFX_DIBitmap> GetBitmap();
  const RetainPtr<CFX_DIBitmap>& GetBitmap() const;

  bool colored() const { return m_bColored; }
  uint32_t width() const { return m_Width; }
  const CFX_Matrix& matrix() const { return m_ImageMatrix; }
  const FX_RECT& bbox() const { return m_BBox; }

 private:
  friend class CPDF_RenderStatus;

  const CPDF_Form* form() const { return m_pForm.get(); }
  CPDF_Form* form() { return m_pForm.get(); }

  std::unique_ptr<CPDF_Form> m_pForm;
  RetainPtr<CFX_DIBitmap> m_pBitmap;
  bool m_bColored = false;
  uint32_t m_Width = 0;
  CFX_Matrix m_ImageMatrix;
  FX_RECT m_BBox;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TYPE3CHAR_H_
