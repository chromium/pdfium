// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_combobox.h"

#include "third_party/base/ptr_util.h"
#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_comboboxproxy.h"
#include "xfa/fwl/core/ifwl_comboedit.h"
#include "xfa/fwl/core/ifwl_combolist.h"
#include "xfa/fwl/core/ifwl_formproxy.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"

IFWL_ComboBox::IFWL_ComboBox(const IFWL_App* app,
                             const CFWL_WidgetImpProperties& properties)
    : IFWL_Widget(app, properties, nullptr),
      m_pComboBoxProxy(nullptr),
      m_bLButtonDown(false),
      m_iCurSel(-1),
      m_iBtnState(CFWL_PartState_Normal),
      m_fComboFormHandler(0),
      m_bNeedShowList(false) {
  m_rtClient.Reset();
  m_rtBtn.Reset();
  m_rtHandler.Reset();

  if (m_pWidgetMgr->IsFormDisabled()) {
    DisForm_InitComboList();
    DisForm_InitComboEdit();
    return;
  }

  CFWL_WidgetImpProperties prop;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  prop.m_dwStyles |= FWL_WGTSTYLE_Border | FWL_WGTSTYLE_VScroll;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListItemIconText)
    prop.m_dwStyleExes |= FWL_STYLEEXT_LTB_Icon;

  prop.m_pDataProvider = m_pProperties->m_pDataProvider;
  m_pListBox.reset(new IFWL_ComboList(m_pOwnerApp, prop, this));
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_DropDown) && !m_pEdit) {
    CFWL_WidgetImpProperties prop2;
    m_pEdit.reset(new IFWL_ComboEdit(m_pOwnerApp, prop2, this));
    m_pEdit->SetOuter(this);
  }
  if (m_pEdit)
    m_pEdit->SetParent(this);

  SetStates(m_pProperties->m_dwStates);
}

IFWL_ComboBox::~IFWL_ComboBox() {}

FWL_Type IFWL_ComboBox::GetClassID() const {
  return FWL_Type::ComboBox;
}

FWL_Error IFWL_ComboBox::GetWidgetRect(CFX_RectF& rect, bool bAutoSize) {
  if (bAutoSize) {
    rect.Reset();
    bool bIsDropDown = IsDropDownStyle();
    if (bIsDropDown && m_pEdit) {
      m_pEdit->GetWidgetRect(rect, true);
    } else {
      rect.width = 100;
      rect.height = 16;
    }
    if (!m_pProperties->m_pThemeProvider) {
      ReSetTheme();
    }
    FX_FLOAT* pFWidth = static_cast<FX_FLOAT*>(
        GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
    if (!pFWidth)
      return FWL_Error::Indefinite;
    rect.Inflate(0, 0, *pFWidth, 0);
    IFWL_Widget::GetWidgetRect(rect, true);
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ComboBox::ModifyStylesEx(uint32_t dwStylesExAdded,
                                        uint32_t dwStylesExRemoved) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
  }
  bool bAddDropDown = !!(dwStylesExAdded & FWL_STYLEEXT_CMB_DropDown);
  bool bRemoveDropDown = !!(dwStylesExRemoved & FWL_STYLEEXT_CMB_DropDown);
  if (bAddDropDown && !m_pEdit) {
    CFWL_WidgetImpProperties prop;
    m_pEdit.reset(new IFWL_ComboEdit(m_pOwnerApp, prop, nullptr));
    m_pEdit->SetOuter(this);
    m_pEdit->SetParent(this);
  } else if (bRemoveDropDown && m_pEdit) {
    m_pEdit->SetStates(FWL_WGTSTATE_Invisible, true);
  }
  return IFWL_Widget::ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

FWL_Error IFWL_ComboBox::Update() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Update();
  }
  if (IsLocked()) {
    return FWL_Error::Indefinite;
  }
  ReSetTheme();
  bool bDropDown = IsDropDownStyle();
  if (bDropDown && m_pEdit) {
    ReSetEditAlignment();
  }
  if (!m_pProperties->m_pThemeProvider) {
    m_pProperties->m_pThemeProvider = GetAvailableTheme();
  }
  Layout();
  CFWL_ThemePart part;
  part.m_pWidget = this;
  m_fComboFormHandler =
      *static_cast<FX_FLOAT*>(m_pProperties->m_pThemeProvider->GetCapacity(
          &part, CFWL_WidgetCapacity::ComboFormHandler));
  return FWL_Error::Succeeded;
}

FWL_WidgetHit IFWL_ComboBox::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_HitTest(fx, fy);
  }
  return IFWL_Widget::HitTest(fx, fy);
}

