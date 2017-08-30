// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_WIDETEXTBUF_H_
#define CORE_FXCRT_CFX_WIDETEXTBUF_H_

#include "core/fxcrt/cfx_binarybuf.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

class CFX_WideTextBuf : public CFX_BinaryBuf {
 public:
  void AppendChar(wchar_t wch);
  FX_STRSIZE GetLength() const { return m_DataSize / sizeof(wchar_t); }
  wchar_t* GetBuffer() const {
    return reinterpret_cast<wchar_t*>(m_pBuffer.get());
  }

  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                           m_DataSize / sizeof(wchar_t));
  }
  CFX_WideString MakeString() const {
    return CFX_WideString(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                          m_DataSize / sizeof(wchar_t));
  }

  void Delete(int start_index, int count) {
    CFX_BinaryBuf::Delete(start_index * sizeof(wchar_t),
                          count * sizeof(wchar_t));
  }

  CFX_WideTextBuf& operator<<(int i);
  CFX_WideTextBuf& operator<<(double f);
  CFX_WideTextBuf& operator<<(const wchar_t* lpsz);
  CFX_WideTextBuf& operator<<(const CFX_WideStringC& str);
  CFX_WideTextBuf& operator<<(const CFX_WideString& str);
  CFX_WideTextBuf& operator<<(const CFX_WideTextBuf& buf);
};

#endif  // CORE_FXCRT_CFX_WIDETEXTBUF_H_
