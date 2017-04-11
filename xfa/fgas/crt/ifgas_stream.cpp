// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/ifgas_stream.h"

#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
#include <io.h>
#endif

#include <algorithm>
#include <memory>

#include "third_party/base/ptr_util.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

class IFGAS_StreamImp {
 public:
  virtual ~IFGAS_StreamImp() {}

  virtual int32_t GetLength() const = 0;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) = 0;
  virtual int32_t GetPosition() = 0;
  virtual bool IsEOF() const = 0;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) = 0;
  virtual int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) = 0;
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) = 0;
  virtual int32_t WriteString(const wchar_t* pStr, int32_t iLength) = 0;
  virtual void Flush() = 0;
  virtual bool SetLength(int32_t iLength) = 0;

 protected:
  IFGAS_StreamImp();

  uint32_t GetAccessModes() const { return m_dwAccess; }

 private:
  uint32_t m_dwAccess;
};

class CFGAS_BufferStreamImp : public IFGAS_StreamImp {
 public:
  CFGAS_BufferStreamImp();
  ~CFGAS_BufferStreamImp() override {}

  bool LoadBuffer(uint8_t* pData, int32_t iTotalSize);

  // IFGAS_StreamImp:
  int32_t GetLength() const override;
  int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) override;
  int32_t GetPosition() override;
  bool IsEOF() const override;
  int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) override;
  int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t WriteString(const wchar_t* pStr, int32_t iLength) override;
  void Flush() override {}
  bool SetLength(int32_t iLength) override { return false; }

 protected:
  uint8_t* m_pData;
  int32_t m_iTotalSize;
  int32_t m_iPosition;
  int32_t m_iLength;
};

class CFGAS_FileReadStreamImp : public IFGAS_StreamImp {
 public:
  CFGAS_FileReadStreamImp();
  ~CFGAS_FileReadStreamImp() override {}

  bool LoadFileRead(const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead);

  // IFGAS_StreamImp:
  int32_t GetLength() const override;
  int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) override;
  int32_t GetPosition() override { return m_iPosition; }
  bool IsEOF() const override;
  int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) override;
  int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) override {
    return 0;
  }
  int32_t WriteString(const wchar_t* pStr, int32_t iLength) override {
    return 0;
  }
  void Flush() override {}
  bool SetLength(int32_t iLength) override { return false; }

 protected:
  CFX_RetainPtr<IFX_SeekableReadStream> m_pFileRead;
  int32_t m_iPosition;
  int32_t m_iLength;
};

class CFGAS_FileWriteStreamImp : public IFGAS_StreamImp {
 public:
  CFGAS_FileWriteStreamImp();
  ~CFGAS_FileWriteStreamImp() override {}

  bool LoadFileWrite(const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite,
                     uint32_t dwAccess);

  // IFGAS_StreamImp:
  int32_t GetLength() const override;
  int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) override;
  int32_t GetPosition() override { return m_iPosition; }
  bool IsEOF() const override;
  int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) override { return 0; }
  int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) override {
    return 0;
  }
  int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t WriteString(const wchar_t* pStr, int32_t iLength) override;
  void Flush() override;
  bool SetLength(int32_t iLength) override { return false; }

 protected:
  CFX_RetainPtr<IFX_SeekableWriteStream> m_pFileWrite;
  int32_t m_iPosition;
};

enum FX_STREAMTYPE {
  FX_SREAMTYPE_Unknown = 0,
  FX_STREAMTYPE_File,
  FX_STREAMTYPE_Buffer,
  FX_STREAMTYPE_Stream,
  FX_STREAMTYPE_BufferRead,
};

