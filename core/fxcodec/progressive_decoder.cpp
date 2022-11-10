// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/progressive_decoder.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/jpeg/jpeg_progressive_decoder.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/dib/cfx_cmyk_to_srgb.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"
#include "third_party/base/numerics/safe_conversions.h"

#ifdef PDF_ENABLE_XFA_BMP
#include "core/fxcodec/bmp/bmp_progressive_decoder.h"
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
#include "core/fxcodec/gif/gif_progressive_decoder.h"
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_TIFF
#include "core/fxcodec/tiff/tiff_decoder.h"
#endif  // PDF_ENABLE_XFA_TIFF

namespace fxcodec {

namespace {

constexpr size_t kBlockSize = 4096;

#ifdef PDF_ENABLE_XFA_PNG
#if BUILDFLAG(IS_APPLE)
const double kPngGamma = 1.7;
#else
const double kPngGamma = 2.2;
#endif  // BUILDFLAG(IS_APPLE)
#endif  // PDF_ENABLE_XFA_PNG

void RGB2BGR(uint8_t* buffer, int width = 1) {
  if (buffer && width > 0) {
    uint8_t temp;
    int i = 0;
    int j = 0;
    for (; i < width; i++, j += 3) {
      temp = buffer[j];
      buffer[j] = buffer[j + 2];
      buffer[j + 2] = temp;
    }
  }
}

}  // namespace

ProgressiveDecoder::HorzTable::HorzTable() = default;

ProgressiveDecoder::HorzTable::~HorzTable() = default;

void ProgressiveDecoder::HorzTable::CalculateWeights(int dest_len,
                                                     int src_len) {
  CHECK_GE(dest_len, 0);
  m_ItemSize =
      pdfium::base::checked_cast<int>(PixelWeight::TotalBytesForWeightCount(2));
  FX_SAFE_SIZE_T safe_size = m_ItemSize;
  safe_size *= dest_len;
  m_pWeightTables.resize(safe_size.ValueOrDie(), 0);
  double scale = (double)dest_len / (double)src_len;
  if (scale > 1) {
    int pre_dest_col = 0;
    for (int src_col = 0; src_col < src_len; src_col++) {
      double dest_col_f = src_col * scale;
      int dest_col = FXSYS_roundf((float)dest_col_f);
      PixelWeight* pWeight = GetPixelWeight(dest_col);
      pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
      pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
      pWeight->m_Weights[1] = 0;
      if (src_col == src_len - 1 && dest_col < dest_len - 1) {
        for (int dest_col_index = pre_dest_col + 1; dest_col_index < dest_len;
             dest_col_index++) {
          pWeight = GetPixelWeight(dest_col_index);
          pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
          pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
          pWeight->m_Weights[1] = 0;
        }
        return;
      }
      int dest_col_len = dest_col - pre_dest_col;
      for (int dest_col_index = pre_dest_col + 1; dest_col_index < dest_col;
           dest_col_index++) {
        pWeight = GetPixelWeight(dest_col_index);
        pWeight->m_SrcStart = src_col - 1;
        pWeight->m_SrcEnd = src_col;
        pWeight->m_Weights[0] = CStretchEngine::FixedFromFloat(
            ((float)dest_col - (float)dest_col_index) / (float)dest_col_len);
        pWeight->m_Weights[1] =
            CStretchEngine::kFixedPointOne - pWeight->m_Weights[0];
      }
      pre_dest_col = dest_col;
    }
    return;
  }
  for (int dest_col = 0; dest_col < dest_len; dest_col++) {
    double src_col_f = dest_col / scale;
    int src_col = FXSYS_roundf((float)src_col_f);
    PixelWeight* pWeight = GetPixelWeight(dest_col);
    pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
    pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
    pWeight->m_Weights[1] = 0;
  }
}

ProgressiveDecoder::VertTable::VertTable() = default;

ProgressiveDecoder::VertTable::~VertTable() = default;

void ProgressiveDecoder::VertTable::CalculateWeights(int dest_len,
                                                     int src_len) {
  CHECK_GE(dest_len, 0);
  m_ItemSize =
      pdfium::base::checked_cast<int>(PixelWeight::TotalBytesForWeightCount(2));
  FX_SAFE_SIZE_T safe_size = m_ItemSize;
  safe_size *= dest_len;
  m_pWeightTables.resize(safe_size.ValueOrDie(), 0);
  double scale = (double)dest_len / (double)src_len;
  if (scale <= 1) {
    for (int dest_row = 0; dest_row < dest_len; dest_row++) {
      PixelWeight* pWeight = GetPixelWeight(dest_row);
      pWeight->m_SrcStart = dest_row;
      pWeight->m_SrcEnd = dest_row;
      pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
      pWeight->m_Weights[1] = 0;
    }
    return;
  }

  double step = 0.0;
  int src_row = 0;
  while (step < (double)dest_len) {
    int start_step = (int)step;
    step = scale * (++src_row);
    int end_step = (int)step;
    if (end_step >= dest_len) {
      end_step = dest_len;
      for (int dest_row = start_step; dest_row < end_step; dest_row++) {
        PixelWeight* pWeight = GetPixelWeight(dest_row);
        pWeight->m_SrcStart = start_step;
        pWeight->m_SrcEnd = start_step;
        pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
        pWeight->m_Weights[1] = 0;
      }
      return;
    }
    int length = end_step - start_step;
    {
      PixelWeight* pWeight = GetPixelWeight(start_step);
      pWeight->m_SrcStart = start_step;
      pWeight->m_SrcEnd = start_step;
      pWeight->m_Weights[0] = CStretchEngine::kFixedPointOne;
      pWeight->m_Weights[1] = 0;
    }
    for (int dest_row = start_step + 1; dest_row < end_step; dest_row++) {
      PixelWeight* pWeight = GetPixelWeight(dest_row);
      pWeight->m_SrcStart = start_step;
      pWeight->m_SrcEnd = end_step;
      pWeight->m_Weights[0] = CStretchEngine::FixedFromFloat(
          (float)(end_step - dest_row) / (float)length);
      pWeight->m_Weights[1] =
          CStretchEngine::kFixedPointOne - pWeight->m_Weights[0];
    }
  }
}

ProgressiveDecoder::ProgressiveDecoder() = default;

ProgressiveDecoder::~ProgressiveDecoder() = default;

#ifdef PDF_ENABLE_XFA_PNG
bool ProgressiveDecoder::PngReadHeader(int width,
                                       int height,
                                       int bpc,
                                       int pass,
                                       int* color_type,
                                       double* gamma) {
  if (!m_pDeviceBitmap) {
    m_SrcWidth = width;
    m_SrcHeight = height;
    m_SrcBPC = bpc;
    m_SrcPassNumber = pass;
    switch (*color_type) {
      case 0:
        m_SrcComponents = 1;
        break;
      case 4:
        m_SrcComponents = 2;
        break;
      case 2:
        m_SrcComponents = 3;
        break;
      case 3:
      case 6:
        m_SrcComponents = 4;
        break;
      default:
        m_SrcComponents = 0;
        break;
    }
    m_clipBox = FX_RECT(0, 0, width, height);
    return false;
  }
  FXDIB_Format format = m_pDeviceBitmap->GetFormat();
  switch (format) {
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb:
      NOTREACHED();
      return false;
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      *color_type = 0;
      break;
    case FXDIB_Format::kRgb:
      *color_type = 2;
      break;
    case FXDIB_Format::kRgb32:
    case FXDIB_Format::kArgb:
      *color_type = 6;
      break;
    default:
      NOTREACHED();
      return false;
  }
  *gamma = kPngGamma;
  return true;
}

bool ProgressiveDecoder::PngAskScanlineBuf(int line, uint8_t** pSrcBuf) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = m_pDeviceBitmap;
  if (!pDIBitmap) {
    NOTREACHED();
    return false;
  }
  if (line < m_clipBox.top || line >= m_clipBox.bottom)
    return true;

