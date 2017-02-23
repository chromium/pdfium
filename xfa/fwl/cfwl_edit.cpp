// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_edit.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_txtedtengine.h"
#include "xfa/fde/fde_gedevice.h"
#include "xfa/fde/fde_render.h"
#include "xfa/fde/ifde_txtedtpage.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_caret.h"
#include "xfa/fwl/cfwl_event.h"
#include "xfa/fwl/cfwl_eventcheckword.h"
#include "xfa/fwl/cfwl_eventtextchanged.h"
#include "xfa/fwl/cfwl_eventvalidate.h"
#include "xfa/fwl/cfwl_messagekey.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fwl/ifwl_themeprovider.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_ffwidget.h"
#include "xfa/fxgraphics/cfx_path.h"

namespace {

const int kEditMargin = 3;

bool FX_EDIT_ISLATINWORD(FX_WCHAR c) {
  return c == 0x2D || (c <= 0x005A && c >= 0x0041) ||
         (c <= 0x007A && c >= 0x0061) || (c <= 0x02AF && c >= 0x00C0) ||
         c == 0x0027;
}

void AddSquigglyPath(CFX_Path* pPathData,
                     FX_FLOAT fStartX,
                     FX_FLOAT fEndX,
                     FX_FLOAT fY,
                     FX_FLOAT fStep) {
  pPathData->MoveTo(CFX_PointF(fStartX, fY));
  int i = 1;
  for (FX_FLOAT fx = fStartX + fStep; fx < fEndX; fx += fStep, ++i)
    pPathData->LineTo(CFX_PointF(fx, fY + (i & 1) * fStep));
}

}  // namespace

CFWL_Edit::CFWL_Edit(const CFWL_App* app,
                     std::unique_ptr<CFWL_WidgetProperties> properties,
                     CFWL_Widget* pOuter)
    : CFWL_Widget(app, std::move(properties), pOuter),
      m_fVAlignOffset(0.0f),
      m_fScrollOffsetX(0.0f),
      m_fScrollOffsetY(0.0f),
      m_bLButtonDown(false),
      m_nSelStart(0),
      m_nLimit(-1),
      m_fFontSize(0),
      m_bSetRange(false),
      m_iMax(0xFFFFFFF),
      m_iCurRecord(-1),
      m_iMaxRecord(128) {
  m_rtClient.Reset();
  m_rtEngine.Reset();
  m_rtStatic.Reset();

  InitCaret();
}

CFWL_Edit::~CFWL_Edit() {
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused)
    HideCaret(nullptr);
  ClearRecord();
}

FWL_Type CFWL_Edit::GetClassID() const {
  return FWL_Type::Edit;
}

CFX_RectF CFWL_Edit::GetWidgetRect() {
  CFX_RectF rect = m_pProperties->m_rtWidget;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
    IFWL_ThemeProvider* theme = GetAvailableTheme();
    float scrollbarWidth = theme ? theme->GetScrollBarWidth() : 0.0f;
    if (IsShowScrollBar(true)) {
      rect.width += scrollbarWidth;
      rect.width += kEditMargin;
    }
    if (IsShowScrollBar(false)) {
      rect.height += scrollbarWidth;
      rect.height += kEditMargin;
    }
  }
  return rect;
}

CFX_RectF CFWL_Edit::GetAutosizedWidgetRect() {
  CFX_RectF rect;
  if (m_EdtEngine.GetTextLength() > 0) {
    CFX_SizeF sz = CalcTextSize(
        m_EdtEngine.GetText(0, -1), m_pProperties->m_pThemeProvider,
        !!(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine));
    rect = CFX_RectF(0, 0, sz);
  }
  InflateWidgetRect(rect);
  return rect;
}

void CFWL_Edit::SetStates(uint32_t dwStates) {
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Invisible) ||
      (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
    HideCaret(nullptr);
  }
  CFWL_Widget::SetStates(dwStates);
}

void CFWL_Edit::Update() {
  if (IsLocked())
    return;
  if (!m_pProperties->m_pThemeProvider)
    m_pProperties->m_pThemeProvider = GetAvailableTheme();

  Layout();
  if (m_rtClient.IsEmpty())
    return;

  UpdateEditEngine();
  UpdateVAlignment();
  UpdateScroll();
  InitCaret();
}

FWL_WidgetHit CFWL_Edit::HitTest(const CFX_PointF& point) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
    if (IsShowScrollBar(true)) {
      if (m_pVertScrollBar->GetWidgetRect().Contains(point))
        return FWL_WidgetHit::VScrollBar;
    }
    if (IsShowScrollBar(false)) {
      if (m_pHorzScrollBar->GetWidgetRect().Contains(point))
        return FWL_WidgetHit::HScrollBar;
    }
  }
  if (m_rtClient.Contains(point))
    return FWL_WidgetHit::Edit;
  return FWL_WidgetHit::Unknown;
}

void CFWL_Edit::AddSpellCheckObj(CFX_Path& PathData,
                                 int32_t nStart,
                                 int32_t nCount,
                                 FX_FLOAT fOffSetX,
                                 FX_FLOAT fOffSetY) {
  FX_FLOAT fStartX = 0.0f;
  FX_FLOAT fEndX = 0.0f;
  FX_FLOAT fY = 0.0f;
  FX_FLOAT fStep = 0.0f;
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  const FDE_TXTEDTPARAMS* txtEdtParams = m_EdtEngine.GetEditParams();
  FX_FLOAT fAsent = static_cast<FX_FLOAT>(txtEdtParams->pFont->GetAscent()) *
                    txtEdtParams->fFontSize / 1000;

  std::vector<CFX_RectF> rectArray;
  pPage->CalcRangeRectArray(nStart, nCount, &rectArray);

  for (const auto& rectText : rectArray) {
    fY = rectText.top + fAsent + fOffSetY;
    fStep = txtEdtParams->fFontSize / 16.0f;
    fStartX = rectText.left + fOffSetX;
    fEndX = fStartX + rectText.Width();
    AddSquigglyPath(&PathData, fStartX, fEndX, fY, fStep);
  }
}

