// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fde_render.h"
void FDE_GetPageMatrix(CFX_Matrix& pageMatrix,
                       const CFX_RectF& docPageRect,
                       const CFX_Rect& devicePageRect,
                       int32_t iRotate,
                       FX_DWORD dwCoordinatesType) {
  FXSYS_assert(iRotate >= 0 && iRotate <= 3);
  FX_BOOL bFlipX = (dwCoordinatesType & 0x01) != 0;
  FX_BOOL bFlipY = (dwCoordinatesType & 0x02) != 0;
  CFX_Matrix m;
  m.Set((bFlipX ? -1.0f : 1.0f), 0, 0, (bFlipY ? -1.0f : 1.0f), 0, 0);
  if (iRotate == 0 || iRotate == 2) {
    m.a *= (FX_FLOAT)devicePageRect.width / docPageRect.width;
    m.d *= (FX_FLOAT)devicePageRect.height / docPageRect.height;
  } else {
    m.a *= (FX_FLOAT)devicePageRect.height / docPageRect.width;
    m.d *= (FX_FLOAT)devicePageRect.width / docPageRect.height;
  }
  m.Rotate(iRotate * 1.57079632675f);
  switch (iRotate) {
    case 0:
      m.e = bFlipX ? (FX_FLOAT)devicePageRect.right()
                   : (FX_FLOAT)devicePageRect.left;
      m.f = bFlipY ? (FX_FLOAT)devicePageRect.bottom()
                   : (FX_FLOAT)devicePageRect.top;
      break;
    case 1:
      m.e = bFlipY ? (FX_FLOAT)devicePageRect.left
                   : (FX_FLOAT)devicePageRect.right();
      m.f = bFlipX ? (FX_FLOAT)devicePageRect.bottom()
                   : (FX_FLOAT)devicePageRect.top;
      break;
    case 2:
      m.e = bFlipX ? (FX_FLOAT)devicePageRect.left
                   : (FX_FLOAT)devicePageRect.right();
      m.f = bFlipY ? (FX_FLOAT)devicePageRect.top
                   : (FX_FLOAT)devicePageRect.bottom();
      break;
    case 3:
      m.e = bFlipY ? (FX_FLOAT)devicePageRect.right()
                   : (FX_FLOAT)devicePageRect.left;
      m.f = bFlipX ? (FX_FLOAT)devicePageRect.top
                   : (FX_FLOAT)devicePageRect.bottom();
      break;
    default:
      break;
  }
  pageMatrix = m;
}
IFDE_RenderContext* IFDE_RenderContext::Create() {
  return new CFDE_RenderContext;
}
CFDE_RenderContext::CFDE_RenderContext()
    : CFX_ThreadLock(),
      m_eStatus(FDE_RENDERSTATUS_Reset),
      m_pRenderDevice(NULL),
      m_pSolidBrush(NULL),
      m_Transform(),
      m_pCharPos(NULL),
      m_iCharPosCount(0),
      m_pIterator(NULL) {
  m_Transform.SetIdentity();
}
CFDE_RenderContext::~CFDE_RenderContext() {
  StopRender();
}
FX_BOOL CFDE_RenderContext::StartRender(IFDE_RenderDevice* pRenderDevice,
                                        IFDE_CanvasSet* pCanvasSet,
                                        const CFX_Matrix& tmDoc2Device) {
  if (m_pRenderDevice != NULL) {
    return FALSE;
  }
  if (pRenderDevice == NULL) {
    return FALSE;
  }
  if (pCanvasSet == NULL) {
    return FALSE;
  }
  Lock();
  m_eStatus = FDE_RENDERSTATUS_Paused;
  m_pRenderDevice = pRenderDevice;
  m_Transform = tmDoc2Device;
  if (m_pIterator == NULL) {
    m_pIterator = IFDE_VisualSetIterator::Create();
    FXSYS_assert(m_pIterator != NULL);
  }
  FX_BOOL bAttach =
      m_pIterator->AttachCanvas(pCanvasSet) && m_pIterator->FilterObjects();
  Unlock();
  return bAttach;
}
FDE_RENDERSTATUS CFDE_RenderContext::DoRender(IFX_Pause* pPause) {
  if (m_pRenderDevice == NULL) {
    return FDE_RENDERSTATUS_Failed;
  }
  if (m_pIterator == NULL) {
    return FDE_RENDERSTATUS_Failed;
  }
  Lock();
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
    if (hVisualObj == NULL || pVisualSet == NULL) {
      eStatus = FDE_RENDERSTATUS_Done;
      break;
    }
    rtObj.Empty();
    pVisualSet->GetRect(hVisualObj, rtObj);
    if (!rtDocClip.IntersectWith(rtObj)) {
      continue;
    }
    switch (pVisualSet->GetType()) {
      case FDE_VISUALOBJ_Text:
        RenderText((IFDE_TextSet*)pVisualSet, hVisualObj);
        iCount += 5;
        break;
      case FDE_VISUALOBJ_Path:
        RenderPath((IFDE_PathSet*)pVisualSet, hVisualObj);
        iCount += 20;
        break;
      case FDE_VISUALOBJ_Widget:
        iCount += 10;
        break;
      case FDE_VISUALOBJ_Canvas:
        FXSYS_assert(FALSE);
        break;
      default:
        break;
    }
    if (iCount >= 100 && pPause != NULL && pPause->NeedToPauseNow()) {
      eStatus = FDE_RENDERSTATUS_Paused;
      break;
    }
  }
  Unlock();
  return m_eStatus = eStatus;
}
void CFDE_RenderContext::StopRender() {
  Lock();
  m_eStatus = FDE_RENDERSTATUS_Reset;
  m_pRenderDevice = nullptr;
  m_Transform.SetIdentity();
  if (m_pIterator) {
    m_pIterator->Release();
    m_pIterator = nullptr;
  }
  if (m_pSolidBrush) {
    m_pSolidBrush->Release();
    m_pSolidBrush = nullptr;
  }
  FX_Free(m_pCharPos);
  m_pCharPos = nullptr;
  m_iCharPosCount = 0;
  Unlock();
}
void CFDE_RenderContext::RenderText(IFDE_TextSet* pTextSet,
                                    FDE_HVISUALOBJ hText) {
  FXSYS_assert(m_pRenderDevice != NULL);
  FXSYS_assert(pTextSet != NULL && hText != NULL);
  IFX_Font* pFont = pTextSet->GetFont(hText);
  if (pFont == NULL) {
    return;
  }
  int32_t iCount = pTextSet->GetDisplayPos(hText, NULL, FALSE);
  if (iCount < 1) {
    return;
  }
  if (m_pSolidBrush == NULL) {
    m_pSolidBrush = (IFDE_SolidBrush*)IFDE_Brush::Create(FDE_BRUSHTYPE_Solid);
    if (m_pSolidBrush == NULL) {
      return;
    }
  }
  if (m_pCharPos == NULL) {
    m_pCharPos = FX_Alloc(FXTEXT_CHARPOS, iCount);
  } else if (m_iCharPosCount < iCount) {
    m_pCharPos = FX_Realloc(FXTEXT_CHARPOS, m_pCharPos, iCount);
  }
  if (m_iCharPosCount < iCount) {
    m_iCharPosCount = iCount;
  }
  iCount = pTextSet->GetDisplayPos(hText, m_pCharPos, FALSE);
  FX_FLOAT fFontSize = pTextSet->GetFontSize(hText);
  FX_ARGB dwColor = pTextSet->GetFontColor(hText);
  m_pSolidBrush->SetColor(dwColor);
  FDE_HDEVICESTATE hState;
  FX_BOOL bClip = ApplyClip(pTextSet, hText, hState);
  m_pRenderDevice->DrawString(m_pSolidBrush, pFont, m_pCharPos, iCount,
                              fFontSize, &m_Transform);
  if (bClip) {
    RestoreClip(hState);
  }
}
void CFDE_RenderContext::RenderPath(IFDE_PathSet* pPathSet,
                                    FDE_HVISUALOBJ hPath) {
  FXSYS_assert(m_pRenderDevice != NULL);
  FXSYS_assert(pPathSet != NULL && hPath != NULL);
  IFDE_Path* pPath = pPathSet->GetPath(hPath);
  if (pPath == NULL) {
    return;
  }
  FDE_HDEVICESTATE hState;
  FX_BOOL bClip = ApplyClip(pPathSet, hPath, hState);
  int32_t iRenderMode = pPathSet->GetRenderMode(hPath);
  if (iRenderMode & FDE_PATHRENDER_Stroke) {
    IFDE_Pen* pPen = pPathSet->GetPen(hPath);
    FX_FLOAT fWidth = pPathSet->GetPenWidth(hPath);
    if (pPen != NULL && fWidth > 0) {
      m_pRenderDevice->DrawPath(pPen, fWidth, pPath, &m_Transform);
    }
  }
  if (iRenderMode & FDE_PATHRENDER_Fill) {
    IFDE_Brush* pBrush = pPathSet->GetBrush(hPath);
    if (pBrush != NULL) {
      m_pRenderDevice->FillPath(pBrush, pPath, &m_Transform);
    }
  }
  if (bClip) {
    RestoreClip(hState);
  }
}
FX_BOOL CFDE_RenderContext::ApplyClip(IFDE_VisualSet* pVisualSet,
                                      FDE_HVISUALOBJ hObj,
                                      FDE_HDEVICESTATE& hState) {
  CFX_RectF rtClip;
  if (!pVisualSet->GetClip(hObj, rtClip)) {
    return FALSE;
  }
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