  double scale_y = static_cast<double>(m_sizeY) / m_clipBox.Height();
  int32_t row =
      static_cast<int32_t>((line - m_clipBox.top) * scale_y) + m_startY;
  *pSrcBuf = m_DecodeBuf.data();
  int32_t src_Bpp = pDIBitmap->GetBPP() >> 3;
  int32_t dest_Bpp = (m_SrcFormat & 0xff) >> 3;
  int32_t src_left = m_startX;
  int32_t dest_left = m_clipBox.left;
  pdfium::span<const uint8_t> src_span =
      pDIBitmap->GetScanline(row).subspan(src_left * src_Bpp);
  pdfium::span<uint8_t> dest_span =
      pdfium::make_span(m_DecodeBuf).subspan(dest_left * dest_Bpp);
  const uint8_t* src_scan = src_span.data();
  uint8_t* dest_scan = dest_span.data();
  switch (pDIBitmap->GetFormat()) {
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb:
      for (int32_t src_col = 0; src_col < m_sizeX; src_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(src_col);
        if (pPixelWeights->m_SrcStart != pPixelWeights->m_SrcEnd)
          continue;
        NOTREACHED();
        return false;
      }
      return true;
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      if (pDIBitmap->HasPalette())
        return false;
      for (int32_t src_col = 0; src_col < m_sizeX; src_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(src_col);
        if (pPixelWeights->m_SrcStart != pPixelWeights->m_SrcEnd)
          continue;
        uint32_t dest_g = pPixelWeights->m_Weights[0] * src_scan[src_col];
        dest_scan[pPixelWeights->m_SrcStart] =
            CStretchEngine::PixelFromFixed(dest_g);
      }
      return true;
    case FXDIB_Format::kRgb:
    case FXDIB_Format::kRgb32:
      for (int32_t src_col = 0; src_col < m_sizeX; src_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(src_col);
        if (pPixelWeights->m_SrcStart != pPixelWeights->m_SrcEnd)
          continue;
        const uint8_t* p = src_scan + src_col * src_Bpp;
        uint32_t dest_b = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_g = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_r = pPixelWeights->m_Weights[0] * (*p);
        uint8_t* pDes = &dest_scan[pPixelWeights->m_SrcStart * dest_Bpp];
        *pDes++ = CStretchEngine::PixelFromFixed(dest_b);
        *pDes++ = CStretchEngine::PixelFromFixed(dest_g);
        *pDes = CStretchEngine::PixelFromFixed(dest_r);
      }
      return true;
    case FXDIB_Format::kArgb:
      for (int32_t src_col = 0; src_col < m_sizeX; src_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(src_col);
        if (pPixelWeights->m_SrcStart != pPixelWeights->m_SrcEnd)
          continue;
        const uint8_t* p = src_scan + src_col * src_Bpp;
        uint32_t dest_b = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_g = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_r = pPixelWeights->m_Weights[0] * (*p++);
        uint8_t dest_a = *p;
        uint8_t* pDes = &dest_scan[pPixelWeights->m_SrcStart * dest_Bpp];
        *pDes++ = CStretchEngine::PixelFromFixed(dest_b);
        *pDes++ = CStretchEngine::PixelFromFixed(dest_g);
        *pDes++ = CStretchEngine::PixelFromFixed(dest_r);
        *pDes = dest_a;
      }
      return true;
    default:
      return false;
  }
}

void ProgressiveDecoder::PngFillScanlineBufCompleted(int pass, int line) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = m_pDeviceBitmap;
  DCHECK(pDIBitmap);
  int src_top = m_clipBox.top;
  int src_bottom = m_clipBox.bottom;
  int dest_top = m_startY;
  int src_height = m_clipBox.Height();
  int dest_height = m_sizeY;
  if (line >= src_top && line < src_bottom) {
    double scale_y = static_cast<double>(dest_height) / src_height;
    int src_row = line - src_top;
    int dest_row = (int)(src_row * scale_y) + dest_top;
    if (dest_row >= dest_top + dest_height) {
      return;
    }
    PngOneOneMapResampleHorz(pDIBitmap, dest_row, m_DecodeBuf, m_SrcFormat);
    if (m_SrcPassNumber == 1 && scale_y > 1.0) {
      ResampleVert(pDIBitmap, scale_y, dest_row);
      return;
    }
    if (pass == 6 && scale_y > 1.0) {
      ResampleVert(pDIBitmap, scale_y, dest_row);
    }
  }
}
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_GIF
uint32_t ProgressiveDecoder::GifCurrentPosition() const {
  uint32_t remain_size = pdfium::base::checked_cast<uint32_t>(
      GifDecoder::GetAvailInput(m_pGifContext.get()));
  return m_offSet - remain_size;
}

bool ProgressiveDecoder::GifInputRecordPositionBuf(uint32_t rcd_pos,
                                                   const FX_RECT& img_rc,
                                                   int32_t pal_num,
                                                   CFX_GifPalette* pal_ptr,
                                                   int32_t trans_index,
                                                   bool interlace) {
  m_offSet = rcd_pos;

  FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
  m_pCodecMemory->Seek(m_pCodecMemory->GetSize());
  if (!GifReadMoreData(&error_status))
    return false;

  CFX_GifPalette* pPalette = nullptr;
  if (pal_num != 0 && pal_ptr) {
    pPalette = pal_ptr;
  } else {
    if (!m_pGifPalette)
      return false;
    pal_num = m_GifPltNumber;
    pPalette = m_pGifPalette;
  }
  m_SrcPalette.resize(pal_num);
  m_SrcPaletteNumber = pal_num;
  for (int i = 0; i < pal_num; i++) {
    m_SrcPalette[i] =
        ArgbEncode(0xff, pPalette[i].r, pPalette[i].g, pPalette[i].b);
  }
  m_GifTransIndex = trans_index;
  m_GifFrameRect = img_rc;
  m_SrcPassNumber = interlace ? 4 : 1;
  int32_t pal_index = m_GifBgIndex;
  RetainPtr<CFX_DIBitmap> pDevice = m_pDeviceBitmap;
  if (trans_index >= pal_num)
    trans_index = -1;
  if (trans_index != -1) {
    m_SrcPalette[trans_index] &= 0x00ffffff;
    if (pDevice->IsAlphaFormat())
      pal_index = trans_index;
  }
  if (pal_index >= pal_num)
    return false;

  int startX = m_startX;
  int startY = m_startY;
  int sizeX = m_sizeX;
  int sizeY = m_sizeY;
  int Bpp = pDevice->GetBPP() / 8;
  FX_ARGB argb = m_SrcPalette[pal_index];
  for (int row = 0; row < sizeY; row++) {
    uint8_t* pScanline =
        pDevice->GetWritableScanline(row + startY).subspan(startX * Bpp).data();
    switch (m_TransMethod) {
      case 3: {
        uint8_t gray =
            FXRGB2GRAY(FXARGB_R(argb), FXARGB_G(argb), FXARGB_B(argb));
        memset(pScanline, gray, sizeX);
        break;
      }
      case 8: {
        for (int col = 0; col < sizeX; col++) {
          *pScanline++ = FXARGB_B(argb);
          *pScanline++ = FXARGB_G(argb);
          *pScanline++ = FXARGB_R(argb);
          pScanline += Bpp - 3;
        }
        break;
      }
      case 12: {
        for (int col = 0; col < sizeX; col++) {
          FXARGB_SETDIB(pScanline, argb);
          pScanline += 4;
        }
        break;
      }
    }
  }
  return true;
}

void ProgressiveDecoder::GifReadScanline(int32_t row_num,
                                         pdfium::span<uint8_t> row_buf) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = m_pDeviceBitmap;
  DCHECK(pDIBitmap);
  int32_t img_width = m_GifFrameRect.Width();
  if (!pDIBitmap->IsAlphaFormat()) {
    pdfium::span<uint8_t> byte_span = row_buf;
    for (int i = 0; i < img_width; i++) {
      if (byte_span.front() == m_GifTransIndex) {
        byte_span.front() = m_GifBgIndex;
      }
      byte_span = byte_span.subspan(1);
    }
  }
  int32_t pal_index = m_GifBgIndex;
  if (m_GifTransIndex != -1 && m_pDeviceBitmap->IsAlphaFormat()) {
    pal_index = m_GifTransIndex;
  }
  const int32_t left = m_GifFrameRect.left;
  const pdfium::span<uint8_t> decode_span = m_DecodeBuf;
  fxcrt::spanset(decode_span.first(m_SrcWidth), pal_index);
  fxcrt::spancpy(decode_span.subspan(left), row_buf.first(img_width));

  bool bLastPass = (row_num % 2) == 1;
  int32_t line = row_num + m_GifFrameRect.top;
  int src_top = m_clipBox.top;
  int src_bottom = m_clipBox.bottom;
  int dest_top = m_startY;
  int src_height = m_clipBox.Height();
  int dest_height = m_sizeY;
  if (line < src_top || line >= src_bottom)
    return;

  double scale_y = static_cast<double>(dest_height) / src_height;
  int src_row = line - src_top;
  int dest_row = (int)(src_row * scale_y) + dest_top;
  if (dest_row >= dest_top + dest_height)
    return;

  ResampleScanline(pDIBitmap, dest_row, decode_span, m_SrcFormat);
  if (scale_y > 1.0 && m_SrcPassNumber == 1) {
    ResampleVert(pDIBitmap, scale_y, dest_row);
    return;
  }
  if (scale_y <= 1.0)
    return;

  int dest_bottom = dest_top + m_sizeY;
  int dest_Bpp = pDIBitmap->GetBPP() >> 3;
  uint32_t dest_ScanOffset = m_startX * dest_Bpp;
  if (dest_row + (int)scale_y >= dest_bottom - 1) {
    const uint8_t* scan_src =
        pDIBitmap->GetScanline(dest_row).subspan(dest_ScanOffset).data();
    int cur_row = dest_row;
    while (++cur_row < dest_bottom) {
      uint8_t* scan_des = pDIBitmap->GetWritableScanline(cur_row)
                              .subspan(dest_ScanOffset)
                              .data();
      uint32_t size = m_sizeX * dest_Bpp;
      memmove(scan_des, scan_src, size);
    }
  }
  if (bLastPass)
    GifDoubleLineResampleVert(pDIBitmap, scale_y, dest_row);
}
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_BMP
bool ProgressiveDecoder::BmpInputImagePositionBuf(uint32_t rcd_pos) {
  m_offSet = rcd_pos;
  FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
  return BmpReadMoreData(m_pBmpContext.get(), &error_status);
}

