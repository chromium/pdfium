// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_transferfuncdib.h"

#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
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
  FXDIB_Format format = GetDestFormat();
  m_bpp = GetBppFromFormat(format);
  m_AlphaFlag = GetAlphaFlagFromFormat(format);
  m_Pitch = (m_Width * m_bpp + 31) / 32 * 4;
  m_pPalette.reset();
  m_Scanline.resize(m_Pitch);
}

CPDF_TransferFuncDIB::~CPDF_TransferFuncDIB() = default;

FXDIB_Format CPDF_TransferFuncDIB::GetDestFormat() const {
  if (m_pSrc->IsAlphaMask())
    return FXDIB_8bppMask;

#if defined(OS_MACOSX)
  return m_pSrc->HasAlpha() ? FXDIB_Argb : FXDIB_Rgb32;
#else
  return m_pSrc->HasAlpha() ? FXDIB_Argb : FXDIB_Rgb;
#endif
}

void CPDF_TransferFuncDIB::TranslateScanline(
    const uint8_t* src_buf,
    std::vector<uint8_t>* dest_buf) const {
  bool bSkip = false;
  switch (m_pSrc->GetFormat()) {
    case FXDIB_1bppRgb: {
      int r0 = m_RampR[0];
      int g0 = m_RampG[0];
      int b0 = m_RampB[0];
      int r1 = m_RampR[255];
      int g1 = m_RampG[255];
      int b1 = m_RampB[255];
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (src_buf[i / 8] & (1 << (7 - i % 8))) {
          (*dest_buf)[index++] = b1;
          (*dest_buf)[index++] = g1;
          (*dest_buf)[index++] = r1;
        } else {
          (*dest_buf)[index++] = b0;
          (*dest_buf)[index++] = g0;
          (*dest_buf)[index++] = r0;
        }
#if defined(OS_MACOSX)
        index++;
#endif
      }
      break;
    }
    case FXDIB_1bppMask: {
      int m0 = m_RampR[0];
      int m1 = m_RampR[255];
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (src_buf[i / 8] & (1 << (7 - i % 8)))
          (*dest_buf)[index++] = m1;
        else
          (*dest_buf)[index++] = m0;
      }
      break;
    }
    case FXDIB_8bppRgb: {
      FX_ARGB* pPal = m_pSrc->GetPalette();
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        if (pPal) {
          FX_ARGB src_argb = pPal[*src_buf];
          (*dest_buf)[index++] = m_RampB[FXARGB_R(src_argb)];
          (*dest_buf)[index++] = m_RampG[FXARGB_G(src_argb)];
          (*dest_buf)[index++] = m_RampR[FXARGB_B(src_argb)];
        } else {
          uint32_t src_byte = *src_buf;
          (*dest_buf)[index++] = m_RampB[src_byte];
          (*dest_buf)[index++] = m_RampG[src_byte];
          (*dest_buf)[index++] = m_RampR[src_byte];
        }
        src_buf++;
#if defined(OS_MACOSX)
        index++;
#endif
      }
      break;
    }
    case FXDIB_8bppMask: {
      int index = 0;
      for (int i = 0; i < m_Width; i++)
        (*dest_buf)[index++] = m_RampR[*(src_buf++)];
      break;
    }
    case FXDIB_Rgb: {
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        (*dest_buf)[index++] = m_RampB[*(src_buf++)];
        (*dest_buf)[index++] = m_RampG[*(src_buf++)];
        (*dest_buf)[index++] = m_RampR[*(src_buf++)];
#if defined(OS_MACOSX)
        index++;
#endif
      }
      break;
    }
    case FXDIB_Rgb32:
      bSkip = true;
      FALLTHROUGH;
    case FXDIB_Argb: {
      int index = 0;
      for (int i = 0; i < m_Width; i++) {
        (*dest_buf)[index++] = m_RampB[*(src_buf++)];
        (*dest_buf)[index++] = m_RampG[*(src_buf++)];
        (*dest_buf)[index++] = m_RampR[*(src_buf++)];
        if (!bSkip) {
          (*dest_buf)[index++] = *src_buf;
#if defined(OS_MACOSX)
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

void CPDF_TransferFuncDIB::TranslateDownSamples(uint8_t* dest_buf,
                                                const uint8_t* src_buf,
                                                int pixels,
                                                int Bpp) const {
  if (Bpp == 8) {
    for (int i = 0; i < pixels; i++)
      *dest_buf++ = m_RampR[*(src_buf++)];
  } else if (Bpp == 24) {
    for (int i = 0; i < pixels; i++) {
      *dest_buf++ = m_RampB[*(src_buf++)];
      *dest_buf++ = m_RampG[*(src_buf++)];
      *dest_buf++ = m_RampR[*(src_buf++)];
    }
  } else {
#if defined(OS_MACOSX)
    if (!m_pSrc->HasAlpha()) {
      for (int i = 0; i < pixels; i++) {
        *dest_buf++ = m_RampB[*(src_buf++)];
        *dest_buf++ = m_RampG[*(src_buf++)];
        *dest_buf++ = m_RampR[*(src_buf++)];
        dest_buf++;
        src_buf++;
      }
    } else {
#endif
      for (int i = 0; i < pixels; i++) {
        *dest_buf++ = m_RampB[*(src_buf++)];
        *dest_buf++ = m_RampG[*(src_buf++)];
        *dest_buf++ = m_RampR[*(src_buf++)];
        *dest_buf++ = *(src_buf++);
      }
#if defined(OS_MACOSX)
    }
#endif
  }
}

const uint8_t* CPDF_TransferFuncDIB::GetScanline(int line) const {
  TranslateScanline(m_pSrc->GetScanline(line), &m_Scanline);
  return m_Scanline.data();
}

void CPDF_TransferFuncDIB::DownSampleScanline(int line,
                                              uint8_t* dest_scan,
                                              int dest_bpp,
                                              int dest_width,
                                              bool bFlipX,
                                              int clip_left,
                                              int clip_width) const {
  m_pSrc->DownSampleScanline(line, dest_scan, dest_bpp, dest_width, bFlipX,
                             clip_left, clip_width);
  TranslateDownSamples(dest_scan, dest_scan, clip_width, dest_bpp);
}
