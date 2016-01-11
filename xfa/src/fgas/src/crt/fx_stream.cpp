// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/fgas/src/fgas_base.h"
#include "fx_stream.h"
IFX_Stream* IFX_Stream::CreateStream(IFX_BufferRead* pBufferRead,
                                     FX_DWORD dwAccess,
                                     int32_t iFileSize,
                                     FX_BOOL bReleaseBufferRead) {
  CFX_Stream* pSR = new CFX_Stream;
  if (!pSR->LoadBufferRead(pBufferRead, iFileSize, dwAccess,
                           bReleaseBufferRead)) {
    pSR->Release();
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
IFX_Stream* IFX_Stream::CreateStream(IFX_FileRead* pFileRead,
                                     FX_DWORD dwAccess) {
  CFX_Stream* pSR = new CFX_Stream;
  if (!pSR->LoadFileRead(pFileRead, dwAccess)) {
    pSR->Release();
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
IFX_Stream* IFX_Stream::CreateStream(IFX_FileWrite* pFileWrite,
                                     FX_DWORD dwAccess) {
  CFX_Stream* pSR = new CFX_Stream;
  if (!pSR->LoadFileWrite(pFileWrite, dwAccess)) {
    pSR->Release();
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
IFX_Stream* IFX_Stream::CreateStream(const FX_WCHAR* pszFileName,
                                     FX_DWORD dwAccess) {
  CFX_Stream* pSR = new CFX_Stream;
  if (!pSR->LoadFile(pszFileName, dwAccess)) {
    pSR->Release();
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
IFX_Stream* IFX_Stream::CreateStream(uint8_t* pData,
                                     int32_t length,
                                     FX_DWORD dwAccess) {
  CFX_Stream* pSR = new CFX_Stream;
  if (!pSR->LoadBuffer(pData, length, dwAccess)) {
    pSR->Release();
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
CFX_StreamImp::CFX_StreamImp() : CFX_ThreadLock(), m_dwAccess(0) {}
CFX_FileStreamImp::CFX_FileStreamImp()
    : CFX_StreamImp(), m_hFile(NULL), m_iLength(0) {}
CFX_FileStreamImp::~CFX_FileStreamImp() {
  if (m_hFile != NULL) {
    FXSYS_fclose(m_hFile);
  }
}
FX_BOOL CFX_FileStreamImp::LoadFile(const FX_WCHAR* pszSrcFileName,
                                    FX_DWORD dwAccess) {
  FXSYS_assert(m_hFile == NULL);
  FXSYS_assert(pszSrcFileName != NULL && FXSYS_wcslen(pszSrcFileName) > 0);
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
  CFX_WideString wsMode;
  if (dwAccess & FX_STREAMACCESS_Write) {
    if (dwAccess & FX_STREAMACCESS_Append) {
      wsMode = L"a+b";
    } else if (dwAccess & FX_STREAMACCESS_Truncate) {
      wsMode = L"w+b";
    } else {
      wsMode = L"r+b";
    }
  } else {
    wsMode = L"rb";
  }
#ifdef _FX_WINAPI_PARTITION_APP_
  CFX_WideString wsSrcFileName(pszSrcFileName);
  _wfopen_s(&m_hFile, wsSrcFileName, wsMode);
#else
  m_hFile = FXSYS_wfopen(pszSrcFileName, wsMode);
#endif
  if (m_hFile == NULL) {
    if (dwAccess & FX_STREAMACCESS_Write) {
      if (dwAccess & FX_STREAMACCESS_Create) {
#ifdef _FX_WINAPI_PARTITION_APP_
        CFX_WideString wsSrcFileName(pszSrcFileName);
        _wfopen_s(&m_hFile, wsSrcFileName, L"w+b");
#else
        m_hFile = FXSYS_wfopen(pszSrcFileName, L"w+b");
#endif
      }
      if (m_hFile == NULL) {
#ifdef _FX_WINAPI_PARTITION_APP_
        CFX_WideString wsSrcFileName(pszSrcFileName);
        _wfopen_s(&m_hFile, wsSrcFileName, L"r+b");
#else
        m_hFile = FXSYS_wfopen(pszSrcFileName, L"r+b");
#endif
        if (m_hFile == NULL) {
          return FALSE;
        }
        if (dwAccess & FX_STREAMACCESS_Truncate) {
          FX_fsetsize(m_hFile, 0);
        }
      }
    } else {
      return FALSE;
    }
  }
#else
  CFX_ByteString wsMode;
  if (dwAccess & FX_STREAMACCESS_Write) {
    if (dwAccess & FX_STREAMACCESS_Append) {
      wsMode = "a+b";
    } else if (dwAccess & FX_STREAMACCESS_Truncate) {
      wsMode = "w+b";
    } else {
      wsMode = "r+b";
    }
  } else {
    wsMode = "rb";
  }
  CFX_ByteString szFileName = CFX_ByteString::FromUnicode(pszSrcFileName);
  m_hFile = FXSYS_fopen(szFileName, wsMode);
  if (m_hFile == NULL) {
    if (dwAccess & FX_STREAMACCESS_Write) {
      if (dwAccess & FX_STREAMACCESS_Create) {
        m_hFile = FXSYS_fopen(szFileName, "w+b");
      }
      if (m_hFile == NULL) {
        m_hFile = FXSYS_fopen(szFileName, "r+b");
        if (m_hFile == NULL) {
          return FALSE;
        }
        if (dwAccess & FX_STREAMACCESS_Truncate) {
          FX_fsetsize(m_hFile, 0);
        }
      }
    } else {
      return FALSE;
    }
  }
#endif
  m_dwAccess = dwAccess;
  if ((dwAccess & (FX_STREAMACCESS_Write | FX_STREAMACCESS_Truncate)) ==
      (FX_STREAMACCESS_Write | FX_STREAMACCESS_Truncate)) {
    m_iLength = 0;
  } else {
    m_iLength = FX_filelength(m_hFile);
  }
  return TRUE;
}
int32_t CFX_FileStreamImp::GetLength() const {
  FXSYS_assert(m_hFile != NULL);
  return m_iLength;
}
int32_t CFX_FileStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  FXSYS_assert(m_hFile != NULL);
  FXSYS_fseek(m_hFile, iOffset, eSeek);
  return FXSYS_ftell(m_hFile);
}
int32_t CFX_FileStreamImp::GetPosition() {
  FXSYS_assert(m_hFile != NULL);
  return FXSYS_ftell(m_hFile);
}
FX_BOOL CFX_FileStreamImp::IsEOF() const {
  FXSYS_assert(m_hFile != NULL);
  return FXSYS_ftell(m_hFile) >= m_iLength;
}
int32_t CFX_FileStreamImp::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  FXSYS_assert(m_hFile != NULL);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  return FXSYS_fread(pBuffer, 1, iBufferSize, m_hFile);
}
int32_t CFX_FileStreamImp::ReadString(FX_WCHAR* pStr,
                                      int32_t iMaxLength,
                                      FX_BOOL& bEOS) {
  FXSYS_assert(m_hFile != NULL);
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  if (m_iLength <= 0) {
    return 0;
  }
  int32_t iPosition = FXSYS_ftell(m_hFile);
  int32_t iLen = std::min((m_iLength - iPosition) / 2, iMaxLength);
  if (iLen <= 0) {
    return 0;
  }
  iLen = FXSYS_fread(pStr, 2, iLen, m_hFile);
  int32_t iCount = 0;
  while (*pStr != L'\0' && iCount < iLen) {
    pStr++, iCount++;
  }
  iPosition += iCount * 2;
  if (FXSYS_ftell(m_hFile) != iPosition) {
    FXSYS_fseek(m_hFile, iPosition, 0);
  }
  bEOS = (iPosition >= m_iLength);
  return iCount;
}
int32_t CFX_FileStreamImp::WriteData(const uint8_t* pBuffer,
                                     int32_t iBufferSize) {
  FXSYS_assert(m_hFile != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  int32_t iRet = FXSYS_fwrite(pBuffer, 1, iBufferSize, m_hFile);
  if (iRet != 0) {
    int32_t iPos = FXSYS_ftell(m_hFile);
    if (iPos > m_iLength) {
      m_iLength = iPos;
    }
  }
  return iRet;
}
int32_t CFX_FileStreamImp::WriteString(const FX_WCHAR* pStr, int32_t iLength) {
  FXSYS_assert(m_hFile != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FXSYS_assert(pStr != NULL && iLength > 0);
  int32_t iRet = FXSYS_fwrite(pStr, 2, iLength, m_hFile);
  if (iRet != 0) {
    int32_t iPos = FXSYS_ftell(m_hFile);
    if (iPos > m_iLength) {
      m_iLength = iPos;
    }
  }
  return iRet;
}
void CFX_FileStreamImp::Flush() {
  FXSYS_assert(m_hFile != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FXSYS_fflush(m_hFile);
}
FX_BOOL CFX_FileStreamImp::SetLength(int32_t iLength) {
  FXSYS_assert(m_hFile != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FX_BOOL bRet = FX_fsetsize(m_hFile, iLength);
  m_iLength = FX_filelength(m_hFile);
  return bRet;
}
CFX_FileReadStreamImp::CFX_FileReadStreamImp()
    : m_pFileRead(NULL), m_iPosition(0), m_iLength(0) {}
FX_BOOL CFX_FileReadStreamImp::LoadFileRead(IFX_FileRead* pFileRead,
                                            FX_DWORD dwAccess) {
  FXSYS_assert(m_pFileRead == NULL && pFileRead != NULL);
  if (dwAccess & FX_STREAMACCESS_Write) {
    return FALSE;
  }
  m_pFileRead = pFileRead;
  m_iLength = m_pFileRead->GetSize();
  return TRUE;
}
int32_t CFX_FileReadStreamImp::GetLength() const {
  return m_iLength;
}
int32_t CFX_FileReadStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
    case FX_STREAMSEEK_End:
      m_iPosition = m_iLength + iOffset;
      break;
  }
  if (m_iPosition < 0) {
    m_iPosition = 0;
  } else if (m_iPosition >= m_iLength) {
    m_iPosition = m_iLength;
  }
  return m_iPosition;
}
FX_BOOL CFX_FileReadStreamImp::IsEOF() const {
  return m_iPosition >= m_iLength;
}
int32_t CFX_FileReadStreamImp::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  FXSYS_assert(m_pFileRead != NULL);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  if (iBufferSize > m_iLength - m_iPosition) {
    iBufferSize = m_iLength - m_iPosition;
  }
  if (m_pFileRead->ReadBlock(pBuffer, m_iPosition, iBufferSize)) {
    m_iPosition += iBufferSize;
    return iBufferSize;
  }
  return 0;
}
int32_t CFX_FileReadStreamImp::ReadString(FX_WCHAR* pStr,
                                          int32_t iMaxLength,
                                          FX_BOOL& bEOS) {
  FXSYS_assert(m_pFileRead != NULL);
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  iMaxLength = ReadData((uint8_t*)pStr, iMaxLength * 2) / 2;
  if (iMaxLength <= 0) {
    return 0;
  }
  int32_t i = 0;
  while (i < iMaxLength && pStr[i] != L'\0') {
    ++i;
  }
  bEOS = (m_iPosition >= m_iLength) || pStr[i] == L'\0';
  return i;
}
CFX_BufferReadStreamImp::CFX_BufferReadStreamImp()
    : m_pBufferRead(NULL),
      m_bReleaseBufferRead(FALSE),
      m_iPosition(0),
      m_iBufferSize(0) {}
CFX_BufferReadStreamImp::~CFX_BufferReadStreamImp() {
  if (m_bReleaseBufferRead && m_pBufferRead != NULL) {
    m_pBufferRead->Release();
  }
}
FX_BOOL CFX_BufferReadStreamImp::LoadBufferRead(IFX_BufferRead* pBufferRead,
                                                int32_t iFileSize,
                                                FX_DWORD dwAccess,
                                                FX_BOOL bReleaseBufferRead) {
  FXSYS_assert(m_pBufferRead == NULL && pBufferRead != NULL);
  if (dwAccess & FX_STREAMACCESS_Write) {
    return FALSE;
  }
  m_bReleaseBufferRead = bReleaseBufferRead;
  m_pBufferRead = pBufferRead;
  m_iBufferSize = iFileSize;
  if (m_iBufferSize >= 0) {
    return TRUE;
  }
  if (!m_pBufferRead->ReadNextBlock(TRUE)) {
    return FALSE;
  }
  m_iBufferSize = m_pBufferRead->GetBlockSize();
  while (!m_pBufferRead->IsEOF()) {
    m_pBufferRead->ReadNextBlock(FALSE);
    m_iBufferSize += m_pBufferRead->GetBlockSize();
  }
  return TRUE;
}
int32_t CFX_BufferReadStreamImp::GetLength() const {
  return m_iBufferSize;
}
int32_t CFX_BufferReadStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  int32_t iLength = GetLength();
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
    case FX_STREAMSEEK_End:
      m_iPosition = iLength + iOffset;
      break;
  }
  if (m_iPosition < 0) {
    m_iPosition = 0;
  } else if (m_iPosition >= iLength) {
    m_iPosition = iLength;
  }
  return m_iPosition;
}
FX_BOOL CFX_BufferReadStreamImp::IsEOF() const {
  return m_pBufferRead ? m_pBufferRead->IsEOF() : TRUE;
}
int32_t CFX_BufferReadStreamImp::ReadData(uint8_t* pBuffer,
                                          int32_t iBufferSize) {
  FXSYS_assert(m_pBufferRead != NULL);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  int32_t iLength = GetLength();
  if (m_iPosition >= iLength) {
    return 0;
  }
  if (iBufferSize > iLength - m_iPosition) {
    iBufferSize = iLength - m_iPosition;
  }
  FX_DWORD dwBlockOffset = m_pBufferRead->GetBlockOffset();
  FX_DWORD dwBlockSize = m_pBufferRead->GetBlockSize();
  if (m_iPosition < (int32_t)dwBlockOffset) {
    if (!m_pBufferRead->ReadNextBlock(TRUE)) {
      return 0;
    }
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
    dwBlockSize = m_pBufferRead->GetBlockSize();
  }
  while (m_iPosition < (int32_t)dwBlockOffset ||
         m_iPosition >= (int32_t)(dwBlockOffset + dwBlockSize)) {
    if (m_pBufferRead->IsEOF() || !m_pBufferRead->ReadNextBlock(FALSE)) {
      break;
    }
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
    dwBlockSize = m_pBufferRead->GetBlockSize();
  }
  if (m_iPosition < (int32_t)dwBlockOffset ||
      m_iPosition >= (int32_t)(dwBlockOffset + dwBlockSize)) {
    return 0;
  }
  const uint8_t* pBufferTmp = m_pBufferRead->GetBlockBuffer();
  FX_DWORD dwOffsetTmp = m_iPosition - dwBlockOffset;
  FX_DWORD dwCopySize =
      std::min(iBufferSize, (int32_t)(dwBlockSize - dwOffsetTmp));
  FXSYS_memcpy(pBuffer, pBufferTmp + dwOffsetTmp, dwCopySize);
  dwOffsetTmp = dwCopySize;
  iBufferSize -= dwCopySize;
  while (iBufferSize > 0) {
    if (!m_pBufferRead->ReadNextBlock(FALSE)) {
      break;
    }
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
    dwBlockSize = m_pBufferRead->GetBlockSize();
    pBufferTmp = m_pBufferRead->GetBlockBuffer();
    dwCopySize = std::min((FX_DWORD)iBufferSize, dwBlockSize);
    FXSYS_memcpy(pBuffer + dwOffsetTmp, pBufferTmp, dwCopySize);
    dwOffsetTmp += dwCopySize;
    iBufferSize -= dwCopySize;
  }
  m_iPosition += dwOffsetTmp;
  return dwOffsetTmp;
}
int32_t CFX_BufferReadStreamImp::ReadString(FX_WCHAR* pStr,
                                            int32_t iMaxLength,
                                            FX_BOOL& bEOS) {
  FXSYS_assert(m_pBufferRead != NULL);
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  iMaxLength = ReadData((uint8_t*)pStr, iMaxLength * 2) / 2;
  if (iMaxLength <= 0) {
    return 0;
  }
  int32_t i = 0;
  while (i < iMaxLength && pStr[i] != L'\0') {
    ++i;
  }
  bEOS = (m_iPosition >= GetLength()) || pStr[i] == L'\0';
  return i;
}
CFX_FileWriteStreamImp::CFX_FileWriteStreamImp()
    : m_pFileWrite(NULL), m_iPosition(0) {}
FX_BOOL CFX_FileWriteStreamImp::LoadFileWrite(IFX_FileWrite* pFileWrite,
                                              FX_DWORD dwAccess) {
  FXSYS_assert(m_pFileWrite == NULL && pFileWrite != NULL);
  if (dwAccess & FX_STREAMACCESS_Read) {
    return FALSE;
  }
  if (dwAccess & FX_STREAMACCESS_Append) {
    m_iPosition = pFileWrite->GetSize();
  }
  m_pFileWrite = pFileWrite;
  return TRUE;
}
int32_t CFX_FileWriteStreamImp::GetLength() const {
  if (!m_pFileWrite) {
    return 0;
  }
  return (int32_t)m_pFileWrite->GetSize();
}
int32_t CFX_FileWriteStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  int32_t iLength = GetLength();
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
    case FX_STREAMSEEK_End:
      m_iPosition = iLength + iOffset;
      break;
  }
  if (m_iPosition < 0) {
    m_iPosition = 0;
  } else if (m_iPosition >= iLength) {
    m_iPosition = iLength;
  }
  return m_iPosition;
}
FX_BOOL CFX_FileWriteStreamImp::IsEOF() const {
  return m_iPosition >= GetLength();
}
int32_t CFX_FileWriteStreamImp::WriteData(const uint8_t* pBuffer,
                                          int32_t iBufferSize) {
  if (!m_pFileWrite) {
    return 0;
  }
  if (m_pFileWrite->WriteBlock(pBuffer, m_iPosition, iBufferSize)) {
    m_iPosition += iBufferSize;
  }
  return iBufferSize;
}
int32_t CFX_FileWriteStreamImp::WriteString(const FX_WCHAR* pStr,
                                            int32_t iLength) {
  return WriteData((const uint8_t*)pStr, iLength * sizeof(FX_WCHAR));
}
void CFX_FileWriteStreamImp::Flush() {
  if (m_pFileWrite) {
    m_pFileWrite->Flush();
  }
}
CFX_BufferStreamImp::CFX_BufferStreamImp()
    : CFX_StreamImp(),
      m_pData(NULL),
      m_iTotalSize(0),
      m_iPosition(0),
      m_iLength(0) {}
FX_BOOL CFX_BufferStreamImp::LoadBuffer(uint8_t* pData,
                                        int32_t iTotalSize,
                                        FX_DWORD dwAccess) {
  FXSYS_assert(m_pData == NULL);
  FXSYS_assert(pData != NULL && iTotalSize > 0);
  m_dwAccess = dwAccess;
  m_pData = pData;
  m_iTotalSize = iTotalSize;
  m_iPosition = 0;
  m_iLength = (dwAccess & FX_STREAMACCESS_Write) != 0 ? 0 : iTotalSize;
  return TRUE;
}
int32_t CFX_BufferStreamImp::GetLength() const {
  FXSYS_assert(m_pData != NULL);
  return m_iLength;
}
int32_t CFX_BufferStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  FXSYS_assert(m_pData != NULL);
  if (eSeek == FX_STREAMSEEK_Begin) {
    m_iPosition = iOffset;
  } else if (eSeek == FX_STREAMSEEK_Current) {
    m_iPosition += iOffset;
  } else if (eSeek == FX_STREAMSEEK_End) {
    m_iPosition = m_iLength + iOffset;
  }
  if (m_iPosition > m_iLength) {
    m_iPosition = m_iLength;
  }
  if (m_iPosition < 0) {
    m_iPosition = 0;
  }
  return m_iPosition;
}
int32_t CFX_BufferStreamImp::GetPosition() {
  FXSYS_assert(m_pData != NULL);
  return m_iPosition;
}
FX_BOOL CFX_BufferStreamImp::IsEOF() const {
  FXSYS_assert(m_pData != NULL);
  return m_iPosition >= m_iLength;
}
int32_t CFX_BufferStreamImp::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  FXSYS_assert(m_pData != NULL);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  int32_t iLen = std::min(m_iLength - m_iPosition, iBufferSize);
  if (iLen <= 0) {
    return 0;
  }
  FXSYS_memcpy(pBuffer, m_pData + m_iPosition, iLen);
  m_iPosition += iLen;
  return iLen;
}
int32_t CFX_BufferStreamImp::ReadString(FX_WCHAR* pStr,
                                        int32_t iMaxLength,
                                        FX_BOOL& bEOS) {
  FXSYS_assert(m_pData != NULL);
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  int32_t iLen = std::min((m_iLength - m_iPosition) / 2, iMaxLength);
  if (iLen <= 0) {
    return 0;
  }
  const FX_WCHAR* pSrc = (const FX_WCHAR*)(FX_CHAR*)(m_pData + m_iPosition);
  int32_t iCount = 0;
  while (*pSrc != L'\0' && iCount < iLen) {
    *pStr++ = *pSrc++, iCount++;
  }
  m_iPosition += iCount * 2;
  bEOS = (*pSrc == L'\0') || (m_iPosition >= m_iLength);
  return iCount;
}
int32_t CFX_BufferStreamImp::WriteData(const uint8_t* pBuffer,
                                       int32_t iBufferSize) {
  FXSYS_assert(m_pData != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  int32_t iLen = std::min(m_iTotalSize - m_iPosition, iBufferSize);
  if (iLen <= 0) {
    return 0;
  }
  FXSYS_memcpy(m_pData + m_iPosition, pBuffer, iLen);
  m_iPosition += iLen;
  if (m_iPosition > m_iLength) {
    m_iLength = m_iPosition;
  }
  return iLen;
}
int32_t CFX_BufferStreamImp::WriteString(const FX_WCHAR* pStr,
                                         int32_t iLength) {
  FXSYS_assert(m_pData != NULL && (m_dwAccess & FX_STREAMACCESS_Write) != 0);
  FXSYS_assert(pStr != NULL && iLength > 0);
  int32_t iLen = std::min((m_iTotalSize - m_iPosition) / 2, iLength);
  if (iLen <= 0) {
    return 0;
  }
  FXSYS_memcpy(m_pData + m_iPosition, pStr, iLen * 2);
  m_iPosition += iLen * 2;
  if (m_iPosition > m_iLength) {
    m_iLength = m_iPosition;
  }
  return iLen;
}
IFX_Stream* IFX_Stream::CreateTextStream(IFX_Stream* pBaseStream,
                                         FX_BOOL bDeleteOnRelease) {
  FXSYS_assert(pBaseStream != NULL);
  return new CFX_TextStream(pBaseStream, bDeleteOnRelease);
}
CFX_TextStream::CFX_TextStream(IFX_Stream* pStream, FX_BOOL bDelStream)
    : m_wCodePage(FX_CODEPAGE_DefANSI),
      m_wBOMLength(0),
      m_dwBOM(0),
      m_pBuf(NULL),
      m_iBufSize(0),
      m_bDelStream(bDelStream),
      m_pStreamImp(pStream),
      m_iRefCount(1) {
  FXSYS_assert(m_pStreamImp != NULL);
  m_pStreamImp->Retain();
  InitStream();
}
CFX_TextStream::~CFX_TextStream() {
  m_pStreamImp->Release();
  if (m_bDelStream) {
    m_pStreamImp->Release();
  }
  if (m_pBuf != NULL) {
    FX_Free(m_pBuf);
  }
}
void CFX_TextStream::InitStream() {
  int32_t iPosition = m_pStreamImp->GetPosition();
  m_pStreamImp->Seek(FX_STREAMSEEK_Begin, 0);
  m_pStreamImp->ReadData((uint8_t*)&m_dwBOM, 3);
#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
  m_dwBOM &= 0x00FFFFFF;
  if (m_dwBOM == 0x00BFBBEF) {
    m_wBOMLength = 3;
    m_wCodePage = FX_CODEPAGE_UTF8;
  } else {
    m_dwBOM &= 0x0000FFFF;
    if (m_dwBOM == 0x0000FFFE) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16BE;
    } else if (m_dwBOM == 0x0000FEFF) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16LE;
    } else {
      m_wBOMLength = 0;
      m_dwBOM = 0;
      m_wCodePage = FXSYS_GetACP();
    }
  }
#else
  m_dwBOM &= 0xFFFFFF00;
  if (m_dwBOM == 0xEFBBBF00) {
    m_wBOMLength = 3;
    m_wCodePage = FX_CODEPAGE_UTF8;
  } else {
    m_dwBOM &= 0xFFFF0000;
    if (m_dwBOM == 0xFEFF0000) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16BE;
    } else if (m_dwBOM == 0xFFFE0000) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16LE;
    } else {
      m_wBOMLength = 0;
      m_dwBOM = 0;
      m_wCodePage = FXSYS_GetACP();
    }
  }
#endif
  m_pStreamImp->Seek(FX_STREAMSEEK_Begin, std::max(m_wBOMLength, iPosition));
}
void CFX_TextStream::Release() {
  if (--m_iRefCount < 1) {
    delete this;
  }
}
IFX_Stream* CFX_TextStream::Retain() {
  m_iRefCount++;
  return this;
}
FX_DWORD CFX_TextStream::GetAccessModes() const {
  return m_pStreamImp->GetAccessModes() | FX_STREAMACCESS_Text;
}
int32_t CFX_TextStream::GetLength() const {
  return m_pStreamImp->GetLength();
}
int32_t CFX_TextStream::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  return m_pStreamImp->Seek(eSeek, iOffset);
}
int32_t CFX_TextStream::GetPosition() {
  return m_pStreamImp->GetPosition();
}
FX_BOOL CFX_TextStream::IsEOF() const {
  return m_pStreamImp->IsEOF();
}
int32_t CFX_TextStream::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  return m_pStreamImp->ReadData(pBuffer, iBufferSize);
}
int32_t CFX_TextStream::WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
  return m_pStreamImp->WriteData(pBuffer, iBufferSize);
}
void CFX_TextStream::Flush() {
  m_pStreamImp->Flush();
}
FX_BOOL CFX_TextStream::SetLength(int32_t iLength) {
  return m_pStreamImp->SetLength(iLength);
}
FX_WORD CFX_TextStream::GetCodePage() const {
  return m_wCodePage;
}
IFX_Stream* CFX_TextStream::CreateSharedStream(FX_DWORD dwAccess,
                                               int32_t iOffset,
                                               int32_t iLength) {
  IFX_Stream* pSR =
      m_pStreamImp->CreateSharedStream(dwAccess, iOffset, iLength);
  if (pSR == NULL) {
    return NULL;
  }
  if (dwAccess & FX_STREAMACCESS_Text) {
    return new CFX_TextStream(pSR, TRUE);
  }
  return pSR;
}
int32_t CFX_TextStream::GetBOM(uint8_t bom[4]) const {
  if (m_wBOMLength < 1) {
    return 0;
  }
  *(FX_DWORD*)bom = m_dwBOM;
  return m_wBOMLength;
}
FX_WORD CFX_TextStream::SetCodePage(FX_WORD wCodePage) {
  if (m_wBOMLength > 0) {
    return m_wCodePage;
  }
  FX_WORD v = m_wCodePage;
  m_wCodePage = wCodePage;
  return v;
}
int32_t CFX_TextStream::ReadString(FX_WCHAR* pStr,
                                   int32_t iMaxLength,
                                   FX_BOOL& bEOS,
                                   int32_t const* pByteSize) {
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  if (m_pStreamImp == NULL) {
    return -1;
  }
  int32_t iLen;
  if (m_wCodePage == FX_CODEPAGE_UTF16LE ||
      m_wCodePage == FX_CODEPAGE_UTF16BE) {
    int32_t iBytes = pByteSize == NULL ? iMaxLength * 2 : *pByteSize;
    m_pStreamImp->Lock();
    iLen = m_pStreamImp->ReadData((uint8_t*)pStr, iBytes);
    m_pStreamImp->Unlock();
    iMaxLength = iLen / 2;
    if (sizeof(FX_WCHAR) > 2) {
      FX_UTF16ToWChar(pStr, iMaxLength);
    }
#if _FX_ENDIAN_ == _FX_BIG_ENDIAN_
    if (m_wCodePage == FX_CODEPAGE_UTF16LE) {
      FX_SwapByteOrder(pStr, iMaxLength);
    }
#else
    if (m_wCodePage == FX_CODEPAGE_UTF16BE) {
      FX_SwapByteOrder(pStr, iMaxLength);
    }
#endif
  } else {
    int32_t pos = m_pStreamImp->GetPosition();
    int32_t iBytes = pByteSize == NULL ? iMaxLength : *pByteSize;
    iBytes = std::min(iBytes, m_pStreamImp->GetLength() - pos);
    if (iBytes > 0) {
      if (m_pBuf == NULL) {
        m_pBuf = FX_Alloc(uint8_t, iBytes);
        m_iBufSize = iBytes;
      } else if (iBytes > m_iBufSize) {
        m_pBuf = FX_Realloc(uint8_t, m_pBuf, iBytes);
        m_iBufSize = iBytes;
      }
      m_pStreamImp->Lock();
      iLen = m_pStreamImp->ReadData(m_pBuf, iBytes);
      int32_t iSrc = iLen;
      int32_t iDecode = FX_DecodeString(m_wCodePage, (const FX_CHAR*)m_pBuf,
                                        &iSrc, pStr, &iMaxLength, TRUE);
      m_pStreamImp->Seek(FX_STREAMSEEK_Current, iSrc - iLen);
      m_pStreamImp->Unlock();
      if (iDecode < 1) {
        return -1;
      }
    } else {
      iMaxLength = 0;
    }
  }
  bEOS = m_pStreamImp->IsEOF();
  return iMaxLength;
}
int32_t CFX_TextStream::WriteString(const FX_WCHAR* pStr, int32_t iLength) {
  FXSYS_assert(pStr != NULL && iLength > 0);
  if ((m_pStreamImp->GetAccessModes() & FX_STREAMACCESS_Write) == 0) {
    return -1;
  }
  if (m_wCodePage == FX_CODEPAGE_UTF8) {
    int32_t len = iLength;
    CFX_UTF8Encoder encoder;
    while (len-- > 0) {
      encoder.Input(*pStr++);
    }
    CFX_ByteStringC bsResult = encoder.GetResult();
    m_pStreamImp->Lock();
    m_pStreamImp->WriteData((const uint8_t*)bsResult.GetCStr(),
                            bsResult.GetLength());
    m_pStreamImp->Unlock();
  }
  return iLength;
}
CFX_Stream::CFX_Stream()
    : m_eStreamType(FX_SREAMTYPE_Unknown),
      m_pStreamImp(NULL),
      m_dwAccess(0),
      m_iTotalSize(0),
      m_iPosition(0),
      m_iStart(0),
      m_iLength(0),
      m_iRefCount(1) {}
