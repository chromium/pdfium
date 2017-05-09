// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/fx_gif.h"

#include <utility>

#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcodec/lgif/cgifdecompressor.h"
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

void gif_takeover_gce_ptr(CGifDecompressor* gif_ptr, GifGCE** gce_ptr_ptr) {
  *gce_ptr_ptr = nullptr;
  if (gif_ptr->gce_ptr)
    std::swap(*gce_ptr_ptr, gif_ptr->gce_ptr);
}

uint8_t* gif_read_data(CGifDecompressor* gif_ptr,
                       uint8_t** des_buf_pp,
                       uint32_t data_size) {
  if (!gif_ptr || gif_ptr->avail_in < gif_ptr->skip_size + data_size)
    return nullptr;

  *des_buf_pp = gif_ptr->next_in + gif_ptr->skip_size;
  gif_ptr->skip_size += data_size;
  return *des_buf_pp;
}

void gif_save_decoding_status(CGifDecompressor* gif_ptr, int32_t status) {
  gif_ptr->decode_status = status;
  gif_ptr->next_in += gif_ptr->skip_size;
  gif_ptr->avail_in -= gif_ptr->skip_size;
  gif_ptr->skip_size = 0;
}

GifDecodeStatus gif_decode_extension(CGifDecompressor* gif_ptr) {
  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = gif_ptr->skip_size;
  switch (gif_ptr->decode_status) {
    case GIF_D_STATUS_EXT_CE: {
      if (!gif_read_data(gif_ptr, &data_size_ptr, 1)) {
        gif_ptr->skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }
      gif_ptr->cmt_data_ptr->clear();
      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        uint8_t data_size = *data_size_ptr;
        if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr) ||
            !gif_read_data(gif_ptr, &data_size_ptr, 1)) {
          gif_ptr->skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
        *(gif_ptr->cmt_data_ptr) +=
            CFX_ByteString((const char*)data_ptr, data_size);
      }
    } break;
    case GIF_D_STATUS_EXT_PTE: {
      GifPTE* gif_pte_ptr = nullptr;
      if (!gif_read_data(gif_ptr, (uint8_t**)&gif_pte_ptr, 13))
        return GifDecodeStatus::Unfinished;

      GifPlainText* gif_pt_ptr = FX_Alloc(GifPlainText, 1);
      memset(gif_pt_ptr, 0, sizeof(GifPlainText));
      gif_takeover_gce_ptr(gif_ptr, &gif_pt_ptr->gce_ptr);
      gif_pt_ptr->pte_ptr = FX_Alloc(GifPTE, 1);
      gif_pt_ptr->string_ptr = new CFX_ByteString;
      gif_pt_ptr->pte_ptr->block_size = gif_pte_ptr->block_size;
      gif_pt_ptr->pte_ptr->grid_left =
          GetWord_LSBFirst((uint8_t*)&gif_pte_ptr->grid_left);
      gif_pt_ptr->pte_ptr->grid_top =
          GetWord_LSBFirst((uint8_t*)&gif_pte_ptr->grid_top);
      gif_pt_ptr->pte_ptr->grid_width =
          GetWord_LSBFirst((uint8_t*)&gif_pte_ptr->grid_width);
      gif_pt_ptr->pte_ptr->grid_height =
          GetWord_LSBFirst((uint8_t*)&gif_pte_ptr->grid_height);
      gif_pt_ptr->pte_ptr->char_width = gif_pte_ptr->char_width;
      gif_pt_ptr->pte_ptr->char_height = gif_pte_ptr->char_height;
      gif_pt_ptr->pte_ptr->fc_index = gif_pte_ptr->fc_index;
      gif_pt_ptr->pte_ptr->bc_index = gif_pte_ptr->bc_index;
      if (!gif_read_data(gif_ptr, &data_size_ptr, 1)) {
        gif_ptr->skip_size = skip_size_org;
        if (gif_pt_ptr) {
          FX_Free(gif_pt_ptr->gce_ptr);
          FX_Free(gif_pt_ptr->pte_ptr);
          delete gif_pt_ptr->string_ptr;
          FX_Free(gif_pt_ptr);
        }
        return GifDecodeStatus::Unfinished;
      }
      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        uint8_t data_size = *data_size_ptr;
        if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr) ||
            !gif_read_data(gif_ptr, &data_size_ptr, 1)) {
          gif_ptr->skip_size = skip_size_org;
          if (gif_pt_ptr) {
            FX_Free(gif_pt_ptr->gce_ptr);
            FX_Free(gif_pt_ptr->pte_ptr);
            delete gif_pt_ptr->string_ptr;
            FX_Free(gif_pt_ptr);
          }
          return GifDecodeStatus::Unfinished;
        }
        *(gif_pt_ptr->string_ptr) +=
            CFX_ByteString((const char*)data_ptr, data_size);
      }
      gif_ptr->pt_ptr_arr_ptr->push_back(gif_pt_ptr);
    } break;
    case GIF_D_STATUS_EXT_GCE: {
      GifGCE* gif_gce_ptr = nullptr;
      if (!gif_read_data(gif_ptr, (uint8_t**)&gif_gce_ptr, 6))
        return GifDecodeStatus::Unfinished;

      if (!gif_ptr->gce_ptr)
        gif_ptr->gce_ptr = FX_Alloc(GifGCE, 1);
      gif_ptr->gce_ptr->block_size = gif_gce_ptr->block_size;
      gif_ptr->gce_ptr->gce_flag = gif_gce_ptr->gce_flag;
      gif_ptr->gce_ptr->delay_time =
          GetWord_LSBFirst((uint8_t*)&gif_gce_ptr->delay_time);
      gif_ptr->gce_ptr->trans_index = gif_gce_ptr->trans_index;
    } break;
    default: {
      if (gif_ptr->decode_status == GIF_D_STATUS_EXT_PTE) {
        FX_Free(gif_ptr->gce_ptr);
        gif_ptr->gce_ptr = nullptr;
      }
      if (!gif_read_data(gif_ptr, &data_size_ptr, 1))
        return GifDecodeStatus::Unfinished;

      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr) ||
            !gif_read_data(gif_ptr, &data_size_ptr, 1)) {
          gif_ptr->skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
      }
    }
  }
  gif_save_decoding_status(gif_ptr, GIF_D_STATUS_SIG);
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_decode_image_info(CGifDecompressor* gif_ptr) {
  if (gif_ptr->width == 0 || gif_ptr->height == 0) {
    gif_ptr->ErrorData("No Image Header Info");
    return GifDecodeStatus::Error;
  }
  uint32_t skip_size_org = gif_ptr->skip_size;
  GifImageInfo* gif_img_info_ptr = nullptr;
  if (!gif_read_data(gif_ptr, (uint8_t**)&gif_img_info_ptr, 9))
    return GifDecodeStatus::Unfinished;

  GifImage* gif_image_ptr = FX_Alloc(GifImage, 1);
  memset(gif_image_ptr, 0, sizeof(GifImage));
  gif_image_ptr->image_info_ptr = FX_Alloc(GifImageInfo, 1);
  gif_image_ptr->image_info_ptr->left =
      GetWord_LSBFirst((uint8_t*)&gif_img_info_ptr->left);
  gif_image_ptr->image_info_ptr->top =
      GetWord_LSBFirst((uint8_t*)&gif_img_info_ptr->top);
  gif_image_ptr->image_info_ptr->width =
      GetWord_LSBFirst((uint8_t*)&gif_img_info_ptr->width);
  gif_image_ptr->image_info_ptr->height =
      GetWord_LSBFirst((uint8_t*)&gif_img_info_ptr->height);
  gif_image_ptr->image_info_ptr->local_flag = gif_img_info_ptr->local_flag;
  if (gif_image_ptr->image_info_ptr->left +
              gif_image_ptr->image_info_ptr->width >
          gif_ptr->width ||
      gif_image_ptr->image_info_ptr->top +
              gif_image_ptr->image_info_ptr->height >
          gif_ptr->height) {
    FX_Free(gif_image_ptr->image_info_ptr);
    FX_Free(gif_image_ptr->image_row_buf);
    FX_Free(gif_image_ptr);
    gif_ptr->ErrorData("Image Data Out Of LSD, The File May Be Corrupt");
    return GifDecodeStatus::Error;
  }
  GifLF* gif_img_info_lf_ptr = (GifLF*)&gif_img_info_ptr->local_flag;
  if (gif_img_info_lf_ptr->local_pal) {
    int32_t loc_pal_size = (2 << gif_img_info_lf_ptr->pal_bits) * 3;
    uint8_t* loc_pal_ptr = nullptr;
    if (!gif_read_data(gif_ptr, &loc_pal_ptr, loc_pal_size)) {
      gif_ptr->skip_size = skip_size_org;
      FX_Free(gif_image_ptr->image_info_ptr);
      FX_Free(gif_image_ptr->image_row_buf);
      FX_Free(gif_image_ptr);
      return GifDecodeStatus::Unfinished;
    }
    gif_image_ptr->local_pal_ptr =
        (GifPalette*)gif_ptr->AskBufForPal(loc_pal_size);
    if (gif_image_ptr->local_pal_ptr) {
      memcpy((uint8_t*)gif_image_ptr->local_pal_ptr, loc_pal_ptr, loc_pal_size);
    }
  }
  uint8_t* code_size_ptr = nullptr;
  if (!gif_read_data(gif_ptr, &code_size_ptr, 1)) {
    gif_ptr->skip_size = skip_size_org;
    FX_Free(gif_image_ptr->image_info_ptr);
    FX_Free(gif_image_ptr->local_pal_ptr);
    FX_Free(gif_image_ptr->image_row_buf);
    FX_Free(gif_image_ptr);
    return GifDecodeStatus::Unfinished;
  }
  gif_image_ptr->image_code_size = *code_size_ptr;
  gif_ptr->RecordCurrentPosition(&gif_image_ptr->image_data_pos);
  gif_image_ptr->image_data_pos += gif_ptr->skip_size;
  gif_takeover_gce_ptr(gif_ptr, &gif_image_ptr->image_gce_ptr);
  gif_ptr->img_ptr_arr_ptr->push_back(gif_image_ptr);
  gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_DATA);
  return GifDecodeStatus::Success;
}

