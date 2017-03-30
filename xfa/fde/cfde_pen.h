// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CFDE_PEN_H_
#define XFA_FDE_CFDE_PEN_H_

#include "core/fxge/fx_dib.h"

class CFDE_Pen {
 public:
  CFDE_Pen() : m_Color(0) {}
  ~CFDE_Pen() {}

  FX_ARGB GetColor() const { return m_Color; }
  void SetColor(FX_ARGB color) { m_Color = color; }

 private:
  FX_ARGB m_Color;
};

#endif  // XFA_FDE_CFDE_PEN_H_
