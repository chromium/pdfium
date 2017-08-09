// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_renderdevice.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_renderdevice.h"
#include "core/fxge/cfx_substfont.h"
#include "core/fxge/dib/cfx_imagerenderer.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/cfde_path.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fgas/font/cfgas_gefont.h"

CFDE_RenderDevice::CFDE_RenderDevice(CFX_RenderDevice* pDevice)
    : m_pDevice(pDevice) {
  ASSERT(pDevice);

  FX_RECT rt = m_pDevice->GetClipBox();
  m_rtClip = CFX_RectF(static_cast<float>(rt.left), static_cast<float>(rt.top),
                       static_cast<float>(rt.Width()),
                       static_cast<float>(rt.Height()));
}

CFDE_RenderDevice::~CFDE_RenderDevice() {}

int32_t CFDE_RenderDevice::GetWidth() const {
  return m_pDevice->GetWidth();
}

int32_t CFDE_RenderDevice::GetHeight() const {
  return m_pDevice->GetHeight();
}

void CFDE_RenderDevice::SaveState() {
  m_pDevice->SaveState();
}

void CFDE_RenderDevice::RestoreState() {
  m_pDevice->RestoreState(false);
  const FX_RECT& rt = m_pDevice->GetClipBox();
  m_rtClip = CFX_RectF(static_cast<float>(rt.left), static_cast<float>(rt.top),
                       static_cast<float>(rt.Width()),
                       static_cast<float>(rt.Height()));
}

bool CFDE_RenderDevice::SetClipRect(const CFX_RectF& rtClip) {
  m_rtClip = rtClip;
  return m_pDevice->SetClip_Rect(
      FX_RECT((int32_t)floor(rtClip.left), (int32_t)floor(rtClip.top),
              (int32_t)ceil(rtClip.right()), (int32_t)ceil(rtClip.bottom())));
}

const CFX_RectF& CFDE_RenderDevice::GetClipRect() {
  return m_rtClip;
}

bool CFDE_RenderDevice::DrawString(FX_ARGB color,
                                   const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                                   const FXTEXT_CHARPOS* pCharPos,
                                   int32_t iCount,
                                   float fFontSize,
                                   const CFX_Matrix* pMatrix) {
  ASSERT(pFont && pCharPos && iCount > 0);
  CFX_Font* pFxFont = pFont->GetDevFont();
  if ((pFont->GetFontStyles() & FX_FONTSTYLE_Italic) != 0 &&
      !pFxFont->IsItalic()) {
    FXTEXT_CHARPOS* pCP = (FXTEXT_CHARPOS*)pCharPos;
    float* pAM;
    for (int32_t i = 0; i < iCount; ++i) {
      static const float mc = 0.267949f;
      pAM = pCP->m_AdjustMatrix;
      pAM[2] = mc * pAM[0] + pAM[2];
      pAM[3] = mc * pAM[1] + pAM[3];
      pCP++;
    }
  }
  FXTEXT_CHARPOS* pCP = (FXTEXT_CHARPOS*)pCharPos;
  CFX_RetainPtr<CFGAS_GEFont> pCurFont;
  CFX_RetainPtr<CFGAS_GEFont> pSTFont;
  FXTEXT_CHARPOS* pCurCP = nullptr;
  int32_t iCurCount = 0;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  uint32_t dwFontStyle = pFont->GetFontStyles();
  CFX_Font FxFont;
  auto SubstFxFont = pdfium::MakeUnique<CFX_SubstFont>();
  SubstFxFont->m_Weight = dwFontStyle & FX_FONTSTYLE_Bold ? 700 : 400;
  SubstFxFont->m_ItalicAngle = dwFontStyle & FX_FONTSTYLE_Italic ? -12 : 0;
  SubstFxFont->m_WeightCJK = SubstFxFont->m_Weight;
  SubstFxFont->m_bItalicCJK = !!(dwFontStyle & FX_FONTSTYLE_Italic);
  FxFont.SetSubstFont(std::move(SubstFxFont));
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

  for (int32_t i = 0; i < iCount; ++i) {
    pSTFont = pFont->GetSubstFont((int32_t)pCP->m_GlyphIndex);
    pCP->m_GlyphIndex &= 0x00FFFFFF;
    pCP->m_bFontStyle = false;
    if (pCurFont != pSTFont) {
      if (pCurFont) {
        pFxFont = pCurFont->GetDevFont();
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
        FxFont.SetFace(pFxFont->GetFace());
        m_pDevice->DrawNormalText(iCurCount, pCurCP, &FxFont, -fFontSize,
                                  pMatrix, color, FXTEXT_CLEARTYPE);
#else
        m_pDevice->DrawNormalText(iCurCount, pCurCP, pFxFont, -fFontSize,
                                  pMatrix, color, FXTEXT_CLEARTYPE);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
      }
      pCurFont = pSTFont;
      pCurCP = pCP;
      iCurCount = 1;
    } else {
      iCurCount++;
    }
    pCP++;
  }
  if (pCurFont && iCurCount) {
    pFxFont = pCurFont->GetDevFont();
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
    FxFont.SetFace(pFxFont->GetFace());
    bool bRet =
        m_pDevice->DrawNormalText(iCurCount, pCurCP, &FxFont, -fFontSize,
                                  pMatrix, color, FXTEXT_CLEARTYPE);
    FxFont.SetFace(nullptr);
    return bRet;
#else
    return m_pDevice->DrawNormalText(iCurCount, pCurCP, pFxFont, -fFontSize,
                                     pMatrix, color, FXTEXT_CLEARTYPE);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  }

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  FxFont.SetFace(nullptr);
#endif  // _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_

  return true;
}

bool CFDE_RenderDevice::DrawPath(FX_ARGB color,
                                 float fPenWidth,
                                 const CFDE_Path* pPath,
                                 const CFX_Matrix* pMatrix) {
  CFDE_Path* pGePath = (CFDE_Path*)pPath;
  if (!pGePath)
    return false;

  CFX_GraphStateData graphState;
  graphState.m_LineCap = CFX_GraphStateData::LineCapButt;
  graphState.m_LineJoin = CFX_GraphStateData::LineJoinMiter;
  graphState.m_LineWidth = fPenWidth;
  graphState.m_MiterLimit = 10;
  graphState.m_DashPhase = 0;
  return m_pDevice->DrawPath(&pGePath->m_Path, (const CFX_Matrix*)pMatrix,
                             &graphState, 0, color, 0);
}
