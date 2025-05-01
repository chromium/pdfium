// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_iccprofile.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcodec/icc/icc_transform.h"
#include "core/fxcrt/span.h"

namespace {

bool DetectSRGB(pdfium::span<const uint8_t> span) {
  static constexpr auto kSRGB = pdfium::span_from_cstring("sRGB IEC61966-2.1");
  return span.size() == 3144 && span.subspan<400u, kSRGB.size()>() == kSRGB;
}

}  // namespace

CPDF_IccProfile::CPDF_IccProfile(RetainPtr<const CPDF_StreamAcc> stream_acc,
                                 uint32_t expected_components)
    : stream_acc_(std::move(stream_acc)),
      is_srgb_(expected_components == 3 && DetectSRGB(stream_acc_->GetSpan())) {
  if (is_srgb_) {
    src_components_ = 3;
    return;
  }

  auto transform =
      fxcodec::IccTransform::CreateTransformSRGB(stream_acc_->GetSpan());
  if (!transform) {
    return;
  }

  uint32_t components = transform->components();
  if (components != expected_components) {
    return;
  }

  src_components_ = components;
  transform_ = std::move(transform);
}

CPDF_IccProfile::~CPDF_IccProfile() = default;

bool CPDF_IccProfile::IsNormal() const {
  return transform_->IsNormal();
}

void CPDF_IccProfile::Translate(pdfium::span<const float> pSrcValues,
                                pdfium::span<float> pDestValues) {
  transform_->Translate(pSrcValues, pDestValues);
}

void CPDF_IccProfile::TranslateScanline(pdfium::span<uint8_t> pDest,
                                        pdfium::span<const uint8_t> pSrc,
                                        int pixels) {
  transform_->TranslateScanline(pDest, pSrc, pixels);
}

RetainPtr<const CPDF_StreamAcc> CPDF_IccProfile::GetStreamAcc() const {
  return stream_acc_;
}
