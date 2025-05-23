// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/cfx_gifcontext.h"

#include <stdint.h>
#include <string.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>

#include "core/fxcodec/cfx_codec_memory.h"
#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/data_vector.h"

namespace fxcodec {

namespace {

constexpr std::array<const int32_t, 4> kGifInterlaceStep = {{8, 8, 4, 2}};

}  // namespace

CFX_GifContext::CFX_GifContext(GifDecoder::Delegate* delegate)
    : delegate_(delegate) {}

CFX_GifContext::~CFX_GifContext() = default;

void CFX_GifContext::ReadScanline(int32_t row_num,
                                  pdfium::span<uint8_t> row_buf) {
  delegate_->GifReadScanline(row_num, row_buf);
}

bool CFX_GifContext::GetRecordPosition(uint32_t cur_pos,
                                       int32_t left,
                                       int32_t top,
                                       int32_t width,
                                       int32_t height,
                                       pdfium::span<CFX_GifPalette> pal,
                                       int32_t trans_index,
                                       bool interlace) {
  return delegate_->GifInputRecordPositionBuf(
      cur_pos, FX_RECT(left, top, left + width, top + height), pal, trans_index,
      interlace);
}

GifDecoder::Status CFX_GifContext::ReadHeader() {
  GifDecoder::Status status = ReadGifSignature();
  if (status != GifDecoder::Status::kSuccess) {
    return status;
  }
  return ReadLogicalScreenDescriptor();
}

GifDecoder::Status CFX_GifContext::GetFrame() {
  GifDecoder::Status ret = GifDecoder::Status::kSuccess;
  while (true) {
    switch (decode_status_) {
      case GIF_D_STATUS_TAIL:
        return GifDecoder::Status::kSuccess;
      case GIF_D_STATUS_SIG: {
        uint8_t signature;
        if (!ReadAllOrNone(pdfium::byte_span_from_ref(signature))) {
          return GifDecoder::Status::kUnfinished;
        }
        switch (signature) {
          case GIF_SIG_EXTENSION:
            SaveDecodingStatus(GIF_D_STATUS_EXT);
            continue;
          case GIF_SIG_IMAGE:
            SaveDecodingStatus(GIF_D_STATUS_IMG_INFO);
            continue;
          case GIF_SIG_TRAILER:
            SaveDecodingStatus(GIF_D_STATUS_TAIL);
            return GifDecoder::Status::kSuccess;
          default:
            if (!input_buffer_->IsEOF()) {
              // The Gif File has non_standard Tag!
              SaveDecodingStatus(GIF_D_STATUS_SIG);
              continue;
            }
            // The Gif File Doesn't have Trailer Tag!
            return GifDecoder::Status::kSuccess;
        }
      }
      case GIF_D_STATUS_EXT: {
        uint8_t extension;
        if (!ReadAllOrNone(pdfium::byte_span_from_ref(extension))) {
          return GifDecoder::Status::kUnfinished;
        }
        switch (extension) {
          case GIF_BLOCK_CE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_CE);
            continue;
          case GIF_BLOCK_GCE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_GCE);
            continue;
          case GIF_BLOCK_PTE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_PTE);
            continue;
          default: {
            int32_t status = GIF_D_STATUS_EXT_UNE;
            if (extension == GIF_BLOCK_PTE) {
              status = GIF_D_STATUS_EXT_PTE;
            }
            SaveDecodingStatus(status);
            continue;
          }
        }
      }
      case GIF_D_STATUS_IMG_INFO: {
        ret = DecodeImageInfo();
        if (ret != GifDecoder::Status::kSuccess) {
          return ret;
        }

        continue;
      }
      case GIF_D_STATUS_IMG_DATA: {
        uint8_t img_data_size;
        size_t read_marker = input_buffer_->GetPosition();
        if (!ReadAllOrNone(pdfium::byte_span_from_ref(img_data_size))) {
          return GifDecoder::Status::kUnfinished;
        }
        while (img_data_size != GIF_BLOCK_TERMINAL) {
          if (!input_buffer_->Seek(input_buffer_->GetPosition() +
                                   img_data_size)) {
            input_buffer_->Seek(read_marker);
            return GifDecoder::Status::kUnfinished;
          }

          // This saving of the scan state on partial reads is why
          // ScanForTerminalMarker() cannot be used here.
          SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
          read_marker = input_buffer_->GetPosition();
          if (!ReadAllOrNone(pdfium::byte_span_from_ref(img_data_size))) {
            return GifDecoder::Status::kUnfinished;
          }
        }
        SaveDecodingStatus(GIF_D_STATUS_SIG);
        continue;
      }
      default: {
        ret = DecodeExtension();
        if (ret != GifDecoder::Status::kSuccess) {
          return ret;
        }
        break;
      }
    }
  }
}

