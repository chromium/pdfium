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
#include <utility>
#include <vector>

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/crt/fgas_codepage.h"

namespace {

class CFGAS_Stream : public IFGAS_Stream {
 public:
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFGAS_Stream
  FX_FILESIZE GetLength() const override { return m_pStream->GetSize(); }
  FX_FILESIZE GetPosition() override { return m_iPosition; }
  FX_STRSIZE GetBOMLength() const override { return std::max(0, m_wBOMLength); }
  FX_STRSIZE ReadString(wchar_t* pStr,
                        FX_STRSIZE iMaxLength,
                        bool* bEOS) override;
  void Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) override;
  bool IsEOF() const override { return m_iPosition >= GetLength(); }
  void WriteString(const CFX_WideStringC& str) override;
  uint16_t GetCodePage() const override { return m_wCodePage; }
  void SetCodePage(uint16_t wCodePage) override;

 private:
  CFGAS_Stream(const CFX_RetainPtr<IFX_SeekableStream>& stream,
               bool isWriteSteam);
  ~CFGAS_Stream() override;

  FX_STRSIZE ReadData(uint8_t* pBuffer, FX_STRSIZE iBufferSize);
  void WriteData(const CFX_WideStringC& str);

  uint16_t m_wCodePage;
  FX_STRSIZE m_wBOMLength;
  bool m_IsWriteStream;
  FX_FILESIZE m_iPosition;
  CFX_RetainPtr<IFX_SeekableStream> m_pStream;
};

// Returns {src bytes consumed, dst bytes produced}.
std::pair<FX_STRSIZE, FX_STRSIZE> UTF8Decode(const char* pSrc,
                                             FX_STRSIZE srcLen,
                                             wchar_t* pDst,
                                             FX_STRSIZE dstLen) {
  ASSERT(pDst && dstLen > 0);

  if (srcLen < 1)
    return {0, 0};

  uint32_t dwCode = 0;
  int32_t iPending = 0;
  FX_STRSIZE iSrcNum = 0;
  FX_STRSIZE iDstNum = 0;
  FX_STRSIZE iIndex = 0;
  int32_t k = 1;
  while (iIndex < srcLen) {
    uint8_t byte = static_cast<uint8_t>(*(pSrc + iIndex));
    if (byte < 0x80) {
      iPending = 0;
      k = 1;
      iDstNum++;
      iSrcNum += k;
      *pDst++ = byte;
      if (iDstNum >= dstLen)
        break;
    } else if (byte < 0xc0) {
      if (iPending < 1)
        break;

      iPending--;
      dwCode |= (byte & 0x3f) << (iPending * 6);
      if (iPending == 0) {
        iDstNum++;
        iSrcNum += k;
        *pDst++ = dwCode;
        if (iDstNum >= dstLen)
          break;
      }
    } else if (byte < 0xe0) {
      iPending = 1;
      k = 2;
      dwCode = (byte & 0x1f) << 6;
    } else if (byte < 0xf0) {
      iPending = 2;
      k = 3;
      dwCode = (byte & 0x0f) << 12;
    } else if (byte < 0xf8) {
      iPending = 3;
      k = 4;
      dwCode = (byte & 0x07) << 18;
    } else if (byte < 0xfc) {
      iPending = 4;
      k = 5;
      dwCode = (byte & 0x03) << 24;
    } else if (byte < 0xfe) {
      iPending = 5;
      k = 6;
      dwCode = (byte & 0x01) << 30;
    } else {
      break;
    }
    iIndex++;
  }
  return {iSrcNum, iDstNum};
}

void UTF16ToWChar(void* pBuffer, FX_STRSIZE iLength) {
  ASSERT(pBuffer && iLength > 0);

  if (sizeof(wchar_t) == 2)
    return;

  uint16_t* pSrc = static_cast<uint16_t*>(pBuffer);
  wchar_t* pDst = static_cast<wchar_t*>(pBuffer);
  while (--iLength >= 0)
    pDst[iLength] = static_cast<wchar_t>(pSrc[iLength]);
}

