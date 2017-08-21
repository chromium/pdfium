// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TEXTOUT_H_
#define XFA_FDE_CFDE_TEXTOUT_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_char.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

class CFDE_RenderDevice;
class CFX_RenderDevice;
class CFX_TxtBreak;

enum class FDE_TextAlignment : uint8_t {
  kTopLeft = 0,
  kCenterLeft,
  kCenter,
  kCenterRight
};

struct FDE_TextStyle {
  FDE_TextStyle()
      : single_line_(false), line_wrap_(false), last_line_height_(false) {}
  ~FDE_TextStyle() {}

  void Reset() {
    single_line_ = false;
    line_wrap_ = false;
    last_line_height_ = false;
  }

  bool single_line_;
  bool line_wrap_;
  bool last_line_height_;
};

struct FDE_TTOPIECE {
  FDE_TTOPIECE();
  FDE_TTOPIECE(const FDE_TTOPIECE& that);
  ~FDE_TTOPIECE();

  int32_t iStartChar;
  int32_t iChars;
  uint32_t dwCharStyles;
  CFX_RectF rtPiece;
};

class CFDE_TTOLine {
 public:
  CFDE_TTOLine();
  CFDE_TTOLine(const CFDE_TTOLine& ttoLine);
  ~CFDE_TTOLine();

  bool GetNewReload() const { return m_bNewReload; }
  void SetNewReload(bool reload) { m_bNewReload = reload; }
  int32_t AddPiece(int32_t index, const FDE_TTOPIECE& ttoPiece);
  int32_t GetSize() const;
  FDE_TTOPIECE* GetPtrAt(int32_t index);
  void RemoveLast(int32_t iCount);

 private:
  bool m_bNewReload;
  std::deque<FDE_TTOPIECE> m_pieces;
};

class CFDE_TextOut {
 public:
  static bool DrawString(CFX_RenderDevice* device,
                         FX_ARGB color,
                         const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                         FXTEXT_CHARPOS* pCharPos,
                         int32_t iCount,
                         float fFontSize,
                         const CFX_Matrix* pMatrix);

  CFDE_TextOut();
  ~CFDE_TextOut();

  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont);
  void SetFontSize(float fFontSize);
  void SetTextColor(FX_ARGB color) { m_TxtColor = color; }
  void SetStyles(const FDE_TextStyle& dwStyles);
  void SetAlignment(FDE_TextAlignment iAlignment);
  void SetLineSpace(float fLineSpace);
  void SetMatrix(const CFX_Matrix& matrix) { m_Matrix = matrix; }
  void SetLineBreakTolerance(float fTolerance);

  void CalcLogicSize(const CFX_WideString& str, CFX_SizeF& size);
  void CalcLogicSize(const CFX_WideString& str, CFX_RectF& rect);
  void DrawLogicText(CFX_RenderDevice* device,
                     const CFX_WideStringC& str,
                     const CFX_RectF& rect);
  int32_t GetTotalLines() const { return m_iTotalLines; }

 private:
  bool RetrieveLineWidth(CFX_BreakType dwBreakStatus,
                         float& fStartPos,
                         float& fWidth,
                         float& fHeight);
  void LoadText(const CFX_WideString& str, const CFX_RectF& rect);

  void Reload(const CFX_RectF& rect);
  void ReloadLinePiece(CFDE_TTOLine* pLine, const CFX_RectF& rect);
  bool RetrievePieces(CFX_BreakType dwBreakStatus,
                      int32_t& iStartChar,
                      int32_t& iPieceWidths,
                      bool bReload,
                      const CFX_RectF& rect);
  void AppendPiece(const FDE_TTOPIECE& ttoPiece, bool bNeedReload, bool bEnd);
  void DoAlignment(const CFX_RectF& rect);
  int32_t GetDisplayPos(FDE_TTOPIECE* pPiece);

  std::unique_ptr<CFX_TxtBreak> m_pTxtBreak;
  CFX_RetainPtr<CFGAS_GEFont> m_pFont;
  float m_fFontSize;
  float m_fLineSpace;
  float m_fLinePos;
  float m_fTolerance;
  FDE_TextAlignment m_iAlignment;
  FDE_TextStyle m_Styles;
  std::vector<int32_t> m_CharWidths;
  FX_ARGB m_TxtColor;
  uint32_t m_dwTxtBkStyles;
  CFX_WideString m_wsText;
  CFX_Matrix m_Matrix;
  std::deque<CFDE_TTOLine> m_ttoLines;
  int32_t m_iCurLine;
  int32_t m_iCurPiece;
  int32_t m_iTotalLines;
  std::vector<FXTEXT_CHARPOS> m_CharPos;
};

#endif  // XFA_FDE_CFDE_TEXTOUT_H_
