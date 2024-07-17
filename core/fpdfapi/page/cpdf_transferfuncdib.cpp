// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_transferfuncdib.h"

#include <utility>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxge/calculate_pitch.h"

#if BUILDFLAG(IS_APPLE)
#define INCR_ON_APPLE(x) ++x
#else
#define INCR_ON_APPLE(x)
#endif

CPDF_TransferFuncDIB::CPDF_TransferFuncDIB(
    RetainPtr<const CFX_DIBBase> src,
    RetainPtr<CPDF_TransferFunc> transfer_func)
    : src_(std::move(src)),
      transfer_func_(std::move(transfer_func)),
      r_samples_(transfer_func_->GetSamplesR()),
      g_samples_(transfer_func_->GetSamplesG()),
      b_samples_(transfer_func_->GetSamplesB()) {
  SetWidth(src_->GetWidth());
  SetHeight(src_->GetHeight());
  SetFormat(GetDestFormat());
  SetPitch(fxge::CalculatePitch32OrDie(GetBPP(), GetWidth()));
  scanline_.resize(GetPitch());
  CHECK(!HasPalette());
}

CPDF_TransferFuncDIB::~CPDF_TransferFuncDIB() = default;

FXDIB_Format CPDF_TransferFuncDIB::GetDestFormat() const {
  if (src_->IsMaskFormat()) {
    return FXDIB_Format::k8bppMask;
  }

  if (src_->IsAlphaFormat()) {
    return FXDIB_Format::kArgb;
  }

  return CFX_DIBBase::kPlatformRGBFormat;
}

void CPDF_TransferFuncDIB::TranslateScanline(
    pdfium::span<const uint8_t> src_span) const {
  const uint8_t* src_buf = src_span.data();
  bool skip = false;
  switch (src_->GetFormat()) {
    case FXDIB_Format::kInvalid: {
      break;
    }
    case FXDIB_Format::k1bppRgb: {
      int r0 = r_samples_[0];
      int g0 = g_samples_[0];
      int b0 = b_samples_[0];
      int r1 = r_samples_[255];
      int g1 = g_samples_[255];
      int b1 = b_samples_[255];
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          if (src_buf[i / 8] & (1 << (7 - i % 8))) {
            scanline_[index++] = b1;
            scanline_[index++] = g1;
            scanline_[index++] = r1;
          } else {
            scanline_[index++] = b0;
            scanline_[index++] = g0;
            scanline_[index++] = r0;
          }
          INCR_ON_APPLE(index);
        }
      });
      break;
    }
    case FXDIB_Format::k1bppMask: {
      int m0 = r_samples_[0];
      int m1 = r_samples_[255];
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          if (src_buf[i / 8] & (1 << (7 - i % 8))) {
            scanline_[index++] = m1;
          } else {
            scanline_[index++] = m0;
          }
        }
      });
      break;
    }
    case FXDIB_Format::k8bppRgb: {
      pdfium::span<const uint32_t> src_palette = src_->GetPaletteSpan();
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          if (src_->HasPalette()) {
            FX_ARGB src_argb = src_palette[*src_buf];
            scanline_[index++] = b_samples_[FXARGB_R(src_argb)];
            scanline_[index++] = g_samples_[FXARGB_G(src_argb)];
            scanline_[index++] = r_samples_[FXARGB_B(src_argb)];
          } else {
            uint32_t src_byte = *src_buf;
            scanline_[index++] = b_samples_[src_byte];
            scanline_[index++] = g_samples_[src_byte];
            scanline_[index++] = r_samples_[src_byte];
          }
          src_buf++;
          INCR_ON_APPLE(index);
        }
      });
      break;
    }
    case FXDIB_Format::k8bppMask: {
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          scanline_[index++] = r_samples_[*(src_buf++)];
        }
      });
      break;
    }
    case FXDIB_Format::kRgb: {
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          scanline_[index++] = b_samples_[*(src_buf++)];
          scanline_[index++] = g_samples_[*(src_buf++)];
          scanline_[index++] = r_samples_[*(src_buf++)];
          INCR_ON_APPLE(index);
        }
      });
      break;
    }
    case FXDIB_Format::kRgb32:
      skip = true;
      [[fallthrough]];
    case FXDIB_Format::kArgb: {
      int index = 0;
      UNSAFE_TODO({
        for (int i = 0; i < GetWidth(); i++) {
          scanline_[index++] = b_samples_[*(src_buf++)];
          scanline_[index++] = g_samples_[*(src_buf++)];
          scanline_[index++] = r_samples_[*(src_buf++)];
          if (skip) {
            INCR_ON_APPLE(index);
          } else {
            scanline_[index++] = *src_buf;
          }
          src_buf++;
        }
      });
      break;
    }
  }
}

pdfium::span<const uint8_t> CPDF_TransferFuncDIB::GetScanline(int line) const {
  TranslateScanline(src_->GetScanline(line));
  return scanline_;
}
