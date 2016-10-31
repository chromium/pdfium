// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/ifwl_widget.h"

#include <algorithm>

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_message.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/fwl_noteimp.h"
#include "xfa/fwl/core/ifwl_app.h"
#include "xfa/fwl/core/ifwl_combobox.h"
#include "xfa/fwl/core/ifwl_form.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"
#include "xfa/fwl/core/ifwl_widget.h"
#include "xfa/fxfa/xfa_ffapp.h"

#define FWL_STYLEEXT_MNU_Vert (1L << 0)

IFWL_Widget::IFWL_Widget(const IFWL_App* app,
                         const CFWL_WidgetImpProperties& properties,
                         IFWL_Widget* pOuter)
    : m_pOwnerApp(app),
      m_pWidgetMgr(app->GetWidgetMgr()),
      m_pProperties(new CFWL_WidgetImpProperties(properties)),
      m_pDelegate(nullptr),
      m_pCurDelegate(nullptr),
      m_pOuter(pOuter),
      m_pLayoutItem(nullptr),
      m_pAssociate(nullptr),
      m_iLock(0),
      m_nEventKey(0) {
  ASSERT(m_pWidgetMgr);
}

IFWL_Widget::~IFWL_Widget() {}

void IFWL_Widget::Initialize() {
  IFWL_Widget* pParent = m_pProperties->m_pParent;
  m_pWidgetMgr->InsertWidget(pParent, this);
  if (!IsChild()) {
    IFWL_Widget* pOwner = m_pProperties->m_pOwner;
    if (pOwner)
      m_pWidgetMgr->SetOwner(pOwner, this);
  }
}

void IFWL_Widget::Finalize() {
  NotifyDriver();
  m_pWidgetMgr->RemoveWidget(this);
}

FX_BOOL IFWL_Widget::IsInstance(const CFX_WideStringC& wsClass) const {
  return FALSE;
}