void ProgressiveDecoder::BmpReadScanline(uint32_t row_num,
                                         pdfium::span<const uint8_t> row_buf) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = m_pDeviceBitmap;
  DCHECK(pDIBitmap);

  fxcrt::spancpy(pdfium::make_span(m_DecodeBuf), row_buf.first(m_ScanlineSize));

  int src_top = m_clipBox.top;
  int src_bottom = m_clipBox.bottom;
  int dest_top = m_startY;
  int src_height = m_clipBox.Height();
  int dest_height = m_sizeY;
  if ((src_top >= 0 && row_num < static_cast<uint32_t>(src_top)) ||
      src_bottom < 0 || row_num >= static_cast<uint32_t>(src_bottom)) {
    return;
  }

  double scale_y = static_cast<double>(dest_height) / src_height;
  int src_row = row_num - src_top;
  int dest_row = (int)(src_row * scale_y) + dest_top;
  if (dest_row >= dest_top + dest_height)
    return;

  ResampleScanline(pDIBitmap, dest_row, m_DecodeBuf, m_SrcFormat);
  if (scale_y <= 1.0)
    return;

  if (m_BmpIsTopBottom) {
    ResampleVert(pDIBitmap, scale_y, dest_row);
    return;
  }
  ResampleVertBT(pDIBitmap, scale_y, dest_row);
}

void ProgressiveDecoder::ResampleVertBT(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    double scale_y,
    int dest_row) {
  int dest_Bpp = pDeviceBitmap->GetBPP() >> 3;
  uint32_t dest_ScanOffset = m_startX * dest_Bpp;
  int dest_top = m_startY;
  int dest_bottom = m_startY + m_sizeY;
  FX_SAFE_INT32 check_dest_row_1 = dest_row;
  check_dest_row_1 += pdfium::base::checked_cast<int>(scale_y);
  int dest_row_1 = check_dest_row_1.ValueOrDie();
  if (dest_row_1 >= dest_bottom - 1) {
    const uint8_t* scan_src =
        pDeviceBitmap->GetScanline(dest_row).subspan(dest_ScanOffset).data();
    while (++dest_row < dest_bottom) {
      uint8_t* scan_des = pDeviceBitmap->GetWritableScanline(dest_row)
                              .subspan(dest_ScanOffset)
                              .data();
      uint32_t size = m_sizeX * dest_Bpp;
      memmove(scan_des, scan_src, size);
    }
    return;
  }
  for (; dest_row_1 > dest_row; dest_row_1--) {
    uint8_t* scan_des = pDeviceBitmap->GetWritableScanline(dest_row_1)
                            .subspan(dest_ScanOffset)
                            .data();
    PixelWeight* pWeight = m_WeightVert.GetPixelWeight(dest_row_1 - dest_top);
    const uint8_t* scan_src1 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcStart + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    const uint8_t* scan_src2 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    switch (pDeviceBitmap->GetFormat()) {
      case FXDIB_Format::kInvalid:
      case FXDIB_Format::k1bppMask:
      case FXDIB_Format::k1bppRgb:
        return;
      case FXDIB_Format::k8bppMask:
      case FXDIB_Format::k8bppRgb:
        if (pDeviceBitmap->HasPalette())
          return;
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_g = 0;
          dest_g += pWeight->m_Weights[0] * (*scan_src1++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
        }
        break;
      case FXDIB_Format::kRgb:
      case FXDIB_Format::kRgb32:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          scan_src1 += dest_Bpp - 3;
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          scan_src2 += dest_Bpp - 3;
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          scan_des += dest_Bpp - 3;
        }
        break;
      case FXDIB_Format::kArgb:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_a = pWeight->m_Weights[0] * (*scan_src1++);
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          dest_a += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_a);
        }
        break;
      default:
        return;
    }
  }
}

bool ProgressiveDecoder::BmpDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  std::unique_ptr<ProgressiveDecoderIface::Context> pBmpContext =
      BmpDecoder::StartDecode(this);
  BmpDecoder::Input(pBmpContext.get(), m_pCodecMemory);

  const std::vector<uint32_t>* palette;
  BmpDecoder::Status read_result = BmpDecoder::ReadHeader(
      pBmpContext.get(), &m_SrcWidth, &m_SrcHeight, &m_BmpIsTopBottom,
      &m_SrcComponents, &m_SrcPaletteNumber, &palette, pAttribute);
  while (read_result == BmpDecoder::Status::kContinue) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!BmpReadMoreData(pBmpContext.get(), &error_status)) {
      m_status = error_status;
      return false;
    }
    read_result = BmpDecoder::ReadHeader(
        pBmpContext.get(), &m_SrcWidth, &m_SrcHeight, &m_BmpIsTopBottom,
        &m_SrcComponents, &m_SrcPaletteNumber, &palette, pAttribute);
  }

  if (read_result != BmpDecoder::Status::kSuccess) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }

  FXDIB_Format format = FXDIB_Format::kInvalid;
  switch (m_SrcComponents) {
    case 1:
      m_SrcFormat = FXCodec_8bppRgb;
      format = FXDIB_Format::k8bppRgb;
      break;
    case 3:
      m_SrcFormat = FXCodec_Rgb;
      format = FXDIB_Format::kRgb;
      break;
    case 4:
      m_SrcFormat = FXCodec_Rgb32;
      format = FXDIB_Format::kRgb32;
      break;
    default:
      m_status = FXCODEC_STATUS::kError;
      return false;
  }

  // Set to 0 to make CalculatePitchAndSize() calculate it.
  constexpr uint32_t kNoPitch = 0;
  absl::optional<CFX_DIBitmap::PitchAndSize> needed_data =
      CFX_DIBitmap::CalculatePitchAndSize(m_SrcWidth, m_SrcHeight, format,
                                          kNoPitch);
  if (!needed_data.has_value()) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }

  uint32_t available_data = pdfium::base::checked_cast<uint32_t>(
      m_pFile->GetSize() - m_offSet +
      BmpDecoder::GetAvailInput(pBmpContext.get()));
  if (needed_data.value().size > available_data) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }

  m_SrcBPC = 8;
  m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
  m_pBmpContext = std::move(pBmpContext);
  if (m_SrcPaletteNumber) {
    m_SrcPalette.resize(m_SrcPaletteNumber);
    memcpy(m_SrcPalette.data(), palette->data(),
           m_SrcPaletteNumber * sizeof(FX_ARGB));
  } else {
    m_SrcPalette.clear();
  }
  return true;
}

bool ProgressiveDecoder::BmpReadMoreData(
    ProgressiveDecoderIface::Context* pContext,
    FXCODEC_STATUS* err_status) {
  return ReadMoreData(BmpProgressiveDecoder::GetInstance(), pContext,
                      err_status);
}

FXCODEC_STATUS ProgressiveDecoder::BmpStartDecode() {
  GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
  m_ScanlineSize = FxAlignToBoundary<4>(m_SrcWidth * m_SrcComponents);
  m_DecodeBuf.resize(m_ScanlineSize);
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  m_WeightHorz.CalculateWeights(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                                m_clipBox.Width(), options);
  m_WeightVert.CalculateWeights(m_sizeY, m_clipBox.Height());
  m_status = FXCODEC_STATUS::kDecodeToBeContinued;
  return m_status;
}

FXCODEC_STATUS ProgressiveDecoder::BmpContinueDecode() {
  BmpDecoder::Status read_res = BmpDecoder::LoadImage(m_pBmpContext.get());
  while (read_res == BmpDecoder::Status::kContinue) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
    if (!BmpReadMoreData(m_pBmpContext.get(), &error_status)) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = error_status;
      return m_status;
    }
    read_res = BmpDecoder::LoadImage(m_pBmpContext.get());
  }

  m_pDeviceBitmap = nullptr;
  m_pFile = nullptr;
  m_status = read_res == BmpDecoder::Status::kSuccess
                 ? FXCODEC_STATUS::kDecodeFinished
                 : FXCODEC_STATUS::kError;
  return m_status;
}
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
bool ProgressiveDecoder::GifReadMoreData(FXCODEC_STATUS* err_status) {
  return ReadMoreData(GifProgressiveDecoder::GetInstance(), m_pGifContext.get(),
                      err_status);
}

bool ProgressiveDecoder::GifDetectImageTypeInBuffer() {
  m_pGifContext = GifDecoder::StartDecode(this);
  GifDecoder::Input(m_pGifContext.get(), m_pCodecMemory);
  m_SrcComponents = 1;
  GifDecoder::Status readResult =
      GifDecoder::ReadHeader(m_pGifContext.get(), &m_SrcWidth, &m_SrcHeight,
                             &m_GifPltNumber, &m_pGifPalette, &m_GifBgIndex);
  while (readResult == GifDecoder::Status::kUnfinished) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!GifReadMoreData(&error_status)) {
      m_pGifContext = nullptr;
      m_status = error_status;
      return false;
    }
    readResult =
        GifDecoder::ReadHeader(m_pGifContext.get(), &m_SrcWidth, &m_SrcHeight,
                               &m_GifPltNumber, &m_pGifPalette, &m_GifBgIndex);
  }
  if (readResult == GifDecoder::Status::kSuccess) {
    m_SrcBPC = 8;
    m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
    return true;
  }
  m_pGifContext = nullptr;
  m_status = FXCODEC_STATUS::kError;
  return false;
}

FXCODEC_STATUS ProgressiveDecoder::GifStartDecode() {
  m_SrcFormat = FXCodec_8bppRgb;
  GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
  int scanline_size = FxAlignToBoundary<4>(m_SrcWidth);
  m_DecodeBuf.resize(scanline_size);
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  m_WeightHorz.CalculateWeights(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                                m_clipBox.Width(), options);
  m_WeightVert.CalculateWeights(m_sizeY, m_clipBox.Height());
  m_FrameCur = 0;
  m_status = FXCODEC_STATUS::kDecodeToBeContinued;
  return m_status;
}

