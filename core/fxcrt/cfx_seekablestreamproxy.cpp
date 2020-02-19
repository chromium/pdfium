// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/cfx_seekablestreamproxy.h"

#if defined(OS_WIN)
#include <io.h>
#endif

#include <algorithm>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_memory_wrappers.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/stl_util.h"

namespace {

// Returns {src bytes consumed, dst chars produced}.
// Invalid sequences are silently not output.
std::pair<size_t, size_t> UTF8Decode(pdfium::span<const uint8_t> pSrc,
                                     pdfium::span<wchar_t> pDst) {
  ASSERT(!pDst.empty());

  uint32_t dwCode = 0;
  int32_t iPending = 0;
  size_t iSrcNum = 0;
  size_t iDstNum = 0;
  for (size_t iIndex = 0; iIndex < pSrc.size() && iDstNum < pDst.size();
       ++iIndex) {
    ++iSrcNum;
    uint8_t byte = pSrc[iIndex];
    if (byte < 0x80) {
      iPending = 0;
      pDst[iDstNum++] = byte;
    } else if (byte < 0xc0) {
      if (iPending < 1)
        continue;

      dwCode = dwCode << 6;
      dwCode |= (byte & 0x3f);
      --iPending;
      if (iPending == 0)
        pDst[iDstNum++] = dwCode;
    } else if (byte < 0xe0) {
      iPending = 1;
      dwCode = (byte & 0x1f);
    } else if (byte < 0xf0) {
      iPending = 2;
      dwCode = (byte & 0x0f);
    } else if (byte < 0xf8) {
      iPending = 3;
      dwCode = (byte & 0x07);
    } else if (byte < 0xfc) {
      iPending = 4;
      dwCode = (byte & 0x03);
    } else if (byte < 0xfe) {
      iPending = 5;
      dwCode = (byte & 0x01);
    }
  }
  return {iSrcNum, iDstNum};
}

#if defined(WCHAR_T_IS_UTF32)
static_assert(sizeof(wchar_t) > 2, "wchar_t is too small");

void UTF16ToWChar(void* pBuffer, size_t iLength) {
  ASSERT(pBuffer);
  ASSERT(iLength > 0);

  uint16_t* pSrc = static_cast<uint16_t*>(pBuffer);
  wchar_t* pDst = static_cast<wchar_t*>(pBuffer);

  // Perform self-intersecting copy in reverse order.
  for (size_t i = iLength; i > 0; --i)
    pDst[i - 1] = static_cast<wchar_t>(pSrc[i - 1]);
}
#endif  // defined(WCHAR_T_IS_UTF32)

void SwapByteOrder(uint16_t* pStr, size_t iLength) {
  while (iLength-- > 0) {
    uint16_t wch = *pStr;
    *pStr++ = (wch >> 8) | (wch << 8);
  }
}

}  // namespace

#define BOM_UTF8_MASK 0x00FFFFFF
#define BOM_UTF8 0x00BFBBEF
#define BOM_UTF16_MASK 0x0000FFFF
#define BOM_UTF16_BE 0x0000FFFE
#define BOM_UTF16_LE 0x0000FEFF

CFX_SeekableStreamProxy::CFX_SeekableStreamProxy(
    const RetainPtr<IFX_SeekableReadStream>& stream)
    : m_wCodePage(FX_CODEPAGE_DefANSI),
      m_wBOMLength(0),
      m_iPosition(0),
      m_pStream(stream) {
  ASSERT(m_pStream);

  Seek(From::Begin, 0);

  uint32_t bom = 0;
  ReadData(reinterpret_cast<uint8_t*>(&bom), 3);

  bom &= BOM_UTF8_MASK;
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

  Seek(From::Begin, static_cast<FX_FILESIZE>(m_wBOMLength));
}

CFX_SeekableStreamProxy::~CFX_SeekableStreamProxy() = default;

FX_FILESIZE CFX_SeekableStreamProxy::GetSize() {
  return m_pStream->GetSize();
}

FX_FILESIZE CFX_SeekableStreamProxy::GetPosition() {
  return m_iPosition;
}

bool CFX_SeekableStreamProxy::IsEOF() {
  return m_iPosition >= GetSize();
}

void CFX_SeekableStreamProxy::Seek(From eSeek, FX_FILESIZE iOffset) {
  switch (eSeek) {
    case From::Begin:
      m_iPosition = iOffset;
      break;
    case From::Current: {
      FX_SAFE_FILESIZE new_pos = m_iPosition;
      new_pos += iOffset;
      m_iPosition =
          new_pos.ValueOrDefault(std::numeric_limits<FX_FILESIZE>::max());
    } break;
  }
  m_iPosition =
      pdfium::clamp(m_iPosition, static_cast<FX_FILESIZE>(0), GetSize());
}

void CFX_SeekableStreamProxy::SetCodePage(uint16_t wCodePage) {
  if (m_wBOMLength > 0)
    return;
  m_wCodePage = wCodePage;
}

size_t CFX_SeekableStreamProxy::ReadData(uint8_t* pBuffer, size_t iBufferSize) {
  ASSERT(pBuffer);
  ASSERT(iBufferSize > 0);

  iBufferSize =
      std::min(iBufferSize, static_cast<size_t>(GetSize() - m_iPosition));
  if (iBufferSize <= 0)
    return 0;

  if (!m_pStream->ReadBlockAtOffset(pBuffer, m_iPosition, iBufferSize))
    return 0;

  FX_SAFE_FILESIZE new_pos = m_iPosition;
  new_pos += iBufferSize;
  m_iPosition = new_pos.ValueOrDefault(m_iPosition);
  return new_pos.IsValid() ? iBufferSize : 0;
}

size_t CFX_SeekableStreamProxy::ReadBlock(wchar_t* pStr, size_t size) {
  if (!pStr || size == 0)
    return 0;

  if (m_wCodePage == FX_CODEPAGE_UTF16LE ||
      m_wCodePage == FX_CODEPAGE_UTF16BE) {
    size_t iBytes = size * 2;
    size_t iLen = ReadData(reinterpret_cast<uint8_t*>(pStr), iBytes);
    size = iLen / 2;
    if (m_wCodePage == FX_CODEPAGE_UTF16BE)
      SwapByteOrder(reinterpret_cast<uint16_t*>(pStr), size);

#if defined(WCHAR_T_IS_UTF32)
    if (size > 0)
      UTF16ToWChar(pStr, size);
#endif
    return size;
  }

  FX_FILESIZE pos = GetPosition();
  size_t iBytes = std::min(size, static_cast<size_t>(GetSize() - pos));
  if (iBytes == 0)
    return 0;

  std::vector<uint8_t, FxAllocAllocator<uint8_t>> buf(iBytes);
  size_t iLen = ReadData(buf.data(), iBytes);
  if (m_wCodePage != FX_CODEPAGE_UTF8)
    return 0;

  size_t iSrc;
  std::tie(iSrc, size) = UTF8Decode({buf.data(), iLen}, {pStr, size});
  Seek(From::Current, iSrc - iLen);
  return size;
}
