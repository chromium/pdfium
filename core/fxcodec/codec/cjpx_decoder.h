// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_CODEC_CJPX_DECODER_H_
#define CORE_FXCODEC_CODEC_CJPX_DECODER_H_

#include <vector>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "third_party/libopenjpeg20/openjpeg.h"

class CPDF_ColorSpace;

class CJPX_Decoder {
 public:
  explicit CJPX_Decoder(CPDF_ColorSpace* cs);
  ~CJPX_Decoder();

  bool Init(const unsigned char* src_data, uint32_t src_size);
  void GetInfo(uint32_t* width, uint32_t* height, uint32_t* components);
  bool Decode(uint8_t* dest_buf,
              int pitch,
              const std::vector<uint8_t>& offsets);

 private:
  const uint8_t* m_SrcData;
  uint32_t m_SrcSize;
  opj_image_t* image;
  opj_codec_t* l_codec;
  opj_stream_t* l_stream;
  CFX_UnownedPtr<const CPDF_ColorSpace> const m_ColorSpace;
};

#endif  // CORE_FXCODEC_CODEC_CJPX_DECODER_H_
