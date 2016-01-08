// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fde_iterator.h"
IFDE_VisualSetIterator* IFDE_VisualSetIterator::Create() {
  return new CFDE_VisualSetIterator;
}
CFDE_VisualSetIterator::CFDE_VisualSetIterator() : m_dwFilter(0) {}
CFDE_VisualSetIterator::~CFDE_VisualSetIterator() {
  m_CanvasStack.RemoveAll();
}
FX_BOOL CFDE_VisualSetIterator::AttachCanvas(IFDE_CanvasSet* pCanvas) {
  FXSYS_assert(pCanvas != NULL);
  m_CanvasStack.RemoveAll();
  FDE_CANVASITEM canvas;
  canvas.hCanvas = NULL;
  canvas.pCanvas = pCanvas;
  canvas.hPos = pCanvas->GetFirstPosition(NULL);
  if (canvas.hPos == NULL) {
    return FALSE;
  }
  return m_CanvasStack.Push(canvas) == 0;
}
FX_BOOL CFDE_VisualSetIterator::FilterObjects(FX_DWORD dwObjects) {
  if (m_CanvasStack.GetSize() == 0) {
    return FALSE;
  }
  while (m_CanvasStack.GetSize() > 1) {
    m_CanvasStack.Pop();
  }
  m_dwFilter = dwObjects & ~(FX_DWORD)FDE_VISUALOBJ_Widget;
  if (dwObjects & FDE_VISUALOBJ_Widget) {
    m_dwFilter |= 0xFF00;
  }
  FDE_LPCANVASITEM pCanvas = m_CanvasStack.GetTopElement();
  FXSYS_assert(pCanvas != NULL && pCanvas->pCanvas != NULL);
  pCanvas->hPos = pCanvas->pCanvas->GetFirstPosition(NULL);
  return pCanvas->hPos != NULL;
}
void CFDE_VisualSetIterator::Reset() {
  FilterObjects(m_dwFilter);
}
FDE_HVISUALOBJ CFDE_VisualSetIterator::GetNext(IFDE_VisualSet*& pVisualSet,
                                               FDE_HVISUALOBJ* phCanvasObj,
                                               IFDE_CanvasSet** ppCanvasSet) {
  while (m_CanvasStack.GetSize() > 0) {
    FDE_LPCANVASITEM pCanvas = m_CanvasStack.GetTopElement();
    FXSYS_assert(pCanvas != NULL && pCanvas->pCanvas != NULL);
    if (pCanvas->hPos == NULL) {
      if (m_CanvasStack.GetSize() == 1) {
        break;
      }
      m_CanvasStack.Pop();
      continue;
    }
    do {
      FDE_HVISUALOBJ hObj = pCanvas->pCanvas->GetNext(
          pCanvas->hCanvas, pCanvas->hPos, pVisualSet);
      FXSYS_assert(hObj != NULL);
      FDE_VISUALOBJTYPE eType = pVisualSet->GetType();
      if (eType == FDE_VISUALOBJ_Canvas) {
        FDE_CANVASITEM canvas;
        canvas.hCanvas = hObj;
        canvas.pCanvas = (IFDE_CanvasSet*)pVisualSet;
        canvas.hPos = canvas.pCanvas->GetFirstPosition(hObj);
        m_CanvasStack.Push(canvas);
        break;
      }
      FX_DWORD dwObj =
          (eType == FDE_VISUALOBJ_Widget)
              ? (FX_DWORD)((IFDE_WidgetSet*)pVisualSet)->GetWidgetType(hObj)
              : (FX_DWORD)eType;
      if ((m_dwFilter & dwObj) != 0) {
        if (ppCanvasSet) {
          *ppCanvasSet = pCanvas->pCanvas;
        }
        if (phCanvasObj) {
          *phCanvasObj = pCanvas->hCanvas;
        }
        return hObj;
      }
    } while (pCanvas->hPos != NULL);
  }
  if (ppCanvasSet) {
    *ppCanvasSet = NULL;
  }
  if (phCanvasObj) {
    *phCanvasObj = NULL;
  }
  pVisualSet = NULL;
  return NULL;
}
