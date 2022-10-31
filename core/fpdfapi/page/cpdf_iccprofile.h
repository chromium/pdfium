// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_
#define CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_

#include <stdint.h>

#include <memory>

#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/base/span.h"

class CPDF_Stream;

namespace fxcodec {
class IccTransform;
}  // namespace fxcodec

class CPDF_IccProfile final : public Retainable, public Observable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  bool IsValid() const { return IsSRGB() || IsSupported(); }
  bool IsSRGB() const { return m_bsRGB; }
  bool IsSupported() const { return !!m_Transform; }
  uint32_t GetComponents() const { return m_nSrcComponents; }

  bool IsNormal() const;
  void Translate(pdfium::span<const float> pSrcValues,
                 pdfium::span<float> pDestValues);
  void TranslateScanline(pdfium::span<uint8_t> pDest,
                         pdfium::span<const uint8_t> pSrc,
                         int pixels);

 private:
  // Keeps stream alive for the duration of the CPDF_IccProfile.
  CPDF_IccProfile(RetainPtr<const CPDF_Stream> pStream,
                  pdfium::span<const uint8_t> span);
  ~CPDF_IccProfile() override;

  const bool m_bsRGB;
  uint32_t m_nSrcComponents = 0;
  RetainPtr<const CPDF_Stream> const m_pStream;  // Used by `m_Transform`.
  std::unique_ptr<fxcodec::IccTransform> m_Transform;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_ICCPROFILE_H_