FWL_Error IFWL_ComboBox::DrawWidget(CFX_Graphics* pGraphics,
                                    const CFX_Matrix* pMatrix) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_DrawWidget(pGraphics, pMatrix);
  }
  if (!pGraphics)
    return FWL_Error::Indefinite;
  if (!m_pProperties->m_pThemeProvider)
    return FWL_Error::Indefinite;
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  bool bIsDropDown = IsDropDownStyle();
  if (HasBorder()) {
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
  }
  if (HasEdge()) {
    DrawEdge(pGraphics, CFWL_Part::Edge, pTheme, pMatrix);
  }
  if (!bIsDropDown) {
    CFX_RectF rtTextBk(m_rtClient);
    rtTextBk.width -= m_rtBtn.width;
    CFWL_ThemeBackground param;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::Background;
    param.m_pGraphics = pGraphics;
    if (pMatrix) {
      param.m_matrix.Concat(*pMatrix);
    }
    param.m_rtPart = rtTextBk;
    if (m_iCurSel >= 0) {
      IFWL_ListBoxDP* pData = static_cast<IFWL_ListBoxDP*>(
          m_pListBox->m_pProperties->m_pDataProvider);
      void* p = pData->GetItemData(m_pListBox.get(),
                                   pData->GetItem(m_pListBox.get(), m_iCurSel));
      if (p) {
        param.m_pData = p;
      }
    }
    if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) {
      param.m_dwStates = CFWL_PartState_Disabled;
    } else if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) &&
               (m_iCurSel >= 0)) {
      param.m_dwStates = CFWL_PartState_Selected;
    } else {
      param.m_dwStates = CFWL_PartState_Normal;
    }
    pTheme->DrawBackground(&param);
    if (m_iCurSel >= 0) {
      if (!m_pListBox)
        return FWL_Error::Indefinite;
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem = pData->GetItem(this, m_iCurSel);
      m_pListBox->GetItemText(hItem, wsText);
      CFWL_ThemeText theme_text;
      theme_text.m_pWidget = this;
      theme_text.m_iPart = CFWL_Part::Caption;
      theme_text.m_dwStates = m_iBtnState;
      theme_text.m_pGraphics = pGraphics;
      theme_text.m_matrix.Concat(*pMatrix);
      theme_text.m_rtPart = rtTextBk;
      theme_text.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)
                                  ? CFWL_PartState_Selected
                                  : CFWL_PartState_Normal;
      theme_text.m_wsText = wsText;
      theme_text.m_dwTTOStyles = FDE_TTOSTYLE_SingleLine;
      theme_text.m_iTTOAlign = FDE_TTOALIGNMENT_CenterLeft;
      pTheme->DrawText(&theme_text);
    }
  }
  {
    CFWL_ThemeBackground param;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::DropDownButton;
    param.m_dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
                           ? CFWL_PartState_Disabled
                           : m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_matrix.Concat(*pMatrix);
    param.m_rtPart = m_rtBtn;
    pTheme->DrawBackground(&param);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ComboBox::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  if (!pThemeProvider)
    return FWL_Error::Indefinite;
  m_pProperties->m_pThemeProvider = pThemeProvider;
  if (m_pListBox)
    m_pListBox->SetThemeProvider(pThemeProvider);
  if (m_pEdit)
    m_pEdit->SetThemeProvider(pThemeProvider);
  return FWL_Error::Succeeded;
}

int32_t IFWL_ComboBox::GetCurSel() {
  return m_iCurSel;
}

FWL_Error IFWL_ComboBox::SetCurSel(int32_t iSel) {
  int32_t iCount = m_pListBox->CountItems();
  bool bClearSel = iSel < 0 || iSel >= iCount;
  bool bDropDown = IsDropDownStyle();
  if (bDropDown && m_pEdit) {
    if (bClearSel) {
      m_pEdit->SetText(CFX_WideString());
    } else {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem = pData->GetItem(this, iSel);
      m_pListBox->GetItemText(hItem, wsText);
      m_pEdit->SetText(wsText);
    }
    m_pEdit->Update();
  }
  m_iCurSel = bClearSel ? -1 : iSel;
  return FWL_Error::Succeeded;
}

void IFWL_ComboBox::SetStates(uint32_t dwStates, bool bSet) {
  bool bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit)
    m_pEdit->SetStates(dwStates, bSet);
  if (m_pListBox)
    m_pListBox->SetStates(dwStates, bSet);
  IFWL_Widget::SetStates(dwStates, bSet);
}

FWL_Error IFWL_ComboBox::SetEditText(const CFX_WideString& wsText) {
  if (!m_pEdit)
    return FWL_Error::Indefinite;
  m_pEdit->SetText(wsText);
  return m_pEdit->Update();
}

int32_t IFWL_ComboBox::GetEditTextLength() const {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetTextLength();
}

FWL_Error IFWL_ComboBox::GetEditText(CFX_WideString& wsText,
                                     int32_t nStart,
                                     int32_t nCount) const {
  if (m_pEdit) {
    return m_pEdit->GetText(wsText, nStart, nCount);
  } else if (m_pListBox) {
    IFWL_ComboBoxDP* pData =
        static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
    IFWL_ListItem* hItem = pData->GetItem(this, m_iCurSel);
    return m_pListBox->GetItemText(hItem, wsText);
  }
  return FWL_Error::Indefinite;
}

FWL_Error IFWL_ComboBox::SetEditSelRange(int32_t nStart, int32_t nCount) {
  if (!m_pEdit)
    return FWL_Error::Indefinite;
  m_pEdit->ClearSelected();
  m_pEdit->AddSelRange(nStart, nCount);
  return FWL_Error::Succeeded;
}

int32_t IFWL_ComboBox::GetEditSelRange(int32_t nIndex, int32_t& nStart) {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetSelRange(nIndex, nStart);
}

int32_t IFWL_ComboBox::GetEditLimit() {
  if (!m_pEdit)
    return -1;
  return m_pEdit->GetLimit();
}

FWL_Error IFWL_ComboBox::SetEditLimit(int32_t nLimit) {
  if (!m_pEdit)
    return FWL_Error::Indefinite;
  return m_pEdit->SetLimit(nLimit);
}

FWL_Error IFWL_ComboBox::EditDoClipboard(int32_t iCmd) {
  if (!m_pEdit)
    return FWL_Error::Indefinite;
  return m_pEdit->DoClipboard(iCmd);
}

bool IFWL_ComboBox::EditRedo(const IFDE_TxtEdtDoRecord* pRecord) {
  return m_pEdit && m_pEdit->Redo(pRecord);
}

