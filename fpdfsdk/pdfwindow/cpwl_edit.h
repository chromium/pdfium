// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_EDIT_H_
#define FPDFSDK_PDFWINDOW_CPWL_EDIT_H_

#include <utility>

#include "core/fpdfdoc/cpvt_wordrange.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_basic.h"
#include "fpdfsdk/pdfwindow/cpwl_edit_ctrl.h"

class IPWL_Filler_Notify {
 public:
  virtual ~IPWL_Filler_Notify() {}

  // Must write to |bBottom| and |fPopupRet|.
  virtual void QueryWherePopup(void* pPrivateData,
                               float fPopupMin,
                               float fPopupMax,
                               bool* bBottom,
                               float* fPopupRet) = 0;
  virtual std::pair<bool, bool> OnBeforeKeyStroke(
      void* pPrivateData,
      CFX_WideString& strChange,
      const CFX_WideString& strChangeEx,
      int nSelStart,
      int nSelEnd,
      bool bKeyDown,
      uint32_t nFlag) = 0;
#ifdef PDF_ENABLE_XFA
  virtual bool OnPopupPreOpen(void* pPrivateData, uint32_t nFlag) = 0;
  virtual bool OnPopupPostOpen(void* pPrivateData, uint32_t nFlag) = 0;
#endif  // PDF_ENABLE_XFA
};

class CPWL_Edit : public CPWL_EditCtrl {
 public:
  CPWL_Edit();
  ~CPWL_Edit() override;

  // CPWL_EditCtrl
  CFX_ByteString GetClassName() const override;
  void OnCreated() override;
  void RePosChildWnd() override;
  CFX_FloatRect GetClientRect() const override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  bool OnLButtonDown(const CFX_PointF& point, uint32_t nFlag) override;
  bool OnLButtonDblClk(const CFX_PointF& point, uint32_t nFlag) override;
  bool OnRButtonUp(const CFX_PointF& point, uint32_t nFlag) override;
  bool OnMouseWheel(short zDelta,
                    const CFX_PointF& point,
                    uint32_t nFlag) override;
  bool OnKeyDown(uint16_t nChar, uint32_t nFlag) override;
  bool OnChar(uint16_t nChar, uint32_t nFlag) override;
  CFX_FloatRect GetFocusRect() const override;
  void OnSetFocus() override;
  void OnKillFocus() override;

  void SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat = PEAV_TOP,
                       bool bPaint = true);  // 0:top 1:bottom 2:center

  void SetCharArray(int32_t nCharArray);
  void SetLimitChar(int32_t nLimitChar);

  void SetCharSpace(float fCharSpace);

  bool CanSelectAll() const;
  bool CanClear() const;
  bool CanCopy() const;
  bool CanCut() const;

  void CutText();

  void SetText(const CFX_WideString& csText);
  void ReplaceSel(const CFX_WideString& csText);

  bool IsTextFull() const;

  static float GetCharArrayAutoFontSize(CPDF_Font* pFont,
                                        const CFX_FloatRect& rcPlate,
                                        int32_t nCharArray);

  void SetFillerNotify(IPWL_Filler_Notify* pNotify) {
    m_pFillerNotify = pNotify;
  }

  bool IsProceedtoOnChar(uint16_t nKeyCode, uint32_t nFlag);
  void AttachFFLData(CFFL_FormFiller* pData) { m_pFormFiller = pData; }

  void OnInsertWord(const CPVT_WordPlace& place,
                    const CPVT_WordPlace& oldplace);
  void OnInsertReturn(const CPVT_WordPlace& place,
                      const CPVT_WordPlace& oldplace);
  void OnBackSpace(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
  void OnDelete(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
  void OnClear(const CPVT_WordPlace& place, const CPVT_WordPlace& oldplace);
  void OnInsertText(const CPVT_WordPlace& place,
                    const CPVT_WordPlace& oldplace);

 private:
  CPVT_WordRange GetSelectWordRange() const;
  bool IsVScrollBarVisible() const;
  void SetParamByFlag();

  float GetCharArrayAutoFontSize(int32_t nCharArray);
  CFX_PointF GetWordRightBottomPoint(const CPVT_WordPlace& wpWord);

  CPVT_WordRange CombineWordRange(const CPVT_WordRange& wr1,
                                  const CPVT_WordRange& wr2);
  CPVT_WordRange GetLatinWordsRange(const CFX_PointF& point) const;
  CPVT_WordRange GetLatinWordsRange(const CPVT_WordPlace& place) const;
  CPVT_WordRange GetArabicWordsRange(const CPVT_WordPlace& place) const;
  CPVT_WordRange GetSameWordsRange(const CPVT_WordPlace& place,
                                   bool bLatin,
                                   bool bArabic) const;

  bool m_bFocus;
  CFX_FloatRect m_rcOldWindow;
  CFX_UnownedPtr<IPWL_Filler_Notify> m_pFillerNotify;
  CFX_UnownedPtr<CFFL_FormFiller> m_pFormFiller;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_EDIT_H_
