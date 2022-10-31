// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_
#define CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_types.h"
#include "core/fxcrt/retain_ptr.h"

class CFX_SeekableStreamProxy final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  // Unlike IFX_SeekableStreamProxy, buffers and sizes are always in terms
  // of the number of wchar_t elementss, not bytes.
  FX_FILESIZE GetSize();  // Estimate under worst possible expansion.
  bool IsEOF();
  size_t ReadBlock(wchar_t* pStr, size_t size);

  FX_CodePage GetCodePage() const { return m_wCodePage; }
  void SetCodePage(FX_CodePage wCodePage);

 private:
  enum class From {
    Begin = 0,
    Current,
  };

  explicit CFX_SeekableStreamProxy(
      const RetainPtr<IFX_SeekableReadStream>& stream);
  ~CFX_SeekableStreamProxy() override;

  FX_FILESIZE GetPosition();
  void Seek(From eSeek, FX_FILESIZE iOffset);
  size_t ReadData(uint8_t* pBuffer, size_t iBufferSize);

  FX_CodePage m_wCodePage = FX_CodePage::kDefANSI;
  size_t m_wBOMLength = 0;
  FX_FILESIZE m_iPosition = 0;
  RetainPtr<IFX_SeekableReadStream> const m_pStream;
};

#endif  // CORE_FXCRT_CFX_SEEKABLESTREAMPROXY_H_
