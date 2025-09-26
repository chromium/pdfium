// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/png/png_decoder_delegate.h"

#include "core/fxcrt/notreached.h"

namespace fxcodec {

// static
int PngDecoderDelegate::GetNumberOfComponents(EncodedColorType color_type) {
  switch (color_type) {
    case EncodedColorType::kGrayscale:
      return 1;
    case EncodedColorType::kGrayscaleWithAlpha:
      return 2;
    case EncodedColorType::kTruecolor:
      return 3;
    case EncodedColorType::kIndexedColor:
    case EncodedColorType::kTruecolorWithAlpha:
      return 4;
  }
  NOTREACHED();
}

}  // namespace fxcodec
