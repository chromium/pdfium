// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/jpeg/jpegmodule.h"

#include <setjmp.h>

#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/scanlinedecoder.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxge/dib/cfx_dibbase.h"
#include "core/fxge/fx_dib.h"
#include "third_party/base/logging.h"
#include "third_party/base/optional.h"
#include "third_party/base/ptr_util.h"

extern "C" {
#undef FAR
#if defined(USE_SYSTEM_LIBJPEG)
#include <jerror.h>
#include <jpeglib.h>
#elif defined(USE_LIBJPEG_TURBO)
#include "third_party/libjpeg_turbo/jerror.h"
#include "third_party/libjpeg_turbo/jpeglib.h"
#else
#include "third_party/libjpeg/jerror.h"
#include "third_party/libjpeg/jpeglib.h"
#endif
}  // extern "C"

class CJpegContext final : public ModuleIface::Context {
 public:
  CJpegContext();
  ~CJpegContext() override;

  jmp_buf& GetJumpMark() { return m_JumpMark; }

  jmp_buf m_JumpMark;
  jpeg_decompress_struct m_Info = {};
  jpeg_error_mgr m_ErrMgr = {};
  jpeg_source_mgr m_SrcMgr = {};
  unsigned int m_SkipSize = 0;
  void* (*m_AllocFunc)(unsigned int);
  void (*m_FreeFunc)(void*);
};

static pdfium::span<const uint8_t> JpegScanSOI(
    pdfium::span<const uint8_t> src_span) {
  ASSERT(!src_span.empty());

  for (size_t offset = 0; offset < src_span.size() - 1; ++offset) {
    if (src_span[offset] == 0xff && src_span[offset + 1] == 0xd8)
      return src_span.subspan(offset);
  }
  return src_span;
}

extern "C" {

static void src_do_nothing(jpeg_decompress_struct* cinfo) {}

static void error_fatal(j_common_ptr cinfo) {
  longjmp(*(jmp_buf*)cinfo->client_data, -1);
}

static void src_skip_data(jpeg_decompress_struct* cinfo, long num) {
  if (num > (long)cinfo->src->bytes_in_buffer) {
    error_fatal((j_common_ptr)cinfo);
  }
  cinfo->src->next_input_byte += num;
  cinfo->src->bytes_in_buffer -= num;
}

static boolean src_fill_buffer(j_decompress_ptr cinfo) {
  return FALSE;
}

static boolean src_resync(j_decompress_ptr cinfo, int desired) {
  return FALSE;
}

static void error_do_nothing(j_common_ptr cinfo) {}

static void error_do_nothing1(j_common_ptr cinfo, int) {}

static void error_do_nothing2(j_common_ptr cinfo, char*) {}

#if defined(OS_WIN)
static void dest_do_nothing(j_compress_ptr cinfo) {}

static boolean dest_empty(j_compress_ptr cinfo) {
  return false;
}
#endif  // defined(OS_WIN)

static void error_fatal1(j_common_ptr cinfo) {
  auto* pContext = reinterpret_cast<CJpegContext*>(cinfo->client_data);
  longjmp(pContext->m_JumpMark, -1);
}

static void src_skip_data1(jpeg_decompress_struct* cinfo, long num) {
  if (cinfo->src->bytes_in_buffer < static_cast<size_t>(num)) {
    auto* pContext = reinterpret_cast<CJpegContext*>(cinfo->client_data);
    pContext->m_SkipSize = (unsigned int)(num - cinfo->src->bytes_in_buffer);
    cinfo->src->bytes_in_buffer = 0;
  } else {
    cinfo->src->next_input_byte += num;
    cinfo->src->bytes_in_buffer -= num;
  }
}

static void* jpeg_alloc_func(unsigned int size) {
  return FX_Alloc(char, size);
}

static void jpeg_free_func(void* p) {
  FX_Free(p);
}

}  // extern "C"

#ifdef PDF_ENABLE_XFA
static void JpegLoadAttribute(const jpeg_decompress_struct& info,
                              CFX_DIBAttribute* pAttribute) {
  pAttribute->m_nXDPI = info.X_density;
  pAttribute->m_nYDPI = info.Y_density;
  pAttribute->m_wDPIUnit = info.density_unit;
}
#endif  // PDF_ENABLE_XFA

