// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/progressive_decoder.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "build/build_config.h"
#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcodec/jpeg/jpeg_progressive_decoder.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_2d_size.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/dib/cfx_cmyk_to_srgb.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/dib/fx_dib.h"

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
    UNSAFE_TODO({
      for (; i < width; i++, j += 3) {
        temp = buffer[j];
        buffer[j] = buffer[j + 2];
        buffer[j + 2] = temp;
      }
    });
  }
}

}  // namespace

ProgressiveDecoder::ProgressiveDecoder() = default;

ProgressiveDecoder::~ProgressiveDecoder() = default;

#ifdef PDF_ENABLE_XFA_PNG
bool ProgressiveDecoder::PngReadHeader(int width,
                                       int height,
                                       int bpc,
                                       int pass,
                                       int* color_type,
                                       double* gamma) {
  if (!device_bitmap_) {
    src_width_ = width;
    src_height_ = height;
    src_bpc_ = bpc;
    src_pass_number_ = pass;
    switch (*color_type) {
      case 0:
        src_components_ = 1;
        break;
      case 4:
        src_components_ = 2;
        break;
      case 2:
        src_components_ = 3;
        break;
      case 3:
      case 6:
        src_components_ = 4;
        break;
      default:
        src_components_ = 0;
        break;
    }
    return false;
  }
  switch (device_bitmap_->GetFormat()) {
    case FXDIB_Format::kInvalid:
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb:
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      NOTREACHED();
    case FXDIB_Format::kBgr:
      *color_type = 2;
      break;
    case FXDIB_Format::kBgrx:
    case FXDIB_Format::kBgra:
      *color_type = 6;
      break;
#if defined(PDF_USE_SKIA)
    case FXDIB_Format::kBgraPremul:
      // TODO(crbug.com/355630556): Consider adding support for
      // `FXDIB_Format::kBgraPremul`
      NOTREACHED();
#endif
  }
  *gamma = kPngGamma;
  return true;
}

uint8_t* ProgressiveDecoder::PngAskScanlineBuf(int line) {
  CHECK_GE(line, 0);
  CHECK_LT(line, src_height_);
  CHECK_EQ(device_bitmap_->GetFormat(), FXDIB_Format::kBgra);
  CHECK_EQ(src_format_, FXCodec_Argb);
  pdfium::span<const uint8_t> src_span = device_bitmap_->GetScanline(line);
  pdfium::span<uint8_t> dest_span = pdfium::span(decode_buf_);
  const size_t byte_size = Fx2DSizeOrDie(
      src_width_, GetCompsFromFormat(device_bitmap_->GetFormat()));
  fxcrt::Copy(src_span.first(byte_size), dest_span);
  return decode_buf_.data();
}

void ProgressiveDecoder::PngFillScanlineBufCompleted(int pass, int line) {
  if (line < 0 || line >= src_height_) {
    return;
  }

  CHECK_EQ(device_bitmap_->GetFormat(), FXDIB_Format::kBgra);
  pdfium::span<const uint8_t> src_span = pdfium::span(decode_buf_);
  pdfium::span<uint8_t> dest_span = device_bitmap_->GetWritableScanline(line);
  const size_t byte_size = Fx2DSizeOrDie(
      src_width_, GetCompsFromFormat(device_bitmap_->GetFormat()));
  fxcrt::Copy(src_span.first(byte_size), dest_span);
}
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_GIF
uint32_t ProgressiveDecoder::GifCurrentPosition() const {
  uint32_t remain_size = pdfium::checked_cast<uint32_t>(
      GifDecoder::GetAvailInput(gif_context_.get()));
  return offset_ - remain_size;
}

bool ProgressiveDecoder::GifInputRecordPositionBuf(
    uint32_t rcd_pos,
    const FX_RECT& img_rc,
    pdfium::span<CFX_GifPalette> pal_span,
    int32_t trans_index,
    bool interlace) {
  offset_ = rcd_pos;

  FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
  codec_memory_->Seek(codec_memory_->GetSize());
  if (!GifReadMoreData(&error_status)) {
    return false;
  }

  if (pal_span.empty()) {
    pal_span = gif_palette_;
  }
  if (pal_span.empty()) {
    return false;
  }
  src_palette_.resize(pal_span.size());
  for (size_t i = 0; i < pal_span.size(); i++) {
    src_palette_[i] =
        ArgbEncode(0xff, pal_span[i].r, pal_span[i].g, pal_span[i].b);
  }
  gif_trans_index_ = trans_index;
  gif_frame_rect_ = img_rc;
  src_pass_number_ = interlace ? 4 : 1;
  int32_t pal_index = gif_bg_index_;
  RetainPtr<CFX_DIBitmap> pDevice = device_bitmap_;
  if (trans_index >= static_cast<int>(pal_span.size())) {
    trans_index = -1;
  }
  if (trans_index != -1) {
    src_palette_[trans_index] &= 0x00ffffff;
    if (pDevice->IsAlphaFormat()) {
      pal_index = trans_index;
    }
  }
  if (pal_index >= static_cast<int>(pal_span.size())) {
    return false;
  }
  int startX = 0;
  int startY = 0;
  int sizeX = src_width_;
  int sizeY = src_height_;
  const int bytes_per_pixel = pDevice->GetBPP() / 8;
  FX_ARGB argb = src_palette_[pal_index];
  for (int row = 0; row < sizeY; row++) {
    pdfium::span<uint8_t> scan_span =
        pDevice->GetWritableScanline(row + startY)
            .subspan(static_cast<size_t>(startX * bytes_per_pixel));
    switch (trans_method_) {
      case TransformMethod::k8BppRgbToRgbNoAlpha: {
        uint8_t* pScanline = scan_span.data();
        UNSAFE_TODO({
          for (int col = 0; col < sizeX; col++) {
            *pScanline++ = FXARGB_B(argb);
            *pScanline++ = FXARGB_G(argb);
            *pScanline++ = FXARGB_R(argb);
            pScanline += bytes_per_pixel - 3;
          }
        });
        break;
      }
      case TransformMethod::k8BppRgbToArgb: {
        uint8_t* pScanline = scan_span.data();
        UNSAFE_TODO({
          for (int col = 0; col < sizeX; col++) {
            FXARGB_SetDIB(pScanline, argb);
            pScanline += 4;
          }
        });
        break;
      }
      default:
        break;
    }
  }
  return true;
}