void gif_decoding_failure_at_tail_cleanup(CGifDecompressor* gif_ptr,
                                          GifImage* gif_image_ptr) {
  FX_Free(gif_image_ptr->image_row_buf);
  gif_image_ptr->image_row_buf = nullptr;
  gif_save_decoding_status(gif_ptr, GIF_D_STATUS_TAIL);
  gif_ptr->ErrorData("Decode Image Data Error");
}

}  // namespace

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
  ASSERT(code_size < 32);
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

void CGifLZWDecoder::DecodeString(uint16_t code) {
  stack_size = 0;
  while (true) {
    if (code < code_clear || code > code_next)
      break;

    stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = code_table[code].suffix;
    code = code_table[code].prefix;
  }
  stack[GIF_MAX_LZW_CODE - 1 - stack_size++] = static_cast<uint8_t>(code);
  code_first = static_cast<uint8_t>(code);
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
            DecodeString(code);
          } else if (code > code_next) {
            strncpy(err_msg_ptr, "Decode Error, Out Of Range",
                    GIF_MAX_ERROR_SIZE - 1);
            return GifDecodeStatus::Error;
          } else {
            DecodeString(code);
            uint8_t append_char = stack[GIF_MAX_LZW_CODE - stack_size];
            AddCode(code_old, append_char);
          }
        }
      } else {
        DecodeString(code);
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

GifDecodeStatus gif_read_header(CGifDecompressor* gif_ptr) {
  if (!gif_ptr)
    return GifDecodeStatus::Error;

  uint32_t skip_size_org = gif_ptr->skip_size;
  GifHeader* gif_header_ptr = nullptr;
  if (!gif_read_data(gif_ptr, reinterpret_cast<uint8_t**>(&gif_header_ptr), 6))
    return GifDecodeStatus::Unfinished;

  if (strncmp(gif_header_ptr->signature, GIF_SIGNATURE, 3) != 0 ||
      gif_header_ptr->version[0] != '8' || gif_header_ptr->version[2] != 'a') {
    gif_ptr->ErrorData("Not A Gif Image");
    return GifDecodeStatus::Error;
  }
  GifLSD* gif_lsd_ptr = nullptr;
  if (!gif_read_data(gif_ptr, reinterpret_cast<uint8_t**>(&gif_lsd_ptr), 7)) {
    gif_ptr->skip_size = skip_size_org;
    return GifDecodeStatus::Unfinished;
  }
  if (reinterpret_cast<GifGF*>(&gif_lsd_ptr->global_flag)->global_pal) {
    gif_ptr->global_pal_num =
        2 << reinterpret_cast<GifGF*>(&gif_lsd_ptr->global_flag)->pal_bits;
    int32_t global_pal_size = gif_ptr->global_pal_num * 3;
    uint8_t* global_pal_ptr = nullptr;
    if (!gif_read_data(gif_ptr, &global_pal_ptr, global_pal_size)) {
      gif_ptr->skip_size = skip_size_org;
      return GifDecodeStatus::Unfinished;
    }
    gif_ptr->global_sort_flag = ((GifGF*)&gif_lsd_ptr->global_flag)->sort_flag;
    gif_ptr->global_color_resolution =
        ((GifGF*)&gif_lsd_ptr->global_flag)->color_resolution;
    gif_ptr->m_GlobalPalette.resize(global_pal_size / 3);
    memcpy(gif_ptr->m_GlobalPalette.data(), global_pal_ptr, global_pal_size);
  }
  gif_ptr->width = (int)GetWord_LSBFirst((uint8_t*)&gif_lsd_ptr->width);
  gif_ptr->height = (int)GetWord_LSBFirst((uint8_t*)&gif_lsd_ptr->height);
  gif_ptr->bc_index = gif_lsd_ptr->bc_index;
  gif_ptr->pixel_aspect = gif_lsd_ptr->pixel_aspect;
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_get_frame(CGifDecompressor* gif_ptr) {
  if (!gif_ptr)
    return GifDecodeStatus::Error;

  GifDecodeStatus ret = GifDecodeStatus::Success;
  while (true) {
    switch (gif_ptr->decode_status) {
      case GIF_D_STATUS_TAIL:
        return GifDecodeStatus::Success;
      case GIF_D_STATUS_SIG: {
        uint8_t* sig_ptr = nullptr;
        if (!gif_read_data(gif_ptr, &sig_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*sig_ptr) {
          case GIF_SIG_EXTENSION:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_EXT);
            continue;
          case GIF_SIG_IMAGE:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_INFO);
            continue;
          case GIF_SIG_TRAILER:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_TAIL);
            return GifDecodeStatus::Success;
          default:
            if (gif_ptr->avail_in) {
              // The Gif File has non_standard Tag!
              gif_save_decoding_status(gif_ptr, GIF_D_STATUS_SIG);
              continue;
            }
            // The Gif File Doesn't have Trailer Tag!
            return GifDecodeStatus::Success;
        }
      }
      case GIF_D_STATUS_EXT: {
        uint8_t* ext_ptr = nullptr;
        if (!gif_read_data(gif_ptr, &ext_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*ext_ptr) {
          case GIF_BLOCK_CE:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_EXT_CE);
            continue;
          case GIF_BLOCK_GCE:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_EXT_GCE);
            continue;
          case GIF_BLOCK_PTE:
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_EXT_PTE);
            continue;
          default: {
            int32_t status = GIF_D_STATUS_EXT_UNE;
            if (*ext_ptr == GIF_BLOCK_PTE) {
              status = GIF_D_STATUS_EXT_PTE;
            }
            gif_save_decoding_status(gif_ptr, status);
            continue;
          }
        }
      }
      case GIF_D_STATUS_IMG_INFO: {
        ret = gif_decode_image_info(gif_ptr);
        if (ret != GifDecodeStatus::Success)
          return ret;
        continue;
      }
      case GIF_D_STATUS_IMG_DATA: {
        uint8_t* data_size_ptr = nullptr;
        uint8_t* data_ptr = nullptr;
        uint32_t skip_size_org = gif_ptr->skip_size;
        if (!gif_read_data(gif_ptr, &data_size_ptr, 1))
          return GifDecodeStatus::Unfinished;

        while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
          if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr)) {
            gif_ptr->skip_size = skip_size_org;
            return GifDecodeStatus::Unfinished;
          }
          gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_DATA);
          skip_size_org = gif_ptr->skip_size;
          if (!gif_read_data(gif_ptr, &data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;
        }
        gif_save_decoding_status(gif_ptr, GIF_D_STATUS_SIG);
        continue;
      }
      default: {
        ret = gif_decode_extension(gif_ptr);
        if (ret != GifDecodeStatus::Success)
          return ret;
        break;
      }
    }
  }
  return GifDecodeStatus::Success;
}