FXCODEC_STATUS ProgressiveDecoder::GifContinueDecode() {
  GifDecoder::Status readRes =
      GifDecoder::LoadFrame(m_pGifContext.get(), m_FrameCur);
  while (readRes == GifDecoder::Status::kUnfinished) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
    if (!GifReadMoreData(&error_status)) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = error_status;
      return m_status;
    }
    readRes = GifDecoder::LoadFrame(m_pGifContext.get(), m_FrameCur);
  }

  if (readRes == GifDecoder::Status::kSuccess) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kDecodeFinished;
    return m_status;
  }

  m_pDeviceBitmap = nullptr;
  m_pFile = nullptr;
  m_status = FXCODEC_STATUS::kError;
  return m_status;
}

void ProgressiveDecoder::GifDoubleLineResampleVert(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    double scale_y,
    int dest_row) {
  int dest_Bpp = pDeviceBitmap->GetBPP() >> 3;
  uint32_t dest_ScanOffset = m_startX * dest_Bpp;
  int dest_top = m_startY;
  pdfium::base::CheckedNumeric<double> scale_y2 = scale_y;
  scale_y2 *= 2;
  FX_SAFE_INT32 check_dest_row_1 = dest_row;
  check_dest_row_1 -= scale_y2.ValueOrDie();
  int dest_row_1 = check_dest_row_1.ValueOrDie();
  dest_row_1 = std::max(dest_row_1, dest_top);
  for (; dest_row_1 < dest_row; dest_row_1++) {
    uint8_t* scan_des = pDeviceBitmap->GetWritableScanline(dest_row_1)
                            .subspan(dest_ScanOffset)
                            .data();
    PixelWeight* pWeight = m_WeightVert.GetPixelWeight(dest_row_1 - dest_top);
    const uint8_t* scan_src1 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcStart + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    const uint8_t* scan_src2 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    switch (pDeviceBitmap->GetFormat()) {
      case FXDIB_Format::kInvalid:
      case FXDIB_Format::k1bppMask:
      case FXDIB_Format::k1bppRgb:
        return;
      case FXDIB_Format::k8bppMask:
      case FXDIB_Format::k8bppRgb:
        if (pDeviceBitmap->HasPalette())
          return;
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_g = 0;
          dest_g += pWeight->m_Weights[0] * (*scan_src1++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
        }
        break;
      case FXDIB_Format::kRgb:
      case FXDIB_Format::kRgb32:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          scan_src1 += dest_Bpp - 3;
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          scan_src2 += dest_Bpp - 3;
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          scan_des += dest_Bpp - 3;
        }
        break;
      case FXDIB_Format::kArgb:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_a = pWeight->m_Weights[0] * (*scan_src1++);
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          dest_a += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_a);
        }
        break;
      default:
        return;
    }
  }
  int dest_bottom = dest_top + m_sizeY - 1;
  if (dest_row + (int)(2 * scale_y) >= dest_bottom &&
      dest_row + (int)scale_y < dest_bottom) {
    GifDoubleLineResampleVert(pDeviceBitmap, scale_y, dest_row + (int)scale_y);
  }
}
#endif  // PDF_ENABLE_XFA_GIF

bool ProgressiveDecoder::JpegReadMoreData(FXCODEC_STATUS* err_status) {
  return ReadMoreData(JpegProgressiveDecoder::GetInstance(),
                      m_pJpegContext.get(), err_status);
}

bool ProgressiveDecoder::JpegDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  m_pJpegContext = JpegProgressiveDecoder::Start();
  if (!m_pJpegContext) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  JpegProgressiveDecoder::GetInstance()->Input(m_pJpegContext.get(),
                                               m_pCodecMemory);

  // Setting jump marker before calling ReadHeader, since a longjmp to
  // the marker indicates a fatal error.
  if (setjmp(JpegProgressiveDecoder::GetJumpMark(m_pJpegContext.get())) == -1) {
    m_pJpegContext.reset();
    m_status = FXCODEC_STATUS::kError;
    return false;
  }

  int32_t readResult = JpegProgressiveDecoder::ReadHeader(
      m_pJpegContext.get(), &m_SrcWidth, &m_SrcHeight, &m_SrcComponents,
      pAttribute);
  while (readResult == 2) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!JpegReadMoreData(&error_status)) {
      m_status = error_status;
      return false;
    }
    readResult = JpegProgressiveDecoder::ReadHeader(
        m_pJpegContext.get(), &m_SrcWidth, &m_SrcHeight, &m_SrcComponents,
        pAttribute);
  }
  if (!readResult) {
    m_SrcBPC = 8;
    m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
    return true;
  }
  m_pJpegContext.reset();
  m_status = FXCODEC_STATUS::kError;
  return false;
}

FXCODEC_STATUS ProgressiveDecoder::JpegStartDecode(FXDIB_Format format) {
  int down_scale = GetDownScale();
  // Setting jump marker before calling StartScanLine, since a longjmp to
  // the marker indicates a fatal error.
  if (setjmp(JpegProgressiveDecoder::GetJumpMark(m_pJpegContext.get())) == -1) {
    m_pJpegContext.reset();
    m_status = FXCODEC_STATUS::kError;
    return FXCODEC_STATUS::kError;
  }

  bool startStatus =
      JpegProgressiveDecoder::StartScanline(m_pJpegContext.get(), down_scale);
  while (!startStatus) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!JpegReadMoreData(&error_status)) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = error_status;
      return m_status;
    }

    startStatus =
        JpegProgressiveDecoder::StartScanline(m_pJpegContext.get(), down_scale);
  }
  int scanline_size = (m_SrcWidth + down_scale - 1) / down_scale;
  scanline_size = FxAlignToBoundary<4>(scanline_size * m_SrcComponents);
  m_DecodeBuf.resize(scanline_size);
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  m_WeightHorz.CalculateWeights(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                                m_clipBox.Width(), options);
  m_WeightVert.CalculateWeights(m_sizeY, m_clipBox.Height());
  switch (m_SrcComponents) {
    case 1:
      m_SrcFormat = FXCodec_8bppGray;
      break;
    case 3:
      m_SrcFormat = FXCodec_Rgb;
      break;
    case 4:
      m_SrcFormat = FXCodec_Cmyk;
      break;
  }
  GetTransMethod(format, m_SrcFormat);
  m_status = FXCODEC_STATUS::kDecodeToBeContinued;
  return m_status;
}

FXCODEC_STATUS ProgressiveDecoder::JpegContinueDecode() {
  // JpegModule* pJpegModule = m_pCodecMgr->GetJpegModule();
  // Setting jump marker before calling ReadScanLine, since a longjmp to
  // the marker indicates a fatal error.
  if (setjmp(JpegProgressiveDecoder::GetJumpMark(m_pJpegContext.get())) == -1) {
    m_pJpegContext.reset();
    m_status = FXCODEC_STATUS::kError;
    return FXCODEC_STATUS::kError;
  }

  while (true) {
    bool readRes = JpegProgressiveDecoder::ReadScanline(m_pJpegContext.get(),
                                                        m_DecodeBuf.data());
    while (!readRes) {
      FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
      if (!JpegReadMoreData(&error_status)) {
        m_pDeviceBitmap = nullptr;
        m_pFile = nullptr;
        m_status = error_status;
        return m_status;
      }
      readRes = JpegProgressiveDecoder::ReadScanline(m_pJpegContext.get(),
                                                     m_DecodeBuf.data());
    }
    if (m_SrcFormat == FXCodec_Rgb) {
      int src_Bpp = (m_SrcFormat & 0xff) >> 3;
      RGB2BGR(m_DecodeBuf.data() + m_clipBox.left * src_Bpp, m_clipBox.Width());
    }
    if (m_SrcRow >= m_clipBox.bottom) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = FXCODEC_STATUS::kDecodeFinished;
      return m_status;
    }
    Resample(m_pDeviceBitmap, m_SrcRow, m_DecodeBuf.data(), m_SrcFormat);
    m_SrcRow++;
  }
}

