// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_CFGAS_STREAM_H_
#define XFA_FGAS_CRT_CFGAS_STREAM_H_

#include <algorithm>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"

class CFGAS_Stream : public CFX_Retainable {
 public:
  enum class Pos {
    Begin = 0,
    Current,
  };

  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  FX_FILESIZE GetLength() const { return m_pStream->GetSize(); }
  FX_FILESIZE GetPosition() { return m_iPosition; }
  FX_STRSIZE GetBOMLength() const { return std::max(0, m_wBOMLength); }
  bool IsEOF() const { return m_iPosition >= GetLength(); }

  void Seek(CFGAS_Stream::Pos eSeek, FX_FILESIZE iOffset);
  FX_STRSIZE ReadString(wchar_t* pStr, FX_STRSIZE iMaxLength, bool* bEOS);

  void WriteString(const CFX_WideStringC& str);

  uint16_t GetCodePage() const { return m_wCodePage; }
  void SetCodePage(uint16_t wCodePage);

 private:
  CFGAS_Stream(const CFX_RetainPtr<IFX_SeekableStream>& stream,
               bool isWriteSteam);
  CFGAS_Stream(uint8_t* data, FX_STRSIZE size);
  ~CFGAS_Stream() override;

  FX_STRSIZE ReadData(uint8_t* pBuffer, FX_STRSIZE iBufferSize);

  uint16_t m_wCodePage;
  FX_STRSIZE m_wBOMLength;
  bool m_IsWriteStream;
  FX_FILESIZE m_iPosition;
  CFX_RetainPtr<IFX_SeekableStream> m_pStream;
};

#endif  // XFA_FGAS_CRT_CFGAS_STREAM_H_