void CFWL_Edit::DrawSpellCheck(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix) {
  pGraphics->SaveGraphState();
  if (pMatrix)
    pGraphics->ConcatMatrix(const_cast<CFX_Matrix*>(pMatrix));

  CFX_Color crLine(0xFFFF0000);
  CFWL_EventCheckWord checkWordEvent(this);
  CFX_ByteString sLatinWord;
  CFX_Path pathSpell;
  int32_t nStart = 0;
  FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
  FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
  CFX_WideString wsSpell = GetText();
  int32_t nContentLen = wsSpell.GetLength();
  for (int i = 0; i < nContentLen; i++) {
    if (FX_EDIT_ISLATINWORD(wsSpell[i])) {
      if (sLatinWord.IsEmpty())
        nStart = i;
      sLatinWord += (FX_CHAR)wsSpell[i];
      continue;
    }
    checkWordEvent.bsWord = sLatinWord;
    checkWordEvent.bCheckWord = true;
    DispatchEvent(&checkWordEvent);

    if (!sLatinWord.IsEmpty() && !checkWordEvent.bCheckWord) {
      AddSpellCheckObj(pathSpell, nStart, sLatinWord.GetLength(), fOffSetX,
                       fOffSetY);
    }
    sLatinWord.clear();
  }

  checkWordEvent.bsWord = sLatinWord;
  checkWordEvent.bCheckWord = true;
  DispatchEvent(&checkWordEvent);

  if (!sLatinWord.IsEmpty() && !checkWordEvent.bCheckWord) {
    AddSpellCheckObj(pathSpell, nStart, sLatinWord.GetLength(), fOffSetX,
                     fOffSetY);
  }
  if (!pathSpell.IsEmpty()) {
    CFX_RectF rtClip = m_rtEngine;
    CFX_Matrix mt(1, 0, 0, 1, fOffSetX, fOffSetY);
    if (pMatrix) {
      pMatrix->TransformRect(rtClip);
      mt.Concat(*pMatrix);
    }
    pGraphics->SetClipRect(rtClip);
    pGraphics->SetStrokeColor(&crLine);
    pGraphics->SetLineWidth(0);
    pGraphics->StrokePath(&pathSpell, nullptr);
  }
  pGraphics->RestoreGraphState();
}

void CFWL_Edit::DrawWidget(CFX_Graphics* pGraphics, const CFX_Matrix* pMatrix) {
  if (!pGraphics)
    return;
  if (!m_pProperties->m_pThemeProvider)
    return;
  if (m_rtClient.IsEmpty())
    return;

  IFWL_ThemeProvider* pTheme = m_pProperties->m_pThemeProvider;
  if (!m_pWidgetMgr->IsFormDisabled())
    DrawTextBk(pGraphics, pTheme, pMatrix);
  DrawContent(pGraphics, pTheme, pMatrix);

  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) &&
      !(m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly)) {
    DrawSpellCheck(pGraphics, pMatrix);
  }
  if (HasBorder())
    DrawBorder(pGraphics, CFWL_Part::Border, pTheme, pMatrix);
}

void CFWL_Edit::SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) {
  if (!pThemeProvider)
    return;
  if (m_pHorzScrollBar)
    m_pHorzScrollBar->SetThemeProvider(pThemeProvider);
  if (m_pVertScrollBar)
    m_pVertScrollBar->SetThemeProvider(pThemeProvider);
  if (m_pCaret)
    m_pCaret->SetThemeProvider(pThemeProvider);
  m_pProperties->m_pThemeProvider = pThemeProvider;
}

void CFWL_Edit::SetText(const CFX_WideString& wsText) {
  m_EdtEngine.SetText(wsText);
}

int32_t CFWL_Edit::GetTextLength() const {
  return m_EdtEngine.GetTextLength();
}

CFX_WideString CFWL_Edit::GetText() const {
  return m_EdtEngine.GetText(0, -1);
}

void CFWL_Edit::ClearText() {
  m_EdtEngine.ClearText();
}

void CFWL_Edit::AddSelRange(int32_t nStart) {
  m_EdtEngine.AddSelRange(nStart, -1);
}

int32_t CFWL_Edit::CountSelRanges() const {
  return m_EdtEngine.CountSelRanges();
}

int32_t CFWL_Edit::GetSelRange(int32_t nIndex, int32_t* nStart) const {
  return m_EdtEngine.GetSelRange(nIndex, nStart);
}

void CFWL_Edit::ClearSelections() {
  m_EdtEngine.ClearSelection();
}

int32_t CFWL_Edit::GetLimit() const {
  return m_nLimit;
}

void CFWL_Edit::SetLimit(int32_t nLimit) {
  m_nLimit = nLimit;
  m_EdtEngine.SetLimit(nLimit);
}

void CFWL_Edit::SetAliasChar(FX_WCHAR wAlias) {
  m_EdtEngine.SetAliasChar(wAlias);
}

bool CFWL_Edit::Copy(CFX_WideString& wsCopy) {
  int32_t nCount = m_EdtEngine.CountSelRanges();
  if (nCount == 0)
    return false;

  wsCopy.clear();
  int32_t nStart;
  int32_t nLength;
  for (int32_t i = 0; i < nCount; i++) {
    nLength = m_EdtEngine.GetSelRange(i, &nStart);
    wsCopy += m_EdtEngine.GetText(nStart, nLength);
  }
  return true;
}

bool CFWL_Edit::Cut(CFX_WideString& wsCut) {
  int32_t nCount = m_EdtEngine.CountSelRanges();
  if (nCount == 0)
    return false;

  wsCut.clear();
  CFX_WideString wsTemp;
  int32_t nStart, nLength;
  for (int32_t i = 0; i < nCount; i++) {
    nLength = m_EdtEngine.GetSelRange(i, &nStart);
    wsTemp = m_EdtEngine.GetText(nStart, nLength);
    wsCut += wsTemp;
    wsTemp.clear();
  }
  m_EdtEngine.Delete(0);
  return true;
}

bool CFWL_Edit::Paste(const CFX_WideString& wsPaste) {
  int32_t nCaret = m_EdtEngine.GetCaretPos();
  int32_t iError =
      m_EdtEngine.Insert(nCaret, wsPaste.c_str(), wsPaste.GetLength());
  if (iError < 0) {
    ProcessInsertError(iError);
    return false;
  }
  return true;
}

bool CFWL_Edit::Redo(const IFDE_TxtEdtDoRecord* pRecord) {
  return m_EdtEngine.Redo(pRecord);
}

bool CFWL_Edit::Undo(const IFDE_TxtEdtDoRecord* pRecord) {
  return m_EdtEngine.Undo(pRecord);
}

bool CFWL_Edit::Undo() {
  if (!CanUndo())
    return false;
  return Undo(m_DoRecords[m_iCurRecord--].get());
}

bool CFWL_Edit::Redo() {
  if (!CanRedo())
    return false;
  return Redo(m_DoRecords[++m_iCurRecord].get());
}

bool CFWL_Edit::CanUndo() {
  return m_iCurRecord >= 0;
}

bool CFWL_Edit::CanRedo() {
  return m_iCurRecord < pdfium::CollectionSize<int32_t>(m_DoRecords) - 1;
}

void CFWL_Edit::SetOuter(CFWL_Widget* pOuter) {
  m_pOuter = pOuter;
}

void CFWL_Edit::OnCaretChanged() {
  if (m_rtEngine.IsEmpty())
    return;
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
    return;

  bool bRepaintContent = UpdateOffset();
  UpdateCaret();
  CFX_RectF rtInvalid;
  bool bRepaintScroll = false;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) {
    CFWL_ScrollBar* pScroll = UpdateScroll();
    if (pScroll) {
      rtInvalid = pScroll->GetWidgetRect();
      bRepaintScroll = true;
    }
  }
  if (bRepaintContent || bRepaintScroll) {
    if (bRepaintContent)
      rtInvalid.Union(m_rtEngine);
    RepaintRect(rtInvalid);
  }
}

