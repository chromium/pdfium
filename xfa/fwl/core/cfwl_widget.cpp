// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/core/cfwl_widget.h"

#include <algorithm>
#include <utility>

#include "xfa/fde/tto/fde_textout.h"
#include "xfa/fwl/core/cfwl_app.h"
#include "xfa/fwl/core/cfwl_combobox.h"
#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_evtmouse.h"
#include "xfa/fwl/core/cfwl_form.h"
#include "xfa/fwl/core/cfwl_msgkey.h"
#include "xfa/fwl/core/cfwl_msgkillfocus.h"
#include "xfa/fwl/core/cfwl_msgmouse.h"
#include "xfa/fwl/core/cfwl_msgmousewheel.h"
#include "xfa/fwl/core/cfwl_msgsetfocus.h"
#include "xfa/fwl/core/cfwl_notedriver.h"
#include "xfa/fwl/core/cfwl_themebackground.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/cfwl_themetext.h"
#include "xfa/fwl/core/cfwl_widgetmgr.h"
#include "xfa/fwl/core/ifwl_themeprovider.h"
#include "xfa/fxfa/xfa_ffapp.h"

#define FWL_STYLEEXT_MNU_Vert (1L << 0)
#define FWL_WGT_CalcHeight 2048
#define FWL_WGT_CalcWidth 2048
#define FWL_WGT_CalcMultiLineDefWidth 120.0f

CFWL_Widget::CFWL_Widget(const CFWL_App* app,
                         std::unique_ptr<CFWL_WidgetProperties> properties,
                         CFWL_Widget* pOuter)
    : m_pOwnerApp(app),
      m_pWidgetMgr(app->GetWidgetMgr()),
      m_pProperties(std::move(properties)),
      m_pOuter(pOuter),
      m_iLock(0),
      m_pLayoutItem(nullptr),
      m_nEventKey(0),
      m_pDelegate(nullptr) {
  ASSERT(m_pWidgetMgr);

  CFWL_Widget* pParent = m_pProperties->m_pParent;
  m_pWidgetMgr->InsertWidget(pParent, this);
  if (IsChild())
    return;

  CFWL_Widget* pOwner = m_pProperties->m_pOwner;
  if (pOwner)
    m_pWidgetMgr->SetOwner(pOwner, this);
}

CFWL_Widget::~CFWL_Widget() {
  NotifyDriver();
  m_pWidgetMgr->RemoveWidget(this);
}

bool CFWL_Widget::IsInstance(const CFX_WideStringC& wsClass) const {
  return false;
}

void CFWL_Widget::GetWidgetRect(CFX_RectF& rect, bool bAutoSize) {
  if (!bAutoSize) {
    rect = m_pProperties->m_rtWidget;
    return;
  }
  InflateWidgetRect(rect);
}

void CFWL_Widget::InflateWidgetRect(CFX_RectF& rect) {
  if (HasEdge()) {
    FX_FLOAT fEdge = GetEdgeWidth();
    rect.Inflate(fEdge, fEdge);
  }
  if (HasBorder()) {
    FX_FLOAT fBorder = GetBorderSize(true);
    rect.Inflate(fBorder, fBorder);
  }
}

void CFWL_Widget::SetWidgetRect(const CFX_RectF& rect) {
  m_pProperties->m_rtWidget = rect;
  if (IsChild())
    return;

  m_pWidgetMgr->SetWidgetRect_Native(this, rect);
}

void CFWL_Widget::GetClientRect(CFX_RectF& rect) {
  GetEdgeRect(rect);
  if (HasEdge()) {
    FX_FLOAT fEdge = GetEdgeWidth();
    rect.Deflate(fEdge, fEdge);
  }
}

void CFWL_Widget::SetParent(CFWL_Widget* pParent) {
  m_pProperties->m_pParent = pParent;
  m_pWidgetMgr->SetParent(pParent, this);
}

uint32_t CFWL_Widget::GetStyles() const {
  return m_pProperties->m_dwStyles;
}

void CFWL_Widget::ModifyStyles(uint32_t dwStylesAdded,
                               uint32_t dwStylesRemoved) {
  m_pProperties->m_dwStyles =
      (m_pProperties->m_dwStyles & ~dwStylesRemoved) | dwStylesAdded;
}

