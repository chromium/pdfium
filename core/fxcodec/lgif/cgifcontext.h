// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_LGIF_CGIFCONTEXT_H_
#define CORE_FXCODEC_LGIF_CGIFCONTEXT_H_

#include <memory>
#include <vector>

#include "core/fxcodec/codec/ccodec_gifmodule.h"
#include "core/fxcodec/lgif/cfx_lzwdecoder.h"
#include "core/fxcodec/lgif/fx_gif.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"

class CGifContext : public CCodec_GifModule::Context {
 public:
  CGifContext(CCodec_GifModule* gif_module,
              CCodec_GifModule::Delegate* pDelegate);
  ~CGifContext() override;

  void AddError(const char* err_msg);
  void RecordCurrentPosition(uint32_t* cur_pos_ptr);
  void ReadScanline(int32_t row_num, uint8_t* row_buf);
  bool GetRecordPosition(uint32_t cur_pos,
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
                         bool interlace);
  GifDecodeStatus ReadHeader();
  GifDecodeStatus GetFrame();
  GifDecodeStatus LoadFrame(int32_t frame_num);
  void SetInputBuffer(uint8_t* src_buf, uint32_t src_size);
  uint32_t GetAvailInput(uint8_t** avail_buf_ptr) const;
  int32_t GetFrameNum() const;

  UnownedPtr<CCodec_GifModule> m_pModule;
  UnownedPtr<CCodec_GifModule::Delegate> m_pDelegate;
  std::vector<GifPalette> m_GlobalPalette;
  uint8_t global_pal_exp;
  uint32_t img_row_offset;
  uint32_t img_row_avail_size;
  uint32_t avail_in;
  int32_t decode_status;
  uint32_t skip_size;
  ByteString cmt_data;
  std::unique_ptr<GifGraphicControlExtension> m_GraphicControlExtension;
  uint8_t* next_in;
  std::vector<std::unique_ptr<GifImage>> m_Images;
  std::unique_ptr<CFX_LZWDecoder> m_ImgDecoder;
  int width;
  int height;
  uint8_t bc_index;
  uint8_t pixel_aspect;
  uint8_t global_sort_flag;
  uint8_t global_color_resolution;
  uint8_t img_pass_num;
  char m_szLastError[GIF_MAX_ERROR_SIZE];

 private:
  uint8_t* ReadData(uint8_t** des_buf_pp, uint32_t data_size);
  void SaveDecodingStatus(int32_t status);
  GifDecodeStatus DecodeExtension();
  GifDecodeStatus DecodeImageInfo();
  void DecodingFailureAtTailCleanup(GifImage* gif_image_ptr);
};

#endif  // CORE_FXCODEC_LGIF_CGIFCONTEXT_H_