void CFWL_Edit::OnTextChanged(const FDE_TXTEDT_TEXTCHANGE_INFO& ChangeInfo) {
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VAlignMask)
    UpdateVAlignment();

  CFWL_EventTextChanged event(this);
  event.wsPrevText = ChangeInfo.wsPrevText;
  DispatchEvent(&event);

  LayoutScrollBar();
  RepaintRect(GetClientRect());
}

void CFWL_Edit::OnSelChanged() {
  RepaintRect(GetClientRect());
}

bool CFWL_Edit::OnPageLoad(int32_t nPageIndex) {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(nPageIndex);
  if (!pPage)
    return false;

  pPage->LoadPage(nullptr, nullptr);
  return true;
}

bool CFWL_Edit::OnPageUnload(int32_t nPageIndex) {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(nPageIndex);
  if (!pPage)
    return false;

  pPage->UnloadPage(nullptr);
  return true;
}

void CFWL_Edit::OnAddDoRecord(std::unique_ptr<IFDE_TxtEdtDoRecord> pRecord) {
  AddDoRecord(std::move(pRecord));
}

bool CFWL_Edit::OnValidate(const CFX_WideString& wsText) {
  CFWL_Widget* pDst = GetOuter();
  if (!pDst)
    pDst = this;

  CFWL_EventValidate event(this);
  event.wsInsert = wsText;
  event.bValidate = true;
  DispatchEvent(&event);
  return event.bValidate;
}

void CFWL_Edit::SetScrollOffset(FX_FLOAT fScrollOffset) {
  m_fScrollOffsetY = fScrollOffset;
}

void CFWL_Edit::DrawTextBk(CFX_Graphics* pGraphics,
                           IFWL_ThemeProvider* pTheme,
                           const CFX_Matrix* pMatrix) {
  CFWL_ThemeBackground param;
  param.m_pWidget = this;
  param.m_iPart = CFWL_Part::Background;
  param.m_bStaticBackground = false;
  param.m_dwStates = m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly
                         ? CFWL_PartState_ReadOnly
                         : CFWL_PartState_Normal;
  uint32_t dwStates = (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled);
  if (dwStates)
    param.m_dwStates = CFWL_PartState_Disabled;
  param.m_pGraphics = pGraphics;
  param.m_matrix = *pMatrix;
  param.m_rtPart = m_rtClient;
  pTheme->DrawBackground(&param);

  if (!IsShowScrollBar(true) || !IsShowScrollBar(false))
    return;

  CFX_RectF rtScroll = m_pHorzScrollBar->GetWidgetRect();

  CFX_RectF rtStatic(m_rtClient.right() - rtScroll.height,
                     m_rtClient.bottom() - rtScroll.height, rtScroll.height,
                     rtScroll.height);
  param.m_bStaticBackground = true;
  param.m_bMaximize = true;
  param.m_rtPart = rtStatic;
  pTheme->DrawBackground(&param);
}

void CFWL_Edit::DrawContent(CFX_Graphics* pGraphics,
                            IFWL_ThemeProvider* pTheme,
                            const CFX_Matrix* pMatrix) {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return;

  pGraphics->SaveGraphState();
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText)
    pGraphics->SaveGraphState();

  CFX_RectF rtClip = m_rtEngine;
  FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
  FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
  CFX_Matrix mt(1, 0, 0, 1, fOffSetX, fOffSetY);
  if (pMatrix) {
    pMatrix->TransformRect(rtClip);
    mt.Concat(*pMatrix);
  }

  bool bShowSel = !!(m_pProperties->m_dwStates & FWL_WGTSTATE_Focused);
  int32_t nSelCount = m_EdtEngine.CountSelRanges();
  if (bShowSel && nSelCount > 0) {
    int32_t nPageCharStart = pPage->GetCharStart();
    int32_t nPageCharCount = pPage->GetCharCount();
    int32_t nPageCharEnd = nPageCharStart + nPageCharCount - 1;
    int32_t nCharCount;
    int32_t nCharStart;
    std::vector<CFX_RectF> rectArr;
    for (int32_t i = 0; i < nSelCount; i++) {
      nCharCount = m_EdtEngine.GetSelRange(i, &nCharStart);
      int32_t nCharEnd = nCharStart + nCharCount - 1;
      if (nCharEnd < nPageCharStart || nCharStart > nPageCharEnd)
        continue;

      int32_t nBgn = std::max(nCharStart, nPageCharStart);
      int32_t nEnd = std::min(nCharEnd, nPageCharEnd);
      pPage->CalcRangeRectArray(nBgn - nPageCharStart, nEnd - nBgn + 1,
                                &rectArr);
    }

    CFX_Path path;
    for (auto& rect : rectArr) {
      rect.left += fOffSetX;
      rect.top += fOffSetY;
      path.AddRectangle(rect.left, rect.top, rect.width, rect.height);
    }
    pGraphics->SetClipRect(rtClip);

    CFWL_ThemeBackground param;
    param.m_pGraphics = pGraphics;
    param.m_matrix = *pMatrix;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::Background;
    param.m_pPath = &path;
    pTheme->DrawBackground(&param);
  }

  CFX_RenderDevice* pRenderDev = pGraphics->GetRenderDevice();
  if (!pRenderDev)
    return;

  std::unique_ptr<CFDE_RenderDevice> pRenderDevice(
      new CFDE_RenderDevice(pRenderDev, false));
  std::unique_ptr<CFDE_RenderContext> pRenderContext(new CFDE_RenderContext);
  pRenderDevice->SetClipRect(rtClip);
  pRenderContext->StartRender(pRenderDevice.get(), pPage, mt);
  pRenderContext->DoRender(nullptr);

  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText) {
    pGraphics->RestoreGraphState();
    CFX_Path path;
    int32_t iLimit = m_nLimit > 0 ? m_nLimit : 1;
    FX_FLOAT fStep = m_rtEngine.width / iLimit;
    FX_FLOAT fLeft = m_rtEngine.left + 1;
    for (int32_t i = 1; i < iLimit; i++) {
      fLeft += fStep;
      path.AddLine(CFX_PointF(fLeft, m_rtClient.top),
                   CFX_PointF(fLeft, m_rtClient.bottom()));
    }

    CFWL_ThemeBackground param;
    param.m_pGraphics = pGraphics;
    param.m_matrix = *pMatrix;
    param.m_pWidget = this;
    param.m_iPart = CFWL_Part::CombTextLine;
    param.m_pPath = &path;
    pTheme->DrawBackground(&param);
  }
  pGraphics->RestoreGraphState();
}

void CFWL_Edit::UpdateEditEngine() {
  UpdateEditParams();
  UpdateEditLayout();
  if (m_nLimit > -1)
    m_EdtEngine.SetLimit(m_nLimit);
}

