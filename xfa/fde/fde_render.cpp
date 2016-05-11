// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/fde_render.h"

#include "xfa/fde/fde_gedevice.h"
#include "xfa/fde/fde_object.h"
#include "xfa/fgas/crt/fgas_memory.h"

#define FDE_PATHRENDER_Stroke 1
#define FDE_PATHRENDER_Fill 2

CFDE_RenderContext::CFDE_RenderContext()
    : m_eStatus(FDE_RENDERSTATUS_Reset),
      m_pRenderDevice(nullptr),
      m_Transform(),
      m_pCharPos(nullptr),
      m_iCharPosCount(0) {
  m_Transform.SetIdentity();
}

CFDE_RenderContext::~CFDE_RenderContext() {
  StopRender();
}

FX_BOOL CFDE_RenderContext::StartRender(CFDE_RenderDevice* pRenderDevice,
                                        IFDE_CanvasSet* pCanvasSet,
                                        const CFX_Matrix& tmDoc2Device) {
  if (m_pRenderDevice)
    return FALSE;
  if (!pRenderDevice)
    return FALSE;
  if (!pCanvasSet)
    return FALSE;

  m_eStatus = FDE_RENDERSTATUS_Paused;
  m_pRenderDevice = pRenderDevice;
  m_Transform = tmDoc2Device;
  if (!m_pIterator)
    m_pIterator.reset(new CFDE_VisualSetIterator);

  return m_pIterator->AttachCanvas(pCanvasSet) && m_pIterator->FilterObjects();
}

FDE_RENDERSTATUS CFDE_RenderContext::DoRender(IFX_Pause* pPause) {
  if (!m_pRenderDevice)
    return FDE_RENDERSTATUS_Failed;
  if (!m_pIterator)
    return FDE_RENDERSTATUS_Failed;

  FDE_RENDERSTATUS eStatus = FDE_RENDERSTATUS_Paused;
  CFX_Matrix rm;
  rm.SetReverse(m_Transform);
  CFX_RectF rtDocClip = m_pRenderDevice->GetClipRect();
  if (rtDocClip.IsEmpty()) {
    rtDocClip.left = rtDocClip.top = 0;
    rtDocClip.width = (FX_FLOAT)m_pRenderDevice->GetWidth();
    rtDocClip.height = (FX_FLOAT)m_pRenderDevice->GetHeight();
  }
  rm.TransformRect(rtDocClip);
  IFDE_VisualSet* pVisualSet;
  FDE_HVISUALOBJ hVisualObj;
  CFX_RectF rtObj;
  int32_t iCount = 0;
  while (TRUE) {
    hVisualObj = m_pIterator->GetNext(pVisualSet);
    if (!hVisualObj || !pVisualSet) {
      eStatus = FDE_RENDERSTATUS_Done;
      break;
    }
    rtObj.Empty();
    pVisualSet->GetRect(hVisualObj, rtObj);
    if (!rtDocClip.IntersectWith(rtObj))
      continue;

    switch (pVisualSet->GetType()) {
      case FDE_VISUALOBJ_Text:
        RenderText((IFDE_TextSet*)pVisualSet, hVisualObj);
        iCount += 5;
        break;
      case FDE_VISUALOBJ_Canvas:
        ASSERT(FALSE);
        break;
      default:
        break;
    }
    if (iCount >= 100 && pPause && pPause->NeedToPauseNow()) {
      eStatus = FDE_RENDERSTATUS_Paused;
      break;
    }
  }
  return m_eStatus = eStatus;
}

void CFDE_RenderContext::StopRender() {
  m_eStatus = FDE_RENDERSTATUS_Reset;
  m_pRenderDevice = nullptr;
  m_Transform.SetIdentity();
  m_pIterator.reset();
  m_pBrush.reset();
  FX_Free(m_pCharPos);
  m_pCharPos = nullptr;
  m_iCharPosCount = 0;
}

void CFDE_RenderContext::RenderText(IFDE_TextSet* pTextSet,
                                    FDE_HVISUALOBJ hText) {
  ASSERT(m_pRenderDevice);
  ASSERT(pTextSet && hText);

  IFX_Font* pFont = pTextSet->GetFont(hText);
  if (!pFont)
    return;

  int32_t iCount = pTextSet->GetDisplayPos(hText, nullptr, FALSE);
  if (iCount < 1)
    return;

  if (!m_pBrush)
    m_pBrush.reset(new CFDE_Brush);

  if (!m_pCharPos)
    m_pCharPos = FX_Alloc(FXTEXT_CHARPOS, iCount);
  else if (m_iCharPosCount < iCount)
    m_pCharPos = FX_Realloc(FXTEXT_CHARPOS, m_pCharPos, iCount);

  if (m_iCharPosCount < iCount)
    m_iCharPosCount = iCount;

  iCount = pTextSet->GetDisplayPos(hText, m_pCharPos, FALSE);
  FX_FLOAT fFontSize = pTextSet->GetFontSize(hText);
  FX_ARGB dwColor = pTextSet->GetFontColor(hText);
  m_pBrush->SetColor(dwColor);
  FDE_HDEVICESTATE hState;
  FX_BOOL bClip = ApplyClip(pTextSet, hText, hState);
  m_pRenderDevice->DrawString(m_pBrush.get(), pFont, m_pCharPos, iCount,
                              fFontSize, &m_Transform);
  if (bClip)
    RestoreClip(hState);
}

FX_BOOL CFDE_RenderContext::ApplyClip(IFDE_VisualSet* pVisualSet,
                                      FDE_HVISUALOBJ hObj,
                                      FDE_HDEVICESTATE& hState) {
  CFX_RectF rtClip;
  if (!pVisualSet->GetClip(hObj, rtClip))
    return FALSE;

  CFX_RectF rtObj;
  pVisualSet->GetRect(hObj, rtObj);
  rtClip.Offset(rtObj.left, rtObj.top);
  m_Transform.TransformRect(rtClip);
  const CFX_RectF& rtDevClip = m_pRenderDevice->GetClipRect();
  rtClip.Intersect(rtDevClip);
  hState = m_pRenderDevice->SaveState();
  return m_pRenderDevice->SetClipRect(rtClip);
}

void CFDE_RenderContext::RestoreClip(FDE_HDEVICESTATE hState) {
  m_pRenderDevice->RestoreState(hState);
}
