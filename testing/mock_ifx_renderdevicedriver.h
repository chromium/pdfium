// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_MOCK_IFX_RENDERDEVICEDRIVER_H_
#define TESTING_MOCK_IFX_RENDERDEVICEDRIVER_H_

#include <memory>

#include "core/fxge/ifx_renderdevicedriver.h"
#include "testing/gmock/include/gmock/gmock.h"

class MockIFXRenderDeviceDriver : public IFX_RenderDeviceDriver {
 public:
  MOCK_CONST_METHOD1(GetDeviceCaps, int(int caps_id));
  MOCK_METHOD0(SaveState, void());
  MOCK_METHOD1(RestoreState, void(bool bKeepSaved));
  MOCK_METHOD3(SetClip_PathFill,
               bool(const CFX_PathData* pPathData,
                    const CFX_Matrix* pObject2Device,
                    int fill_mode));
  MOCK_METHOD7(DrawPath,
               bool(const CFX_PathData* pPathData,
                    const CFX_Matrix* pObject2Device,
                    const CFX_GraphStateData* pGraphState,
                    uint32_t fill_color,
                    uint32_t stroke_color,
                    int fill_mode,
                    int blend_type));
  MOCK_METHOD1(GetClipBox, bool(FX_RECT* pRect));
  MOCK_METHOD6(SetDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    uint32_t color,
                    const FX_RECT* pSrcRect,
                    int dest_left,
                    int dest_top,
                    int blend_type));

  MOCK_METHOD7(StartDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    int bitmap_alpha,
                    uint32_t color,
                    const CFX_Matrix* pMatrix,
                    uint32_t flags,
                    std::unique_ptr<CFX_ImageRenderer>* handle,
                    int blend_type));
  MOCK_METHOD9(StretchDIBits,
               bool(const CFX_RetainPtr<CFX_DIBSource>& pBitmap,
                    uint32_t color,
                    int dest_left,
                    int dest_top,
                    int dest_width,
                    int dest_height,
                    const FX_RECT* pClipRect,
                    uint32_t flags,
                    int blend_type));
};

#endif  // TESTING_MOCK_IFX_RENDERDEVICEDRIVER_H_