void CFWL_Edit::UpdateEditParams() {
  FDE_TXTEDTPARAMS params;
  params.nHorzScale = 100;
  params.fPlateWidth = m_rtEngine.width;
  params.fPlateHeight = m_rtEngine.height;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_CombText)
    params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_CombText;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_LastLineHeight)
    params.dwLayoutStyles |= FDE_TEXTEDITLAYOUT_LastLineHeight;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Validate)
    params.dwMode |= FDE_TEXTEDITMODE_Validate;
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Password)
    params.dwMode |= FDE_TEXTEDITMODE_Password;

  switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HAlignMask) {
    case FWL_STYLEEXT_EDT_HNear: {
      params.dwAlignment |= FDE_TEXTEDITALIGN_Left;
      break;
    }
    case FWL_STYLEEXT_EDT_HCenter: {
      params.dwAlignment |= FDE_TEXTEDITALIGN_Center;
      break;
    }
    case FWL_STYLEEXT_EDT_HFar: {
      params.dwAlignment |= FDE_TEXTEDITALIGN_Right;
      break;
    }
    default:
      break;
  }
  switch (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_HAlignModeMask) {
    case FWL_STYLEEXT_EDT_Justified: {
      params.dwAlignment |= FDE_TEXTEDITALIGN_Justified;
      break;
    }
    default: {
      params.dwAlignment |= FDE_TEXTEDITALIGN_Normal;
      break;
    }
  }
  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) {
    params.dwMode |= FDE_TEXTEDITMODE_MultiLines;
    if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoHScroll) == 0) {
      params.dwMode |=
          FDE_TEXTEDITMODE_AutoLineWrap | FDE_TEXTEDITMODE_LimitArea_Horz;
    }
    if ((m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll) == 0 &&
        (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoVScroll) == 0) {
      params.dwMode |= FDE_TEXTEDITMODE_LimitArea_Vert;
    } else {
      params.fPlateHeight = 0x00FFFFFF;
    }
  } else if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_AutoHScroll) ==
             0) {
    params.dwMode |= FDE_TEXTEDITMODE_LimitArea_Horz;
  }
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly) ||
      (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
    params.dwMode |= FDE_TEXTEDITMODE_ReadOnly;
  }

  IFWL_ThemeProvider* theme = GetAvailableTheme();
  CFWL_ThemePart part;
  part.m_pWidget = this;
  m_fFontSize = theme ? theme->GetFontSize(&part) : FWLTHEME_CAPACITY_FontSize;

  if (!theme)
    return;

  params.dwFontColor = theme->GetTextColor(&part);
  params.fLineSpace = theme->GetLineHeight(&part);

  CFX_RetainPtr<CFGAS_GEFont> pFont = theme->GetFont(&part);
  if (!pFont)
    return;

  params.pFont = pFont;
  params.fFontSize = m_fFontSize;
  params.nLineCount = (int32_t)(params.fPlateHeight / params.fLineSpace);
  if (params.nLineCount <= 0)
    params.nLineCount = 1;
  params.fTabWidth = params.fFontSize * 1;
  params.bTabEquidistant = true;
  params.wLineBreakChar = L'\n';
  params.nCharRotation = 0;
  params.pEventSink = this;
  m_EdtEngine.SetEditParams(params);
}

void CFWL_Edit::UpdateEditLayout() {
  if (m_EdtEngine.GetTextLength() <= 0)
    m_EdtEngine.SetTextByStream(nullptr);

  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (pPage)
    pPage->UnloadPage(nullptr);

  m_EdtEngine.StartLayout();
  m_EdtEngine.DoLayout(nullptr);
  m_EdtEngine.EndLayout();
  pPage = m_EdtEngine.GetPage(0);
  if (pPage)
    pPage->LoadPage(nullptr, nullptr);
}

bool CFWL_Edit::UpdateOffset() {
  CFX_RectF rtCaret;
  m_EdtEngine.GetCaretRect(rtCaret);
  FX_FLOAT fOffSetX = m_rtEngine.left - m_fScrollOffsetX;
  FX_FLOAT fOffSetY = m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset;
  rtCaret.Offset(fOffSetX, fOffSetY);
  const CFX_RectF& rtEidt = m_rtEngine;
  if (rtEidt.Contains(rtCaret)) {
    IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
    if (!pPage)
      return false;

    CFX_RectF rtFDE = pPage->GetContentsBox();
    rtFDE.Offset(fOffSetX, fOffSetY);
    if (rtFDE.right() < rtEidt.right() && m_fScrollOffsetX > 0) {
      m_fScrollOffsetX += rtFDE.right() - rtEidt.right();
      m_fScrollOffsetX = std::max(m_fScrollOffsetX, 0.0f);
    }
    if (rtFDE.bottom() < rtEidt.bottom() && m_fScrollOffsetY > 0) {
      m_fScrollOffsetY += rtFDE.bottom() - rtEidt.bottom();
      m_fScrollOffsetY = std::max(m_fScrollOffsetY, 0.0f);
    }
    return false;
  }

  FX_FLOAT offsetX = 0.0;
  FX_FLOAT offsetY = 0.0;
  if (rtCaret.left < rtEidt.left)
    offsetX = rtCaret.left - rtEidt.left;
  if (rtCaret.right() > rtEidt.right())
    offsetX = rtCaret.right() - rtEidt.right();
  if (rtCaret.top < rtEidt.top)
    offsetY = rtCaret.top - rtEidt.top;
  if (rtCaret.bottom() > rtEidt.bottom())
    offsetY = rtCaret.bottom() - rtEidt.bottom();
  m_fScrollOffsetX += offsetX;
  m_fScrollOffsetY += offsetY;
  if (m_fFontSize > m_rtEngine.height)
    m_fScrollOffsetY = 0;
  return true;
}

bool CFWL_Edit::UpdateOffset(CFWL_ScrollBar* pScrollBar, FX_FLOAT fPosChanged) {
  if (pScrollBar == m_pHorzScrollBar.get())
    m_fScrollOffsetX += fPosChanged;
  else
    m_fScrollOffsetY += fPosChanged;
  return true;
}

void CFWL_Edit::UpdateVAlignment() {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return;

  const CFX_RectF& rtFDE = pPage->GetContentsBox();
  FX_FLOAT fOffsetY = 0.0f;
  FX_FLOAT fSpaceAbove = 0.0f;
  FX_FLOAT fSpaceBelow = 0.0f;
  IFWL_ThemeProvider* theme = GetAvailableTheme();
  if (theme) {
    CFWL_ThemePart part;
    part.m_pWidget = this;

    CFX_SizeF pSpace = theme->GetSpaceAboveBelow(&part);
    fSpaceAbove = pSpace.width;
    fSpaceBelow = pSpace.height;
  }
  if (fSpaceAbove < 0.1f)
    fSpaceAbove = 0;
  if (fSpaceBelow < 0.1f)
    fSpaceBelow = 0;

  if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VCenter) {
    fOffsetY = (m_rtEngine.height - rtFDE.height) / 2;
    if (fOffsetY < (fSpaceAbove + fSpaceBelow) / 2 &&
        fSpaceAbove < fSpaceBelow) {
      return;
    }
    fOffsetY += (fSpaceAbove - fSpaceBelow) / 2;
  } else if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_VFar) {
    fOffsetY = (m_rtEngine.height - rtFDE.height);
    fOffsetY -= fSpaceBelow;
  } else {
    fOffsetY += fSpaceAbove;
  }
  m_fVAlignOffset = std::max(fOffsetY, 0.0f);
}

