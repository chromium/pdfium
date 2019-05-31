// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_CODEC_CJPX_DECODER_H_
#define CORE_FXCODEC_CODEC_CJPX_DECODER_H_

#include <memory>
#include <vector>

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/span.h"

#ifdef USE_SYSTEM_LIBOPENJPEG2
#include <openjpeg.h>
#else
#include "third_party/libopenjpeg20/openjpeg.h"
#endif

class CJPX_Decoder {
 public:
  enum ColorSpaceOption {
    kNoColorSpace,
    kNormalColorSpace,
    kIndexedColorSpace
  };

  explicit CJPX_Decoder(ColorSpaceOption option);
  ~CJPX_Decoder();

  bool Init(pdfium::span<const uint8_t> src_data);
  void GetInfo(uint32_t* width, uint32_t* height, uint32_t* components);
  bool StartDecode();
  bool Decode(uint8_t* dest_buf,
              uint32_t pitch,
              const std::vector<uint8_t>& offsets);

 private:
  const ColorSpaceOption m_ColorSpaceOption;
  pdfium::span<const uint8_t> m_SrcData;
  UnownedPtr<opj_image_t> m_Image;
  UnownedPtr<opj_codec_t> m_Codec;
  std::unique_ptr<DecodeData> m_DecodeData;
  UnownedPtr<opj_stream_t> m_Stream;
  opj_dparameters_t m_Parameters;
};

#endif  // CORE_FXCODEC_CODEC_CJPX_DECODER_H_
