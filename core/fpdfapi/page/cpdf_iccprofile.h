// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_
#define CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"

class CPDF_StreamAcc;

namespace fxcodec {
class IccTransform;
}  // namespace fxcodec

class CPDF_IccProfile final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  bool IsValid() const { return IsSRGB() || IsSupported(); }
  bool IsSRGB() const { return is_srgb_; }
  bool IsSupported() const { return !!transform_; }
  uint32_t GetComponents() const { return src_components_; }

  bool IsNormal() const;
  void Translate(pdfium::span<const float> pSrcValues,
                 pdfium::span<float> pDestValues);
  void TranslateScanline(pdfium::span<uint8_t> pDest,
                         pdfium::span<const uint8_t> pSrc,
                         int pixels);

  RetainPtr<const CPDF_StreamAcc> GetStreamAcc() const;

 private:
  CPDF_IccProfile(RetainPtr<const CPDF_StreamAcc> stream_acc,
                  uint32_t expected_components);
  ~CPDF_IccProfile() override;

  // Keeps stream alive for the lifetime of this object, so `transform_` can
  // safely access the stream data.
  RetainPtr<const CPDF_StreamAcc> const stream_acc_;
  // Uses data from `stream_acc_`.
  std::unique_ptr<fxcodec::IccTransform> transform_;
  const bool is_srgb_;
  uint32_t src_components_ = 0;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_