static bool JpegLoadInfo(pdfium::span<const uint8_t> src_span,
                         int* width,
                         int* height,
                         int* num_components,
                         int* bits_per_components,
                         bool* color_transform) {
  src_span = JpegScanSOI(src_span);
  jpeg_decompress_struct cinfo;
  jpeg_error_mgr jerr;
  jerr.error_exit = error_fatal;
  jerr.emit_message = error_do_nothing1;
  jerr.output_message = error_do_nothing;
  jerr.format_message = error_do_nothing2;
  jerr.reset_error_mgr = error_do_nothing;
  jerr.trace_level = 0;
  cinfo.err = &jerr;
  jmp_buf mark;
  cinfo.client_data = &mark;
  if (setjmp(mark) == -1)
    return false;

  jpeg_create_decompress(&cinfo);
  jpeg_source_mgr src;
  src.init_source = src_do_nothing;
  src.term_source = src_do_nothing;
  src.skip_input_data = src_skip_data;
  src.fill_input_buffer = src_fill_buffer;
  src.resync_to_restart = src_resync;
  src.bytes_in_buffer = src_span.size();
  src.next_input_byte = src_span.data();
  cinfo.src = &src;
  if (setjmp(mark) == -1) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }
  int ret = jpeg_read_header(&cinfo, TRUE);
  if (ret != JPEG_HEADER_OK) {
    jpeg_destroy_decompress(&cinfo);
    return false;
  }
  *width = cinfo.image_width;
  *height = cinfo.image_height;
  *num_components = cinfo.num_components;
  *color_transform =
      cinfo.jpeg_color_space == JCS_YCbCr || cinfo.jpeg_color_space == JCS_YCCK;
  *bits_per_components = cinfo.data_precision;
  jpeg_destroy_decompress(&cinfo);
  return true;
}

CJpegContext::CJpegContext()
    : m_AllocFunc(jpeg_alloc_func), m_FreeFunc(jpeg_free_func) {
  m_Info.client_data = this;
  m_Info.err = &m_ErrMgr;

  m_ErrMgr.error_exit = error_fatal1;
  m_ErrMgr.emit_message = error_do_nothing1;
  m_ErrMgr.output_message = error_do_nothing;
  m_ErrMgr.format_message = error_do_nothing2;
  m_ErrMgr.reset_error_mgr = error_do_nothing;

  m_SrcMgr.init_source = src_do_nothing;
  m_SrcMgr.term_source = src_do_nothing;
  m_SrcMgr.skip_input_data = src_skip_data1;
  m_SrcMgr.fill_input_buffer = src_fill_buffer;
  m_SrcMgr.resync_to_restart = src_resync;
}

CJpegContext::~CJpegContext() {
  jpeg_destroy_decompress(&m_Info);
}

namespace fxcodec {

namespace {

constexpr size_t kKnownBadHeaderWithInvalidHeightByteOffsetStarts[] = {94, 163};

class JpegDecoder final : public ScanlineDecoder {
 public:
  JpegDecoder();
  ~JpegDecoder() override;

  bool Create(pdfium::span<const uint8_t> src_span,
              int width,
              int height,
              int nComps,
              bool ColorTransform);

  // ScanlineDecoder:
  bool v_Rewind() override;
  uint8_t* v_GetNextLine() override;
  uint32_t GetSrcOffset() override;

  bool InitDecode(bool bAcceptKnownBadHeader);

  jmp_buf m_JmpBuf;
  jpeg_decompress_struct m_Cinfo;
  jpeg_error_mgr m_Jerr;
  jpeg_source_mgr m_Src;
  pdfium::span<const uint8_t> m_SrcSpan;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pScanlineBuf;
  bool m_bInited = false;
  bool m_bStarted = false;
  bool m_bJpegTransform = false;

 private:
  void CalcPitch();
  void InitDecompressSrc();

  // Can only be called inside a jpeg_read_header() setjmp handler.
  bool HasKnownBadHeaderWithInvalidHeight(size_t dimension_offset) const;

  // Is a JPEG SOFn marker, which is defined as 0xff, 0xc[0-9a-f].
  bool IsSofSegment(size_t marker_offset) const;