uint32_t CFWL_Widget::GetStylesEx() const {
  return m_pProperties->m_dwStyleExes;
}
uint32_t CFWL_Widget::GetStates() const {
  return m_pProperties->m_dwStates;
}

void CFWL_Widget::ModifyStylesEx(uint32_t dwStylesExAdded,
                                 uint32_t dwStylesExRemoved) {
  m_pProperties->m_dwStyleExes =
      (m_pProperties->m_dwStyleExes & ~dwStylesExRemoved) | dwStylesExAdded;
}

static void NotifyHideChildWidget(CFWL_WidgetMgr* widgetMgr,
                                  CFWL_Widget* widget,
                                  CFWL_NoteDriver* noteDriver) {
  CFWL_Widget* child = widgetMgr->GetFirstChildWidget(widget);
  while (child) {
    noteDriver->NotifyTargetHide(child);
    NotifyHideChildWidget(widgetMgr, child, noteDriver);
    child = widgetMgr->GetNextSiblingWidget(child);
  }
}

void CFWL_Widget::SetStates(uint32_t dwStates) {
  m_pProperties->m_dwStates |= dwStates;
  if (!(dwStates & FWL_WGTSTATE_Invisible))
    return;

  CFWL_NoteDriver* noteDriver =
      static_cast<CFWL_NoteDriver*>(GetOwnerApp()->GetNoteDriver());
  CFWL_WidgetMgr* widgetMgr = GetOwnerApp()->GetWidgetMgr();
  noteDriver->NotifyTargetHide(this);
  CFWL_Widget* child = widgetMgr->GetFirstChildWidget(this);
  while (child) {
    noteDriver->NotifyTargetHide(child);
    NotifyHideChildWidget(widgetMgr, child, noteDriver);
    child = widgetMgr->GetNextSiblingWidget(child);
  }
  return;
}

void CFWL_Widget::RemoveStates(uint32_t dwStates) {
  m_pProperties->m_dwStates &= ~dwStates;
}

FWL_WidgetHit CFWL_Widget::HitTest(FX_FLOAT fx, FX_FLOAT fy) {
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

void CFWL_Widget::TransformTo(CFWL_Widget* pWidget,
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
    return;
  }
  CFX_RectF r;
  CFX_Matrix m;
  CFWL_Widget* parent = GetParent();
  if (parent) {
    GetWidgetRect(r, false);
    fx += r.left;
    fy += r.top;
    GetMatrix(m, true);
    m.TransformPoint(fx, fy);
  }
  CFWL_Widget* form1 = m_pWidgetMgr->GetSystemFormWidget(this);
  if (!form1)
    return;
  if (!pWidget) {
    form1->GetWidgetRect(r, false);
    fx += r.left;
    fy += r.top;
    return;
  }
  CFWL_Widget* form2 = m_pWidgetMgr->GetSystemFormWidget(pWidget);
  if (!form2)
    return;
  if (form1 != form2) {
    form1->GetWidgetRect(r, false);
    fx += r.left;
    fy += r.top;
    form2->GetWidgetRect(r, false);
    fx -= r.left;
    fy -= r.top;
  }
  parent = pWidget->GetParent();
  if (parent) {
    pWidget->GetMatrix(m, true);
    CFX_Matrix m1;
    m1.SetIdentity();
    m1.SetReverse(m);
    m1.TransformPoint(fx, fy);
    pWidget->GetWidgetRect(r, false);
    fx -= r.left;
    fy -= r.top;
  }
}

void CFWL_Widget::GetMatrix(CFX_Matrix& matrix, bool bGlobal) {
  if (!m_pProperties)
    return;
  if (!bGlobal) {
    matrix.SetIdentity();
    return;
  }

  CFWL_Widget* parent = GetParent();
  CFX_ArrayTemplate<CFWL_Widget*> parents;
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
    parent->GetMatrix(ctmOnParent, false);
    parent->GetWidgetRect(rect, false);
    matrix.Concat(ctmOnParent, true);
    matrix.Translate(rect.left, rect.top, true);
  }
  CFX_Matrix m;
  m.SetIdentity();
  matrix.Concat(m, true);
  parents.RemoveAll();
}

IFWL_ThemeProvider* CFWL_Widget::GetThemeProvider() const {
  return m_pProperties->m_pThemeProvider;
}

void CFWL_Widget::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  m_pProperties->m_pThemeProvider = pThemeProvider;
}

bool CFWL_Widget::IsEnabled() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled) == 0;
}

