// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cpsoutput.h"

#include <algorithm>

#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/span.h"

CPSOutput::CPSOutput(HDC hDC, OutputMode mode) : dc_handle_(hDC), mode_(mode) {}

CPSOutput::~CPSOutput() = default;

bool CPSOutput::WriteBlock(pdfium::span<const uint8_t> input) {
  while (!input.empty()) {
    uint8_t buffer[1026];
    size_t send_len = std::min<size_t>(input.size(), 1024);
    *(reinterpret_cast<uint16_t*>(buffer)) = static_cast<uint16_t>(send_len);
    UNSAFE_TODO(FXSYS_memcpy(buffer + 2, input.data(), send_len));
    switch (mode_) {
      case OutputMode::kExtEscape:
        ExtEscape(dc_handle_, PASSTHROUGH, static_cast<int>(send_len + 2),
                  reinterpret_cast<const char*>(buffer), 0, nullptr);
        break;
      case OutputMode::kGdiComment:
        GdiComment(dc_handle_, static_cast<UINT>(send_len + 2), buffer);
        break;
    }
    input = input.subspan(send_len);
  }
  return true;
}