void SwapByteOrder(wchar_t* pStr, FX_STRSIZE iLength) {
  ASSERT(pStr);

  if (iLength < 0)
    iLength = FXSYS_wcslen(pStr);

  uint16_t wch;
  if (sizeof(wchar_t) > 2) {
    while (iLength-- > 0) {
      wch = static_cast<uint16_t>(*pStr);
      wch = (wch >> 8) | (wch << 8);
      wch &= 0x00FF;
      *pStr = wch;
      ++pStr;
    }
    return;
  }

  while (iLength-- > 0) {
    wch = static_cast<uint16_t>(*pStr);
    wch = (wch >> 8) | (wch << 8);
    *pStr = wch;
    ++pStr;
  }
}

#if _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_
#define BOM_MASK 0x00FFFFFF
#define BOM_UTF8 0x00BFBBEF
#define BOM_UTF16_MASK 0x0000FFFF
#define BOM_UTF16_BE 0x0000FFFE
#define BOM_UTF16_LE 0x0000FEFF
#else
#define BOM_MASK 0xFFFFFF00
#define BOM_UTF8 0xEFBBBF00
#define BOM_UTF16_MASK 0xFFFF0000
#define BOM_UTF16_BE 0xFEFF0000
#define BOM_UTF16_LE 0xFFFE0000
#endif  // _FX_ENDIAN_ == _FX_LITTLE_ENDIAN_

CFGAS_Stream::CFGAS_Stream(const CFX_RetainPtr<IFX_SeekableStream>& stream,
                           bool isWriteStream)
    : m_wCodePage(FX_CODEPAGE_DefANSI),
      m_wBOMLength(0),
      m_IsWriteStream(isWriteStream),
      m_iPosition(0),
      m_pStream(stream) {
  ASSERT(m_pStream);

  if (isWriteStream) {
    m_iPosition = m_pStream->GetSize();
    return;
  }

  FX_FILESIZE iPosition = GetPosition();
  Seek(FX_STREAMSEEK_Begin, 0);

  uint32_t bom;
  ReadData(reinterpret_cast<uint8_t*>(&bom), 3);

  bom &= BOM_MASK;
  if (bom == BOM_UTF8) {
    m_wBOMLength = 3;
    m_wCodePage = FX_CODEPAGE_UTF8;
  } else {
    bom &= BOM_UTF16_MASK;
    if (bom == BOM_UTF16_BE) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16BE;
    } else if (bom == BOM_UTF16_LE) {
      m_wBOMLength = 2;
      m_wCodePage = FX_CODEPAGE_UTF16LE;
    } else {
      m_wBOMLength = 0;
      m_wCodePage = FXSYS_GetACP();
    }
  }

  Seek(FX_STREAMSEEK_Begin,
       std::max(static_cast<FX_FILESIZE>(m_wBOMLength), iPosition));
}

CFGAS_Stream::~CFGAS_Stream() {}

void CFGAS_Stream::Seek(FX_STREAMSEEK eSeek, FX_FILESIZE iOffset) {
  switch (eSeek) {
    case FX_STREAMSEEK_Begin:
      m_iPosition = iOffset;
      break;
    case FX_STREAMSEEK_Current:
      m_iPosition += iOffset;
      break;
  }
  m_iPosition =
      pdfium::clamp(m_iPosition, static_cast<FX_FILESIZE>(0), GetLength());
}

void CFGAS_Stream::SetCodePage(uint16_t wCodePage) {
  if (m_wBOMLength > 0)
    return;
  m_wCodePage = wCodePage;
}

