// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "fde_object.h"
IFDE_Pen* IFDE_Pen::Create() {
  return new CFDE_Pen();
}
IFDE_Brush* IFDE_Brush::Create(int32_t iType) {
  switch (iType) {
    case FDE_BRUSHTYPE_Solid:
      return new CFDE_SolidBrush;
    case FDE_BRUSHTYPE_Hatch:
      return new CFDE_HatchBrush;
    case FDE_BRUSHTYPE_Texture:
      return new CFDE_TextureBrush;
    case FDE_BRUSHTYPE_LinearGradient:
      return new CFDE_LinearBrush;
    default:
      return NULL;
  }
}