void ProgressiveDecoder::GifReadScanline(int32_t row_num,
                                         pdfium::span<uint8_t> row_buf) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = device_bitmap_;
  DCHECK(pDIBitmap);
  int32_t img_width = gif_frame_rect_.Width();
  if (!pDIBitmap->IsAlphaFormat()) {
    pdfium::span<uint8_t> byte_span = row_buf;
    for (int i = 0; i < img_width; i++) {
      if (byte_span.front() == gif_trans_index_) {
        byte_span.front() = gif_bg_index_;
      }
      byte_span = byte_span.subspan<1u>();
    }
  }
  int32_t pal_index = gif_bg_index_;
  if (gif_trans_index_ != -1 && device_bitmap_->IsAlphaFormat()) {
    pal_index = gif_trans_index_;
  }
  const int32_t left = gif_frame_rect_.left;
  const pdfium::span<uint8_t> decode_span = decode_buf_;
  std::ranges::fill(decode_span.first(static_cast<size_t>(src_width_)),
                    pal_index);
  fxcrt::Copy(row_buf.first(static_cast<size_t>(img_width)),
              decode_span.subspan(static_cast<size_t>(left)));

  int32_t line = row_num + gif_frame_rect_.top;
  if (line < 0 || line >= src_height_) {
    return;
  }

  ResampleScanline(pDIBitmap, line, decode_span, src_format_);
}
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_BMP
bool ProgressiveDecoder::BmpInputImagePositionBuf(uint32_t rcd_pos) {
  offset_ = rcd_pos;
  FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
  return BmpReadMoreData(bmp_context_.get(), &error_status);
}

void ProgressiveDecoder::BmpReadScanline(uint32_t row_num,
                                         pdfium::span<const uint8_t> row_buf) {
  RetainPtr<CFX_DIBitmap> pDIBitmap = device_bitmap_;
  DCHECK(pDIBitmap);

  fxcrt::Copy(row_buf.first(static_cast<size_t>(scanline_size_)), decode_buf_);

  if (row_num >= static_cast<uint32_t>(src_height_)) {
    return;
  }

  ResampleScanline(pDIBitmap, row_num, decode_buf_, src_format_);
}

bool ProgressiveDecoder::BmpDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  std::unique_ptr<ProgressiveDecoderIface::Context> pBmpContext =
      BmpDecoder::StartDecode(this);
  BmpDecoder::Input(pBmpContext.get(), codec_memory_);

  pdfium::span<const FX_ARGB> palette;
  BmpDecoder::Status read_result = BmpDecoder::ReadHeader(
      pBmpContext.get(), &src_width_, &src_height_, &bmp_is_top_bottom_,
      &src_components_, &palette, pAttribute);
  while (read_result == BmpDecoder::Status::kContinue) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!BmpReadMoreData(pBmpContext.get(), &error_status)) {
      status_ = error_status;
      return false;
    }
    read_result = BmpDecoder::ReadHeader(
        pBmpContext.get(), &src_width_, &src_height_, &bmp_is_top_bottom_,
        &src_components_, &palette, pAttribute);
  }

  if (read_result != BmpDecoder::Status::kSuccess) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }

  FXDIB_Format format = FXDIB_Format::kInvalid;
  switch (src_components_) {
    case 1:
      src_format_ = FXCodec_8bppRgb;
      format = FXDIB_Format::k8bppRgb;
      break;
    case 3:
      src_format_ = FXCodec_Rgb;
      format = FXDIB_Format::kBgr;
      break;
    case 4:
      src_format_ = FXCodec_Rgb32;
      format = FXDIB_Format::kBgrx;
      break;
    default:
      status_ = FXCODEC_STATUS::kError;
      return false;
  }

  // Set to 0 to make CalculatePitchAndSize() calculate it.
  static constexpr uint32_t kNoPitch = 0;
  std::optional<CFX_DIBitmap::PitchAndSize> needed_data =
      CFX_DIBitmap::CalculatePitchAndSize(src_width_, src_height_, format,
                                          kNoPitch);
  if (!needed_data.has_value()) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }

  uint32_t available_data = pdfium::checked_cast<uint32_t>(
      file_->GetSize() - offset_ +
      BmpDecoder::GetAvailInput(pBmpContext.get()));
  if (needed_data.value().size > available_data) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }

  src_bpc_ = 8;
  bmp_context_ = std::move(pBmpContext);
  if (!palette.empty()) {
    src_palette_.resize(palette.size());
    fxcrt::Copy(palette, src_palette_);
  } else {
    src_palette_.clear();
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
  SetTransMethod();
  scanline_size_ = FxAlignToBoundary<4>(src_width_ * src_components_);
  decode_buf_.resize(scanline_size_);
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  weight_horz_.CalculateWeights(src_width_, 0, src_width_, src_width_, 0,
                                src_width_, options);
  status_ = FXCODEC_STATUS::kDecodeToBeContinued;
  return status_;
}