GifDecoder::Status CFX_GifContext::LoadFrame(size_t frame_num) {
  if (frame_num >= images_.size()) {
    return GifDecoder::Status::kError;
  }

  CFX_GifImage* gif_image = images_[frame_num].get();
  if (gif_image->image_info.height == 0) {
    return GifDecoder::Status::kError;
  }

  uint32_t gif_img_row_bytes = gif_image->image_info.width;
  if (gif_img_row_bytes == 0) {
    return GifDecoder::Status::kError;
  }

  if (decode_status_ == GIF_D_STATUS_TAIL) {
    gif_image->row_buffer.resize(gif_img_row_bytes);
    CFX_GifGraphicControlExtension* gif_img_gce = gif_image->image_GCE.get();
    pdfium::span<CFX_GifPalette> pLocalPalette = gif_image->local_palettes;
    if (!gif_img_gce) {
      bool bRes = GetRecordPosition(
          gif_image->data_pos, gif_image->image_info.left,
          gif_image->image_info.top, gif_image->image_info.width,
          gif_image->image_info.height, pLocalPalette, -1,
          gif_image->image_info.local_flags.interlace);
      if (!bRes) {
        gif_image->row_buffer.clear();
        return GifDecoder::Status::kError;
      }
    } else {
      bool bRes = GetRecordPosition(
          gif_image->data_pos, gif_image->image_info.left,
          gif_image->image_info.top, gif_image->image_info.width,
          gif_image->image_info.height, pLocalPalette,
          gif_image->image_GCE->gce_flags.transparency
              ? static_cast<int32_t>(gif_image->image_GCE->trans_index)
              : -1,
          gif_image->image_info.local_flags.interlace);
      if (!bRes) {
        gif_image->row_buffer.clear();
        return GifDecoder::Status::kError;
      }
    }

    if (gif_image->code_exp > GIF_MAX_LZW_EXP) {
      gif_image->row_buffer.clear();
      return GifDecoder::Status::kError;
    }

    img_row_offset_ = 0;
    img_row_avail_size_ = 0;
    img_pass_num_ = 0;
    gif_image->row_num = 0;
    SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  }

  uint8_t img_data_size;
  DataVector<uint8_t> img_data;
  size_t read_marker = input_buffer_->GetPosition();

  // TODO(crbug.com/pdfium/1793): This logic can be simplified a lot, but it
  // probably makes more sense to switch to a different GIF decoder altogether.
  if (decode_status_ == GIF_D_STATUS_IMG_DATA) {
    if (!ReadAllOrNone(pdfium::byte_span_from_ref(img_data_size))) {
      return GifDecoder::Status::kUnfinished;
    }
    if (img_data_size != GIF_BLOCK_TERMINAL) {
      img_data.resize(img_data_size);
      if (!ReadAllOrNone(img_data)) {
        input_buffer_->Seek(read_marker);
        return GifDecoder::Status::kUnfinished;
      }
      if (!lzw_decompressor_) {
        lzw_decompressor_ = LZWDecompressor::Create(GetPaletteExp(gif_image),
                                                    gif_image->code_exp);
        if (!lzw_decompressor_) {
          DecodingFailureAtTailCleanup(gif_image);
          return GifDecoder::Status::kError;
        }
      }
      lzw_decompressor_->SetSource(img_data);

      SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
      img_row_offset_ += img_row_avail_size_;
      img_row_avail_size_ = gif_img_row_bytes - img_row_offset_;
      auto img_row_span = pdfium::span(gif_image->row_buffer)
                              .subspan(img_row_offset_, img_row_avail_size_);
      LZWDecompressor::Status ret = UNSAFE_TODO(
          lzw_decompressor_->Decode(img_row_span.data(), &img_row_avail_size_));
      if (ret == LZWDecompressor::Status::kError) {
        DecodingFailureAtTailCleanup(gif_image);
        return GifDecoder::Status::kError;
      }

      while (ret != LZWDecompressor::Status::kError) {
        if (ret == LZWDecompressor::Status::kSuccess) {
          ReadScanline(gif_image->row_num, gif_image->row_buffer);
          gif_image->row_buffer.clear();
          SaveDecodingStatus(GIF_D_STATUS_TAIL);
          return GifDecoder::Status::kSuccess;
        }

        if (ret == LZWDecompressor::Status::kUnfinished) {
          read_marker = input_buffer_->GetPosition();
          if (!ReadAllOrNone(pdfium::byte_span_from_ref(img_data_size))) {
            return GifDecoder::Status::kUnfinished;
          }
          if (img_data_size != GIF_BLOCK_TERMINAL) {
            img_data.resize(img_data_size);
            if (!ReadAllOrNone(img_data)) {
              input_buffer_->Seek(read_marker);
              return GifDecoder::Status::kUnfinished;
            }
            lzw_decompressor_->SetSource(img_data);

            SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
            img_row_offset_ += img_row_avail_size_;
            img_row_avail_size_ = gif_img_row_bytes - img_row_offset_;
            img_row_span = pdfium::span(gif_image->row_buffer)
                               .subspan(img_row_offset_, img_row_avail_size_);
            ret = UNSAFE_TODO(lzw_decompressor_->Decode(img_row_span.data(),
                                                        &img_row_avail_size_));
          }
        }

        if (ret == LZWDecompressor::Status::kInsufficientDestSize) {
          if (gif_image->image_info.local_flags.interlace) {
            ReadScanline(gif_image->row_num, gif_image->row_buffer);
            gif_image->row_num += kGifInterlaceStep[img_pass_num_];
            if (gif_image->row_num >=
                static_cast<int32_t>(gif_image->image_info.height)) {
              img_pass_num_++;
              if (img_pass_num_ == std::size(kGifInterlaceStep)) {
                DecodingFailureAtTailCleanup(gif_image);
                return GifDecoder::Status::kError;
              }
              gif_image->row_num = kGifInterlaceStep[img_pass_num_] / 2;
            }
          } else {
            ReadScanline(gif_image->row_num++, gif_image->row_buffer);
          }

          img_row_offset_ = 0;
          img_row_avail_size_ = gif_img_row_bytes;
          img_row_span = pdfium::span(gif_image->row_buffer)
                             .subspan(img_row_offset_, img_row_avail_size_);
          ret = UNSAFE_TODO(lzw_decompressor_->Decode(img_row_span.data(),
                                                      &img_row_avail_size_));
        }

        if (ret == LZWDecompressor::Status::kError) {
          DecodingFailureAtTailCleanup(gif_image);
          return GifDecoder::Status::kError;
        }
      }
    }
    SaveDecodingStatus(GIF_D_STATUS_TAIL);
  }
  return GifDecoder::Status::kError;
}

