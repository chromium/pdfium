// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fxge/fx_dib.h"
#include "codec_int.h"

extern "C" {
#include "third_party/libtiff/tiffiop.h"
}

void* IccLib_CreateTransform_sRGB(const unsigned char* pProfileData,
                                  unsigned int dwProfileSize,
                                  int nComponents,
                                  int intent,
                                  FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT);
void IccLib_TranslateImage(void* pTransform,
                           unsigned char* pDest,
                           const unsigned char* pSrc,
                           int pixels);
void IccLib_DestroyTransform(void* pTransform);
class CCodec_TiffContext {
 public:
  CCodec_TiffContext();
  ~CCodec_TiffContext();

  FX_BOOL InitDecoder(IFX_FileRead* file_ptr);
  void GetFrames(int32_t& frames);
  FX_BOOL LoadFrameInfo(int32_t frame,
                        FX_DWORD& width,
                        FX_DWORD& height,
                        FX_DWORD& comps,
                        FX_DWORD& bpc,
                        CFX_DIBAttribute* pAttribute);
  FX_BOOL Decode(CFX_DIBitmap* pDIBitmap);

  union {
    IFX_FileRead* in;
    IFX_FileStream* out;
  } io;

  FX_DWORD offset;

  TIFF* tif_ctx;
  void* icc_ctx;
  int32_t frame_num;
  int32_t frame_cur;
  FX_BOOL isDecoder;

 private:
  FX_BOOL isSupport(CFX_DIBitmap* pDIBitmap);
  void SetPalette(CFX_DIBitmap* pDIBitmap, uint16_t bps);
  FX_BOOL Decode1bppRGB(CFX_DIBitmap* pDIBitmap,
                        int32_t height,
                        int32_t width,
                        uint16_t bps,
                        uint16_t spp);
  FX_BOOL Decode8bppRGB(CFX_DIBitmap* pDIBitmap,
                        int32_t height,
                        int32_t width,
                        uint16_t bps,
                        uint16_t spp);
  FX_BOOL Decode24bppRGB(CFX_DIBitmap* pDIBitmap,
                         int32_t height,
                         int32_t width,
                         uint16_t bps,
                         uint16_t spp);
};
CCodec_TiffContext::CCodec_TiffContext() {
  offset = 0;
  frame_num = 0;
  frame_cur = 0;
  io.in = NULL;
  tif_ctx = NULL;
  icc_ctx = NULL;
  isDecoder = TRUE;
}
CCodec_TiffContext::~CCodec_TiffContext() {
  if (icc_ctx) {
    IccLib_DestroyTransform(icc_ctx);
    icc_ctx = NULL;
  }
  if (tif_ctx) {
    TIFFClose(tif_ctx);
  }
}
static tsize_t _tiff_read(thandle_t context, tdata_t buf, tsize_t length) {
  CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
  FX_BOOL ret = FALSE;
  if (pTiffContext->isDecoder) {
    ret = pTiffContext->io.in->ReadBlock(buf, pTiffContext->offset, length);
  } else {
    ret = pTiffContext->io.out->ReadBlock(buf, pTiffContext->offset, length);
  }
  if (!ret) {
    return 0;
  }
  pTiffContext->offset += (FX_DWORD)length;
  return length;
}
static tsize_t _tiff_write(thandle_t context, tdata_t buf, tsize_t length) {
  CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
  ASSERT(!pTiffContext->isDecoder);
  if (!pTiffContext->io.out->WriteBlock(buf, pTiffContext->offset, length)) {
    return 0;
  }
  pTiffContext->offset += (FX_DWORD)length;
  return length;
}
static toff_t _tiff_seek(thandle_t context, toff_t offset, int whence) {
  CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
  switch (whence) {
    case 0:
      pTiffContext->offset = (FX_DWORD)offset;
      break;
    case 1:
      pTiffContext->offset += (FX_DWORD)offset;
      break;
    case 2:
      if (pTiffContext->isDecoder) {
        if (pTiffContext->io.in->GetSize() < (FX_FILESIZE)offset) {
          return -1;
        }
        pTiffContext->offset =
            (FX_DWORD)(pTiffContext->io.in->GetSize() - offset);
      } else {
        if (pTiffContext->io.out->GetSize() < (FX_FILESIZE)offset) {
          return -1;
        }
        pTiffContext->offset =
            (FX_DWORD)(pTiffContext->io.out->GetSize() - offset);
      }
      break;
    default:
      return -1;
  }
  ASSERT(pTiffContext->isDecoder ? (pTiffContext->offset <=
                                    (FX_DWORD)pTiffContext->io.in->GetSize())
                                 : TRUE);
  return pTiffContext->offset;
}
static int _tiff_close(thandle_t context) {
  return 0;
}
static toff_t _tiff_get_size(thandle_t context) {
  CCodec_TiffContext* pTiffContext = (CCodec_TiffContext*)context;
  return pTiffContext->isDecoder ? (toff_t)pTiffContext->io.in->GetSize()
                                 : (toff_t)pTiffContext->io.out->GetSize();
}
static int _tiff_map(thandle_t context, tdata_t*, toff_t*) {
  return 0;
}
static void _tiff_unmap(thandle_t context, tdata_t, toff_t) {}
TIFF* _tiff_open(void* context, const char* mode) {
  TIFF* tif = TIFFClientOpen("Tiff Image", mode, (thandle_t)context, _tiff_read,
                             _tiff_write, _tiff_seek, _tiff_close,
                             _tiff_get_size, _tiff_map, _tiff_unmap);
  if (tif) {
    tif->tif_fd = (int)(intptr_t)context;
  }
  return tif;
}
void* _TIFFmalloc(tmsize_t size) {
  return FXMEM_DefaultAlloc(size, 0);
}
void _TIFFfree(void* ptr) {
  FXMEM_DefaultFree(ptr, 0);
}
void* _TIFFrealloc(void* ptr, tmsize_t size) {
  return FXMEM_DefaultRealloc(ptr, size, 0);
}
void _TIFFmemset(void* ptr, int val, tmsize_t size) {
  FXSYS_memset(ptr, val, (size_t)size);
}
void _TIFFmemcpy(void* des, const void* src, tmsize_t size) {
  FXSYS_memcpy(des, src, (size_t)size);
}
int _TIFFmemcmp(const void* ptr1, const void* ptr2, tmsize_t size) {
  return FXSYS_memcmp(ptr1, ptr2, (size_t)size);
}