FXCODEC_STATUS ProgressiveDecoder::BmpContinueDecode() {
  BmpDecoder::Status read_res = BmpDecoder::LoadImage(bmp_context_.get());
  while (read_res == BmpDecoder::Status::kContinue) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
    if (!BmpReadMoreData(bmp_context_.get(), &error_status)) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = error_status;
      return status_;
    }
    read_res = BmpDecoder::LoadImage(bmp_context_.get());
  }

  device_bitmap_ = nullptr;
  file_ = nullptr;
  status_ = read_res == BmpDecoder::Status::kSuccess
                ? FXCODEC_STATUS::kDecodeFinished
                : FXCODEC_STATUS::kError;
  return status_;
}
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
bool ProgressiveDecoder::GifReadMoreData(FXCODEC_STATUS* err_status) {
  return ReadMoreData(GifProgressiveDecoder::GetInstance(), gif_context_.get(),
                      err_status);
}

bool ProgressiveDecoder::GifDetectImageTypeInBuffer() {
  gif_context_ = GifDecoder::StartDecode(this);
  GifDecoder::Input(gif_context_.get(), codec_memory_);
  src_components_ = 1;
  GifDecoder::Status readResult =
      GifDecoder::ReadHeader(gif_context_.get(), &src_width_, &src_height_,
                             &gif_palette_, &gif_bg_index_);
  while (readResult == GifDecoder::Status::kUnfinished) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!GifReadMoreData(&error_status)) {
      gif_context_ = nullptr;
      status_ = error_status;
      return false;
    }
    readResult =
        GifDecoder::ReadHeader(gif_context_.get(), &src_width_, &src_height_,
                               &gif_palette_, &gif_bg_index_);
  }
  if (readResult == GifDecoder::Status::kSuccess) {
    src_bpc_ = 8;
    return true;
  }
  gif_context_ = nullptr;
  status_ = FXCODEC_STATUS::kError;
  return false;
}

FXCODEC_STATUS ProgressiveDecoder::GifStartDecode() {
  src_format_ = FXCodec_8bppRgb;
  SetTransMethod();
  int scanline_size = FxAlignToBoundary<4>(src_width_);
  decode_buf_.resize(scanline_size);
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  weight_horz_.CalculateWeights(src_width_, 0, src_width_, src_width_, 0,
                                src_width_, options);
  frame_cur_ = 0;
  status_ = FXCODEC_STATUS::kDecodeToBeContinued;
  return status_;
}

FXCODEC_STATUS ProgressiveDecoder::GifContinueDecode() {
  GifDecoder::Status readRes =
      GifDecoder::LoadFrame(gif_context_.get(), frame_cur_);
  while (readRes == GifDecoder::Status::kUnfinished) {
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
    if (!GifReadMoreData(&error_status)) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = error_status;
      return status_;
    }
    readRes = GifDecoder::LoadFrame(gif_context_.get(), frame_cur_);
  }

  if (readRes == GifDecoder::Status::kSuccess) {
    device_bitmap_ = nullptr;
    file_ = nullptr;
    status_ = FXCODEC_STATUS::kDecodeFinished;
    return status_;
  }

  device_bitmap_ = nullptr;
  file_ = nullptr;
  status_ = FXCODEC_STATUS::kError;
  return status_;
}
#endif  // PDF_ENABLE_XFA_GIF

bool ProgressiveDecoder::JpegReadMoreData(FXCODEC_STATUS* err_status) {
  return ReadMoreData(JpegProgressiveDecoder::GetInstance(),
                      jpeg_context_.get(), err_status);
}

bool ProgressiveDecoder::JpegDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  jpeg_context_ = JpegProgressiveDecoder::Start();
  if (!jpeg_context_) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  JpegProgressiveDecoder::GetInstance()->Input(jpeg_context_.get(),
                                               codec_memory_);

  while (1) {
    int read_result = JpegProgressiveDecoder::ReadHeader(
        jpeg_context_.get(), &src_width_, &src_height_, &src_components_,
        pAttribute);
    switch (read_result) {
      case JpegProgressiveDecoder::kFatal:
      case JpegProgressiveDecoder::kError:
        status_ = FXCODEC_STATUS::kError;
        return false;
      case JpegProgressiveDecoder::kOk:
        src_bpc_ = 8;
        return true;
      case JpegProgressiveDecoder::kNeedsMoreInput: {
        FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
        if (!JpegReadMoreData(&error_status)) {
          status_ = error_status;
          return false;
        }
        break;
      }
      default:
        NOTREACHED();
    }
  }
}