FWL_Error IFWL_Widget::GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize) {
  if (bAutoSize) {
    if (HasEdge()) {
      FX_FLOAT fEdge = GetEdgeWidth();
      rect.Inflate(fEdge, fEdge);
    }
    if (HasBorder()) {
      FX_FLOAT fBorder = GetBorderSize();
      rect.Inflate(fBorder, fBorder);
    }
  } else {
    rect = m_pProperties->m_rtWidget;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::GetGlobalRect(CFX_RectF& rect) {
  IFWL_Widget* pForm = m_pWidgetMgr->GetSystemFormWidget(this);
  if (!pForm)
    return FWL_Error::Indefinite;

  rect.Set(0, 0, m_pProperties->m_rtWidget.width,
           m_pProperties->m_rtWidget.height);
  if (pForm == this)
    return FWL_Error::Succeeded;

  return TransformTo(pForm, rect);
}

FWL_Error IFWL_Widget::SetWidgetRect(const CFX_RectF& rect) {
  CFX_RectF rtOld = m_pProperties->m_rtWidget;
  m_pProperties->m_rtWidget = rect;
  if (IsChild()) {
    if (FXSYS_fabs(rtOld.width - rect.width) > 0.5f ||
        FXSYS_fabs(rtOld.height - rect.height) > 0.5f) {
      CFWL_EvtSizeChanged ev;
      ev.m_pSrcTarget = this;
      ev.m_rtOld = rtOld;
      ev.m_rtNew = rect;
      IFWL_WidgetDelegate* pDelegate = SetDelegate(nullptr);
      if (pDelegate) {
        pDelegate->OnProcessEvent(&ev);
      }
    }
    return FWL_Error::Succeeded;
  }
  m_pWidgetMgr->SetWidgetRect_Native(this, rect);
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::GetClientRect(CFX_RectF& rect) {
  GetEdgeRect(rect);
  if (HasEdge()) {
    FX_FLOAT fEdge = GetEdgeWidth();
    rect.Deflate(fEdge, fEdge);
  }
  return FWL_Error::Succeeded;
}

IFWL_Widget* IFWL_Widget::GetParent() {
  return m_pWidgetMgr->GetParentWidget(this);
}

FWL_Error IFWL_Widget::SetParent(IFWL_Widget* pParent) {
  m_pProperties->m_pParent = pParent;
  m_pWidgetMgr->SetParent(pParent, this);
  return FWL_Error::Succeeded;
}

IFWL_Widget* IFWL_Widget::GetOwner() {
  return m_pWidgetMgr->GetOwnerWidget(this);
}

FWL_Error IFWL_Widget::SetOwner(IFWL_Widget* pOwner) {
  m_pProperties->m_pOwner = pOwner;
  m_pWidgetMgr->SetOwner(pOwner, this);
  return FWL_Error::Succeeded;
}

IFWL_Widget* IFWL_Widget::GetOuter() {
  return m_pOuter;
}

uint32_t IFWL_Widget::GetStyles() {
  return m_pProperties->m_dwStyles;
}

FWL_Error IFWL_Widget::ModifyStyles(uint32_t dwStylesAdded,
                                    uint32_t dwStylesRemoved) {
  m_pProperties->m_dwStyles =
      (m_pProperties->m_dwStyles & ~dwStylesRemoved) | dwStylesAdded;
  return FWL_Error::Succeeded;
}

uint32_t IFWL_Widget::GetStylesEx() {
  return m_pProperties->m_dwStyleExes;
}

FWL_Error IFWL_Widget::ModifyStylesEx(uint32_t dwStylesExAdded,
                                      uint32_t dwStylesExRemoved) {
  m_pProperties->m_dwStyleExes =
      (m_pProperties->m_dwStyleExes & ~dwStylesExRemoved) | dwStylesExAdded;
  return FWL_Error::Succeeded;
}

uint32_t IFWL_Widget::GetStates() {
  return m_pProperties->m_dwStates;
}

static void NotifyHideChildWidget(CFWL_WidgetMgr* widgetMgr,
                                  IFWL_Widget* widget,
                                  CFWL_NoteDriver* noteDriver) {
  IFWL_Widget* child = widgetMgr->GetFirstChildWidget(widget);
  while (child) {
    noteDriver->NotifyTargetHide(child);
    NotifyHideChildWidget(widgetMgr, child, noteDriver);
    child = widgetMgr->GetNextSiblingWidget(child);
  }
}

void IFWL_Widget::SetStates(uint32_t dwStates, FX_BOOL bSet) {
  bSet ? (m_pProperties->m_dwStates |= dwStates)
       : (m_pProperties->m_dwStates &= ~dwStates);
  if (!(dwStates & FWL_WGTSTATE_Invisible) || !bSet)
    return;

  CFWL_NoteDriver* noteDriver =
      static_cast<CFWL_NoteDriver*>(GetOwnerApp()->GetNoteDriver());
  CFWL_WidgetMgr* widgetMgr = GetOwnerApp()->GetWidgetMgr();
  noteDriver->NotifyTargetHide(this);
  IFWL_Widget* child = widgetMgr->GetFirstChildWidget(this);
  while (child) {
    noteDriver->NotifyTargetHide(child);
    NotifyHideChildWidget(widgetMgr, child, noteDriver);
    child = widgetMgr->GetNextSiblingWidget(child);
  }
  return;
}

FWL_Error IFWL_Widget::Update() {
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::LockUpdate() {
  m_iLock++;
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::UnlockUpdate() {
  if (IsLocked()) {
    m_iLock--;
  }
  return FWL_Error::Succeeded;
}

FWL_WidgetHit IFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
  CFX_RectF rtClient;
  GetClientRect(rtClient);
  if (rtClient.Contains(fx, fy))
    return FWL_WidgetHit::Client;
  if (HasEdge()) {
    CFX_RectF rtEdge;
    GetEdgeRect(rtEdge);
    if (rtEdge.Contains(fx, fy))
      return FWL_WidgetHit::Edge;
  }
  if (HasBorder()) {
    CFX_RectF rtRelative;
    GetRelativeRect(rtRelative);
    if (rtRelative.Contains(fx, fy))
      return FWL_WidgetHit::Border;
  }
  return FWL_WidgetHit::Unknown;
}

FWL_Error IFWL_Widget::TransformTo(IFWL_Widget* pWidget,
                                   FX_FLOAT& fx,
                                   FX_FLOAT& fy) {
  if (m_pWidgetMgr->IsFormDisabled()) {
    CFX_SizeF szOffset;
    if (IsParent(pWidget)) {
      szOffset = GetOffsetFromParent(pWidget);
    } else {
      szOffset = pWidget->GetOffsetFromParent(this);
      szOffset.x = -szOffset.x;
      szOffset.y = -szOffset.y;
    }
    fx += szOffset.x;
    fy += szOffset.y;
    return FWL_Error::Succeeded;
  }
  CFX_RectF r;
  CFX_Matrix m;
  IFWL_Widget* parent = GetParent();
  if (parent) {
    GetWidgetRect(r);
    fx += r.left;
    fy += r.top;
    GetMatrix(m, TRUE);
    m.TransformPoint(fx, fy);
  }
  IFWL_Widget* form1 = m_pWidgetMgr->GetSystemFormWidget(this);
  if (!form1)
    return FWL_Error::Indefinite;
  if (!pWidget) {
    form1->GetWidgetRect(r);
    fx += r.left;
    fy += r.top;
    return FWL_Error::Succeeded;
  }
  IFWL_Widget* form2 = m_pWidgetMgr->GetSystemFormWidget(pWidget);
  if (!form2)
    return FWL_Error::Indefinite;
  if (form1 != form2) {
    form1->GetWidgetRect(r);
    fx += r.left;
    fy += r.top;
    form2->GetWidgetRect(r);
    fx -= r.left;
    fy -= r.top;
  }
  parent = pWidget->GetParent();
  if (parent) {
    pWidget->GetMatrix(m, TRUE);
    CFX_Matrix m1;
    m1.SetIdentity();
    m1.SetReverse(m);
    m1.TransformPoint(fx, fy);
    pWidget->GetWidgetRect(r);
    fx -= r.left;
    fy -= r.top;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::TransformTo(IFWL_Widget* pWidget, CFX_RectF& rt) {
  return TransformTo(pWidget, rt.left, rt.top);
}

FWL_Error IFWL_Widget::GetMatrix(CFX_Matrix& matrix, FX_BOOL bGlobal) {
  if (!m_pProperties)
    return FWL_Error::Indefinite;
  if (bGlobal) {
    IFWL_Widget* parent = GetParent();
    CFX_ArrayTemplate<IFWL_Widget*> parents;
    while (parent) {
      parents.Add(parent);
      parent = parent->GetParent();
    }
    matrix.SetIdentity();
    CFX_Matrix ctmOnParent;
    CFX_RectF rect;
    int32_t count = parents.GetSize();
    for (int32_t i = count - 2; i >= 0; i--) {
      parent = parents.GetAt(i);
      parent->GetMatrix(ctmOnParent, FALSE);
      parent->GetWidgetRect(rect);
      matrix.Concat(ctmOnParent, TRUE);
      matrix.Translate(rect.left, rect.top, TRUE);
    }
    matrix.Concat(m_pProperties->m_ctmOnParent, TRUE);
    parents.RemoveAll();
  } else {
    matrix = m_pProperties->m_ctmOnParent;
  }
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::SetMatrix(const CFX_Matrix& matrix) {
  if (!m_pProperties)
    return FWL_Error::Indefinite;
  IFWL_Widget* parent = GetParent();
  if (!parent) {
    return FWL_Error::Indefinite;
  }
  m_pProperties->m_ctmOnParent = matrix;
  return FWL_Error::Succeeded;
}

FWL_Error IFWL_Widget::DrawWidget(CFX_Graphics* pGraphics,
                                  const CFX_Matrix* pMatrix) {
  return FWL_Error::Indefinite;
}

IFWL_ThemeProvider* IFWL_Widget::GetThemeProvider() {
  return m_pProperties->m_pThemeProvider;
}
FWL_Error IFWL_Widget::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  m_pProperties->m_pThemeProvider = pThemeProvider;
  return FWL_Error::Succeeded;
}

IFWL_WidgetDelegate* IFWL_Widget::SetDelegate(IFWL_WidgetDelegate* pDelegate) {
  if (!m_pCurDelegate) {
    m_pCurDelegate = m_pDelegate;
  }
  if (!pDelegate) {
    return m_pCurDelegate;
  }
  IFWL_WidgetDelegate* pOldDelegate = m_pCurDelegate;
  m_pCurDelegate = pDelegate;
  return pOldDelegate;
}

const IFWL_App* IFWL_Widget::GetOwnerApp() const {
  return m_pOwnerApp;
}

uint32_t IFWL_Widget::GetEventKey() const {
  return m_nEventKey;
}

void IFWL_Widget::SetEventKey(uint32_t key) {
  m_nEventKey = key;
}

void* IFWL_Widget::GetLayoutItem() const {
  return m_pLayoutItem;
}

void IFWL_Widget::SetLayoutItem(void* pItem) {
  m_pLayoutItem = pItem;
}

void IFWL_Widget::SetAssociateWidget(CFWL_Widget* pAssociate) {
  m_pAssociate = pAssociate;
}
FX_BOOL IFWL_Widget::IsEnabled() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) == 0;
}

FX_BOOL IFWL_Widget::IsVisible() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible) == 0;
}

FX_BOOL IFWL_Widget::IsActive() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) == 0;
}

