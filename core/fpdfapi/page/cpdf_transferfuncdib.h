// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_
#define CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_

#include <vector>

#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "third_party/base/span.h"

class CPDF_TransferFunc;

class CPDF_TransferFuncDIB final : public CFX_DIBBase {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  void TranslateScanline(const uint8_t* src_buf,
                         std::vector<uint8_t>* dest_buf) const;
  void TranslateDownSamples(uint8_t* dest_buf,
                            const uint8_t* src_buf,
                            int pixels,
                            int Bpp) const;

 private:
  CPDF_TransferFuncDIB(const RetainPtr<CFX_DIBBase>& pSrc,
                       const RetainPtr<CPDF_TransferFunc>& pTransferFunc);
  ~CPDF_TransferFuncDIB() override;

  // CFX_DIBBase:
  const uint8_t* GetScanline(int line) const override;
  void DownSampleScanline(int line,
                          uint8_t* dest_scan,
                          int dest_bpp,
                          int dest_width,
                          bool bFlipX,
                          int clip_left,
                          int clip_width) const override;

  FXDIB_Format GetDestFormat() const;

  RetainPtr<CFX_DIBBase> const m_pSrc;
  mutable std::vector<uint8_t> m_Scanline;
  RetainPtr<CPDF_TransferFunc> const m_pTransferFunc;
  const pdfium::span<const uint8_t> m_RampR;
  const pdfium::span<const uint8_t> m_RampG;
  const pdfium::span<const uint8_t> m_RampB;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_
