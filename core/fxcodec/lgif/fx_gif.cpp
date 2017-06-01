// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/fx_gif.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcodec/lgif/cgifcontext.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

static_assert(sizeof(GifImageInfo) == 9,
              "GifImageInfo should have a size of 9");
static_assert(sizeof(GifPalette) == 3, "GifPalette should have a size of 3");
static_assert(sizeof(GifPTE) == 13, "GifPTE should have a size of 13");
static_assert(sizeof(GifGCE) == 5, "GifGCE should have a size of 5");
static_assert(sizeof(GifHeader) == 6, "GifHeader should have a size of 6");
static_assert(sizeof(GifLSD) == 7, "GifLSD should have a size of 7");

namespace {

uint8_t* gif_read_data(CGifContext* context,
                       uint8_t** des_buf_pp,
                       uint32_t data_size) {
  if (!context || context->avail_in < context->skip_size + data_size)
    return nullptr;

  *des_buf_pp = context->next_in + context->skip_size;
  context->skip_size += data_size;
  return *des_buf_pp;
}

void gif_save_decoding_status(CGifContext* context, int32_t status) {
  context->decode_status = status;
  context->next_in += context->skip_size;
  context->avail_in -= context->skip_size;
  context->skip_size = 0;
}

GifDecodeStatus gif_decode_extension(CGifContext* context) {
  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = context->skip_size;
  switch (context->decode_status) {
    case GIF_D_STATUS_EXT_CE: {
      if (!gif_read_data(context, &data_size_ptr, 1)) {
        context->skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }
      context->cmt_data.clear();
      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        uint8_t data_size = *data_size_ptr;
        if (!gif_read_data(context, &data_ptr, *data_size_ptr) ||
            !gif_read_data(context, &data_size_ptr, 1)) {
          context->skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
        context->cmt_data += CFX_ByteString(data_ptr, data_size);
      }
      break;
    }
    case GIF_D_STATUS_EXT_PTE: {
      GifPTE* gif_pte = nullptr;
      if (!gif_read_data(context, reinterpret_cast<uint8_t**>(&gif_pte), 13))
        return GifDecodeStatus::Unfinished;

      context->m_GifGCE = nullptr;
      if (!gif_read_data(context, &data_size_ptr, 1)) {
        context->skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }
      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        if (!gif_read_data(context, &data_ptr, *data_size_ptr) ||
            !gif_read_data(context, &data_size_ptr, 1)) {
          context->skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
      }
      break;
    }
    case GIF_D_STATUS_EXT_GCE: {
      GifGCE* gif_gce_ptr = nullptr;
      if (!gif_read_data(context, reinterpret_cast<uint8_t**>(&gif_gce_ptr), 6))
        return GifDecodeStatus::Unfinished;

      if (!context->m_GifGCE.get())
        context->m_GifGCE = pdfium::MakeUnique<GifGCE>();
      context->m_GifGCE->block_size = gif_gce_ptr->block_size;
      context->m_GifGCE->gce_flag = gif_gce_ptr->gce_flag;
      context->m_GifGCE->delay_time = GetWord_LSBFirst(
          reinterpret_cast<uint8_t*>(&gif_gce_ptr->delay_time));
      context->m_GifGCE->trans_index = gif_gce_ptr->trans_index;
      break;
    }
    default: {
      if (context->decode_status == GIF_D_STATUS_EXT_PTE)
        context->m_GifGCE = nullptr;
      if (!gif_read_data(context, &data_size_ptr, 1))
        return GifDecodeStatus::Unfinished;

      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        if (!gif_read_data(context, &data_ptr, *data_size_ptr) ||
            !gif_read_data(context, &data_size_ptr, 1)) {
          context->skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
      }
    }
  }
  gif_save_decoding_status(context, GIF_D_STATUS_SIG);
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_decode_image_info(CGifContext* context) {
  if (context->width == 0 || context->height == 0) {
    context->AddError("No Image Header Info");
    return GifDecodeStatus::Error;
  }
  uint32_t skip_size_org = context->skip_size;
  GifImageInfo* gif_img_info_ptr = nullptr;
  if (!gif_read_data(context, reinterpret_cast<uint8_t**>(&gif_img_info_ptr),
                     9))
    return GifDecodeStatus::Unfinished;

  auto gif_image = pdfium::MakeUnique<GifImage>();
  gif_image->m_ImageInfo.left =
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_img_info_ptr->left));
  gif_image->m_ImageInfo.top =
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_img_info_ptr->top));
  gif_image->m_ImageInfo.width =
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_img_info_ptr->width));
  gif_image->m_ImageInfo.height =
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_img_info_ptr->height));
  gif_image->m_ImageInfo.local_flag = gif_img_info_ptr->local_flag;
  if (gif_image->m_ImageInfo.left + gif_image->m_ImageInfo.width >
          context->width ||
      gif_image->m_ImageInfo.top + gif_image->m_ImageInfo.height >
          context->height) {
    context->AddError("Image Data Out Of LSD, The File May Be Corrupt");
    return GifDecodeStatus::Error;
  }
  GifLF* gif_img_info_lf_ptr = (GifLF*)&gif_img_info_ptr->local_flag;
  if (gif_img_info_lf_ptr->local_pal) {
    int32_t loc_pal_size = (2 << gif_img_info_lf_ptr->pal_bits) * 3;
    uint8_t* loc_pal_ptr = nullptr;
    if (!gif_read_data(context, &loc_pal_ptr, loc_pal_size)) {
      context->skip_size = skip_size_org;
      return GifDecodeStatus::Unfinished;
    }
    gif_image->m_LocalPalettes = std::vector<GifPalette>(loc_pal_size / 3);
    std::copy(loc_pal_ptr, loc_pal_ptr + loc_pal_size,
              reinterpret_cast<uint8_t*>(gif_image->m_LocalPalettes.data()));
  }
  uint8_t* code_size_ptr = nullptr;
  if (!gif_read_data(context, &code_size_ptr, 1)) {
    context->skip_size = skip_size_org;
    return GifDecodeStatus::Unfinished;
  }
  gif_image->image_code_size = *code_size_ptr;
  context->RecordCurrentPosition(&gif_image->image_data_pos);
  gif_image->image_data_pos += context->skip_size;
  gif_image->m_ImageGCE = nullptr;
  if (context->m_GifGCE.get()) {
    gif_image->m_ImageGCE = std::move(context->m_GifGCE);
    context->m_GifGCE = nullptr;
  }
  context->m_Images.push_back(std::move(gif_image));
  gif_save_decoding_status(context, GIF_D_STATUS_IMG_DATA);
  return GifDecodeStatus::Success;
}