void CFX_GifContext::SetInputBuffer(RetainPtr<CFX_CodecMemory> codec_memory) {
  input_buffer_ = std::move(codec_memory);
}

uint32_t CFX_GifContext::GetAvailInput() const {
  if (!input_buffer_) {
    return 0;
  }

  return pdfium::checked_cast<uint32_t>(input_buffer_->GetSize() -
                                        input_buffer_->GetPosition());
}

bool CFX_GifContext::ReadAllOrNone(pdfium::span<uint8_t> dest) {
  if (!input_buffer_ || dest.empty()) {
    return false;
  }
  size_t read_marker = input_buffer_->GetPosition();
  size_t read = input_buffer_->ReadBlock(dest);
  if (read < dest.size()) {
    input_buffer_->Seek(read_marker);
    return false;
  }
  return true;
}

GifDecoder::Status CFX_GifContext::ReadGifSignature() {
  CFX_GifHeader header;
  if (!ReadAllOrNone(pdfium::byte_span_from_ref(header))) {
    return GifDecoder::Status::kUnfinished;
  }
  if (UNSAFE_TODO(strncmp(header.signature, kGifSignature87, 6)) != 0 &&
      UNSAFE_TODO(strncmp(header.signature, kGifSignature89, 6)) != 0) {
    return GifDecoder::Status::kError;
  }
  return GifDecoder::Status::kSuccess;
}

