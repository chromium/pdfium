// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_ITERATOR_H_
#define XFA_FDE_FDE_ITERATOR_H_

#include "xfa/fde/fde_visualset.h"
#include "xfa/fgas/crt/fgas_memory.h"

struct FDE_CANVASITEM {
  IFDE_CanvasSet* pCanvas;
  FDE_HVISUALOBJ hCanvas;
  FX_POSITION hPos;
};

class CFDE_VisualSetIterator : public IFDE_VisualSetIterator,
                               public CFX_Target {
 public:
  CFDE_VisualSetIterator();
  ~CFDE_VisualSetIterator();
  virtual void Release() { delete this; }

  virtual FX_BOOL AttachCanvas(IFDE_CanvasSet* pCanvas);
  virtual FX_BOOL FilterObjects(uint32_t dwObjects = 0xFFFFFFFF);

  virtual void Reset();
  virtual FDE_HVISUALOBJ GetNext(IFDE_VisualSet*& pVisualSet,
                                 FDE_HVISUALOBJ* phCanvasObj = NULL,
                                 IFDE_CanvasSet** ppCanvasSet = NULL);

 protected:
  uint32_t m_dwFilter;
  CFX_StackTemplate<FDE_CANVASITEM> m_CanvasStack;
};

#endif  // XFA_FDE_FDE_ITERATOR_H_