void gif_decoding_failure_at_tail_cleanup(CGifContext* context,
                                          GifImage* gif_image_ptr) {
  gif_image_ptr->m_ImageRowBuf.clear();
  gif_save_decoding_status(context, GIF_D_STATUS_TAIL);
  context->AddError("Decode Image Data Error");
}

}  // namespace

GifImage::GifImage() {}

GifImage::~GifImage() {}

void CGifLZWDecoder::Input(uint8_t* src_buf, uint32_t src_size) {
  next_in = src_buf;
  avail_in = src_size;
}

uint32_t CGifLZWDecoder::GetAvailInput() {
  return avail_in;
}

CGifLZWDecoder::CGifLZWDecoder(char* error_ptr)
    : code_size(0),
      code_size_cur(0),
      code_clear(0),
      code_end(0),
      code_next(0),
      code_first(0),
      stack_size(0),
      code_old(0),
      next_in(nullptr),
      avail_in(0),
      bits_left(0),
      code_store(0),
      err_msg_ptr(error_ptr) {}

CGifLZWDecoder::~CGifLZWDecoder() {}

void CGifLZWDecoder::InitTable(uint8_t code_len) {
  code_size = code_len;
  ASSERT(code_size < 13);
  code_clear = 1 << code_size;
  code_end = code_clear + 1;
  bits_left = 0;
  code_store = 0;
  next_in = nullptr;
  avail_in = 0;
  stack_size = 0;
  code_first = 0;
  ClearTable();
}

void CGifLZWDecoder::ClearTable() {
  code_size_cur = code_size + 1;
  code_next = code_end + 1;
  code_old = static_cast<uint16_t>(-1);
  memset(code_table, 0, sizeof(tag_Table) * GIF_MAX_LZW_CODE);
  memset(stack, 0, GIF_MAX_LZW_CODE);
  for (uint16_t i = 0; i < code_clear; i++)
    code_table[i].suffix = static_cast<uint8_t>(i);
}

bool CGifLZWDecoder::DecodeString(uint16_t code) {
  stack_size = 0;
  while (code >= code_clear && code <= code_next) {
    if (code == code_table[code].prefix || stack_size == GIF_MAX_LZW_CODE - 1)
      return false;

    stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = code_table[code].suffix;
    code = code_table[code].prefix;
  }
  stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = static_cast<uint8_t>(code);
  code_first = static_cast<uint8_t>(code);
  return true;
}