CFX_Stream::~CFX_Stream() {
  if (m_eStreamType != FX_STREAMTYPE_Stream && m_pStreamImp != NULL) {
    m_pStreamImp->Release();
  }
}
FX_BOOL CFX_Stream::LoadFile(const FX_WCHAR* pszSrcFileName,
                             FX_DWORD dwAccess) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp != NULL) {
    return FALSE;
  }
  if (pszSrcFileName == NULL || FXSYS_wcslen(pszSrcFileName) < 1) {
    return FALSE;
  }
  m_pStreamImp = new CFX_FileStreamImp();
  FX_BOOL bRet =
      ((CFX_FileStreamImp*)m_pStreamImp)->LoadFile(pszSrcFileName, dwAccess);
  if (!bRet) {
    m_pStreamImp->Release();
    m_pStreamImp = NULL;
  } else {
    m_eStreamType = FX_STREAMTYPE_File;
    m_dwAccess = dwAccess;
    m_iLength = m_pStreamImp->GetLength();
  }
  return bRet;
}
FX_BOOL CFX_Stream::LoadFileRead(IFX_FileRead* pFileRead, FX_DWORD dwAccess) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp != NULL) {
    return FALSE;
  }
  if (pFileRead == NULL) {
    return FALSE;
  }
  m_pStreamImp = new CFX_FileReadStreamImp();
  FX_BOOL bRet =
      ((CFX_FileReadStreamImp*)m_pStreamImp)->LoadFileRead(pFileRead, dwAccess);
  if (!bRet) {
    m_pStreamImp->Release();
    m_pStreamImp = NULL;
  } else {
    m_eStreamType = FX_STREAMTYPE_File;
    m_dwAccess = dwAccess;
    m_iLength = m_pStreamImp->GetLength();
  }
  return bRet;
}
FX_BOOL CFX_Stream::LoadFileWrite(IFX_FileWrite* pFileWrite,
                                  FX_DWORD dwAccess) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp != NULL) {
    return FALSE;
  }
  if (pFileWrite == NULL) {
    return FALSE;
  }
  m_pStreamImp = new CFX_FileWriteStreamImp();
  FX_BOOL bRet = ((CFX_FileWriteStreamImp*)m_pStreamImp)
                     ->LoadFileWrite(pFileWrite, dwAccess);
  if (!bRet) {
    m_pStreamImp->Release();
    m_pStreamImp = NULL;
  } else {
    m_eStreamType = FX_STREAMTYPE_File;
    m_dwAccess = dwAccess;
    m_iLength = m_pStreamImp->GetLength();
  }
  return bRet;
}
FX_BOOL CFX_Stream::LoadBuffer(uint8_t* pData,
                               int32_t iTotalSize,
                               FX_DWORD dwAccess) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp != NULL) {
    return FALSE;
  }
  if (pData == NULL || iTotalSize < 1) {
    return FALSE;
  }
  m_pStreamImp = new CFX_BufferStreamImp();
  FX_BOOL bRet = ((CFX_BufferStreamImp*)m_pStreamImp)
                     ->LoadBuffer(pData, iTotalSize, dwAccess);
  if (!bRet) {
    m_pStreamImp->Release();
    m_pStreamImp = NULL;
  } else {
    m_eStreamType = FX_STREAMTYPE_Buffer;
    m_dwAccess = dwAccess;
    m_iLength = m_pStreamImp->GetLength();
  }
  return bRet;
}
FX_BOOL CFX_Stream::LoadBufferRead(IFX_BufferRead* pBufferRead,
                                   int32_t iFileSize,
                                   FX_DWORD dwAccess,
                                   FX_BOOL bReleaseBufferRead) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp != NULL) {
    return FALSE;
  }
  if (!pBufferRead) {
    return FALSE;
  }
  m_pStreamImp = new CFX_BufferReadStreamImp;
  FX_BOOL bRet = ((CFX_BufferReadStreamImp*)m_pStreamImp)
                     ->LoadBufferRead(pBufferRead, iFileSize, dwAccess,
                                      bReleaseBufferRead);
  if (!bRet) {
    m_pStreamImp->Release();
    m_pStreamImp = NULL;
  } else {
    m_eStreamType = FX_STREAMTYPE_BufferRead;
    m_dwAccess = dwAccess;
    m_iLength = m_pStreamImp->GetLength();
  }
  return bRet;
}
void CFX_Stream::Release() {
  if (--m_iRefCount < 1) {
    delete this;
  }
}
IFX_Stream* CFX_Stream::Retain() {
  m_iRefCount++;
  return this;
}
int32_t CFX_Stream::GetLength() const {
  if (m_pStreamImp == NULL) {
    return -1;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_pStreamImp->GetLength();
  }
  return m_iLength;
}
int32_t CFX_Stream::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  if (m_pStreamImp == NULL) {
    return -1;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_iPosition = m_pStreamImp->Seek(eSeek, iOffset);
  }
  int32_t iEnd = m_iStart + m_iLength;
  int32_t iPosition = m_iStart + iOffset;
  if (eSeek == FX_STREAMSEEK_Begin) {
    m_iPosition = iPosition;
  } else if (eSeek == FX_STREAMSEEK_Current) {
    m_iPosition += iOffset;
  } else if (eSeek == FX_STREAMSEEK_End) {
    m_iPosition = iEnd + iOffset;
  }
  if (m_iPosition > iEnd) {
    m_iPosition = iEnd;
  }
  if (m_iPosition < m_iStart) {
    m_iPosition = m_iStart;
  }
  return m_iPosition - m_iStart;
}
int32_t CFX_Stream::GetPosition() {
  if (m_pStreamImp == NULL) {
    return -1;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_iPosition = m_pStreamImp->GetPosition();
  }
  return m_iPosition - m_iStart;
}
FX_BOOL CFX_Stream::IsEOF() const {
  if (m_pStreamImp == NULL) {
    return TRUE;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_pStreamImp->IsEOF();
  }
  return m_iPosition >= m_iStart + m_iLength;
}
int32_t CFX_Stream::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  if (m_pStreamImp == NULL) {
    return -1;
  }
  int32_t iLen = std::min(m_iStart + m_iLength - m_iPosition, iBufferSize);
  if (iLen <= 0) {
    return 0;
  }
  m_pStreamImp->Lock();
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->ReadData(pBuffer, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  m_pStreamImp->Unlock();
  return iLen;
}
int32_t CFX_Stream::ReadString(FX_WCHAR* pStr,
                               int32_t iMaxLength,
                               FX_BOOL& bEOS,
                               int32_t const* pByteSize) {
  FXSYS_assert(pStr != NULL && iMaxLength > 0);
  if (m_pStreamImp == NULL) {
    return -1;
  }
  int32_t iEnd = m_iStart + m_iLength;
  int32_t iLen = iEnd - m_iPosition;
  if (pByteSize != NULL) {
    iLen = std::min(iLen, *pByteSize);
  }
  iLen = std::min(iEnd / 2, iMaxLength);
  if (iLen <= 0) {
    return 0;
  }
  m_pStreamImp->Lock();
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->ReadString(pStr, iLen, bEOS);
  m_iPosition = m_pStreamImp->GetPosition();
  if (iLen > 0 && m_iPosition >= iEnd) {
    bEOS = TRUE;
  }
  m_pStreamImp->Unlock();
  return iLen;
}
int32_t CFX_Stream::WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
  FXSYS_assert(pBuffer != NULL && iBufferSize > 0);
  if (m_pStreamImp == NULL) {
    return -1;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return -1;
  }
  int32_t iLen = iBufferSize;
  if (m_eStreamType == FX_STREAMTYPE_Stream) {
    iLen = std::min(m_iStart + m_iTotalSize - m_iPosition, iBufferSize);
    if (iLen <= 0) {
      return 0;
    }
  }
  m_pStreamImp->Lock();
  int32_t iEnd = m_iStart + m_iLength;
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->WriteData(pBuffer, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  if (m_iPosition > iEnd) {
    m_iLength = m_iPosition - m_iStart;
  }
  m_pStreamImp->Unlock();
  return iLen;
}
int32_t CFX_Stream::WriteString(const FX_WCHAR* pStr, int32_t iLength) {
  FXSYS_assert(pStr != NULL && iLength > 0);
  if (m_pStreamImp == NULL) {
    return -1;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return -1;
  }
  int32_t iLen = iLength;
  if (m_eStreamType == FX_STREAMTYPE_Stream) {
    iLen = std::min((m_iStart + m_iTotalSize - m_iPosition) / 2, iLength);
    if (iLen <= 0) {
      return 0;
    }
  }
  m_pStreamImp->Lock();
  int32_t iEnd = m_iStart + m_iLength;
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->WriteString(pStr, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  if (m_iPosition > iEnd) {
    m_iLength = m_iPosition - m_iStart;
  }
  m_pStreamImp->Unlock();
  return iLen;
}
void CFX_Stream::Flush() {
  if (m_pStreamImp == NULL) {
    return;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return;
  }
  m_pStreamImp->Flush();
}
FX_BOOL CFX_Stream::SetLength(int32_t iLength) {
  if (m_pStreamImp == NULL) {
    return FALSE;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return FALSE;
  }
  return m_pStreamImp->SetLength(iLength);
}
int32_t CFX_Stream::GetBOM(uint8_t bom[4]) const {
  if (m_pStreamImp == NULL) {
    return -1;
  }
  return 0;
}
FX_WORD CFX_Stream::GetCodePage() const {
#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
  return FX_CODEPAGE_UTF16LE;
#else
  return FX_CODEPAGE_UTF16BE;
#endif
}
FX_WORD CFX_Stream::SetCodePage(FX_WORD wCodePage) {
#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
  return FX_CODEPAGE_UTF16LE;
#else
  return FX_CODEPAGE_UTF16BE;
#endif
}
IFX_Stream* CFX_Stream::CreateSharedStream(FX_DWORD dwAccess,
                                           int32_t iOffset,
                                           int32_t iLength) {
  FXSYS_assert(iLength > 0);
  if (m_pStreamImp == NULL) {
    return NULL;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Text) != 0 &&
      (dwAccess & FX_STREAMACCESS_Text) == 0) {
    return NULL;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0 &&
      (dwAccess & FX_STREAMACCESS_Write) != 0) {
    return NULL;
  }
  int32_t iStart = m_iStart + iOffset;
  int32_t iTotal = m_iStart + m_iLength;
  if (iStart < m_iStart || iStart >= iTotal) {
    return NULL;
  }
  int32_t iEnd = iStart + iLength;
  if (iEnd < iStart || iEnd > iTotal) {
    return NULL;
  }
  CFX_Stream* pShared = new CFX_Stream;
  pShared->m_eStreamType = FX_STREAMTYPE_Stream;
  pShared->m_pStreamImp = m_pStreamImp;
  pShared->m_dwAccess = dwAccess;
  pShared->m_iTotalSize = iLength;
  pShared->m_iPosition = iStart;
  pShared->m_iStart = iStart;
  pShared->m_iLength = (dwAccess & FX_STREAMACCESS_Write) != 0 ? 0 : iLength;
  if (dwAccess & FX_STREAMACCESS_Text) {
    return IFX_Stream::CreateTextStream(pShared, TRUE);
  }
  return pShared;
}
IFX_FileRead* FX_CreateFileRead(IFX_Stream* pBaseStream,
                                FX_BOOL bReleaseStream) {
  FXSYS_assert(pBaseStream != NULL);
  return new CFGAS_FileRead(pBaseStream, bReleaseStream);
}
CFGAS_FileRead::CFGAS_FileRead(IFX_Stream* pStream, FX_BOOL bReleaseStream)
    : m_bReleaseStream(bReleaseStream), m_pStream(pStream) {
  FXSYS_assert(m_pStream != NULL);
}
CFGAS_FileRead::~CFGAS_FileRead() {
  if (m_bReleaseStream) {
    m_pStream->Release();
  }
}
FX_FILESIZE CFGAS_FileRead::GetSize() {
  return (FX_FILESIZE)m_pStream->GetLength();
}
FX_BOOL CFGAS_FileRead::ReadBlock(void* buffer,
                                  FX_FILESIZE offset,
                                  size_t size) {
  m_pStream->Lock();
  m_pStream->Seek(FX_STREAMSEEK_Begin, (int32_t)offset);
  int32_t iLen = m_pStream->ReadData((uint8_t*)buffer, (int32_t)size);
  m_pStream->Unlock();
  return iLen == (int32_t)size;
}

