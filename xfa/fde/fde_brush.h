// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_FDE_BRUSH_H_
#define XFA_FDE_FDE_BRUSH_H_

#include "core/fxcrt/include/fx_system.h"
#include "core/fxge/include/fx_dib.h"

#define FDE_BRUSHTYPE_Unknown -1
#define FDE_BRUSHTYPE_Solid 0
#define FDE_BRUSHTYPE_MAX 0

class IFDE_Brush {
 public:
  static IFDE_Brush* Create(int32_t iType);
  virtual ~IFDE_Brush() {}
  virtual void Release() = 0;
  virtual int32_t GetType() const = 0;
};

class IFDE_SolidBrush : public IFDE_Brush {
 public:
  virtual FX_ARGB GetColor() const = 0;
  virtual void SetColor(FX_ARGB color) = 0;
  virtual const CFX_Matrix& GetMatrix() const = 0;
  virtual void ResetMatrix() = 0;
  virtual void TranslateMatrix(FX_FLOAT dx, FX_FLOAT dy) = 0;
  virtual void RotateMatrix(FX_FLOAT fRadian) = 0;
  virtual void ScaleMatrix(FX_FLOAT sx, FX_FLOAT sy) = 0;
  virtual void ConcatMatrix(const CFX_Matrix& matrix) = 0;
  virtual void SetMatrix(const CFX_Matrix& matrix) = 0;
};

#endif  // XFA_FDE_FDE_BRUSH_H_