bool CFWL_Widget::IsActive() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Deactivated) == 0;
}

bool CFWL_Widget::HasBorder() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Border);
}

bool CFWL_Widget::HasEdge() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_EdgeMask);
}

bool CFWL_Widget::IsVisible() const {
  return (m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible) == 0;
}

bool CFWL_Widget::IsOverLapper() const {
  return (m_pProperties->m_dwStyles & FWL_WGTSTYLE_WindowTypeMask) ==
         FWL_WGTSTYLE_OverLapper;
}

bool CFWL_Widget::IsPopup() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Popup);
}

bool CFWL_Widget::IsChild() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Child);
}

bool CFWL_Widget::IsOffscreen() const {
  return !!(m_pProperties->m_dwStyles & FWL_WGTSTYLE_Offscreen);
}

void CFWL_Widget::GetEdgeRect(CFX_RectF& rtEdge) {
  rtEdge = m_pProperties->m_rtWidget;
  rtEdge.left = rtEdge.top = 0;
  if (HasBorder()) {
    FX_FLOAT fCX = GetBorderSize(true);
    FX_FLOAT fCY = GetBorderSize(false);
    rtEdge.Deflate(fCX, fCY);
  }
}

FX_FLOAT CFWL_Widget::GetBorderSize(bool bCX) {
  FX_FLOAT* pfBorder = static_cast<FX_FLOAT*>(GetThemeCapacity(
      bCX ? CFWL_WidgetCapacity::CXBorder : CFWL_WidgetCapacity::CYBorder));
  if (!pfBorder)
    return 0;
  return *pfBorder;
}

