// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TEXTSTATE_H_
#define CORE_FPDFAPI_PAGE_CPDF_TEXTSTATE_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/shared_copy_on_write.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Document;
class CPDF_Font;

// See PDF Reference 1.7, page 402, table 5.3.
enum class TextRenderingMode {
  MODE_UNKNOWN = -1,
  MODE_FILL = 0,
  MODE_STROKE = 1,
  MODE_FILL_STROKE = 2,
  MODE_INVISIBLE = 3,
  MODE_FILL_CLIP = 4,
  MODE_STROKE_CLIP = 5,
  MODE_FILL_STROKE_CLIP = 6,
  MODE_CLIP = 7,
  MODE_LAST = MODE_CLIP,
};

class CPDF_TextState {
 public:
  CPDF_TextState();
  ~CPDF_TextState();

  void Emplace();

  RetainPtr<CPDF_Font> GetFont() const;
  void SetFont(const RetainPtr<CPDF_Font>& pFont);

  float GetFontSize() const;
  void SetFontSize(float size);

  const float* GetMatrix() const;
  float* GetMutableMatrix();

  float GetCharSpace() const;
  void SetCharSpace(float sp);

  float GetWordSpace() const;
  void SetWordSpace(float sp);

  float GetFontSizeH() const;

  TextRenderingMode GetTextMode() const;
  void SetTextMode(TextRenderingMode mode);

  const float* GetCTM() const;
  float* GetMutableCTM();

 private:
  class TextData final : public Retainable {
   public:
    template <typename T, typename... Args>
    friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

    RetainPtr<TextData> Clone() const;

    void SetFont(const RetainPtr<CPDF_Font>& pFont);
    float GetFontSizeV() const;
    float GetFontSizeH() const;

    RetainPtr<CPDF_Font> m_pFont;
    UnownedPtr<CPDF_Document> m_pDocument;
    float m_FontSize = 1.0f;
    float m_CharSpace = 0.0f;
    float m_WordSpace = 0.0f;
    TextRenderingMode m_TextMode = TextRenderingMode::MODE_FILL;
    float m_Matrix[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    float m_CTM[4] = {1.0f, 0.0f, 0.0f, 1.0f};

   private:
    TextData();
    TextData(const TextData& that);
    ~TextData() override;
  };

  SharedCopyOnWrite<TextData> m_Ref;
};

bool SetTextRenderingModeFromInt(int iMode, TextRenderingMode* mode);
bool TextRenderingModeIsClipMode(const TextRenderingMode& mode);
bool TextRenderingModeIsStrokeMode(const TextRenderingMode& mode);

#endif  // CORE_FPDFAPI_PAGE_CPDF_TEXTSTATE_H_