bool IFWL_ComboBox::EditUndo(const IFDE_TxtEdtDoRecord* pRecord) {
  return m_pEdit && m_pEdit->Undo(pRecord);
}

IFWL_ListBox* IFWL_ComboBox::GetListBoxt() {
  return m_pListBox.get();
}

bool IFWL_ComboBox::AfterFocusShowDropList() {
  if (!m_bNeedShowList) {
    return false;
  }
  if (m_pEdit) {
    MatchEditText();
  }
  ShowDropList(true);
  m_bNeedShowList = false;
  return true;
}

FWL_Error IFWL_ComboBox::OpenDropDownList(bool bActivate) {
  ShowDropList(bActivate);
  return FWL_Error::Succeeded;
}

bool IFWL_ComboBox::EditCanUndo() {
  return m_pEdit->CanUndo();
}

bool IFWL_ComboBox::EditCanRedo() {
  return m_pEdit->CanRedo();
}

bool IFWL_ComboBox::EditUndo() {
  return m_pEdit->Undo();
}

bool IFWL_ComboBox::EditRedo() {
  return m_pEdit->Redo();
}

bool IFWL_ComboBox::EditCanCopy() {
  return m_pEdit->CountSelRanges() > 0;
}

bool IFWL_ComboBox::EditCanCut() {
  if (m_pEdit->GetStylesEx() & FWL_STYLEEXT_EDT_ReadOnly) {
    return false;
  }
  return m_pEdit->CountSelRanges() > 0;
}

bool IFWL_ComboBox::EditCanSelectAll() {
  return m_pEdit->GetTextLength() > 0;
}

bool IFWL_ComboBox::EditCopy(CFX_WideString& wsCopy) {
  return m_pEdit->Copy(wsCopy);
}

bool IFWL_ComboBox::EditCut(CFX_WideString& wsCut) {
  return m_pEdit->Cut(wsCut);
}

bool IFWL_ComboBox::EditPaste(const CFX_WideString& wsPaste) {
  return m_pEdit->Paste(wsPaste);
}

bool IFWL_ComboBox::EditSelectAll() {
  return m_pEdit->AddSelRange(0) == FWL_Error::Succeeded;
}

bool IFWL_ComboBox::EditDelete() {
  return m_pEdit->ClearText() == FWL_Error::Succeeded;
}

bool IFWL_ComboBox::EditDeSelect() {
  return m_pEdit->ClearSelections() == FWL_Error::Succeeded;
}

FWL_Error IFWL_ComboBox::GetBBox(CFX_RectF& rect) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_GetBBox(rect);
  }
  rect = m_pProperties->m_rtWidget;
  if (m_pListBox && IsDropListShowed()) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    rtList.Offset(rect.left, rect.top);
    rect.Union(rtList);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ComboBox::EditModifyStylesEx(uint32_t dwStylesExAdded,
                                            uint32_t dwStylesExRemoved) {
  if (!m_pEdit)
    return FWL_Error::ParameterInvalid;
  return m_pEdit->ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

FX_FLOAT IFWL_ComboBox::GetListHeight() {
  return static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider)
      ->GetListHeight(this);
}

void IFWL_ComboBox::DrawStretchHandler(CFX_Graphics* pGraphics,
                                       const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pGraphics = pGraphics;
  param.m_iPart = CFWL_Part::StretchHandler;
  param.m_dwStates = CFWL_PartState_Normal;
  param.m_pWidget = this;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix);
  }
  param.m_rtPart = m_rtHandler;
  m_pProperties->m_pThemeProvider->DrawBackground(&param);
}

void IFWL_ComboBox::ShowDropList(bool bActivate) {
  if (m_pWidgetMgr->IsFormDisabled())
    return DisForm_ShowDropList(bActivate);

  bool bDropList = IsDropListShowed();
  if (bDropList == bActivate)
    return;
  if (!m_pComboBoxProxy)
    InitProxyForm();

  m_pComboBoxProxy->Reset();
  if (bActivate) {
    m_pListBox->ChangeSelected(m_iCurSel);
    ReSetListItemAlignment();
    uint32_t dwStyleAdd = m_pProperties->m_dwStyleExes &
                          (FWL_STYLEEXT_CMB_Sort | FWL_STYLEEXT_CMB_OwnerDraw);
    m_pListBox->ModifyStylesEx(dwStyleAdd, 0);
    m_pListBox->GetWidgetRect(m_rtList, true);
    FX_FLOAT fHeight = GetListHeight();
    if (fHeight > 0) {
      if (m_rtList.height > GetListHeight()) {
        m_rtList.height = GetListHeight();
        m_pListBox->ModifyStyles(FWL_WGTSTYLE_VScroll, 0);
      }
    }
    CFX_RectF rtAnchor;
    rtAnchor.Set(0, 0, m_pProperties->m_rtWidget.width,
                 m_pProperties->m_rtWidget.height);
    FX_FLOAT fMinHeight = 0;
    if (m_rtList.width < m_rtClient.width) {
      m_rtList.width = m_rtClient.width;
    }
    m_rtProxy = m_rtList;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListDrag) {
      m_rtProxy.height += m_fComboFormHandler;
    }
    GetPopupPos(fMinHeight, m_rtProxy.height, rtAnchor, m_rtProxy);
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_ListDrag) {
      FX_FLOAT fx = 0;
      FX_FLOAT fy = m_rtClient.top + m_rtClient.height / 2;
      TransformTo(nullptr, fx, fy);
      m_bUpFormHandler = fy > m_rtProxy.top;
      if (m_bUpFormHandler) {
        m_rtHandler.Set(0, 0, m_rtList.width, m_fComboFormHandler);
        m_rtList.top = m_fComboFormHandler;
      } else {
        m_rtHandler.Set(0, m_rtList.height, m_rtList.width,
                        m_fComboFormHandler);
      }
    }
    m_pComboBoxProxy->SetWidgetRect(m_rtProxy);
    m_pComboBoxProxy->Update();
    m_pListBox->SetWidgetRect(m_rtList);
    m_pListBox->Update();
    CFWL_EvtCmbPreDropDown ev;
    ev.m_pSrcTarget = this;
    DispatchEvent(&ev);
    m_fItemHeight = m_pListBox->m_fItemHeight;
    m_pListBox->SetFocus(true);
    m_pComboBoxProxy->DoModal();
    m_pListBox->SetFocus(false);
  } else {
    m_pComboBoxProxy->EndDoModal();
    CFWL_EvtCmbCloseUp ev;
    ev.m_pSrcTarget = this;
    DispatchEvent(&ev);
    m_bLButtonDown = false;
    m_pListBox->m_bNotifyOwner = true;
    SetFocus(true);
  }
}

