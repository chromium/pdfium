// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_
#define CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_

#include "build/build_config.h"
#include "core/fpdfapi/font/cpdf_simplefont.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_fontmapper.h"

class CPDF_Type1Font final : public CPDF_SimpleFont {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ~CPDF_Type1Font() override;

  // CPDF_Font:
  bool IsType1Font() const override;
  const CPDF_Type1Font* AsType1Font() const override;
  CPDF_Type1Font* AsType1Font() override;
#if defined(OS_MACOSX)
  int GlyphFromCharCodeExt(uint32_t charcode) override;
#endif

  bool IsBase14Font() const { return m_Base14Font.has_value(); }

 private:
  CPDF_Type1Font(CPDF_Document* pDocument, CPDF_Dictionary* pFontDict);

  // CPDF_Font:
  bool Load() override;

  // CPDF_SimpleFont:
  void LoadGlyphMap() override;

  bool IsSymbolicFont() const;
  bool IsFixedFont() const;

#if defined(OS_MACOSX)
  void SetExtGID(const char* name, uint32_t charcode);
  void CalcExtGID(uint32_t charcode);

  uint16_t m_ExtGID[256];
#endif

  Optional<CFX_FontMapper::StandardFont> m_Base14Font;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TYPE1FONT_H_