#ifdef PDF_ENABLE_XFA_PNG
void ProgressiveDecoder::PngOneOneMapResampleHorz(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    int32_t dest_line,
    pdfium::span<uint8_t> src_span,
    FXCodec_Format src_format) {
  int32_t src_Bpp = (m_SrcFormat & 0xff) >> 3;
  int32_t dest_Bpp = pDeviceBitmap->GetBPP() >> 3;
  int32_t src_left = m_clipBox.left;
  int32_t dest_left = m_startX;
  uint8_t* src_scan = src_span.subspan(src_left * src_Bpp).data();
  uint8_t* dest_scan = pDeviceBitmap->GetWritableScanline(dest_line)
                           .subspan(dest_left * dest_Bpp)
                           .data();
  switch (pDeviceBitmap->GetFormat()) {
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb:
      NOTREACHED();
      return;
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      if (pDeviceBitmap->HasPalette())
        return;
      for (int32_t dest_col = 0; dest_col < m_sizeX; dest_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(dest_col);
        uint32_t dest_g =
            pPixelWeights->m_Weights[0] * src_scan[pPixelWeights->m_SrcStart];
        dest_g +=
            pPixelWeights->m_Weights[1] * src_scan[pPixelWeights->m_SrcEnd];
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
      }
      break;
    case FXDIB_Format::kRgb:
    case FXDIB_Format::kRgb32:
      for (int32_t dest_col = 0; dest_col < m_sizeX; dest_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(dest_col);
        const uint8_t* p = src_scan + pPixelWeights->m_SrcStart * src_Bpp;
        uint32_t dest_b = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_g = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_r = pPixelWeights->m_Weights[0] * (*p);
        p = src_scan + pPixelWeights->m_SrcEnd * src_Bpp;
        dest_b += pPixelWeights->m_Weights[1] * (*p++);
        dest_g += pPixelWeights->m_Weights[1] * (*p++);
        dest_r += pPixelWeights->m_Weights[1] * (*p);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        dest_scan += dest_Bpp - 3;
      }
      break;
    case FXDIB_Format::kArgb:
      for (int32_t dest_col = 0; dest_col < m_sizeX; dest_col++) {
        PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(dest_col);
        const uint8_t* p = src_scan + pPixelWeights->m_SrcStart * src_Bpp;
        uint32_t dest_b = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_g = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_r = pPixelWeights->m_Weights[0] * (*p++);
        uint32_t dest_a = pPixelWeights->m_Weights[0] * (*p);
        p = src_scan + pPixelWeights->m_SrcEnd * src_Bpp;
        dest_b += pPixelWeights->m_Weights[1] * (*p++);
        dest_g += pPixelWeights->m_Weights[1] * (*p++);
        dest_r += pPixelWeights->m_Weights[1] * (*p++);
        dest_a += pPixelWeights->m_Weights[1] * (*p);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_a);
      }
      break;
    default:
      return;
  }
}

bool ProgressiveDecoder::PngDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  m_pPngContext = PngDecoder::StartDecode(this);
  if (!m_pPngContext) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  while (PngDecoder::ContinueDecode(m_pPngContext.get(), m_pCodecMemory,
                                    pAttribute)) {
    uint32_t remain_size = static_cast<uint32_t>(m_pFile->GetSize()) - m_offSet;
    uint32_t input_size = std::min<uint32_t>(remain_size, kBlockSize);
    if (input_size == 0) {
      m_pPngContext.reset();
      m_status = FXCODEC_STATUS::kError;
      return false;
    }
    if (m_pCodecMemory && input_size > m_pCodecMemory->GetSize())
      m_pCodecMemory = pdfium::MakeRetain<CFX_CodecMemory>(input_size);

    if (!m_pFile->ReadBlockAtOffset(
            m_pCodecMemory->GetBufferSpan().first(input_size), m_offSet)) {
      m_status = FXCODEC_STATUS::kError;
      return false;
    }
    m_offSet += input_size;
  }
  m_pPngContext.reset();
  if (m_SrcPassNumber == 0) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  return true;
}

FXCODEC_STATUS ProgressiveDecoder::PngStartDecode() {
  m_pPngContext = PngDecoder::StartDecode(this);
  if (!m_pPngContext) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  m_offSet = 0;
  switch (m_pDeviceBitmap->GetFormat()) {
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      m_SrcComponents = 1;
      m_SrcFormat = FXCodec_8bppGray;
      break;
    case FXDIB_Format::kRgb:
      m_SrcComponents = 3;
      m_SrcFormat = FXCodec_Rgb;
      break;
    case FXDIB_Format::kRgb32:
    case FXDIB_Format::kArgb:
      m_SrcComponents = 4;
      m_SrcFormat = FXCodec_Argb;
      break;
    default: {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = FXCODEC_STATUS::kError;
      return m_status;
    }
  }
  GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
  int scanline_size = FxAlignToBoundary<4>(m_SrcWidth * m_SrcComponents);
  m_DecodeBuf.resize(scanline_size);
  m_WeightHorzOO.CalculateWeights(m_sizeX, m_clipBox.Width());
  m_WeightVert.CalculateWeights(m_sizeY, m_clipBox.Height());
  m_status = FXCODEC_STATUS::kDecodeToBeContinued;
  return m_status;
}

