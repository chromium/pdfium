// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FEE_FDE_TXTEDTPAGE_H_
#define XFA_FEE_FDE_TXTEDTPAGE_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "xfa/fde/fde_visualset.h"
#include "xfa/fee/ifde_txtedtpage.h"
#include "xfa/fgas/crt/fgas_utils.h"

class IFX_CharIter;
class CFDE_TxtEdtEngine;
class CFDE_TxtEdtPage;
class CFDE_TxtEdtParag;

enum FDE_TXTEDT_CHARTYPE {
  FDE_TXTEDT_CHARTYPE_Unknown = 0,
  FDE_TXTEDT_CHARTYPE_Tab,
  FDE_TXTEDT_CHARTYPE_Space,
  FDE_TXTEDT_CHARTYPE_Punctuation,
  FDE_TXTEDT_CHARTYPE_LineBreak,
  FDE_TXTEDT_CHARTYPE_Number,
  FDE_TXTEDT_CHARTYPE_Char,
  FDE_TXTEDT_CHARTYPE_CJK,
};

inline FDE_TXTEDT_CHARTYPE FDE_GetEditSelCharType(FX_WCHAR wChar) {
  if (wChar == 0x9) {
    return FDE_TXTEDT_CHARTYPE_Tab;
  } else if (wChar == 0x20 || wChar == 0xA0) {
    return FDE_TXTEDT_CHARTYPE_Space;
  } else if (wChar == 0x9 || wChar == 0x20 || wChar == 0xA0 ||
             (wChar >= L'!' && wChar <= L'/') ||
             (wChar >= L':' && wChar <= L'@') ||
             (wChar >= L'[' && wChar <= L'^') ||
             (wChar >= L'{' && wChar <= L'~') || wChar == 0x60) {
    return FDE_TXTEDT_CHARTYPE_Punctuation;
  } else if (wChar == 0x0a || wChar == 0x0d) {
    return FDE_TXTEDT_CHARTYPE_LineBreak;
  } else if (wChar >= '0' && wChar <= '9') {
    return FDE_TXTEDT_CHARTYPE_Number;
  } else if ((wChar >= 0x2e80 && wChar <= 0x2eff) ||
             (wChar >= 0x3000 && wChar <= 0x303f) ||
             (wChar >= 0x31c0 && wChar <= 0x31ef) ||
             (wChar >= 0x3200 && wChar <= 0x32ff) ||
             (wChar >= 0x3300 && wChar <= 0x33ff) ||
             (wChar >= 0x3400 && wChar <= 0x4dbf) ||
             (wChar >= 0x4e00 && wChar <= 0x9fff) ||
             (wChar >= 0xf900 && wChar <= 0xfaff) ||
             (wChar >= 0xfe30 && wChar <= 0xfe4f)) {
    return FDE_TXTEDT_CHARTYPE_CJK;
  } else {
    return FDE_TXTEDT_CHARTYPE_Char;
  }
}

struct FDE_TEXTEDITPIECE {
  int32_t nStart;
  int32_t nCount;
  int32_t nBidiLevel;
  CFX_RectF rtPiece;
  uint32_t dwCharStyles;
};
typedef CFX_MassArrayTemplate<FDE_TEXTEDITPIECE> CFDE_TXTEDTPieceMassArray;

class CFDE_TxtEdtTextSet : public IFDE_TextSet {
 public:
  CFDE_TxtEdtTextSet(CFDE_TxtEdtPage* pPage);
  ~CFDE_TxtEdtTextSet();