GifDecodeStatus gif_load_frame(CGifDecompressor* gif_ptr, int32_t frame_num) {
  if (!gif_ptr || !pdfium::IndexInBounds(*gif_ptr->img_ptr_arr_ptr, frame_num))
    return GifDecodeStatus::Error;

  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = gif_ptr->skip_size;
  GifImage* gif_image_ptr = (*gif_ptr->img_ptr_arr_ptr)[frame_num];
  uint32_t gif_img_row_bytes = gif_image_ptr->image_info_ptr->width;
  if (gif_img_row_bytes == 0) {
    gif_ptr->ErrorData("Error Invalid Number of Row Bytes");
    return GifDecodeStatus::Error;
  }
  if (gif_ptr->decode_status == GIF_D_STATUS_TAIL) {
    if (gif_image_ptr->image_row_buf) {
      FX_Free(gif_image_ptr->image_row_buf);
      gif_image_ptr->image_row_buf = nullptr;
    }
    gif_image_ptr->image_row_buf = FX_Alloc(uint8_t, gif_img_row_bytes);
    GifGCE* gif_img_gce_ptr = gif_image_ptr->image_gce_ptr;
    int32_t loc_pal_num =
        ((GifLF*)&gif_image_ptr->image_info_ptr->local_flag)->local_pal
            ? (2 << ((GifLF*)&gif_image_ptr->image_info_ptr->local_flag)
                        ->pal_bits)
            : 0;
    gif_ptr->avail_in = 0;
    if (!gif_img_gce_ptr) {
      bool bRes = gif_ptr->GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->image_info_ptr->left,
          gif_image_ptr->image_info_ptr->top,
          gif_image_ptr->image_info_ptr->width,
          gif_image_ptr->image_info_ptr->height, loc_pal_num,
          gif_image_ptr->local_pal_ptr, 0, 0, -1, 0,
          (bool)((GifLF*)&gif_image_ptr->image_info_ptr->local_flag)
              ->interlace);
      if (!bRes) {
        FX_Free(gif_image_ptr->image_row_buf);
        gif_image_ptr->image_row_buf = nullptr;
        gif_ptr->ErrorData("Error Read Record Position Data");
        return GifDecodeStatus::Error;
      }
    } else {
      bool bRes = gif_ptr->GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->image_info_ptr->left,
          gif_image_ptr->image_info_ptr->top,
          gif_image_ptr->image_info_ptr->width,
          gif_image_ptr->image_info_ptr->height, loc_pal_num,
          gif_image_ptr->local_pal_ptr,
          (int32_t)gif_image_ptr->image_gce_ptr->delay_time,
          (bool)((GifCEF*)&gif_image_ptr->image_gce_ptr->gce_flag)->user_input,
          ((GifCEF*)&gif_image_ptr->image_gce_ptr->gce_flag)->transparency
              ? (int32_t)gif_image_ptr->image_gce_ptr->trans_index
              : -1,
          (int32_t)((GifCEF*)&gif_image_ptr->image_gce_ptr->gce_flag)
              ->disposal_method,
          (bool)((GifLF*)&gif_image_ptr->image_info_ptr->local_flag)
              ->interlace);
      if (!bRes) {
        FX_Free(gif_image_ptr->image_row_buf);
        gif_image_ptr->image_row_buf = nullptr;
        gif_ptr->ErrorData("Error Read Record Position Data");
        return GifDecodeStatus::Error;
      }
    }
    if (gif_image_ptr->image_code_size >= 32) {
      FX_Free(gif_image_ptr->image_row_buf);
      gif_image_ptr->image_row_buf = nullptr;
      gif_ptr->ErrorData("Error Invalid Code Size");
      return GifDecodeStatus::Error;
    }
    if (!gif_ptr->m_ImgDecoder.get())
      gif_ptr->m_ImgDecoder =
          pdfium::MakeUnique<CGifLZWDecoder>(gif_ptr->err_ptr);
    gif_ptr->m_ImgDecoder->InitTable(gif_image_ptr->image_code_size);
    gif_ptr->img_row_offset = 0;
    gif_ptr->img_row_avail_size = 0;
    gif_ptr->img_pass_num = 0;
    gif_image_ptr->image_row_num = 0;
    gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_DATA);
  }
  CGifLZWDecoder* img_decoder_ptr = gif_ptr->m_ImgDecoder.get();
  if (gif_ptr->decode_status == GIF_D_STATUS_IMG_DATA) {
    if (!gif_read_data(gif_ptr, &data_size_ptr, 1))
      return GifDecodeStatus::Unfinished;

    if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
      if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr)) {
        gif_ptr->skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }
      img_decoder_ptr->Input(data_ptr, *data_size_ptr);
      gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_DATA);
      gif_ptr->img_row_offset += gif_ptr->img_row_avail_size;
      gif_ptr->img_row_avail_size = gif_img_row_bytes - gif_ptr->img_row_offset;
      GifDecodeStatus ret = img_decoder_ptr->Decode(
          gif_image_ptr->image_row_buf + gif_ptr->img_row_offset,
          &gif_ptr->img_row_avail_size);
      if (ret == GifDecodeStatus::Error) {
        gif_decoding_failure_at_tail_cleanup(gif_ptr, gif_image_ptr);
        return GifDecodeStatus::Error;
      }
      while (ret != GifDecodeStatus::Error) {
        if (ret == GifDecodeStatus::Success) {
          gif_ptr->ReadScanline(gif_image_ptr->image_row_num,
                                gif_image_ptr->image_row_buf);
          FX_Free(gif_image_ptr->image_row_buf);
          gif_image_ptr->image_row_buf = nullptr;
          gif_save_decoding_status(gif_ptr, GIF_D_STATUS_TAIL);
          return GifDecodeStatus::Success;
        }
        if (ret == GifDecodeStatus::Unfinished) {
          ASSERT(img_decoder_ptr->GetAvailInput() == 0);
          skip_size_org = gif_ptr->skip_size;
          if (!gif_read_data(gif_ptr, &data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;

          if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
            if (!gif_read_data(gif_ptr, &data_ptr, *data_size_ptr)) {
              gif_ptr->skip_size = skip_size_org;
              return GifDecodeStatus::Unfinished;
            }
            img_decoder_ptr->Input(data_ptr, *data_size_ptr);
            gif_save_decoding_status(gif_ptr, GIF_D_STATUS_IMG_DATA);
            gif_ptr->img_row_offset += gif_ptr->img_row_avail_size;
            gif_ptr->img_row_avail_size =
                gif_img_row_bytes - gif_ptr->img_row_offset;
            ret = img_decoder_ptr->Decode(
                gif_image_ptr->image_row_buf + gif_ptr->img_row_offset,
                &gif_ptr->img_row_avail_size);
          }
        }
        if (ret == GifDecodeStatus::InsufficientDestSize) {
          if (((GifLF*)&gif_image_ptr->image_info_ptr->local_flag)->interlace) {
            gif_ptr->ReadScanline(gif_image_ptr->image_row_num,
                                  gif_image_ptr->image_row_buf);
            gif_image_ptr->image_row_num +=
                s_gif_interlace_step[gif_ptr->img_pass_num];
            if (gif_image_ptr->image_row_num >=
                (int32_t)gif_image_ptr->image_info_ptr->height) {
              gif_ptr->img_pass_num++;
              if (gif_ptr->img_pass_num == FX_ArraySize(s_gif_interlace_step)) {
                gif_decoding_failure_at_tail_cleanup(gif_ptr, gif_image_ptr);
                return GifDecodeStatus::Error;
              }
              gif_image_ptr->image_row_num =
                  s_gif_interlace_step[gif_ptr->img_pass_num] / 2;
            }
          } else {
            gif_ptr->ReadScanline(gif_image_ptr->image_row_num++,
                                  gif_image_ptr->image_row_buf);
          }
          gif_ptr->img_row_offset = 0;
          gif_ptr->img_row_avail_size = gif_img_row_bytes;
          ret = img_decoder_ptr->Decode(
              gif_image_ptr->image_row_buf + gif_ptr->img_row_offset,
              &gif_ptr->img_row_avail_size);
        }
        if (ret == GifDecodeStatus::Error) {
          gif_decoding_failure_at_tail_cleanup(gif_ptr, gif_image_ptr);
          return GifDecodeStatus::Error;
        }
      }
    }
    gif_save_decoding_status(gif_ptr, GIF_D_STATUS_TAIL);
  }
  gif_ptr->ErrorData("Decode Image Data Error");
  return GifDecodeStatus::Error;
}

void gif_input_buffer(CGifDecompressor* gif_ptr,
                      uint8_t* src_buf,
                      uint32_t src_size) {
  gif_ptr->next_in = src_buf;
  gif_ptr->avail_in = src_size;
  gif_ptr->skip_size = 0;
}

uint32_t gif_get_avail_input(CGifDecompressor* gif_ptr,
                             uint8_t** avail_buf_ptr) {
  if (avail_buf_ptr) {
    *avail_buf_ptr = nullptr;
    if (gif_ptr->avail_in > 0)
      *avail_buf_ptr = gif_ptr->next_in;
  }
  return gif_ptr->avail_in;
}

int32_t gif_get_frame_num(CGifDecompressor* gif_ptr) {
  return pdfium::CollectionSize<int32_t>(*gif_ptr->img_ptr_arr_ptr);
}