class CFGAS_Stream : public IFGAS_Stream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  bool LoadBuffer(uint8_t* pData, int32_t iTotalSize);
  bool LoadFileRead(const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead);
  bool LoadFileWrite(const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite,
                     uint32_t dwAccess);

  // IFGAS_Stream
  uint32_t GetAccessModes() const override;
  int32_t GetLength() const override;
  int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) override;
  int32_t GetPosition() override;
  bool IsEOF() const override;
  int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) override;
  int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t WriteString(const wchar_t* pStr, int32_t iLength) override;
  void Flush() override;
  bool SetLength(int32_t iLength) override;
  int32_t GetBOM(uint8_t bom[4]) const override;
  uint16_t GetCodePage() const override;
  uint16_t SetCodePage(uint16_t wCodePage) override;

 protected:
  CFGAS_Stream();
  ~CFGAS_Stream() override;

  FX_STREAMTYPE m_eStreamType;
  IFGAS_StreamImp* m_pStreamImp;
  uint32_t m_dwAccess;
  int32_t m_iTotalSize;
  int32_t m_iPosition;
  int32_t m_iStart;
  int32_t m_iLength;
  int32_t m_iRefCount;
};

class CFGAS_TextStream : public IFGAS_Stream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFGAS_Stream
  uint32_t GetAccessModes() const override;
  int32_t GetLength() const override;
  int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) override;
  int32_t GetPosition() override;
  bool IsEOF() const override;
  int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t ReadString(wchar_t* pStr, int32_t iMaxLength, bool& bEOS) override;
  int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) override;
  int32_t WriteString(const wchar_t* pStr, int32_t iLength) override;
  void Flush() override;
  bool SetLength(int32_t iLength) override;
  int32_t GetBOM(uint8_t bom[4]) const override;
  uint16_t GetCodePage() const override;
  uint16_t SetCodePage(uint16_t wCodePage) override;

 protected:
  explicit CFGAS_TextStream(const CFX_RetainPtr<IFGAS_Stream>& pStream);
  ~CFGAS_TextStream() override;

  void InitStream();

  uint16_t m_wCodePage;
  int32_t m_wBOMLength;
  uint32_t m_dwBOM;
  uint8_t* m_pBuf;
  int32_t m_iBufSize;
  CFX_RetainPtr<IFGAS_Stream> m_pStreamImp;
};

class CFGAS_FileRead : public IFX_SeekableReadStream {
 public:
  explicit CFGAS_FileRead(const CFX_RetainPtr<IFGAS_Stream>& pStream);
  ~CFGAS_FileRead() override;

  // IFX_SeekableReadStream
  FX_FILESIZE GetSize() override;
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

 protected:
  CFX_RetainPtr<IFGAS_Stream> m_pStream;
};

IFGAS_StreamImp::IFGAS_StreamImp() : m_dwAccess(0) {}

CFGAS_FileReadStreamImp::CFGAS_FileReadStreamImp()
    : m_pFileRead(nullptr), m_iPosition(0), m_iLength(0) {}

bool CFGAS_FileReadStreamImp::LoadFileRead(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead) {
  ASSERT(!m_pFileRead && pFileRead);

  m_pFileRead = pFileRead;
  m_iLength = m_pFileRead->GetSize();
  return true;
}

