// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_CBC_DATAMATRIX_H_
#define FXBARCODE_CBC_DATAMATRIX_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "fxbarcode/cbc_codebase.h"

class CBC_DataMatrix : public CBC_CodeBase {
 public:
  CBC_DataMatrix();
  ~CBC_DataMatrix() override;

  // CBC_OneCode:
  bool Encode(const CFX_WideStringC& contents,
              bool isDevice,
              int32_t& e) override;
  bool RenderDevice(CFX_RenderDevice* device,
                    const CFX_Matrix* matrix,
                    int32_t& e) override;
  bool RenderBitmap(CFX_RetainPtr<CFX_DIBitmap>& pOutBitmap,
                    int32_t& e) override;
  BC_TYPE GetType() override;
};

#endif  // FXBARCODE_CBC_DATAMATRIX_H_