  // Patch up the in-memory JPEG header for known bad JPEGs.
  void PatchUpKnownBadHeaderWithInvalidHeight(size_t dimension_offset);

  // Patch up the JPEG trailer, even if it is correct.
  void PatchUpTrailer();

  uint8_t* GetWritableSrcData();

  // For a given invalid height byte offset in
  // |kKnownBadHeaderWithInvalidHeightByteOffsetStarts|, the SOFn marker should
  // be this many bytes before that.
  static const size_t kSofMarkerByteOffset = 5;

  uint32_t m_nDefaultScaleDenom = 1;
};

JpegDecoder::JpegDecoder() {
  memset(&m_Cinfo, 0, sizeof(m_Cinfo));
  memset(&m_Jerr, 0, sizeof(m_Jerr));
  memset(&m_Src, 0, sizeof(m_Src));
}

JpegDecoder::~JpegDecoder() {
  if (m_bInited)
    jpeg_destroy_decompress(&m_Cinfo);
}

bool JpegDecoder::InitDecode(bool bAcceptKnownBadHeader) {
  m_Cinfo.err = &m_Jerr;
  m_Cinfo.client_data = &m_JmpBuf;
  if (setjmp(m_JmpBuf) == -1)
    return false;

  jpeg_create_decompress(&m_Cinfo);
  InitDecompressSrc();
  m_bInited = true;

  if (setjmp(m_JmpBuf) == -1) {
    Optional<size_t> known_bad_header_offset;
    if (bAcceptKnownBadHeader) {
      for (size_t offset : kKnownBadHeaderWithInvalidHeightByteOffsetStarts) {
        if (HasKnownBadHeaderWithInvalidHeight(offset)) {
          known_bad_header_offset = offset;
          break;
        }
      }
    }
    jpeg_destroy_decompress(&m_Cinfo);
    if (!known_bad_header_offset.has_value()) {
      m_bInited = false;
      return false;
    }

    PatchUpKnownBadHeaderWithInvalidHeight(known_bad_header_offset.value());

    jpeg_create_decompress(&m_Cinfo);
    InitDecompressSrc();
  }
  m_Cinfo.image_width = m_OrigWidth;
  m_Cinfo.image_height = m_OrigHeight;
  int ret = jpeg_read_header(&m_Cinfo, TRUE);
  if (ret != JPEG_HEADER_OK)
    return false;

  if (m_Cinfo.saw_Adobe_marker)
    m_bJpegTransform = true;

  if (m_Cinfo.num_components == 3 && !m_bJpegTransform)
    m_Cinfo.out_color_space = m_Cinfo.jpeg_color_space;

  m_OrigWidth = m_Cinfo.image_width;
  m_OrigHeight = m_Cinfo.image_height;
  m_OutputWidth = m_OrigWidth;
  m_OutputHeight = m_OrigHeight;
  m_nDefaultScaleDenom = m_Cinfo.scale_denom;
  return true;
}

bool JpegDecoder::Create(pdfium::span<const uint8_t> src_span,
                         int width,
                         int height,
                         int nComps,
                         bool ColorTransform) {
  m_SrcSpan = JpegScanSOI(src_span);
  if (m_SrcSpan.size() < 2)
    return false;

  PatchUpTrailer();

  m_Jerr.error_exit = error_fatal;
  m_Jerr.emit_message = error_do_nothing1;
  m_Jerr.output_message = error_do_nothing;
  m_Jerr.format_message = error_do_nothing2;
  m_Jerr.reset_error_mgr = error_do_nothing;
  m_Src.init_source = src_do_nothing;
  m_Src.term_source = src_do_nothing;
  m_Src.skip_input_data = src_skip_data;
  m_Src.fill_input_buffer = src_fill_buffer;
  m_Src.resync_to_restart = src_resync;
  m_bJpegTransform = ColorTransform;
  m_OutputWidth = m_OrigWidth = width;
  m_OutputHeight = m_OrigHeight = height;
  if (!InitDecode(/*bAcceptKnownBadHeader=*/true))
    return false;

  if (m_Cinfo.num_components < nComps)
    return false;

  if (static_cast<int>(m_Cinfo.image_width) < width)
    return false;

  CalcPitch();
  m_pScanlineBuf.reset(FX_Alloc(uint8_t, m_Pitch));
  m_nComps = m_Cinfo.num_components;
  m_bpc = 8;
  m_bStarted = false;
  return true;
}

bool JpegDecoder::v_Rewind() {
  if (m_bStarted) {
    jpeg_destroy_decompress(&m_Cinfo);
    if (!InitDecode(/*bAcceptKnownBadHeader=*/false)) {
      return false;
    }
  }
  if (setjmp(m_JmpBuf) == -1) {
    return false;
  }
  m_Cinfo.scale_denom = m_nDefaultScaleDenom;
  m_OutputWidth = m_OrigWidth;
  m_OutputHeight = m_OrigHeight;
  if (!jpeg_start_decompress(&m_Cinfo)) {
    jpeg_destroy_decompress(&m_Cinfo);
    return false;
  }
  if (static_cast<int>(m_Cinfo.output_width) > m_OrigWidth) {
    NOTREACHED();
    return false;
  }
  m_bStarted = true;
  return true;
}

uint8_t* JpegDecoder::v_GetNextLine() {
  if (setjmp(m_JmpBuf) == -1)
    return nullptr;

  uint8_t* row_array[] = {m_pScanlineBuf.get()};
  int nlines = jpeg_read_scanlines(&m_Cinfo, row_array, 1);
  return nlines > 0 ? m_pScanlineBuf.get() : nullptr;
}

uint32_t JpegDecoder::GetSrcOffset() {
  return static_cast<uint32_t>(m_SrcSpan.size() - m_Src.bytes_in_buffer);
}

void JpegDecoder::CalcPitch() {
  m_Pitch = static_cast<uint32_t>(m_Cinfo.image_width) * m_Cinfo.num_components;
  m_Pitch += 3;
  m_Pitch /= 4;
  m_Pitch *= 4;
}

void JpegDecoder::InitDecompressSrc() {
  m_Cinfo.src = &m_Src;
  m_Src.bytes_in_buffer = m_SrcSpan.size();
  m_Src.next_input_byte = m_SrcSpan.data();
}

bool JpegDecoder::HasKnownBadHeaderWithInvalidHeight(
    size_t dimension_offset) const {
  // Perform lots of possibly redundant checks to make sure this has no false
  // positives.
  bool bDimensionChecks = m_Cinfo.err->msg_code == JERR_IMAGE_TOO_BIG &&
                          m_Cinfo.image_width < JPEG_MAX_DIMENSION &&
                          m_Cinfo.image_height == 0xffff && m_OrigWidth > 0 &&
                          m_OrigWidth <= JPEG_MAX_DIMENSION &&
                          m_OrigHeight > 0 &&
                          m_OrigHeight <= JPEG_MAX_DIMENSION;
  if (!bDimensionChecks)
    return false;

  if (m_SrcSpan.size() <= dimension_offset + 3u)
    return false;

  if (!IsSofSegment(dimension_offset - kSofMarkerByteOffset))
    return false;

  const uint8_t* pHeaderDimensions = &m_SrcSpan[dimension_offset];
  uint8_t nExpectedWidthByte1 = (m_OrigWidth >> 8) & 0xff;
  uint8_t nExpectedWidthByte2 = m_OrigWidth & 0xff;
  // Height high byte, height low byte, width high byte, width low byte.
  return pHeaderDimensions[0] == 0xff && pHeaderDimensions[1] == 0xff &&
         pHeaderDimensions[2] == nExpectedWidthByte1 &&
         pHeaderDimensions[3] == nExpectedWidthByte2;
}

bool JpegDecoder::IsSofSegment(size_t marker_offset) const {
  const uint8_t* pHeaderMarker = &m_SrcSpan[marker_offset];
  return pHeaderMarker[0] == 0xff && pHeaderMarker[1] >= 0xc0 &&
         pHeaderMarker[1] <= 0xcf;
}

void JpegDecoder::PatchUpKnownBadHeaderWithInvalidHeight(
    size_t dimension_offset) {
  ASSERT(m_SrcSpan.size() > dimension_offset + 1u);
  uint8_t* pData = GetWritableSrcData() + dimension_offset;
  pData[0] = (m_OrigHeight >> 8) & 0xff;
  pData[1] = m_OrigHeight & 0xff;
}

void JpegDecoder::PatchUpTrailer() {
  uint8_t* pData = GetWritableSrcData();
  pData[m_SrcSpan.size() - 2] = 0xff;
  pData[m_SrcSpan.size() - 1] = 0xd9;
}

uint8_t* JpegDecoder::GetWritableSrcData() {
  return const_cast<uint8_t*>(m_SrcSpan.data());
}

}  // namespace

std::unique_ptr<ScanlineDecoder> JpegModule::CreateDecoder(
    pdfium::span<const uint8_t> src_span,
    int width,
    int height,
    int nComps,
    bool ColorTransform) {
  ASSERT(!src_span.empty());

  auto pDecoder = pdfium::MakeUnique<JpegDecoder>();
  if (!pDecoder->Create(src_span, width, height, nComps, ColorTransform))
    return nullptr;

  return std::move(pDecoder);
}

Optional<JpegModule::JpegImageInfo> JpegModule::LoadInfo(
    pdfium::span<const uint8_t> src_span) {
  JpegImageInfo info;
  if (!JpegLoadInfo(src_span, &info.width, &info.height, &info.num_components,
                    &info.bits_per_components, &info.color_transform)) {
    return pdfium::nullopt;
  }
  return info;
}

std::unique_ptr<ModuleIface::Context> JpegModule::Start() {
  // Use ordinary pointer until past the possibility of a longjump.
  auto* pContext = new CJpegContext();
  if (setjmp(pContext->m_JumpMark) == -1) {
    delete pContext;
    return nullptr;
  }

  jpeg_create_decompress(&pContext->m_Info);
  pContext->m_Info.src = &pContext->m_SrcMgr;
  pContext->m_SkipSize = 0;
  return pdfium::WrapUnique(pContext);
}

bool JpegModule::Input(Context* pContext,
                       RetainPtr<CFX_CodecMemory> codec_memory,
                       CFX_DIBAttribute*) {
  pdfium::span<uint8_t> src_buf = codec_memory->GetSpan();
  auto* ctx = static_cast<CJpegContext*>(pContext);
  if (ctx->m_SkipSize) {
    if (ctx->m_SkipSize > src_buf.size()) {
      ctx->m_SrcMgr.bytes_in_buffer = 0;
      ctx->m_SkipSize -= src_buf.size();
      return true;
    }
    src_buf = src_buf.subspan(ctx->m_SkipSize);
    ctx->m_SkipSize = 0;
  }
  ctx->m_SrcMgr.next_input_byte = src_buf.data();
  ctx->m_SrcMgr.bytes_in_buffer = src_buf.size();
  return true;
}

#ifdef PDF_ENABLE_XFA
int JpegModule::ReadHeader(Context* pContext,
                           int* width,
                           int* height,
                           int* nComps,
                           CFX_DIBAttribute* pAttribute) {
  ASSERT(pAttribute);

  auto* ctx = static_cast<CJpegContext*>(pContext);
  int ret = jpeg_read_header(&ctx->m_Info, TRUE);
  if (ret == JPEG_SUSPENDED)
    return 2;
  if (ret != JPEG_HEADER_OK)
    return 1;

  *width = ctx->m_Info.image_width;
  *height = ctx->m_Info.image_height;
  *nComps = ctx->m_Info.num_components;
  JpegLoadAttribute(ctx->m_Info, pAttribute);
  return 0;
}
#endif  // PDF_ENABLE_XFA

bool JpegModule::StartScanline(Context* pContext, int down_scale) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  ctx->m_Info.scale_denom = static_cast<unsigned int>(down_scale);
  return !!jpeg_start_decompress(&ctx->m_Info);
}

