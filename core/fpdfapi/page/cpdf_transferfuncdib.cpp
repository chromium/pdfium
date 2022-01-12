// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_transferfuncdib.h"

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "third_party/base/check.h"
#include "third_party/base/compiler_specific.h"

CPDF_TransferFuncDIB::CPDF_TransferFuncDIB(
    const RetainPtr<CFX_DIBBase>& pSrc,
    const RetainPtr<CPDF_TransferFunc>& pTransferFunc)
    : m_pSrc(pSrc),
      m_pTransferFunc(pTransferFunc),
      m_RampR(pTransferFunc->GetSamplesR()),
      m_RampG(pTransferFunc->GetSamplesG()),
      m_RampB(pTransferFunc->GetSamplesB()) {
  m_Width = pSrc->GetWidth();
  m_Height = pSrc->GetHeight();
  m_Format = GetDestFormat();
  m_Pitch = (m_Width * GetBppFromFormat(m_Format) + 31) / 32 * 4;
  m_Scanline.resize(m_Pitch);
  DCHECK(m_palette.empty());
}

CPDF_TransferFuncDIB::~CPDF_TransferFuncDIB() = default;

FXDIB_Format CPDF_TransferFuncDIB::GetDestFormat() const {
  if (m_pSrc->IsMaskFormat())
    return FXDIB_Format::k8bppMask;

  if (m_pSrc->IsAlphaFormat())
    return FXDIB_Format::kArgb;

  return CFX_DIBBase::kPlatformRGBFormat;
}

void CPDF_TransferFuncDIB::TranslateScanline(
    pdfium::span<const uint8_t> src_span) const {
  const uint8_t* src_buf = src_span.data();
  bool bSkip = false;
  switch (m_pSrc->GetFormat()) {
    case FXDIB_Format::k1bppRgb: {
      int r0 = m_RampR[0];
      int g0 = m_RampG[0];
      int b0 = m_RampB[0];
      int r1 = m_RampR[255];
      int g1 = m_RampG[255];
      int b1 = m_RampB[255];
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (src_buf[i / 8] & (1 << (7 - i % 8))) {
          m_Scanline[index++] = b1;
          m_Scanline[index++] = g1;
          m_Scanline[index++] = r1;
        } else {
          m_Scanline[index++] = b0;
          m_Scanline[index++] = g0;
          m_Scanline[index++] = r0;
        }
#if BUILDFLAG(IS_APPLE)
        index++;
#endif
      }
      break;
    }
    case FXDIB_Format::k1bppMask: {
      int m0 = m_RampR[0];
      int m1 = m_RampR[255];
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (src_buf[i / 8] & (1 << (7 - i % 8)))
          m_Scanline[index++] = m1;
        else
          m_Scanline[index++] = m0;
      }
      break;
    }
    case FXDIB_Format::k8bppRgb: {
      pdfium::span<const uint32_t> src_palette = m_pSrc->GetPaletteSpan();
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (m_pSrc->HasPalette()) {
          FX_ARGB src_argb = src_palette[*src_buf];
          m_Scanline[index++] = m_RampB[FXARGB_R(src_argb)];
          m_Scanline[index++] = m_RampG[FXARGB_G(src_argb)];
          m_Scanline[index++] = m_RampR[FXARGB_B(src_argb)];
        } else {
          uint32_t src_byte = *src_buf;
          m_Scanline[index++] = m_RampB[src_byte];
          m_Scanline[index++] = m_RampG[src_byte];
          m_Scanline[index++] = m_RampR[src_byte];
        }
        src_buf++;
#if BUILDFLAG(IS_APPLE)
        index++;
#endif
      }
      break;
    }
    case FXDIB_Format::k8bppMask: {
      int index = 0;
      for (int i = 0; i < m_Width; i++)
        m_Scanline[index++] = m_RampR[*(src_buf++)];
      break;
    }
    case FXDIB_Format::kRgb: {
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        m_Scanline[index++] = m_RampB[*(src_buf++)];
        m_Scanline[index++] = m_RampG[*(src_buf++)];
        m_Scanline[index++] = m_RampR[*(src_buf++)];
#if BUILDFLAG(IS_APPLE)
        index++;
#endif
      }
      break;
    }
    case FXDIB_Format::kRgb32:
      bSkip = true;
      FALLTHROUGH;
    case FXDIB_Format::kArgb: {
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        m_Scanline[index++] = m_RampB[*(src_buf++)];
        m_Scanline[index++] = m_RampG[*(src_buf++)];
        m_Scanline[index++] = m_RampR[*(src_buf++)];
        if (!bSkip) {
          m_Scanline[index++] = *src_buf;
#if BUILDFLAG(IS_APPLE)
        } else {
          index++;
#endif
        }
        src_buf++;
      }
      break;
    }
    default:
      break;
  }
}

pdfium::span<const uint8_t> CPDF_TransferFuncDIB::GetScanline(int line) const {
  TranslateScanline(m_pSrc->GetScanline(line));
  return m_Scanline;
}