void CFWL_Edit::UpdateCaret() {
  CFX_RectF rtFDE;
  m_EdtEngine.GetCaretRect(rtFDE);

  rtFDE.Offset(m_rtEngine.left - m_fScrollOffsetX,
               m_rtEngine.top - m_fScrollOffsetY + m_fVAlignOffset);
  CFX_RectF rtCaret(rtFDE.left, rtFDE.top, rtFDE.width, rtFDE.height);

  CFX_RectF rtClient = GetClientRect();
  rtCaret.Intersect(rtClient);
  if (rtCaret.left > rtClient.right()) {
    FX_FLOAT right = rtCaret.right();
    rtCaret.left = rtClient.right() - 1;
    rtCaret.width = right - rtCaret.left;
  }

  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused && !rtCaret.IsEmpty())
    ShowCaret(&rtCaret);
  else
    HideCaret(&rtCaret);
}

CFWL_ScrollBar* CFWL_Edit::UpdateScroll() {
  bool bShowHorz =
      m_pHorzScrollBar &&
      ((m_pHorzScrollBar->GetStates() & FWL_WGTSTATE_Invisible) == 0);
  bool bShowVert =
      m_pVertScrollBar &&
      ((m_pVertScrollBar->GetStates() & FWL_WGTSTATE_Invisible) == 0);
  if (!bShowHorz && !bShowVert)
    return nullptr;

  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return nullptr;

  const CFX_RectF& rtFDE = pPage->GetContentsBox();
  CFWL_ScrollBar* pRepaint = nullptr;
  if (bShowHorz) {
    CFX_RectF rtScroll = m_pHorzScrollBar->GetWidgetRect();
    if (rtScroll.width < rtFDE.width) {
      m_pHorzScrollBar->LockUpdate();
      FX_FLOAT fRange = rtFDE.width - rtScroll.width;
      m_pHorzScrollBar->SetRange(0.0f, fRange);

      FX_FLOAT fPos = std::min(std::max(m_fScrollOffsetX, 0.0f), fRange);
      m_pHorzScrollBar->SetPos(fPos);
      m_pHorzScrollBar->SetTrackPos(fPos);
      m_pHorzScrollBar->SetPageSize(rtScroll.width);
      m_pHorzScrollBar->SetStepSize(rtScroll.width / 10);
      m_pHorzScrollBar->RemoveStates(FWL_WGTSTATE_Disabled);
      m_pHorzScrollBar->UnlockUpdate();
      m_pHorzScrollBar->Update();
      pRepaint = m_pHorzScrollBar.get();
    } else if ((m_pHorzScrollBar->GetStates() & FWL_WGTSTATE_Disabled) == 0) {
      m_pHorzScrollBar->LockUpdate();
      m_pHorzScrollBar->SetRange(0, -1);
      m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Disabled);
      m_pHorzScrollBar->UnlockUpdate();
      m_pHorzScrollBar->Update();
      pRepaint = m_pHorzScrollBar.get();
    }
  }

  if (bShowVert) {
    CFX_RectF rtScroll = m_pVertScrollBar->GetWidgetRect();
    if (rtScroll.height < rtFDE.height) {
      m_pVertScrollBar->LockUpdate();
      FX_FLOAT fStep = m_EdtEngine.GetEditParams()->fLineSpace;
      FX_FLOAT fRange = std::max(rtFDE.height - m_rtEngine.height, fStep);

      m_pVertScrollBar->SetRange(0.0f, fRange);
      FX_FLOAT fPos = std::min(std::max(m_fScrollOffsetY, 0.0f), fRange);
      m_pVertScrollBar->SetPos(fPos);
      m_pVertScrollBar->SetTrackPos(fPos);
      m_pVertScrollBar->SetPageSize(rtScroll.height);
      m_pVertScrollBar->SetStepSize(fStep);
      m_pVertScrollBar->RemoveStates(FWL_WGTSTATE_Disabled);
      m_pVertScrollBar->UnlockUpdate();
      m_pVertScrollBar->Update();
      pRepaint = m_pVertScrollBar.get();
    } else if ((m_pVertScrollBar->GetStates() & FWL_WGTSTATE_Disabled) == 0) {
      m_pVertScrollBar->LockUpdate();
      m_pVertScrollBar->SetRange(0, -1);
      m_pVertScrollBar->SetStates(FWL_WGTSTATE_Disabled);
      m_pVertScrollBar->UnlockUpdate();
      m_pVertScrollBar->Update();
      pRepaint = m_pVertScrollBar.get();
    }
  }
  return pRepaint;
}

bool CFWL_Edit::IsShowScrollBar(bool bVert) {
  bool bShow =
      (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ShowScrollbarFocus)
          ? (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) ==
                FWL_WGTSTATE_Focused
          : true;
  if (bVert) {
    return bShow && (m_pProperties->m_dwStyles & FWL_WGTSTYLE_VScroll) &&
           (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_MultiLine) &&
           IsContentHeightOverflow();
  }
  return false;
}

bool CFWL_Edit::IsContentHeightOverflow() {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return false;
  return pPage->GetContentsBox().height > m_rtEngine.height + 1.0f;
}

int32_t CFWL_Edit::AddDoRecord(std::unique_ptr<IFDE_TxtEdtDoRecord> pRecord) {
  int32_t nCount = pdfium::CollectionSize<int32_t>(m_DoRecords);
  if (m_iCurRecord == nCount - 1) {
    if (nCount == m_iMaxRecord) {
      m_DoRecords.pop_front();
      m_iCurRecord--;
    }
  } else {
    m_DoRecords.erase(m_DoRecords.begin() + m_iCurRecord + 1,
                      m_DoRecords.end());
  }

  m_DoRecords.push_back(std::move(pRecord));
  m_iCurRecord = pdfium::CollectionSize<int32_t>(m_DoRecords) - 1;
  return m_iCurRecord;
}