FXCODEC_STATUS ProgressiveDecoder::PngContinueDecode() {
  while (true) {
    uint32_t remain_size = (uint32_t)m_pFile->GetSize() - m_offSet;
    uint32_t input_size = std::min<uint32_t>(remain_size, kBlockSize);
    if (input_size == 0) {
      m_pPngContext.reset();
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = FXCODEC_STATUS::kDecodeFinished;
      return m_status;
    }
    if (m_pCodecMemory && input_size > m_pCodecMemory->GetSize())
      m_pCodecMemory = pdfium::MakeRetain<CFX_CodecMemory>(input_size);

    bool bResult = m_pFile->ReadBlockAtOffset(
        m_pCodecMemory->GetBufferSpan().first(input_size), m_offSet);
    if (!bResult) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = FXCODEC_STATUS::kError;
      return m_status;
    }
    m_offSet += input_size;
    bResult = PngDecoder::ContinueDecode(m_pPngContext.get(), m_pCodecMemory,
                                         nullptr);
    if (!bResult) {
      m_pDeviceBitmap = nullptr;
      m_pFile = nullptr;
      m_status = FXCODEC_STATUS::kError;
      return m_status;
    }
  }
}
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
bool ProgressiveDecoder::TiffDetectImageTypeFromFile(
    CFX_DIBAttribute* pAttribute) {
  m_pTiffContext = TiffDecoder::CreateDecoder(m_pFile);
  if (!m_pTiffContext) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  int32_t dummy_bpc;
  bool ret = TiffDecoder::LoadFrameInfo(m_pTiffContext.get(), 0, &m_SrcWidth,
                                        &m_SrcHeight, &m_SrcComponents,
                                        &dummy_bpc, pAttribute);
  m_SrcComponents = 4;
  m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
  if (!ret) {
    m_pTiffContext.reset();
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  return true;
}

FXCODEC_STATUS ProgressiveDecoder::TiffContinueDecode() {
  bool ret = false;
  if (m_pDeviceBitmap->GetBPP() == 32 &&
      m_pDeviceBitmap->GetWidth() == m_SrcWidth && m_SrcWidth == m_sizeX &&
      m_pDeviceBitmap->GetHeight() == m_SrcHeight && m_SrcHeight == m_sizeY &&
      m_startX == 0 && m_startY == 0 && m_clipBox.left == 0 &&
      m_clipBox.top == 0 && m_clipBox.right == m_SrcWidth &&
      m_clipBox.bottom == m_SrcHeight) {
    ret = TiffDecoder::Decode(m_pTiffContext.get(), m_pDeviceBitmap);
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    if (!ret) {
      m_status = FXCODEC_STATUS::kError;
      return m_status;
    }
    m_status = FXCODEC_STATUS::kDecodeFinished;
    return m_status;
  }

  auto pDIBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  pDIBitmap->Create(m_SrcWidth, m_SrcHeight, FXDIB_Format::kArgb);
  if (pDIBitmap->GetBuffer().empty()) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  ret = TiffDecoder::Decode(m_pTiffContext.get(), pDIBitmap);
  if (!ret) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  RetainPtr<CFX_DIBitmap> pClipBitmap =
      (m_clipBox.left == 0 && m_clipBox.top == 0 &&
       m_clipBox.right == m_SrcWidth && m_clipBox.bottom == m_SrcHeight)
          ? pDIBitmap
          : pDIBitmap->ClipTo(m_clipBox);
  if (!pClipBitmap) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  RetainPtr<CFX_DIBitmap> pFormatBitmap;
  switch (m_pDeviceBitmap->GetFormat()) {
    case FXDIB_Format::k8bppRgb:
      pFormatBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
      pFormatBitmap->Create(pClipBitmap->GetWidth(), pClipBitmap->GetHeight(),
                            FXDIB_Format::k8bppRgb);
      break;
    case FXDIB_Format::k8bppMask:
      pFormatBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
      pFormatBitmap->Create(pClipBitmap->GetWidth(), pClipBitmap->GetHeight(),
                            FXDIB_Format::k8bppMask);
      break;
    case FXDIB_Format::kRgb:
      pFormatBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
      pFormatBitmap->Create(pClipBitmap->GetWidth(), pClipBitmap->GetHeight(),
                            FXDIB_Format::kRgb);
      break;
    case FXDIB_Format::kRgb32:
      pFormatBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
      pFormatBitmap->Create(pClipBitmap->GetWidth(), pClipBitmap->GetHeight(),
                            FXDIB_Format::kRgb32);
      break;
    case FXDIB_Format::kArgb:
      pFormatBitmap = pClipBitmap;
      break;
    default:
      break;
  }
  switch (m_pDeviceBitmap->GetFormat()) {
    case FXDIB_Format::k8bppRgb:
    case FXDIB_Format::k8bppMask: {
      for (int32_t row = 0; row < pClipBitmap->GetHeight(); row++) {
        const uint8_t* src_line = pClipBitmap->GetScanline(row).data();
        uint8_t* dest_line = pFormatBitmap->GetWritableScanline(row).data();
        for (int32_t col = 0; col < pClipBitmap->GetWidth(); col++) {
          uint8_t _a = 255 - src_line[3];
          uint8_t b = (src_line[0] * src_line[3] + 0xFF * _a) / 255;
          uint8_t g = (src_line[1] * src_line[3] + 0xFF * _a) / 255;
          uint8_t r = (src_line[2] * src_line[3] + 0xFF * _a) / 255;
          *dest_line++ = FXRGB2GRAY(r, g, b);
          src_line += 4;
        }
      }
    } break;
    case FXDIB_Format::kRgb:
    case FXDIB_Format::kRgb32: {
      int32_t desBpp =
          (m_pDeviceBitmap->GetFormat() == FXDIB_Format::kRgb) ? 3 : 4;
      for (int32_t row = 0; row < pClipBitmap->GetHeight(); row++) {
        const uint8_t* src_line = pClipBitmap->GetScanline(row).data();
        uint8_t* dest_line = pFormatBitmap->GetWritableScanline(row).data();
        for (int32_t col = 0; col < pClipBitmap->GetWidth(); col++) {
          uint8_t _a = 255 - src_line[3];
          uint8_t b = (src_line[0] * src_line[3] + 0xFF * _a) / 255;
          uint8_t g = (src_line[1] * src_line[3] + 0xFF * _a) / 255;
          uint8_t r = (src_line[2] * src_line[3] + 0xFF * _a) / 255;
          *dest_line++ = b;
          *dest_line++ = g;
          *dest_line++ = r;
          dest_line += desBpp - 3;
          src_line += 4;
        }
      }
    } break;
    default:
      break;
  }
  if (!pFormatBitmap) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }

  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  RetainPtr<CFX_DIBitmap> pStrechBitmap =
      pFormatBitmap->StretchTo(m_sizeX, m_sizeY, options, nullptr);
  pFormatBitmap = nullptr;
  if (!pStrechBitmap) {
    m_pDeviceBitmap = nullptr;
    m_pFile = nullptr;
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  m_pDeviceBitmap->TransferBitmap(m_startX, m_startY, m_sizeX, m_sizeY,
                                  pStrechBitmap, 0, 0);
  m_pDeviceBitmap = nullptr;
  m_pFile = nullptr;
  m_status = FXCODEC_STATUS::kDecodeFinished;
  return m_status;
}
#endif  // PDF_ENABLE_XFA_TIFF

bool ProgressiveDecoder::DetectImageType(FXCODEC_IMAGE_TYPE imageType,
                                         CFX_DIBAttribute* pAttribute) {
#ifdef PDF_ENABLE_XFA_TIFF
  if (imageType == FXCODEC_IMAGE_TIFF)
    return TiffDetectImageTypeFromFile(pAttribute);
#endif  // PDF_ENABLE_XFA_TIFF

  size_t size = pdfium::base::checked_cast<size_t>(
      std::min<FX_FILESIZE>(m_pFile->GetSize(), kBlockSize));
  m_pCodecMemory = pdfium::MakeRetain<CFX_CodecMemory>(size);
  m_offSet = 0;
  if (!m_pFile->ReadBlockAtOffset(m_pCodecMemory->GetBufferSpan().first(size),
                                  m_offSet)) {
    m_status = FXCODEC_STATUS::kError;
    return false;
  }
  m_offSet += size;

  if (imageType == FXCODEC_IMAGE_JPG)
    return JpegDetectImageTypeInBuffer(pAttribute);

#ifdef PDF_ENABLE_XFA_BMP
  if (imageType == FXCODEC_IMAGE_BMP)
    return BmpDetectImageTypeInBuffer(pAttribute);
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  if (imageType == FXCODEC_IMAGE_GIF)
    return GifDetectImageTypeInBuffer();
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  if (imageType == FXCODEC_IMAGE_PNG)
    return PngDetectImageTypeInBuffer(pAttribute);
#endif  // PDF_ENABLE_XFA_PNG

  m_status = FXCODEC_STATUS::kError;
  return false;
}

bool ProgressiveDecoder::ReadMoreData(
    ProgressiveDecoderIface* pModule,
    ProgressiveDecoderIface::Context* pContext,
    FXCODEC_STATUS* err_status) {
  // Check for EOF.
  if (m_offSet >= static_cast<uint32_t>(m_pFile->GetSize()))
    return false;

  // Try to get whatever remains.
  uint32_t dwBytesToFetchFromFile =
      pdfium::base::checked_cast<uint32_t>(m_pFile->GetSize() - m_offSet);

  // Figure out if the codec stopped processing midway through the buffer.
  size_t dwUnconsumed;
  FX_SAFE_SIZE_T avail_input = pModule->GetAvailInput(pContext);
  if (!avail_input.AssignIfValid(&dwUnconsumed))
    return false;

  if (dwUnconsumed == m_pCodecMemory->GetSize()) {
    // Codec couldn't make any progress against the bytes in the buffer.
    // Increase the buffer size so that there might be enough contiguous
    // bytes to allow whatever operation is having difficulty to succeed.
    dwBytesToFetchFromFile =
        std::min<uint32_t>(dwBytesToFetchFromFile, kBlockSize);
    size_t dwNewSize = m_pCodecMemory->GetSize() + dwBytesToFetchFromFile;
    if (!m_pCodecMemory->TryResize(dwNewSize)) {
      *err_status = FXCODEC_STATUS::kError;
      return false;
    }
  } else {
    // TODO(crbug.com/pdfium/1904): Simplify the `CFX_CodecMemory` API so we
    // don't need to do this awkward dance to free up exactly enough buffer
    // space for the next read.
    size_t dwConsumable = m_pCodecMemory->GetSize() - dwUnconsumed;
    dwBytesToFetchFromFile = pdfium::base::checked_cast<uint32_t>(
        std::min<size_t>(dwBytesToFetchFromFile, dwConsumable));
    m_pCodecMemory->Consume(dwBytesToFetchFromFile);
    m_pCodecMemory->Seek(dwConsumable - dwBytesToFetchFromFile);
    dwUnconsumed += m_pCodecMemory->GetPosition();
  }

  // Append new data past the bytes not yet processed by the codec.
  if (!m_pFile->ReadBlockAtOffset(m_pCodecMemory->GetBufferSpan().subspan(
                                      dwUnconsumed, dwBytesToFetchFromFile),
                                  m_offSet)) {
    *err_status = FXCODEC_STATUS::kError;
    return false;
  }
  m_offSet += dwBytesToFetchFromFile;
  return pModule->Input(pContext, m_pCodecMemory);
}

FXCODEC_STATUS ProgressiveDecoder::LoadImageInfo(
    RetainPtr<IFX_SeekableReadStream> pFile,
    FXCODEC_IMAGE_TYPE imageType,
    CFX_DIBAttribute* pAttribute,
    bool bSkipImageTypeCheck) {
  DCHECK(pAttribute);

  switch (m_status) {
    case FXCODEC_STATUS::kFrameReady:
    case FXCODEC_STATUS::kFrameToBeContinued:
    case FXCODEC_STATUS::kDecodeReady:
    case FXCODEC_STATUS::kDecodeToBeContinued:
      return FXCODEC_STATUS::kError;
    default:
      break;
  }
  m_pFile = std::move(pFile);
  if (!m_pFile) {
    m_status = FXCODEC_STATUS::kError;
    return m_status;
  }
  m_offSet = 0;
  m_SrcWidth = m_SrcHeight = 0;
  m_SrcComponents = m_SrcBPC = 0;
  m_clipBox = FX_RECT();
  m_startX = m_startY = 0;
  m_sizeX = m_sizeY = 0;
  m_SrcPassNumber = 0;
  if (imageType != FXCODEC_IMAGE_UNKNOWN &&
      DetectImageType(imageType, pAttribute)) {
    m_imageType = imageType;
    m_status = FXCODEC_STATUS::kFrameReady;
    return m_status;
  }
  // If we got here then the image data does not match the requested decoder.
  // If we're skipping the type check then bail out at this point and return
  // the failed status.
  if (bSkipImageTypeCheck)
    return m_status;

  for (int type = FXCODEC_IMAGE_UNKNOWN + 1; type < FXCODEC_IMAGE_MAX; type++) {
    if (DetectImageType(static_cast<FXCODEC_IMAGE_TYPE>(type), pAttribute)) {
      m_imageType = static_cast<FXCODEC_IMAGE_TYPE>(type);
      m_status = FXCODEC_STATUS::kFrameReady;
      return m_status;
    }
  }
  m_status = FXCODEC_STATUS::kError;
  m_pFile = nullptr;
  return m_status;
}

void ProgressiveDecoder::SetClipBox(FX_RECT* clip) {
  if (m_status != FXCODEC_STATUS::kFrameReady)
    return;

  if (clip->IsEmpty()) {
    m_clipBox = FX_RECT();
    return;
  }
  clip->left = std::max(clip->left, 0);
  clip->right = std::min(clip->right, m_SrcWidth);
  clip->top = std::max(clip->top, 0);
  clip->bottom = std::min(clip->bottom, m_SrcHeight);
  if (clip->IsEmpty()) {
    m_clipBox = FX_RECT();
    return;
  }
  m_clipBox = *clip;
}

int ProgressiveDecoder::GetDownScale() {
  int down_scale = 1;
  int ratio_w = m_clipBox.Width() / m_sizeX;
  int ratio_h = m_clipBox.Height() / m_sizeY;
  int ratio = std::min(ratio_w, ratio_h);
  if (ratio >= 8)
    down_scale = 8;
  else if (ratio >= 4)
    down_scale = 4;
  else if (ratio >= 2)
    down_scale = 2;

  m_clipBox.left /= down_scale;
  m_clipBox.right /= down_scale;
  m_clipBox.top /= down_scale;
  m_clipBox.bottom /= down_scale;
  if (m_clipBox.right == m_clipBox.left)
    m_clipBox.right = m_clipBox.left + 1;
  if (m_clipBox.bottom == m_clipBox.top)
    m_clipBox.bottom = m_clipBox.top + 1;
  return down_scale;
}

void ProgressiveDecoder::GetTransMethod(FXDIB_Format dest_format,
                                        FXCodec_Format src_format) {
  switch (dest_format) {
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 0;
          break;
        default:
          m_TransMethod = -1;
      }
    } break;
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 1;
          break;
        case FXCodec_8bppGray:
          m_TransMethod = 2;
          break;
        case FXCodec_1bppRgb:
        case FXCodec_8bppRgb:
          m_TransMethod = 3;
          break;
        case FXCodec_Rgb:
        case FXCodec_Rgb32:
        case FXCodec_Argb:
          m_TransMethod = 4;
          break;
        case FXCodec_Cmyk:
          m_TransMethod = 5;
          break;
        default:
          m_TransMethod = -1;
      }
    } break;
    case FXDIB_Format::kRgb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 6;
          break;
        case FXCodec_8bppGray:
          m_TransMethod = 7;
          break;
        case FXCodec_1bppRgb:
        case FXCodec_8bppRgb:
          m_TransMethod = 8;
          break;
        case FXCodec_Rgb:
        case FXCodec_Rgb32:
        case FXCodec_Argb:
          m_TransMethod = 9;
          break;
        case FXCodec_Cmyk:
          m_TransMethod = 10;
          break;
        default:
          m_TransMethod = -1;
      }
    } break;
    case FXDIB_Format::kRgb32:
    case FXDIB_Format::kArgb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 6;
          break;
        case FXCodec_8bppGray:
          m_TransMethod = 7;
          break;
        case FXCodec_1bppRgb:
        case FXCodec_8bppRgb:
          if (dest_format == FXDIB_Format::kArgb) {
            m_TransMethod = 12;
          } else {
            m_TransMethod = 8;
          }
          break;
        case FXCodec_Rgb:
        case FXCodec_Rgb32:
          m_TransMethod = 9;
          break;
        case FXCodec_Cmyk:
          m_TransMethod = 10;
          break;
        case FXCodec_Argb:
          m_TransMethod = 11;
          break;
        default:
          m_TransMethod = -1;
      }
    } break;
    default:
      m_TransMethod = -1;
  }
}

