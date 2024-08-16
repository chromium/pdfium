// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/tiff/tiff_decoder.h"

#include <memory>
#include <utility>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

extern "C" {
#if defined(USE_SYSTEM_LIBTIFF)
#include <tiffio.h>
#else
#include "third_party/libtiff/tiffio.h"
#endif
}  // extern C

namespace {

// For use with std::unique_ptr<TIFF>.
struct TiffDeleter {
  inline void operator()(TIFF* context) { TIFFClose(context); }
};

}  // namespace

class CTiffContext final : public ProgressiveDecoderIface::Context {
 public:
  CTiffContext() = default;
  ~CTiffContext() override = default;

  bool InitDecoder(const RetainPtr<IFX_SeekableReadStream>& file_ptr);
  bool LoadFrameInfo(int32_t frame,
                     int32_t* width,
                     int32_t* height,
                     int32_t* comps,
                     int32_t* bpc,
                     CFX_DIBAttribute* pAttribute);
  bool Decode(RetainPtr<CFX_DIBitmap> bitmap);

  RetainPtr<IFX_SeekableReadStream> io_in() const { return m_io_in; }
  uint32_t offset() const { return m_offset; }
  void set_offset(uint32_t offset) { m_offset = offset; }

 private:
  RetainPtr<IFX_SeekableReadStream> m_io_in;
  uint32_t m_offset = 0;
  std::unique_ptr<TIFF, TiffDeleter> m_tif_ctx;
};

void* _TIFFcalloc(tmsize_t nmemb, tmsize_t siz) {
  return FXMEM_DefaultCalloc(nmemb, siz);
}

void* _TIFFmalloc(tmsize_t size) {
  return FXMEM_DefaultAlloc(size);
}

void _TIFFfree(void* ptr) {
  if (ptr)
    FXMEM_DefaultFree(ptr);
}

void* _TIFFrealloc(void* ptr, tmsize_t size) {
  return FXMEM_DefaultRealloc(ptr, size);
}

void _TIFFmemset(void* ptr, int val, tmsize_t size) {
  UNSAFE_TODO(FXSYS_memset(ptr, val, static_cast<size_t>(size)));
}

void _TIFFmemcpy(void* des, const void* src, tmsize_t size) {
  UNSAFE_TODO(FXSYS_memcpy(des, src, static_cast<size_t>(size)));
}

int _TIFFmemcmp(const void* ptr1, const void* ptr2, tmsize_t size) {
  return UNSAFE_TODO(memcmp(ptr1, ptr2, static_cast<size_t>(size)));
}

namespace {

tsize_t tiff_read(thandle_t context, tdata_t buf, tsize_t length) {
  CTiffContext* pTiffContext = reinterpret_cast<CTiffContext*>(context);
  FX_SAFE_UINT32 increment = pTiffContext->offset();
  increment += length;
  if (!increment.IsValid())
    return 0;

  FX_FILESIZE offset = pTiffContext->offset();
  // SAFETY: required from caller.
  if (!pTiffContext->io_in()->ReadBlockAtOffset(
          UNSAFE_BUFFERS(pdfium::make_span(static_cast<uint8_t*>(buf),
                                           static_cast<size_t>(length))),
          offset)) {
    return 0;
  }
  pTiffContext->set_offset(increment.ValueOrDie());
  if (offset + length > pTiffContext->io_in()->GetSize()) {
    return pdfium::checked_cast<tsize_t>(pTiffContext->io_in()->GetSize() -
                                         offset);
  }
  return length;
}

tsize_t tiff_write(thandle_t context, tdata_t buf, tsize_t length) {
  NOTREACHED_NORETURN();
}

toff_t tiff_seek(thandle_t context, toff_t offset, int whence) {
  CTiffContext* pTiffContext = reinterpret_cast<CTiffContext*>(context);
  FX_SAFE_FILESIZE safe_offset = offset;
  if (!safe_offset.IsValid())
    return static_cast<toff_t>(-1);
  FX_FILESIZE file_offset = safe_offset.ValueOrDie();

  switch (whence) {
    case 0: {
      if (file_offset > pTiffContext->io_in()->GetSize())
        return static_cast<toff_t>(-1);
      pTiffContext->set_offset(pdfium::checked_cast<uint32_t>(file_offset));
      return pTiffContext->offset();
    }
    case 1: {
      FX_SAFE_UINT32 new_increment = pTiffContext->offset();
      new_increment += file_offset;
      if (!new_increment.IsValid())
        return static_cast<toff_t>(-1);
      pTiffContext->set_offset(new_increment.ValueOrDie());
      return pTiffContext->offset();
    }
    case 2: {
      if (pTiffContext->io_in()->GetSize() < file_offset)
        return static_cast<toff_t>(-1);
      pTiffContext->set_offset(pdfium::checked_cast<uint32_t>(
          pTiffContext->io_in()->GetSize() - file_offset));
      return pTiffContext->offset();
    }
    default:
      return static_cast<toff_t>(-1);
  }
}

int tiff_close(thandle_t context) {
  return 0;
}

toff_t tiff_get_size(thandle_t context) {
  CTiffContext* pTiffContext = reinterpret_cast<CTiffContext*>(context);
  return static_cast<toff_t>(pTiffContext->io_in()->GetSize());
}

int tiff_map(thandle_t context, tdata_t*, toff_t*) {
  return 0;
}

void tiff_unmap(thandle_t context, tdata_t, toff_t) {}

}  // namespace

