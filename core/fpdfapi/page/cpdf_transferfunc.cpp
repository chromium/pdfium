// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_transferfunc.h"

#include <stdint.h>

#include <utility>

#include "core/fpdfapi/page/cpdf_transferfuncdib.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxge/dib/cfx_dibbase.h"

CPDF_TransferFunc::CPDF_TransferFunc(bool bIdentify,
                                     FixedSizeDataVector<uint8_t> samples_r,
                                     FixedSizeDataVector<uint8_t> samples_g,
                                     FixedSizeDataVector<uint8_t> samples_b)
    : identity_(bIdentify),
      samples_r_(std::move(samples_r)),
      samples_g_(std::move(samples_g)),
      samples_b_(std::move(samples_b)) {
  DCHECK_EQ(samples_r_.size(), kChannelSampleSize);
  DCHECK_EQ(samples_g_.size(), kChannelSampleSize);
  DCHECK_EQ(samples_b_.size(), kChannelSampleSize);
}

CPDF_TransferFunc::~CPDF_TransferFunc() = default;

FX_COLORREF CPDF_TransferFunc::TranslateColor(FX_COLORREF colorref) const {
  return FXSYS_BGR(samples_b_.span()[FXSYS_GetBValue(colorref)],
                   samples_g_.span()[FXSYS_GetGValue(colorref)],
                   samples_r_.span()[FXSYS_GetRValue(colorref)]);
}

RetainPtr<CFX_DIBBase> CPDF_TransferFunc::TranslateImage(
    RetainPtr<CFX_DIBBase> pSrc) {
  return pdfium::MakeRetain<CPDF_TransferFuncDIB>(std::move(pSrc),
                                                  pdfium::WrapRetain(this));
}

pdfium::span<const uint8_t> CPDF_TransferFunc::GetSamplesR() const {
  return samples_r_;
}

pdfium::span<const uint8_t> CPDF_TransferFunc::GetSamplesG() const {
  return samples_g_;
}

pdfium::span<const uint8_t> CPDF_TransferFunc::GetSamplesB() const {
  return samples_b_;
}