int32_t CFGAS_FileReadStreamImp::GetLength() const {
  return m_iLength;
}
int32_t CFGAS_FileReadStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
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
bool CFGAS_FileReadStreamImp::IsEOF() const {
  return m_iPosition >= m_iLength;
}
int32_t CFGAS_FileReadStreamImp::ReadData(uint8_t* pBuffer,
                                          int32_t iBufferSize) {
  ASSERT(m_pFileRead);
  ASSERT(pBuffer && iBufferSize > 0);
  if (iBufferSize > m_iLength - m_iPosition) {
    iBufferSize = m_iLength - m_iPosition;
  }
  if (m_pFileRead->ReadBlock(pBuffer, m_iPosition, iBufferSize)) {
    m_iPosition += iBufferSize;
    return iBufferSize;
  }
  return 0;
}
int32_t CFGAS_FileReadStreamImp::ReadString(wchar_t* pStr,
                                            int32_t iMaxLength,
                                            bool& bEOS) {
  ASSERT(m_pFileRead);
  ASSERT(pStr && iMaxLength > 0);
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

CFGAS_FileWriteStreamImp::CFGAS_FileWriteStreamImp()
    : m_pFileWrite(nullptr), m_iPosition(0) {}

bool CFGAS_FileWriteStreamImp::LoadFileWrite(
    const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite,
    uint32_t dwAccess) {
  ASSERT(!m_pFileWrite && pFileWrite);

  m_iPosition = pFileWrite->GetSize();
  m_pFileWrite = pFileWrite;
  return true;
}

int32_t CFGAS_FileWriteStreamImp::GetLength() const {
  if (!m_pFileWrite)
    return 0;

  return (int32_t)m_pFileWrite->GetSize();
}
int32_t CFGAS_FileWriteStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
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
bool CFGAS_FileWriteStreamImp::IsEOF() const {
  return m_iPosition >= GetLength();
}
int32_t CFGAS_FileWriteStreamImp::WriteData(const uint8_t* pBuffer,
                                            int32_t iBufferSize) {
  if (!m_pFileWrite) {
    return 0;
  }
  if (m_pFileWrite->WriteBlock(pBuffer, m_iPosition, iBufferSize)) {
    m_iPosition += iBufferSize;
  }
  return iBufferSize;
}
int32_t CFGAS_FileWriteStreamImp::WriteString(const wchar_t* pStr,
                                              int32_t iLength) {
  return WriteData((const uint8_t*)pStr, iLength * sizeof(wchar_t));
}
void CFGAS_FileWriteStreamImp::Flush() {
  if (m_pFileWrite) {
    m_pFileWrite->Flush();
  }
}
CFGAS_BufferStreamImp::CFGAS_BufferStreamImp()
    : m_pData(nullptr), m_iTotalSize(0), m_iPosition(0), m_iLength(0) {}

bool CFGAS_BufferStreamImp::LoadBuffer(uint8_t* pData, int32_t iTotalSize) {
  ASSERT(!m_pData && pData && iTotalSize > 0);

  m_pData = pData;
  m_iTotalSize = iTotalSize;
  m_iPosition = 0;
  m_iLength = iTotalSize;
  return true;
}
int32_t CFGAS_BufferStreamImp::GetLength() const {
  ASSERT(m_pData);
  return m_iLength;
}
int32_t CFGAS_BufferStreamImp::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  ASSERT(m_pData);
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
int32_t CFGAS_BufferStreamImp::GetPosition() {
  ASSERT(m_pData);
  return m_iPosition;
}
bool CFGAS_BufferStreamImp::IsEOF() const {
  ASSERT(m_pData);
  return m_iPosition >= m_iLength;
}
int32_t CFGAS_BufferStreamImp::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  ASSERT(m_pData);
  ASSERT(pBuffer && iBufferSize > 0);
  int32_t iLen = std::min(m_iLength - m_iPosition, iBufferSize);
  if (iLen <= 0) {
    return 0;
  }
  memcpy(pBuffer, m_pData + m_iPosition, iLen);
  m_iPosition += iLen;
  return iLen;
}
int32_t CFGAS_BufferStreamImp::ReadString(wchar_t* pStr,
                                          int32_t iMaxLength,
                                          bool& bEOS) {
  ASSERT(m_pData);
  ASSERT(pStr && iMaxLength > 0);
  int32_t iLen = std::min((m_iLength - m_iPosition) / 2, iMaxLength);
  if (iLen <= 0) {
    return 0;
  }
  const wchar_t* pSrc = (const wchar_t*)(char*)(m_pData + m_iPosition);
  int32_t iCount = 0;
  while (*pSrc && iCount < iLen) {
    *pStr++ = *pSrc++;
    iCount++;
  }
  m_iPosition += iCount * 2;
  bEOS = (*pSrc == L'\0') || (m_iPosition >= m_iLength);
  return iCount;
}

int32_t CFGAS_BufferStreamImp::WriteData(const uint8_t* pBuffer,
                                         int32_t iBufferSize) {
  ASSERT(false);
  return 0;
}

int32_t CFGAS_BufferStreamImp::WriteString(const wchar_t* pStr,
                                           int32_t iLength) {
  ASSERT(false);
  return 0;
}

CFGAS_TextStream::CFGAS_TextStream(const CFX_RetainPtr<IFGAS_Stream>& pStream)
    : m_wCodePage(FX_CODEPAGE_DefANSI),
      m_wBOMLength(0),
      m_dwBOM(0),
      m_pBuf(nullptr),
      m_iBufSize(0),
      m_pStreamImp(pStream) {
  ASSERT(m_pStreamImp);
  InitStream();
}

