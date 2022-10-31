// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_
#define CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "third_party/base/span.h"

class CPDF_TransferFunc;

class CPDF_TransferFuncDIB final : public CFX_DIBBase {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // CFX_DIBBase:
  pdfium::span<const uint8_t> GetScanline(int line) const override;

 private:
  CPDF_TransferFuncDIB(RetainPtr<CFX_DIBBase> pSrc,
                       RetainPtr<CPDF_TransferFunc> pTransferFunc);
  ~CPDF_TransferFuncDIB() override;

  void TranslateScanline(pdfium::span<const uint8_t> src_span) const;
  FXDIB_Format GetDestFormat() const;

  RetainPtr<CFX_DIBBase> const m_pSrc;
  RetainPtr<CPDF_TransferFunc> const m_pTransferFunc;
  const pdfium::span<const uint8_t> m_RampR;
  const pdfium::span<const uint8_t> m_RampG;
  const pdfium::span<const uint8_t> m_RampB;
  mutable DataVector<uint8_t> m_Scanline;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNCDIB_H_
