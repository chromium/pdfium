// Copyright 2025 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PAGE_JPX_DECODE_CONVERSION_H_
#define CORE_FPDFAPI_PAGE_JPX_DECODE_CONVERSION_H_

#include <optional>

#include "core/fxcodec/jpx/cjpx_decoder.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CPDF_ColorSpace;

class JpxDecodeConversion {
 public:
  static std::optional<JpxDecodeConversion> Create(
      const CJPX_Decoder::JpxImageInfo& jpx_info,
      const CPDF_ColorSpace* pdf_colorspace);

  JpxDecodeConversion(const JpxDecodeConversion&) = delete;
  JpxDecodeConversion& operator=(const JpxDecodeConversion&) = delete;
  JpxDecodeConversion(JpxDecodeConversion&&) noexcept;
  JpxDecodeConversion& operator=(JpxDecodeConversion&&) noexcept;
  ~JpxDecodeConversion();

  const std::optional<RetainPtr<CPDF_ColorSpace>>& override_colorspace() const {
    return override_colorspace_;
  }

  const std::optional<int>& jpx_components_count() const {
    return jpx_components_count_;
  }

  bool swap_rgb() const { return swap_rgb_; }
  bool convert_argb_to_rgb() const { return convert_argb_to_rgb_; }
  FXDIB_Format format() const { return format_; }
  uint32_t width() const { return width_; }

 private:
  JpxDecodeConversion();

  bool swap_rgb_ = false;
  bool convert_argb_to_rgb_ = false;
  FXDIB_Format format_ = FXDIB_Format::kInvalid;
  uint32_t width_ = 0;

  // The colorspace to override the existing colorspace.
  //
  // std::nullopt means no override colorspace.
  // nullptr means reset the colorspace.
  std::optional<RetainPtr<CPDF_ColorSpace>> override_colorspace_;

  // The components count from the JPEG2000 image.
  //
  // std::nullopt means no new components count.
  // Value <= 0 means failure.
  std::optional<int> jpx_components_count_;
};

#endif  // CORE_FPDFAPI_PAGE_JPX_DECODE_CONVERSION_H_