void CFWL_Edit::Layout() {
  m_rtClient = GetClientRect();
  m_rtEngine = m_rtClient;
  IFWL_ThemeProvider* theme = GetAvailableTheme();
  if (!theme)
    return;

  FX_FLOAT fWidth = theme->GetScrollBarWidth();
  CFWL_ThemePart part;
  if (!m_pOuter) {
    part.m_pWidget = this;
    CFX_RectF pUIMargin = theme->GetUIMargin(&part);
    m_rtEngine.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                       pUIMargin.height);
  } else if (m_pOuter->GetClassID() == FWL_Type::DateTimePicker) {
    part.m_pWidget = m_pOuter;
    CFX_RectF pUIMargin = theme->GetUIMargin(&part);
    m_rtEngine.Deflate(pUIMargin.left, pUIMargin.top, pUIMargin.width,
                       pUIMargin.height);
  }

  bool bShowVertScrollbar = IsShowScrollBar(true);
  bool bShowHorzScrollbar = IsShowScrollBar(false);
  if (bShowVertScrollbar) {
    InitVerticalScrollBar();

    CFX_RectF rtVertScr;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
      rtVertScr = CFX_RectF(m_rtClient.right() + kEditMargin, m_rtClient.top,
                            fWidth, m_rtClient.height);
    } else {
      rtVertScr = CFX_RectF(m_rtClient.right() - fWidth, m_rtClient.top, fWidth,
                            m_rtClient.height);
      if (bShowHorzScrollbar)
        rtVertScr.height -= fWidth;
      m_rtEngine.width -= fWidth;
    }

    m_pVertScrollBar->SetWidgetRect(rtVertScr);
    m_pVertScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
    m_pVertScrollBar->Update();
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }

  if (bShowHorzScrollbar) {
    InitHorizontalScrollBar();

    CFX_RectF rtHoriScr;
    if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
      rtHoriScr = CFX_RectF(m_rtClient.left, m_rtClient.bottom() + kEditMargin,
                            m_rtClient.width, fWidth);
    } else {
      rtHoriScr = CFX_RectF(m_rtClient.left, m_rtClient.bottom() - fWidth,
                            m_rtClient.width, fWidth);
      if (bShowVertScrollbar)
        rtHoriScr.width -= fWidth;
      m_rtEngine.height -= fWidth;
    }
    m_pHorzScrollBar->SetWidgetRect(rtHoriScr);
    m_pHorzScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
    m_pHorzScrollBar->Update();
  } else if (m_pHorzScrollBar) {
    m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }
}

void CFWL_Edit::LayoutScrollBar() {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ShowScrollbarFocus) ==
      0) {
    return;
  }

  bool bShowVertScrollbar = IsShowScrollBar(true);
  bool bShowHorzScrollbar = IsShowScrollBar(false);

  IFWL_ThemeProvider* theme = GetAvailableTheme();
  FX_FLOAT fWidth = theme ? theme->GetScrollBarWidth() : 0;
  if (bShowVertScrollbar) {
    if (!m_pVertScrollBar) {
      InitVerticalScrollBar();
      CFX_RectF rtVertScr;
      if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
        rtVertScr = CFX_RectF(m_rtClient.right() + kEditMargin, m_rtClient.top,
                              fWidth, m_rtClient.height);
      } else {
        rtVertScr = CFX_RectF(m_rtClient.right() - fWidth, m_rtClient.top,
                              fWidth, m_rtClient.height);
        if (bShowHorzScrollbar)
          rtVertScr.height -= fWidth;
      }
      m_pVertScrollBar->SetWidgetRect(rtVertScr);
      m_pVertScrollBar->Update();
    }
    m_pVertScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
  } else if (m_pVertScrollBar) {
    m_pVertScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }

  if (bShowHorzScrollbar) {
    if (!m_pHorzScrollBar) {
      InitHorizontalScrollBar();
      CFX_RectF rtHoriScr;
      if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_OuterScrollbar) {
        rtHoriScr =
            CFX_RectF(m_rtClient.left, m_rtClient.bottom() + kEditMargin,
                      m_rtClient.width, fWidth);
      } else {
        rtHoriScr = CFX_RectF(m_rtClient.left, m_rtClient.bottom() - fWidth,
                              m_rtClient.width, fWidth);
        if (bShowVertScrollbar)
          rtHoriScr.width -= (fWidth);
      }
      m_pHorzScrollBar->SetWidgetRect(rtHoriScr);
      m_pHorzScrollBar->Update();
    }
    m_pHorzScrollBar->RemoveStates(FWL_WGTSTATE_Invisible);
  } else if (m_pHorzScrollBar) {
    m_pHorzScrollBar->SetStates(FWL_WGTSTATE_Invisible);
  }
  if (bShowVertScrollbar || bShowHorzScrollbar)
    UpdateScroll();
}

CFX_PointF CFWL_Edit::DeviceToEngine(const CFX_PointF& pt) {
  return pt + CFX_PointF(m_fScrollOffsetX - m_rtEngine.left,
                         m_fScrollOffsetY - m_rtEngine.top - m_fVAlignOffset);
}

void CFWL_Edit::InitVerticalScrollBar() {
  if (m_pVertScrollBar)
    return;

  auto prop = pdfium::MakeUnique<CFWL_WidgetProperties>();
  prop->m_dwStyleExes = FWL_STYLEEXT_SCB_Vert;
  prop->m_dwStates = FWL_WGTSTATE_Disabled | FWL_WGTSTATE_Invisible;
  prop->m_pParent = this;
  prop->m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pVertScrollBar =
      pdfium::MakeUnique<CFWL_ScrollBar>(m_pOwnerApp, std::move(prop), this);
}

void CFWL_Edit::InitHorizontalScrollBar() {
  if (m_pHorzScrollBar)
    return;

  auto prop = pdfium::MakeUnique<CFWL_WidgetProperties>();
  prop->m_dwStyleExes = FWL_STYLEEXT_SCB_Horz;
  prop->m_dwStates = FWL_WGTSTATE_Disabled | FWL_WGTSTATE_Invisible;
  prop->m_pParent = this;
  prop->m_pThemeProvider = m_pProperties->m_pThemeProvider;
  m_pHorzScrollBar =
      pdfium::MakeUnique<CFWL_ScrollBar>(m_pOwnerApp, std::move(prop), this);
}

void CFWL_Edit::ShowCaret(CFX_RectF* pRect) {
  if (m_pCaret) {
    m_pCaret->ShowCaret();
    if (!pRect->IsEmpty())
      m_pCaret->SetWidgetRect(*pRect);
    RepaintRect(m_rtEngine);
    return;
  }

  CFWL_Widget* pOuter = this;
  pRect->Offset(m_pProperties->m_rtWidget.left, m_pProperties->m_rtWidget.top);
  while (pOuter->GetOuter()) {
    pOuter = pOuter->GetOuter();

    CFX_RectF rtOuter = pOuter->GetWidgetRect();
    pRect->Offset(rtOuter.left, rtOuter.top);
  }

  CXFA_FFWidget* pXFAWidget = pOuter->GetLayoutItem();
  if (!pXFAWidget)
    return;

  IXFA_DocEnvironment* pDocEnvironment =
      pXFAWidget->GetDoc()->GetDocEnvironment();
  if (!pDocEnvironment)
    return;

  CFX_RectF rt(*pRect);
  pXFAWidget->GetRotateMatrix().TransformRect(rt);
  pDocEnvironment->DisplayCaret(pXFAWidget, true, &rt);
}