FX_BOOL IFWL_Widget::IsOverLapper() const {
  return (m_pProperties->m_dwStyles & FWL_WGTSTYLE_WindowTypeMask) ==
         FWL_WGTSTYLE_OverLapper;
}

FX_BOOL IFWL_Widget::IsPopup() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Popup);
}

FX_BOOL IFWL_Widget::IsChild() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Child);
}

FX_BOOL IFWL_Widget::IsLocked() const {
  return m_iLock > 0;
}

FX_BOOL IFWL_Widget::IsOffscreen() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Offscreen);
}

FX_BOOL IFWL_Widget::HasBorder() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border);
}

FX_BOOL IFWL_Widget::HasEdge() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_EdgeMask);
}

void IFWL_Widget::GetEdgeRect(CFX_RectF& rtEdge) {
  rtEdge = m_pProperties->m_rtWidget;
  rtEdge.left = rtEdge.top = 0;
  if (HasBorder()) {
    FX_FLOAT fCX = GetBorderSize();
    FX_FLOAT fCY = GetBorderSize(FALSE);
    rtEdge.Deflate(fCX, fCY);
  }
}

FX_FLOAT IFWL_Widget::GetBorderSize(FX_BOOL bCX) {
  FX_FLOAT* pfBorder = static_cast<FX_FLOAT*>(GetThemeCapacity(
      bCX ? CFWL_WidgetCapacity::CXBorder : CFWL_WidgetCapacity::CYBorder));
  if (!pfBorder)
    return 0;
  return *pfBorder;
}