IFX_FileRead* FX_CreateFileRead(IFX_BufferRead* pBufferRead,
                                FX_FILESIZE iFileSize,
                                FX_BOOL bReleaseStream) {
  if (!pBufferRead) {
    return NULL;
  }
  return new CFX_BufferAccImp(pBufferRead, iFileSize, bReleaseStream);
}
CFX_BufferAccImp::CFX_BufferAccImp(IFX_BufferRead* pBufferRead,
                                   FX_FILESIZE iFileSize,
                                   FX_BOOL bReleaseStream)
    : m_pBufferRead(pBufferRead),
      m_bReleaseStream(bReleaseStream),
      m_iBufSize(iFileSize) {
  FXSYS_assert(m_pBufferRead);
}
CFX_BufferAccImp::~CFX_BufferAccImp() {
  if (m_bReleaseStream && m_pBufferRead) {
    m_pBufferRead->Release();
  }
}
FX_FILESIZE CFX_BufferAccImp::GetSize() {
  if (!m_pBufferRead) {
    return 0;
  }
  if (m_iBufSize >= 0) {
    return m_iBufSize;
  }
  if (!m_pBufferRead->ReadNextBlock(TRUE)) {
    return 0;
  }
  m_iBufSize = (FX_FILESIZE)m_pBufferRead->GetBlockSize();
  while (!m_pBufferRead->IsEOF()) {
    m_pBufferRead->ReadNextBlock(FALSE);
    m_iBufSize += (FX_FILESIZE)m_pBufferRead->GetBlockSize();
  }
  return m_iBufSize;
}
FX_BOOL CFX_BufferAccImp::ReadBlock(void* buffer,
                                    FX_FILESIZE offset,
                                    size_t size) {
  if (!m_pBufferRead) {
    return FALSE;
  }
  if (!buffer || !size) {
    return TRUE;
  }
  FX_FILESIZE dwBufSize = GetSize();
  if (offset >= dwBufSize) {
    return FALSE;
  }
  size_t dwBlockSize = m_pBufferRead->GetBlockSize();
  FX_FILESIZE dwBlockOffset = m_pBufferRead->GetBlockOffset();
  if (offset < dwBlockOffset) {
    if (!m_pBufferRead->ReadNextBlock(TRUE)) {
      return FALSE;
    }
    dwBlockSize = m_pBufferRead->GetBlockSize();
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
  }
  while (offset < dwBlockOffset ||
         offset >= (FX_FILESIZE)(dwBlockOffset + dwBlockSize)) {
    if (m_pBufferRead->IsEOF() || !m_pBufferRead->ReadNextBlock(FALSE)) {
      break;
    }
    dwBlockSize = m_pBufferRead->GetBlockSize();
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
  }
  if (offset < dwBlockOffset ||
      offset >= (FX_FILESIZE)(dwBlockOffset + dwBlockSize)) {
    return FALSE;
  }
  const uint8_t* pBuffer = m_pBufferRead->GetBlockBuffer();
  const FX_FILESIZE dwOffset = offset - dwBlockOffset;
  size_t dwCopySize =
      std::min(size, static_cast<size_t>(dwBlockSize - dwOffset));
  FXSYS_memcpy(buffer, pBuffer + dwOffset, dwCopySize);
  offset = dwCopySize;
  size -= dwCopySize;
  while (size) {
    if (!m_pBufferRead->ReadNextBlock(FALSE)) {
      break;
    }
    dwBlockOffset = m_pBufferRead->GetBlockOffset();
    dwBlockSize = m_pBufferRead->GetBlockSize();
    pBuffer = m_pBufferRead->GetBlockBuffer();
    dwCopySize = std::min(size, dwBlockSize);
    FXSYS_memcpy(((uint8_t*)buffer) + offset, pBuffer, dwCopySize);
    offset += dwCopySize;
    size -= dwCopySize;
  }
  return TRUE;
}