CFGAS_TextStream::~CFGAS_TextStream() {
  if (m_pBuf)
    FX_Free(m_pBuf);
}

void CFGAS_TextStream::InitStream() {
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

uint32_t CFGAS_TextStream::GetAccessModes() const {
  return m_pStreamImp->GetAccessModes();
}

int32_t CFGAS_TextStream::GetLength() const {
  return m_pStreamImp->GetLength();
}

int32_t CFGAS_TextStream::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  return m_pStreamImp->Seek(eSeek, iOffset);
}

int32_t CFGAS_TextStream::GetPosition() {
  return m_pStreamImp->GetPosition();
}

bool CFGAS_TextStream::IsEOF() const {
  return m_pStreamImp->IsEOF();
}

int32_t CFGAS_TextStream::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  return m_pStreamImp->ReadData(pBuffer, iBufferSize);
}

int32_t CFGAS_TextStream::WriteData(const uint8_t* pBuffer,
                                    int32_t iBufferSize) {
  return m_pStreamImp->WriteData(pBuffer, iBufferSize);
}

void CFGAS_TextStream::Flush() {
  m_pStreamImp->Flush();
}

bool CFGAS_TextStream::SetLength(int32_t iLength) {
  return m_pStreamImp->SetLength(iLength);
}

uint16_t CFGAS_TextStream::GetCodePage() const {
  return m_wCodePage;
}

int32_t CFGAS_TextStream::GetBOM(uint8_t bom[4]) const {
  if (m_wBOMLength < 1)
    return 0;

  *(uint32_t*)bom = m_dwBOM;
  return m_wBOMLength;
}

uint16_t CFGAS_TextStream::SetCodePage(uint16_t wCodePage) {
  if (m_wBOMLength > 0)
    return m_wCodePage;

  uint16_t v = m_wCodePage;
  m_wCodePage = wCodePage;
  return v;
}