bool IFWL_ComboBox::IsDropListShowed() {
  return m_pComboBoxProxy &&
         !(m_pComboBoxProxy->GetStates() & FWL_WGTSTATE_Invisible);
}

bool IFWL_ComboBox::IsDropDownStyle() const {
  return m_pProperties->m_dwStyleExes & FWL_STYLEEXT_CMB_DropDown;
}

void IFWL_ComboBox::MatchEditText() {
  CFX_WideString wsText;
  m_pEdit->GetText(wsText);
  int32_t iMatch = m_pListBox->MatchItem(wsText);
  if (iMatch != m_iCurSel) {
    m_pListBox->ChangeSelected(iMatch);
    if (iMatch >= 0) {
      SynchrEditText(iMatch);
    }
  } else if (iMatch >= 0) {
    m_pEdit->SetSelected();
  }
  m_iCurSel = iMatch;
}

void IFWL_ComboBox::SynchrEditText(int32_t iListItem) {
  CFX_WideString wsText;
  IFWL_ComboBoxDP* pData =
      static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
  IFWL_ListItem* hItem = pData->GetItem(this, iListItem);
  m_pListBox->GetItemText(hItem, wsText);
  m_pEdit->SetText(wsText);
  m_pEdit->Update();
  m_pEdit->SetSelected();
}

void IFWL_ComboBox::Layout() {
  if (m_pWidgetMgr->IsFormDisabled()) {
    return DisForm_Layout();
  }
  GetClientRect(m_rtClient);
  FX_FLOAT* pFWidth = static_cast<FX_FLOAT*>(
      GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
  if (!pFWidth)
    return;
  FX_FLOAT fBtn = *pFWidth;
  m_rtBtn.Set(m_rtClient.right() - fBtn, m_rtClient.top, fBtn,
              m_rtClient.height);
  bool bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit) {
    CFX_RectF rtEdit;
    rtEdit.Set(m_rtClient.left, m_rtClient.top, m_rtClient.width - fBtn,
               m_rtClient.height);
    m_pEdit->SetWidgetRect(rtEdit);
    if (m_iCurSel >= 0) {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem = pData->GetItem(this, m_iCurSel);
      m_pListBox->GetItemText(hItem, wsText);
      m_pEdit->LockUpdate();
      m_pEdit->SetText(wsText);
      m_pEdit->UnlockUpdate();
    }
    m_pEdit->Update();
  }
}

void IFWL_ComboBox::ReSetTheme() {
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (!pTheme) {
    pTheme = GetAvailableTheme();
    m_pProperties->m_pThemeProvider = pTheme;
  }
  if (m_pListBox && !m_pListBox->GetThemeProvider())
    m_pListBox->SetThemeProvider(pTheme);
  if (m_pEdit && !m_pEdit->GetThemeProvider())
    m_pEdit->SetThemeProvider(pTheme);
}

void IFWL_ComboBox::ReSetEditAlignment() {
  if (!m_pEdit)
    return;
  uint32_t dwStylExes = m_pProperties->m_dwStyleExes;
  uint32_t dwAdd = 0;
  switch (dwStylExes & FWL_STYLEEXT_CMB_EditHAlignMask) {
    case FWL_STYLEEXT_CMB_EditHCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_HCenter;
      break;
    }
    case FWL_STYLEEXT_CMB_EditHFar: {
      dwAdd |= FWL_STYLEEXT_EDT_HFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_HNear; }
  }
  switch (dwStylExes & FWL_STYLEEXT_CMB_EditVAlignMask) {
    case FWL_STYLEEXT_CMB_EditVCenter: {
      dwAdd |= FWL_STYLEEXT_EDT_VCenter;
      break;
    }
    case FWL_STYLEEXT_CMB_EditVFar: {
      dwAdd |= FWL_STYLEEXT_EDT_VFar;
      break;
    }
    default: { dwAdd |= FWL_STYLEEXT_EDT_VNear; }
  }
  if (dwStylExes & FWL_STYLEEXT_CMB_EditJustified) {
    dwAdd |= FWL_STYLEEXT_EDT_Justified;
  }
  if (dwStylExes & FWL_STYLEEXT_CMB_EditDistributed) {
    dwAdd |= FWL_STYLEEXT_EDT_Distributed;
  }
  m_pEdit->ModifyStylesEx(dwAdd, FWL_STYLEEXT_EDT_HAlignMask |
                                     FWL_STYLEEXT_EDT_HAlignModeMask |
                                     FWL_STYLEEXT_EDT_VAlignMask);
}