TIFFErrorHandler _TIFFwarningHandler = nullptr;
TIFFErrorHandler _TIFFerrorHandler = nullptr;

int TIFFCmyk2Rgb(thandle_t context,
                 uint8 c,
                 uint8 m,
                 uint8 y,
                 uint8 k,
                 uint8* r,
                 uint8* g,
                 uint8* b) {
  if (context == NULL) {
    return 0;
  }
  CCodec_TiffContext* p = (CCodec_TiffContext*)context;
  if (p->icc_ctx) {
    unsigned char cmyk[4], bgr[3];
    cmyk[0] = c, cmyk[1] = m, cmyk[2] = y, cmyk[3] = k;
    IccLib_TranslateImage(p->icc_ctx, bgr, cmyk, 1);
    *r = bgr[2], *g = bgr[1], *b = bgr[0];
  } else {
    AdobeCMYK_to_sRGB1(c, m, y, k, *r, *g, *b);
  }
  return 1;
}
FX_BOOL CCodec_TiffContext::InitDecoder(IFX_FileRead* file_ptr) {
  io.in = file_ptr;
  tif_ctx = _tiff_open(this, "r");
  if (tif_ctx == NULL) {
    return FALSE;
  }
  return TRUE;
}
void CCodec_TiffContext::GetFrames(int32_t& frames) {
  frames = frame_num = TIFFNumberOfDirectories(tif_ctx);
}
#define TIFF_EXIF_GETINFO(key, T, tag)      \
  {                                         \
    T val = (T)0;                           \
    TIFFGetField(tif_ctx, tag, &val);       \
    if (val) {                              \
      (key) = FX_Alloc(uint8_t, sizeof(T)); \
      if ((key)) {                          \
        T* ptr = (T*)(key);                 \
        *ptr = val;                         \
        pExif->m_TagVal.SetAt(tag, (key));  \
      }                                     \
    }                                       \
  }                                         \
  (key) = NULL;
#define TIFF_EXIF_GETSTRINGINFO(key, tag)    \
  {                                          \
    FX_DWORD size = 0;                       \
    uint8_t* buf = NULL;                     \
    TIFFGetField(tif_ctx, tag, &size, &buf); \
    if (size && buf) {                       \
      (key) = FX_Alloc(uint8_t, size);       \
      if ((key)) {                           \
        FXSYS_memcpy((key), buf, size);      \
        pExif->m_TagVal.SetAt(tag, (key));   \
      }                                      \
    }                                        \
  }                                          \
  (key) = NULL;