void CGifLZWDecoder::AddCode(uint16_t prefix_code, uint8_t append_char) {
  if (code_next == GIF_MAX_LZW_CODE)
    return;

  code_table[code_next].prefix = prefix_code;
  code_table[code_next].suffix = append_char;
  if (++code_next < GIF_MAX_LZW_CODE) {
    if (code_next >> code_size_cur)
      code_size_cur++;
  }
}

GifDecodeStatus CGifLZWDecoder::Decode(uint8_t* des_buf, uint32_t* des_size) {
  if (*des_size == 0)
    return GifDecodeStatus::InsufficientDestSize;

  uint32_t i = 0;
  if (stack_size != 0) {
    if (*des_size < stack_size) {
      memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], *des_size);
      stack_size -= static_cast<uint16_t>(*des_size);
      return GifDecodeStatus::InsufficientDestSize;
    }
    memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], stack_size);
    des_buf += stack_size;
    i += stack_size;
    stack_size = 0;
  }
  ASSERT(err_msg_ptr);
  while (i <= *des_size && (avail_in > 0 || bits_left >= code_size_cur)) {
    if (code_size_cur > 12) {
      strncpy(err_msg_ptr, "Code Length Out Of Range", GIF_MAX_ERROR_SIZE - 1);
      return GifDecodeStatus::Error;
    }
    if (avail_in > 0) {
      if (bits_left > 31) {
        strncpy(err_msg_ptr, "Decode Error", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
      pdfium::base::CheckedNumeric<uint32_t> safe_code = *next_in++;
      safe_code <<= bits_left;
      safe_code |= code_store;
      if (!safe_code.IsValid()) {
        strncpy(err_msg_ptr, "Code Store Out Of Range", GIF_MAX_ERROR_SIZE - 1);
        return GifDecodeStatus::Error;
      }
      code_store = safe_code.ValueOrDie();
      --avail_in;
      bits_left += 8;
    }
    while (bits_left >= code_size_cur) {
      uint16_t code =
          static_cast<uint16_t>(code_store) & ((1 << code_size_cur) - 1);
      code_store >>= code_size_cur;
      bits_left -= code_size_cur;
      if (code == code_clear) {
        ClearTable();
        continue;
      }
      if (code == code_end) {
        *des_size = i;
        return GifDecodeStatus::Success;
      }
      if (code_old != static_cast<uint16_t>(-1)) {
        if (code_next < GIF_MAX_LZW_CODE) {
          if (code == code_next) {
            AddCode(code_old, code_first);
            if (!DecodeString(code)) {
              strncpy(err_msg_ptr, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
          } else if (code > code_next) {
            strncpy(err_msg_ptr, "Decode Error, Out Of Range",
                    GIF_MAX_ERROR_SIZE - 1);
            return GifDecodeStatus::Error;
          } else {
            if (!DecodeString(code)) {
              strncpy(err_msg_ptr, "String Decoding Error",
                      GIF_MAX_ERROR_SIZE - 1);
              return GifDecodeStatus::Error;
            }
            uint8_t append_char = stack[GIF_MAX_LZW_CODE - stack_size];
            AddCode(code_old, append_char);
          }
        }
      } else {
        if (!DecodeString(code)) {
          strncpy(err_msg_ptr, "String Decoding Error", GIF_MAX_ERROR_SIZE - 1);
          return GifDecodeStatus::Error;
        }
      }
      code_old = code;
      if (i + stack_size > *des_size) {
        memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], *des_size - i);
        stack_size -= static_cast<uint16_t>(*des_size - i);
        return GifDecodeStatus::InsufficientDestSize;
      }
      memcpy(des_buf, &stack[GIF_MAX_LZW_CODE - stack_size], stack_size);
      des_buf += stack_size;
      i += stack_size;
      stack_size = 0;
    }
  }
  if (avail_in == 0) {
    *des_size = i;
    return GifDecodeStatus::Unfinished;
  }
  return GifDecodeStatus::Error;
}

GifDecodeStatus gif_read_header(CGifContext* context) {
  if (!context)
    return GifDecodeStatus::Error;

  uint32_t skip_size_org = context->skip_size;
  GifHeader* gif_header_ptr = nullptr;
  if (!gif_read_data(context, reinterpret_cast<uint8_t**>(&gif_header_ptr), 6))
    return GifDecodeStatus::Unfinished;

  if (strncmp(gif_header_ptr->signature, GIF_SIGNATURE, 3) != 0 ||
      gif_header_ptr->version[0] != '8' || gif_header_ptr->version[2] != 'a') {
    context->AddError("Not A Gif Image");
    return GifDecodeStatus::Error;
  }
  GifLSD* gif_lsd_ptr = nullptr;
  if (!gif_read_data(context, reinterpret_cast<uint8_t**>(&gif_lsd_ptr), 7)) {
    context->skip_size = skip_size_org;
    return GifDecodeStatus::Unfinished;
  }
  if (reinterpret_cast<GifGF*>(&gif_lsd_ptr->global_flag)->global_pal) {
    context->global_pal_num =
        2 << reinterpret_cast<GifGF*>(&gif_lsd_ptr->global_flag)->pal_bits;
    int32_t global_pal_size = context->global_pal_num * 3;
    uint8_t* global_pal_ptr = nullptr;
    if (!gif_read_data(context, &global_pal_ptr, global_pal_size)) {
      context->skip_size = skip_size_org;
      return GifDecodeStatus::Unfinished;
    }
    context->global_sort_flag = ((GifGF*)&gif_lsd_ptr->global_flag)->sort_flag;
    context->global_color_resolution =
        ((GifGF*)&gif_lsd_ptr->global_flag)->color_resolution;
    context->m_GlobalPalette.resize(global_pal_size / 3);
    memcpy(context->m_GlobalPalette.data(), global_pal_ptr, global_pal_size);
  }
  context->width =
      (int)GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_lsd_ptr->width));
  context->height =
      (int)GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_lsd_ptr->height));
  context->bc_index = gif_lsd_ptr->bc_index;
  context->pixel_aspect = gif_lsd_ptr->pixel_aspect;
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_get_frame(CGifContext* context) {
  if (!context)
    return GifDecodeStatus::Error;

  GifDecodeStatus ret = GifDecodeStatus::Success;
  while (true) {
    switch (context->decode_status) {
      case GIF_D_STATUS_TAIL:
        return GifDecodeStatus::Success;
      case GIF_D_STATUS_SIG: {
        uint8_t* sig_ptr = nullptr;
        if (!gif_read_data(context, &sig_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*sig_ptr) {
          case GIF_SIG_EXTENSION:
            gif_save_decoding_status(context, GIF_D_STATUS_EXT);
            continue;
          case GIF_SIG_IMAGE:
            gif_save_decoding_status(context, GIF_D_STATUS_IMG_INFO);
            continue;
          case GIF_SIG_TRAILER:
            gif_save_decoding_status(context, GIF_D_STATUS_TAIL);
            return GifDecodeStatus::Success;
          default:
            if (context->avail_in) {
              // The Gif File has non_standard Tag!
              gif_save_decoding_status(context, GIF_D_STATUS_SIG);
              continue;
            }
            // The Gif File Doesn't have Trailer Tag!
            return GifDecodeStatus::Success;
        }
      }
      case GIF_D_STATUS_EXT: {
        uint8_t* ext_ptr = nullptr;
        if (!gif_read_data(context, &ext_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*ext_ptr) {
          case GIF_BLOCK_CE:
            gif_save_decoding_status(context, GIF_D_STATUS_EXT_CE);
            continue;
          case GIF_BLOCK_GCE:
            gif_save_decoding_status(context, GIF_D_STATUS_EXT_GCE);
            continue;
          case GIF_BLOCK_PTE:
            gif_save_decoding_status(context, GIF_D_STATUS_EXT_PTE);
            continue;
          default: {
            int32_t status = GIF_D_STATUS_EXT_UNE;
            if (*ext_ptr == GIF_BLOCK_PTE) {
              status = GIF_D_STATUS_EXT_PTE;
            }
            gif_save_decoding_status(context, status);
            continue;
          }
        }
      }
      case GIF_D_STATUS_IMG_INFO: {
        ret = gif_decode_image_info(context);
        if (ret != GifDecodeStatus::Success)
          return ret;

        continue;
      }
      case GIF_D_STATUS_IMG_DATA: {
        uint8_t* data_size_ptr = nullptr;
        uint8_t* data_ptr = nullptr;
        uint32_t skip_size_org = context->skip_size;
        if (!gif_read_data(context, &data_size_ptr, 1))
          return GifDecodeStatus::Unfinished;

        while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
          if (!gif_read_data(context, &data_ptr, *data_size_ptr)) {
            context->skip_size = skip_size_org;
            return GifDecodeStatus::Unfinished;
          }
          gif_save_decoding_status(context, GIF_D_STATUS_IMG_DATA);
          skip_size_org = context->skip_size;
          if (!gif_read_data(context, &data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;
        }
        gif_save_decoding_status(context, GIF_D_STATUS_SIG);
        continue;
      }
      default: {
        ret = gif_decode_extension(context);
        if (ret != GifDecodeStatus::Success)
          return ret;
        break;
      }
    }
  }
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_load_frame(CGifContext* context, int32_t frame_num) {
  if (!context || !pdfium::IndexInBounds(context->m_Images, frame_num))
    return GifDecodeStatus::Error;

  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = context->skip_size;
  GifImage* gif_image_ptr = context->m_Images[frame_num].get();
  uint32_t gif_img_row_bytes = gif_image_ptr->m_ImageInfo.width;
  if (gif_img_row_bytes == 0) {
    context->AddError("Error Invalid Number of Row Bytes");
    return GifDecodeStatus::Error;
  }
  if (context->decode_status == GIF_D_STATUS_TAIL) {
    gif_image_ptr->m_ImageRowBuf.resize(gif_img_row_bytes);
    GifGCE* gif_img_gce_ptr = gif_image_ptr->m_ImageGCE.get();
    int32_t loc_pal_num =
        ((GifLF*)&gif_image_ptr->m_ImageInfo.local_flag)->local_pal
            ? (2 << ((GifLF*)&gif_image_ptr->m_ImageInfo.local_flag)->pal_bits)
            : 0;
    context->avail_in = 0;
    GifPalette* pLocalPalette = gif_image_ptr->m_LocalPalettes.empty()
                                    ? nullptr
                                    : gif_image_ptr->m_LocalPalettes.data();
    if (!gif_img_gce_ptr) {
      bool bRes = context->GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->m_ImageInfo.left,
          gif_image_ptr->m_ImageInfo.top, gif_image_ptr->m_ImageInfo.width,
          gif_image_ptr->m_ImageInfo.height, loc_pal_num, pLocalPalette, 0, 0,
          -1, 0,
          (bool)((GifLF*)&gif_image_ptr->m_ImageInfo.local_flag)->interlace);
      if (!bRes) {
        gif_image_ptr->m_ImageRowBuf.clear();
        context->AddError("Error Read Record Position Data");
        return GifDecodeStatus::Error;
      }
    } else {
      bool bRes = context->GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->m_ImageInfo.left,
          gif_image_ptr->m_ImageInfo.top, gif_image_ptr->m_ImageInfo.width,
          gif_image_ptr->m_ImageInfo.height, loc_pal_num, pLocalPalette,
          (int32_t)gif_image_ptr->m_ImageGCE->delay_time,
          (bool)((GifCEF*)&gif_image_ptr->m_ImageGCE->gce_flag)->user_input,
          ((GifCEF*)&gif_image_ptr->m_ImageGCE->gce_flag)->transparency
              ? (int32_t)gif_image_ptr->m_ImageGCE->trans_index
              : -1,
          (int32_t)((GifCEF*)&gif_image_ptr->m_ImageGCE->gce_flag)
              ->disposal_method,
          (bool)((GifLF*)&gif_image_ptr->m_ImageInfo.local_flag)->interlace);
      if (!bRes) {
        gif_image_ptr->m_ImageRowBuf.clear();
        context->AddError("Error Read Record Position Data");
        return GifDecodeStatus::Error;
      }
    }
    if (gif_image_ptr->image_code_size >= 13) {
      gif_image_ptr->m_ImageRowBuf.clear();
      context->AddError("Error Invalid Code Size");
      return GifDecodeStatus::Error;
    }
    if (!context->m_ImgDecoder.get())
      context->m_ImgDecoder =
          pdfium::MakeUnique<CGifLZWDecoder>(context->m_szLastError);
    context->m_ImgDecoder->InitTable(gif_image_ptr->image_code_size);
    context->img_row_offset = 0;
    context->img_row_avail_size = 0;
    context->img_pass_num = 0;
    gif_image_ptr->image_row_num = 0;
    gif_save_decoding_status(context, GIF_D_STATUS_IMG_DATA);
  }
  CGifLZWDecoder* img_decoder_ptr = context->m_ImgDecoder.get();
  if (context->decode_status == GIF_D_STATUS_IMG_DATA) {
    if (!gif_read_data(context, &data_size_ptr, 1))
      return GifDecodeStatus::Unfinished;

    if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
      if (!gif_read_data(context, &data_ptr, *data_size_ptr)) {
        context->skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }
      img_decoder_ptr->Input(data_ptr, *data_size_ptr);
      gif_save_decoding_status(context, GIF_D_STATUS_IMG_DATA);
      context->img_row_offset += context->img_row_avail_size;
      context->img_row_avail_size = gif_img_row_bytes - context->img_row_offset;
      GifDecodeStatus ret = img_decoder_ptr->Decode(
          gif_image_ptr->m_ImageRowBuf.data() + context->img_row_offset,
          &context->img_row_avail_size);
      if (ret == GifDecodeStatus::Error) {
        gif_decoding_failure_at_tail_cleanup(context, gif_image_ptr);
        return GifDecodeStatus::Error;
      }
      while (ret != GifDecodeStatus::Error) {
        if (ret == GifDecodeStatus::Success) {
          context->ReadScanline(gif_image_ptr->image_row_num,
                                gif_image_ptr->m_ImageRowBuf.data());
          gif_image_ptr->m_ImageRowBuf.clear();
          gif_save_decoding_status(context, GIF_D_STATUS_TAIL);
          return GifDecodeStatus::Success;
        }
        if (ret == GifDecodeStatus::Unfinished) {
          ASSERT(img_decoder_ptr->GetAvailInput() == 0);
          skip_size_org = context->skip_size;
          if (!gif_read_data(context, &data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;

          if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
            if (!gif_read_data(context, &data_ptr, *data_size_ptr)) {
              context->skip_size = skip_size_org;
              return GifDecodeStatus::Unfinished;
            }
            img_decoder_ptr->Input(data_ptr, *data_size_ptr);
            gif_save_decoding_status(context, GIF_D_STATUS_IMG_DATA);
            context->img_row_offset += context->img_row_avail_size;
            context->img_row_avail_size =
                gif_img_row_bytes - context->img_row_offset;
            ret = img_decoder_ptr->Decode(
                gif_image_ptr->m_ImageRowBuf.data() + context->img_row_offset,
                &context->img_row_avail_size);
          }
        }
        if (ret == GifDecodeStatus::InsufficientDestSize) {
          if (((GifLF*)&gif_image_ptr->m_ImageInfo.local_flag)->interlace) {
            context->ReadScanline(gif_image_ptr->image_row_num,
                                  gif_image_ptr->m_ImageRowBuf.data());
            gif_image_ptr->image_row_num +=
                s_gif_interlace_step[context->img_pass_num];
            if (gif_image_ptr->image_row_num >=
                (int32_t)gif_image_ptr->m_ImageInfo.height) {
              context->img_pass_num++;
              if (context->img_pass_num == FX_ArraySize(s_gif_interlace_step)) {
                gif_decoding_failure_at_tail_cleanup(context, gif_image_ptr);
                return GifDecodeStatus::Error;
              }
              gif_image_ptr->image_row_num =
                  s_gif_interlace_step[context->img_pass_num] / 2;
            }
          } else {
            context->ReadScanline(gif_image_ptr->image_row_num++,
                                  gif_image_ptr->m_ImageRowBuf.data());
          }
          context->img_row_offset = 0;
          context->img_row_avail_size = gif_img_row_bytes;
          ret = img_decoder_ptr->Decode(
              gif_image_ptr->m_ImageRowBuf.data() + context->img_row_offset,
              &context->img_row_avail_size);
        }
        if (ret == GifDecodeStatus::Error) {
          gif_decoding_failure_at_tail_cleanup(context, gif_image_ptr);
          return GifDecodeStatus::Error;
        }
      }
    }
    gif_save_decoding_status(context, GIF_D_STATUS_TAIL);
  }
  context->AddError("Decode Image Data Error");
  return GifDecodeStatus::Error;
}

void gif_input_buffer(CGifContext* context,
                      uint8_t* src_buf,
                      uint32_t src_size) {
  context->next_in = src_buf;
  context->avail_in = src_size;
  context->skip_size = 0;
}

uint32_t gif_get_avail_input(CGifContext* context, uint8_t** avail_buf_ptr) {
  if (avail_buf_ptr) {
    *avail_buf_ptr = nullptr;
    if (context->avail_in > 0)
      *avail_buf_ptr = context->next_in;
  }
  return context->avail_in;
}

int32_t gif_get_frame_num(CGifContext* context) {
  return pdfium::CollectionSize<int32_t>(context->m_Images);
}
