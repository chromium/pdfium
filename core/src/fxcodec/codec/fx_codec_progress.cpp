// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxge/fx_dib.h"
#include "core/include/fxcodec/fx_codec.h"
#include "fx_codec_progress.h"
void CFXCODEC_WeightTable::Calc(int dest_len,
                                int dest_min,
                                int dest_max,
                                int src_len,
                                int src_min,
                                int src_max,
                                FX_BOOL bInterpol) {
  if (m_pWeightTables) {
    FX_Free(m_pWeightTables);
  }
  double scale, base;
  scale = FXSYS_Div((FX_FLOAT)(src_len), (FX_FLOAT)(dest_len));
  if (dest_len < 0) {
    base = (FX_FLOAT)(src_len);
  } else {
    base = 0.0f;
  }
  m_ItemSize =
      (int)(sizeof(int) * 2 +
            sizeof(int) * (FXSYS_ceil(FXSYS_fabs((FX_FLOAT)scale)) + 1));
  m_DestMin = dest_min;
  m_pWeightTables = FX_Alloc(uint8_t, (dest_max - dest_min) * m_ItemSize + 4);
  if (m_pWeightTables == NULL) {
    return;
  }
  if (FXSYS_fabs((FX_FLOAT)scale) < 1.0f) {
    for (int dest_pixel = dest_min; dest_pixel < dest_max; dest_pixel++) {
      PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
      double src_pos = dest_pixel * scale + scale / 2 + base;
      if (bInterpol) {
        pixel_weights.m_SrcStart =
            (int)FXSYS_floor((FX_FLOAT)src_pos - 1.0f / 2);
        pixel_weights.m_SrcEnd = (int)FXSYS_floor((FX_FLOAT)src_pos + 1.0f / 2);
        if (pixel_weights.m_SrcStart < src_min) {
          pixel_weights.m_SrcStart = src_min;
        }
        if (pixel_weights.m_SrcEnd >= src_max) {
          pixel_weights.m_SrcEnd = src_max - 1;
        }
        if (pixel_weights.m_SrcStart == pixel_weights.m_SrcEnd) {
          pixel_weights.m_Weights[0] = 65536;
        } else {
          pixel_weights.m_Weights[1] = FXSYS_round(
              (FX_FLOAT)(src_pos - pixel_weights.m_SrcStart - 1.0f / 2) *
              65536);
          pixel_weights.m_Weights[0] = 65536 - pixel_weights.m_Weights[1];
        }
      } else {
        pixel_weights.m_SrcStart = pixel_weights.m_SrcEnd =
            (int)FXSYS_floor((FX_FLOAT)src_pos);
        pixel_weights.m_Weights[0] = 65536;
      }
    }
    return;
  }
  for (int dest_pixel = dest_min; dest_pixel < dest_max; dest_pixel++) {
    PixelWeight& pixel_weights = *GetPixelWeight(dest_pixel);
    double src_start = dest_pixel * scale + base;
    double src_end = src_start + scale;
    int start_i, end_i;
    if (src_start < src_end) {
      start_i = (int)FXSYS_floor((FX_FLOAT)src_start);
      end_i = (int)FXSYS_ceil((FX_FLOAT)src_end);
    } else {
      start_i = (int)FXSYS_floor((FX_FLOAT)src_end);
      end_i = (int)FXSYS_ceil((FX_FLOAT)src_start);
    }
    if (start_i < src_min) {
      start_i = src_min;
    }
    if (end_i >= src_max) {
      end_i = src_max - 1;
    }
    if (start_i > end_i) {
      pixel_weights.m_SrcStart = start_i;
      pixel_weights.m_SrcEnd = start_i;
      continue;
    }
    pixel_weights.m_SrcStart = start_i;
    pixel_weights.m_SrcEnd = end_i;
    for (int j = start_i; j <= end_i; j++) {
      double dest_start = FXSYS_Div((FX_FLOAT)(j)-base, scale);
      double dest_end = FXSYS_Div((FX_FLOAT)(j + 1) - base, scale);
      if (dest_start > dest_end) {
        double temp = dest_start;
        dest_start = dest_end;
        dest_end = temp;
      }
      double area_start = dest_start > (FX_FLOAT)(dest_pixel)
                              ? dest_start
                              : (FX_FLOAT)(dest_pixel);
      double area_end = dest_end > (FX_FLOAT)(dest_pixel + 1)
                            ? (FX_FLOAT)(dest_pixel + 1)
                            : dest_end;
      double weight = area_start >= area_end ? 0.0f : area_end - area_start;
      if (weight == 0 && j == end_i) {
        pixel_weights.m_SrcEnd--;
        break;
      }
      pixel_weights.m_Weights[j - start_i] =
          FXSYS_round((FX_FLOAT)(weight * 65536));
    }
  }
}
void CFXCODEC_HorzTable::Calc(int dest_len, int src_len, FX_BOOL bInterpol) {
  if (m_pWeightTables) {
    FX_Free(m_pWeightTables);
  }
  double scale = (double)dest_len / (double)src_len;
  m_ItemSize = sizeof(int) * 4;
  int size = dest_len * m_ItemSize + 4;
  m_pWeightTables = FX_Alloc(uint8_t, size);
  if (m_pWeightTables == NULL) {
    return;
  }
  FXSYS_memset(m_pWeightTables, 0, size);
  if (scale > 1) {
    int pre_des_col = 0;
    for (int src_col = 0; src_col < src_len; src_col++) {
      double des_col_f = src_col * scale;
      int des_col = FXSYS_round((FX_FLOAT)des_col_f);
      PixelWeight* pWeight =
          (PixelWeight*)(m_pWeightTables + des_col * m_ItemSize);
      pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
      pWeight->m_Weights[0] = 65536;
      pWeight->m_Weights[1] = 0;
      if (src_col == src_len - 1 && des_col < dest_len - 1) {
        for (int des_col_index = pre_des_col + 1; des_col_index < dest_len;
             des_col_index++) {
          pWeight =
              (PixelWeight*)(m_pWeightTables + des_col_index * m_ItemSize);
          pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
          pWeight->m_Weights[0] = 65536;
          pWeight->m_Weights[1] = 0;
        }
        return;
      }
      int des_col_len = des_col - pre_des_col;
      for (int des_col_index = pre_des_col + 1; des_col_index < des_col;
           des_col_index++) {
        pWeight = (PixelWeight*)(m_pWeightTables + des_col_index * m_ItemSize);
        pWeight->m_SrcStart = src_col - 1;
        pWeight->m_SrcEnd = src_col;
        pWeight->m_Weights[0] =
            bInterpol ? FXSYS_round((FX_FLOAT)(
                            ((FX_FLOAT)des_col - (FX_FLOAT)des_col_index) /
                            (FX_FLOAT)des_col_len * 65536))
                      : 65536;
        pWeight->m_Weights[1] = 65536 - pWeight->m_Weights[0];
      }
      pre_des_col = des_col;
    }
    return;
  }
  for (int des_col = 0; des_col < dest_len; des_col++) {
    double src_col_f = des_col / scale;
    int src_col = FXSYS_round((FX_FLOAT)src_col_f);
    PixelWeight* pWeight =
        (PixelWeight*)(m_pWeightTables + des_col * m_ItemSize);
    pWeight->m_SrcStart = pWeight->m_SrcEnd = src_col;
    pWeight->m_Weights[0] = 65536;
    pWeight->m_Weights[1] = 0;
  }
}
void CFXCODEC_VertTable::Calc(int dest_len, int src_len) {
  if (m_pWeightTables) {
    FX_Free(m_pWeightTables);
  }
  double scale = (double)dest_len / (double)src_len;
  m_ItemSize = sizeof(int) * 4;
  int size = dest_len * m_ItemSize + 4;
  m_pWeightTables = FX_Alloc(uint8_t, size);
  if (m_pWeightTables == NULL) {
    return;
  }
  FXSYS_memset(m_pWeightTables, 0, size);
  if (scale > 1) {
    double step = 0.0;
    int src_row = 0;
    while (step < (double)dest_len) {
      int start_step = (int)step;
      step = scale * (++src_row);
      int end_step = (int)step;
      if (end_step >= dest_len) {
        end_step = dest_len;
        for (int des_row = start_step; des_row < end_step; des_row++) {
          PixelWeight* pWeight =
              (PixelWeight*)(m_pWeightTables + des_row * m_ItemSize);
          pWeight->m_SrcStart = start_step;
          pWeight->m_SrcEnd = start_step;
          pWeight->m_Weights[0] = 65536;
          pWeight->m_Weights[1] = 0;
        }
        return;
      }
      int length = end_step - start_step;
      {
        PixelWeight* pWeight =
            (PixelWeight*)(m_pWeightTables + start_step * m_ItemSize);
        pWeight->m_SrcStart = start_step;
        pWeight->m_SrcEnd = start_step;
        pWeight->m_Weights[0] = 65536;
        pWeight->m_Weights[1] = 0;
      }
      for (int des_row = start_step + 1; des_row < end_step; des_row++) {
        PixelWeight* pWeight =
            (PixelWeight*)(m_pWeightTables + des_row * m_ItemSize);
        pWeight->m_SrcStart = start_step;
        pWeight->m_SrcEnd = end_step;
        pWeight->m_Weights[0] = FXSYS_round((FX_FLOAT)(end_step - des_row) /
                                            (FX_FLOAT)length * 65536);
        pWeight->m_Weights[1] = 65536 - pWeight->m_Weights[0];
      }
    }
  } else {
    for (int des_row = 0; des_row < dest_len; des_row++) {
      PixelWeight* pWeight =
          (PixelWeight*)(m_pWeightTables + des_row * m_ItemSize);
      pWeight->m_SrcStart = des_row;
      pWeight->m_SrcEnd = des_row;
      pWeight->m_Weights[0] = 65536;
      pWeight->m_Weights[1] = 0;
    }
  }
}
CCodec_ProgressiveDecoder::CCodec_ProgressiveDecoder(
    CCodec_ModuleMgr* pCodecMgr) {
  m_pFile = NULL;
  m_pJpegContext = NULL;
  m_pPngContext = NULL;
  m_pGifContext = NULL;
  m_pBmpContext = NULL;
  m_pTiffContext = NULL;
  m_pCodecMgr = NULL;
  m_pSrcBuf = NULL;
  m_pDecodeBuf = NULL;
  m_pDeviceBitmap = NULL;
  m_pSrcPalette = NULL;
  m_pCodecMgr = pCodecMgr;
  m_offSet = 0;
  m_SrcSize = 0;
  m_ScanlineSize = 0;
  m_SrcWidth = m_SrcHeight = 0;
  m_SrcComponents = 0;
  m_SrcBPC = 0;
  m_SrcPassNumber = 0;
  m_clipBox = FX_RECT(0, 0, 0, 0);
  m_imagType = FXCODEC_IMAGE_UNKNOWN;
  m_status = FXCODEC_STATUS_DECODE_FINISH;
  m_TransMethod = -1;
  m_SrcRow = 0;
  m_SrcFormat = FXCodec_Invalid;
  m_bInterpol = TRUE;
  m_FrameNumber = 0;
  m_FrameCur = 0;
  m_SrcPaletteNumber = 0;
  m_GifPltNumber = 0;
  m_GifBgIndex = 0;
  m_pGifPalette = NULL;
  m_GifTransIndex = -1;
  m_GifFrameRect = FX_RECT(0, 0, 0, 0);
  m_BmpIsTopBottom = FALSE;
}
CCodec_ProgressiveDecoder::~CCodec_ProgressiveDecoder() {
  m_pFile = NULL;
  if (m_pJpegContext != NULL) {
    m_pCodecMgr->GetJpegModule()->Finish(m_pJpegContext);
  }
  if (m_pPngContext != NULL) {
    m_pCodecMgr->GetPngModule()->Finish(m_pPngContext);
  }
  if (m_pGifContext != NULL) {
    m_pCodecMgr->GetGifModule()->Finish(m_pGifContext);
  }
  if (m_pBmpContext != NULL) {
    m_pCodecMgr->GetBmpModule()->Finish(m_pBmpContext);
  }
  if (m_pTiffContext != NULL) {
    m_pCodecMgr->GetTiffModule()->DestroyDecoder(m_pTiffContext);
  }
  if (m_pSrcBuf != NULL) {
    FX_Free(m_pSrcBuf);
  }
  if (m_pDecodeBuf != NULL) {
    FX_Free(m_pDecodeBuf);
  }
  if (m_pSrcPalette != NULL) {
    FX_Free(m_pSrcPalette);
  }
}
FX_BOOL CCodec_ProgressiveDecoder::JpegReadMoreData(
    ICodec_JpegModule* pJpegModule,
    FXCODEC_STATUS& err_status) {
  FX_DWORD dwSize = (FX_DWORD)m_pFile->GetSize();
  if (dwSize <= m_offSet) {
    return FALSE;
  }
  dwSize = dwSize - m_offSet;
  FX_DWORD dwAvail = pJpegModule->GetAvailInput(m_pJpegContext, NULL);
  if (dwAvail == m_SrcSize) {
    if (dwSize > FXCODEC_BLOCK_SIZE) {
      dwSize = FXCODEC_BLOCK_SIZE;
    }
    m_SrcSize = (dwSize + dwAvail + FXCODEC_BLOCK_SIZE - 1) /
                FXCODEC_BLOCK_SIZE * FXCODEC_BLOCK_SIZE;
    m_pSrcBuf = FX_Realloc(uint8_t, m_pSrcBuf, m_SrcSize);
    if (!m_pSrcBuf) {
      err_status = FXCODEC_STATUS_ERR_MEMORY;
      return FALSE;
    }
  } else {
    FX_DWORD dwConsume = m_SrcSize - dwAvail;
    if (dwAvail) {
      FXSYS_memcpy(m_pSrcBuf, m_pSrcBuf + dwConsume, dwAvail);
    }
    if (dwSize > dwConsume) {
      dwSize = dwConsume;
    }
  }
  if (!m_pFile->ReadBlock(m_pSrcBuf + dwAvail, m_offSet, dwSize)) {
    err_status = FXCODEC_STATUS_ERR_READ;
    return FALSE;
  }
  m_offSet += dwSize;
  pJpegModule->Input(m_pJpegContext, m_pSrcBuf, dwSize + dwAvail);
  return TRUE;
}
FX_BOOL CCodec_ProgressiveDecoder::PngReadHeaderFunc(void* pModule,
                                                     int width,
                                                     int height,
                                                     int bpc,
                                                     int pass,
                                                     int* color_type,
                                                     double* gamma) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  if (pCodec->m_pDeviceBitmap == NULL) {
    pCodec->m_SrcWidth = width;
    pCodec->m_SrcHeight = height;
    pCodec->m_SrcBPC = bpc;
    pCodec->m_SrcPassNumber = pass;
    pCodec->m_SrcComponents =
        *color_type == 0 ? 1 : *color_type == 2
                                   ? 3
                                   : *color_type == 3
                                         ? 4
                                         : *color_type == 4
                                               ? 2
                                               : *color_type == 6 ? 4 : 0;
    pCodec->m_clipBox = FX_RECT(0, 0, width, height);
    return FALSE;
  }
  FXDIB_Format format = pCodec->m_pDeviceBitmap->GetFormat();
  switch (format) {
    case FXDIB_1bppMask:
    case FXDIB_1bppRgb:
      ASSERT(FALSE);
      return FALSE;
    case FXDIB_8bppMask:
    case FXDIB_8bppRgb:
      *color_type = 0;
      break;
    case FXDIB_Rgb:
      *color_type = 2;
      break;
    case FXDIB_Rgb32:
    case FXDIB_Argb:
      *color_type = 6;
      break;
    default:
      ASSERT(FALSE);
      return FALSE;
  }
  *gamma = FXCODEC_PNG_GAMMA;
  return TRUE;
}
FX_BOOL CCodec_ProgressiveDecoder::PngAskScanlineBufFunc(void* pModule,
                                                         int line,
                                                         uint8_t*& src_buf) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  CFX_DIBitmap* pDIBitmap = pCodec->m_pDeviceBitmap;
  ASSERT(pDIBitmap != NULL);
  if (pDIBitmap == NULL) {
    return FALSE;
  }
  if (line >= pCodec->m_clipBox.top && line < pCodec->m_clipBox.bottom) {
    double scale_y =
        (double)pCodec->m_sizeY / (double)pCodec->m_clipBox.Height();
    int32_t row =
        (int32_t)((line - pCodec->m_clipBox.top) * scale_y) + pCodec->m_startY;
    uint8_t* src_scan = (uint8_t*)pDIBitmap->GetScanline(row);
    uint8_t* des_scan = pCodec->m_pDecodeBuf;
    src_buf = pCodec->m_pDecodeBuf;
    int32_t src_Bpp = pDIBitmap->GetBPP() >> 3;
    int32_t des_Bpp = (pCodec->m_SrcFormat & 0xff) >> 3;
    int32_t src_left = pCodec->m_startX;
    int32_t des_left = pCodec->m_clipBox.left;
    src_scan += src_left * src_Bpp;
    des_scan += des_left * des_Bpp;
    for (int32_t src_col = 0; src_col < pCodec->m_sizeX; src_col++) {
      PixelWeight* pPixelWeights =
          pCodec->m_WeightHorzOO.GetPixelWeight(src_col);
      if (pPixelWeights->m_SrcStart != pPixelWeights->m_SrcEnd) {
        continue;
      }
      switch (pDIBitmap->GetFormat()) {
        case FXDIB_1bppMask:
        case FXDIB_1bppRgb:
          ASSERT(FALSE);
          return FALSE;
        case FXDIB_8bppMask:
        case FXDIB_8bppRgb: {
          if (pDIBitmap->GetPalette() != NULL) {
            return FALSE;
          }
          FX_DWORD des_g = 0;
          des_g += pPixelWeights->m_Weights[0] * src_scan[src_col];
          des_scan[pPixelWeights->m_SrcStart] = (uint8_t)(des_g >> 16);
        } break;
        case FXDIB_Rgb:
        case FXDIB_Rgb32: {
          FX_DWORD des_b = 0, des_g = 0, des_r = 0;
          const uint8_t* p = src_scan + src_col * src_Bpp;
          des_b += pPixelWeights->m_Weights[0] * (*p++);
          des_g += pPixelWeights->m_Weights[0] * (*p++);
          des_r += pPixelWeights->m_Weights[0] * (*p);
          uint8_t* pDes = &des_scan[pPixelWeights->m_SrcStart * des_Bpp];
          *pDes++ = (uint8_t)((des_b) >> 16);
          *pDes++ = (uint8_t)((des_g) >> 16);
          *pDes = (uint8_t)((des_r) >> 16);
        } break;
        case FXDIB_Argb: {
          FX_DWORD des_r = 0, des_g = 0, des_b = 0;
          const uint8_t* p = src_scan + src_col * src_Bpp;
          des_b += pPixelWeights->m_Weights[0] * (*p++);
          des_g += pPixelWeights->m_Weights[0] * (*p++);
          des_r += pPixelWeights->m_Weights[0] * (*p++);
          uint8_t* pDes = &des_scan[pPixelWeights->m_SrcStart * des_Bpp];
          *pDes++ = (uint8_t)((des_b) >> 16);
          *pDes++ = (uint8_t)((des_g) >> 16);
          *pDes++ = (uint8_t)((des_r) >> 16);
          *pDes = *p;
        } break;
        default:
          return FALSE;
      }
    }
  }
  return TRUE;
}
void CCodec_ProgressiveDecoder::PngOneOneMapResampleHorz(
    CFX_DIBitmap* pDeviceBitmap,
    int32_t des_line,
    uint8_t* src_scan,
    FXCodec_Format src_format) {
  uint8_t* des_scan = (uint8_t*)pDeviceBitmap->GetScanline(des_line);
  int32_t src_Bpp = (m_SrcFormat & 0xff) >> 3;
  int32_t des_Bpp = pDeviceBitmap->GetBPP() >> 3;
  int32_t src_left = m_clipBox.left;
  int32_t des_left = m_startX;
  src_scan += src_left * src_Bpp;
  des_scan += des_left * des_Bpp;
  for (int32_t des_col = 0; des_col < m_sizeX; des_col++) {
    PixelWeight* pPixelWeights = m_WeightHorzOO.GetPixelWeight(des_col);
    switch (pDeviceBitmap->GetFormat()) {
      case FXDIB_1bppMask:
      case FXDIB_1bppRgb:
        ASSERT(FALSE);
        return;
      case FXDIB_8bppMask:
      case FXDIB_8bppRgb: {
        if (pDeviceBitmap->GetPalette() != NULL) {
          return;
        }
        FX_DWORD des_g = 0;
        des_g +=
            pPixelWeights->m_Weights[0] * src_scan[pPixelWeights->m_SrcStart];
        des_g +=
            pPixelWeights->m_Weights[1] * src_scan[pPixelWeights->m_SrcEnd];
        *des_scan++ = (uint8_t)(des_g >> 16);
      } break;
      case FXDIB_Rgb:
      case FXDIB_Rgb32: {
        FX_DWORD des_b = 0, des_g = 0, des_r = 0;
        const uint8_t* p = src_scan;
        p = src_scan + pPixelWeights->m_SrcStart * src_Bpp;
        des_b += pPixelWeights->m_Weights[0] * (*p++);
        des_g += pPixelWeights->m_Weights[0] * (*p++);
        des_r += pPixelWeights->m_Weights[0] * (*p);
        p = src_scan + pPixelWeights->m_SrcEnd * src_Bpp;
        des_b += pPixelWeights->m_Weights[1] * (*p++);
        des_g += pPixelWeights->m_Weights[1] * (*p++);
        des_r += pPixelWeights->m_Weights[1] * (*p);
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        des_scan += des_Bpp - 3;
      } break;
      case FXDIB_Argb: {
        FX_DWORD des_a = 0, des_b = 0, des_g = 0, des_r = 0;
        const uint8_t* p = src_scan;
        p = src_scan + pPixelWeights->m_SrcStart * src_Bpp;
        des_b += pPixelWeights->m_Weights[0] * (*p++);
        des_g += pPixelWeights->m_Weights[0] * (*p++);
        des_r += pPixelWeights->m_Weights[0] * (*p++);
        des_a += pPixelWeights->m_Weights[0] * (*p);
        p = src_scan + pPixelWeights->m_SrcEnd * src_Bpp;
        des_b += pPixelWeights->m_Weights[1] * (*p++);
        des_g += pPixelWeights->m_Weights[1] * (*p++);
        des_r += pPixelWeights->m_Weights[1] * (*p++);
        des_a += pPixelWeights->m_Weights[1] * (*p);
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        *des_scan++ = (uint8_t)((des_a) >> 16);
      } break;
      default:
        return;
    }
  }
}
void CCodec_ProgressiveDecoder::PngFillScanlineBufCompletedFunc(void* pModule,
                                                                int pass,
                                                                int line) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  CFX_DIBitmap* pDIBitmap = pCodec->m_pDeviceBitmap;
  ASSERT(pDIBitmap != NULL);
  int src_top = pCodec->m_clipBox.top;
  int src_bottom = pCodec->m_clipBox.bottom;
  int des_top = pCodec->m_startY;
  int src_hei = pCodec->m_clipBox.Height();
  int des_hei = pCodec->m_sizeY;
  if (line >= src_top && line < src_bottom) {
    double scale_y = (double)des_hei / (double)src_hei;
    int src_row = line - src_top;
    int des_row = (int)(src_row * scale_y) + des_top;
    if (des_row >= des_top + des_hei) {
      return;
    }
    pCodec->PngOneOneMapResampleHorz(pDIBitmap, des_row, pCodec->m_pDecodeBuf,
                                     pCodec->m_SrcFormat);
    if (pCodec->m_SrcPassNumber == 1 && scale_y > 1.0) {
      pCodec->ResampleVert(pDIBitmap, scale_y, des_row);
      return;
    }
    if (pass == 6 && scale_y > 1.0) {
      pCodec->ResampleVert(pDIBitmap, scale_y, des_row);
    }
  }
}
FX_BOOL CCodec_ProgressiveDecoder::GifReadMoreData(ICodec_GifModule* pGifModule,
                                                   FXCODEC_STATUS& err_status) {
  FX_DWORD dwSize = (FX_DWORD)m_pFile->GetSize();
  if (dwSize <= m_offSet) {
    return FALSE;
  }
  dwSize = dwSize - m_offSet;
  FX_DWORD dwAvail = pGifModule->GetAvailInput(m_pGifContext, NULL);
  if (dwAvail == m_SrcSize) {
    if (dwSize > FXCODEC_BLOCK_SIZE) {
      dwSize = FXCODEC_BLOCK_SIZE;
    }
    m_SrcSize = (dwSize + dwAvail + FXCODEC_BLOCK_SIZE - 1) /
                FXCODEC_BLOCK_SIZE * FXCODEC_BLOCK_SIZE;
    m_pSrcBuf = FX_Realloc(uint8_t, m_pSrcBuf, m_SrcSize);
    if (!m_pSrcBuf) {
      err_status = FXCODEC_STATUS_ERR_MEMORY;
      return FALSE;
    }
  } else {
    FX_DWORD dwConsume = m_SrcSize - dwAvail;
    if (dwAvail) {
      FXSYS_memcpy(m_pSrcBuf, m_pSrcBuf + dwConsume, dwAvail);
    }
    if (dwSize > dwConsume) {
      dwSize = dwConsume;
    }
  }
  if (!m_pFile->ReadBlock(m_pSrcBuf + dwAvail, m_offSet, dwSize)) {
    err_status = FXCODEC_STATUS_ERR_READ;
    return FALSE;
  }
  m_offSet += dwSize;
  pGifModule->Input(m_pGifContext, m_pSrcBuf, dwSize + dwAvail);
  return TRUE;
}
void CCodec_ProgressiveDecoder::GifRecordCurrentPositionCallback(
    void* pModule,
    FX_DWORD& cur_pos) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  FX_DWORD remain_size =
      pCodec->m_pCodecMgr->GetGifModule()->GetAvailInput(pCodec->m_pGifContext);
  cur_pos = pCodec->m_offSet - remain_size;
}
uint8_t* CCodec_ProgressiveDecoder::GifAskLocalPaletteBufCallback(
    void* pModule,
    int32_t frame_num,
    int32_t pal_size) {
  return FX_Alloc(uint8_t, pal_size);
}
FX_BOOL CCodec_ProgressiveDecoder::GifInputRecordPositionBufCallback(
    void* pModule,
    FX_DWORD rcd_pos,
    const FX_RECT& img_rc,
    int32_t pal_num,
    void* pal_ptr,
    int32_t delay_time,
    FX_BOOL user_input,
    int32_t trans_index,
    int32_t disposal_method,
    FX_BOOL interlace) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  pCodec->m_offSet = rcd_pos;
  FXCODEC_STATUS error_status = FXCODEC_STATUS_ERROR;
  if (!pCodec->GifReadMoreData(pCodec->m_pCodecMgr->GetGifModule(),
                               error_status)) {
    return FALSE;
  }
  uint8_t* pPalette = NULL;
  if (pal_num != 0 && pal_ptr) {
    pPalette = (uint8_t*)pal_ptr;
  } else {
    pal_num = pCodec->m_GifPltNumber;
    pPalette = pCodec->m_pGifPalette;
  }
  if (pCodec->m_pSrcPalette == NULL) {
    pCodec->m_pSrcPalette = FX_Alloc(FX_ARGB, pal_num);
  } else if (pal_num > pCodec->m_SrcPaletteNumber) {
    pCodec->m_pSrcPalette = FX_Realloc(FX_ARGB, pCodec->m_pSrcPalette, pal_num);
  }
  if (pCodec->m_pSrcPalette == NULL) {
    return FALSE;
  }
  pCodec->m_SrcPaletteNumber = pal_num;
  for (int i = 0; i < pal_num; i++) {
    FX_DWORD j = i * 3;
    pCodec->m_pSrcPalette[i] =
        ArgbEncode(0xff, pPalette[j], pPalette[j + 1], pPalette[j + 2]);
  }
  pCodec->m_GifTransIndex = trans_index;
  pCodec->m_GifFrameRect = img_rc;
  pCodec->m_SrcPassNumber = interlace ? 4 : 1;
  int32_t pal_index = pCodec->m_GifBgIndex;
  CFX_DIBitmap* pDevice = pCodec->m_pDeviceBitmap;
  if (trans_index >= pal_num) {
    trans_index = -1;
  }
  if (trans_index != -1) {
    pCodec->m_pSrcPalette[trans_index] &= 0x00ffffff;
    if (pDevice->HasAlpha()) {
      pal_index = trans_index;
    }
  }
  int startX = pCodec->m_startX;
  int startY = pCodec->m_startY;
  int sizeX = pCodec->m_sizeX;
  int sizeY = pCodec->m_sizeY;
  int Bpp = pDevice->GetBPP() / 8;
  FX_ARGB argb = pCodec->m_pSrcPalette[pal_index];
  for (int row = 0; row < sizeY; row++) {
    uint8_t* pScanline =
        (uint8_t*)pDevice->GetScanline(row + startY) + startX * Bpp;
    switch (pCodec->m_TransMethod) {
      case 3: {
        uint8_t gray =
            FXRGB2GRAY(FXARGB_R(argb), FXARGB_G(argb), FXARGB_B(argb));
        FXSYS_memset(pScanline, gray, sizeX);
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
  return TRUE;
}
void CCodec_ProgressiveDecoder::GifReadScanlineCallback(void* pModule,
                                                        int32_t row_num,
                                                        uint8_t* row_buf) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  CFX_DIBitmap* pDIBitmap = pCodec->m_pDeviceBitmap;
  ASSERT(pDIBitmap != NULL);
  int32_t img_width = pCodec->m_GifFrameRect.Width();
  if (!pDIBitmap->HasAlpha()) {
    uint8_t* byte_ptr = row_buf;
    for (int i = 0; i < img_width; i++) {
      if (*byte_ptr == pCodec->m_GifTransIndex) {
        *byte_ptr = pCodec->m_GifBgIndex;
      }
      byte_ptr++;
    }
  }
  int32_t pal_index = pCodec->m_GifBgIndex;
  if (pCodec->m_GifTransIndex != -1 && pCodec->m_pDeviceBitmap->HasAlpha()) {
    pal_index = pCodec->m_GifTransIndex;
  }
  FXSYS_memset(pCodec->m_pDecodeBuf, pal_index, pCodec->m_SrcWidth);
  FX_BOOL bLastPass = (row_num % 2) == 1;
  int32_t line = row_num + pCodec->m_GifFrameRect.top;
  int32_t left = pCodec->m_GifFrameRect.left;
  FXSYS_memcpy(pCodec->m_pDecodeBuf + left, row_buf, img_width);
  int src_top = pCodec->m_clipBox.top;
  int src_bottom = pCodec->m_clipBox.bottom;
  int des_top = pCodec->m_startY;
  int src_hei = pCodec->m_clipBox.Height();
  int des_hei = pCodec->m_sizeY;
  if (line >= src_top && line < src_bottom) {
    double scale_y = (double)des_hei / (double)src_hei;
    int src_row = line - src_top;
    int des_row = (int)(src_row * scale_y) + des_top;
    if (des_row >= des_top + des_hei) {
      return;
    }
    pCodec->ReSampleScanline(pDIBitmap, des_row, pCodec->m_pDecodeBuf,
                             pCodec->m_SrcFormat);
    if (scale_y > 1.0 &&
        (!pCodec->m_bInterpol || pCodec->m_SrcPassNumber == 1)) {
      pCodec->ResampleVert(pDIBitmap, scale_y, des_row);
      return;
    }
    if (scale_y > 1.0) {
      int des_bottom = des_top + pCodec->m_sizeY;
      int des_Bpp = pDIBitmap->GetBPP() >> 3;
      FX_DWORD des_ScanOffet = pCodec->m_startX * des_Bpp;
      if (des_row + (int)scale_y >= des_bottom - 1) {
        uint8_t* scan_src =
            (uint8_t*)pDIBitmap->GetScanline(des_row) + des_ScanOffet;
        int cur_row = des_row;
        while (++cur_row < des_bottom) {
          uint8_t* scan_des =
              (uint8_t*)pDIBitmap->GetScanline(cur_row) + des_ScanOffet;
          FX_DWORD size = pCodec->m_sizeX * des_Bpp;
          FXSYS_memcpy(scan_des, scan_src, size);
        }
      }
      if (bLastPass) {
        pCodec->GifDoubleLineResampleVert(pDIBitmap, scale_y, des_row);
      }
    }
  }
}
void CCodec_ProgressiveDecoder::GifDoubleLineResampleVert(
    CFX_DIBitmap* pDeviceBitmap,
    double scale_y,
    int des_row) {
  int des_Bpp = pDeviceBitmap->GetBPP() >> 3;
  FX_DWORD des_ScanOffet = m_startX * des_Bpp;
  int des_top = m_startY;
  int des_row_1 = des_row - int(2 * scale_y);
  if (des_row_1 < des_top) {
    des_row_1 = des_top;
  }
  for (; des_row_1 < des_row; des_row_1++) {
    uint8_t* scan_des =
        (uint8_t*)pDeviceBitmap->GetScanline(des_row_1) + des_ScanOffet;
    PixelWeight* pWeight = m_WeightVert.GetPixelWeight(des_row_1 - des_top);
    const uint8_t* scan_src1 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcStart + des_top) +
        des_ScanOffet;
    const uint8_t* scan_src2 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + des_top) + des_ScanOffet;
    for (int des_col = 0; des_col < m_sizeX; des_col++) {
      switch (pDeviceBitmap->GetFormat()) {
        case FXDIB_Invalid:
        case FXDIB_1bppMask:
        case FXDIB_1bppRgb:
          return;
        case FXDIB_8bppMask:
        case FXDIB_8bppRgb: {
          if (pDeviceBitmap->GetPalette() != NULL) {
            return;
          }
          int des_g = 0;
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = (uint8_t)(des_g >> 16);
        } break;
        case FXDIB_Rgb:
        case FXDIB_Rgb32: {
          FX_DWORD des_b = 0, des_g = 0, des_r = 0;
          des_b += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_r += pWeight->m_Weights[0] * (*scan_src1++);
          scan_src1 += des_Bpp - 3;
          des_b += pWeight->m_Weights[1] * (*scan_src2++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          des_r += pWeight->m_Weights[1] * (*scan_src2++);
          scan_src2 += des_Bpp - 3;
          *scan_des++ = (uint8_t)((des_b) >> 16);
          *scan_des++ = (uint8_t)((des_g) >> 16);
          *scan_des++ = (uint8_t)((des_r) >> 16);
          scan_des += des_Bpp - 3;
        } break;
        case FXDIB_Argb: {
          FX_DWORD des_a = 0, des_b = 0, des_g = 0, des_r = 0;
          des_b += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_r += pWeight->m_Weights[0] * (*scan_src1++);
          des_a += pWeight->m_Weights[0] * (*scan_src1++);
          des_b += pWeight->m_Weights[1] * (*scan_src2++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          des_r += pWeight->m_Weights[1] * (*scan_src2++);
          des_a += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = (uint8_t)((des_b) >> 16);
          *scan_des++ = (uint8_t)((des_g) >> 16);
          *scan_des++ = (uint8_t)((des_r) >> 16);
          *scan_des++ = (uint8_t)((des_a) >> 16);
        } break;
        default:
          return;
      }
    }
  }
  int des_bottom = des_top + m_sizeY - 1;
  if (des_row + (int)(2 * scale_y) >= des_bottom &&
      des_row + (int)scale_y < des_bottom) {
    GifDoubleLineResampleVert(pDeviceBitmap, scale_y, des_row + (int)scale_y);
  }
}
FX_BOOL CCodec_ProgressiveDecoder::BmpReadMoreData(ICodec_BmpModule* pBmpModule,
                                                   FXCODEC_STATUS& err_status) {
  FX_DWORD dwSize = (FX_DWORD)m_pFile->GetSize();
  if (dwSize <= m_offSet) {
    return FALSE;
  }
  dwSize = dwSize - m_offSet;
  FX_DWORD dwAvail = pBmpModule->GetAvailInput(m_pBmpContext, NULL);
  if (dwAvail == m_SrcSize) {
    if (dwSize > FXCODEC_BLOCK_SIZE) {
      dwSize = FXCODEC_BLOCK_SIZE;
    }
    m_SrcSize = (dwSize + dwAvail + FXCODEC_BLOCK_SIZE - 1) /
                FXCODEC_BLOCK_SIZE * FXCODEC_BLOCK_SIZE;
    m_pSrcBuf = FX_Realloc(uint8_t, m_pSrcBuf, m_SrcSize);
    if (!m_pSrcBuf) {
      err_status = FXCODEC_STATUS_ERR_MEMORY;
      return FALSE;
    }
  } else {
    FX_DWORD dwConsume = m_SrcSize - dwAvail;
    if (dwAvail) {
      FXSYS_memcpy(m_pSrcBuf, m_pSrcBuf + dwConsume, dwAvail);
    }
    if (dwSize > dwConsume) {
      dwSize = dwConsume;
    }
  }
  if (!m_pFile->ReadBlock(m_pSrcBuf + dwAvail, m_offSet, dwSize)) {
    err_status = FXCODEC_STATUS_ERR_READ;
    return FALSE;
  }
  m_offSet += dwSize;
  pBmpModule->Input(m_pBmpContext, m_pSrcBuf, dwSize + dwAvail);
  return TRUE;
}
FX_BOOL CCodec_ProgressiveDecoder::BmpInputImagePositionBufCallback(
    void* pModule,
    FX_DWORD rcd_pos) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  pCodec->m_offSet = rcd_pos;
  FXCODEC_STATUS error_status = FXCODEC_STATUS_ERROR;
  if (!pCodec->BmpReadMoreData(pCodec->m_pCodecMgr->GetBmpModule(),
                               error_status)) {
    return FALSE;
  }
  return TRUE;
}
void CCodec_ProgressiveDecoder::BmpReadScanlineCallback(void* pModule,
                                                        int32_t row_num,
                                                        uint8_t* row_buf) {
  CCodec_ProgressiveDecoder* pCodec = (CCodec_ProgressiveDecoder*)pModule;
  CFX_DIBitmap* pDIBitmap = pCodec->m_pDeviceBitmap;
  ASSERT(pDIBitmap != NULL);
  FXSYS_memcpy(pCodec->m_pDecodeBuf, row_buf, pCodec->m_ScanlineSize);
  int src_top = pCodec->m_clipBox.top;
  int src_bottom = pCodec->m_clipBox.bottom;
  int des_top = pCodec->m_startY;
  int src_hei = pCodec->m_clipBox.Height();
  int des_hei = pCodec->m_sizeY;
  if (row_num >= src_top && row_num < src_bottom) {
    double scale_y = (double)des_hei / (double)src_hei;
    int src_row = row_num - src_top;
    int des_row = (int)(src_row * scale_y) + des_top;
    if (des_row >= des_top + des_hei) {
      return;
    }
    pCodec->ReSampleScanline(pDIBitmap, des_row, pCodec->m_pDecodeBuf,
                             pCodec->m_SrcFormat);
    if (scale_y > 1.0) {
      if (pCodec->m_BmpIsTopBottom || !pCodec->m_bInterpol) {
        pCodec->ResampleVert(pDIBitmap, scale_y, des_row);
        return;
      } else {
        pCodec->ResampleVertBT(pDIBitmap, scale_y, des_row);
      }
    }
  }
}
void CCodec_ProgressiveDecoder::ResampleVertBT(CFX_DIBitmap* pDeviceBitmap,
                                               double scale_y,
                                               int des_row) {
  int des_Bpp = pDeviceBitmap->GetBPP() >> 3;
  FX_DWORD des_ScanOffet = m_startX * des_Bpp;
  int des_top = m_startY;
  int des_bottom = m_startY + m_sizeY;
  int des_row_1 = des_row + int(scale_y);
  if (des_row_1 >= des_bottom - 1) {
    uint8_t* scan_src =
        (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
    while (++des_row < des_bottom) {
      uint8_t* scan_des =
          (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
      FX_DWORD size = m_sizeX * des_Bpp;
      FXSYS_memcpy(scan_des, scan_src, size);
    }
    return;
  }
  for (; des_row_1 > des_row; des_row_1--) {
    uint8_t* scan_des =
        (uint8_t*)pDeviceBitmap->GetScanline(des_row_1) + des_ScanOffet;
    PixelWeight* pWeight = m_WeightVert.GetPixelWeight(des_row_1 - des_top);
    const uint8_t* scan_src1 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcStart + des_top) +
        des_ScanOffet;
    const uint8_t* scan_src2 =
        pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + des_top) + des_ScanOffet;
    for (int des_col = 0; des_col < m_sizeX; des_col++) {
      switch (pDeviceBitmap->GetFormat()) {
        case FXDIB_Invalid:
        case FXDIB_1bppMask:
        case FXDIB_1bppRgb:
          return;
        case FXDIB_8bppMask:
        case FXDIB_8bppRgb: {
          if (pDeviceBitmap->GetPalette() != NULL) {
            return;
          }
          int des_g = 0;
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = (uint8_t)(des_g >> 16);
        } break;
        case FXDIB_Rgb:
        case FXDIB_Rgb32: {
          FX_DWORD des_b = 0, des_g = 0, des_r = 0;
          des_b += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_r += pWeight->m_Weights[0] * (*scan_src1++);
          scan_src1 += des_Bpp - 3;
          des_b += pWeight->m_Weights[1] * (*scan_src2++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          des_r += pWeight->m_Weights[1] * (*scan_src2++);
          scan_src2 += des_Bpp - 3;
          *scan_des++ = (uint8_t)((des_b) >> 16);
          *scan_des++ = (uint8_t)((des_g) >> 16);
          *scan_des++ = (uint8_t)((des_r) >> 16);
          scan_des += des_Bpp - 3;
        } break;
        case FXDIB_Argb: {
          FX_DWORD des_a = 0, des_b = 0, des_g = 0, des_r = 0;
          des_b += pWeight->m_Weights[0] * (*scan_src1++);
          des_g += pWeight->m_Weights[0] * (*scan_src1++);
          des_r += pWeight->m_Weights[0] * (*scan_src1++);
          des_a += pWeight->m_Weights[0] * (*scan_src1++);
          des_b += pWeight->m_Weights[1] * (*scan_src2++);
          des_g += pWeight->m_Weights[1] * (*scan_src2++);
          des_r += pWeight->m_Weights[1] * (*scan_src2++);
          des_a += pWeight->m_Weights[1] * (*scan_src2++);
          *scan_des++ = (uint8_t)((des_b) >> 16);
          *scan_des++ = (uint8_t)((des_g) >> 16);
          *scan_des++ = (uint8_t)((des_r) >> 16);
          *scan_des++ = (uint8_t)((des_a) >> 16);
        } break;
        default:
          return;
      }
    }
  }
}
FX_BOOL CCodec_ProgressiveDecoder::DetectImageType(
    FXCODEC_IMAGE_TYPE imageType,
    CFX_DIBAttribute* pAttribute) {
  m_offSet = 0;
  FX_DWORD size = (FX_DWORD)m_pFile->GetSize();
  if (size > FXCODEC_BLOCK_SIZE) {
    size = FXCODEC_BLOCK_SIZE;
  }
  if (m_pSrcBuf != NULL) {
    FX_Free(m_pSrcBuf);
    m_pSrcBuf = NULL;
  }
  m_pSrcBuf = FX_Alloc(uint8_t, size);
  if (m_pSrcBuf == NULL) {
    m_status = FXCODEC_STATUS_ERR_MEMORY;
    return FALSE;
  }
  FXSYS_memset(m_pSrcBuf, 0, size);
  m_SrcSize = size;
  switch (imageType) {
    case FXCODEC_IMAGE_BMP: {
      ICodec_BmpModule* pBmpModule = m_pCodecMgr->GetBmpModule();
      if (pBmpModule == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      pBmpModule->InputImagePositionBufCallback =
          BmpInputImagePositionBufCallback;
      pBmpModule->ReadScanlineCallback = BmpReadScanlineCallback;
      m_pBmpContext = pBmpModule->Start((void*)this);
      if (m_pBmpContext == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      FX_BOOL bResult = m_pFile->ReadBlock(m_pSrcBuf, 0, size);
      if (!bResult) {
        m_status = FXCODEC_STATUS_ERR_READ;
        return FALSE;
      }
      m_offSet += size;
      pBmpModule->Input(m_pBmpContext, m_pSrcBuf, size);
      FX_DWORD* pPalette = NULL;
      int32_t readResult = pBmpModule->ReadHeader(
          m_pBmpContext, &m_SrcWidth, &m_SrcHeight, &m_BmpIsTopBottom,
          &m_SrcComponents, &m_SrcPaletteNumber, &pPalette, pAttribute);
      while (readResult == 2) {
        FXCODEC_STATUS error_status = FXCODEC_STATUS_ERR_FORMAT;
        if (!BmpReadMoreData(pBmpModule, error_status)) {
          m_status = error_status;
          return FALSE;
        }
        readResult = pBmpModule->ReadHeader(
            m_pBmpContext, &m_SrcWidth, &m_SrcHeight, &m_BmpIsTopBottom,
            &m_SrcComponents, &m_SrcPaletteNumber, &pPalette, pAttribute);
      }
      if (readResult == 1) {
        m_SrcBPC = 8;
        m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
        if (m_pSrcPalette != NULL) {
          FX_Free(m_pSrcPalette);
          m_pSrcPalette = NULL;
        }
        if (m_SrcPaletteNumber) {
          m_pSrcPalette = FX_Alloc(FX_ARGB, m_SrcPaletteNumber);
          if (m_pSrcPalette == NULL) {
            m_status = FXCODEC_STATUS_ERR_MEMORY;
            return FALSE;
          }
          FXSYS_memcpy(m_pSrcPalette, pPalette,
                       m_SrcPaletteNumber * sizeof(FX_DWORD));
        }
        return TRUE;
      }
      if (m_pBmpContext != NULL) {
        pBmpModule->Finish(m_pBmpContext);
        m_pBmpContext = NULL;
      }
      m_status = FXCODEC_STATUS_ERR_FORMAT;
      return FALSE;
    } break;
    case FXCODEC_IMAGE_JPG: {
      ICodec_JpegModule* pJpegModule = m_pCodecMgr->GetJpegModule();
      if (pJpegModule == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      m_pJpegContext = pJpegModule->Start();
      if (m_pJpegContext == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      FX_BOOL bResult = m_pFile->ReadBlock(m_pSrcBuf, 0, size);
      if (!bResult) {
        m_status = FXCODEC_STATUS_ERR_READ;
        return FALSE;
      }
      m_offSet += size;
      pJpegModule->Input(m_pJpegContext, m_pSrcBuf, size);
      int32_t readResult =
          pJpegModule->ReadHeader(m_pJpegContext, &m_SrcWidth, &m_SrcHeight,
                                  &m_SrcComponents, pAttribute);
      while (readResult == 2) {
        FXCODEC_STATUS error_status = FXCODEC_STATUS_ERR_FORMAT;
        if (!JpegReadMoreData(pJpegModule, error_status)) {
          m_status = error_status;
          return FALSE;
        }
        readResult =
            pJpegModule->ReadHeader(m_pJpegContext, &m_SrcWidth, &m_SrcHeight,
                                    &m_SrcComponents, pAttribute);
      }
      if (!readResult) {
        m_SrcBPC = 8;
        m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
        return TRUE;
      }
      if (m_pJpegContext != NULL) {
        pJpegModule->Finish(m_pJpegContext);
        m_pJpegContext = NULL;
      }
      m_status = FXCODEC_STATUS_ERR_FORMAT;
      return FALSE;
    } break;
    case FXCODEC_IMAGE_PNG: {
      ICodec_PngModule* pPngModule = m_pCodecMgr->GetPngModule();
      if (pPngModule == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      pPngModule->ReadHeaderCallback =
          CCodec_ProgressiveDecoder::PngReadHeaderFunc;
      pPngModule->AskScanlineBufCallback =
          CCodec_ProgressiveDecoder::PngAskScanlineBufFunc;
      pPngModule->FillScanlineBufCompletedCallback =
          CCodec_ProgressiveDecoder::PngFillScanlineBufCompletedFunc;
      m_pPngContext = pPngModule->Start((void*)this);
      if (m_pPngContext == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      FX_BOOL bResult = m_pFile->ReadBlock(m_pSrcBuf, 0, size);
      if (!bResult) {
        m_status = FXCODEC_STATUS_ERR_READ;
        return FALSE;
      }
      m_offSet += size;
      bResult = pPngModule->Input(m_pPngContext, m_pSrcBuf, size, pAttribute);
      while (bResult) {
        FX_DWORD remain_size = (FX_DWORD)m_pFile->GetSize() - m_offSet;
        FX_DWORD input_size =
            remain_size > FXCODEC_BLOCK_SIZE ? FXCODEC_BLOCK_SIZE : remain_size;
        if (input_size == 0) {
          if (m_pPngContext != NULL) {
            pPngModule->Finish(m_pPngContext);
          }
          m_pPngContext = NULL;
          m_status = FXCODEC_STATUS_ERR_FORMAT;
          return FALSE;
        }
        if (m_pSrcBuf != NULL && input_size > m_SrcSize) {
          FX_Free(m_pSrcBuf);
          m_pSrcBuf = FX_Alloc(uint8_t, input_size);
          if (m_pSrcBuf == NULL) {
            m_status = FXCODEC_STATUS_ERR_MEMORY;
            return FALSE;
          }
          FXSYS_memset(m_pSrcBuf, 0, input_size);
          m_SrcSize = input_size;
        }
        bResult = m_pFile->ReadBlock(m_pSrcBuf, m_offSet, input_size);
        if (!bResult) {
          m_status = FXCODEC_STATUS_ERR_READ;
          return FALSE;
        }
        m_offSet += input_size;
        bResult =
            pPngModule->Input(m_pPngContext, m_pSrcBuf, input_size, pAttribute);
      }
      ASSERT(!bResult);
      if (m_pPngContext != NULL) {
        pPngModule->Finish(m_pPngContext);
        m_pPngContext = NULL;
      }
      if (m_SrcPassNumber == 0) {
        m_status = FXCODEC_STATUS_ERR_FORMAT;
        return FALSE;
      }
    } break;
    case FXCODEC_IMAGE_GIF: {
      ICodec_GifModule* pGifModule = m_pCodecMgr->GetGifModule();
      if (pGifModule == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      pGifModule->RecordCurrentPositionCallback =
          CCodec_ProgressiveDecoder::GifRecordCurrentPositionCallback;
      pGifModule->AskLocalPaletteBufCallback =
          CCodec_ProgressiveDecoder::GifAskLocalPaletteBufCallback;
      pGifModule->InputRecordPositionBufCallback =
          CCodec_ProgressiveDecoder::GifInputRecordPositionBufCallback;
      pGifModule->ReadScanlineCallback =
          CCodec_ProgressiveDecoder::GifReadScanlineCallback;
      m_pGifContext = pGifModule->Start((void*)this);
      if (m_pGifContext == NULL) {
        m_status = FXCODEC_STATUS_ERR_MEMORY;
        return FALSE;
      }
      FX_BOOL bResult = m_pFile->ReadBlock(m_pSrcBuf, 0, size);
      if (!bResult) {
        m_status = FXCODEC_STATUS_ERR_READ;
        return FALSE;
      }
      m_offSet += size;
      pGifModule->Input(m_pGifContext, m_pSrcBuf, size);
      m_SrcComponents = 1;
      int32_t readResult = pGifModule->ReadHeader(
          m_pGifContext, &m_SrcWidth, &m_SrcHeight, &m_GifPltNumber,
          (void**)&m_pGifPalette, &m_GifBgIndex, nullptr);
      while (readResult == 2) {
        FXCODEC_STATUS error_status = FXCODEC_STATUS_ERR_FORMAT;
        if (!GifReadMoreData(pGifModule, error_status)) {
          m_status = error_status;
          return FALSE;
        }
        readResult = pGifModule->ReadHeader(
            m_pGifContext, &m_SrcWidth, &m_SrcHeight, &m_GifPltNumber,
            (void**)&m_pGifPalette, &m_GifBgIndex, nullptr);
      }
      if (readResult == 1) {
        m_SrcBPC = 8;
        m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
        return TRUE;
      }
      if (m_pGifContext != NULL) {
        pGifModule->Finish(m_pGifContext);
        m_pGifContext = NULL;
      }
      m_status = FXCODEC_STATUS_ERR_FORMAT;
      return FALSE;
    } break;
    case FXCODEC_IMAGE_TIF: {
      ICodec_TiffModule* pTiffModule = m_pCodecMgr->GetTiffModule();
      if (pTiffModule == NULL) {
        m_status = FXCODEC_STATUS_ERR_FORMAT;
        return FALSE;
      }
      m_pTiffContext = pTiffModule->CreateDecoder(m_pFile);
      if (m_pTiffContext == NULL) {
        m_status = FXCODEC_STATUS_ERR_FORMAT;
        return FALSE;
      }
      int32_t frames = 0;
      pTiffModule->GetFrames(m_pTiffContext, frames);
      FX_DWORD bpc;
      FX_BOOL ret = pTiffModule->LoadFrameInfo(
          m_pTiffContext, 0, (FX_DWORD&)m_SrcWidth, (FX_DWORD&)m_SrcHeight,
          (FX_DWORD&)m_SrcComponents, bpc, pAttribute);
      m_SrcComponents = 4;
      m_clipBox = FX_RECT(0, 0, m_SrcWidth, m_SrcHeight);
      if (!ret) {
        pTiffModule->DestroyDecoder(m_pTiffContext);
        (m_pTiffContext = NULL);
        (m_status = FXCODEC_STATUS_ERR_FORMAT);
        return FALSE;
      }
    } break;
    default:
      m_status = FXCODEC_STATUS_ERR_FORMAT;
      return FALSE;
  }
  return TRUE;
}
FXCODEC_STATUS CCodec_ProgressiveDecoder::LoadImageInfo(
    IFX_FileRead* pFile,
    FXCODEC_IMAGE_TYPE imageType,
    CFX_DIBAttribute* pAttribute) {
  switch (m_status) {
    case FXCODEC_STATUS_FRAME_READY:
    case FXCODEC_STATUS_FRAME_TOBECONTINUE:
    case FXCODEC_STATUS_DECODE_READY:
    case FXCODEC_STATUS_DECODE_TOBECONTINUE:
      return FXCODEC_STATUS_ERROR;
    default:;
  }
  if (pFile == NULL) {
    m_status = FXCODEC_STATUS_ERR_PARAMS;
    m_pFile = NULL;
    return m_status;
  }
  m_pFile = pFile;
  m_offSet = 0;
  m_SrcWidth = m_SrcHeight = 0;
  m_SrcComponents = m_SrcBPC = 0;
  m_clipBox = FX_RECT(0, 0, 0, 0);
  m_startX = m_startY = 0;
  m_sizeX = m_sizeY = 0;
  m_SrcPassNumber = 0;
  if (imageType != FXCODEC_IMAGE_UNKNOWN &&
      DetectImageType(imageType, pAttribute)) {
    m_imagType = imageType;
    m_status = FXCODEC_STATUS_FRAME_READY;
    return m_status;
  }
  for (int type = FXCODEC_IMAGE_BMP; type < FXCODEC_IMAGE_MAX; type++) {
    if (DetectImageType((FXCODEC_IMAGE_TYPE)type, pAttribute)) {
      m_imagType = (FXCODEC_IMAGE_TYPE)type;
      m_status = FXCODEC_STATUS_FRAME_READY;
      return m_status;
    }
  }
  m_status = FXCODEC_STATUS_ERR_FORMAT;
  m_pFile = NULL;
  return m_status;
}
void CCodec_ProgressiveDecoder::SetClipBox(FX_RECT* clip) {
  if (m_status != FXCODEC_STATUS_FRAME_READY) {
    return;
  }
  if (clip->IsEmpty()) {
    m_clipBox = FX_RECT(0, 0, 0, 0);
    return;
  }
  if (clip->left < 0) {
    clip->left = 0;
  }
  if (clip->right > m_SrcWidth) {
    clip->right = m_SrcWidth;
  }
  if (clip->top < 0) {
    clip->top = 0;
  }
  if (clip->bottom > m_SrcHeight) {
    clip->bottom = m_SrcHeight;
  }
  if (clip->IsEmpty()) {
    m_clipBox = FX_RECT(0, 0, 0, 0);
    return;
  }
  m_clipBox = *clip;
}
void CCodec_ProgressiveDecoder::GetDownScale(int& down_scale) {
  down_scale = 1;
  int ratio_w = m_clipBox.Width() / m_sizeX;
  int ratio_h = m_clipBox.Height() / m_sizeY;
  int ratio = (ratio_w > ratio_h) ? ratio_h : ratio_w;
  if (ratio >= 8) {
    down_scale = 8;
  } else if (ratio >= 4) {
    down_scale = 4;
  } else if (ratio >= 2) {
    down_scale = 2;
  }
  m_clipBox.left /= down_scale;
  m_clipBox.right /= down_scale;
  m_clipBox.top /= down_scale;
  m_clipBox.bottom /= down_scale;
  if (m_clipBox.right == m_clipBox.left) {
    m_clipBox.right = m_clipBox.left + 1;
  }
  if (m_clipBox.bottom == m_clipBox.top) {
    m_clipBox.bottom = m_clipBox.top + 1;
  }
}
void CCodec_ProgressiveDecoder::GetTransMethod(FXDIB_Format des_format,
                                               FXCodec_Format src_format) {
  switch (des_format) {
    case FXDIB_1bppMask:
    case FXDIB_1bppRgb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 0;
          break;
        default:
          m_TransMethod = -1;
      }
    } break;
    case FXDIB_8bppMask:
    case FXDIB_8bppRgb: {
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
    case FXDIB_Rgb: {
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
    case FXDIB_Rgb32:
    case FXDIB_Argb: {
      switch (src_format) {
        case FXCodec_1bppGray:
          m_TransMethod = 6;
          break;
        case FXCodec_8bppGray:
          m_TransMethod = 7;
          break;
        case FXCodec_1bppRgb:
        case FXCodec_8bppRgb:
          if (des_format == FXDIB_Argb) {
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
void _RGB2BGR(uint8_t* buffer, int width = 1) {
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
void CCodec_ProgressiveDecoder::ReSampleScanline(CFX_DIBitmap* pDeviceBitmap,
                                                 int des_line,
                                                 uint8_t* src_scan,
                                                 FXCodec_Format src_format) {
  int src_left = m_clipBox.left;
  int des_left = m_startX;
  uint8_t* des_scan =
      pDeviceBitmap->GetBuffer() + des_line * pDeviceBitmap->GetPitch();
  int src_bpp = src_format & 0xff;
  int des_bpp = pDeviceBitmap->GetBPP();
  int src_Bpp = src_bpp >> 3;
  int des_Bpp = des_bpp >> 3;
  src_scan += src_left * src_Bpp;
  des_scan += des_left * des_Bpp;
  for (int des_col = 0; des_col < m_sizeX; des_col++) {
    PixelWeight* pPixelWeights = m_WeightHorz.GetPixelWeight(des_col);
    switch (m_TransMethod) {
      case -1:
        return;
      case 0:
        return;
      case 1:
        return;
      case 2: {
        FX_DWORD des_g = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          des_g += pixel_weight * src_scan[j];
        }
        *des_scan++ = (uint8_t)(des_g >> 16);
      } break;
      case 3: {
        int des_r = 0, des_g = 0, des_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          unsigned long argb = m_pSrcPalette[src_scan[j]];
          des_r += pixel_weight * (uint8_t)(argb >> 16);
          des_g += pixel_weight * (uint8_t)(argb >> 8);
          des_b += pixel_weight * (uint8_t)argb;
        }
        *des_scan++ =
            (uint8_t)FXRGB2GRAY((des_r >> 16), (des_g >> 16), (des_b >> 16));
      } break;
      case 4: {
        FX_DWORD des_b = 0, des_g = 0, des_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_Bpp;
          des_b += pixel_weight * (*src_pixel++);
          des_g += pixel_weight * (*src_pixel++);
          des_r += pixel_weight * (*src_pixel);
        }
        *des_scan++ =
            (uint8_t)FXRGB2GRAY((des_r >> 16), (des_g >> 16), (des_b >> 16));
      } break;
      case 5: {
        FX_DWORD des_b = 0, des_g = 0, des_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_Bpp;
          uint8_t src_b = 0, src_g = 0, src_r = 0;
          AdobeCMYK_to_sRGB1(255 - src_pixel[0], 255 - src_pixel[1],
                             255 - src_pixel[2], 255 - src_pixel[3], src_r,
                             src_g, src_b);
          des_b += pixel_weight * src_b;
          des_g += pixel_weight * src_g;
          des_r += pixel_weight * src_r;
        }
        *des_scan++ =
            (uint8_t)FXRGB2GRAY((des_r >> 16), (des_g >> 16), (des_b >> 16));
      } break;
      case 6:
        return;
      case 7: {
        FX_DWORD des_g = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          des_g += pixel_weight * src_scan[j];
        }
        FXSYS_memset(des_scan, (uint8_t)(des_g >> 16), 3);
        des_scan += des_Bpp;
      } break;
      case 8: {
        int des_r = 0, des_g = 0, des_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          unsigned long argb = m_pSrcPalette[src_scan[j]];
          des_r += pixel_weight * (uint8_t)(argb >> 16);
          des_g += pixel_weight * (uint8_t)(argb >> 8);
          des_b += pixel_weight * (uint8_t)argb;
        }
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        des_scan += des_Bpp - 3;
      } break;
      case 12: {
        if (m_pBmpContext) {
          int des_r = 0, des_g = 0, des_b = 0;
          for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
               j++) {
            int pixel_weight =
                pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
            unsigned long argb = m_pSrcPalette[src_scan[j]];
            des_r += pixel_weight * (uint8_t)(argb >> 16);
            des_g += pixel_weight * (uint8_t)(argb >> 8);
            des_b += pixel_weight * (uint8_t)argb;
          }
          *des_scan++ = (uint8_t)((des_b) >> 16);
          *des_scan++ = (uint8_t)((des_g) >> 16);
          *des_scan++ = (uint8_t)((des_r) >> 16);
          *des_scan++ = 0xFF;
        } else {
          int des_a = 0, des_r = 0, des_g = 0, des_b = 0;
          for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
               j++) {
            int pixel_weight =
                pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
            unsigned long argb = m_pSrcPalette[src_scan[j]];
            des_a += pixel_weight * (uint8_t)(argb >> 24);
            des_r += pixel_weight * (uint8_t)(argb >> 16);
            des_g += pixel_weight * (uint8_t)(argb >> 8);
            des_b += pixel_weight * (uint8_t)argb;
          }
          *des_scan++ = (uint8_t)((des_b) >> 16);
          *des_scan++ = (uint8_t)((des_g) >> 16);
          *des_scan++ = (uint8_t)((des_r) >> 16);
          *des_scan++ = (uint8_t)((des_a) >> 16);
        }
      } break;
      case 9: {
        FX_DWORD des_b = 0, des_g = 0, des_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_Bpp;
          des_b += pixel_weight * (*src_pixel++);
          des_g += pixel_weight * (*src_pixel++);
          des_r += pixel_weight * (*src_pixel);
        }
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        des_scan += des_Bpp - 3;
      } break;
      case 10: {
        FX_DWORD des_b = 0, des_g = 0, des_r = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_Bpp;
          uint8_t src_b = 0, src_g = 0, src_r = 0;
          AdobeCMYK_to_sRGB1(255 - src_pixel[0], 255 - src_pixel[1],
                             255 - src_pixel[2], 255 - src_pixel[3], src_r,
                             src_g, src_b);
          des_b += pixel_weight * src_b;
          des_g += pixel_weight * src_g;
          des_r += pixel_weight * src_r;
        }
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        des_scan += des_Bpp - 3;
      } break;
      case 11: {
        FX_DWORD des_alpha = 0, des_r = 0, des_g = 0, des_b = 0;
        for (int j = pPixelWeights->m_SrcStart; j <= pPixelWeights->m_SrcEnd;
             j++) {
          int pixel_weight =
              pPixelWeights->m_Weights[j - pPixelWeights->m_SrcStart];
          const uint8_t* src_pixel = src_scan + j * src_Bpp;
          pixel_weight = pixel_weight * src_pixel[3] / 255;
          des_b += pixel_weight * (*src_pixel++);
          des_g += pixel_weight * (*src_pixel++);
          des_r += pixel_weight * (*src_pixel);
          des_alpha += pixel_weight;
        }
        *des_scan++ = (uint8_t)((des_b) >> 16);
        *des_scan++ = (uint8_t)((des_g) >> 16);
        *des_scan++ = (uint8_t)((des_r) >> 16);
        *des_scan++ = (uint8_t)((des_alpha * 255) >> 16);
      } break;
      default:
        return;
    }
  }
}
void CCodec_ProgressiveDecoder::ResampleVert(CFX_DIBitmap* pDeviceBitmap,
                                             double scale_y,
                                             int des_row) {
  int des_Bpp = pDeviceBitmap->GetBPP() >> 3;
  FX_DWORD des_ScanOffet = m_startX * des_Bpp;
  if (m_bInterpol) {
    int des_top = m_startY;
    int des_row_1 = des_row - int(scale_y);
    if (des_row_1 < des_top) {
      int des_bottom = des_top + m_sizeY;
      if (des_row + (int)scale_y >= des_bottom - 1) {
        uint8_t* scan_src =
            (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
        while (++des_row < des_bottom) {
          uint8_t* scan_des =
              (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
          FX_DWORD size = m_sizeX * des_Bpp;
          FXSYS_memcpy(scan_des, scan_src, size);
        }
      }
      return;
    }
    for (; des_row_1 < des_row; des_row_1++) {
      uint8_t* scan_des =
          (uint8_t*)pDeviceBitmap->GetScanline(des_row_1) + des_ScanOffet;
      PixelWeight* pWeight = m_WeightVert.GetPixelWeight(des_row_1 - des_top);
      const uint8_t* scan_src1 =
          pDeviceBitmap->GetScanline(pWeight->m_SrcStart + des_top) +
          des_ScanOffet;
      const uint8_t* scan_src2 =
          pDeviceBitmap->GetScanline(pWeight->m_SrcEnd + des_top) +
          des_ScanOffet;
      for (int des_col = 0; des_col < m_sizeX; des_col++) {
        switch (pDeviceBitmap->GetFormat()) {
          case FXDIB_Invalid:
          case FXDIB_1bppMask:
          case FXDIB_1bppRgb:
            return;
          case FXDIB_8bppMask:
          case FXDIB_8bppRgb: {
            if (pDeviceBitmap->GetPalette() != NULL) {
              return;
            }
            int des_g = 0;
            des_g += pWeight->m_Weights[0] * (*scan_src1++);
            des_g += pWeight->m_Weights[1] * (*scan_src2++);
            *scan_des++ = (uint8_t)(des_g >> 16);
          } break;
          case FXDIB_Rgb:
          case FXDIB_Rgb32: {
            FX_DWORD des_b = 0, des_g = 0, des_r = 0;
            des_b += pWeight->m_Weights[0] * (*scan_src1++);
            des_g += pWeight->m_Weights[0] * (*scan_src1++);
            des_r += pWeight->m_Weights[0] * (*scan_src1++);
            scan_src1 += des_Bpp - 3;
            des_b += pWeight->m_Weights[1] * (*scan_src2++);
            des_g += pWeight->m_Weights[1] * (*scan_src2++);
            des_r += pWeight->m_Weights[1] * (*scan_src2++);
            scan_src2 += des_Bpp - 3;
            *scan_des++ = (uint8_t)((des_b) >> 16);
            *scan_des++ = (uint8_t)((des_g) >> 16);
            *scan_des++ = (uint8_t)((des_r) >> 16);
            scan_des += des_Bpp - 3;
          } break;
          case FXDIB_Argb: {
            FX_DWORD des_a = 0, des_b = 0, des_g = 0, des_r = 0;
            des_b += pWeight->m_Weights[0] * (*scan_src1++);
            des_g += pWeight->m_Weights[0] * (*scan_src1++);
            des_r += pWeight->m_Weights[0] * (*scan_src1++);
            des_a += pWeight->m_Weights[0] * (*scan_src1++);
            des_b += pWeight->m_Weights[1] * (*scan_src2++);
            des_g += pWeight->m_Weights[1] * (*scan_src2++);
            des_r += pWeight->m_Weights[1] * (*scan_src2++);
            des_a += pWeight->m_Weights[1] * (*scan_src2++);
            *scan_des++ = (uint8_t)((des_b) >> 16);
            *scan_des++ = (uint8_t)((des_g) >> 16);
            *scan_des++ = (uint8_t)((des_r) >> 16);
            *scan_des++ = (uint8_t)((des_a) >> 16);
          } break;
          default:
            return;
        }
      }
    }
    int des_bottom = des_top + m_sizeY;
    if (des_row + (int)scale_y >= des_bottom - 1) {
      uint8_t* scan_src =
          (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
      while (++des_row < des_bottom) {
        uint8_t* scan_des =
            (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
        FX_DWORD size = m_sizeX * des_Bpp;
        FXSYS_memcpy(scan_des, scan_src, size);
      }
    }
    return;
  }
  int multiple = (int)FXSYS_ceil((FX_FLOAT)scale_y - 1);
  if (multiple > 0) {
    uint8_t* scan_src =
        (uint8_t*)pDeviceBitmap->GetScanline(des_row) + des_ScanOffet;
    for (int i = 1; i <= multiple; i++) {
      if (des_row + i >= m_startY + m_sizeY) {
        return;
      }
      uint8_t* scan_des =
          (uint8_t*)pDeviceBitmap->GetScanline(des_row + i) + des_ScanOffet;
      FX_DWORD size = m_sizeX * des_Bpp;
      FXSYS_memcpy(scan_des, scan_src, size);
    }
  }
}
void CCodec_ProgressiveDecoder::Resample(CFX_DIBitmap* pDeviceBitmap,
                                         int32_t src_line,
                                         uint8_t* src_scan,
                                         FXCodec_Format src_format) {
  int src_top = m_clipBox.top;
  int des_top = m_startY;
  int src_hei = m_clipBox.Height();
  int des_hei = m_sizeY;
  if (src_line >= src_top) {
    double scale_y = (double)des_hei / (double)src_hei;
    int src_row = src_line - src_top;
    int des_row = (int)(src_row * scale_y) + des_top;
    if (des_row >= des_top + des_hei) {
      return;
    }
    ReSampleScanline(pDeviceBitmap, des_row, m_pDecodeBuf, src_format);
    if (scale_y > 1.0) {
      ResampleVert(pDeviceBitmap, scale_y, des_row);
    }
  }
}
FXCODEC_STATUS CCodec_ProgressiveDecoder::GetFrames(int32_t& frames,
                                                    IFX_Pause* pPause) {
  if (!(m_status == FXCODEC_STATUS_FRAME_READY ||
        m_status == FXCODEC_STATUS_FRAME_TOBECONTINUE)) {
    return FXCODEC_STATUS_ERROR;
  }
  switch (m_imagType) {
    case FXCODEC_IMAGE_BMP:
    case FXCODEC_IMAGE_JPG:
    case FXCODEC_IMAGE_PNG:
    case FXCODEC_IMAGE_TIF:
      frames = m_FrameNumber = 1;
      return m_status = FXCODEC_STATUS_DECODE_READY;
    case FXCODEC_IMAGE_GIF: {
      ICodec_GifModule* pGifModule = m_pCodecMgr->GetGifModule();
      while (TRUE) {
        int32_t readResult =
            pGifModule->LoadFrameInfo(m_pGifContext, &m_FrameNumber);
        while (readResult == 2) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS_ERR_READ;
          if (!GifReadMoreData(pGifModule, error_status)) {
            return error_status;
          }
          if (pPause && pPause->NeedToPauseNow()) {
            return m_status = FXCODEC_STATUS_FRAME_TOBECONTINUE;
          }
          readResult = pGifModule->LoadFrameInfo(m_pGifContext, &m_FrameNumber);
        }
        if (readResult == 1) {
          frames = m_FrameNumber;
          return m_status = FXCODEC_STATUS_DECODE_READY;
        }
        if (m_pGifContext != NULL) {
          pGifModule->Finish(m_pGifContext);
          m_pGifContext = NULL;
        }
        return m_status = FXCODEC_STATUS_ERROR;
      }
    } break;
    default:;
  }
  return FXCODEC_STATUS_ERROR;
}
FXCODEC_STATUS CCodec_ProgressiveDecoder::StartDecode(CFX_DIBitmap* pDIBitmap,
                                                      int start_x,
                                                      int start_y,
                                                      int size_x,
                                                      int size_y,
                                                      int32_t frames,
                                                      FX_BOOL bInterpol) {
  if (m_status != FXCODEC_STATUS_DECODE_READY) {
    return FXCODEC_STATUS_ERROR;
  }
  if (pDIBitmap == NULL || pDIBitmap->GetBPP() < 8 || frames < 0 ||
      frames >= m_FrameNumber) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  m_pDeviceBitmap = pDIBitmap;
  if (m_clipBox.IsEmpty()) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  if (size_x <= 0 || size_x > 65535 || size_y <= 0 || size_y > 65535) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  FX_RECT device_rc =
      FX_RECT(start_x, start_y, start_x + size_x, start_y + size_y);
  int32_t out_range_x = device_rc.right - pDIBitmap->GetWidth();
  int32_t out_range_y = device_rc.bottom - pDIBitmap->GetHeight();
  device_rc.Intersect(
      FX_RECT(0, 0, pDIBitmap->GetWidth(), pDIBitmap->GetHeight()));
  if (device_rc.IsEmpty()) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  m_startX = device_rc.left;
  m_startY = device_rc.top;
  m_sizeX = device_rc.Width();
  m_sizeY = device_rc.Height();
  m_bInterpol = bInterpol;
  m_FrameCur = 0;
  if (start_x < 0 || out_range_x > 0) {
    FX_FLOAT scaleX = (FX_FLOAT)m_clipBox.Width() / (FX_FLOAT)size_x;
    if (start_x < 0) {
      m_clipBox.left -= (int32_t)FXSYS_ceil((FX_FLOAT)start_x * scaleX);
    }
    if (out_range_x > 0) {
      m_clipBox.right -= (int32_t)FXSYS_floor((FX_FLOAT)out_range_x * scaleX);
    }
  }
  if (start_y < 0 || out_range_y > 0) {
    FX_FLOAT scaleY = (FX_FLOAT)m_clipBox.Height() / (FX_FLOAT)size_y;
    if (start_y < 0) {
      m_clipBox.top -= (int32_t)FXSYS_ceil((FX_FLOAT)start_y * scaleY);
    }
    if (out_range_y > 0) {
      m_clipBox.bottom -= (int32_t)FXSYS_floor((FX_FLOAT)out_range_y * scaleY);
    }
  }
  if (m_clipBox.IsEmpty()) {
    return FXCODEC_STATUS_ERR_PARAMS;
  }
  switch (m_imagType) {
    case FXCODEC_IMAGE_JPG: {
      ICodec_JpegModule* pJpegModule = m_pCodecMgr->GetJpegModule();
      int down_scale = 1;
      GetDownScale(down_scale);
      FX_BOOL bStart = pJpegModule->StartScanline(m_pJpegContext, down_scale);
      while (!bStart) {
        FXCODEC_STATUS error_status = FXCODEC_STATUS_ERROR;
        if (!JpegReadMoreData(pJpegModule, error_status)) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = error_status;
        }
        bStart = pJpegModule->StartScanline(m_pJpegContext, down_scale);
      }
      int scanline_size = (m_SrcWidth + down_scale - 1) / down_scale;
      scanline_size = (scanline_size * m_SrcComponents + 3) / 4 * 4;
      if (m_pDecodeBuf != NULL) {
        FX_Free(m_pDecodeBuf);
        m_pDecodeBuf = NULL;
      }
      m_pDecodeBuf = FX_Alloc(uint8_t, scanline_size);
      if (m_pDecodeBuf == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      FXSYS_memset(m_pDecodeBuf, 0, scanline_size);
      m_WeightHorz.Calc(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                        m_clipBox.Width(), m_bInterpol);
      m_WeightVert.Calc(m_sizeY, m_clipBox.Height());
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
      GetTransMethod(pDIBitmap->GetFormat(), m_SrcFormat);
      return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    } break;
    case FXCODEC_IMAGE_PNG: {
      ICodec_PngModule* pPngModule = m_pCodecMgr->GetPngModule();
      if (pPngModule == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      if (m_pPngContext != NULL) {
        pPngModule->Finish(m_pPngContext);
        m_pPngContext = NULL;
      }
      m_pPngContext = pPngModule->Start((void*)this);
      if (m_pPngContext == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      m_offSet = 0;
      switch (m_pDeviceBitmap->GetFormat()) {
        case FXDIB_8bppMask:
        case FXDIB_8bppRgb:
          m_SrcComponents = 1;
          m_SrcFormat = FXCodec_8bppGray;
          break;
        case FXDIB_Rgb:
          m_SrcComponents = 3;
          m_SrcFormat = FXCodec_Rgb;
          break;
        case FXDIB_Rgb32:
        case FXDIB_Argb:
          m_SrcComponents = 4;
          m_SrcFormat = FXCodec_Argb;
          break;
        default: {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_PARAMS;
        }
      }
      GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
      int scanline_size = (m_SrcWidth * m_SrcComponents + 3) / 4 * 4;
      if (m_pDecodeBuf != NULL) {
        FX_Free(m_pDecodeBuf);
        m_pDecodeBuf = NULL;
      }
      m_pDecodeBuf = FX_Alloc(uint8_t, scanline_size);
      if (m_pDecodeBuf == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      FXSYS_memset(m_pDecodeBuf, 0, scanline_size);
      m_WeightHorzOO.Calc(m_sizeX, m_clipBox.Width(), m_bInterpol);
      m_WeightVert.Calc(m_sizeY, m_clipBox.Height());
      return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    } break;
    case FXCODEC_IMAGE_GIF: {
      ICodec_GifModule* pGifModule = m_pCodecMgr->GetGifModule();
      if (pGifModule == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      m_SrcFormat = FXCodec_8bppRgb;
      GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
      int scanline_size = (m_SrcWidth + 3) / 4 * 4;
      if (m_pDecodeBuf != NULL) {
        FX_Free(m_pDecodeBuf);
        m_pDecodeBuf = NULL;
      }
      m_pDecodeBuf = FX_Alloc(uint8_t, scanline_size);
      if (m_pDecodeBuf == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      FXSYS_memset(m_pDecodeBuf, 0, scanline_size);
      m_WeightHorz.Calc(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                        m_clipBox.Width(), m_bInterpol);
      m_WeightVert.Calc(m_sizeY, m_clipBox.Height());
      m_FrameCur = frames;
      return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    } break;
    case FXCODEC_IMAGE_BMP: {
      ICodec_BmpModule* pBmpModule = m_pCodecMgr->GetBmpModule();
      if (pBmpModule == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      switch (m_SrcComponents) {
        case 1:
          m_SrcFormat = FXCodec_8bppRgb;
          break;
        case 3:
          m_SrcFormat = FXCodec_Rgb;
          break;
        case 4:
          m_SrcFormat = FXCodec_Rgb32;
          break;
      }
      GetTransMethod(m_pDeviceBitmap->GetFormat(), m_SrcFormat);
      m_ScanlineSize = (m_SrcWidth * m_SrcComponents + 3) / 4 * 4;
      if (m_pDecodeBuf != NULL) {
        FX_Free(m_pDecodeBuf);
        m_pDecodeBuf = NULL;
      }
      m_pDecodeBuf = FX_Alloc(uint8_t, m_ScanlineSize);
      if (m_pDecodeBuf == NULL) {
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERR_MEMORY;
      }
      FXSYS_memset(m_pDecodeBuf, 0, m_ScanlineSize);
      m_WeightHorz.Calc(m_sizeX, 0, m_sizeX, m_clipBox.Width(), 0,
                        m_clipBox.Width(), m_bInterpol);
      m_WeightVert.Calc(m_sizeY, m_clipBox.Height());
      return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    } break;
    case FXCODEC_IMAGE_TIF:
      return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
    default:
      break;
  }
  return FXCODEC_STATUS_ERROR;
}
FXCODEC_STATUS CCodec_ProgressiveDecoder::ContinueDecode(IFX_Pause* pPause) {
  if (m_status != FXCODEC_STATUS_DECODE_TOBECONTINUE) {
    return FXCODEC_STATUS_ERROR;
  }
  switch (m_imagType) {
    case FXCODEC_IMAGE_JPG: {
      ICodec_JpegModule* pJpegModule = m_pCodecMgr->GetJpegModule();
      while (TRUE) {
        FX_BOOL readRes =
            pJpegModule->ReadScanline(m_pJpegContext, m_pDecodeBuf);
        while (!readRes) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS_DECODE_FINISH;
          if (!JpegReadMoreData(pJpegModule, error_status)) {
            m_pDeviceBitmap = NULL;
            m_pFile = NULL;
            return m_status = error_status;
          }
          readRes = pJpegModule->ReadScanline(m_pJpegContext, m_pDecodeBuf);
        }
        if (m_SrcFormat == FXCodec_Rgb) {
          int src_Bpp = (m_SrcFormat & 0xff) >> 3;
          _RGB2BGR(m_pDecodeBuf + m_clipBox.left * src_Bpp, m_clipBox.Width());
        }
        if (m_SrcRow >= m_clipBox.bottom) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_DECODE_FINISH;
        }
        Resample(m_pDeviceBitmap, m_SrcRow, m_pDecodeBuf, m_SrcFormat);
        m_SrcRow++;
        if (pPause && pPause->NeedToPauseNow()) {
          return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
      }
    } break;
    case FXCODEC_IMAGE_PNG: {
      ICodec_PngModule* pPngModule = m_pCodecMgr->GetPngModule();
      while (TRUE) {
        FX_DWORD remain_size = (FX_DWORD)m_pFile->GetSize() - m_offSet;
        FX_DWORD input_size =
            remain_size > FXCODEC_BLOCK_SIZE ? FXCODEC_BLOCK_SIZE : remain_size;
        if (input_size == 0) {
          if (m_pPngContext != NULL) {
            pPngModule->Finish(m_pPngContext);
          }
          m_pPngContext = NULL;
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_DECODE_FINISH;
        }
        if (m_pSrcBuf != NULL && input_size > m_SrcSize) {
          FX_Free(m_pSrcBuf);
          m_pSrcBuf = FX_Alloc(uint8_t, input_size);
          if (m_pSrcBuf == NULL) {
            m_pDeviceBitmap = NULL;
            m_pFile = NULL;
            return m_status = FXCODEC_STATUS_ERR_MEMORY;
          }
          FXSYS_memset(m_pSrcBuf, 0, input_size);
          m_SrcSize = input_size;
        }
        FX_BOOL bResult = m_pFile->ReadBlock(m_pSrcBuf, m_offSet, input_size);
        if (!bResult) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_READ;
        }
        m_offSet += input_size;
        bResult =
            pPngModule->Input(m_pPngContext, m_pSrcBuf, input_size, nullptr);
        if (!bResult) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERROR;
        }
        if (pPause && pPause->NeedToPauseNow()) {
          return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
        }
      }
    } break;
    case FXCODEC_IMAGE_GIF: {
      ICodec_GifModule* pGifModule = m_pCodecMgr->GetGifModule();
      while (TRUE) {
        int32_t readRes =
            pGifModule->LoadFrame(m_pGifContext, m_FrameCur, nullptr);
        while (readRes == 2) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS_DECODE_FINISH;
          if (!GifReadMoreData(pGifModule, error_status)) {
            m_pDeviceBitmap = NULL;
            m_pFile = NULL;
            return m_status = error_status;
          }
          if (pPause && pPause->NeedToPauseNow()) {
            return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
          }
          readRes = pGifModule->LoadFrame(m_pGifContext, m_FrameCur, nullptr);
        }
        if (readRes == 1) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_DECODE_FINISH;
        }
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERROR;
      }
    } break;
    case FXCODEC_IMAGE_BMP: {
      ICodec_BmpModule* pBmpModule = m_pCodecMgr->GetBmpModule();
      while (TRUE) {
        int32_t readRes = pBmpModule->LoadImage(m_pBmpContext);
        while (readRes == 2) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS_DECODE_FINISH;
          if (!BmpReadMoreData(pBmpModule, error_status)) {
            m_pDeviceBitmap = NULL;
            m_pFile = NULL;
            return m_status = error_status;
          }
          if (pPause && pPause->NeedToPauseNow()) {
            return m_status = FXCODEC_STATUS_DECODE_TOBECONTINUE;
          }
          readRes = pBmpModule->LoadImage(m_pBmpContext);
        }
        if (readRes == 1) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_DECODE_FINISH;
        }
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_ERROR;
      }
    } break;
    case FXCODEC_IMAGE_TIF: {
      ICodec_TiffModule* pTiffModule = m_pCodecMgr->GetTiffModule();
      FX_BOOL ret = FALSE;
      if (m_pDeviceBitmap->GetBPP() == 32 &&
          m_pDeviceBitmap->GetWidth() == m_SrcWidth && m_SrcWidth == m_sizeX &&
          m_pDeviceBitmap->GetHeight() == m_SrcHeight &&
          m_SrcHeight == m_sizeY && m_startX == 0 && m_startY == 0 &&
          m_clipBox.left == 0 && m_clipBox.top == 0 &&
          m_clipBox.right == m_SrcWidth && m_clipBox.bottom == m_SrcHeight) {
        ret = pTiffModule->Decode(m_pTiffContext, m_pDeviceBitmap);
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        if (!ret) {
          return m_status = FXCODEC_STATUS_ERROR;
        }
        return m_status = FXCODEC_STATUS_DECODE_FINISH;
      } else {
        CFX_DIBitmap* pDIBitmap = new CFX_DIBitmap;
        pDIBitmap->Create(m_SrcWidth, m_SrcHeight, FXDIB_Argb);
        if (pDIBitmap->GetBuffer() == NULL) {
          delete pDIBitmap;
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_MEMORY;
        }
        ret = pTiffModule->Decode(m_pTiffContext, pDIBitmap);
        if (!ret) {
          delete pDIBitmap;
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERROR;
        }
        CFX_DIBitmap* pClipBitmap =
            (m_clipBox.left == 0 && m_clipBox.top == 0 &&
             m_clipBox.right == m_SrcWidth && m_clipBox.bottom == m_SrcHeight)
                ? pDIBitmap
                : pDIBitmap->Clone(&m_clipBox);
        if (pDIBitmap != pClipBitmap) {
          delete pDIBitmap;
        }
        if (pClipBitmap == NULL) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_MEMORY;
        }
        CFX_DIBitmap* pFormatBitmap = NULL;
        switch (m_pDeviceBitmap->GetFormat()) {
          case FXDIB_8bppRgb:
            pFormatBitmap = new CFX_DIBitmap;
            pFormatBitmap->Create(pClipBitmap->GetWidth(),
                                  pClipBitmap->GetHeight(), FXDIB_8bppRgb);
            break;
          case FXDIB_8bppMask:
            pFormatBitmap = new CFX_DIBitmap;
            pFormatBitmap->Create(pClipBitmap->GetWidth(),
                                  pClipBitmap->GetHeight(), FXDIB_8bppMask);
            break;
          case FXDIB_Rgb:
            pFormatBitmap = new CFX_DIBitmap;
            pFormatBitmap->Create(pClipBitmap->GetWidth(),
                                  pClipBitmap->GetHeight(), FXDIB_Rgb);
            break;
          case FXDIB_Rgb32:
            pFormatBitmap = new CFX_DIBitmap;
            pFormatBitmap->Create(pClipBitmap->GetWidth(),
                                  pClipBitmap->GetHeight(), FXDIB_Rgb32);
            break;
          case FXDIB_Argb:
            pFormatBitmap = pClipBitmap;
            break;
          default:;
        }
        switch (m_pDeviceBitmap->GetFormat()) {
          case FXDIB_8bppRgb:
          case FXDIB_8bppMask: {
            for (int32_t row = 0; row < pClipBitmap->GetHeight(); row++) {
              uint8_t* src_line = (uint8_t*)pClipBitmap->GetScanline(row);
              uint8_t* des_line = (uint8_t*)pFormatBitmap->GetScanline(row);
              for (int32_t col = 0; col < pClipBitmap->GetWidth(); col++) {
                uint8_t _a = 255 - src_line[3];
                uint8_t b = (src_line[0] * src_line[3] + 0xFF * _a) / 255;
                uint8_t g = (src_line[1] * src_line[3] + 0xFF * _a) / 255;
                uint8_t r = (src_line[2] * src_line[3] + 0xFF * _a) / 255;
                *des_line++ = FXRGB2GRAY(r, g, b);
                src_line += 4;
              }
            }
          } break;
          case FXDIB_Rgb:
          case FXDIB_Rgb32: {
            int32_t desBpp =
                (m_pDeviceBitmap->GetFormat() == FXDIB_Rgb) ? 3 : 4;
            for (int32_t row = 0; row < pClipBitmap->GetHeight(); row++) {
              uint8_t* src_line = (uint8_t*)pClipBitmap->GetScanline(row);
              uint8_t* des_line = (uint8_t*)pFormatBitmap->GetScanline(row);
              for (int32_t col = 0; col < pClipBitmap->GetWidth(); col++) {
                uint8_t _a = 255 - src_line[3];
                uint8_t b = (src_line[0] * src_line[3] + 0xFF * _a) / 255;
                uint8_t g = (src_line[1] * src_line[3] + 0xFF * _a) / 255;
                uint8_t r = (src_line[2] * src_line[3] + 0xFF * _a) / 255;
                *des_line++ = b;
                *des_line++ = g;
                *des_line++ = r;
                des_line += desBpp - 3;
                src_line += 4;
              }
            }
          } break;
          default:;
        }
        if (pClipBitmap != pFormatBitmap) {
          delete pClipBitmap;
        }
        if (pFormatBitmap == NULL) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_MEMORY;
        }
        CFX_DIBitmap* pStrechBitmap = pFormatBitmap->StretchTo(
            m_sizeX, m_sizeY, m_bInterpol ? FXDIB_INTERPOL : FXDIB_DOWNSAMPLE);
        delete pFormatBitmap;
        pFormatBitmap = NULL;
        if (pStrechBitmap == NULL) {
          m_pDeviceBitmap = NULL;
          m_pFile = NULL;
          return m_status = FXCODEC_STATUS_ERR_MEMORY;
        }
        m_pDeviceBitmap->TransferBitmap(m_startX, m_startY, m_sizeX, m_sizeY,
                                        pStrechBitmap, 0, 0);
        delete pStrechBitmap;
        pStrechBitmap = NULL;
        m_pDeviceBitmap = NULL;
        m_pFile = NULL;
        return m_status = FXCODEC_STATUS_DECODE_FINISH;
      }
    } break;
    default:
      break;
  }
  return FXCODEC_STATUS_ERROR;
}
ICodec_ProgressiveDecoder* CCodec_ModuleMgr::CreateProgressiveDecoder() {
  return new CCodec_ProgressiveDecoder(this);
}
