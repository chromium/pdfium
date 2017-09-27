// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lgif/cgifcontext.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/codec/ccodec_gifmodule.h"
#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const int32_t s_gif_interlace_step[4] = {8, 8, 4, 2};

}  // namespace

CGifContext::CGifContext(CCodec_GifModule* gif_module,
                         CCodec_GifModule::Delegate* pDelegate)
    : m_pModule(gif_module),
      m_pDelegate(pDelegate),
      global_pal_exp(0),
      img_row_offset(0),
      img_row_avail_size(0),
      avail_in(0),
      decode_status(GIF_D_STATUS_SIG),
      skip_size(0),
      next_in(nullptr),
      width(0),
      height(0),
      bc_index(0),
      pixel_aspect(0),
      global_sort_flag(0),
      global_color_resolution(0),
      img_pass_num(0) {
}

CGifContext::~CGifContext() {}

void CGifContext::RecordCurrentPosition(uint32_t* cur_pos_ptr) {
  m_pDelegate->GifRecordCurrentPosition(*cur_pos_ptr);
}

void CGifContext::ReadScanline(int32_t row_num, uint8_t* row_buf) {
  m_pDelegate->GifReadScanline(row_num, row_buf);
}

bool CGifContext::GetRecordPosition(uint32_t cur_pos,
                                    int32_t left,
                                    int32_t top,
                                    int32_t width,
                                    int32_t height,
                                    int32_t pal_num,
                                    GifPalette* pal_ptr,
                                    int32_t delay_time,
                                    bool user_input,
                                    int32_t trans_index,
                                    int32_t disposal_method,
                                    bool interlace) {
  return m_pDelegate->GifInputRecordPositionBuf(
      cur_pos, FX_RECT(left, top, left + width, top + height), pal_num, pal_ptr,
      delay_time, user_input, trans_index, disposal_method, interlace);
}

GifDecodeStatus CGifContext::ReadHeader() {
  uint32_t skip_size_org = skip_size;
  GifHeader* gif_header_ptr = nullptr;
  if (!ReadData(reinterpret_cast<uint8_t**>(&gif_header_ptr), 6))
    return GifDecodeStatus::Unfinished;

  if (strncmp(gif_header_ptr->signature, GIF_SIGNATURE, 3) != 0 ||
      gif_header_ptr->version[0] != '8' || gif_header_ptr->version[2] != 'a')
    return GifDecodeStatus::Error;

  GifLocalScreenDescriptor* gif_lsd_ptr = nullptr;
  if (!ReadData(reinterpret_cast<uint8_t**>(&gif_lsd_ptr), 7)) {
    skip_size = skip_size_org;
    return GifDecodeStatus::Unfinished;
  }

  if (gif_lsd_ptr->global_flags.global_pal) {
    global_pal_exp = gif_lsd_ptr->global_flags.pal_bits;
    uint32_t global_pal_size = unsigned(2 << global_pal_exp) * 3u;
    uint8_t* global_pal_ptr = nullptr;
    if (!ReadData(&global_pal_ptr, global_pal_size)) {
      skip_size = skip_size_org;
      return GifDecodeStatus::Unfinished;
    }

    global_sort_flag = gif_lsd_ptr->global_flags.sort_flag;
    global_color_resolution = gif_lsd_ptr->global_flags.color_resolution;
    m_GlobalPalette.resize(global_pal_size / 3);
    memcpy(m_GlobalPalette.data(), global_pal_ptr, global_pal_size);
  }

  width = static_cast<int>(
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_lsd_ptr->width)));
  height = static_cast<int>(
      GetWord_LSBFirst(reinterpret_cast<uint8_t*>(&gif_lsd_ptr->height)));
  bc_index = gif_lsd_ptr->bc_index;
  pixel_aspect = gif_lsd_ptr->pixel_aspect;
  return GifDecodeStatus::Success;
}

