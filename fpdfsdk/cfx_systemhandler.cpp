// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cfx_systemhandler.h"

#include <memory>

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fxge/include/cfx_fontmapper.h"
#include "core/fxge/include/cfx_fontmgr.h"
#include "core/fxge/include/cfx_gemodule.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/include/cpdfsdk_annot.h"
#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_pageview.h"
#include "fpdfsdk/include/cpdfsdk_widget.h"

namespace {

int CharSet2CP(int charset) {
  if (charset == FXFONT_SHIFTJIS_CHARSET)
    return 932;
  if (charset == FXFONT_GB2312_CHARSET)
    return 936;
  if (charset == FXFONT_HANGUL_CHARSET)
    return 949;
  if (charset == FXFONT_CHINESEBIG5_CHARSET)
    return 950;
  return 0;
}

}  // namespace

void CFX_SystemHandler::InvalidateRect(CPDFSDK_Widget* widget, FX_RECT rect) {
  CPDFSDK_PageView* pPageView = widget->GetPageView();
  UnderlyingPageType* pPage = widget->GetUnderlyingPage();
  if (!pPage || !pPageView)
    return;

  CFX_Matrix page2device;
  pPageView->GetCurrentMatrix(page2device);

  CFX_Matrix device2page;
  device2page.SetReverse(page2device);

  FX_FLOAT left;
  FX_FLOAT top;
  FX_FLOAT right;
  FX_FLOAT bottom;
  device2page.Transform(static_cast<FX_FLOAT>(rect.left),
                        static_cast<FX_FLOAT>(rect.top), left, top);
  device2page.Transform(static_cast<FX_FLOAT>(rect.right),
                        static_cast<FX_FLOAT>(rect.bottom), right, bottom);
  CFX_FloatRect rcPDF(left, bottom, right, top);
  rcPDF.Normalize();

  m_pEnv->Invalidate(pPage, rcPDF.left, rcPDF.top, rcPDF.right, rcPDF.bottom);
}

void CFX_SystemHandler::OutputSelectedRect(CFFL_FormFiller* pFormFiller,
                                           CFX_FloatRect& rect) {
  if (!pFormFiller)
    return;

  CFX_FloatPoint leftbottom = CFX_FloatPoint(rect.left, rect.bottom);
  CFX_FloatPoint righttop = CFX_FloatPoint(rect.right, rect.top);
  CFX_FloatPoint ptA = pFormFiller->PWLtoFFL(leftbottom);
  CFX_FloatPoint ptB = pFormFiller->PWLtoFFL(righttop);

  CPDFSDK_Annot* pAnnot = pFormFiller->GetSDKAnnot();
  UnderlyingPageType* pPage = pAnnot->GetUnderlyingPage();
  ASSERT(pPage);

  m_pEnv->OutputSelectedRect(pPage, ptA.x, ptB.y, ptB.x, ptA.y);
}

bool CFX_SystemHandler::IsSelectionImplemented() const {
  if (!m_pEnv)
    return false;

  FPDF_FORMFILLINFO* pInfo = m_pEnv->GetFormFillInfo();
  return pInfo && pInfo->FFI_OutputSelectedRect;
}

void CFX_SystemHandler::SetCursor(int32_t nCursorType) {
  m_pEnv->SetCursor(nCursorType);
}

bool CFX_SystemHandler::FindNativeTrueTypeFont(CFX_ByteString sFontFaceName) {
  CFX_FontMgr* pFontMgr = CFX_GEModule::Get()->GetFontMgr();
  if (!pFontMgr)
    return false;

  CFX_FontMapper* pFontMapper = pFontMgr->GetBuiltinMapper();
  if (!pFontMapper)
    return false;

  if (pFontMapper->m_InstalledTTFonts.empty())
    pFontMapper->LoadInstalledFonts();

  for (const auto& font : pFontMapper->m_InstalledTTFonts) {
    if (font.Compare(sFontFaceName.AsStringC()))
      return true;
  }
  return false;
}

CPDF_Font* CFX_SystemHandler::AddNativeTrueTypeFontToPDF(
    CPDF_Document* pDoc,
    CFX_ByteString sFontFaceName,
    uint8_t nCharset) {
  if (!pDoc)
    return nullptr;

  std::unique_ptr<CFX_Font> pFXFont(new CFX_Font);
  pFXFont->LoadSubst(sFontFaceName, TRUE, 0, 0, 0, CharSet2CP(nCharset), FALSE);
  return pDoc->AddFont(pFXFont.get(), nCharset, FALSE);
}

int32_t CFX_SystemHandler::SetTimer(int32_t uElapse,
                                    TimerCallback lpTimerFunc) {
  return m_pEnv->SetTimer(uElapse, lpTimerFunc);
}

void CFX_SystemHandler::KillTimer(int32_t nID) {
  m_pEnv->KillTimer(nID);
}

bool CFX_SystemHandler::IsSHIFTKeyDown(uint32_t nFlag) const {
  return !!m_pEnv->IsSHIFTKeyDown(nFlag);
}

bool CFX_SystemHandler::IsCTRLKeyDown(uint32_t nFlag) const {
  return !!m_pEnv->IsCTRLKeyDown(nFlag);
}

bool CFX_SystemHandler::IsALTKeyDown(uint32_t nFlag) const {
  return !!m_pEnv->IsALTKeyDown(nFlag);
}
