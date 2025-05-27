// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TEXTOBJECT_H_
#define CORE_FPDFAPI_PAGE_CPDF_TEXTOBJECT_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

class CPDF_TextObject final : public CPDF_PageObject {
 public:
  struct Item {
    Item();
    Item(const Item& that);
    ~Item();

    uint32_t char_code_ = 0;
    CFX_PointF origin_;
  };

  explicit CPDF_TextObject(int32_t content_stream);
  CPDF_TextObject();
  ~CPDF_TextObject() override;

  // CPDF_PageObject:
  Type GetType() const override;
  void Transform(const CFX_Matrix& matrix) override;
  bool IsText() const override;
  CPDF_TextObject* AsText() override;
  const CPDF_TextObject* AsText() const override;

  std::unique_ptr<CPDF_TextObject> Clone() const;

  size_t CountItems() const;
  Item GetItemInfo(size_t index) const;

  size_t CountChars() const;
  uint32_t GetCharCode(size_t index) const;
  Item GetCharInfo(size_t index) const;
  float GetCharWidth(uint32_t charcode) const;
  int CountWords() const;
  WideString GetWordString(int nWordIndex) const;

  CFX_PointF GetPos() const { return pos_; }
  CFX_Matrix GetTextMatrix() const;

  RetainPtr<CPDF_Font> GetFont() const;
  float GetFontSize() const;

  TextRenderingMode GetTextRenderMode() const;
  void SetTextRenderMode(TextRenderingMode mode);

  void SetText(const ByteString& str);
  void SetPosition(const CFX_PointF& pos) { pos_ = pos; }

  const std::vector<uint32_t>& GetCharCodes() const { return char_codes_; }
  const std::vector<float>& GetCharPositions() const { return char_pos_; }

  // Caller is expected to call SetDirty(true) when done changing the object.
  void SetTextMatrix(const CFX_Matrix& matrix);

  void SetSegments(pdfium::span<const ByteString> strings,
                   pdfium::span<const float> kernings);

  CFX_PointF CalcPositionData(float horz_scale);

 private:
  float CalcPositionDataInternal(const RetainPtr<CPDF_Font>& font);

  CFX_PointF pos_;
  std::vector<uint32_t> char_codes_;
  std::vector<float> char_pos_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TEXTOBJECT_H_
