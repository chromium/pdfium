// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_TYPE3FONT_H_
#define CORE_FPDFAPI_FONT_CPDF_TYPE3FONT_H_

#include <stdint.h>

#include <array>
#include <map>
#include <memory>

#include "core/fpdfapi/font/cpdf_simplefont.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;
class CPDF_Document;
class CPDF_Type3Char;

class CPDF_Type3Font final : public CPDF_SimpleFont {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_Type3Font() override;

  // CPDF_Font:
  bool IsType3Font() const override;
  const CPDF_Type3Font* AsType3Font() const override;
  CPDF_Type3Font* AsType3Font() override;
  void WillBeDestroyed() override;
  int GetCharWidthF(uint32_t charcode) override;
  FX_RECT GetCharBBox(uint32_t charcode) override;

  void SetPageResources(CPDF_Dictionary* pResources) {
    page_resources_.Reset(pResources);
  }
  CPDF_Type3Char* LoadChar(uint32_t charcode);
  void CheckType3FontMetrics();

  CFX_Matrix& GetFontMatrix() { return font_matrix_; }

 private:
  CPDF_Type3Font(CPDF_Document* document,
                 RetainPtr<CPDF_Dictionary> font_dict,
                 FormFactoryIface* pFormFactory);

  // CPDF_Font:
  bool Load() override;

  // CPDF_SimpleFont:
  void LoadGlyphMap() override;

  // The depth char loading is in, to avoid recurive calling LoadChar().
  int char_loading_depth_ = 0;
  CFX_Matrix font_matrix_;
  UnownedPtr<FormFactoryIface> const form_factory_;
  RetainPtr<CPDF_Dictionary> char_procs_;
  RetainPtr<CPDF_Dictionary> page_resources_;
  RetainPtr<CPDF_Dictionary> font_resources_;
  std::map<uint32_t, std::unique_ptr<CPDF_Type3Char>> cache_map_;
  std::array<int, 256> char_width_l_ = {};
};

#endif  // CORE_FPDFAPI_FONT_CPDF_TYPE3FONT_H_
