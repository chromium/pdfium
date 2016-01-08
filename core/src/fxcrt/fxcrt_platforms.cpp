// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxcrt_platforms.h"

#include "core/include/fxcrt/fx_basic.h"

#if (_FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_ && \
     _FXM_PLATFORM_ != _FXM_PLATFORM_LINUX_ &&   \
     _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_ &&   \
     _FXM_PLATFORM_ != _FXM_PLATFORM_ANDROID_)
IFXCRT_FileAccess* FXCRT_FileAccess_Create() {
  return new CFXCRT_FileAccess_CRT;
}
void FXCRT_GetFileModeString(FX_DWORD dwModes, CFX_ByteString& bsMode) {
  if (dwModes & FX_FILEMODE_ReadOnly) {
    bsMode = "rb";
  } else if (dwModes & FX_FILEMODE_Truncate) {
    bsMode = "w+b";
  } else {
    bsMode = "a+b";
  }
}
void FXCRT_GetFileModeString(FX_DWORD dwModes, CFX_WideString& wsMode) {
  if (dwModes & FX_FILEMODE_ReadOnly) {
    wsMode = FX_WSTRC(L"rb");
  } else if (dwModes & FX_FILEMODE_Truncate) {
    wsMode = FX_WSTRC(L"w+b");
  } else {
    wsMode = FX_WSTRC(L"a+b");
  }
}
CFXCRT_FileAccess_CRT::CFXCRT_FileAccess_CRT() : m_hFile(NULL) {}
CFXCRT_FileAccess_CRT::~CFXCRT_FileAccess_CRT() {
  Close();
}
FX_BOOL CFXCRT_FileAccess_CRT::Open(const CFX_ByteStringC& fileName,
                                    FX_DWORD dwMode) {
  if (m_hFile) {
    return FALSE;
  }
  CFX_ByteString strMode;
  FXCRT_GetFileModeString(dwMode, strMode);
  m_hFile = FXSYS_fopen(fileName.GetCStr(), strMode.c_str());
  return m_hFile != NULL;
}
FX_BOOL CFXCRT_FileAccess_CRT::Open(const CFX_WideStringC& fileName,
                                    FX_DWORD dwMode) {
  if (m_hFile) {
    return FALSE;
  }
  CFX_WideString strMode;
  FXCRT_GetFileModeString(dwMode, strMode);
  m_hFile = FXSYS_wfopen(fileName.GetPtr(), strMode.c_str());
  return m_hFile != NULL;
}
void CFXCRT_FileAccess_CRT::Close() {
  if (!m_hFile) {
    return;
  }
  FXSYS_fclose(m_hFile);
  m_hFile = NULL;
}
void CFXCRT_FileAccess_CRT::Release() {
  delete this;
}
FX_FILESIZE CFXCRT_FileAccess_CRT::GetSize() const {
  if (!m_hFile) {
    return 0;
  }
  FX_FILESIZE pos = (FX_FILESIZE)FXSYS_ftell(m_hFile);
  FXSYS_fseek(m_hFile, 0, FXSYS_SEEK_END);
  FX_FILESIZE size = (FX_FILESIZE)FXSYS_ftell(m_hFile);
  FXSYS_fseek(m_hFile, pos, FXSYS_SEEK_SET);
  return size;
}
FX_FILESIZE CFXCRT_FileAccess_CRT::GetPosition() const {
  if (!m_hFile) {
    return (FX_FILESIZE)-1;
  }
  return (FX_FILESIZE)FXSYS_ftell(m_hFile);
}
FX_FILESIZE CFXCRT_FileAccess_CRT::SetPosition(FX_FILESIZE pos) {
  if (!m_hFile) {
    return (FX_FILESIZE)-1;
  }
  FXSYS_fseek(m_hFile, pos, FXSYS_SEEK_SET);
  return (FX_FILESIZE)FXSYS_ftell(m_hFile);
}
size_t CFXCRT_FileAccess_CRT::Read(void* pBuffer, size_t szBuffer) {
  if (!m_hFile) {
    return 0;
  }
  return FXSYS_fread(pBuffer, 1, szBuffer, m_hFile);
}
size_t CFXCRT_FileAccess_CRT::Write(const void* pBuffer, size_t szBuffer) {
  if (!m_hFile) {
    return 0;
  }
  return FXSYS_fwrite(pBuffer, 1, szBuffer, m_hFile);
}
size_t CFXCRT_FileAccess_CRT::ReadPos(void* pBuffer,
                                      size_t szBuffer,
                                      FX_FILESIZE pos) {
  if (!m_hFile) {
    return (FX_FILESIZE)-1;
  }
  FXSYS_fseek(m_hFile, pos, FXSYS_SEEK_SET);
  return FXSYS_fread(pBuffer, 1, szBuffer, m_hFile);
}
size_t CFXCRT_FileAccess_CRT::WritePos(const void* pBuffer,
                                       size_t szBuffer,
                                       FX_FILESIZE pos) {
  if (!m_hFile) {
    return (FX_FILESIZE)-1;
  }
  FXSYS_fseek(m_hFile, pos, FXSYS_SEEK_SET);
  return FXSYS_fwrite(pBuffer, 1, szBuffer, m_hFile);
}
FX_BOOL CFXCRT_FileAccess_CRT::Flush() {
  if (!m_hFile) {
    return FALSE;
  }
  return !FXSYS_fflush(m_hFile);
}
FX_BOOL CFXCRT_FileAccess_CRT::Truncate(FX_FILESIZE szFile) {
  return FALSE;
}
#endif