FXCODEC_STATUS ProgressiveDecoder::JpegStartDecode() {
  while (!JpegProgressiveDecoder::StartScanline(jpeg_context_.get())) {
    // Maybe it needs more data.
    FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
    if (!JpegReadMoreData(&error_status)) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = error_status;
      return status_;
    }
  }
  decode_buf_.resize(FxAlignToBoundary<4>(src_width_ * src_components_));
  FXDIB_ResampleOptions options;
  options.bInterpolateBilinear = true;
  weight_horz_.CalculateWeights(src_width_, 0, src_width_, src_width_, 0,
                                src_width_, options);
  switch (src_components_) {
    case 1:
      src_format_ = FXCodec_8bppGray;
      break;
    case 3:
      src_format_ = FXCodec_Rgb;
      break;
    case 4:
      src_format_ = FXCodec_Cmyk;
      break;
  }
  SetTransMethod();
  status_ = FXCODEC_STATUS::kDecodeToBeContinued;
  return status_;
}

FXCODEC_STATUS ProgressiveDecoder::JpegContinueDecode() {
  while (true) {
    int err_code = JpegProgressiveDecoder::ReadScanline(jpeg_context_.get(),
                                                        decode_buf_.data());
    if (err_code == JpegProgressiveDecoder::kFatal) {
      jpeg_context_.reset();
      status_ = FXCODEC_STATUS::kError;
      return FXCODEC_STATUS::kError;
    }
    if (err_code != JpegProgressiveDecoder::kOk) {
      // Maybe it needs more data.
      FXCODEC_STATUS error_status = FXCODEC_STATUS::kDecodeFinished;
      if (JpegReadMoreData(&error_status)) {
        continue;
      }
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = error_status;
      return status_;
    }
    if (src_format_ == FXCodec_Rgb) {
      RGB2BGR(UNSAFE_TODO(decode_buf_.data()), src_width_);
    }
    if (src_row_ >= src_height_) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = FXCODEC_STATUS::kDecodeFinished;
      return status_;
    }
    Resample(device_bitmap_, src_row_, decode_buf_.data(), src_format_);
    src_row_++;
  }
}

#ifdef PDF_ENABLE_XFA_PNG
bool ProgressiveDecoder::PngDetectImageTypeInBuffer(
    CFX_DIBAttribute* pAttribute) {
  png_context_ = PngDecoder::StartDecode(this);
  if (!png_context_) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  while (PngDecoder::ContinueDecode(png_context_.get(), codec_memory_,
                                    pAttribute)) {
    uint32_t remain_size = static_cast<uint32_t>(file_->GetSize()) - offset_;
    uint32_t input_size = std::min<uint32_t>(remain_size, kBlockSize);
    if (input_size == 0) {
      png_context_.reset();
      status_ = FXCODEC_STATUS::kError;
      return false;
    }
    if (codec_memory_ && input_size > codec_memory_->GetSize()) {
      codec_memory_ = pdfium::MakeRetain<CFX_CodecMemory>(input_size);
    }

    if (!file_->ReadBlockAtOffset(
            codec_memory_->GetBufferSpan().first(input_size), offset_)) {
      status_ = FXCODEC_STATUS::kError;
      return false;
    }
    offset_ += input_size;
  }
  png_context_.reset();
  if (src_pass_number_ == 0) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  return true;
}

FXCODEC_STATUS ProgressiveDecoder::PngStartDecode() {
  png_context_ = PngDecoder::StartDecode(this);
  if (!png_context_) {
    device_bitmap_ = nullptr;
    file_ = nullptr;
    status_ = FXCODEC_STATUS::kError;
    return status_;
  }
  offset_ = 0;
  CHECK_EQ(device_bitmap_->GetFormat(), FXDIB_Format::kBgra);
  src_components_ = 4;
  src_format_ = FXCodec_Argb;
  SetTransMethod();
  int scanline_size = FxAlignToBoundary<4>(src_width_ * src_components_);
  decode_buf_.resize(scanline_size);
  status_ = FXCODEC_STATUS::kDecodeToBeContinued;
  return status_;
}

FXCODEC_STATUS ProgressiveDecoder::PngContinueDecode() {
  while (true) {
    uint32_t remain_size = (uint32_t)file_->GetSize() - offset_;
    uint32_t input_size = std::min<uint32_t>(remain_size, kBlockSize);
    if (input_size == 0) {
      png_context_.reset();
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = FXCODEC_STATUS::kDecodeFinished;
      return status_;
    }
    if (codec_memory_ && input_size > codec_memory_->GetSize()) {
      codec_memory_ = pdfium::MakeRetain<CFX_CodecMemory>(input_size);
    }

    bool bResult = file_->ReadBlockAtOffset(
        codec_memory_->GetBufferSpan().first(input_size), offset_);
    if (!bResult) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = FXCODEC_STATUS::kError;
      return status_;
    }
    offset_ += input_size;
    bResult =
        PngDecoder::ContinueDecode(png_context_.get(), codec_memory_, nullptr);
    if (!bResult) {
      device_bitmap_ = nullptr;
      file_ = nullptr;
      status_ = FXCODEC_STATUS::kError;
      return status_;
    }
  }
}
#endif  // PDF_ENABLE_XFA_PNG

