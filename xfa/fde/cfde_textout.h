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
  void SetTextColor(FX_ARGB color) { txt_color_ = color; }
  void SetStyles(const FDE_TextStyle& dwStyles);
  void SetAlignment(FDE_TextAlignment iAlignment);
  void SetLineSpace(float fLineSpace);
  void SetMatrix(const CFX_Matrix& matrix) { matrix_ = matrix; }
  void SetLineBreakTolerance(float fTolerance);

  void CalcLogicSize(WideStringView str, CFX_SizeF* pSize);
  void CalcLogicSize(WideStringView str, CFX_RectF* pRect);
  void DrawLogicText(CFX_RenderDevice* device,
                     const WideString& str,
                     const CFX_RectF& rect);
  int32_t GetTotalLines() const { return total_lines_; }

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

  std::unique_ptr<CFGAS_TxtBreak> const txt_break_;
  RetainPtr<CFGAS_GEFont> font_;
  float font_size_ = 12.0f;
  float line_space_ = 12.0f;
  float line_pos_ = 0.0f;
  float tolerance_ = 0.0f;
  FDE_TextAlignment alignment_ = FDE_TextAlignment::kTopLeft;
  FDE_TextStyle styles_;
  std::vector<int32_t> char_widths_;
  FX_ARGB txt_color_ = 0xFF000000;
  Mask<CFGAS_Break::LayoutStyle> txt_bk_styles_;
  WideString text_;
  CFX_Matrix matrix_;
  std::deque<Line> tto_lines_;
  size_t cur_line_ = 0;
  size_t cur_piece_ = 0;
  int32_t total_lines_ = 0;
  std::vector<TextCharPos> char_pos_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFDE_TextOut;

#endif  // XFA_FDE_CFDE_TEXTOUT_H_
