// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cfx_windowsdib.h"

#include <windows.h>

#include "core/fxcrt/fx_system.h"

// static
ByteString CFX_WindowsDIB::GetBitmapInfo(
    const RetainPtr<CFX_DIBitmap>& pBitmap) {
  int len = sizeof(BITMAPINFOHEADER);
  if (pBitmap->GetBPP() == 1 || pBitmap->GetBPP() == 8)
    len += sizeof(DWORD) * (int)(1 << pBitmap->GetBPP());

  ByteString result;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<char> cspan = result.GetBuffer(len);
    BITMAPINFOHEADER* pbmih = reinterpret_cast<BITMAPINFOHEADER*>(cspan.data());
    memset(pbmih, 0, sizeof(BITMAPINFOHEADER));
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
    pbmih->biBitCount = pBitmap->GetBPP();
    pbmih->biCompression = BI_RGB;
    pbmih->biHeight = -(int)pBitmap->GetHeight();
    pbmih->biPlanes = 1;
    pbmih->biWidth = pBitmap->GetWidth();
    if (pBitmap->GetBPP() == 8) {
      uint32_t* pPalette = (uint32_t*)(pbmih + 1);
      if (pBitmap->HasPalette()) {
        pdfium::span<const uint32_t> palette = pBitmap->GetPaletteSpan();
        for (int i = 0; i < 256; i++) {
          pPalette[i] = palette[i];
        }
      } else {
        for (int i = 0; i < 256; i++) {
          pPalette[i] = i * 0x010101;
        }
      }
    }
    if (pBitmap->GetBPP() == 1) {
      uint32_t* pPalette = (uint32_t*)(pbmih + 1);
      if (pBitmap->HasPalette()) {
        pPalette[0] = pBitmap->GetPaletteSpan()[0];
        pPalette[1] = pBitmap->GetPaletteSpan()[1];
      } else {
        pPalette[0] = 0;
        pPalette[1] = 0xffffff;
      }
    }
  }
  result.ReleaseBuffer(len);
  return result;
}
