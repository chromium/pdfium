// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_GIF_CFX_GIFCONTEXT_H_
#define CORE_FXCODEC_GIF_CFX_GIFCONTEXT_H_

#include <memory>
#include <vector>

#include "core/fxcodec/gif/cfx_gif.h"
#include "core/fxcodec/gif/gif_decoder.h"
#include "core/fxcodec/gif/lzw_decompressor.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/span.h"

class CFX_CodecMemory;

namespace fxcodec {

class CFX_GifContext : public ProgressiveDecoderIface::Context {
 public:
  explicit CFX_GifContext(GifDecoder::Delegate* delegate);
  ~CFX_GifContext() override;

  void ReadScanline(int32_t row_num, pdfium::span<uint8_t> row_buf);
  bool GetRecordPosition(uint32_t cur_pos,
                         int32_t left,
                         int32_t top,
                         int32_t width,
                         int32_t height,
                         int32_t pal_num,
                         CFX_GifPalette* pal,
                         int32_t trans_index,
                         bool interlace);
  GifDecoder::Status ReadHeader();
  GifDecoder::Status GetFrame();
  GifDecoder::Status LoadFrame(size_t frame_num);
  void SetInputBuffer(RetainPtr<CFX_CodecMemory> codec_memory);
  uint32_t GetAvailInput() const;
  size_t GetFrameNum() const { return images_.size(); }

  UnownedPtr<GifDecoder::Delegate> const delegate_;
  std::vector<CFX_GifPalette> global_palette_;
  uint8_t global_palette_exp_ = 0;
  uint32_t img_row_offset_ = 0;
  uint32_t img_row_avail_size_ = 0;
  int32_t decode_status_ = GIF_D_STATUS_SIG;
  std::unique_ptr<CFX_GifGraphicControlExtension> graphic_control_extension_;
  std::vector<std::unique_ptr<CFX_GifImage>> images_;
  std::unique_ptr<LZWDecompressor> lzw_decompressor_;
  int width_ = 0;
  int height_ = 0;
  uint8_t bc_index_ = 0;
  uint8_t global_sort_flag_ = 0;
  uint8_t global_color_resolution_ = 0;
  uint8_t img_pass_num_ = 0;

 protected:
  bool ReadAllOrNone(uint8_t* dest, uint32_t size);
  GifDecoder::Status ReadGifSignature();
  GifDecoder::Status ReadLogicalScreenDescriptor();

  RetainPtr<CFX_CodecMemory> input_buffer_;

 private:
  void SaveDecodingStatus(int32_t status);
  GifDecoder::Status DecodeExtension();
  GifDecoder::Status DecodeImageInfo();
  void DecodingFailureAtTailCleanup(CFX_GifImage* gif_image);
  bool ScanForTerminalMarker();
  uint8_t GetPaletteExp(CFX_GifImage* gif_image) const;
};

}  // namespace fxcodec

#endif  // CORE_FXCODEC_GIF_CFX_GIFCONTEXT_H_