  virtual FDE_VISUALOBJTYPE GetType();
  virtual FX_BOOL GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox);
  virtual FX_BOOL GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix& matrix);
  virtual FX_BOOL GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt);
  virtual FX_BOOL GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt);
  virtual int32_t GetString(FDE_HVISUALOBJ hText, CFX_WideString& wsText);
  virtual IFX_Font* GetFont(FDE_HVISUALOBJ hText);
  virtual FX_FLOAT GetFontSize(FDE_HVISUALOBJ hText);
  virtual FX_ARGB GetFontColor(FDE_HVISUALOBJ hText);
  virtual int32_t GetDisplayPos(FDE_HVISUALOBJ hText,
                                FXTEXT_CHARPOS* pCharPos,
                                FX_BOOL bCharCode = FALSE,
                                CFX_WideString* pWSForms = NULL);
  virtual int32_t GetCharRects(FDE_HVISUALOBJ hText, CFX_RectFArray& rtArray);
  virtual int32_t GetCharRects_Impl(FDE_HVISUALOBJ hText,
                                    CFX_RectFArray& rtArray,
                                    FX_BOOL bBBox = FALSE);

 private:
  CFDE_TxtEdtPage* m_pPage;
};

class CFDE_TxtEdtPage : public IFDE_TxtEdtPage {
 public:
  CFDE_TxtEdtPage(IFDE_TxtEdtEngine* pEngine, int32_t nLineIndex);

  // IFDE_TxtEditPage:
  void Release() override;
  IFDE_TxtEdtEngine* GetEngine() const override;
  int32_t GetCharRect(int32_t nIndex,
                      CFX_RectF& rect,
                      FX_BOOL bBBox = FALSE) const override;
  int32_t GetCharIndex(const CFX_PointF& fPoint, FX_BOOL& bBefore) override;
  void CalcRangeRectArray(int32_t nStart,
                          int32_t nCount,
                          CFX_RectFArray& RectFArr) const override;
  int32_t SelectWord(const CFX_PointF& fPoint, int32_t& nCount) override;
  int32_t GetCharStart() const override;
  int32_t GetCharCount() const override;
  int32_t GetDisplayPos(const CFX_RectF& rtClip,
                        FXTEXT_CHARPOS*& pCharPos,
                        CFX_RectF* pBBox) const override;
  FX_BOOL IsLoaded(const CFX_RectF* pClipBox) override;
  int32_t LoadPage(const CFX_RectF* pClipBox, IFX_Pause* pPause) override;
  void UnloadPage(const CFX_RectF* pClipBox) override;
  const CFX_RectF& GetContentsBox() override;

  // IFDE_VisualSet:
  FDE_VISUALOBJTYPE GetType() override;
  FX_BOOL GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox) override;
  FX_BOOL GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix& matrix) override;
  FX_BOOL GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) override;
  FX_BOOL GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) override;

  // IFDE_CanvasSet:
  FX_POSITION GetFirstPosition(FDE_HVISUALOBJ hCanvas) override;
  FDE_HVISUALOBJ GetNext(FDE_HVISUALOBJ hCanvas,
                         FX_POSITION& pos,
                         IFDE_VisualSet*& pVisualSet) override;
  FDE_HVISUALOBJ GetParentCanvas(FDE_HVISUALOBJ hCanvas,
                                 IFDE_VisualSet*& pVisualSet) override;

  // IFX_TxtAccess:
  FX_WCHAR GetChar(void* pIdentity, int32_t index) const override;
  int32_t GetWidth(void* pIdentity, int32_t index) const override;

 protected:
  virtual ~CFDE_TxtEdtPage();

 private:
  void NormalizePt2Rect(CFX_PointF& ptF,
                        const CFX_RectF& rtF,
                        FX_FLOAT fTolerance) const;

  IFX_CharIter* m_pIter;
  CFDE_TxtEdtTextSet* m_pTextSet;
  CFDE_TxtEdtEngine* m_pEditEngine;
  CFDE_TXTEDTPieceMassArray m_PieceMassArr;
  CFDE_TxtEdtParag* m_pBgnParag;
  CFDE_TxtEdtParag* m_pEndParag;
  int32_t m_nRefCount;
  int32_t m_nPageStart;
  int32_t m_nCharCount;
  int32_t m_nPageIndex;
  FX_BOOL m_bLoaded;
  CFX_RectF m_rtPage;
  CFX_RectF m_rtPageMargin;
  CFX_RectF m_rtPageContents;
  CFX_RectF m_rtPageCanvas;
  int32_t* m_pCharWidth;
};

#endif  // XFA_FEE_FDE_TXTEDTPAGE_H_