FX_FLOAT IFWL_Widget::GetEdgeWidth() {
  CFWL_WidgetCapacity dwCapacity = CFWL_WidgetCapacity::None;
  switch (m_pProperties->m_dwStyles & FWL_WGTSTYLE_EdgeMask) {
    case FWL_WGTSTYLE_EdgeFlat: {
      dwCapacity = CFWL_WidgetCapacity::EdgeFlat;
      break;
    }
    case FWL_WGTSTYLE_EdgeRaised: {
      dwCapacity = CFWL_WidgetCapacity::EdgeRaised;
      break;
    }
    case FWL_WGTSTYLE_EdgeSunken: {
      dwCapacity = CFWL_WidgetCapacity::EdgeSunken;
      break;
    }
  }
  if (dwCapacity != CFWL_WidgetCapacity::None) {
    FX_FLOAT* fRet = static_cast<FX_FLOAT*>(GetThemeCapacity(dwCapacity));
    return fRet ? *fRet : 0;
  }
  return 0;
}

void IFWL_Widget::GetRelativeRect(CFX_RectF& rect) {
  rect = m_pProperties->m_rtWidget;
  rect.left = rect.top = 0;
}

void* IFWL_Widget::GetThemeCapacity(CFWL_WidgetCapacity dwCapacity) {
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (!pTheme)
    return nullptr;
  CFWL_ThemePart part;
  part.m_pWidget = this;
  return pTheme->GetCapacity(&part, dwCapacity);
}