namespace {

template <class T>
FX_BOOL Tiff_Exif_GetInfo(TIFF* tif_ctx, ttag_t tag, CFX_DIBAttribute* pAttr) {
  T val = 0;
  TIFFGetField(tif_ctx, tag, &val);
  if (!val)
    return FALSE;
  T* ptr = FX_Alloc(T, 1);
  *ptr = val;
  pAttr->m_Exif[tag] = (void*)ptr;
  return TRUE;
}
void Tiff_Exif_GetStringInfo(TIFF* tif_ctx,
                             ttag_t tag,
                             CFX_DIBAttribute* pAttr) {
  FX_CHAR* buf = nullptr;
  TIFFGetField(tif_ctx, tag, &buf);
  if (!buf)
    return;
  FX_STRSIZE size = FXSYS_strlen(buf);
  uint8_t* ptr = FX_Alloc(uint8_t, size + 1);
  FXSYS_memcpy(ptr, buf, size);
  ptr[size] = 0;
  pAttr->m_Exif[tag] = ptr;
}

}  // namespace

FX_BOOL CCodec_TiffContext::LoadFrameInfo(int32_t frame,
                                          FX_DWORD& width,
                                          FX_DWORD& height,
                                          FX_DWORD& comps,
                                          FX_DWORD& bpc,
                                          CFX_DIBAttribute* pAttribute) {
  if (!TIFFSetDirectory(tif_ctx, (uint16)frame)) {
    return FALSE;
  }
  FX_WORD tif_cs;
  FX_DWORD tif_icc_size = 0;
  uint8_t* tif_icc_buf = NULL;
  FX_WORD tif_bpc = 0;
  FX_WORD tif_cps;
  FX_DWORD tif_rps;
  width = height = comps = 0;
  TIFFGetField(tif_ctx, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif_ctx, TIFFTAG_IMAGELENGTH, &height);
  TIFFGetField(tif_ctx, TIFFTAG_SAMPLESPERPIXEL, &comps);
  TIFFGetField(tif_ctx, TIFFTAG_BITSPERSAMPLE, &tif_bpc);
  TIFFGetField(tif_ctx, TIFFTAG_PHOTOMETRIC, &tif_cs);
  TIFFGetField(tif_ctx, TIFFTAG_COMPRESSION, &tif_cps);
  TIFFGetField(tif_ctx, TIFFTAG_ROWSPERSTRIP, &tif_rps);
  TIFFGetField(tif_ctx, TIFFTAG_ICCPROFILE, &tif_icc_size, &tif_icc_buf);
  if (pAttribute) {
    pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_INCH;
    if (TIFFGetField(tif_ctx, TIFFTAG_RESOLUTIONUNIT,
                     &pAttribute->m_wDPIUnit)) {
      pAttribute->m_wDPIUnit -= 1;
    }
    Tiff_Exif_GetInfo<FX_WORD>(tif_ctx, TIFFTAG_ORIENTATION, pAttribute);
    if (Tiff_Exif_GetInfo<FX_FLOAT>(tif_ctx, TIFFTAG_XRESOLUTION, pAttribute)) {
      void* val = pAttribute->m_Exif[TIFFTAG_XRESOLUTION];
      FX_FLOAT fDpi = val ? *reinterpret_cast<FX_FLOAT*>(val) : 0;
      pAttribute->m_nXDPI = (int32_t)(fDpi + 0.5f);
    }
    if (Tiff_Exif_GetInfo<FX_FLOAT>(tif_ctx, TIFFTAG_YRESOLUTION, pAttribute)) {
      void* val = pAttribute->m_Exif[TIFFTAG_YRESOLUTION];
      FX_FLOAT fDpi = val ? *reinterpret_cast<FX_FLOAT*>(val) : 0;
      pAttribute->m_nYDPI = (int32_t)(fDpi + 0.5f);
    }
    Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_IMAGEDESCRIPTION, pAttribute);
    Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_MAKE, pAttribute);
    Tiff_Exif_GetStringInfo(tif_ctx, TIFFTAG_MODEL, pAttribute);
  }
  bpc = tif_bpc;
  if (tif_rps > height) {
    TIFFSetField(tif_ctx, TIFFTAG_ROWSPERSTRIP, tif_rps = height);
  }
  return TRUE;
}
void _TiffBGRA2RGBA(uint8_t* pBuf, int32_t pixel, int32_t spp) {
  for (int32_t n = 0; n < pixel; n++) {
    uint8_t tmp = pBuf[0];
    pBuf[0] = pBuf[2];
    pBuf[2] = tmp;
    pBuf += spp;
  }
}
FX_BOOL CCodec_TiffContext::isSupport(CFX_DIBitmap* pDIBitmap) {
  if (TIFFIsTiled(tif_ctx)) {
    return FALSE;
  }
  uint16_t photometric;
  if (!TIFFGetField(tif_ctx, TIFFTAG_PHOTOMETRIC, &photometric)) {
    return FALSE;
  }
  switch (pDIBitmap->GetBPP()) {
    case 1:
    case 8:
      if (photometric != PHOTOMETRIC_PALETTE) {
        return FALSE;
      }
      break;
    case 24:
      if (photometric != PHOTOMETRIC_RGB) {
        return FALSE;
      }
      break;
    default:
      return FALSE;
  }
  uint16_t planarconfig;
  if (!TIFFGetFieldDefaulted(tif_ctx, TIFFTAG_PLANARCONFIG, &planarconfig)) {
    return FALSE;
  }
  if (planarconfig == PLANARCONFIG_SEPARATE) {
    return FALSE;
  }
  return TRUE;
}
void CCodec_TiffContext::SetPalette(CFX_DIBitmap* pDIBitmap, uint16_t bps) {
  uint16_t *red_orig, *green_orig, *blue_orig;
  TIFFGetField(tif_ctx, TIFFTAG_COLORMAP, &red_orig, &green_orig, &blue_orig);
  for (int32_t i = (1L << bps) - 1; i >= 0; i--) {
#define CVT(x) ((uint16_t)((x) >> 8))
    red_orig[i] = CVT(red_orig[i]);
    green_orig[i] = CVT(green_orig[i]);
    blue_orig[i] = CVT(blue_orig[i]);
#undef CVT
  }
  int32_t len = 1 << bps;
  for (int32_t index = 0; index < len; index++) {
    FX_DWORD r = red_orig[index] & 0xFF;
    FX_DWORD g = green_orig[index] & 0xFF;
    FX_DWORD b = blue_orig[index] & 0xFF;
    FX_DWORD color = (uint32_t)b | ((uint32_t)g << 8) | ((uint32_t)r << 16) |
                     (((uint32)0xffL) << 24);
    pDIBitmap->SetPaletteEntry(index, color);
  }
}
FX_BOOL CCodec_TiffContext::Decode1bppRGB(CFX_DIBitmap* pDIBitmap,
                                          int32_t height,
                                          int32_t width,
                                          uint16_t bps,
                                          uint16_t spp) {
  if (pDIBitmap->GetBPP() != 1 || spp != 1 || bps != 1 ||
      !isSupport(pDIBitmap)) {
    return FALSE;
  }
  SetPalette(pDIBitmap, bps);
  int32_t size = (int32_t)TIFFScanlineSize(tif_ctx);
  uint8_t* buf = (uint8_t*)_TIFFmalloc(size);
  if (buf == NULL) {
    TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
    return FALSE;
  }
  uint8_t* bitMapbuffer = (uint8_t*)pDIBitmap->GetBuffer();
  FX_DWORD pitch = pDIBitmap->GetPitch();
  for (int32_t row = 0; row < height; row++) {
    TIFFReadScanline(tif_ctx, buf, row, 0);
    for (int32_t j = 0; j < size; j++) {
      bitMapbuffer[row * pitch + j] = buf[j];
    }
  }
  _TIFFfree(buf);
  return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode8bppRGB(CFX_DIBitmap* pDIBitmap,
                                          int32_t height,
                                          int32_t width,
                                          uint16_t bps,
                                          uint16_t spp) {
  if (pDIBitmap->GetBPP() != 8 || spp != 1 || (bps != 4 && bps != 8) ||
      !isSupport(pDIBitmap)) {
    return FALSE;
  }
  SetPalette(pDIBitmap, bps);
  int32_t size = (int32_t)TIFFScanlineSize(tif_ctx);
  uint8_t* buf = (uint8_t*)_TIFFmalloc(size);
  if (buf == NULL) {
    TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
    return FALSE;
  }
  uint8_t* bitMapbuffer = (uint8_t*)pDIBitmap->GetBuffer();
  FX_DWORD pitch = pDIBitmap->GetPitch();
  for (int32_t row = 0; row < height; row++) {
    TIFFReadScanline(tif_ctx, buf, row, 0);
    for (int32_t j = 0; j < size; j++) {
      switch (bps) {
        case 4:
          bitMapbuffer[row * pitch + 2 * j + 0] = (buf[j] & 0xF0) >> 4;
          bitMapbuffer[row * pitch + 2 * j + 1] = (buf[j] & 0x0F) >> 0;
          break;
        case 8:
          bitMapbuffer[row * pitch + j] = buf[j];
          break;
      }
    }
  }
  _TIFFfree(buf);
  return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode24bppRGB(CFX_DIBitmap* pDIBitmap,
                                           int32_t height,
                                           int32_t width,
                                           uint16_t bps,
                                           uint16_t spp) {
  if (pDIBitmap->GetBPP() != 24 || !isSupport(pDIBitmap)) {
    return FALSE;
  }
  int32_t size = (int32_t)TIFFScanlineSize(tif_ctx);
  uint8_t* buf = (uint8_t*)_TIFFmalloc(size);
  if (buf == NULL) {
    TIFFError(TIFFFileName(tif_ctx), "No space for scanline buffer");
    return FALSE;
  }
  uint8_t* bitMapbuffer = (uint8_t*)pDIBitmap->GetBuffer();
  FX_DWORD pitch = pDIBitmap->GetPitch();
  for (int32_t row = 0; row < height; row++) {
    TIFFReadScanline(tif_ctx, buf, row, 0);
    for (int32_t j = 0; j < size - 2; j += 3) {
      bitMapbuffer[row * pitch + j + 0] = buf[j + 2];
      bitMapbuffer[row * pitch + j + 1] = buf[j + 1];
      bitMapbuffer[row * pitch + j + 2] = buf[j + 0];
    }
  }
  _TIFFfree(buf);
  return TRUE;
}
FX_BOOL CCodec_TiffContext::Decode(CFX_DIBitmap* pDIBitmap) {
  FX_DWORD img_wid = pDIBitmap->GetWidth();
  FX_DWORD img_hei = pDIBitmap->GetHeight();
  FX_DWORD width = 0;
  FX_DWORD height = 0;
  TIFFGetField(tif_ctx, TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(tif_ctx, TIFFTAG_IMAGELENGTH, &height);
  if (img_wid != width || img_hei != height) {
    return FALSE;
  }
  if (pDIBitmap->GetBPP() == 32) {
    FX_WORD rotation = ORIENTATION_TOPLEFT;
    TIFFGetField(tif_ctx, TIFFTAG_ORIENTATION, &rotation);
    if (TIFFReadRGBAImageOriented(tif_ctx, img_wid, img_hei,
                                  (uint32*)pDIBitmap->GetBuffer(), rotation,
                                  1)) {
      for (FX_DWORD row = 0; row < img_hei; row++) {
        uint8_t* row_buf = (uint8_t*)pDIBitmap->GetScanline(row);
        _TiffBGRA2RGBA(row_buf, img_wid, 4);
      }
      return TRUE;
    }
  }
  uint16_t spp, bps;
  TIFFGetField(tif_ctx, TIFFTAG_SAMPLESPERPIXEL, &spp);
  TIFFGetField(tif_ctx, TIFFTAG_BITSPERSAMPLE, &bps);
  FX_DWORD bpp = bps * spp;
  if (bpp == 1) {
    return Decode1bppRGB(pDIBitmap, height, width, bps, spp);
  } else if (bpp <= 8) {
    return Decode8bppRGB(pDIBitmap, height, width, bps, spp);
  } else if (bpp <= 24) {
    return Decode24bppRGB(pDIBitmap, height, width, bps, spp);
  }
  return FALSE;
}
void* CCodec_TiffModule::CreateDecoder(IFX_FileRead* file_ptr) {
  CCodec_TiffContext* pDecoder = new CCodec_TiffContext;
  if (!pDecoder->InitDecoder(file_ptr)) {
    delete pDecoder;
    return NULL;
  }
  return pDecoder;
}
void CCodec_TiffModule::GetFrames(void* ctx, int32_t& frames) {
  CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
  pDecoder->GetFrames(frames);
}
FX_BOOL CCodec_TiffModule::LoadFrameInfo(void* ctx,
                                         int32_t frame,
                                         FX_DWORD& width,
                                         FX_DWORD& height,
                                         FX_DWORD& comps,
                                         FX_DWORD& bpc,
                                         CFX_DIBAttribute* pAttribute) {
  CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
  return pDecoder->LoadFrameInfo(frame, width, height, comps, bpc, pAttribute);
}
FX_BOOL CCodec_TiffModule::Decode(void* ctx, class CFX_DIBitmap* pDIBitmap) {
  CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
  return pDecoder->Decode(pDIBitmap);
}
void CCodec_TiffModule::DestroyDecoder(void* ctx) {
  CCodec_TiffContext* pDecoder = (CCodec_TiffContext*)ctx;
  delete pDecoder;
}
