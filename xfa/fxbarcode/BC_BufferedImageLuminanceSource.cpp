// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2009 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>

#include "core/fxcodec/codec/include/ccodec_progressivedecoder.h"
#include "core/fxcodec/include/fx_codec.h"
#include "xfa/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/fxbarcode/BC_LuminanceSource.h"
#include "xfa/fxbarcode/utils.h"

class CBC_Pause : public IFX_Pause {
 public:
  virtual FX_BOOL NeedToPauseNow() { return TRUE; }
};

static CFX_DIBitmap* CreateDIBSource(IFX_FileRead* fileread) {
  std::unique_ptr<CCodec_ModuleMgr> pCodecMgr(new CCodec_ModuleMgr());
  std::unique_ptr<CCodec_ProgressiveDecoder> pImageCodec(
      pCodecMgr->CreateProgressiveDecoder());
  FXCODEC_STATUS status = FXCODEC_STATUS_DECODE_FINISH;
  status = pImageCodec->LoadImageInfo(fileread, FXCODEC_IMAGE_UNKNOWN, nullptr);
  if (status != FXCODEC_STATUS_FRAME_READY)
    return nullptr;

  std::unique_ptr<CFX_DIBitmap> bitmap(new CFX_DIBitmap);
  bitmap->Create(pImageCodec->GetWidth(), pImageCodec->GetHeight(), FXDIB_Argb);
  bitmap->Clear(FXARGB_MAKE(0xFF, 0xFF, 0xFF, 0xFF));
  CBC_Pause pause;
  int32_t frames;
  status = pImageCodec->GetFrames(frames, &pause);
  while (status == FXCODEC_STATUS_FRAME_TOBECONTINUE)
    status = pImageCodec->GetFrames(frames, &pause);

  if (status != FXCODEC_STATUS_DECODE_READY)
    return nullptr;

  status = pImageCodec->StartDecode(bitmap.get(), 0, 0, bitmap->GetWidth(),
                                    bitmap->GetHeight(), 0, FALSE);
  if (status != FXCODEC_STATUS_DECODE_TOBECONTINUE)
    return nullptr;

  while (status == FXCODEC_STATUS_DECODE_TOBECONTINUE)
    status = pImageCodec->ContinueDecode(&pause);

  return status == FXCODEC_STATUS_DECODE_FINISH ? bitmap.release() : nullptr;
}

CBC_BufferedImageLuminanceSource::CBC_BufferedImageLuminanceSource(
    const CFX_WideString& filename)
    : CBC_LuminanceSource(0, 0), m_filename(filename) {
  m_height = 0;
  m_width = 0;
  m_bytesPerLine = 0;
  m_top = 0;
  m_left = 0;
}

void CBC_BufferedImageLuminanceSource::Init(int32_t& e) {
  IFX_FileRead* fileread = FX_CreateFileRead(m_filename.c_str());
  m_pBitmap = CreateDIBSource(fileread);
  if (!m_pBitmap) {
    e = BCExceptionLoadFile;
    return;
  }
  m_pBitmap->ConvertFormat(FXDIB_Argb);
  m_height = m_pBitmap->GetHeight();
  m_width = m_pBitmap->GetWidth();
  m_rgbData.SetSize(m_height * m_width);
  m_bytesPerLine = m_width * 4;
  m_top = 0;
  m_left = 0;
}

CBC_BufferedImageLuminanceSource::CBC_BufferedImageLuminanceSource(
    CFX_DIBitmap* pBitmap)
    : CBC_LuminanceSource(0, 0) {
  m_pBitmap = pBitmap->Clone();
  m_pBitmap->ConvertFormat(FXDIB_Argb);
  m_height = m_pBitmap->GetHeight();
  m_width = m_pBitmap->GetWidth();
  m_rgbData.SetSize(m_height * m_width);
  m_bytesPerLine = m_width * 4;
  m_top = 0;
  m_left = 0;
}

CBC_BufferedImageLuminanceSource::~CBC_BufferedImageLuminanceSource() {
  delete m_pBitmap;
}

CFX_ByteArray* CBC_BufferedImageLuminanceSource::GetRow(int32_t y,
                                                        CFX_ByteArray& row,
                                                        int32_t& e) {
  if (y < 0 || y >= m_height) {
    e = BCExceptionRequestedRowIsOutSizeTheImage;
    return nullptr;
  }
  int32_t width = m_width;
  if (row.GetSize() == 0 || row.GetSize() < width) {
    row.SetSize(width);
  }
  if (m_rgbData.GetSize() == 0 || m_rgbData.GetSize() < width) {
    m_rgbData.SetSize(width);
  }
  int32_t* rowLine = (int32_t*)m_pBitmap->GetScanline(y);
  int32_t x;
  for (x = 0; x < width; x++) {
    int32_t pixel = rowLine[x];
    int32_t luminance = (306 * ((pixel >> 16) & 0xFF) +
                         601 * ((pixel >> 8) & 0xFF) + 117 * (pixel & 0xFF)) >>
                        10;
    row[x] = (uint8_t)luminance;
  }
  return &row;
}

CFX_ByteArray* CBC_BufferedImageLuminanceSource::GetMatrix() {
  CFX_ByteArray* matrix = new CFX_ByteArray();
  matrix->SetSize(m_bytesPerLine * m_height);
  int32_t* rgb = (int32_t*)m_pBitmap->GetBuffer();
  int32_t y;
  for (y = 0; y < m_height; y++) {
    int32_t offset = y * m_width;
    int32_t x;
    for (x = 0; x < m_width; x++) {
      int32_t pixel = rgb[offset + x];
      int32_t luminance =
          (306 * ((pixel >> 16) & 0xFF) + 601 * ((pixel >> 8) & 0xFF) +
           117 * (pixel & 0xFF)) >>
          10;
      (*matrix)[offset + x] = (uint8_t)luminance;
    }
  }
  return matrix;
}
