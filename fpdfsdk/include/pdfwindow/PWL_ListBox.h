// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTBOX_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTBOX_H_

#include "PWL_Wnd.h"
#include "fpdfsdk/include/fxedit/fx_edit.h"

class CPDF_ListCtrl;
class CPWL_List_Notify;
class CPWL_ListBox;
class IPWL_Filler_Notify;

class CPWL_List_Notify : public IFX_List_Notify {
 public:
  CPWL_List_Notify(CPWL_ListBox* pList);
  ~CPWL_List_Notify() override;

  // IFX_List_Notify
  void IOnSetScrollInfoX(FX_FLOAT fPlateMin,
                         FX_FLOAT fPlateMax,
                         FX_FLOAT fContentMin,
                         FX_FLOAT fContentMax,
                         FX_FLOAT fSmallStep,
                         FX_FLOAT fBigStep) override {}
  void IOnSetScrollInfoY(FX_FLOAT fPlateMin,
                         FX_FLOAT fPlateMax,
                         FX_FLOAT fContentMin,
                         FX_FLOAT fContentMax,
                         FX_FLOAT fSmallStep,
                         FX_FLOAT fBigStep) override;
  void IOnSetScrollPosX(FX_FLOAT fx) override {}
  void IOnSetScrollPosY(FX_FLOAT fy) override;
  void IOnInvalidateRect(CPDF_Rect* pRect) override;

  void IOnSetCaret(FX_BOOL bVisible,
                   const CPDF_Point& ptHead,
                   const CPDF_Point& ptFoot,
                   const CPVT_WordPlace& place);
  void IOnCaretChange(const CPVT_SecProps& secProps,
                      const CPVT_WordProps& wordProps);

 private:
  CPWL_ListBox* m_pList;
};

class CPWL_ListBox : public CPWL_Wnd {
 public:
  CPWL_ListBox();
  ~CPWL_ListBox() override;

  // CPWL_Wnd
  CFX_ByteString GetClassName() const override;
  void OnCreated() override;
  void OnDestroy() override;
  void GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  FX_BOOL OnKeyDown(FX_WORD nChar, FX_DWORD nFlag) override;
  FX_BOOL OnChar(FX_WORD nChar, FX_DWORD nFlag) override;
  FX_BOOL OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnMouseMove(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnMouseWheel(short zDelta,
                       const CPDF_Point& point,
                       FX_DWORD nFlag) override;
  void KillFocus() override;
  void OnNotify(CPWL_Wnd* pWnd,
                FX_DWORD msg,
                intptr_t wParam = 0,
                intptr_t lParam = 0) override;
  void RePosChildWnd() override;
  CPDF_Rect GetFocusRect() const override;
  void SetFontSize(FX_FLOAT fFontSize) override;
  FX_FLOAT GetFontSize() const override;

  virtual CFX_WideString GetText() const;

  void OnNotifySelChanged(FX_BOOL bKeyDown, FX_BOOL& bExit, FX_DWORD nFlag);

  void AddString(const FX_WCHAR* string);
  void SetTopVisibleIndex(int32_t nItemIndex);
  void ScrollToListItem(int32_t nItemIndex);
  void ResetContent();
  void Reset();
  void Select(int32_t nItemIndex);
  void SetCaret(int32_t nItemIndex);
  void SetHoverSel(FX_BOOL bHoverSel);

  int32_t GetCount() const;
  FX_BOOL IsMultipleSel() const;
  int32_t GetCaretIndex() const;
  int32_t GetCurSel() const;
  FX_BOOL IsItemSelected(int32_t nItemIndex) const;
  int32_t GetTopVisibleIndex() const;
  int32_t FindNext(int32_t nIndex, FX_WCHAR nChar) const;
  CPDF_Rect GetContentRect() const;
  FX_FLOAT GetFirstHeight() const;
  CPDF_Rect GetListRect() const;

  void SetFillerNotify(IPWL_Filler_Notify* pNotify) {
    m_pFillerNotify = pNotify;
  }

 protected:
  IFX_List* m_pList;
  CPWL_List_Notify* m_pListNotify;
  FX_BOOL m_bMouseDown;
  FX_BOOL m_bHoverSel;
  IPWL_Filler_Notify* m_pFillerNotify;

 public:
  void AttachFFLData(void* pData) { m_pFormFiller = pData; }

 private:
  void* m_pFormFiller;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_LISTBOX_H_
