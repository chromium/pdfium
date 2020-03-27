// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WIDETEXTBUF_H_
#define CORE_FXCRT_CFX_WIDETEXTBUF_H_

#include "core/fxcrt/cfx_binarybuf.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/span.h"

class CFX_WideTextBuf final : public CFX_BinaryBuf {
 public:
  // CFX_BinaryBuf:
  size_t GetLength() const override;

  pdfium::span<wchar_t> GetWideSpan();
  pdfium::span<const wchar_t> GetWideSpan() const;
  WideStringView AsStringView() const;
  WideString MakeString() const;

  void AppendChar(wchar_t wch);
  void Delete(int start_index, int count);

  CFX_WideTextBuf& operator<<(int i);
  CFX_WideTextBuf& operator<<(double f);
  CFX_WideTextBuf& operator<<(ByteStringView ascii);
  CFX_WideTextBuf& operator<<(const wchar_t* lpsz);
  CFX_WideTextBuf& operator<<(WideStringView str);
  CFX_WideTextBuf& operator<<(const WideString& str);
  CFX_WideTextBuf& operator<<(const CFX_WideTextBuf& buf);

 private:
  void ExpandWideBuf(size_t char_count);
};

#endif  // CORE_FXCRT_CFX_WIDETEXTBUF_H_