void CFWL_Edit::HideCaret(CFX_RectF* pRect) {
  if (m_pCaret) {
    m_pCaret->HideCaret();
    RepaintRect(m_rtEngine);
    return;
  }

  CFWL_Widget* pOuter = this;
  while (pOuter->GetOuter())
    pOuter = pOuter->GetOuter();

  CXFA_FFWidget* pXFAWidget = pOuter->GetLayoutItem();
  if (!pXFAWidget)
    return;

  IXFA_DocEnvironment* pDocEnvironment =
      pXFAWidget->GetDoc()->GetDocEnvironment();
  if (!pDocEnvironment)
    return;

  pDocEnvironment->DisplayCaret(pXFAWidget, false, pRect);
}

bool CFWL_Edit::ValidateNumberChar(FX_WCHAR cNum) {
  if (!m_bSetRange)
    return true;

  CFX_WideString wsText = m_EdtEngine.GetText(0, -1);
  if (wsText.IsEmpty()) {
    if (cNum == L'0')
      return false;
    return true;
  }

  int32_t caretPos = m_EdtEngine.GetCaretPos();
  if (CountSelRanges() == 0) {
    if (cNum == L'0' && caretPos == 0)
      return false;

    int32_t nLen = wsText.GetLength();
    CFX_WideString l = wsText.Mid(0, caretPos);
    CFX_WideString r = wsText.Mid(caretPos, nLen - caretPos);
    CFX_WideString wsNew = l + cNum + r;
    if (wsNew.GetInteger() <= m_iMax)
      return true;
    return false;
  }

  if (wsText.GetInteger() <= m_iMax)
    return true;
  return false;
}

void CFWL_Edit::InitCaret() {
  if (!m_pCaret)
    return;
  m_pCaret.reset();
}

void CFWL_Edit::ClearRecord() {
  m_iCurRecord = -1;
  m_DoRecords.clear();
}

void CFWL_Edit::ProcessInsertError(int32_t iError) {
  if (iError != -2)
    return;

  CFWL_Event textFullEvent(CFWL_Event::Type::TextFull, this);
  DispatchEvent(&textFullEvent);
}

void CFWL_Edit::OnProcessMessage(CFWL_Message* pMessage) {
  if (!pMessage)
    return;

  switch (pMessage->GetType()) {
    case CFWL_Message::Type::SetFocus:
      OnFocusChanged(pMessage, true);
      break;
    case CFWL_Message::Type::KillFocus:
      OnFocusChanged(pMessage, false);
      break;
    case CFWL_Message::Type::Mouse: {
      CFWL_MessageMouse* pMsg = static_cast<CFWL_MessageMouse*>(pMessage);
      switch (pMsg->m_dwCmd) {
        case FWL_MouseCommand::LeftButtonDown:
          OnLButtonDown(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonUp:
          OnLButtonUp(pMsg);
          break;
        case FWL_MouseCommand::LeftButtonDblClk:
          OnButtonDblClk(pMsg);
          break;
        case FWL_MouseCommand::Move:
          OnMouseMove(pMsg);
          break;
        case FWL_MouseCommand::RightButtonDown:
          DoButtonDown(pMsg);
          break;
        default:
          break;
      }
      break;
    }
    case CFWL_Message::Type::Key: {
      CFWL_MessageKey* pKey = static_cast<CFWL_MessageKey*>(pMessage);
      if (pKey->m_dwCmd == FWL_KeyCommand::KeyDown)
        OnKeyDown(pKey);
      else if (pKey->m_dwCmd == FWL_KeyCommand::Char)
        OnChar(pKey);
      break;
    }
    default:
      break;
  }
  CFWL_Widget::OnProcessMessage(pMessage);
}

void CFWL_Edit::OnProcessEvent(CFWL_Event* pEvent) {
  if (!pEvent)
    return;
  if (pEvent->GetType() != CFWL_Event::Type::Scroll)
    return;

  CFWL_Widget* pSrcTarget = pEvent->m_pSrcTarget;
  if ((pSrcTarget == m_pVertScrollBar.get() && m_pVertScrollBar) ||
      (pSrcTarget == m_pHorzScrollBar.get() && m_pHorzScrollBar)) {
    CFWL_EventScroll* pScrollEvent = static_cast<CFWL_EventScroll*>(pEvent);
    OnScroll(static_cast<CFWL_ScrollBar*>(pSrcTarget),
             pScrollEvent->m_iScrollCode, pScrollEvent->m_fPos);
  }
}

void CFWL_Edit::OnDrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix) {
  DrawWidget(pGraphics, pMatrix);
}

void CFWL_Edit::DoButtonDown(CFWL_MessageMouse* pMsg) {
  if ((m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) == 0)
    SetFocus(true);

  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return;

  bool bBefore = true;
  int32_t nIndex = pPage->GetCharIndex(DeviceToEngine(pMsg->m_pos), bBefore);
  if (nIndex < 0)
    nIndex = 0;

  m_EdtEngine.SetCaretPos(nIndex, bBefore);
}

void CFWL_Edit::OnFocusChanged(CFWL_Message* pMsg, bool bSet) {
  bool bRepaint = false;
  if (bSet) {
    m_pProperties->m_dwStates |= FWL_WGTSTATE_Focused;

    UpdateVAlignment();
    UpdateOffset();
    UpdateCaret();
  } else if (m_pProperties->m_dwStates & FWL_WGTSTATE_Focused) {
    m_pProperties->m_dwStates &= ~FWL_WGTSTATE_Focused;
    HideCaret(nullptr);

    int32_t nSel = CountSelRanges();
    if (nSel > 0) {
      ClearSelections();
      bRepaint = true;
    }
    m_EdtEngine.SetCaretPos(0, true);
    UpdateOffset();

    ClearRecord();
  }

  LayoutScrollBar();
  if (!bRepaint)
    return;

  CFX_RectF rtInvalidate(0, 0, m_pProperties->m_rtWidget.width,
                         m_pProperties->m_rtWidget.height);
  RepaintRect(rtInvalidate);
}

void CFWL_Edit::OnLButtonDown(CFWL_MessageMouse* pMsg) {
  if (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)
    return;

  m_bLButtonDown = true;
  SetGrab(true);
  DoButtonDown(pMsg);
  int32_t nIndex = m_EdtEngine.GetCaretPos();
  bool bRepaint = false;
  if (m_EdtEngine.CountSelRanges() > 0) {
    m_EdtEngine.ClearSelection();
    bRepaint = true;
  }

  if ((pMsg->m_dwFlags & FWL_KEYFLAG_Shift) && m_nSelStart != nIndex) {
    int32_t iStart = std::min(m_nSelStart, nIndex);
    int32_t iEnd = std::max(m_nSelStart, nIndex);
    m_EdtEngine.AddSelRange(iStart, iEnd - iStart);
    bRepaint = true;
  } else {
    m_nSelStart = nIndex;
  }
  if (bRepaint)
    RepaintRect(m_rtEngine);
}

void CFWL_Edit::OnLButtonUp(CFWL_MessageMouse* pMsg) {
  m_bLButtonDown = false;
  SetGrab(false);
}

