// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_VISUALSET_H_
#define XFA_FDE_FDE_VISUALSET_H_

#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_system.h"
#include "core/fxge/include/fx_dib.h"
#include "core/fxge/include/fx_ge.h"
#include "xfa/fde/cfde_path.h"
#include "xfa/fde/fde_object.h"
#include "xfa/fgas/crt/fgas_memory.h"
#include "xfa/fgas/font/fgas_font.h"

enum FDE_VISUALOBJTYPE {
  FDE_VISUALOBJ_Canvas = 0x00,
  FDE_VISUALOBJ_Text = 0x01
};

typedef struct FDE_HVISUALOBJ_ { void* pData; } const* FDE_HVISUALOBJ;

class IFDE_VisualSet {
 public:
  virtual ~IFDE_VisualSet() {}
  virtual FDE_VISUALOBJTYPE GetType() = 0;
  virtual FX_BOOL GetBBox(FDE_HVISUALOBJ hVisualObj, CFX_RectF& bbox) = 0;
  virtual FX_BOOL GetMatrix(FDE_HVISUALOBJ hVisualObj, CFX_Matrix& matrix) = 0;
  virtual FX_BOOL GetRect(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) = 0;
  virtual FX_BOOL GetClip(FDE_HVISUALOBJ hVisualObj, CFX_RectF& rt) = 0;
};

class IFDE_CanvasSet : public IFDE_VisualSet {
 public:
  virtual FX_POSITION GetFirstPosition(FDE_HVISUALOBJ hCanvas) = 0;
  virtual FDE_HVISUALOBJ GetNext(FDE_HVISUALOBJ hCanvas,
                                 FX_POSITION& pos,
                                 IFDE_VisualSet*& pVisualSet) = 0;
  virtual FDE_HVISUALOBJ GetParentCanvas(FDE_HVISUALOBJ hCanvas,
                                         IFDE_VisualSet*& pVisualSet) = 0;
};

class IFDE_TextSet : public IFDE_VisualSet {
 public:
  virtual int32_t GetString(FDE_HVISUALOBJ hText, CFX_WideString& wsText) = 0;
  virtual IFX_Font* GetFont(FDE_HVISUALOBJ hText) = 0;
  virtual FX_FLOAT GetFontSize(FDE_HVISUALOBJ hText) = 0;
  virtual FX_ARGB GetFontColor(FDE_HVISUALOBJ hText) = 0;
  virtual int32_t GetDisplayPos(FDE_HVISUALOBJ hText,
                                FXTEXT_CHARPOS* pCharPos,
                                FX_BOOL bCharCode = FALSE,
                                CFX_WideString* pWSForms = NULL) = 0;
  virtual int32_t GetCharRects(FDE_HVISUALOBJ hText,
                               CFX_RectFArray& rtArray) = 0;
};

#endif  // XFA_FDE_FDE_VISUALSET_H_
