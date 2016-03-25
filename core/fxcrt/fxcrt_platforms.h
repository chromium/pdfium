// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FXCRT_PLATFORMS_H_
#define CORE_FXCRT_FXCRT_PLATFORMS_H_

#include "core/fxcrt/extension.h"

#if _FX_OS_ == _FX_ANDROID_
void FXCRT_GetFileModeString(uint32_t dwModes, CFX_ByteString& bsMode);
void FXCRT_GetFileModeString(uint32_t dwModes, CFX_WideString& wsMode);
class CFXCRT_FileAccess_CRT : public IFXCRT_FileAccess {
 public:
  CFXCRT_FileAccess_CRT();
  ~CFXCRT_FileAccess_CRT() override;

  // IFXCRT_FileAccess
  FX_BOOL Open(const CFX_ByteStringC& fileName, uint32_t dwMode) override;
  FX_BOOL Open(const CFX_WideStringC& fileName, uint32_t dwMode) override;
  void Close() override;
  void Release() override;
  FX_FILESIZE GetSize() const override;
  FX_FILESIZE GetPosition() const override;
  FX_FILESIZE SetPosition(FX_FILESIZE pos) override;
  size_t Read(void* pBuffer, size_t szBuffer) override;
  size_t Write(const void* pBuffer, size_t szBuffer) override;
  size_t ReadPos(void* pBuffer, size_t szBuffer, FX_FILESIZE pos) override;
  size_t WritePos(const void* pBuffer,
                  size_t szBuffer,
                  FX_FILESIZE pos) override;
  FX_BOOL Flush() override;
  FX_BOOL Truncate(FX_FILESIZE szFile) override;

 protected:
  FXSYS_FILE* m_hFile;
};
#endif

#endif  // CORE_FXCRT_FXCRT_PLATFORMS_H_
