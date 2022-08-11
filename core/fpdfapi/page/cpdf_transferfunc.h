// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNC_H_
#define CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNC_H_

#include <stdint.h>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/span.h"

class CFX_DIBBase;

class CPDF_TransferFunc final : public Retainable, public Observable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  static constexpr size_t kChannelSampleSize = 256;

  FX_COLORREF TranslateColor(FX_COLORREF colorref) const;
  RetainPtr<CFX_DIBBase> TranslateImage(RetainPtr<CFX_DIBBase> pSrc);

  // Spans are |kChannelSampleSize| in size.
  pdfium::span<const uint8_t> GetSamplesR() const;
  pdfium::span<const uint8_t> GetSamplesG() const;
  pdfium::span<const uint8_t> GetSamplesB() const;

  bool GetIdentity() const { return m_bIdentity; }

 private:
  CPDF_TransferFunc(bool bIdentify,
                    DataVector<uint8_t> samples_r,
                    DataVector<uint8_t> samples_g,
                    DataVector<uint8_t> samples_b);
  ~CPDF_TransferFunc() override;

  const bool m_bIdentity;
  const DataVector<uint8_t> m_SamplesR;
  const DataVector<uint8_t> m_SamplesG;
  const DataVector<uint8_t> m_SamplesB;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TRANSFERFUNC_H_