void ProgressiveDecoder::ResampleScanline(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    int dest_line,
    pdfium::span<uint8_t> src_span,
    FXCodec_Format src_format) {
  uint8_t* src_scan = src_span.data();
  int src_left = m_clipBox.left;
  int dest_left = m_startX;
  uint8_t* dest_scan = pDeviceBitmap->GetWritableScanline(dest_line).data();
  int src_bytes_per_pixel = (src_format & 0xff) / 8;
  int dest_bytes_per_pixel = pDeviceBitmap->GetBPP() / 8;
  src_scan += src_left * src_bytes_per_pixel;
  dest_scan += dest_left * dest_bytes_per_pixel;
  for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
    PixelWeight* pPixelWeights = m_WeightHorz.GetPixelWeight(dest_col);
    switch (m_TransMethod) {
      case -1:
        return;
      case 0:
        return;
      case 1:
        return;
      case 2: {
        uint32_t dest_g = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          dest_g += pixel_weight * src_scan[j];
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
      } break;
      case 3: {
        uint32_t dest_r = 0;
        uint32_t dest_g = 0;
        uint32_t dest_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          uint32_t argb = m_SrcPalette[src_scan[j]];
          dest_r += pixel_weight * FXARGB_R(argb);
          dest_g += pixel_weight * FXARGB_G(argb);
          dest_b += pixel_weight * FXARGB_B(argb);
        }
        *dest_scan++ = static_cast<uint8_t>(
            FXRGB2GRAY(CStretchEngine::PixelFromFixed(dest_r),
                       CStretchEngine::PixelFromFixed(dest_g),
                       CStretchEngine::PixelFromFixed(dest_b)));
      } break;
      case 4: {
        uint32_t dest_b = 0;
        uint32_t dest_g = 0;
        uint32_t dest_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
          dest_b += pixel_weight * (*src_pixel++);
          dest_g += pixel_weight * (*src_pixel++);
          dest_r += pixel_weight * (*src_pixel);
        }
        *dest_scan++ = static_cast<uint8_t>(
            FXRGB2GRAY(CStretchEngine::PixelFromFixed(dest_r),
                       CStretchEngine::PixelFromFixed(dest_g),
                       CStretchEngine::PixelFromFixed(dest_b)));
      } break;
      case 5: {
        uint32_t dest_b = 0;
        uint32_t dest_g = 0;
        uint32_t dest_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
          uint8_t src_b = 0;
          uint8_t src_g = 0;
          uint8_t src_r = 0;
          std::tie(src_r, src_g, src_b) =
              AdobeCMYK_to_sRGB1(255 - src_pixel[0], 255 - src_pixel[1],
                                 255 - src_pixel[2], 255 - src_pixel[3]);
          dest_b += pixel_weight * src_b;
          dest_g += pixel_weight * src_g;
          dest_r += pixel_weight * src_r;
        }
        *dest_scan++ = static_cast<uint8_t>(
            FXRGB2GRAY(CStretchEngine::PixelFromFixed(dest_r),
                       CStretchEngine::PixelFromFixed(dest_g),
                       CStretchEngine::PixelFromFixed(dest_b)));
      } break;
      case 6:
        return;
      case 7: {
        uint32_t dest_g = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          dest_g += pixel_weight * src_scan[j];
        }
        memset(dest_scan, CStretchEngine::PixelFromFixed(dest_g), 3);
        dest_scan += dest_bytes_per_pixel;
      } break;
      case 8: {
        uint32_t dest_r = 0;
        uint32_t dest_g = 0;
        uint32_t dest_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          uint32_t argb = m_SrcPalette[src_scan[j]];
          dest_r += pixel_weight * FXARGB_R(argb);
          dest_g += pixel_weight * FXARGB_G(argb);
          dest_b += pixel_weight * FXARGB_B(argb);
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        dest_scan += dest_bytes_per_pixel - 3;
      } break;
      case 12: {
#ifdef PDF_ENABLE_XFA_BMP
        if (m_pBmpContext) {
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
            uint32_t argb = m_SrcPalette[src_scan[j]];
            dest_r += pixel_weight * FXARGB_R(argb);
            dest_g += pixel_weight * FXARGB_G(argb);
            dest_b += pixel_weight * FXARGB_B(argb);
          }
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
          *dest_scan++ = 0xFF;
          break;
        }
#endif  // PDF_ENABLE_XFA_BMP
        uint32_t dest_a = 0;
        uint32_t dest_r = 0;
        uint32_t dest_g = 0;
        uint32_t dest_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          unsigned long argb = m_SrcPalette[src_scan[j]];
          dest_a += pixel_weight * FXARGB_A(argb);
          dest_r += pixel_weight * FXARGB_R(argb);
          dest_g += pixel_weight * FXARGB_G(argb);
          dest_b += pixel_weight * FXARGB_B(argb);
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_a);
      } break;
      case 9: {
        uint32_t dest_b = 0;
        uint32_t dest_g = 0;
        uint32_t dest_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
          dest_b += pixel_weight * (*src_pixel++);
          dest_g += pixel_weight * (*src_pixel++);
          dest_r += pixel_weight * (*src_pixel);
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        dest_scan += dest_bytes_per_pixel - 3;
      } break;
      case 10: {
        uint32_t dest_b = 0;
        uint32_t dest_g = 0;
        uint32_t dest_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
          uint8_t src_b = 0;
          uint8_t src_g = 0;
          uint8_t src_r = 0;
          std::tie(src_r, src_g, src_b) =
              AdobeCMYK_to_sRGB1(255 - src_pixel[0], 255 - src_pixel[1],
                                 255 - src_pixel[2], 255 - src_pixel[3]);
          dest_b += pixel_weight * src_b;
          dest_g += pixel_weight * src_g;
          dest_r += pixel_weight * src_r;
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        dest_scan += dest_bytes_per_pixel - 3;
      } break;
      case 11: {
        uint32_t dest_alpha = 0;
        uint32_t dest_r = 0;
        uint32_t dest_g = 0;
        uint32_t dest_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          uint32_t pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
          pixel_weight = pixel_weight * src_pixel[3] / 255;
          dest_b += pixel_weight * (*src_pixel++);
          dest_g += pixel_weight * (*src_pixel++);
          dest_r += pixel_weight * (*src_pixel);
          dest_alpha += pixel_weight;
        }
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
        *dest_scan++ = CStretchEngine::PixelFromFixed(dest_alpha * 255);
      } break;
      default:
        return;
    }
  }
}