void IFWL_ComboBox::ReSetListItemAlignment() {
  if (!m_pListBox)
    return;
  uint32_t dwStylExes = m_pProperties->m_dwStyleExes;
  uint32_t dwAdd = 0;
  switch (dwStylExes & FWL_STYLEEXT_CMB_ListItemAlignMask) {
    case FWL_STYLEEXT_CMB_ListItemCenterAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_CenterAlign;
    }
    case FWL_STYLEEXT_CMB_ListItemRightAlign: {
      dwAdd |= FWL_STYLEEXT_LTB_RightAlign;
    }
    default: { dwAdd |= FWL_STYLEEXT_LTB_LeftAlign; }
  }
  m_pListBox->ModifyStylesEx(dwAdd, FWL_STYLEEXT_CMB_ListItemAlignMask);
}

void IFWL_ComboBox::ProcessSelChanged(bool bLButtonUp) {
  IFWL_ComboBoxDP* pDatas =
      static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
  m_iCurSel = pDatas->GetItemIndex(this, m_pListBox->GetSelItem(0));
  bool bDropDown = IsDropDownStyle();
  if (bDropDown) {
    IFWL_ComboBoxDP* pData =
        static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
    IFWL_ListItem* hItem = pData->GetItem(this, m_iCurSel);
    if (hItem) {
      CFX_WideString wsText;
      pData->GetItemText(this, hItem, wsText);
      if (m_pEdit) {
        m_pEdit->SetText(wsText);
        m_pEdit->Update();
        m_pEdit->SetSelected();
      }
      CFWL_EvtCmbSelChanged ev;
      ev.bLButtonUp = bLButtonUp;
      ev.m_pSrcTarget = this;
      ev.iArraySels.Add(m_iCurSel);
      DispatchEvent(&ev);
    }
  } else {
    Repaint(&m_rtClient);
  }
}

void IFWL_ComboBox::InitProxyForm() {
  if (m_pComboBoxProxy)
    return;
  if (!m_pListBox)
    return;

  CFWL_WidgetImpProperties propForm;
  propForm.m_pOwner = this;
  propForm.m_dwStyles = FWL_WGTSTYLE_Popup;
  propForm.m_dwStates = FWL_WGTSTATE_Invisible;

  m_pComboBoxProxy =
      new IFWL_ComboBoxProxy(this, m_pOwnerApp, propForm, m_pListBox.get());
  m_pListBox->SetParent(m_pComboBoxProxy);
}

void IFWL_ComboBox::DisForm_InitComboList() {
  if (m_pListBox)
    return;

  CFWL_WidgetImpProperties prop;
  prop.m_pParent = this;
  prop.m_dwStyles = FWL_WGTSTYLE_Border | FWL_WGTSTYLE_VScroll;
  prop.m_dwStates = FWL_WGTSTATE_Invisible;
  prop.m_pDataProvider = m_pProperties->m_pDataProvider;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pListBox.reset(new IFWL_ComboList(m_pOwnerApp, prop, this));
}

void IFWL_ComboBox::DisForm_InitComboEdit() {
  if (m_pEdit)
    return;

  CFWL_WidgetImpProperties prop;
  prop.m_pParent = this;
  prop.m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pEdit.reset(new IFWL_ComboEdit(m_pOwnerApp, prop, this));
  m_pEdit->SetOuter(this);
}

void IFWL_ComboBox::DisForm_ShowDropList(bool bActivate) {
  bool bDropList = DisForm_IsDropListShowed();
  if (bDropList == bActivate) {
    return;
  }
  if (bActivate) {
    CFWL_EvtCmbPreDropDown preEvent;
    preEvent.m_pSrcTarget = this;
    DispatchEvent(&preEvent);
    IFWL_ComboList* pComboList = m_pListBox.get();
    int32_t iItems = pComboList->CountItems();
    if (iItems < 1) {
      return;
    }
    ReSetListItemAlignment();
    pComboList->ChangeSelected(m_iCurSel);
    FX_FLOAT fItemHeight = pComboList->GetItemHeigt();
    FX_FLOAT fBorder = GetBorderSize();
    FX_FLOAT fPopupMin = 0.0f;
    if (iItems > 3) {
      fPopupMin = fItemHeight * 3 + fBorder * 2;
    }
    FX_FLOAT fPopupMax = fItemHeight * iItems + fBorder * 2;
    CFX_RectF rtList;
    rtList.left = m_rtClient.left;
    rtList.width = m_pProperties->m_rtWidget.width;
    rtList.top = 0;
    rtList.height = 0;
    GetPopupPos(fPopupMin, fPopupMax, m_pProperties->m_rtWidget, rtList);
    m_pListBox->SetWidgetRect(rtList);
    m_pListBox->Update();
  } else {
    SetFocus(true);
  }
  m_pListBox->SetStates(FWL_WGTSTATE_Invisible, !bActivate);
  if (bActivate) {
    CFWL_EvtCmbPostDropDown postEvent;
    postEvent.m_pSrcTarget = this;
    DispatchEvent(&postEvent);
  }
  CFX_RectF rect;
  m_pListBox->GetWidgetRect(rect);
  rect.Inflate(2, 2);
  Repaint(&rect);
}

bool IFWL_ComboBox::DisForm_IsDropListShowed() {
  return !(m_pListBox->GetStates() & FWL_WGTSTATE_Invisible);
}