IFWL_ThemeProvider* IFWL_Widget::GetAvailableTheme() {
  if (m_pProperties->m_pThemeProvider) {
    return m_pProperties->m_pThemeProvider;
  }
  IFWL_Widget* pUp = this;
  do {
    pUp = (pUp->GetStyles() & FWL_WGTSTYLE_Popup)
              ? m_pWidgetMgr->GetOwnerWidget(pUp)
              : m_pWidgetMgr->GetParentWidget(pUp);
    if (pUp) {
      IFWL_ThemeProvider* pRet = pUp->GetThemeProvider();
      if (pRet)
        return pRet;
    }
  } while (pUp);
  return nullptr;
}

IFWL_Widget* IFWL_Widget::GetRootOuter() {
  IFWL_Widget* pRet = m_pOuter;
  if (!pRet)
    return nullptr;
  while (IFWL_Widget* pOuter = pRet->GetOuter()) {
    pRet = pOuter;
  }
  return pRet;
}

#define FWL_WGT_CalcHeight 2048
#define FWL_WGT_CalcWidth 2048
#define FWL_WGT_CalcMultiLineDefWidth 120.0f

CFX_SizeF IFWL_Widget::CalcTextSize(const CFX_WideString& wsText,
                                    IFWL_ThemeProvider* pTheme,
                                    FX_BOOL bMultiLine,
                                    int32_t iLineWidth) {
  if (!pTheme)
    return CFX_SizeF();

  CFWL_ThemeText calPart;
  calPart.m_pWidget = this;
  calPart.m_wsText = wsText;
  calPart.m_dwTTOStyles =
      bMultiLine ? FDE_TTOSTYLE_LineWrap : FDE_TTOSTYLE_SingleLine;
  calPart.m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
  CFX_RectF rect;
  FX_FLOAT fWidth = bMultiLine
                        ? (iLineWidth > 0 ? (FX_FLOAT)iLineWidth
                                          : FWL_WGT_CalcMultiLineDefWidth)
                        : FWL_WGT_CalcWidth;
  rect.Set(0, 0, fWidth, FWL_WGT_CalcHeight);
  pTheme->CalcTextRect(&calPart, rect);
  return CFX_SizeF(rect.width, rect.height);
}

void IFWL_Widget::CalcTextRect(const CFX_WideString& wsText,
                               IFWL_ThemeProvider* pTheme,
                               uint32_t dwTTOStyles,
                               int32_t iTTOAlign,
                               CFX_RectF& rect) {
  CFWL_ThemeText calPart;
  calPart.m_pWidget = this;
  calPart.m_wsText = wsText;
  calPart.m_dwTTOStyles = dwTTOStyles;
  calPart.m_iTTOAlign = iTTOAlign;
  pTheme->CalcTextRect(&calPart, rect);
}

void IFWL_Widget::SetFocus(FX_BOOL bFocus) {
  if (m_pWidgetMgr->IsFormDisabled())
    return;

  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  if (!pDriver)
    return;

  IFWL_Widget* curFocus = pDriver->GetFocus();
  if (bFocus && curFocus != this) {
    pDriver->SetFocus(this);
  } else if (!bFocus && curFocus == this) {
    pDriver->SetFocus(nullptr);
  }
}

void IFWL_Widget::SetGrab(FX_BOOL bSet) {
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;
  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  pDriver->SetGrab(this, bSet);
}

