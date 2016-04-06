// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_PWL_NOTE_H_
#define FPDFSDK_PDFWINDOW_PWL_NOTE_H_

#include "fpdfsdk/pdfwindow/PWL_Button.h"
#include "fpdfsdk/pdfwindow/PWL_Edit.h"
#include "fpdfsdk/pdfwindow/PWL_ListCtrl.h"
#include "fpdfsdk/pdfwindow/PWL_ScrollBar.h"
#include "fpdfsdk/pdfwindow/PWL_Wnd.h"

class CPWL_Label;
class CPWL_Note;
class CPWL_NoteItem;
class CPWL_Note_CloseBox;
class CPWL_Note_Contents;
class CPWL_Note_Edit;
class CPWL_Note_Icon;
class CPWL_Note_LBBox;
class CPWL_Note_Options;
class CPWL_Note_RBBox;
class IPWL_NoteHandler;
class IPWL_NoteItem;
class IPopup_Note;

class IPWL_NoteHandler {
 public:
  virtual ~IPWL_NoteHandler() {}
  virtual void OnNoteColorChanged(const CPWL_Color& color) = 0;
};

class IPWL_NoteItem {
 public:
  virtual ~IPWL_NoteItem() {}
  virtual void SetPrivateData(void* pData) = 0;
  virtual void SetBkColor(const CPWL_Color& color) = 0;
  virtual void SetSubjectName(const CFX_WideString& sName) = 0;
  virtual void SetDateTime(FX_SYSTEMTIME time) = 0;
  virtual void SetContents(const CFX_WideString& sContents) = 0;

  virtual int32_t CountSubItems() const = 0;
  virtual IPWL_NoteItem* GetSubItems(int32_t index) const = 0;
  virtual void SetFocus() = 0;

  virtual IPWL_NoteItem* GetParentItem() const = 0;
  virtual void* GetPrivateData() const = 0;
  virtual CFX_WideString GetAuthorName() const = 0;
  virtual CPWL_Color GetBkColor() const = 0;
  virtual CFX_WideString GetContents() const = 0;
  virtual FX_SYSTEMTIME GetDateTime() const = 0;
  virtual CFX_WideString GetSubjectName() const = 0;

  virtual CPWL_Edit* GetEdit() const = 0;
};

class CPWL_Note_Icon : public CPWL_Wnd {
 public:
  CPWL_Note_Icon();
  ~CPWL_Note_Icon() override;

  void SetIconType(int32_t nType);

 protected:
  // CPWL_Wnd
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;

 private:
  int32_t m_nType;
};

class CPWL_Note_CloseBox : public CPWL_Button {
 public:
  CPWL_Note_CloseBox();
  ~CPWL_Note_CloseBox() override;

 protected:
  // CPWL_Button
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  FX_BOOL OnLButtonDown(const CFX_FloatPoint& point, uint32_t nFlag) override;
  FX_BOOL OnLButtonUp(const CFX_FloatPoint& point, uint32_t nFlag) override;

 private:
  FX_BOOL m_bMouseDown;
};

class CPWL_Note_LBBox : public CPWL_Wnd {
 public:
  CPWL_Note_LBBox();
  ~CPWL_Note_LBBox() override;

 protected:
  // CPWL_Wnd
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
};

class CPWL_Note_RBBox : public CPWL_Wnd {
 public:
  CPWL_Note_RBBox();
  ~CPWL_Note_RBBox() override;

 protected:
  // CPWL_Wnd
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
};

class CPWL_Note_Edit : public CPWL_Edit {
 public:
  CPWL_Note_Edit();
  ~CPWL_Note_Edit() override;

  void EnableNotify(FX_BOOL bEnable) { m_bEnableNotify = bEnable; }

  // CPWL_Edit
  FX_FLOAT GetItemLeftMargin() override;
  FX_FLOAT GetItemRightMargin() override;
  FX_FLOAT GetItemHeight(FX_FLOAT fLimitWidth) override;
  void SetText(const FX_WCHAR* csText) override;
  void OnNotify(CPWL_Wnd* pWnd,
                uint32_t msg,
                intptr_t wParam = 0,
                intptr_t lParam = 0) override;
  void RePosChildWnd() override;
  void OnSetFocus() override;
  void OnKillFocus() override;

 private:
  FX_BOOL m_bEnableNotify;
  FX_FLOAT m_fOldItemHeight;
  FX_BOOL m_bSizeChanged;
  FX_FLOAT m_fOldMin;
  FX_FLOAT m_fOldMax;
};