FX_STRSIZE CFGAS_Stream::ReadData(uint8_t* pBuffer, FX_STRSIZE iBufferSize) {
  ASSERT(pBuffer && iBufferSize > 0);

  if (m_IsWriteStream)
    return -1;

  iBufferSize = std::min(
      iBufferSize, static_cast<FX_STRSIZE>(m_pStream->GetSize() - m_iPosition));
  if (iBufferSize <= 0)
    return 0;

  if (m_pStream->ReadBlock(pBuffer, m_iPosition, iBufferSize)) {
    pdfium::base::CheckedNumeric<FX_FILESIZE> new_pos = m_iPosition;
    new_pos += iBufferSize;
    if (!new_pos.IsValid())
      return 0;

    m_iPosition = new_pos.ValueOrDie();
    return iBufferSize;
  }
  return 0;
}

FX_STRSIZE CFGAS_Stream::ReadString(wchar_t* pStr,
                                    FX_STRSIZE iMaxLength,
                                    bool* bEOS) {
  ASSERT(pStr && iMaxLength > 0);

  if (m_IsWriteStream)
    return -1;

  if (m_wCodePage == FX_CODEPAGE_UTF16LE ||
      m_wCodePage == FX_CODEPAGE_UTF16BE) {
    FX_FILESIZE iBytes = iMaxLength * 2;
    FX_STRSIZE iLen = ReadData(reinterpret_cast<uint8_t*>(pStr), iBytes);
    iMaxLength = iLen / 2;
    if (sizeof(wchar_t) > 2)
      UTF16ToWChar(pStr, iMaxLength);

#if _FX_ENDIAN_ == _FX_BIG_ENDIAN_
    if (m_wCodePage == FX_CODEPAGE_UTF16LE)
      SwapByteOrder(pStr, iMaxLength);
#else
    if (m_wCodePage == FX_CODEPAGE_UTF16BE)
      SwapByteOrder(pStr, iMaxLength);
#endif

  } else {
    FX_FILESIZE pos = GetPosition();
    FX_STRSIZE iBytes =
        std::min(iMaxLength, static_cast<FX_STRSIZE>(GetLength() - pos));

    if (iBytes > 0) {
      std::vector<uint8_t> buf(iBytes);

      FX_STRSIZE iLen = ReadData(buf.data(), iBytes);
      if (m_wCodePage != FX_CODEPAGE_UTF8)
        return -1;

      FX_STRSIZE iSrc = 0;
      std::tie(iSrc, iMaxLength) = UTF8Decode(
          reinterpret_cast<const char*>(buf.data()), iLen, pStr, iMaxLength);
      Seek(FX_STREAMSEEK_Current, iSrc - iLen);
    } else {
      iMaxLength = 0;
    }
  }

  *bEOS = IsEOF();
  return iMaxLength;
}

void CFGAS_Stream::WriteData(const CFX_WideStringC& str) {
  if (!m_IsWriteStream || str.GetLength() == 0)
    return;
  if (m_pStream->WriteBlock(str.c_str(), m_iPosition,
                            str.GetLength() * sizeof(wchar_t))) {
    pdfium::base::CheckedNumeric<FX_STRSIZE> new_pos = m_iPosition;
    new_pos += str.GetLength() * sizeof(wchar_t);
    // TODO(dsinclair): Not sure what to do if we over flow ....
    if (!new_pos.IsValid())
      return;

    m_iPosition = new_pos.ValueOrDie();
  }
}

void CFGAS_Stream::WriteString(const CFX_WideStringC& str) {
  if (!m_IsWriteStream)
    return;
  if (str.GetLength() == 0)
    return;
  if (m_wCodePage != FX_CODEPAGE_UTF8)
    return;

  WriteData(str);
}

}  // namespace

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateReadStream(
    const CFX_RetainPtr<IFX_SeekableStream>& pFileRead) {
  if (!pFileRead)
    return nullptr;

  return pdfium::MakeRetain<CFGAS_Stream>(pFileRead, false);
}

// static
CFX_RetainPtr<IFGAS_Stream> IFGAS_Stream::CreateWriteStream(
    const CFX_RetainPtr<IFX_SeekableStream>& pFileWrite) {
  if (!pFileWrite)
    return nullptr;

  return pdfium::MakeRetain<CFGAS_Stream>(pFileWrite, true);
}