FX_BOOL IFWL_Widget::GetPopupPos(FX_FLOAT fMinHeight,
                                 FX_FLOAT fMaxHeight,
                                 const CFX_RectF& rtAnchor,
                                 CFX_RectF& rtPopup) {
  if (GetClassID() == FWL_Type::ComboBox) {
    if (m_pWidgetMgr->IsFormDisabled()) {
      return m_pWidgetMgr->GetAdapterPopupPos(this, fMinHeight, fMaxHeight,
                                              rtAnchor, rtPopup);
    }
    return GetPopupPosComboBox(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
  }
  if (GetClassID() == FWL_Type::DateTimePicker &&
      m_pWidgetMgr->IsFormDisabled()) {
    return m_pWidgetMgr->GetAdapterPopupPos(this, fMinHeight, fMaxHeight,
                                            rtAnchor, rtPopup);
  }
  return GetPopupPosGeneral(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
}

FX_BOOL IFWL_Widget::GetPopupPosMenu(FX_FLOAT fMinHeight,
                                     FX_FLOAT fMaxHeight,
                                     const CFX_RectF& rtAnchor,
                                     CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;
  FX_FLOAT fScreenWidth = 0;
  FX_FLOAT fScreenHeight = 0;
  GetScreenSize(fScreenWidth, fScreenHeight);
  if (GetStylesEx() & FWL_STYLEEXT_MNU_Vert) {
    FX_BOOL bLeft = m_pProperties->m_rtWidget.left < 0;
    FX_FLOAT fRight = rtAnchor.right() + rtPopup.width;
    TransformTo(nullptr, fx, fy);
    if (fRight + fx > fScreenWidth || bLeft) {
      rtPopup.Set(rtAnchor.left - rtPopup.width, rtAnchor.top, rtPopup.width,
                  rtPopup.height);
    } else {
      rtPopup.Set(rtAnchor.right(), rtAnchor.top, rtPopup.width,
                  rtPopup.height);
    }
  } else {
    FX_FLOAT fBottom = rtAnchor.bottom() + rtPopup.height;
    TransformTo(nullptr, fx, fy);
    if (fBottom + fy > fScreenHeight) {
      rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width,
                  rtPopup.height);
    } else {
      rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width,
                  rtPopup.height);
    }
  }
  rtPopup.Offset(fx, fy);
  return TRUE;
}

FX_BOOL IFWL_Widget::GetPopupPosComboBox(FX_FLOAT fMinHeight,
                                         FX_FLOAT fMaxHeight,
                                         const CFX_RectF& rtAnchor,
                                         CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;
  FX_FLOAT fScreenWidth = 0;
  FX_FLOAT fScreenHeight = 0;
  GetScreenSize(fScreenWidth, fScreenHeight);
  FX_FLOAT fPopHeight = rtPopup.height;
  if (rtPopup.height > fMaxHeight) {
    fPopHeight = fMaxHeight;
  } else if (rtPopup.height < fMinHeight) {
    fPopHeight = fMinHeight;
  }
  FX_FLOAT fWidth = std::max(rtAnchor.width, rtPopup.width);
  FX_FLOAT fBottom = rtAnchor.bottom() + fPopHeight;
  TransformTo(nullptr, fx, fy);
  if (fBottom + fy > fScreenHeight) {
    rtPopup.Set(rtAnchor.left, rtAnchor.top - fPopHeight, fWidth, fPopHeight);
  } else {
    rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), fWidth, fPopHeight);
  }
  rtPopup.Offset(fx, fy);
  return TRUE;
}

FX_BOOL IFWL_Widget::GetPopupPosGeneral(FX_FLOAT fMinHeight,
                                        FX_FLOAT fMaxHeight,
                                        const CFX_RectF& rtAnchor,
                                        CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;
  FX_FLOAT fScreenWidth = 0;
  FX_FLOAT fScreenHeight = 0;
  GetScreenSize(fScreenWidth, fScreenHeight);
  TransformTo(nullptr, fx, fy);
  if (rtAnchor.bottom() + fy > fScreenHeight) {
    rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width,
                rtPopup.height);
  } else {
    rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width,
                rtPopup.height);
  }
  rtPopup.Offset(fx, fy);
  return TRUE;
}

FX_BOOL IFWL_Widget::GetScreenSize(FX_FLOAT& fx, FX_FLOAT& fy) {
  return FALSE;
}

void IFWL_Widget::RegisterEventTarget(IFWL_Widget* pEventSource,
                                      uint32_t dwFilter) {
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;

  pNoteDriver->RegisterEventTarget(this, pEventSource, dwFilter);
}