bool JpegModule::ReadScanline(Context* pContext, unsigned char* dest_buf) {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  unsigned int nlines = jpeg_read_scanlines(&ctx->m_Info, &dest_buf, 1);
  return nlines == 1;
}

FX_FILESIZE JpegModule::GetAvailInput(Context* pContext) const {
  auto* ctx = static_cast<CJpegContext*>(pContext);
  return static_cast<FX_FILESIZE>(ctx->m_SrcMgr.bytes_in_buffer);
}

jmp_buf& JpegModule::GetJumpMark(Context* pContext) {
  return static_cast<CJpegContext*>(pContext)->GetJumpMark();
}

#if defined(OS_WIN)
bool JpegModule::JpegEncode(const RetainPtr<CFX_DIBBase>& pSource,
                            uint8_t** dest_buf,
                            size_t* dest_size) {
  jpeg_error_mgr jerr;
  jerr.error_exit = error_do_nothing;
  jerr.emit_message = error_do_nothing1;
  jerr.output_message = error_do_nothing;
  jerr.format_message = error_do_nothing2;
  jerr.reset_error_mgr = error_do_nothing;

  jpeg_compress_struct cinfo;
  memset(&cinfo, 0, sizeof(cinfo));
  cinfo.err = &jerr;
  jpeg_create_compress(&cinfo);
  int Bpp = pSource->GetBPP() / 8;
  uint32_t nComponents = Bpp >= 3 ? (pSource->IsCmykImage() ? 4 : 3) : 1;
  uint32_t pitch = pSource->GetPitch();
  uint32_t width = pdfium::base::checked_cast<uint32_t>(pSource->GetWidth());
  uint32_t height = pdfium::base::checked_cast<uint32_t>(pSource->GetHeight());
  FX_SAFE_UINT32 safe_buf_len = width;
  safe_buf_len *= height;
  safe_buf_len *= nComponents;
  safe_buf_len += 1024;
  if (!safe_buf_len.IsValid())
    return false;

  uint32_t dest_buf_length = safe_buf_len.ValueOrDie();
  *dest_buf = FX_TryAlloc(uint8_t, dest_buf_length);
  const int MIN_TRY_BUF_LEN = 1024;
  while (!(*dest_buf) && dest_buf_length > MIN_TRY_BUF_LEN) {
    dest_buf_length >>= 1;
    *dest_buf = FX_TryAlloc(uint8_t, dest_buf_length);
  }
  if (!(*dest_buf))
    return false;

  jpeg_destination_mgr dest;
  dest.init_destination = dest_do_nothing;
  dest.term_destination = dest_do_nothing;
  dest.empty_output_buffer = dest_empty;
  dest.next_output_byte = *dest_buf;
  dest.free_in_buffer = dest_buf_length;
  cinfo.dest = &dest;
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = nComponents;
  if (nComponents == 1) {
    cinfo.in_color_space = JCS_GRAYSCALE;
  } else if (nComponents == 3) {
    cinfo.in_color_space = JCS_RGB;
  } else {
    cinfo.in_color_space = JCS_CMYK;
  }
  uint8_t* line_buf = nullptr;
  if (nComponents > 1)
    line_buf = FX_Alloc2D(uint8_t, width, nComponents);

  jpeg_set_defaults(&cinfo);
  jpeg_start_compress(&cinfo, TRUE);
  JSAMPROW row_pointer[1];
  JDIMENSION row;
  while (cinfo.next_scanline < cinfo.image_height) {
    const uint8_t* src_scan = pSource->GetScanline(cinfo.next_scanline);
    if (nComponents > 1) {
      uint8_t* dest_scan = line_buf;
      if (nComponents == 3) {
        for (uint32_t i = 0; i < width; i++) {
          dest_scan[0] = src_scan[2];
          dest_scan[1] = src_scan[1];
          dest_scan[2] = src_scan[0];
          dest_scan += 3;
          src_scan += Bpp;
        }
      } else {
        for (uint32_t i = 0; i < pitch; i++) {
          *dest_scan++ = ~*src_scan++;
        }
      }
      row_pointer[0] = line_buf;
    } else {
      row_pointer[0] = const_cast<uint8_t*>(src_scan);
    }
    row = cinfo.next_scanline;
    jpeg_write_scanlines(&cinfo, row_pointer, 1);
    if (cinfo.next_scanline == row) {
      constexpr size_t kJpegBlockSize = 1048576;
      *dest_buf =
          FX_Realloc(uint8_t, *dest_buf, dest_buf_length + kJpegBlockSize);
      dest.next_output_byte = *dest_buf + dest_buf_length - dest.free_in_buffer;
      dest_buf_length += kJpegBlockSize;
      dest.free_in_buffer += kJpegBlockSize;
    }
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
  FX_Free(line_buf);
  *dest_size = dest_buf_length - static_cast<size_t>(dest.free_in_buffer);

  return true;
}
#endif  // defined(OS_WIN)

}  // namespace fxcodec