GifDecodeStatus CGifContext::GetFrame() {
  GifDecodeStatus ret = GifDecodeStatus::Success;
  while (true) {
    switch (decode_status) {
      case GIF_D_STATUS_TAIL:
        return GifDecodeStatus::Success;
      case GIF_D_STATUS_SIG: {
        uint8_t* sig_ptr = nullptr;
        if (!ReadData(&sig_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*sig_ptr) {
          case GIF_SIG_EXTENSION:
            SaveDecodingStatus(GIF_D_STATUS_EXT);
            continue;
          case GIF_SIG_IMAGE:
            SaveDecodingStatus(GIF_D_STATUS_IMG_INFO);
            continue;
          case GIF_SIG_TRAILER:
            SaveDecodingStatus(GIF_D_STATUS_TAIL);
            return GifDecodeStatus::Success;
          default:
            if (avail_in) {
              // The Gif File has non_standard Tag!
              SaveDecodingStatus(GIF_D_STATUS_SIG);
              continue;
            }
            // The Gif File Doesn't have Trailer Tag!
            return GifDecodeStatus::Success;
        }
      }
      case GIF_D_STATUS_EXT: {
        uint8_t* ext_ptr = nullptr;
        if (!ReadData(&ext_ptr, 1))
          return GifDecodeStatus::Unfinished;

        switch (*ext_ptr) {
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
            if (*ext_ptr == GIF_BLOCK_PTE) {
              status = GIF_D_STATUS_EXT_PTE;
            }
            SaveDecodingStatus(status);
            continue;
          }
        }
      }
      case GIF_D_STATUS_IMG_INFO: {
        ret = DecodeImageInfo();
        if (ret != GifDecodeStatus::Success)
          return ret;

        continue;
      }
      case GIF_D_STATUS_IMG_DATA: {
        uint8_t* data_size_ptr = nullptr;
        uint8_t* data_ptr = nullptr;
        uint32_t skip_size_org = skip_size;
        if (!ReadData(&data_size_ptr, 1))
          return GifDecodeStatus::Unfinished;

        while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
          if (!ReadData(&data_ptr, *data_size_ptr)) {
            skip_size = skip_size_org;
            return GifDecodeStatus::Unfinished;
          }

          SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
          skip_size_org = skip_size;
          if (!ReadData(&data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;
        }
        SaveDecodingStatus(GIF_D_STATUS_SIG);
        continue;
      }
      default: {
        ret = DecodeExtension();
        if (ret != GifDecodeStatus::Success)
          return ret;
        break;
      }
    }
  }
  return GifDecodeStatus::Success;
}

GifDecodeStatus CGifContext::LoadFrame(int32_t frame_num) {
  if (!pdfium::IndexInBounds(m_Images, frame_num))
    return GifDecodeStatus::Error;

  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = skip_size;
  GifImage* gif_image_ptr = m_Images[frame_num].get();
  uint32_t gif_img_row_bytes = gif_image_ptr->m_ImageInfo.width;
  if (gif_img_row_bytes == 0)
    return GifDecodeStatus::Error;

  if (decode_status == GIF_D_STATUS_TAIL) {
    gif_image_ptr->m_ImageRowBuf.resize(gif_img_row_bytes);
    GifGraphicControlExtension* gif_img_gce_ptr =
        gif_image_ptr->m_ImageGCE.get();
    int32_t loc_pal_num =
        gif_image_ptr->m_ImageInfo.local_flags.local_pal
            ? (2 << gif_image_ptr->m_ImageInfo.local_flags.pal_bits)
            : 0;
    avail_in = 0;
    GifPalette* pLocalPalette = gif_image_ptr->m_LocalPalettes.empty()
                                    ? nullptr
                                    : gif_image_ptr->m_LocalPalettes.data();
    if (!gif_img_gce_ptr) {
      bool bRes = GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->m_ImageInfo.left,
          gif_image_ptr->m_ImageInfo.top, gif_image_ptr->m_ImageInfo.width,
          gif_image_ptr->m_ImageInfo.height, loc_pal_num, pLocalPalette, 0, 0,
          -1, 0, gif_image_ptr->m_ImageInfo.local_flags.interlace);
      if (!bRes) {
        gif_image_ptr->m_ImageRowBuf.clear();
        return GifDecodeStatus::Error;
      }
    } else {
      bool bRes = GetRecordPosition(
          gif_image_ptr->image_data_pos, gif_image_ptr->m_ImageInfo.left,
          gif_image_ptr->m_ImageInfo.top, gif_image_ptr->m_ImageInfo.width,
          gif_image_ptr->m_ImageInfo.height, loc_pal_num, pLocalPalette,
          static_cast<int32_t>(gif_image_ptr->m_ImageGCE->delay_time),
          gif_image_ptr->m_ImageGCE->gce_flags.user_input,
          gif_image_ptr->m_ImageGCE->gce_flags.transparency
              ? static_cast<int32_t>(gif_image_ptr->m_ImageGCE->trans_index)
              : -1,
          static_cast<int32_t>(
              gif_image_ptr->m_ImageGCE->gce_flags.disposal_method),
          gif_image_ptr->m_ImageInfo.local_flags.interlace);
      if (!bRes) {
        gif_image_ptr->m_ImageRowBuf.clear();
        return GifDecodeStatus::Error;
      }
    }

    if (gif_image_ptr->image_code_exp > GIF_MAX_LZW_EXP) {
      gif_image_ptr->m_ImageRowBuf.clear();
      return GifDecodeStatus::Error;
    }

    img_row_offset = 0;
    img_row_avail_size = 0;
    img_pass_num = 0;
    gif_image_ptr->image_row_num = 0;
    SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  }

  if (decode_status == GIF_D_STATUS_IMG_DATA) {
    if (!ReadData(&data_size_ptr, 1))
      return GifDecodeStatus::Unfinished;

    if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
      if (!ReadData(&data_ptr, *data_size_ptr)) {
        skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }

      if (!m_ImgDecoder.get())
        m_ImgDecoder =
            CFX_LZWDecoder::Create(!gif_image_ptr->m_LocalPalettes.empty()
                                       ? gif_image_ptr->local_pallette_exp
                                       : global_pal_exp,
                                   gif_image_ptr->image_code_exp);
      SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
      img_row_offset += img_row_avail_size;
      img_row_avail_size = gif_img_row_bytes - img_row_offset;
      GifDecodeStatus ret =
          m_ImgDecoder.get()
              ? m_ImgDecoder->Decode(
                    data_ptr, *data_size_ptr,
                    gif_image_ptr->m_ImageRowBuf.data() + img_row_offset,
                    &img_row_avail_size)
              : GifDecodeStatus::Error;
      if (ret == GifDecodeStatus::Error) {
        DecodingFailureAtTailCleanup(gif_image_ptr);
        return GifDecodeStatus::Error;
      }
      while (ret != GifDecodeStatus::Error) {
        if (ret == GifDecodeStatus::Success) {
          ReadScanline(gif_image_ptr->image_row_num,
                       gif_image_ptr->m_ImageRowBuf.data());
          gif_image_ptr->m_ImageRowBuf.clear();
          SaveDecodingStatus(GIF_D_STATUS_TAIL);
          return GifDecodeStatus::Success;
        }
        if (ret == GifDecodeStatus::Unfinished) {
          skip_size_org = skip_size;
          if (!ReadData(&data_size_ptr, 1))
            return GifDecodeStatus::Unfinished;

          if (*data_size_ptr != GIF_BLOCK_TERMINAL) {
            if (!ReadData(&data_ptr, *data_size_ptr)) {
              skip_size = skip_size_org;
              return GifDecodeStatus::Unfinished;
            }
            if (!m_ImgDecoder.get())
              m_ImgDecoder =
                  CFX_LZWDecoder::Create(!gif_image_ptr->m_LocalPalettes.empty()
                                             ? gif_image_ptr->local_pallette_exp
                                             : global_pal_exp,
                                         gif_image_ptr->image_code_exp);
            SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
            img_row_offset += img_row_avail_size;
            img_row_avail_size = gif_img_row_bytes - img_row_offset;
            ret =
                m_ImgDecoder.get()
                    ? m_ImgDecoder->Decode(
                          data_ptr, *data_size_ptr,
                          gif_image_ptr->m_ImageRowBuf.data() + img_row_offset,
                          &img_row_avail_size)
                    : GifDecodeStatus::Error;
          }
        }
        if (ret == GifDecodeStatus::InsufficientDestSize) {
          if (gif_image_ptr->m_ImageInfo.local_flags.interlace) {
            ReadScanline(gif_image_ptr->image_row_num,
                         gif_image_ptr->m_ImageRowBuf.data());
            gif_image_ptr->image_row_num += s_gif_interlace_step[img_pass_num];
            if (gif_image_ptr->image_row_num >=
                static_cast<int32_t>(gif_image_ptr->m_ImageInfo.height)) {
              img_pass_num++;
              if (img_pass_num == FX_ArraySize(s_gif_interlace_step)) {
                DecodingFailureAtTailCleanup(gif_image_ptr);
                return GifDecodeStatus::Error;
              }
              gif_image_ptr->image_row_num =
                  s_gif_interlace_step[img_pass_num] / 2;
            }
          } else {
            ReadScanline(gif_image_ptr->image_row_num++,
                         gif_image_ptr->m_ImageRowBuf.data());
          }
          img_row_offset = 0;
          img_row_avail_size = gif_img_row_bytes;
          ret = m_ImgDecoder.get()
                    ? m_ImgDecoder->Decode(
                          data_ptr, *data_size_ptr,
                          gif_image_ptr->m_ImageRowBuf.data() + img_row_offset,
                          &img_row_avail_size)
                    : GifDecodeStatus::Error;
        }
        if (ret == GifDecodeStatus::Error) {
          DecodingFailureAtTailCleanup(gif_image_ptr);
          return GifDecodeStatus::Error;
        }
      }
    }
    SaveDecodingStatus(GIF_D_STATUS_TAIL);
  }
  return GifDecodeStatus::Error;
}