IFX_FileWrite* FX_CreateFileWrite(IFX_Stream* pBaseStream,
                                  FX_BOOL bReleaseStream) {
  FXSYS_assert(pBaseStream != NULL);
  return new CFGAS_FileWrite(pBaseStream, bReleaseStream);
}

CFGAS_FileWrite::CFGAS_FileWrite(IFX_Stream* pStream, FX_BOOL bReleaseStream)
    : m_pStream(pStream), m_bReleaseStream(bReleaseStream) {
  FXSYS_assert(m_pStream != NULL);
}
CFGAS_FileWrite::~CFGAS_FileWrite() {
  if (m_bReleaseStream) {
    m_pStream->Release();
  }
}
FX_FILESIZE CFGAS_FileWrite::GetSize() {
  return m_pStream->GetLength();
}
FX_BOOL CFGAS_FileWrite::Flush() {
  m_pStream->Flush();
  return TRUE;
}
FX_BOOL CFGAS_FileWrite::WriteBlock(const void* pData, size_t size) {
  return m_pStream->WriteData((const uint8_t*)pData, (int32_t)size) ==
         (int32_t)size;
}
FX_BOOL CFGAS_FileWrite::WriteBlock(const void* pData,
                                    FX_FILESIZE offset,
                                    size_t size) {
  m_pStream->Lock();
  m_pStream->Seek(FX_STREAMSEEK_Begin, offset);
  int32_t iLen = m_pStream->WriteData((uint8_t*)pData, (int32_t)size);
  m_pStream->Unlock();
  return iLen == (int32_t)size;
}
