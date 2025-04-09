// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxbarcode/BC_TwoDimWriter.h"

#include <algorithm>

#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fxbarcode/BC_Writer.h"
#include "fxbarcode/common/BC_CommonBitMatrix.h"

CBC_TwoDimWriter::CBC_TwoDimWriter(bool bFixedSize) : fixed_size_(bFixedSize) {}

CBC_TwoDimWriter::~CBC_TwoDimWriter() = default;

bool CBC_TwoDimWriter::RenderResult(pdfium::span<const uint8_t> code,
                                    int32_t codeWidth,
                                    int32_t codeHeight) {
  if (code.empty()) {
    return false;
  }

  input_width_ = codeWidth;
  input_height_ = codeHeight;
  int32_t tempWidth = input_width_ + 2;
  int32_t tempHeight = input_height_ + 2;
  const float module_size =
      std::clamp<float>(std::min(module_width_, module_height_), 1.0f, 8.0f);
  FX_SAFE_INT32 scaledWidth = tempWidth;
  FX_SAFE_INT32 scaledHeight = tempHeight;
  scaledWidth *= module_size;
  scaledHeight *= module_size;
  output_width_ = scaledWidth.ValueOrDie();
  output_height_ = scaledHeight.ValueOrDie();

  if (fixed_size_) {
    if (width_ < output_width_ || height_ < output_height_) {
      return false;
    }
  } else {
    if (width_ > output_width_ || height_ > output_height_) {
      int32_t width_factor = static_cast<int32_t>(
          floor(static_cast<float>(width_) / output_width_));
      int32_t height_factor = static_cast<int32_t>(
          floor(static_cast<float>(height_) / output_height_));
      width_factor = std::max(width_factor, 1);
      height_factor = std::max(height_factor, 1);

      output_width_ *= width_factor;
      output_height_ *= height_factor;
    }
  }
  multi_x_ =
      static_cast<int32_t>(ceil(static_cast<float>(output_width_) / tempWidth));
  multi_y_ = static_cast<int32_t>(
      ceil(static_cast<float>(output_height_) / tempHeight));
  if (fixed_size_) {
    multi_x_ = std::min(multi_x_, multi_y_);
    multi_y_ = multi_x_;
  }

  left_padding_ = std::max((width_ - output_width_) / 2, 0);
  top_padding_ = std::max((height_ - output_height_) / 2, 0);

  output_ = std::make_unique<CBC_CommonBitMatrix>(input_width_, input_height_);
  for (int32_t y = 0; y < input_height_; ++y) {
    for (int32_t x = 0; x < input_width_; ++x) {
      if (code[x + y * input_width_] == 1) {
        output_->Set(x, y);
      }
    }
  }
  return true;
}

void CBC_TwoDimWriter::RenderDeviceResult(CFX_RenderDevice* device,
                                          const CFX_Matrix& matrix) {
  DCHECK(output_);

  CFX_GraphStateData stateData;
  CFX_Path path;
  path.AppendRect(0, 0, width_, height_);
  device->DrawPath(path, &matrix, &stateData, kBackgroundColor,
                   kBackgroundColor, CFX_FillRenderOptions::EvenOddOptions());
  int32_t leftPos = left_padding_;
  int32_t topPos = top_padding_;

  CFX_Matrix matri = matrix;
  if (width_ < output_width_ && height_ < output_height_) {
    CFX_Matrix matriScale(static_cast<float>(width_) / output_width_, 0.0, 0.0,
                          static_cast<float>(height_) / output_height_, 0.0,
                          0.0);
    matriScale.Concat(matrix);
    matri = matriScale;
  }

  CFX_GraphStateData data;
  for (int32_t x = 0; x < input_width_; x++) {
    for (int32_t y = 0; y < input_height_; y++) {
      if (output_->Get(x, y)) {
        // In the output, each module is shifted by 1 due to the one module
        // padding added to create quiet areas.
        int start_x_output = x + 1;
        int end_x_output = x + 2;
        int start_y_output = y + 1;
        int end_y_output = y + 2;

        CFX_Path rect;
        rect.AppendRect(leftPos + start_x_output * multi_x_,
                        topPos + start_y_output * multi_y_,
                        leftPos + end_x_output * multi_x_,
                        topPos + end_y_output * multi_y_);
        device->DrawPath(rect, &matri, &data, kBarColor, 0,
                         CFX_FillRenderOptions::WindingOptions());
      }
    }
  }
}
