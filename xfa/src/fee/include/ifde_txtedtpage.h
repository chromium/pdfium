// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _IFDE_TXTEDTPAGE_H
#define _IFDE_TXTEDTPAGE_H
class IFDE_TxtEdtEngine;
class IFDE_TxtEdtPage;
class IFDE_TxtEdtPage : public IFDE_CanvasSet, public IFX_TxtAccess {
 public:
  static IFDE_TxtEdtPage* Create(IFDE_TxtEdtEngine* pEngine, int32_t nIndex);

  virtual void Release() = 0;

  virtual IFDE_TxtEdtEngine* GetEngine() const = 0;
  virtual int32_t GetCharRect(int32_t nIndex,
                              CFX_RectF& rect,
                              FX_BOOL bBBox = FALSE) const = 0;
  virtual int32_t GetCharIndex(const CFX_PointF& fPoint, FX_BOOL& bBefore) = 0;
  virtual void CalcRangeRectArray(int32_t nStart,
                                  int32_t nCount,
                                  CFX_RectFArray& RectFArr) const = 0;
  virtual int32_t SelectWord(const CFX_PointF& fPoint, int32_t& nCount) = 0;
  virtual int32_t GetCharStart() const = 0;
  virtual int32_t GetCharCount() const = 0;

  virtual int32_t GetDisplayPos(const CFX_RectF& rtClip,
                                FXTEXT_CHARPOS*& pCharPos,
                                FX_LPRECTF pBBox = NULL) const = 0;
  virtual FX_BOOL IsLoaded(FX_LPCRECTF pClipBox = NULL) = 0;
  virtual int32_t LoadPage(FX_LPCRECTF pClipBox = NULL,
                           IFX_Pause* pPause = NULL) = 0;
  virtual void UnloadPage(FX_LPCRECTF pClipBox = NULL) = 0;
  virtual const CFX_RectF& GetContentsBox() = 0;
};
#endif