void CGifContext::SetInputBuffer(uint8_t* src_buf, uint32_t src_size) {
  next_in = src_buf;
  avail_in = src_size;
  skip_size = 0;
}

uint32_t CGifContext::GetAvailInput(uint8_t** avail_buf_ptr) const {
  if (avail_buf_ptr) {
    *avail_buf_ptr = nullptr;
    if (avail_in > 0)
      *avail_buf_ptr = next_in;
  }
  return avail_in;
}

int32_t CGifContext::GetFrameNum() const {
  return pdfium::CollectionSize<int32_t>(m_Images);
}

uint8_t* CGifContext::ReadData(uint8_t** des_buf_pp, uint32_t data_size) {
  if (avail_in < skip_size + data_size)
    return nullptr;

  *des_buf_pp = next_in + skip_size;
  skip_size += data_size;
  return *des_buf_pp;
}

void CGifContext::SaveDecodingStatus(int32_t status) {
  decode_status = status;
  next_in += skip_size;
  avail_in -= skip_size;
  skip_size = 0;
}

GifDecodeStatus CGifContext::DecodeExtension() {
  uint8_t* data_size_ptr = nullptr;
  uint8_t* data_ptr = nullptr;
  uint32_t skip_size_org = skip_size;
  switch (decode_status) {
    case GIF_D_STATUS_EXT_CE: {
      if (!ReadData(&data_size_ptr, 1)) {
        skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }

      cmt_data.clear();
      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        uint8_t data_size = *data_size_ptr;
        if (!ReadData(&data_ptr, *data_size_ptr) ||
            !ReadData(&data_size_ptr, 1)) {
          skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }

        cmt_data += ByteString(data_ptr, data_size);
      }
      break;
    }
    case GIF_D_STATUS_EXT_PTE: {
      GifPlainTextExtension* gif_pte = nullptr;
      if (!ReadData(reinterpret_cast<uint8_t**>(&gif_pte), 13))
        return GifDecodeStatus::Unfinished;

      m_GraphicControlExtension = nullptr;
      if (!ReadData(&data_size_ptr, 1)) {
        skip_size = skip_size_org;
        return GifDecodeStatus::Unfinished;
      }

      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        if (!ReadData(&data_ptr, *data_size_ptr) ||
            !ReadData(&data_size_ptr, 1)) {
          skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
      }
      break;
    }
    case GIF_D_STATUS_EXT_GCE: {
      GifGraphicControlExtension* gif_gce_ptr = nullptr;
      if (!ReadData(reinterpret_cast<uint8_t**>(&gif_gce_ptr), 6))
        return GifDecodeStatus::Unfinished;

      if (!m_GraphicControlExtension.get())
        m_GraphicControlExtension =
            pdfium::MakeUnique<GifGraphicControlExtension>();
      m_GraphicControlExtension->block_size = gif_gce_ptr->block_size;
      m_GraphicControlExtension->gce_flags = gif_gce_ptr->gce_flags;
      m_GraphicControlExtension->delay_time = GetWord_LSBFirst(
          reinterpret_cast<uint8_t*>(&gif_gce_ptr->delay_time));
      m_GraphicControlExtension->trans_index = gif_gce_ptr->trans_index;
      break;
    }
    default: {
      if (decode_status == GIF_D_STATUS_EXT_PTE)
        m_GraphicControlExtension = nullptr;
      if (!ReadData(&data_size_ptr, 1))
        return GifDecodeStatus::Unfinished;

      while (*data_size_ptr != GIF_BLOCK_TERMINAL) {
        if (!ReadData(&data_ptr, *data_size_ptr) ||
            !ReadData(&data_size_ptr, 1)) {
          skip_size = skip_size_org;
          return GifDecodeStatus::Unfinished;
        }
      }
    }
  }
  SaveDecodingStatus(GIF_D_STATUS_SIG);
  return GifDecodeStatus::Success;
}

