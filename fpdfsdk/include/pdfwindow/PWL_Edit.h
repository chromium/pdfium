// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_

#include "core/include/fxcrt/fx_basic.h"
#include "fpdfsdk/include/fxedit/fx_edit.h"
#include "fpdfsdk/include/pdfwindow/PWL_EditCtrl.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

class IPWL_SpellCheck;

class IPWL_Filler_Notify {
 public:
  virtual ~IPWL_Filler_Notify() {}
  virtual void QueryWherePopup(
      void* pPrivateData,
      FX_FLOAT fPopupMin,
      FX_FLOAT fPopupMax,
      int32_t& nRet,
      FX_FLOAT& fPopupRet) = 0;  // nRet: (0:bottom 1:top)
  virtual void OnBeforeKeyStroke(void* pPrivateData,
                                 CFX_WideString& strChange,
                                 const CFX_WideString& strChangeEx,
                                 int nSelStart,
                                 int nSelEnd,
                                 FX_BOOL bKeyDown,
                                 FX_BOOL& bRC,
                                 FX_BOOL& bExit,
                                 FX_DWORD nFlag) = 0;
#ifdef PDF_ENABLE_XFA
  virtual void OnPopupPreOpen(void* pPrivateData,
                              FX_BOOL& bExit,
                              FX_DWORD nFlag) = 0;
  virtual void OnPopupPostOpen(void* pPrivateData,
                               FX_BOOL& bExit,
                               FX_DWORD nFlag) = 0;
#endif  // PDF_ENABLE_XFA
};

class CPWL_Edit : public CPWL_EditCtrl, public IFX_Edit_OprNotify {
 public:
  CPWL_Edit();
  ~CPWL_Edit() override;

  // CPWL_EditCtrl
  CFX_ByteString GetClassName() const override;
  void OnDestroy() override;
  void OnCreated() override;
  void RePosChildWnd() override;
  CPDF_Rect GetClientRect() const override;
  void GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  FX_BOOL OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnLButtonDblClk(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnRButtonUp(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnMouseWheel(short zDelta,
                       const CPDF_Point& point,
                       FX_DWORD nFlag) override;
  FX_BOOL OnKeyDown(FX_WORD nChar, FX_DWORD nFlag) override;
  FX_BOOL OnChar(FX_WORD nChar, FX_DWORD nFlag) override;
  CPDF_Rect GetFocusRect() const override;
  void OnSetFocus() override;
  void OnKillFocus() override;

  void SetAlignFormatH(PWL_EDIT_ALIGNFORMAT_H nFormat = PEAH_LEFT,
                       FX_BOOL bPaint = TRUE);  // 0:left 1:right 2:middle
  void SetAlignFormatV(PWL_EDIT_ALIGNFORMAT_V nFormat = PEAV_TOP,
                       FX_BOOL bPaint = TRUE);  // 0:top 1:bottom 2:center

  void SetCharArray(int32_t nCharArray);
  void SetLimitChar(int32_t nLimitChar);

  void SetHorzScale(int32_t nHorzScale, FX_BOOL bPaint = TRUE);
  void SetCharSpace(FX_FLOAT fCharSpace, FX_BOOL bPaint = TRUE);

  void SetLineLeading(FX_FLOAT fLineLeading, FX_BOOL bPaint = TRUE);

  void EnableSpellCheck(FX_BOOL bEnabled);

  FX_BOOL CanSelectAll() const;
  FX_BOOL CanClear() const;
  FX_BOOL CanCopy() const;
  FX_BOOL CanCut() const;
  FX_BOOL CanPaste() const;

  virtual void CopyText();
  virtual void PasteText();
  virtual void CutText();

  virtual void SetText(const FX_WCHAR* csText);
  void ReplaceSel(const FX_WCHAR* csText);

  CFX_ByteString GetTextAppearanceStream(const CPDF_Point& ptOffset) const;
  CFX_ByteString GetCaretAppearanceStream(const CPDF_Point& ptOffset) const;
  CFX_ByteString GetSelectAppearanceStream(const CPDF_Point& ptOffset) const;

  FX_BOOL IsTextFull() const;

  static FX_FLOAT GetCharArrayAutoFontSize(CPDF_Font* pFont,
                                           const CPDF_Rect& rcPlate,
                                           int32_t nCharArray);

  void SetFillerNotify(IPWL_Filler_Notify* pNotify) {
    m_pFillerNotify = pNotify;
  }

  void GeneratePageObjects(CPDF_PageObjects* pPageObjects,
                           const CPDF_Point& ptOffset,
                           CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray);
  void GeneratePageObjects(CPDF_PageObjects* pPageObjects,
                           const CPDF_Point& ptOffset);

 protected:
  // IFX_Edit_OprNotify
  void OnInsertWord(const CPVT_WordPlace& place,
                    const CPVT_WordPlace& oldplace) override;
  void OnInsertReturn(const CPVT_WordPlace& place,
                      const CPVT_WordPlace& oldplace) override;
  void OnBackSpace(const CPVT_WordPlace& place,
                   const CPVT_WordPlace& oldplace) override;
  void OnDelete(const CPVT_WordPlace& place,
                const CPVT_WordPlace& oldplace) override;
  void OnClear(const CPVT_WordPlace& place,
               const CPVT_WordPlace& oldplace) override;
  void OnSetText(const CPVT_WordPlace& place,
                 const CPVT_WordPlace& oldplace) override;
  void OnInsertText(const CPVT_WordPlace& place,
                    const CPVT_WordPlace& oldplace) override;
  void OnAddUndo(IFX_Edit_UndoItem* pUndoItem) override;

 private:
  CPVT_WordRange GetSelectWordRange() const;
  virtual void ShowVScrollBar(FX_BOOL bShow);
  FX_BOOL IsVScrollBarVisible() const;
  void SetParamByFlag();

  FX_FLOAT GetCharArrayAutoFontSize(int32_t nCharArray);
  CPDF_Point GetWordRightBottomPoint(const CPVT_WordPlace& wpWord);

  CPVT_WordRange CombineWordRange(const CPVT_WordRange& wr1,
                                  const CPVT_WordRange& wr2);
  CPVT_WordRange GetLatinWordsRange(const CPDF_Point& point) const;
  CPVT_WordRange GetLatinWordsRange(const CPVT_WordPlace& place) const;
  CPVT_WordRange GetArabicWordsRange(const CPVT_WordPlace& place) const;
  CPVT_WordRange GetSameWordsRange(const CPVT_WordPlace& place,
                                   FX_BOOL bLatin,
                                   FX_BOOL bArabic) const;

 public:
  FX_BOOL IsProceedtoOnChar(FX_WORD nKeyCode, FX_DWORD nFlag);

 private:
  IPWL_Filler_Notify* m_pFillerNotify;
  IPWL_SpellCheck* m_pSpellCheck;
  FX_BOOL m_bFocus;
  CPDF_Rect m_rcOldWindow;

 public:
  void AttachFFLData(void* pData) { m_pFormFiller = pData; }

 private:
  void* m_pFormFiller;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_EDIT_H_