FWL_Error IFWL_ComboBox::DisForm_ModifyStylesEx(uint32_t dwStylesExAdded,
                                                uint32_t dwStylesExRemoved) {
  if (!m_pEdit)
    DisForm_InitComboEdit();

  bool bAddDropDown = !!(dwStylesExAdded & FWL_STYLEEXT_CMB_DropDown);
  bool bDelDropDown = !!(dwStylesExRemoved & FWL_STYLEEXT_CMB_DropDown);
  dwStylesExRemoved &= ~FWL_STYLEEXT_CMB_DropDown;
  m_pProperties->m_dwStyleExes |= FWL_STYLEEXT_CMB_DropDown;
  if (bAddDropDown) {
    m_pEdit->ModifyStylesEx(0, FWL_STYLEEXT_EDT_ReadOnly);
  } else if (bDelDropDown) {
    m_pEdit->ModifyStylesEx(FWL_STYLEEXT_EDT_ReadOnly, 0);
  }
  return IFWL_Widget::ModifyStylesEx(dwStylesExAdded, dwStylesExRemoved);
}

FWL_Error IFWL_ComboBox::DisForm_Update() {
  if (m_iLock) {
    return FWL_Error::Indefinite;
  }
  if (m_pEdit) {
    ReSetEditAlignment();
  }
  ReSetTheme();
  Layout();
  return FWL_Error::Succeeded;
}

FWL_WidgetHit IFWL_ComboBox::DisForm_HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rect;
  rect.Set(0, 0, m_pProperties->m_rtWidget.width - m_rtBtn.width,
           m_pProperties->m_rtWidget.height);
  if (rect.Contains(fx, fy))
    return FWL_WidgetHit::Edit;
  if (m_rtBtn.Contains(fx, fy))
    return FWL_WidgetHit::Client;
  if (DisForm_IsDropListShowed()) {
    m_pListBox->GetWidgetRect(rect);
    if (rect.Contains(fx, fy))
      return FWL_WidgetHit::Client;
  }
  return FWL_WidgetHit::Unknown;
}

FWL_Error IFWL_ComboBox::DisForm_DrawWidget(CFX_Graphics* pGraphics,
                                            const CFX_Matrix* pMatrix) {
  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  CFX_Matrix mtOrg;
  mtOrg.Set(1, 0, 0, 1, 0, 0);
  if (pMatrix) {
    mtOrg = *pMatrix;
  }
  bool bListShowed = m_pListBox && DisForm_IsDropListShowed();
  pGraphics->SaveGraphState();
  pGraphics->ConcatMatrix(&mtOrg);
  if (!m_rtBtn.IsEmpty(0.1f)) {
    CFWL_ThemeBackground param;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::DropDownButton;
    param.m_dwStates = m_iBtnState;
    param.m_pGraphics = pGraphics;
    param.m_rtPart = m_rtBtn;
    pTheme->DrawBackground(&param);
  }
  pGraphics->RestoreGraphState();
  if (m_pEdit) {
    CFX_RectF rtEdit;
    m_pEdit->GetWidgetRect(rtEdit);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtEdit.left, rtEdit.top);
    mt.Concat(mtOrg);
    m_pEdit->DrawWidget(pGraphics, &mt);
  }
  if (bListShowed) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    CFX_Matrix mt;
    mt.Set(1, 0, 0, 1, rtList.left, rtList.top);
    mt.Concat(mtOrg);
    m_pListBox->DrawWidget(pGraphics, &mt);
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_ComboBox::DisForm_GetBBox(CFX_RectF& rect) {
  rect = m_pProperties->m_rtWidget;
  if (m_pListBox && DisForm_IsDropListShowed()) {
    CFX_RectF rtList;
    m_pListBox->GetWidgetRect(rtList);
    rtList.Offset(rect.left, rect.top);
    rect.Union(rtList);
  }
  return FWL_Error::Succeeded;
}

void IFWL_ComboBox::DisForm_Layout() {
  GetClientRect(m_rtClient);
  m_rtContent = m_rtClient;
  FX_FLOAT* pFWidth = static_cast<FX_FLOAT*>(
      GetThemeCapacity(CFWL_WidgetCapacity::ScrollBarWidth));
  if (!pFWidth)
    return;
  FX_FLOAT borderWidth = 1;
  FX_FLOAT fBtn = *pFWidth;
  if (!(GetStylesEx() & FWL_STYLEEXT_CMB_ReadOnly)) {
    m_rtBtn.Set(m_rtClient.right() - fBtn, m_rtClient.top + borderWidth,
                fBtn - borderWidth, m_rtClient.height - 2 * borderWidth);
  }
  CFX_RectF* pUIMargin =
      static_cast<CFX_RectF*>(GetThemeCapacity(CFWL_WidgetCapacity::UIMargin));
  if (pUIMargin) {
    m_rtContent.Deflate(pUIMargin->left, pUIMargin->top, pUIMargin->width,
                        pUIMargin->height);
  }
  bool bIsDropDown = IsDropDownStyle();
  if (bIsDropDown && m_pEdit) {
    CFX_RectF rtEdit;
    rtEdit.Set(m_rtContent.left, m_rtContent.top, m_rtContent.width - fBtn,
               m_rtContent.height);
    m_pEdit->SetWidgetRect(rtEdit);
    if (m_iCurSel >= 0) {
      CFX_WideString wsText;
      IFWL_ComboBoxDP* pData =
          static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
      IFWL_ListItem* hItem = pData->GetItem(this, m_iCurSel);
      m_pListBox->GetItemText(hItem, wsText);
      m_pEdit->LockUpdate();
      m_pEdit->SetText(wsText);
      m_pEdit->UnlockUpdate();
    }
    m_pEdit->Update();
  }
}

