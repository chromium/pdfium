// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_CSS_CFDE_CSSTEXTBUF_H_
#define XFA_FDE_CSS_CFDE_CSSTEXTBUF_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/crt/fgas_stream.h"

class CFDE_CSSTextBuf {
 public:
  CFDE_CSSTextBuf();
  ~CFDE_CSSTextBuf();

  bool AttachBuffer(const FX_WCHAR* pBuffer, int32_t iBufLen);
  bool EstimateSize(int32_t iAllocSize);
  int32_t LoadFromStream(const CFX_RetainPtr<IFGAS_Stream>& pTxtStream,
                         int32_t iStreamOffset,
                         int32_t iMaxChars,
                         bool& bEOS);
  bool AppendChar(FX_WCHAR wch) {
    if (m_iDatLen >= m_iBufLen && !ExpandBuf(m_iBufLen * 2))
      return false;
    m_pBuffer[m_iDatLen++] = wch;
    return true;
  }

  void Clear() { m_iDatPos = m_iDatLen = 0; }
  void Reset();

  int32_t TrimEnd() {
    while (m_iDatLen > 0 && m_pBuffer[m_iDatLen - 1] <= ' ')
      --m_iDatLen;
    AppendChar(0);
    return --m_iDatLen;
  }

  void Subtract(int32_t iStart, int32_t iLength);
  bool IsEOF() const { return m_iDatPos >= m_iDatLen; }

  FX_WCHAR GetAt(int32_t index) const { return m_pBuffer[index]; }
  FX_WCHAR GetChar() const { return m_pBuffer[m_iDatPos]; }
  FX_WCHAR GetNextChar() const {
    return (m_iDatPos + 1 >= m_iDatLen) ? 0 : m_pBuffer[m_iDatPos + 1];
  }

  void MoveNext() { m_iDatPos++; }

  int32_t GetLength() const { return m_iDatLen; }
  const FX_WCHAR* GetBuffer() const { return m_pBuffer; }

 protected:
  bool ExpandBuf(int32_t iDesiredSize);
  bool m_bExtBuf;
  FX_WCHAR* m_pBuffer;
  int32_t m_iBufLen;
  int32_t m_iDatLen;
  int32_t m_iDatPos;
};

#endif  // XFA_FDE_CSS_CFDE_CSSTEXTBUF_H_