FX_FLOAT CFWL_Widget::GetEdgeWidth() {
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

void CFWL_Widget::GetRelativeRect(CFX_RectF& rect) {
  rect = m_pProperties->m_rtWidget;
  rect.left = rect.top = 0;
}

void* CFWL_Widget::GetThemeCapacity(CFWL_WidgetCapacity dwCapacity) {
  IFWL_ThemeProvider* pTheme = GetAvailableTheme();
  if (!pTheme)
    return nullptr;

  CFWL_ThemePart part;
  part.m_pWidget = this;
  return pTheme->GetCapacity(&part, dwCapacity);
}

IFWL_ThemeProvider* CFWL_Widget::GetAvailableTheme() {
  if (m_pProperties->m_pThemeProvider)
    return m_pProperties->m_pThemeProvider;

  CFWL_Widget* pUp = this;
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

CFWL_Widget* CFWL_Widget::GetRootOuter() {
  CFWL_Widget* pRet = m_pOuter;
  if (!pRet)
    return nullptr;

  while (CFWL_Widget* pOuter = pRet->GetOuter())
    pRet = pOuter;
  return pRet;
}

CFX_SizeF CFWL_Widget::CalcTextSize(const CFX_WideString& wsText,
                                    IFWL_ThemeProvider* pTheme,
                                    bool bMultiLine) {
  if (!pTheme)
    return CFX_SizeF();

  CFWL_ThemeText calPart;
  calPart.m_pWidget = this;
  calPart.m_wsText = wsText;
  calPart.m_dwTTOStyles =
      bMultiLine ? FDE_TTOSTYLE_LineWrap : FDE_TTOSTYLE_SingleLine;
  calPart.m_iTTOAlign = FDE_TTOALIGNMENT_TopLeft;
  CFX_RectF rect;
  FX_FLOAT fWidth =
      bMultiLine ? FWL_WGT_CalcMultiLineDefWidth : FWL_WGT_CalcWidth;
  rect.Set(0, 0, fWidth, FWL_WGT_CalcHeight);
  pTheme->CalcTextRect(&calPart, rect);
  return CFX_SizeF(rect.width, rect.height);
}

void CFWL_Widget::CalcTextRect(const CFX_WideString& wsText,
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

void CFWL_Widget::SetFocus(bool bFocus) {
  if (m_pWidgetMgr->IsFormDisabled())
    return;

  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  if (!pDriver)
    return;

  CFWL_Widget* curFocus = pDriver->GetFocus();
  if (bFocus && curFocus != this)
    pDriver->SetFocus(this);
  else if (!bFocus && curFocus == this)
    pDriver->SetFocus(nullptr);
}

void CFWL_Widget::SetGrab(bool bSet) {
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  pDriver->SetGrab(this, bSet);
}

void CFWL_Widget::GetPopupPos(FX_FLOAT fMinHeight,
                              FX_FLOAT fMaxHeight,
                              const CFX_RectF& rtAnchor,
                              CFX_RectF& rtPopup) {
  if (GetClassID() == FWL_Type::ComboBox) {
    if (m_pWidgetMgr->IsFormDisabled()) {
      m_pWidgetMgr->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                       rtPopup);
      return;
    }
    GetPopupPosComboBox(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
    return;
  }
  if (GetClassID() == FWL_Type::DateTimePicker &&
      m_pWidgetMgr->IsFormDisabled()) {
    m_pWidgetMgr->GetAdapterPopupPos(this, fMinHeight, fMaxHeight, rtAnchor,
                                     rtPopup);
    return;
  }
  GetPopupPosGeneral(fMinHeight, fMaxHeight, rtAnchor, rtPopup);
}

bool CFWL_Widget::GetPopupPosMenu(FX_FLOAT fMinHeight,
                                  FX_FLOAT fMaxHeight,
                                  const CFX_RectF& rtAnchor,
                                  CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;

  if (GetStylesEx() & FWL_STYLEEXT_MNU_Vert) {
    bool bLeft = m_pProperties->m_rtWidget.left < 0;
    FX_FLOAT fRight = rtAnchor.right() + rtPopup.width;
    TransformTo(nullptr, fx, fy);
    if (fRight + fx > 0.0f || bLeft) {
      rtPopup.Set(rtAnchor.left - rtPopup.width, rtAnchor.top, rtPopup.width,
                  rtPopup.height);
    } else {
      rtPopup.Set(rtAnchor.right(), rtAnchor.top, rtPopup.width,
                  rtPopup.height);
    }
  } else {
    FX_FLOAT fBottom = rtAnchor.bottom() + rtPopup.height;
    TransformTo(nullptr, fx, fy);
    if (fBottom + fy > 0.0f) {
      rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width,
                  rtPopup.height);
    } else {
      rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width,
                  rtPopup.height);
    }
  }
  rtPopup.Offset(fx, fy);
  return true;
}

bool CFWL_Widget::GetPopupPosComboBox(FX_FLOAT fMinHeight,
                                      FX_FLOAT fMaxHeight,
                                      const CFX_RectF& rtAnchor,
                                      CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;

  FX_FLOAT fPopHeight = rtPopup.height;
  if (rtPopup.height > fMaxHeight)
    fPopHeight = fMaxHeight;
  else if (rtPopup.height < fMinHeight)
    fPopHeight = fMinHeight;

  FX_FLOAT fWidth = std::max(rtAnchor.width, rtPopup.width);
  FX_FLOAT fBottom = rtAnchor.bottom() + fPopHeight;
  TransformTo(nullptr, fx, fy);
  if (fBottom + fy > 0.0f)
    rtPopup.Set(rtAnchor.left, rtAnchor.top - fPopHeight, fWidth, fPopHeight);
  else
    rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), fWidth, fPopHeight);

  rtPopup.Offset(fx, fy);
  return true;
}

bool CFWL_Widget::GetPopupPosGeneral(FX_FLOAT fMinHeight,
                                     FX_FLOAT fMaxHeight,
                                     const CFX_RectF& rtAnchor,
                                     CFX_RectF& rtPopup) {
  FX_FLOAT fx = 0;
  FX_FLOAT fy = 0;

  TransformTo(nullptr, fx, fy);
  if (rtAnchor.bottom() + fy > 0.0f) {
    rtPopup.Set(rtAnchor.left, rtAnchor.top - rtPopup.height, rtPopup.width,
                rtPopup.height);
  } else {
    rtPopup.Set(rtAnchor.left, rtAnchor.bottom(), rtPopup.width,
                rtPopup.height);
  }
  rtPopup.Offset(fx, fy);
  return true;
}

void CFWL_Widget::RegisterEventTarget(CFWL_Widget* pEventSource) {
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;

  pNoteDriver->RegisterEventTarget(this, pEventSource);
}

void CFWL_Widget::UnregisterEventTarget() {
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;

  pNoteDriver->UnregisterEventTarget(this);
}