#ifdef PDF_ENABLE_XFA_TIFF
bool ProgressiveDecoder::TiffDetectImageTypeFromFile(
    CFX_DIBAttribute* pAttribute) {
  tiff_context_ = TiffDecoder::CreateDecoder(file_);
  if (!tiff_context_) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  int32_t dummy_bpc;
  bool ret = TiffDecoder::LoadFrameInfo(tiff_context_.get(), 0, &src_width_,
                                        &src_height_, &src_components_,
                                        &dummy_bpc, pAttribute);
  src_components_ = 4;
  if (!ret) {
    tiff_context_.reset();
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  return true;
}

FXCODEC_STATUS ProgressiveDecoder::TiffContinueDecode() {
  // TODO(crbug.com/355630556): Consider adding support for
  // `FXDIB_Format::kBgraPremul`
  CHECK_EQ(device_bitmap_->GetFormat(), FXDIB_Format::kBgra);
  status_ = TiffDecoder::Decode(tiff_context_.get(), std::move(device_bitmap_))
                ? FXCODEC_STATUS::kDecodeFinished
                : FXCODEC_STATUS::kError;
  file_ = nullptr;
  return status_;
}
#endif  // PDF_ENABLE_XFA_TIFF

bool ProgressiveDecoder::DetectImageType(FXCODEC_IMAGE_TYPE imageType,
                                         CFX_DIBAttribute* pAttribute) {
#ifdef PDF_ENABLE_XFA_TIFF
  if (imageType == FXCODEC_IMAGE_TIFF) {
    return TiffDetectImageTypeFromFile(pAttribute);
  }
#endif  // PDF_ENABLE_XFA_TIFF

  size_t size = pdfium::checked_cast<size_t>(
      std::min<FX_FILESIZE>(file_->GetSize(), kBlockSize));
  codec_memory_ = pdfium::MakeRetain<CFX_CodecMemory>(size);
  offset_ = 0;
  if (!file_->ReadBlockAtOffset(codec_memory_->GetBufferSpan().first(size),
                                offset_)) {
    status_ = FXCODEC_STATUS::kError;
    return false;
  }
  offset_ += size;

  if (imageType == FXCODEC_IMAGE_JPG) {
    return JpegDetectImageTypeInBuffer(pAttribute);
  }

#ifdef PDF_ENABLE_XFA_BMP
  if (imageType == FXCODEC_IMAGE_BMP) {
    return BmpDetectImageTypeInBuffer(pAttribute);
  }
#endif  // PDF_ENABLE_XFA_BMP

#ifdef PDF_ENABLE_XFA_GIF
  if (imageType == FXCODEC_IMAGE_GIF) {
    return GifDetectImageTypeInBuffer();
  }
#endif  // PDF_ENABLE_XFA_GIF

#ifdef PDF_ENABLE_XFA_PNG
  if (imageType == FXCODEC_IMAGE_PNG) {
    return PngDetectImageTypeInBuffer(pAttribute);
  }
#endif  // PDF_ENABLE_XFA_PNG

  status_ = FXCODEC_STATUS::kError;
  return false;
}

bool ProgressiveDecoder::ReadMoreData(
    ProgressiveDecoderIface* pModule,
    ProgressiveDecoderIface::Context* pContext,
    FXCODEC_STATUS* err_status) {
  // Check for EOF.
  if (offset_ >= static_cast<uint32_t>(file_->GetSize())) {
    return false;
  }

  // Try to get whatever remains.
  uint32_t dwBytesToFetchFromFile =
      pdfium::checked_cast<uint32_t>(file_->GetSize() - offset_);

  // Figure out if the codec stopped processing midway through the buffer.
  size_t dwUnconsumed;
  FX_SAFE_SIZE_T avail_input = pModule->GetAvailInput(pContext);
  if (!avail_input.AssignIfValid(&dwUnconsumed)) {
    return false;
  }

  if (dwUnconsumed == codec_memory_->GetSize()) {
    // Codec couldn't make any progress against the bytes in the buffer.
    // Increase the buffer size so that there might be enough contiguous
    // bytes to allow whatever operation is having difficulty to succeed.
    dwBytesToFetchFromFile =
        std::min<uint32_t>(dwBytesToFetchFromFile, kBlockSize);
    size_t dwNewSize = codec_memory_->GetSize() + dwBytesToFetchFromFile;
    if (!codec_memory_->TryResize(dwNewSize)) {
      *err_status = FXCODEC_STATUS::kError;
      return false;
    }
  } else {
    // TODO(crbug.com/pdfium/1904): Simplify the `CFX_CodecMemory` API so we
    // don't need to do this awkward dance to free up exactly enough buffer
    // space for the next read.
    size_t dwConsumable = codec_memory_->GetSize() - dwUnconsumed;
    dwBytesToFetchFromFile = pdfium::checked_cast<uint32_t>(
        std::min<size_t>(dwBytesToFetchFromFile, dwConsumable));
    codec_memory_->Consume(dwBytesToFetchFromFile);
    codec_memory_->Seek(dwConsumable - dwBytesToFetchFromFile);
    dwUnconsumed += codec_memory_->GetPosition();
  }

  // Append new data past the bytes not yet processed by the codec.
  if (!file_->ReadBlockAtOffset(codec_memory_->GetBufferSpan().subspan(
                                    dwUnconsumed, dwBytesToFetchFromFile),
                                offset_)) {
    *err_status = FXCODEC_STATUS::kError;
    return false;
  }
  offset_ += dwBytesToFetchFromFile;
  return pModule->Input(pContext, codec_memory_);
}

FXCODEC_STATUS ProgressiveDecoder::LoadImageInfo(
    RetainPtr<IFX_SeekableReadStream> pFile,
    FXCODEC_IMAGE_TYPE imageType,
    CFX_DIBAttribute* pAttribute,
    bool bSkipImageTypeCheck) {
  DCHECK(pAttribute);

  switch (status_) {
    case FXCODEC_STATUS::kFrameReady:
    case FXCODEC_STATUS::kFrameToBeContinued:
    case FXCODEC_STATUS::kDecodeReady:
    case FXCODEC_STATUS::kDecodeToBeContinued:
      return FXCODEC_STATUS::kError;
    case FXCODEC_STATUS::kError:
    case FXCODEC_STATUS::kDecodeFinished:
      break;
  }
  file_ = std::move(pFile);
  if (!file_) {
    status_ = FXCODEC_STATUS::kError;
    return status_;
  }
  offset_ = 0;
  src_width_ = 0;
  src_height_ = 0;
  src_components_ = 0;
  src_bpc_ = 0;
  src_pass_number_ = 0;
  if (imageType != FXCODEC_IMAGE_UNKNOWN &&
      DetectImageType(imageType, pAttribute)) {
    image_type_ = imageType;
    status_ = FXCODEC_STATUS::kFrameReady;
    return status_;
  }
  // If we got here then the image data does not match the requested decoder.
  // If we're skipping the type check then bail out at this point and return
  // the failed status.
  if (bSkipImageTypeCheck) {
    return status_;
  }

  for (int type = FXCODEC_IMAGE_UNKNOWN + 1; type < FXCODEC_IMAGE_MAX; type++) {
    if (DetectImageType(static_cast<FXCODEC_IMAGE_TYPE>(type), pAttribute)) {
      image_type_ = static_cast<FXCODEC_IMAGE_TYPE>(type);
      status_ = FXCODEC_STATUS::kFrameReady;
      return status_;
    }
  }
  status_ = FXCODEC_STATUS::kError;
  file_ = nullptr;
  return status_;
}

void ProgressiveDecoder::SetTransMethod() {
  switch (device_bitmap_->GetFormat()) {
    case FXDIB_Format::kInvalid:
    case FXDIB_Format::k1bppMask:
    case FXDIB_Format::k1bppRgb:
    case FXDIB_Format::k8bppMask:
    case FXDIB_Format::k8bppRgb:
      NOTREACHED();
    case FXDIB_Format::kBgr: {
      switch (src_format_) {
        case FXCodec_Invalid:
          trans_method_ = TransformMethod::kInvalid;
          break;
        case FXCodec_8bppGray:
          trans_method_ = TransformMethod::k8BppGrayToRgbMaybeAlpha;
          break;
        case FXCodec_8bppRgb:
          trans_method_ = TransformMethod::k8BppRgbToRgbNoAlpha;
          break;
        case FXCodec_Rgb:
        case FXCodec_Rgb32:
        case FXCodec_Argb:
          trans_method_ = TransformMethod::kRgbMaybeAlphaToRgbMaybeAlpha;
          break;
        case FXCodec_Cmyk:
          trans_method_ = TransformMethod::kCmykToRgbMaybeAlpha;
          break;
      }
      break;
    }
    case FXDIB_Format::kBgrx:
    case FXDIB_Format::kBgra: {
      switch (src_format_) {
        case FXCodec_Invalid:
          trans_method_ = TransformMethod::kInvalid;
          break;
        case FXCodec_8bppGray:
          trans_method_ = TransformMethod::k8BppGrayToRgbMaybeAlpha;
          break;
        case FXCodec_8bppRgb:
          if (device_bitmap_->GetFormat() == FXDIB_Format::kBgra) {
            trans_method_ = TransformMethod::k8BppRgbToArgb;
          } else {
            trans_method_ = TransformMethod::k8BppRgbToRgbNoAlpha;
          }
          break;
        case FXCodec_Rgb:
        case FXCodec_Rgb32:
          trans_method_ = TransformMethod::kRgbMaybeAlphaToRgbMaybeAlpha;
          break;
        case FXCodec_Cmyk:
          trans_method_ = TransformMethod::kCmykToRgbMaybeAlpha;
          break;
        case FXCodec_Argb:
          trans_method_ = TransformMethod::kArgbToArgb;
          break;
      }
      break;
    }
#if defined(PDF_USE_SKIA)
    case FXDIB_Format::kBgraPremul:
      // TODO(crbug.com/355630556): Consider adding support for
      // `FXDIB_Format::kBgraPremul`
      NOTREACHED();
#endif
  }
}

void ProgressiveDecoder::ResampleScanline(
    const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
    int dest_line,
    pdfium::span<uint8_t> src_span,
    FXCodec_Format src_format) {
  uint8_t* src_scan = src_span.data();
  uint8_t* dest_scan = pDeviceBitmap->GetWritableScanline(dest_line).data();
  const int src_bytes_per_pixel = (src_format & 0xff) / 8;
  const int dest_bytes_per_pixel = pDeviceBitmap->GetBPP() / 8;
  for (int dest_col = 0; dest_col < src_width_; dest_col++) {
    CStretchEngine::PixelWeight* pPixelWeights =
        weight_horz_.GetPixelWeight(dest_col);
    switch (trans_method_) {
      case TransformMethod::kInvalid:
        return;
      case TransformMethod::k8BppGrayToRgbMaybeAlpha: {
        UNSAFE_TODO({
          uint32_t dest_g = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
            dest_g += pixel_weight * src_scan[j];
          }
          FXSYS_memset(dest_scan, CStretchEngine::PixelFromFixed(dest_g), 3);
          dest_scan += dest_bytes_per_pixel;
          break;
        });
      }
      case TransformMethod::k8BppRgbToRgbNoAlpha: {
        UNSAFE_TODO({
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
            uint32_t argb = src_palette_[src_scan[j]];
            dest_r += pixel_weight * FXARGB_R(argb);
            dest_g += pixel_weight * FXARGB_G(argb);
            dest_b += pixel_weight * FXARGB_B(argb);
          }
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
          dest_scan += dest_bytes_per_pixel - 3;
          break;
        });
      }
      case TransformMethod::k8BppRgbToArgb: {
#ifdef PDF_ENABLE_XFA_BMP
        if (bmp_context_) {
          UNSAFE_TODO({
            uint32_t dest_r = 0;
            uint32_t dest_g = 0;
            uint32_t dest_b = 0;
            for (int j = pPixelWeights->src_start_;
                 j <= pPixelWeights->src_end_; j++) {
              uint32_t pixel_weight =
                  pPixelWeights->weights_[j - pPixelWeights->src_start_];
              uint32_t argb = src_palette_[src_scan[j]];
              dest_r += pixel_weight * FXARGB_R(argb);
              dest_g += pixel_weight * FXARGB_G(argb);
              dest_b += pixel_weight * FXARGB_B(argb);
            }
            *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
            *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
            *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
            *dest_scan++ = 0xFF;
            break;
          });
        }
#endif  // PDF_ENABLE_XFA_BMP
        UNSAFE_TODO({
          uint32_t dest_a = 0;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
            FX_ARGB argb = src_palette_[src_scan[j]];
            dest_a += pixel_weight * FXARGB_A(argb);
            dest_r += pixel_weight * FXARGB_R(argb);
            dest_g += pixel_weight * FXARGB_G(argb);
            dest_b += pixel_weight * FXARGB_B(argb);
          }
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_a);
          break;
        });
      }
      case TransformMethod::kRgbMaybeAlphaToRgbMaybeAlpha: {
        UNSAFE_TODO({
          uint32_t dest_b = 0;
          uint32_t dest_g = 0;
          uint32_t dest_r = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
            const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
            dest_b += pixel_weight * (*src_pixel++);
            dest_g += pixel_weight * (*src_pixel++);
            dest_r += pixel_weight * (*src_pixel);
          }
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
          dest_scan += dest_bytes_per_pixel - 3;
          break;
        });
      }
      case TransformMethod::kCmykToRgbMaybeAlpha: {
        UNSAFE_TODO({
          uint32_t dest_b = 0;
          uint32_t dest_g = 0;
          uint32_t dest_r = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
            const uint8_t* src_pixel = src_scan + j * src_bytes_per_pixel;
            FX_RGB_STRUCT<uint8_t> src_rgb =
                AdobeCMYK_to_sRGB1(255 - src_pixel[0], 255 - src_pixel[1],
                                   255 - src_pixel[2], 255 - src_pixel[3]);
            dest_b += pixel_weight * src_rgb.blue;
            dest_g += pixel_weight * src_rgb.green;
            dest_r += pixel_weight * src_rgb.red;
          }
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_b);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_g);
          *dest_scan++ = CStretchEngine::PixelFromFixed(dest_r);
          dest_scan += dest_bytes_per_pixel - 3;
          break;
        });
      }
      case TransformMethod::kArgbToArgb: {
        UNSAFE_TODO({
          uint32_t dest_alpha = 0;
          uint32_t dest_r = 0;
          uint32_t dest_g = 0;
          uint32_t dest_b = 0;
          for (int j = pPixelWeights->src_start_; j <= pPixelWeights->src_end_;
               j++) {
            uint32_t pixel_weight =
                pPixelWeights->weights_[j - pPixelWeights->src_start_];
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
          break;
        });
      }
    }
  }
}

