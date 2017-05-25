// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_TXTEDTPAGE_H_
#define XFA_FDE_CFDE_TXTEDTPAGE_H_

#include <deque>
#include <memory>
#include <vector>

#include "core/fxcrt/ifx_chariter.h"
#include "xfa/fde/ifde_visualset.h"

class CFDE_TxtEdtEngine;
class CFDE_TxtEdtParag;
class CFDE_TxtEdtTextSet;

class CFDE_TxtEdtPage : public IFDE_VisualSet {
 public:
  CFDE_TxtEdtPage(CFDE_TxtEdtEngine* pEngine, int32_t nLineIndex);
  ~CFDE_TxtEdtPage() override;

  // IFDE_VisualSet:
  FDE_VISUALOBJTYPE GetType() override;
  CFX_RectF GetRect(const FDE_TEXTEDITPIECE& pPiece) override;

  CFDE_TxtEdtEngine* GetEngine() const;
  int32_t GetCharRect(int32_t nIndex,
                      CFX_RectF& rect,
                      bool bBBox = false) const;
  int32_t GetCharIndex(const CFX_PointF& fPoint, bool& bBefore);
  void CalcRangeRectArray(int32_t nStart,
                          int32_t nCount,
                          std::vector<CFX_RectF>* RectFArr) const;
  int32_t SelectWord(const CFX_PointF& fPoint, int32_t& nCount);
  int32_t GetCharStart() const;
  int32_t GetCharCount() const;
  int32_t GetDisplayPos(const CFX_RectF& rtClip,
                        FXTEXT_CHARPOS*& pCharPos,
                        CFX_RectF* pBBox) const;
  bool IsLoaded(const CFX_RectF* pClipBox);
  int32_t LoadPage(const CFX_RectF* pClipBox);
  void UnloadPage(const CFX_RectF* pClipBox);
  const CFX_RectF& GetContentsBox();

  size_t GetFirstPosition();
  FDE_TEXTEDITPIECE* GetNext(size_t* pos, IFDE_VisualSet*& pVisualSet);

  wchar_t GetChar(const FDE_TEXTEDITPIECE* pIdentity, int32_t index) const;
  int32_t GetWidth(const FDE_TEXTEDITPIECE* pIdentity, int32_t index) const;

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
