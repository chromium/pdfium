// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_CODEC_JPEGMODULE_H_
#define CORE_FXCODEC_CODEC_JPEGMODULE_H_

#include <csetjmp>
#include <memory>

#include "build/build_config.h"
#include "core/fxcodec/codec/codec_module_iface.h"
#include "third_party/base/span.h"

class CFX_DIBAttribute;
class CFX_DIBBase;

namespace fxcodec {

class ScanlineDecoder;

class JpegModule final : public CodecModuleIface {
 public:
  std::unique_ptr<ScanlineDecoder> CreateDecoder(
      pdfium::span<const uint8_t> src_buf,
      int width,
      int height,
      int nComps,
      bool ColorTransform);

  // CodecModuleIface:
  FX_FILESIZE GetAvailInput(Context* pContext) const override;
  bool Input(Context* pContext,
             RetainPtr<CFX_CodecMemory> codec_memory,
             CFX_DIBAttribute* pAttribute) override;

  jmp_buf* GetJumpMark(Context* pContext);
  bool LoadInfo(pdfium::span<const uint8_t> src_span,
                int* width,
                int* height,
                int* num_components,
                int* bits_per_components,
                bool* color_transform);

  std::unique_ptr<Context> Start();

#ifdef PDF_ENABLE_XFA
  int ReadHeader(Context* pContext,
                 int* width,
                 int* height,
                 int* nComps,
                 CFX_DIBAttribute* pAttribute);
#endif  // PDF_ENABLE_XFA

  bool StartScanline(Context* pContext, int down_scale);
  bool ReadScanline(Context* pContext, uint8_t* dest_buf);

#if defined(OS_WIN)
  static bool JpegEncode(const RetainPtr<CFX_DIBBase>& pSource,
                         uint8_t** dest_buf,
                         size_t* dest_size);
#endif  // defined(OS_WIN)
};

}  // namespace fxcodec

using JpegModule = fxcodec::JpegModule;

#endif  // CORE_FXCODEC_CODEC_JPEGMODULE_H_