void ProgressiveDecoder::Resample(const RetainPtr<CFX_DIBitmap>& pDeviceBitmap,
                                  int32_t src_line,
                                  uint8_t* src_scan,
                                  FXCodec_Format src_format) {
  if (src_line < 0 || src_line >= src_height_) {
    return;
  }

  ResampleScanline(pDeviceBitmap, src_line, decode_buf_, src_format);
}

FXDIB_Format ProgressiveDecoder::GetBitmapFormat() const {
  switch (image_type_) {
    case FXCODEC_IMAGE_JPG:
#ifdef PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_BMP:
#endif  // PDF_ENABLE_XFA_BMP
      return GetBitsPerPixel() <= 24 ? FXDIB_Format::kBgr : FXDIB_Format::kBgrx;
#ifdef PDF_ENABLE_XFA_PNG
    case FXCODEC_IMAGE_PNG:
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
    case FXCODEC_IMAGE_TIFF:
#endif  // PDF_ENABLE_XFA_TIFF
    default:
      // TODO(crbug.com/355630556): Consider adding support for
      // `FXDIB_Format::kBgraPremul`
      return FXDIB_Format::kBgra;
  }
}

std::pair<FXCODEC_STATUS, size_t> ProgressiveDecoder::GetFrames() {
  if (!(status_ == FXCODEC_STATUS::kFrameReady ||
        status_ == FXCODEC_STATUS::kFrameToBeContinued)) {
    return {FXCODEC_STATUS::kError, 0};
  }

  switch (image_type_) {
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
      frame_number_ = 1;
      status_ = FXCODEC_STATUS::kDecodeReady;
      return {status_, 1};
#ifdef PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_GIF: {
      while (true) {
        GifDecoder::Status readResult;
        std::tie(readResult, frame_number_) =
            GifDecoder::LoadFrameInfo(gif_context_.get());
        while (readResult == GifDecoder::Status::kUnfinished) {
          FXCODEC_STATUS error_status = FXCODEC_STATUS::kError;
          if (!GifReadMoreData(&error_status)) {
            return {error_status, 0};
          }

          std::tie(readResult, frame_number_) =
              GifDecoder::LoadFrameInfo(gif_context_.get());
        }
        if (readResult == GifDecoder::Status::kSuccess) {
          status_ = FXCODEC_STATUS::kDecodeReady;
          return {status_, frame_number_};
        }
        gif_context_ = nullptr;
        status_ = FXCODEC_STATUS::kError;
        return {status_, 0};
      }
    }
#endif  // PDF_ENABLE_XFA_GIF
    default:
      return {FXCODEC_STATUS::kError, 0};
  }
}