class CPWL_Note_Options : public CPWL_Wnd {
 public:
  CPWL_Note_Options();
  ~CPWL_Note_Options() override;

  CFX_FloatRect GetContentRect() const;
  void SetText(const CFX_WideString& sText);

  // CPWL_Wnd
  void RePosChildWnd() override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void SetTextColor(const CPWL_Color& color) override;

 private:
  CPWL_Label* m_pText;
};

class CPWL_Note_Contents : public CPWL_ListCtrl {
 public:
  CPWL_Note_Contents();
  ~CPWL_Note_Contents() override;

  void SetEditFocus(FX_BOOL bLast);
  CPWL_Edit* GetEdit() const;

  void SetText(const CFX_WideString& sText);
  CFX_WideString GetText() const;

  int32_t CountSubItems() const;
  IPWL_NoteItem* GetSubItems(int32_t index) const;

  virtual IPWL_NoteItem* GetHitNoteItem(const CFX_FloatPoint& point);
  void EnableRead(FX_BOOL bEnabled);
  void EnableModify(FX_BOOL bEnabled);

  // CPWL_ListCtrl
  CFX_ByteString GetClassName() const override;
  void OnNotify(CPWL_Wnd* pWnd,
                uint32_t msg,
                intptr_t wParam = 0,
                intptr_t lParam = 0) override;
  FX_BOOL OnLButtonDown(const CFX_FloatPoint& point, uint32_t nFlag) override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;

 private:
  CPWL_Note_Edit* m_pEdit;
};

class CPWL_NoteItem : public CPWL_Wnd, public IPWL_NoteItem {
 public:
  CPWL_NoteItem();
  ~CPWL_NoteItem() override;

  virtual IPWL_NoteItem* GetHitNoteItem(const CFX_FloatPoint& point);
  virtual IPWL_NoteItem* GetFocusedNoteItem() const;

  virtual FX_BOOL IsTopItem() const { return FALSE; }

  void EnableRead(FX_BOOL bEnabled);
  void EnableModify(FX_BOOL bEnabled);

  // IPWL_NoteItem
  void SetPrivateData(void* pData) override;
  void SetBkColor(const CPWL_Color& color) override;
  void SetSubjectName(const CFX_WideString& sName) override;
  void SetDateTime(FX_SYSTEMTIME time) override;
  void SetContents(const CFX_WideString& sContents) override;
  int32_t CountSubItems() const override;
  IPWL_NoteItem* GetSubItems(int32_t index) const override;
  void SetFocus() override { SetNoteFocus(FALSE); }
  IPWL_NoteItem* GetParentItem() const override;
  void* GetPrivateData() const override;
  CFX_WideString GetAuthorName() const override;
  CPWL_Color GetBkColor() const override;
  CFX_WideString GetContents() const override;
  FX_SYSTEMTIME GetDateTime() const override;
  CFX_WideString GetSubjectName() const override;
  CPWL_Edit* GetEdit() const override;

 protected:
  // CPWL_Wnd
  FX_BOOL OnLButtonDown(const CFX_FloatPoint& point, uint32_t nFlag) override;
  FX_BOOL OnRButtonUp(const CFX_FloatPoint& point, uint32_t nFlag) override;
  CFX_ByteString GetClassName() const override;
  void RePosChildWnd() override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  void OnNotify(CPWL_Wnd* pWnd,
                uint32_t msg,
                intptr_t wParam = 0,
                intptr_t lParam = 0) override;
  FX_FLOAT GetItemHeight(FX_FLOAT fLimitWidth) override;
  FX_FLOAT GetItemLeftMargin() override;
  FX_FLOAT GetItemRightMargin() override;

  CPWL_NoteItem* GetParentNoteItem() const;

  void SetNoteFocus(FX_BOOL bLast);
  void PopupNoteItemMenu(const CFX_FloatPoint& point);

 protected:
  CPWL_Label* m_pSubject;
  CPWL_Label* m_pDateTime;
  CPWL_Note_Contents* m_pContents;

 private:
  void* m_pPrivateData;
  FX_SYSTEMTIME m_dtNote;
  CFX_WideString m_sAuthor;

  FX_FLOAT m_fOldItemHeight;
  FX_BOOL m_bSizeChanged;
  FX_BOOL m_bAllowModify;
};

#endif  // FPDFSDK_PDFWINDOW_PWL_NOTE_H_
