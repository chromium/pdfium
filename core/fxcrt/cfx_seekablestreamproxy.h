// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_
#define CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_

#include <algorithm>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_SeekableStreamProxy : public Retainable {
 public:
  enum class From {
    Begin = 0,
    Current,
  };

  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  FX_FILESIZE GetLength() const { return m_pStream->GetSize(); }
  FX_FILESIZE GetPosition() { return m_iPosition; }
  size_t GetBOMLength() const { return m_wBOMLength; }
  bool IsEOF() const { return m_iPosition >= GetLength(); }

  void Seek(From eSeek, FX_FILESIZE iOffset);
  size_t ReadString(wchar_t* pStr, size_t iMaxLength, bool* bEOS);

  void WriteString(const WideStringView& str);

  uint16_t GetCodePage() const { return m_wCodePage; }
  void SetCodePage(uint16_t wCodePage);

 private:
  CFX_SeekableStreamProxy(const RetainPtr<IFX_SeekableStream>& stream,
                          bool isWriteSteam);
  CFX_SeekableStreamProxy(uint8_t* data, size_t size);
  ~CFX_SeekableStreamProxy() override;

  size_t ReadData(uint8_t* pBuffer, size_t iBufferSize);

  bool m_IsWriteStream;
  uint16_t m_wCodePage;
  size_t m_wBOMLength;
  FX_FILESIZE m_iPosition;
  RetainPtr<IFX_SeekableStream> m_pStream;
};

#endif  // CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_