FXCODEC_STATUS ProgressiveDecoder::StartDecode(RetainPtr<CFX_DIBitmap> bitmap) {
  CHECK(bitmap);
  CHECK_EQ(bitmap->GetWidth(), src_width_);
  CHECK_EQ(bitmap->GetHeight(), src_height_);
  CHECK_GT(src_width_, 0);
  CHECK_GT(src_height_, 0);

  const FXDIB_Format format = bitmap->GetFormat();
  CHECK(format == FXDIB_Format::kBgra || format == FXDIB_Format::kBgr ||
        format == FXDIB_Format::kBgrx);

  if (status_ != FXCODEC_STATUS::kDecodeReady) {
    return FXCODEC_STATUS::kError;
  }

  if (frame_number_ == 0) {
    return FXCODEC_STATUS::kError;
  }

  if (bitmap->GetWidth() > 65535 || bitmap->GetHeight() > 65535) {
    return FXCODEC_STATUS::kError;
  }

  frame_cur_ = 0;
  device_bitmap_ = std::move(bitmap);
  switch (image_type_) {
#ifdef PDF_ENABLE_XFA_BMP
    case FXCODEC_IMAGE_BMP:
      return BmpStartDecode();
#endif  // PDF_ENABLE_XFA_BMP
#ifdef PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_GIF:
      return GifStartDecode();
#endif  // PDF_ENABLE_XFA_GIF
    case FXCODEC_IMAGE_JPG:
      return JpegStartDecode();
#ifdef PDF_ENABLE_XFA_PNG
    case FXCODEC_IMAGE_PNG:
      return PngStartDecode();
#endif  // PDF_ENABLE_XFA_PNG
#ifdef PDF_ENABLE_XFA_TIFF
    case FXCODEC_IMAGE_TIFF:
      status_ = FXCODEC_STATUS::kDecodeToBeContinued;
      return status_;
#endif  // PDF_ENABLE_XFA_TIFF
    default:
      return FXCODEC_STATUS::kError;
  }
}

FXCODEC_STATUS ProgressiveDecoder::ContinueDecode() {
  if (status_ != FXCODEC_STATUS::kDecodeToBeContinued) {
    return FXCODEC_STATUS::kError;
  }

  switch (image_type_) {
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
