// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTPAGE_H_
#define XFA_FDE_CFDE_TXTEDTPAGE_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/ifx_chariter.h"
#include "core/fxge/cfx_renderdevice.h"

class CFDE_TxtEdtEngine;
class CFDE_TxtEdtParag;
class CFDE_TxtEdtTextSet;

struct FDE_TEXTEDITPIECE {
  FDE_TEXTEDITPIECE();
  FDE_TEXTEDITPIECE(const FDE_TEXTEDITPIECE& that);
  ~FDE_TEXTEDITPIECE();

  int32_t nStart;
  int32_t nCount;
  int32_t nBidiLevel;
  CFX_RectF rtPiece;
  uint32_t dwCharStyles;
};

inline FDE_TEXTEDITPIECE::FDE_TEXTEDITPIECE() = default;
inline FDE_TEXTEDITPIECE::FDE_TEXTEDITPIECE(const FDE_TEXTEDITPIECE& that) =
    default;
inline FDE_TEXTEDITPIECE::~FDE_TEXTEDITPIECE() = default;

class CFDE_TxtEdtPage {
 public:
  CFDE_TxtEdtPage(CFDE_TxtEdtEngine* pEngine, int32_t nLineIndex);
  ~CFDE_TxtEdtPage();

  CFX_RectF GetRect(const FDE_TEXTEDITPIECE& pPiece) { return CFX_RectF(); }
  CFDE_TxtEdtEngine* GetEngine() const { return m_pEditEngine.Get(); }
  int32_t GetCharRect(int32_t nIndex, CFX_RectF& rect, bool bBBox) const;
  int32_t GetCharIndex(const CFX_PointF& fPoint, bool& bBefore);
  void CalcRangeRectArray(int32_t nStart,
                          int32_t nCount,
                          std::vector<CFX_RectF>* RectFArr) const;
  int32_t SelectWord(const CFX_PointF& fPoint, int32_t& nCount);
  int32_t GetCharStart() const { return m_nPageStart; }
  int32_t GetCharCount() const { return m_nCharCount; }
  int32_t GetDisplayPos(const CFX_RectF& rtClip,
                        FXTEXT_CHARPOS*& pCharPos,
                        CFX_RectF* pBBox) const;
  bool IsLoaded(const CFX_RectF* pClipBox) { return m_bLoaded; }
  int32_t LoadPage(const CFX_RectF* pClipBox);
  void UnloadPage(const CFX_RectF* pClipBox);
  const CFX_RectF& GetContentsBox() { return m_rtPageContents; }

  size_t GetTextPieceCount() const { return m_pTextSet ? m_Pieces.size() : 0; }
  const FDE_TEXTEDITPIECE& GetTextPiece(size_t pos) const;

  wchar_t GetChar(const FDE_TEXTEDITPIECE* pIdentity, int32_t index) const;
  int32_t GetWidth(const FDE_TEXTEDITPIECE* pIdentity, int32_t index) const;

  CFDE_TxtEdtTextSet* GetTextSet() const { return m_pTextSet.get(); }

 private:
  void NormalizePt2Rect(CFX_PointF& ptF,
                        const CFX_RectF& rtF,
                        float fTolerance) const;

  std::unique_ptr<IFX_CharIter> m_pIter;
  std::unique_ptr<CFDE_TxtEdtTextSet> m_pTextSet;
  CFX_UnownedPtr<CFDE_TxtEdtEngine> const m_pEditEngine;
  std::deque<FDE_TEXTEDITPIECE> m_Pieces;
  CFDE_TxtEdtParag* m_pBgnParag;
  CFDE_TxtEdtParag* m_pEndParag;
  int32_t m_nRefCount;
  int32_t m_nPageStart;
  int32_t m_nCharCount;
  int32_t m_nPageIndex;
  bool m_bLoaded;
  CFX_RectF m_rtPage;
  CFX_RectF m_rtPageMargin;
  CFX_RectF m_rtPageContents;
  CFX_RectF m_rtPageCanvas;
  std::vector<int32_t> m_CharWidths;
};

#endif  // XFA_FDE_CFDE_TXTEDTPAGE_H_