void IFWL_ComboBox::OnProcessMessage(CFWL_Message* pMessage) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    DisForm_OnProcessMessage(pMessage);
    return;
  }
  if (!pMessage)
    return;

  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    case CFWL_MessageType::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_MessageType::Mouse: {
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case FWL_MouseCommand::Move:
          OnMouseMove(pMsg);
          break;
        case FWL_MouseCommand::Leave:
          OnMouseLeave(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_MessageType::Key:
      OnKey(static_cast<CFWL_MsgKey*>(pMessage));
      break;
    default:
      break;
  }

  IFWL_Widget::OnProcessMessage(pMessage);
}

void IFWL_ComboBox::OnProcessEvent(CFWL_Event* pEvent) {
  CFWL_EventType dwFlag = pEvent->GetClassID();
  if (dwFlag == CFWL_EventType::DrawItem) {
    CFWL_EvtLtbDrawItem* pDrawItemEvent =
        static_cast<CFWL_EvtLtbDrawItem*>(pEvent);
    CFWL_EvtCmbDrawItem pTemp;
    pTemp.m_pSrcTarget = this;
    pTemp.m_pGraphics = pDrawItemEvent->m_pGraphics;
    pTemp.m_index = pDrawItemEvent->m_index;
    pTemp.m_rtItem = pDrawItemEvent->m_rect;
    DispatchEvent(&pTemp);
  } else if (dwFlag == CFWL_EventType::Scroll) {
    CFWL_EvtScroll* pScrollEvent = static_cast<CFWL_EvtScroll*>(pEvent);
    CFWL_EvtScroll pScrollEv;
    pScrollEv.m_pSrcTarget = this;
    pScrollEv.m_iScrollCode = pScrollEvent->m_iScrollCode;
    pScrollEv.m_fPos = pScrollEvent->m_fPos;
    DispatchEvent(&pScrollEv);
  } else if (dwFlag == CFWL_EventType::TextChanged) {
    CFWL_EvtEdtTextChanged* pTextChangedEvent =
        static_cast<CFWL_EvtEdtTextChanged*>(pEvent);
    CFWL_EvtCmbEditChanged pTemp;
    pTemp.m_pSrcTarget = this;
    pTemp.wsInsert = pTextChangedEvent->wsInsert;
    pTemp.wsDelete = pTextChangedEvent->wsDelete;
    pTemp.nChangeType = pTextChangedEvent->nChangeType;
    DispatchEvent(&pTemp);
  }
}

void IFWL_ComboBox::OnDrawWidget(CFX_Graphics* pGraphics,
                                 const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void IFWL_ComboBox::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  IFWL_Widget* pDstTarget = pMsg->m_pDstTarget;
  IFWL_Widget* pSrcTarget = pMsg->m_pSrcTarget;
  bool bDropDown = IsDropDownStyle();
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if (bDropDown && pSrcTarget != m_pListBox.get()) {
      if (!m_pEdit)
        return;
      m_pEdit->SetSelected();
    } else {
      Repaint(&m_rtClient);
    }
  } else {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    if (bDropDown && pDstTarget != m_pListBox.get()) {
      if (!m_pEdit)
        return;
      m_pEdit->FlagFocus(false);
      m_pEdit->ClearSelected();
    } else {
      Repaint(&m_rtClient);
    }
  }
}

void IFWL_ComboBox::OnLButtonDown(CFWL_MsgMouse* pMsg) {
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
    return;

  bool bDropDown = IsDropDownStyle();
  CFX_RectF& rtBtn = bDropDown ? m_rtBtn : m_rtClient;
  bool bClickBtn = rtBtn.Contains(pMsg->m_fx, pMsg->m_fy);
  if (!bClickBtn)
    return;

  if (bDropDown && m_pEdit)
    MatchEditText();

  m_bLButtonDown = true;
  m_iBtnState = CFWL_PartState_Pressed;
  Repaint(&m_rtClient);
  ShowDropList(true);
  m_iBtnState = CFWL_PartState_Normal;
  Repaint(&m_rtClient);
}

void IFWL_ComboBox::OnLButtonUp(CFWL_MsgMouse* pMsg) {
  m_bLButtonDown = false;
  if (m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy))
    m_iBtnState = CFWL_PartState_Hovered;
  else
    m_iBtnState = CFWL_PartState_Normal;

  Repaint(&m_rtBtn);
}

void IFWL_ComboBox::OnMouseMove(CFWL_MsgMouse* pMsg) {
  int32_t iOldState = m_iBtnState;
  if (m_rtBtn.Contains(pMsg->m_fx, pMsg->m_fy)) {
    m_iBtnState =
        m_bLButtonDown ? CFWL_PartState_Pressed : CFWL_PartState_Hovered;
  } else {
    m_iBtnState = CFWL_PartState_Normal;
  }
  if ((iOldState != m_iBtnState) &&
      !((m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) ==
        FWL_WGTSTATE_Disabled)) {
    Repaint(&m_rtBtn);
  }
}

void IFWL_ComboBox::OnMouseLeave(CFWL_MsgMouse* pMsg) {
  if (!IsDropListShowed() &&
      !((m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) ==
        FWL_WGTSTATE_Disabled)) {
    m_iBtnState = CFWL_PartState_Normal;
    Repaint(&m_rtBtn);
  }
}

void IFWL_ComboBox::OnKey(CFWL_MsgKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  if (dwKeyCode == FWL_VKEY_Tab) {
    DispatchKeyEvent(pMsg);
    return;
  }
  if (pMsg->m_pDstTarget == this)
    DoSubCtrlKey(pMsg);
}