int32_t CFGAS_TextStream::ReadString(wchar_t* pStr,
                                     int32_t iMaxLength,
                                     bool& bEOS) {
  ASSERT(pStr && iMaxLength > 0);
  if (!m_pStreamImp) {
    return -1;
  }
  int32_t iLen;
  if (m_wCodePage == FX_CODEPAGE_UTF16LE ||
      m_wCodePage == FX_CODEPAGE_UTF16BE) {
    int32_t iBytes = iMaxLength * 2;
    iLen = m_pStreamImp->ReadData((uint8_t*)pStr, iBytes);
    iMaxLength = iLen / 2;
    if (sizeof(wchar_t) > 2) {
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
    int32_t iBytes = iMaxLength;
    iBytes = std::min(iBytes, m_pStreamImp->GetLength() - pos);
    if (iBytes > 0) {
      if (!m_pBuf) {
        m_pBuf = FX_Alloc(uint8_t, iBytes);
        m_iBufSize = iBytes;
      } else if (iBytes > m_iBufSize) {
        m_pBuf = FX_Realloc(uint8_t, m_pBuf, iBytes);
        m_iBufSize = iBytes;
      }
      iLen = m_pStreamImp->ReadData(m_pBuf, iBytes);
      int32_t iSrc = iLen;
      int32_t iDecode = FX_DecodeString(m_wCodePage, (const char*)m_pBuf, &iSrc,
                                        pStr, &iMaxLength, true);
      m_pStreamImp->Seek(FX_STREAMSEEK_Current, iSrc - iLen);
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

int32_t CFGAS_TextStream::WriteString(const wchar_t* pStr, int32_t iLength) {
  ASSERT(pStr && iLength > 0);
  if ((m_pStreamImp->GetAccessModes() & FX_STREAMACCESS_Write) == 0)
    return -1;

  if (m_wCodePage == FX_CODEPAGE_UTF8) {
    int32_t len = iLength;
    CFX_UTF8Encoder encoder;
    while (len-- > 0) {
      encoder.Input(*pStr++);
    }
    CFX_ByteStringC bsResult = encoder.GetResult();
    m_pStreamImp->WriteData((const uint8_t*)bsResult.c_str(),
                            bsResult.GetLength());
  }
  return iLength;
}

CFGAS_Stream::CFGAS_Stream()
    : m_eStreamType(FX_SREAMTYPE_Unknown),
      m_pStreamImp(nullptr),
      m_dwAccess(0),
      m_iTotalSize(0),
      m_iPosition(0),
      m_iStart(0),
      m_iLength(0),
      m_iRefCount(1) {}

CFGAS_Stream::~CFGAS_Stream() {
  if (m_eStreamType != FX_STREAMTYPE_Stream)
    delete m_pStreamImp;
}

bool CFGAS_Stream::LoadFileRead(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp)
    return false;

  if (!pFileRead)
    return false;

  auto pImp = pdfium::MakeUnique<CFGAS_FileReadStreamImp>();
  if (!pImp->LoadFileRead(pFileRead))
    return false;

  m_pStreamImp = pImp.release();
  m_eStreamType = FX_STREAMTYPE_File;
  m_dwAccess = 0;
  m_iLength = m_pStreamImp->GetLength();
  return true;
}

bool CFGAS_Stream::LoadFileWrite(
    const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite,
    uint32_t dwAccess) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp)
    return false;

  if (!pFileWrite)
    return false;

  auto pImp = pdfium::MakeUnique<CFGAS_FileWriteStreamImp>();
  if (!pImp->LoadFileWrite(pFileWrite, dwAccess))
    return false;

  m_pStreamImp = pImp.release();
  m_eStreamType = FX_STREAMTYPE_File;
  m_dwAccess = dwAccess;
  m_iLength = m_pStreamImp->GetLength();
  return true;
}

bool CFGAS_Stream::LoadBuffer(uint8_t* pData, int32_t iTotalSize) {
  if (m_eStreamType != FX_SREAMTYPE_Unknown || m_pStreamImp)
    return false;

  if (!pData || iTotalSize < 1)
    return false;

  auto pImp = pdfium::MakeUnique<CFGAS_BufferStreamImp>();
  if (!pImp->LoadBuffer(pData, iTotalSize))
    return false;

  m_pStreamImp = pImp.release();
  m_eStreamType = FX_STREAMTYPE_Buffer;
  m_dwAccess = 0;
  m_iLength = m_pStreamImp->GetLength();
  return true;
}

uint32_t CFGAS_Stream::GetAccessModes() const {
  return m_dwAccess;
}

int32_t CFGAS_Stream::GetLength() const {
  if (!m_pStreamImp) {
    return -1;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_pStreamImp->GetLength();
  }
  return m_iLength;
}
int32_t CFGAS_Stream::Seek(FX_STREAMSEEK eSeek, int32_t iOffset) {
  if (!m_pStreamImp) {
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
int32_t CFGAS_Stream::GetPosition() {
  if (!m_pStreamImp) {
    return -1;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_iPosition = m_pStreamImp->GetPosition();
  }
  return m_iPosition - m_iStart;
}
bool CFGAS_Stream::IsEOF() const {
  if (!m_pStreamImp) {
    return true;
  }
  if (m_eStreamType == FX_STREAMTYPE_File ||
      m_eStreamType == FX_STREAMTYPE_Buffer) {
    return m_pStreamImp->IsEOF();
  }
  return m_iPosition >= m_iStart + m_iLength;
}
int32_t CFGAS_Stream::ReadData(uint8_t* pBuffer, int32_t iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);
  if (!m_pStreamImp) {
    return -1;
  }
  int32_t iLen = std::min(m_iStart + m_iLength - m_iPosition, iBufferSize);
  if (iLen <= 0) {
    return 0;
  }
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->ReadData(pBuffer, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  return iLen;
}
int32_t CFGAS_Stream::ReadString(wchar_t* pStr,
                                 int32_t iMaxLength,
                                 bool& bEOS) {
  ASSERT(pStr && iMaxLength > 0);
  if (!m_pStreamImp) {
    return -1;
  }
  int32_t iEnd = m_iStart + m_iLength;
  int32_t iLen = iEnd - m_iPosition;
  iLen = std::min(iEnd / 2, iMaxLength);
  if (iLen <= 0) {
    return 0;
  }
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->ReadString(pStr, iLen, bEOS);
  m_iPosition = m_pStreamImp->GetPosition();
  if (iLen > 0 && m_iPosition >= iEnd) {
    bEOS = true;
  }
  return iLen;
}

int32_t CFGAS_Stream::WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);
  if (!m_pStreamImp) {
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
  int32_t iEnd = m_iStart + m_iLength;
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->WriteData(pBuffer, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  if (m_iPosition > iEnd) {
    m_iLength = m_iPosition - m_iStart;
  }
  return iLen;
}
int32_t CFGAS_Stream::WriteString(const wchar_t* pStr, int32_t iLength) {
  ASSERT(pStr && iLength > 0);
  if (!m_pStreamImp) {
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
  int32_t iEnd = m_iStart + m_iLength;
  if (m_pStreamImp->GetPosition() != m_iPosition) {
    m_pStreamImp->Seek(FX_STREAMSEEK_Begin, m_iPosition);
  }
  iLen = m_pStreamImp->WriteString(pStr, iLen);
  m_iPosition = m_pStreamImp->GetPosition();
  if (m_iPosition > iEnd) {
    m_iLength = m_iPosition - m_iStart;
  }
  return iLen;
}
void CFGAS_Stream::Flush() {
  if (!m_pStreamImp) {
    return;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return;
  }
  m_pStreamImp->Flush();
}
bool CFGAS_Stream::SetLength(int32_t iLength) {
  if (!m_pStreamImp) {
    return false;
  }
  if ((m_dwAccess & FX_STREAMACCESS_Write) == 0) {
    return false;
  }
  return m_pStreamImp->SetLength(iLength);
}
int32_t CFGAS_Stream::GetBOM(uint8_t bom[4]) const {
  if (!m_pStreamImp) {
    return -1;
  }
  return 0;
}
uint16_t CFGAS_Stream::GetCodePage() const {
#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
  return FX_CODEPAGE_UTF16LE;
#else
  return FX_CODEPAGE_UTF16BE;
#endif
}
uint16_t CFGAS_Stream::SetCodePage(uint16_t wCodePage) {
#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
  return FX_CODEPAGE_UTF16LE;
#else
  return FX_CODEPAGE_UTF16BE;
#endif
}

CFGAS_FileRead::CFGAS_FileRead(const CFX_RetainPtr<IFGAS_Stream>& pStream)
    : m_pStream(pStream) {
  ASSERT(m_pStream);
}

CFGAS_FileRead::~CFGAS_FileRead() {}

FX_FILESIZE CFGAS_FileRead::GetSize() {
  return (FX_FILESIZE)m_pStream->GetLength();
}

bool CFGAS_FileRead::ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) {
  m_pStream->Seek(FX_STREAMSEEK_Begin, (int32_t)offset);
  int32_t iLen = m_pStream->ReadData((uint8_t*)buffer, (int32_t)size);
  return iLen == (int32_t)size;
}

}  // namespace

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateReadStream(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pFileRead) {
  auto pSR = pdfium::MakeRetain<CFGAS_Stream>();
  if (!pSR->LoadFileRead(pFileRead))
    return nullptr;
  return pdfium::MakeRetain<CFGAS_TextStream>(pSR);
}

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateWriteStream(
    const CFX_RetainPtr<IFX_SeekableWriteStream>& pFileWrite) {
  auto pSR = pdfium::MakeRetain<CFGAS_Stream>();
  if (!pSR->LoadFileWrite(pFileWrite, FX_STREAMACCESS_Write))
    return nullptr;
  return pdfium::MakeRetain<CFGAS_TextStream>(pSR);
}

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateStream(uint8_t* pData,
                                                       int32_t length) {
  auto pSR = pdfium::MakeRetain<CFGAS_Stream>();
  if (!pSR->LoadBuffer(pData, length))
    return nullptr;
  return pSR;
}

CFX_RetainPtr<IFX_SeekableReadStream> IFGAS_Stream::MakeSeekableReadStream() {
  return pdfium::MakeRetain<CFGAS_FileRead>(CFX_RetainPtr<IFGAS_Stream>(this));
}
