// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_FAX_FAXMODULE_H_
#define CORE_FXCODEC_FAX_FAXMODULE_H_

#include <memory>

#include "build/build_config.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/span.h"

namespace fxcodec {

class ScanlineDecoder;

class FaxModule {
 public:
  static std::unique_ptr<ScanlineDecoder> CreateDecoder(
      pdfium::span<const uint8_t> src_span,
      int width,
      int height,
      int K,
      bool EndOfLine,
      bool EncodedByteAlign,
      bool BlackIs1,
      int Columns,
      int Rows);

  // Return the ending bit position.
  static int FaxG4Decode(const uint8_t* src_buf,
                         uint32_t src_size,
                         int starting_bitpos,
                         int width,
                         int height,
                         int pitch,
                         uint8_t* dest_buf);

#if defined(OS_WIN)
  static void FaxEncode(const uint8_t* src_buf,
                        int width,
                        int height,
                        int pitch,
                        std::unique_ptr<uint8_t, FxFreeDeleter>* dest_buf,
                        uint32_t* dest_size);
#endif  // defined(OS_WIN)

  FaxModule() = delete;
  FaxModule(const FaxModule&) = delete;
  FaxModule& operator=(const FaxModule&) = delete;
};

}  // namespace fxcodec

using FaxModule = fxcodec::FaxModule;

#endif  // CORE_FXCODEC_FAX_FAXMODULE_H_
