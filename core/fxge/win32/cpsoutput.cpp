// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cpsoutput.h"

#include <algorithm>

#include "core/fxcrt/fx_system.h"

CPSOutput::CPSOutput(HDC hDC) : m_hDC(hDC) {}

CPSOutput::~CPSOutput() {}

bool CPSOutput::WriteBlock(const void* str, size_t len) {
  int sent_len = 0;
  while (len > 0) {
    char buffer[1026];
    size_t send_len = std::min(len, static_cast<size_t>(1024));
    *(reinterpret_cast<uint16_t*>(buffer)) = send_len;
    memcpy(buffer + 2, static_cast<const char*>(str) + sent_len, send_len);

    // TODO(thestig/rbpotter): Do PASSTHROUGH for non-Chromium usage.
    // ExtEscape(m_hDC, PASSTHROUGH, send_len + 2, buffer, 0, nullptr);
    ::GdiComment(m_hDC, send_len + 2, reinterpret_cast<const BYTE*>(buffer));
    sent_len += send_len;
    len -= send_len;
  }
  return true;
}

bool CPSOutput::WriteString(const ByteStringView& str) {
  return WriteBlock(str.unterminated_c_str(), str.GetLength());
}