void IFWL_Widget::UnregisterEventTarget() {
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;

  pNoteDriver->UnregisterEventTarget(this);
}

void IFWL_Widget::DispatchKeyEvent(CFWL_MsgKey* pNote) {
  if (!pNote)
    return;
  CFWL_EvtKey* pEvent = new CFWL_EvtKey;
  pEvent->m_pSrcTarget = this;
  pEvent->m_dwCmd = pNote->m_dwCmd;
  pEvent->m_dwKeyCode = pNote->m_dwKeyCode;
  pEvent->m_dwFlags = pNote->m_dwFlags;
  DispatchEvent(pEvent);
  pEvent->Release();
}

void IFWL_Widget::DispatchEvent(CFWL_Event* pEvent) {
  if (m_pOuter) {
    IFWL_WidgetDelegate* pDelegate = m_pOuter->SetDelegate(nullptr);
    pDelegate->OnProcessEvent(pEvent);
    return;
  }
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;
  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;
  pNoteDriver->SendEvent(pEvent);
}

void IFWL_Widget::Repaint(const CFX_RectF* pRect) {
  if (pRect) {
    m_pWidgetMgr->RepaintWidget(this, pRect);
    return;
  }
  CFX_RectF rect;
  rect = m_pProperties->m_rtWidget;
  rect.left = rect.top = 0;
  m_pWidgetMgr->RepaintWidget(this, &rect);
}

void IFWL_Widget::DrawBackground(CFX_Graphics* pGraphics,
                                 CFWL_Part iPartBk,
                                 IFWL_ThemeProvider* pTheme,
                                 const CFX_Matrix* pMatrix) {
  CFX_RectF rtRelative;
  GetRelativeRect(rtRelative);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartBk;
  param.m_pGraphics = pGraphics;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix, TRUE);
  }
  param.m_rtPart = rtRelative;
  pTheme->DrawBackground(&param);
}

void IFWL_Widget::DrawBorder(CFX_Graphics* pGraphics,
                             CFWL_Part iPartBorder,
                             IFWL_ThemeProvider* pTheme,
                             const CFX_Matrix* pMatrix) {
  CFX_RectF rtRelative;
  GetRelativeRect(rtRelative);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartBorder;
  param.m_pGraphics = pGraphics;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix, TRUE);
  }
  param.m_rtPart = rtRelative;
  pTheme->DrawBackground(&param);
}

void IFWL_Widget::DrawEdge(CFX_Graphics* pGraphics,
                           CFWL_Part iPartEdge,
                           IFWL_ThemeProvider* pTheme,
                           const CFX_Matrix* pMatrix) {
  CFX_RectF rtEdge;
  GetEdgeRect(rtEdge);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartEdge;
  param.m_pGraphics = pGraphics;
  if (pMatrix) {
    param.m_matrix.Concat(*pMatrix, TRUE);
  }
  param.m_rtPart = rtEdge;
  pTheme->DrawBackground(&param);
}

void IFWL_Widget::NotifyDriver() {
  const IFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  if (!pDriver)
    return;

  pDriver->NotifyTargetDestroy(this);
}

CFX_SizeF IFWL_Widget::GetOffsetFromParent(IFWL_Widget* pParent) {
  if (pParent == this)
    return CFX_SizeF();

  CFWL_WidgetMgr* pWidgetMgr = GetOwnerApp()->GetWidgetMgr();
  if (!pWidgetMgr)
    return CFX_SizeF();

  CFX_SizeF szRet(m_pProperties->m_rtWidget.left,
                  m_pProperties->m_rtWidget.top);

  IFWL_Widget* pDstWidget = GetParent();
  while (pDstWidget && pDstWidget != pParent) {
    CFX_RectF rtDst;
    pDstWidget->GetWidgetRect(rtDst);
    szRet += CFX_SizeF(rtDst.left, rtDst.top);
    pDstWidget = pWidgetMgr->GetParentWidget(pDstWidget);
  }
  return szRet;
}

