// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TEXTOUT_H_
#define XFA_FDE_CFDE_TEXTOUT_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/dib/fx_dib.h"
#include "xfa/fde/cfde_data.h"
#include "xfa/fgas/layout/cfgas_break.h"
#include "xfa/fgas/layout/cfgas_char.h"

class CFGAS_GEFont;
class CFGAS_TxtBreak;
class CFX_RenderDevice;
class TextCharPos;

namespace pdfium {

class CFDE_TextOut {
 public:
  static bool DrawString(CFX_RenderDevice* device,
                         FX_ARGB color,
                         const RetainPtr<CFGAS_GEFont>& pFont,
                         span<TextCharPos> pCharPos,
                         float fFontSize,
                         const CFX_Matrix& matrix);

  CFDE_TextOut();
  ~CFDE_TextOut();

  void SetFont(RetainPtr<CFGAS_GEFont> pFont);
  void SetFontSize(float fFontSize);
  void SetTextColor(FX_ARGB color) { m_TxtColor = color; }
  void SetStyles(const FDE_TextStyle& dwStyles);
  void SetAlignment(FDE_TextAlignment iAlignment);
  void SetLineSpace(float fLineSpace);
  void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  void SetLineBreakTolerance(float fTolerance);

  void CalcLogicSize(WideStringView str, CFX_SizeF* pSize);
  void CalcLogicSize(WideStringView str, CFX_RectF* pRect);
  void DrawLogicText(CFX_RenderDevice* device,
                     const WideString& str,
                     const CFX_RectF& rect);
  int32_t GetTotalLines() const { return m_iTotalLines; }

 private:
  struct Piece {
    Piece();
    Piece(const Piece& that);
    ~Piece();

    size_t start_char = 0;
    size_t char_count = 0;
    uint32_t char_styles = 0;
    CFX_RectF bounds;
  };

  class Line {
   public:
    Line();
    Line(const Line& that);
    ~Line();

    bool new_reload() const { return new_reload_; }
    void set_new_reload(bool reload) { new_reload_ = reload; }

    size_t AddPiece(size_t index, const Piece& piece);
    size_t GetSize() const;
    const Piece* GetPieceAtIndex(size_t index) const;
    Piece* GetPieceAtIndex(size_t index);
    void RemoveLast(size_t count);

   private:
    bool new_reload_ = false;
    std::deque<Piece> pieces_;
  };

  bool RetrieveLineWidth(CFGAS_Char::BreakType dwBreakStatus,
                         float* pStartPos,
                         float* pWidth,
                         float* pHeight);
  void LoadText(const WideString& str, const CFX_RectF& rect);

  void Reload(const CFX_RectF& rect);
  void ReloadLinePiece(Line* pLine, const CFX_RectF& rect);
  bool RetrievePieces(CFGAS_Char::BreakType dwBreakStatus,
                      bool bReload,
                      const CFX_RectF& rect,
                      size_t* pStartChar,
                      int32_t* pPieceWidths);
  void AppendPiece(const Piece& piece, bool bNeedReload, bool bEnd);
  void DoAlignment(const CFX_RectF& rect);
  size_t GetDisplayPos(const Piece* pPiece);

  std::unique_ptr<CFGAS_TxtBreak> const m_pTxtBreak;
  RetainPtr<CFGAS_GEFont> m_pFont;
  float m_fFontSize = 12.0f;
  float m_fLineSpace = 12.0f;
  float m_fLinePos = 0.0f;
  float m_fTolerance = 0.0f;
  FDE_TextAlignment m_iAlignment = FDE_TextAlignment::kTopLeft;
  FDE_TextStyle m_Styles;
  std::vector<int32_t> m_CharWidths;
  FX_ARGB m_TxtColor = 0xFF000000;
  Mask<CFGAS_Break::LayoutStyle> m_dwTxtBkStyles;
  WideString m_wsText;
  CFX_Matrix m_Matrix;
  std::deque<Line> m_ttoLines;
  size_t m_iCurLine = 0;
  size_t m_iCurPiece = 0;
  int32_t m_iTotalLines = 0;
  std::vector<TextCharPos> m_CharPos;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFDE_TextOut;

#endif  // XFA_FDE_CFDE_TEXTOUT_H_