void CFWL_Edit::OnButtonDblClk(CFWL_MessageMouse* pMsg) {
  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return;

  int32_t nCount = 0;
  int32_t nIndex = pPage->SelectWord(DeviceToEngine(pMsg->m_pos), nCount);
  if (nIndex < 0)
    return;

  m_EdtEngine.AddSelRange(nIndex, nCount);
  m_EdtEngine.SetCaretPos(nIndex + nCount - 1, false);
  RepaintRect(m_rtEngine);
}

void CFWL_Edit::OnMouseMove(CFWL_MessageMouse* pMsg) {
  if (m_nSelStart == -1 || !m_bLButtonDown)
    return;

  IFDE_TxtEdtPage* pPage = m_EdtEngine.GetPage(0);
  if (!pPage)
    return;

  bool bBefore = true;
  int32_t nIndex = pPage->GetCharIndex(DeviceToEngine(pMsg->m_pos), bBefore);
  m_EdtEngine.SetCaretPos(nIndex, bBefore);
  nIndex = m_EdtEngine.GetCaretPos();
  m_EdtEngine.ClearSelection();

  if (nIndex == m_nSelStart)
    return;

  int32_t nLen = m_EdtEngine.GetTextLength();
  if (m_nSelStart >= nLen)
    m_nSelStart = nLen;

  m_EdtEngine.AddSelRange(std::min(m_nSelStart, nIndex),
                          FXSYS_abs(nIndex - m_nSelStart));
}

void CFWL_Edit::OnKeyDown(CFWL_MessageKey* pMsg) {
  FDE_TXTEDTMOVECARET MoveCaret = MC_MoveNone;
  bool bShift = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Shift);
  bool bCtrl = !!(pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl);
  uint32_t dwKeyCode = pMsg->m_dwKeyCode;
  switch (dwKeyCode) {
    case FWL_VKEY_Left: {
      MoveCaret = MC_Left;
      break;
    }
    case FWL_VKEY_Right: {
      MoveCaret = MC_Right;
      break;
    }
    case FWL_VKEY_Up: {
      MoveCaret = MC_Up;
      break;
    }
    case FWL_VKEY_Down: {
      MoveCaret = MC_Down;
      break;
    }
    case FWL_VKEY_Home: {
      MoveCaret = bCtrl ? MC_Home : MC_LineStart;
      break;
    }
    case FWL_VKEY_End: {
      MoveCaret = bCtrl ? MC_End : MC_LineEnd;
      break;
    }
    case FWL_VKEY_Insert:
      break;
    case FWL_VKEY_Delete: {
      if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly) ||
          (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
        break;
      }
      int32_t nCaret = m_EdtEngine.GetCaretPos();
#if (_FX_OS_ == _FX_MACOSX_)
      m_EdtEngine.Delete(nCaret, true);
#else
      m_EdtEngine.Delete(nCaret);
#endif
      break;
    }
    case FWL_VKEY_F2:
    case FWL_VKEY_Tab:
    default:
      break;
  }
  if (MoveCaret != MC_MoveNone)
    m_EdtEngine.MoveCaretPos(MoveCaret, bShift, bCtrl);
}

void CFWL_Edit::OnChar(CFWL_MessageKey* pMsg) {
  if ((m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_ReadOnly) ||
      (m_pProperties->m_dwStates & FWL_WGTSTATE_Disabled)) {
    return;
  }

  int32_t iError = 0;
  FX_WCHAR c = static_cast<FX_WCHAR>(pMsg->m_dwKeyCode);
  int32_t nCaret = m_EdtEngine.GetCaretPos();
  switch (c) {
    case FWL_VKEY_Back:
      m_EdtEngine.Delete(nCaret, true);
      break;
    case FWL_VKEY_NewLine:
    case FWL_VKEY_Escape:
      break;
    case FWL_VKEY_Tab: {
      iError = m_EdtEngine.Insert(nCaret, L"\t", 1);
      break;
    }
    case FWL_VKEY_Return: {
      if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_WantReturn) {
        iError = m_EdtEngine.Insert(nCaret, L"\n", 1);
      }
      break;
    }
    default: {
      if (!m_pWidgetMgr->IsFormDisabled()) {
        if (m_pProperties->m_dwStyleExes & FWL_STYLEEXT_EDT_Number) {
          if (((pMsg->m_dwKeyCode < FWL_VKEY_0) &&
               (pMsg->m_dwKeyCode != 0x2E && pMsg->m_dwKeyCode != 0x2D)) ||
              pMsg->m_dwKeyCode > FWL_VKEY_9) {
            break;
          }
          if (!ValidateNumberChar(c))
            break;
        }
      }
#if (_FX_OS_ == _FX_MACOSX_)
      if (pMsg->m_dwFlags & FWL_KEYFLAG_Command)
#else
      if (pMsg->m_dwFlags & FWL_KEYFLAG_Ctrl)
#endif
      {
        break;
      }
      iError = m_EdtEngine.Insert(nCaret, &c, 1);
      break;
    }
  }
  if (iError < 0)
    ProcessInsertError(iError);
}

bool CFWL_Edit::OnScroll(CFWL_ScrollBar* pScrollBar,
                         CFWL_EventScroll::Code dwCode,
                         FX_FLOAT fPos) {
  CFX_SizeF fs;
  pScrollBar->GetRange(&fs.width, &fs.height);
  FX_FLOAT iCurPos = pScrollBar->GetPos();
  FX_FLOAT fStep = pScrollBar->GetStepSize();
  switch (dwCode) {
    case CFWL_EventScroll::Code::Min: {
      fPos = fs.width;
      break;
    }
    case CFWL_EventScroll::Code::Max: {
      fPos = fs.height;
      break;
    }
    case CFWL_EventScroll::Code::StepBackward: {
      fPos -= fStep;
      if (fPos < fs.width + fStep / 2) {
        fPos = fs.width;
      }
      break;
    }
    case CFWL_EventScroll::Code::StepForward: {
      fPos += fStep;
      if (fPos > fs.height - fStep / 2) {
        fPos = fs.height;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageBackward: {
      fPos -= pScrollBar->GetPageSize();
      if (fPos < fs.width) {
        fPos = fs.width;
      }
      break;
    }
    case CFWL_EventScroll::Code::PageForward: {
      fPos += pScrollBar->GetPageSize();
      if (fPos > fs.height) {
        fPos = fs.height;
      }
      break;
    }
    case CFWL_EventScroll::Code::Pos:
    case CFWL_EventScroll::Code::TrackPos:
    case CFWL_EventScroll::Code::None:
      break;
    case CFWL_EventScroll::Code::EndScroll:
      return false;
  }
  if (iCurPos == fPos)
    return true;

  pScrollBar->SetPos(fPos);
  pScrollBar->SetTrackPos(fPos);
  UpdateOffset(pScrollBar, fPos - iCurPos);
  UpdateCaret();

  CFX_RectF rect = GetWidgetRect();
  RepaintRect(CFX_RectF(0, 0, rect.width + 2, rect.height + 2));
  return true;
}