FX_BOOL IFWL_Widget::IsParent(IFWL_Widget* pParent) {
  IFWL_Widget* pUpWidget = GetParent();
  while (pUpWidget) {
    if (pUpWidget == pParent)
      return TRUE;
    pUpWidget = pUpWidget->GetParent();
  }
  return FALSE;
}

CFWL_WidgetImpDelegate::CFWL_WidgetImpDelegate() {}

void CFWL_WidgetImpDelegate::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage->m_pDstTarget)
    return;

  IFWL_Widget* pWidget = pMessage->m_pDstTarget;
  CFWL_MessageType dwMsgCode = pMessage->GetClassID();
  switch (dwMsgCode) {
    case CFWL_MessageType::Mouse: {
      CFWL_MsgMouse* pMsgMouse = static_cast<CFWL_MsgMouse*>(pMessage);
      CFWL_EvtMouse evt;
      evt.m_pSrcTarget = pWidget;
      evt.m_pDstTarget = pWidget;
      evt.m_dwCmd = pMsgMouse->m_dwCmd;
      evt.m_dwFlags = pMsgMouse->m_dwFlags;
      evt.m_fx = pMsgMouse->m_fx;
      evt.m_fy = pMsgMouse->m_fy;
      pWidget->DispatchEvent(&evt);
      break;
    }
    case CFWL_MessageType::MouseWheel: {
      CFWL_MsgMouseWheel* pMsgMouseWheel =
          static_cast<CFWL_MsgMouseWheel*>(pMessage);
      CFWL_EvtMouseWheel evt;
      evt.m_pSrcTarget = pWidget;
      evt.m_pDstTarget = pWidget;
      evt.m_dwFlags = pMsgMouseWheel->m_dwFlags;
      evt.m_fDeltaX = pMsgMouseWheel->m_fDeltaX;
      evt.m_fDeltaY = pMsgMouseWheel->m_fDeltaY;
      evt.m_fx = pMsgMouseWheel->m_fx;
      evt.m_fy = pMsgMouseWheel->m_fy;
      pWidget->DispatchEvent(&evt);
      break;
    }
    case CFWL_MessageType::Key: {
      CFWL_MsgKey* pMsgKey = static_cast<CFWL_MsgKey*>(pMessage);
      CFWL_EvtKey evt;
      evt.m_pSrcTarget = pWidget;
      evt.m_pDstTarget = pWidget;
      evt.m_dwKeyCode = pMsgKey->m_dwKeyCode;
      evt.m_dwFlags = pMsgKey->m_dwFlags;
      evt.m_dwCmd = pMsgKey->m_dwCmd;
      pWidget->DispatchEvent(&evt);
      break;
    }
    case CFWL_MessageType::SetFocus: {
      CFWL_MsgSetFocus* pMsgSetFocus = static_cast<CFWL_MsgSetFocus*>(pMessage);
      CFWL_EvtSetFocus evt;
      evt.m_pSrcTarget = pMsgSetFocus->m_pDstTarget;
      evt.m_pDstTarget = pMsgSetFocus->m_pDstTarget;
      evt.m_pSetFocus = pWidget;
      pWidget->DispatchEvent(&evt);
      break;
    }
    case CFWL_MessageType::KillFocus: {
      CFWL_MsgKillFocus* pMsgKillFocus =
          static_cast<CFWL_MsgKillFocus*>(pMessage);
      CFWL_EvtKillFocus evt;
      evt.m_pSrcTarget = pMsgKillFocus->m_pDstTarget;
      evt.m_pDstTarget = pMsgKillFocus->m_pDstTarget;
      evt.m_pKillFocus = pWidget;
      pWidget->DispatchEvent(&evt);
      break;
    }
    default:
      break;
  }
}

void CFWL_WidgetImpDelegate::OnProcessEvent(CFWL_Event* pEvent) {}

void CFWL_WidgetImpDelegate::OnDrawWidget(CFX_Graphics* pGraphics,
                                          const CFX_Matrix* pMatrix) {
  CFWL_EvtDraw evt;
  evt.m_pGraphics = pGraphics;
}
