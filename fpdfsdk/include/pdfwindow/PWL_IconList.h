// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICONLIST_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICONLIST_H_

#include "PWL_ListCtrl.h"
#include "PWL_Wnd.h"
#include "core/include/fxcrt/fx_string.h"

class IPWL_IconList_Notify;
class CPWL_IconList_Item;
class CPWL_IconList_Content;
class CPWL_IconList;
class CPWL_Label;

class IPWL_IconList_Notify {
 public:
  virtual ~IPWL_IconList_Notify() {}
  virtual void OnNoteListSelChanged(int32_t nItemIndex) = 0;
};

class CPWL_IconList_Item : public CPWL_Wnd {
 public:
  CPWL_IconList_Item();
  ~CPWL_IconList_Item() override;

  void SetSelect(FX_BOOL bSelected);
  FX_BOOL IsSelected() const;
  void SetData(void* pData);
  void SetIcon(int32_t nIconIndex);
  void SetText(const CFX_WideString& str);
  void SetIconFillColor(const CPWL_Color& color);
  CFX_WideString GetText() const;

 protected:
  // CPWL_Wnd
  CFX_ByteString GetClassName() const override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  void RePosChildWnd() override;
  FX_FLOAT GetItemHeight(FX_FLOAT fLimitWidth) override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void OnEnabled() override;
  void OnDisabled() override;

 private:
  int32_t m_nIconIndex;
  void* m_pData;
  FX_BOOL m_bSelected;
  CPWL_Label* m_pText;
  CPWL_Color m_crIcon;
};

class CPWL_IconList_Content : public CPWL_ListCtrl {
 public:
  CPWL_IconList_Content(int32_t nListCount);
  ~CPWL_IconList_Content() override;

  void SetSelect(int32_t nIndex);
  int32_t GetSelect() const;
  void SetNotify(IPWL_IconList_Notify* pNotify);
  void EnableNotify(FX_BOOL bNotify);
  void SetListData(int32_t nItemIndex, void* pData);
  void SetListIcon(int32_t nItemIndex, int32_t nIconIndex);
  void SetListString(int32_t nItemIndex, const CFX_WideString& str);
  void SetIconFillColor(const CPWL_Color& color);
  CFX_WideString GetListString(int32_t nItemIndex) const;
  IPWL_IconList_Notify* GetNotify() const;
  void ScrollToItem(int32_t nItemIndex);

 protected:
  // CPWL_ListCtrl
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  FX_BOOL OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnMouseMove(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnKeyDown(FX_WORD nChar, FX_DWORD nFlag) override;

 private:
  CPWL_IconList_Item* GetListItem(int32_t nItemIndex) const;
  void SelectItem(int32_t nItemIndex, FX_BOOL bSelect);
  int32_t FindItemIndex(const CPDF_Point& point);

  int32_t m_nSelectIndex;
  IPWL_IconList_Notify* m_pNotify;
  FX_BOOL m_bEnableNotify;
  FX_BOOL m_bMouseDown;
  int32_t m_nListCount;
};

class CPWL_IconList : public CPWL_Wnd {
 public:
  CPWL_IconList(int32_t nListCount);
  ~CPWL_IconList() override;

  void SetSelect(int32_t nIndex);
  void SetTopItem(int32_t nIndex);
  int32_t GetSelect() const;
  void SetNotify(IPWL_IconList_Notify* pNotify);
  void EnableNotify(FX_BOOL bNotify);
  void SetListData(int32_t nItemIndex, void* pData);
  void SetListIcon(int32_t nItemIndex, int32_t nIconIndex);
  void SetListString(int32_t nItemIndex, const CFX_WideString& str);
  void SetIconFillColor(const CPWL_Color& color);
  CFX_WideString GetListString(int32_t nItemIndex) const;

 protected:
  // CPWL_Wnd
  FX_BOOL OnMouseWheel(short zDelta,
                       const CPDF_Point& point,
                       FX_DWORD nFlag) override;
  void OnCreated() override;
  void RePosChildWnd() override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  void OnNotify(CPWL_Wnd* pWnd,
                FX_DWORD msg,
                intptr_t wParam = 0,
                intptr_t lParam = 0) override;

 private:
  CPWL_IconList_Content* m_pListContent;
  int32_t m_nListCount;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_ICONLIST_H_