void ProgressiveDecoder::ResampleVert(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    double scale_y,
    int dest_row) {
  int dest_Bpp = pDeviceBitmap->GetBPP() >> 3;
  uint32_t dest_ScanOffset = m_startX * dest_Bpp;
  int dest_top = m_startY;
  FX_SAFE_INT32 check_dest_row_1 = dest_row;
  check_dest_row_1 -= pdfium::base::checked_cast<int>(scale_y);
  int dest_row_1 = check_dest_row_1.ValueOrDie();
  if (dest_row_1 < dest_top) {
    int dest_bottom = dest_top + m_sizeY;
    if (dest_row + (int)scale_y >= dest_bottom - 1) {
      pdfium::span<const uint8_t> scan_src =
          pDeviceBitmap->GetScanline(dest_row).subspan(dest_ScanOffset,
                                                       m_sizeX * dest_Bpp);
      while (++dest_row < dest_bottom) {
        fxcrt::spanmove(pDeviceBitmap->GetWritableScanline(dest_row).subspan(
                            dest_ScanOffset),
                        scan_src);
      }
    }
    return;
  }
  for (; dest_row_1 < dest_row; dest_row_1++) {
    uint8_t* scan_des = pDeviceBitmap->GetWritableScanline(dest_row_1)
                            .subspan(dest_ScanOffset)
                            .data();
    PixelWeight* pWeight = m_WeightVert.GetPixelWeight(dest_row_1 - dest_top);
    const uint8_t* scan_src1 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcStart + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    const uint8_t* scan_src2 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + dest_top)
            .subspan(dest_ScanOffset)
            .data();
    switch (pDeviceBitmap->GetFormat()) {
      case FXDIB_Format::kInvalid:
      case FXDIB_Format::k1bppMask:
      case FXDIB_Format::k1bppRgb:
        return;
      case FXDIB_Format::k8bppMask:
      case FXDIB_Format::k8bppRgb:
        if (pDeviceBitmap->HasPalette())
          return;
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_g = 0;
          dest_g += pWeight->m_Weights[0] * (*scan_src1++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
        }
        break;
      case FXDIB_Format::kRgb:
      case FXDIB_Format::kRgb32:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          scan_src1 += dest_Bpp - 3;
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          scan_src2 += dest_Bpp - 3;
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          scan_des += dest_Bpp - 3;
        }
        break;
      case FXDIB_Format::kArgb:
        for (int dest_col = 0; dest_col < m_sizeX; dest_col++) {
          uint32_t dest_b = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_g = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_r = pWeight->m_Weights[0] * (*scan_src1++);
          uint32_t dest_a = pWeight->m_Weights[0] * (*scan_src1++);
          dest_b += pWeight->m_Weights[1] * (*scan_src2++);
          dest_g += pWeight->m_Weights[1] * (*scan_src2++);
          dest_r += pWeight->m_Weights[1] * (*scan_src2++);
          dest_a += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_b);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_g);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_r);
          *scan_des++ = CStretchEngine::PixelFromFixed(dest_a);
        }
        break;
      default:
        return;
    }
  }
  int dest_bottom = dest_top + m_sizeY;
  if (dest_row + (int)scale_y >= dest_bottom - 1) {
    pdfium::span<const uint8_t> scan_src =
        pDeviceBitmap->GetScanline(dest_row).subspan(dest_ScanOffset,
                                                     m_sizeX * dest_Bpp);
    while (++dest_row < dest_bottom) {
      fxcrt::spanmove(
          pDeviceBitmap->GetWritableScanline(dest_row).subspan(dest_ScanOffset),
          scan_src);
    }
  }
}

void ProgressiveDecoder::Resample(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                                  int32_t src_line,
                                  uint8_t* src_scan,
                                  FXCodec_Format src_format) {
  int src_top = m_clipBox.top;
  int dest_top = m_startY;
  int src_height = m_clipBox.Height();
  int dest_height = m_sizeY;
  if (src_line >= src_top) {
    double scale_y = static_cast<double>(dest_height) / src_height;
    int src_row = src_line - src_top;
    int dest_row = (int)(src_row * scale_y) + dest_top;
    if (dest_row >= dest_top + dest_height)
      return;

    ResampleScanline(pDeviceBitmap, dest_row, m_DecodeBuf, src_format);
    if (scale_y > 1.0)
      ResampleVert(pDeviceBitmap, scale_y, dest_row);
  }
}

std::pair<FXCODEC_STATUS, size_t> ProgressiveDecoder::GetFrames() {
  if (!(m_status == FXCODEC_STATUS::kFrameReady ||
        m_status == FXCODEC_STATUS::kFrameToBeContinued)) {
    return {FXCODEC_STATUS::kError, 0};
  }

  switch (m_imageType) {
#ifdef PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_BMP:
#endif  // PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_JPG:
#ifdef PDF_ENABLE_XFA_PNG
    case FXCODEC_IMAGE_PNG:
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
    case FXCODEC_IMAGE_TIFF:
#endif  // PDF_ENABLE_XFA_TIFF
      m_FrameNumber = 1;
      m_status = FXCODEC_STATUS::kDecodeReady;
      return {m_status, 1};
#ifdef PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_GIF: {
      while (true) {
        GifDecoder::Status readResult;
        std::tie(readResult, m_FrameNumber) =
            GifDecoder::LoadFrameInfo(m_pGifContext.get());
        while (readResult == GifDecoder::Status::kUnfinished) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
          if (!GifReadMoreData(&error_status))
            return {error_status, 0};

          std::tie(readResult, m_FrameNumber) =
              GifDecoder::LoadFrameInfo(m_pGifContext.get());
        }
        if (readResult == GifDecoder::Status::kSuccess) {
          m_status = FXCODEC_STATUS::kDecodeReady;
          return {m_status, m_FrameNumber};
        }
        m_pGifContext = nullptr;
        m_status = FXCODEC_STATUS::kError;
        return {m_status, 0};
      }
    }
#endif  // PDF_ENABLE_XFA_GIF
    default:
      return {FXCODEC_STATUS::kError, 0};
  }
}

FXCODEC_STATUS ProgressiveDecoder::StartDecode(
    const RetainPtr<CFX_DIBitmap>& pDIBitmap,
    int start_x,
    int start_y,
    int size_x,
    int size_y) {
  if (m_status != FXCODEC_STATUS::kDecodeReady)
    return FXCODEC_STATUS::kError;

  if (!pDIBitmap || pDIBitmap->GetBPP() < 8 || m_FrameNumber == 0)
    return FXCODEC_STATUS::kError;

  m_pDeviceBitmap = pDIBitmap;
  if (m_clipBox.IsEmpty())
    return FXCODEC_STATUS::kError;
  if (size_x <= 0 || size_x > 65535 || size_y <= 0 || size_y > 65535)
    return FXCODEC_STATUS::kError;

  FX_RECT device_rc =
      FX_RECT(start_x, start_y, start_x + size_x, start_y + size_y);
  int32_t out_range_x = device_rc.right - pDIBitmap->GetWidth();
  int32_t out_range_y = device_rc.bottom - pDIBitmap->GetHeight();
  device_rc.Intersect(
      FX_RECT(0, 0, pDIBitmap->GetWidth(), pDIBitmap->GetHeight()));
  if (device_rc.IsEmpty())
    return FXCODEC_STATUS::kError;

  m_startX = device_rc.left;
  m_startY = device_rc.top;
  m_sizeX = device_rc.Width();
  m_sizeY = device_rc.Height();
  m_FrameCur = 0;
  if (start_x < 0 || out_range_x > 0) {
    float scaleX = (float)m_clipBox.Width() / (float)size_x;
    if (start_x < 0) {
      m_clipBox.left -= static_cast<int32_t>(ceil((float)start_x * scaleX));
    }
    if (out_range_x > 0) {
      m_clipBox.right -=
          static_cast<int32_t>(floor((float)out_range_x * scaleX));
    }
  }
  if (start_y < 0 || out_range_y > 0) {
    float scaleY = (float)m_clipBox.Height() / (float)size_y;
    if (start_y < 0) {
      m_clipBox.top -= static_cast<int32_t>(ceil((float)start_y * scaleY));
    }
    if (out_range_y > 0) {
      m_clipBox.bottom -=
          static_cast<int32_t>(floor((float)out_range_y * scaleY));
    }
  }
  if (m_clipBox.IsEmpty()) {
    return FXCODEC_STATUS::kError;
  }
  switch (m_imageType) {
#ifdef PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_BMP:
      return BmpStartDecode();
#endif  // PDF_ENABLE_XFA_BMP
#ifdef PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_GIF:
      return GifStartDecode();
#endif  // PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_JPG:
      return JpegStartDecode(pDIBitmap->GetFormat());
#ifdef PDF_ENABLE_XFA_PNG
    case FXCODEC_IMAGE_PNG:
      return PngStartDecode();
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
    case FXCODEC_IMAGE_TIFF:
      m_status = FXCODEC_STATUS::kDecodeToBeContinued;
      return m_status;
#endif  // PDF_ENABLE_XFA_TIFF
    default:
      return FXCODEC_STATUS::kError;
  }
}

FXCODEC_STATUS ProgressiveDecoder::ContinueDecode() {
  if (m_status != FXCODEC_STATUS::kDecodeToBeContinued)
    return FXCODEC_STATUS::kError;

  switch (m_imageType) {
    case FXCODEC_IMAGE_JPG:
      return JpegContinueDecode();
#ifdef PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_BMP:
      return BmpContinueDecode();
#endif  // PDF_ENABLE_XFA_BMP
#ifdef PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_GIF:
      return GifContinueDecode();
#endif  // PDF_ENABLE_XFA_GIF
#ifdef PDF_ENABLE_XFA_PNG
    case FXCODEC_IMAGE_PNG:
      return PngContinueDecode();
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
    case FXCODEC_IMAGE_TIFF:
      return TiffContinueDecode();
#endif  // PDF_ENABLE_XFA_TIFF
    default:
      return FXCODEC_STATUS::kError;
  }
}

}  // namespace fxcodec