void CFWL_Widget::DispatchEvent(CFWL_Event* pEvent) {
  if (m_pOuter) {
    m_pOuter->GetDelegate()->OnProcessEvent(pEvent);
    return;
  }
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pNoteDriver = pApp->GetNoteDriver();
  if (!pNoteDriver)
    return;
  pNoteDriver->SendEvent(pEvent);
}

void CFWL_Widget::Repaint(const CFX_RectF* pRect) {
  if (pRect) {
    m_pWidgetMgr->RepaintWidget(this, pRect);
    return;
  }
  CFX_RectF rect;
  rect = m_pProperties->m_rtWidget;
  rect.left = rect.top = 0;
  m_pWidgetMgr->RepaintWidget(this, &rect);
}

void CFWL_Widget::DrawBackground(CFX_Graphics* pGraphics,
                                 CFWL_Part iPartBk,
                                 IFWL_ThemeProvider* pTheme,
                                 const CFX_Matrix* pMatrix) {
  CFX_RectF rtRelative;
  GetRelativeRect(rtRelative);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartBk;
  param.m_pGraphics = pGraphics;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix, true);
  param.m_rtPart = rtRelative;
  pTheme->DrawBackground(&param);
}

void CFWL_Widget::DrawBorder(CFX_Graphics* pGraphics,
                             CFWL_Part iPartBorder,
                             IFWL_ThemeProvider* pTheme,
                             const CFX_Matrix* pMatrix) {
  CFX_RectF rtRelative;
  GetRelativeRect(rtRelative);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartBorder;
  param.m_pGraphics = pGraphics;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix, true);
  param.m_rtPart = rtRelative;
  pTheme->DrawBackground(&param);
}

void CFWL_Widget::DrawEdge(CFX_Graphics* pGraphics,
                           CFWL_Part iPartEdge,
                           IFWL_ThemeProvider* pTheme,
                           const CFX_Matrix* pMatrix) {
  CFX_RectF rtEdge;
  GetEdgeRect(rtEdge);
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = iPartEdge;
  param.m_pGraphics = pGraphics;
  if (pMatrix)
    param.m_matrix.Concat(*pMatrix, true);
  param.m_rtPart = rtEdge;
  pTheme->DrawBackground(&param);
}

void CFWL_Widget::NotifyDriver() {
  const CFWL_App* pApp = GetOwnerApp();
  if (!pApp)
    return;

  CFWL_NoteDriver* pDriver =
      static_cast<CFWL_NoteDriver*>(pApp->GetNoteDriver());
  if (!pDriver)
    return;

  pDriver->NotifyTargetDestroy(this);
}

CFX_SizeF CFWL_Widget::GetOffsetFromParent(CFWL_Widget* pParent) {
  if (pParent == this)
    return CFX_SizeF();

  CFWL_WidgetMgr* pWidgetMgr = GetOwnerApp()->GetWidgetMgr();
  if (!pWidgetMgr)
    return CFX_SizeF();

  CFX_SizeF szRet(m_pProperties->m_rtWidget.left,
                  m_pProperties->m_rtWidget.top);

  CFWL_Widget* pDstWidget = GetParent();
  while (pDstWidget && pDstWidget != pParent) {
    CFX_RectF rtDst;
    pDstWidget->GetWidgetRect(rtDst, false);
    szRet += CFX_SizeF(rtDst.left, rtDst.top);
    pDstWidget = pWidgetMgr->GetParentWidget(pDstWidget);
  }
  return szRet;
}

bool CFWL_Widget::IsParent(CFWL_Widget* pParent) {
  CFWL_Widget* pUpWidget = GetParent();
  while (pUpWidget) {
    if (pUpWidget == pParent)
      return true;
    pUpWidget = pUpWidget->GetParent();
  }
  return false;
}

void CFWL_Widget::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage->m_pDstTarget)
    return;

  CFWL_Widget* pWidget = pMessage->m_pDstTarget;
  switch (pMessage->GetType()) {
    case CFWL_Message::Type::Mouse: {
      CFWL_MsgMouse* pMsgMouse = static_cast<CFWL_MsgMouse*>(pMessage);

      CFWL_EvtMouse evt(pWidget, pWidget);
      evt.m_dwCmd = pMsgMouse->m_dwCmd;
      pWidget->DispatchEvent(&evt);
      break;
    }
    default:
      break;
  }
}

void CFWL_Widget::OnProcessEvent(CFWL_Event* pEvent) {}

void CFWL_Widget::OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix) {}