GifDecoder::Status CFX_GifContext::ReadLogicalScreenDescriptor() {
  CFX_GifLocalScreenDescriptor lsd;
  size_t read_marker = input_buffer_->GetPosition();
  if (!ReadAllOrNone(pdfium::byte_span_from_ref(lsd))) {
    return GifDecoder::Status::kUnfinished;
  }
  if (lsd.global_flags.global_pal) {
    uint32_t palette_count = unsigned(2 << lsd.global_flags.pal_bits);
    if (lsd.bc_index >= palette_count) {
      return GifDecoder::Status::kError;
    }
    bc_index_ = lsd.bc_index;

    std::vector<CFX_GifPalette> palette(palette_count);
    if (!ReadAllOrNone(pdfium::as_writable_byte_span(palette))) {
      // Roll back the read for the LSD
      input_buffer_->Seek(read_marker);
      return GifDecoder::Status::kUnfinished;
    }

    global_palette_exp_ = lsd.global_flags.pal_bits;
    global_sort_flag_ = lsd.global_flags.sort_flag;
    global_color_resolution_ = lsd.global_flags.color_resolution;
    std::swap(global_palette_, palette);
  }

  width_ = fxcrt::FromLE16(lsd.width);
  height_ = fxcrt::FromLE16(lsd.height);

  return GifDecoder::Status::kSuccess;
}

void CFX_GifContext::SaveDecodingStatus(int32_t status) {
  decode_status_ = status;
}

GifDecoder::Status CFX_GifContext::DecodeExtension() {
  size_t read_marker = input_buffer_->GetPosition();

  switch (decode_status_) {
    case GIF_D_STATUS_EXT_CE: {
      if (!ScanForTerminalMarker()) {
        input_buffer_->Seek(read_marker);
        return GifDecoder::Status::kUnfinished;
      }
      break;
    }
    case GIF_D_STATUS_EXT_PTE: {
      CFX_GifPlainTextExtension gif_pte;
      if (!ReadAllOrNone(pdfium::byte_span_from_ref(gif_pte))) {
        return GifDecoder::Status::kUnfinished;
      }
      graphic_control_extension_ = nullptr;
      if (!ScanForTerminalMarker()) {
        input_buffer_->Seek(read_marker);
        return GifDecoder::Status::kUnfinished;
      }
      break;
    }
    case GIF_D_STATUS_EXT_GCE: {
      CFX_GifGraphicControlExtension gif_gce;
      if (!ReadAllOrNone(pdfium::byte_span_from_ref(gif_gce))) {
        return GifDecoder::Status::kUnfinished;
      }
      if (!graphic_control_extension_.get()) {
        graphic_control_extension_ =
            std::make_unique<CFX_GifGraphicControlExtension>();
      }
      graphic_control_extension_->block_size = gif_gce.block_size;
      graphic_control_extension_->gce_flags = gif_gce.gce_flags;
      graphic_control_extension_->delay_time =
          fxcrt::FromLE16(gif_gce.delay_time);
      graphic_control_extension_->trans_index = gif_gce.trans_index;
      break;
    }
    default: {
      if (decode_status_ == GIF_D_STATUS_EXT_PTE) {
        graphic_control_extension_ = nullptr;
      }
      if (!ScanForTerminalMarker()) {
        input_buffer_->Seek(read_marker);
        return GifDecoder::Status::kUnfinished;
      }
    }
  }

  SaveDecodingStatus(GIF_D_STATUS_SIG);
  return GifDecoder::Status::kSuccess;
}

