// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_CRT_IFGAS_STREAM_H_
#define XFA_FGAS_CRT_IFGAS_STREAM_H_

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_system.h"

enum FX_STREAMSEEK {
  FX_STREAMSEEK_Begin = 0,
  FX_STREAMSEEK_Current,
};

class IFGAS_Stream : public CFX_Retainable {
 public:
  static CFX_RetainPtr<IFGAS_Stream> CreateReadStream(
      const CFX_RetainPtr<IFX_SeekableStream>& pFileRead);
  static CFX_RetainPtr<IFGAS_Stream> CreateWriteStream(
      const CFX_RetainPtr<IFX_SeekableStream>& pFileWrite);

  virtual FX_FILESIZE GetLength() const = 0;
  virtual FX_FILESIZE GetPosition() = 0;
  virtual FX_STRSIZE GetBOMLength() const = 0;

  virtual void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) = 0;

  virtual FX_STRSIZE ReadString(wchar_t* pStr,
                                FX_STRSIZE iMaxLength,
                                bool* bEOS) = 0;
  virtual void WriteString(const CFX_WideStringC& str) = 0;

  virtual uint16_t GetCodePage() const = 0;
  virtual void SetCodePage(uint16_t wCodePage) = 0;

 protected:
  virtual bool IsEOF() const = 0;
};

#endif  // XFA_FGAS_CRT_IFGAS_STREAM_H_