void IFWL_ComboBox::DoSubCtrlKey(CFWL_MsgKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  const bool bUp = dwKeyCode == FWL_VKEY_Up;
  const bool bDown = dwKeyCode == FWL_VKEY_Down;
  if (bUp || bDown) {
    int32_t iCount = m_pListBox->CountItems();
    if (iCount < 1)
      return;

    bool bMatchEqual = false;
    int32_t iCurSel = m_iCurSel;
    bool bDropDown = IsDropDownStyle();
    if (bDropDown && m_pEdit) {
      CFX_WideString wsText;
      m_pEdit->GetText(wsText);
      iCurSel = m_pListBox->MatchItem(wsText);
      if (iCurSel >= 0) {
        CFX_WideString wsTemp;
        IFWL_ComboBoxDP* pData =
            static_cast<IFWL_ComboBoxDP*>(m_pProperties->m_pDataProvider);
        IFWL_ListItem* hItem = pData->GetItem(this, iCurSel);
        m_pListBox->GetItemText(hItem, wsTemp);
        bMatchEqual = wsText == wsTemp;
      }
    }
    if (iCurSel < 0) {
      iCurSel = 0;
    } else if (!bDropDown || bMatchEqual) {
      if ((bUp && iCurSel == 0) || (bDown && iCurSel == iCount - 1))
        return;
      if (bUp)
        iCurSel--;
      else
        iCurSel++;
    }
    m_iCurSel = iCurSel;
    if (bDropDown && m_pEdit)
      SynchrEditText(m_iCurSel);
    else
      Repaint(&m_rtClient);
    return;
  }

  bool bDropDown = IsDropDownStyle();
  if (bDropDown)
    m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
}

void IFWL_ComboBox::DisForm_OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  bool backDefault = true;
  switch (pMessage->GetClassID()) {
    case CFWL_MessageType::SetFocus: {
      backDefault = false;
      DisForm_OnFocusChanged(pMessage, true);
      break;
    }
    case CFWL_MessageType::KillFocus: {
      backDefault = false;
      DisForm_OnFocusChanged(pMessage, false);
      break;
    }
    case CFWL_MessageType::Mouse: {
      backDefault = false;
      CFWL_MsgMouse* pMsg = static_cast<CFWL_MsgMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          DisForm_OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_MessageType::Key: {
      backDefault = false;
      CFWL_MsgKey* pKey = static_cast<CFWL_MsgKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_KeyCommand::KeyUp)
        break;
      if (DisForm_IsDropListShowed() &&
          pKey->m_dwCmd == FWL_KeyCommand::KeyDown) {
        bool bListKey = pKey->m_dwKeyCode == FWL_VKEY_Up ||
                        pKey->m_dwKeyCode == FWL_VKEY_Down ||
                        pKey->m_dwKeyCode == FWL_VKEY_Return ||
                        pKey->m_dwKeyCode == FWL_VKEY_Escape;
        if (bListKey) {
          m_pListBox->GetDelegate()->OnProcessMessage(pMessage);
          break;
        }
      }
      DisForm_OnKey(pKey);
      break;
    }
    default:
      break;
  }
  if (backDefault)
    IFWL_Widget::OnProcessMessage(pMessage);
}

void IFWL_ComboBox::DisForm_OnLButtonDown(CFWL_MsgMouse* pMsg) {
  bool bDropDown = DisForm_IsDropListShowed();
  CFX_RectF& rtBtn = bDropDown ? m_rtBtn : m_rtClient;
  if (!rtBtn.Contains(pMsg->m_fx, pMsg->m_fy))
    return;

  if (DisForm_IsDropListShowed()) {
    DisForm_ShowDropList(false);
    return;
  }
  if (m_pEdit)
    MatchEditText();
  DisForm_ShowDropList(true);
}

void IFWL_ComboBox::DisForm_OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;
    if ((m_pEdit->GetStates() & FWL_WGTSTATE_Focused) == 0) {
      CFWL_MsgSetFocus msg;
      msg.m_pDstTarget = m_pEdit.get();
      msg.m_pSrcTarget = nullptr;
      m_pEdit->GetDelegate()->OnProcessMessage(&msg);
    }
  } else {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    DisForm_ShowDropList(false);
    CFWL_MsgKillFocus msg;
    msg.m_pDstTarget = nullptr;
    msg.m_pSrcTarget = m_pEdit.get();
    m_pEdit->GetDelegate()->OnProcessMessage(&msg);
  }
}

void IFWL_ComboBox::DisForm_OnKey(CFWL_MsgKey* pMsg) {
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  const bool bUp = dwKeyCode == FWL_VKEY_Up;
  const bool bDown = dwKeyCode == FWL_VKEY_Down;
  if (bUp || bDown) {
    IFWL_ComboList* pComboList = m_pListBox.get();
    int32_t iCount = pComboList->CountItems();
    if (iCount < 1)
      return;

    bool bMatchEqual = false;
    int32_t iCurSel = m_iCurSel;
    if (m_pEdit) {
      CFX_WideString wsText;
      m_pEdit->GetText(wsText);
      iCurSel = pComboList->MatchItem(wsText);
      if (iCurSel >= 0) {
        CFX_WideString wsTemp;
        IFWL_ListItem* item = m_pListBox->GetSelItem(iCurSel);
        m_pListBox->GetItemText(item, wsTemp);
        bMatchEqual = wsText == wsTemp;
      }
    }
    if (iCurSel < 0) {
      iCurSel = 0;
    } else if (bMatchEqual) {
      if ((bUp && iCurSel == 0) || (bDown && iCurSel == iCount - 1))
        return;
      if (bUp)
        iCurSel--;
      else
        iCurSel++;
    }
    m_iCurSel = iCurSel;
    SynchrEditText(m_iCurSel);
    return;
  }
  if (m_pEdit)
    m_pEdit->GetDelegate()->OnProcessMessage(pMsg);
}