GifDecodeStatus CGifContext::DecodeImageInfo() {
  if (width == 0 || height == 0)
    return GifDecodeStatus::Error;

  uint32_t skip_size_org = skip_size;
  GifImageInfo* gif_img_info_ptr = nullptr;
  if (!ReadData(reinterpret_cast<uint8_t**>(&gif_img_info_ptr), 9))
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
  gif_image->m_ImageInfo.local_flags = gif_img_info_ptr->local_flags;
  if (gif_image->m_ImageInfo.left + gif_image->m_ImageInfo.width > width ||
      gif_image->m_ImageInfo.top + gif_image->m_ImageInfo.height > height)
    return GifDecodeStatus::Error;

  GifLocalFlags* gif_img_info_lf_ptr = &gif_img_info_ptr->local_flags;
  if (gif_img_info_lf_ptr->local_pal) {
    gif_image->local_pallette_exp = gif_img_info_lf_ptr->pal_bits;
    uint32_t loc_pal_size = unsigned(2 << gif_img_info_lf_ptr->pal_bits) * 3u;
    uint8_t* loc_pal_ptr = nullptr;
    if (!ReadData(&loc_pal_ptr, loc_pal_size)) {
      skip_size = skip_size_org;
      return GifDecodeStatus::Unfinished;
    }

    gif_image->m_LocalPalettes = std::vector<GifPalette>(loc_pal_size / 3);
    std::copy(loc_pal_ptr, loc_pal_ptr + loc_pal_size,
              reinterpret_cast<uint8_t*>(gif_image->m_LocalPalettes.data()));
  }

  uint8_t* code_size_ptr = nullptr;
  if (!ReadData(&code_size_ptr, 1)) {
    skip_size = skip_size_org;
    return GifDecodeStatus::Unfinished;
  }

  gif_image->image_code_exp = *code_size_ptr;
  RecordCurrentPosition(&gif_image->image_data_pos);
  gif_image->image_data_pos += skip_size;
  gif_image->m_ImageGCE = nullptr;
  if (m_GraphicControlExtension.get()) {
    gif_image->m_ImageGCE = std::move(m_GraphicControlExtension);
    m_GraphicControlExtension = nullptr;
  }
  m_Images.push_back(std::move(gif_image));
  SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  return GifDecodeStatus::Success;
}

void CGifContext::DecodingFailureAtTailCleanup(GifImage* gif_image_ptr) {
  gif_image_ptr->m_ImageRowBuf.clear();
  SaveDecodingStatus(GIF_D_STATUS_TAIL);
}