bool CTiffContext::InitDecoder(
    const RetainPtr<IFX_SeekableReadStream>& file_ptr) {
  m_io_in = file_ptr;
  m_tif_ctx.reset(TIFFClientOpen(
      /*name=*/"Tiff Image", /*mode=*/"r", /*clientdata=*/this, tiff_read,
      tiff_write, tiff_seek, tiff_close, tiff_get_size, tiff_map, tiff_unmap));
  return !!m_tif_ctx;
}

bool CTiffContext::LoadFrameInfo(int32_t frame,
                                 int32_t* width,
                                 int32_t* height,
                                 int32_t* comps,
                                 int32_t* bpc,
                                 CFX_DIBAttribute* pAttribute) {
  if (!TIFFSetDirectory(m_tif_ctx.get(), (uint16_t)frame))
    return false;

  uint32_t tif_width = 0;
  uint32_t tif_height = 0;
  uint16_t tif_comps = 0;
  uint16_t tif_bpc = 0;
  uint32_t tif_rps = 0;
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_IMAGEWIDTH, &tif_width);
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_IMAGELENGTH, &tif_height);
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_SAMPLESPERPIXEL, &tif_comps);
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_BITSPERSAMPLE, &tif_bpc);
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_ROWSPERSTRIP, &tif_rps);

  uint16_t tif_resunit = 0;
  if (TIFFGetField(m_tif_ctx.get(), TIFFTAG_RESOLUTIONUNIT, &tif_resunit)) {
    pAttribute->m_wDPIUnit =
        static_cast<CFX_DIBAttribute::ResUnit>(tif_resunit - 1);
  } else {
    pAttribute->m_wDPIUnit = CFX_DIBAttribute::kResUnitInch;
  }

  float tif_xdpi = 0.0f;
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_XRESOLUTION, &tif_xdpi);
  if (tif_xdpi)
    pAttribute->m_nXDPI = static_cast<int32_t>(tif_xdpi + 0.5f);

  float tif_ydpi = 0.0f;
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_YRESOLUTION, &tif_ydpi);
  if (tif_ydpi)
    pAttribute->m_nYDPI = static_cast<int32_t>(tif_ydpi + 0.5f);

  FX_SAFE_INT32 checked_width = tif_width;
  FX_SAFE_INT32 checked_height = tif_height;
  if (!checked_width.IsValid() || !checked_height.IsValid())
    return false;

  *width = checked_width.ValueOrDie();
  *height = checked_height.ValueOrDie();
  *comps = tif_comps;
  *bpc = tif_bpc;
  if (tif_rps > tif_height) {
    tif_rps = tif_height;
    TIFFSetField(m_tif_ctx.get(), TIFFTAG_ROWSPERSTRIP, tif_rps);
  }
  return true;
}

bool CTiffContext::Decode(RetainPtr<CFX_DIBitmap> bitmap) {
  // TODO(crbug.com/355630556): Consider adding support for
  // `FXDIB_Format::kBgraPremul`
  CHECK_EQ(bitmap->GetFormat(), FXDIB_Format::kBgra);
  const uint32_t img_width = bitmap->GetWidth();
  const uint32_t img_height = bitmap->GetHeight();
  uint32_t width = 0;
  uint32_t height = 0;
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_IMAGEWIDTH, &width);
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_IMAGELENGTH, &height);
  if (img_width != width || img_height != height) {
    return false;
  }

  uint16_t rotation = ORIENTATION_TOPLEFT;
  TIFFGetField(m_tif_ctx.get(), TIFFTAG_ORIENTATION, &rotation);
  uint32_t* data =
      fxcrt::reinterpret_span<uint32_t>(bitmap->GetWritableBuffer()).data();
  if (!TIFFReadRGBAImageOriented(m_tif_ctx.get(), img_width, img_height, data,
                                 rotation, 1)) {
    return false;
  }

  for (uint32_t row = 0; row < img_height; row++) {
    auto row_span = bitmap->GetWritableScanlineAs<FX_BGRA_STRUCT<uint8_t>>(row);
    for (auto& pixel : row_span) {
      std::swap(pixel.blue, pixel.red);
    }
  }
  return true;
}

namespace fxcodec {

// static
std::unique_ptr<ProgressiveDecoderIface::Context> TiffDecoder::CreateDecoder(
    const RetainPtr<IFX_SeekableReadStream>& file_ptr) {
  auto pDecoder = std::make_unique<CTiffContext>();
  if (!pDecoder->InitDecoder(file_ptr))
    return nullptr;

  return pDecoder;
}

// static
bool TiffDecoder::LoadFrameInfo(ProgressiveDecoderIface::Context* pContext,
                                int32_t frame,
                                int32_t* width,
                                int32_t* height,
                                int32_t* comps,
                                int32_t* bpc,
                                CFX_DIBAttribute* pAttribute) {
  DCHECK(pAttribute);

  auto* ctx = static_cast<CTiffContext*>(pContext);
  return ctx->LoadFrameInfo(frame, width, height, comps, bpc, pAttribute);
}

// static
bool TiffDecoder::Decode(ProgressiveDecoderIface::Context* pContext,
                         RetainPtr<CFX_DIBitmap> bitmap) {
  auto* ctx = static_cast<CTiffContext*>(pContext);
  return ctx->Decode(std::move(bitmap));
}

}  // namespace fxcodec