GifDecoder::Status CFX_GifContext::DecodeImageInfo() {
  if (width_ <= 0 || height_ <= 0) {
    return GifDecoder::Status::kError;
  }

  size_t read_marker = input_buffer_->GetPosition();
  CFX_GifImageInfo img_info;
  if (!ReadAllOrNone(pdfium::byte_span_from_ref(img_info))) {
    return GifDecoder::Status::kUnfinished;
  }
  auto gif_image = std::make_unique<CFX_GifImage>();
  gif_image->image_info.left = fxcrt::FromLE16(img_info.left);
  gif_image->image_info.top = fxcrt::FromLE16(img_info.top);
  gif_image->image_info.width = fxcrt::FromLE16(img_info.width);
  gif_image->image_info.height = fxcrt::FromLE16(img_info.height);
  gif_image->image_info.local_flags = img_info.local_flags;
  if (gif_image->image_info.left + gif_image->image_info.width > width_ ||
      gif_image->image_info.top + gif_image->image_info.height > height_) {
    return GifDecoder::Status::kError;
  }

  CFX_GifLocalFlags* gif_img_info_lf = &img_info.local_flags;
  if (gif_img_info_lf->local_pal) {
    gif_image->local_palette_exp = gif_img_info_lf->pal_bits;
    uint32_t loc_pal_count = unsigned(2 << gif_img_info_lf->pal_bits);
    std::vector<CFX_GifPalette> loc_pal(loc_pal_count);
    if (!ReadAllOrNone(pdfium::as_writable_byte_span(loc_pal))) {
      input_buffer_->Seek(read_marker);
      return GifDecoder::Status::kUnfinished;
    }
    gif_image->local_palettes = std::move(loc_pal);
  }

  uint8_t code_size;
  if (!ReadAllOrNone(pdfium::span_from_ref(code_size))) {
    input_buffer_->Seek(read_marker);
    return GifDecoder::Status::kUnfinished;
  }

  gif_image->code_exp = code_size;
  gif_image->data_pos = delegate_->GifCurrentPosition();
  gif_image->image_GCE = nullptr;
  if (graphic_control_extension_.get()) {
    if (graphic_control_extension_->gce_flags.transparency) {
      // Need to test that the color that is going to be transparent is actually
      // in the palette being used.
      if (graphic_control_extension_->trans_index >=
          (2 << GetPaletteExp(gif_image.get()))) {
        return GifDecoder::Status::kError;
      }
    }
    gif_image->image_GCE = std::move(graphic_control_extension_);
    graphic_control_extension_ = nullptr;
  }

  images_.push_back(std::move(gif_image));
  SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  return GifDecoder::Status::kSuccess;
}

void CFX_GifContext::DecodingFailureAtTailCleanup(CFX_GifImage* gif_image) {
  gif_image->row_buffer.clear();
  SaveDecodingStatus(GIF_D_STATUS_TAIL);
}

bool CFX_GifContext::ScanForTerminalMarker() {
  uint8_t data_size;
  if (!ReadAllOrNone(pdfium::span_from_ref(data_size))) {
    return false;
  }
  while (data_size != GIF_BLOCK_TERMINAL) {
    if (!input_buffer_->Seek(input_buffer_->GetPosition() + data_size) ||
        !ReadAllOrNone(pdfium::span_from_ref(data_size))) {
      return false;
    }
  }
  return true;
}

uint8_t CFX_GifContext::GetPaletteExp(CFX_GifImage* gif_image) const {
  return !gif_image->local_palettes.empty() ? gif_image->local_palette_exp
                                            : global_palette_exp_;
}

}  // namespace fxcodec
