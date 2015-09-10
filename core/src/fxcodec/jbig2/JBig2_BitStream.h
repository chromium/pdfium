// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_BIT_STREAM_H_
#define _JBIG2_BIT_STREAM_H_

#include "JBig2_Define.h"

class CJBig2_BitStream {
 public:
  CJBig2_BitStream(const uint8_t* pBuffer, FX_DWORD dwLength);

  CJBig2_BitStream(CJBig2_BitStream& bs);

  ~CJBig2_BitStream();

  int32_t readNBits(FX_DWORD nBits, FX_DWORD* dwResult);

  int32_t readNBits(FX_DWORD nBits, int32_t* nResult);

  int32_t read1Bit(FX_DWORD* dwResult);

  int32_t read1Bit(FX_BOOL* bResult);

  int32_t read1Byte(uint8_t* cResult);

  int32_t readInteger(FX_DWORD* dwResult);

  int32_t readShortInteger(FX_WORD* wResult);

  void alignByte();

  void align4Byte();

  uint8_t getAt(FX_DWORD dwOffset);

  uint8_t getCurByte();

  uint8_t getNextByte();

  int32_t incByteIdx();

  uint8_t getCurByte_arith();

  uint8_t getNextByte_arith();

  FX_DWORD getOffset();

  void setOffset(FX_DWORD dwOffset);

  FX_DWORD getBitPos();

  void setBitPos(FX_DWORD dwBitPos);

  const uint8_t* getBuf();

  FX_DWORD getLength() { return m_dwLength; }

  const uint8_t* getPointer();

  void offset(FX_DWORD dwOffset);

  FX_DWORD getByteLeft();

 private:
  const uint8_t* m_pBuf;

  FX_DWORD m_dwLength;

  FX_DWORD m_dwByteIdx;

  FX_DWORD m_dwBitIdx;
};
inline CJBig2_BitStream::CJBig2_BitStream(const uint8_t* pBuffer,
                                          FX_DWORD dwLength) {
  m_pBuf = pBuffer;
  m_dwLength = dwLength;
  m_dwByteIdx = 0;
  m_dwBitIdx = 0;
  if (m_dwLength > 256 * 1024 * 1024) {
    m_dwLength = 0;
    m_pBuf = NULL;
  }
}
inline CJBig2_BitStream::CJBig2_BitStream(CJBig2_BitStream& bs) {
  m_pBuf = bs.m_pBuf;
  m_dwLength = bs.m_dwLength;
  m_dwByteIdx = bs.m_dwByteIdx;
  m_dwBitIdx = bs.m_dwBitIdx;
}
inline CJBig2_BitStream::~CJBig2_BitStream() {}
inline int32_t CJBig2_BitStream::readNBits(FX_DWORD dwBits,
                                           FX_DWORD* dwResult) {
  FX_DWORD dwTemp = (m_dwByteIdx << 3) + m_dwBitIdx;
  if (dwTemp <= (m_dwLength << 3)) {
    *dwResult = 0;
    if (dwTemp + dwBits <= (m_dwLength << 3)) {
      dwTemp = dwBits;
    } else {
      dwTemp = (m_dwLength << 3) - dwTemp;
    }
    while (dwTemp > 0) {
      *dwResult =
          (*dwResult << 1) | ((m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
      if (m_dwBitIdx == 7) {
        m_dwByteIdx++;
        m_dwBitIdx = 0;
      } else {
        m_dwBitIdx++;
      }
      dwTemp--;
    }
    return 0;
  } else {
    return -1;
  }
}
inline int32_t CJBig2_BitStream::readNBits(FX_DWORD dwBits, int32_t* nResult) {
  FX_DWORD dwTemp = (m_dwByteIdx << 3) + m_dwBitIdx;
  if (dwTemp <= (m_dwLength << 3)) {
    *nResult = 0;
    if (dwTemp + dwBits <= (m_dwLength << 3)) {
      dwTemp = dwBits;
    } else {
      dwTemp = (m_dwLength << 3) - dwTemp;
    }
    while (dwTemp > 0) {
      *nResult =
          (*nResult << 1) | ((m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01);
      if (m_dwBitIdx == 7) {
        m_dwByteIdx++;
        m_dwBitIdx = 0;
      } else {
        m_dwBitIdx++;
      }
      dwTemp--;
    }
    return 0;
  } else {
    return -1;
  }
}

inline int32_t CJBig2_BitStream::read1Bit(FX_DWORD* dwResult) {
  if (m_dwByteIdx < m_dwLength) {
    *dwResult = (m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
    if (m_dwBitIdx == 7) {
      m_dwByteIdx++;
      m_dwBitIdx = 0;
    } else {
      m_dwBitIdx++;
    }
    return 0;
  } else {
    return -1;
  }
}

inline int32_t CJBig2_BitStream::read1Bit(FX_BOOL* bResult) {
  if (m_dwByteIdx < m_dwLength) {
    *bResult = (m_pBuf[m_dwByteIdx] >> (7 - m_dwBitIdx)) & 0x01;
    if (m_dwBitIdx == 7) {
      m_dwByteIdx++;
      m_dwBitIdx = 0;
    } else {
      m_dwBitIdx++;
    }
    return 0;
  } else {
    return -1;
  }
}
inline int32_t CJBig2_BitStream::read1Byte(uint8_t* cResult) {
  if (m_dwByteIdx < m_dwLength) {
    *cResult = m_pBuf[m_dwByteIdx];
    m_dwByteIdx++;
    return 0;
  } else {
    return -1;
  }
}

inline int32_t CJBig2_BitStream::readInteger(FX_DWORD* dwResult) {
  if (m_dwByteIdx + 3 < m_dwLength) {
    *dwResult = (m_pBuf[m_dwByteIdx] << 24) | (m_pBuf[m_dwByteIdx + 1] << 16) |
                (m_pBuf[m_dwByteIdx + 2] << 8) | m_pBuf[m_dwByteIdx + 3];
    m_dwByteIdx += 4;
    return 0;
  } else {
    return -1;
  }
}

inline int32_t CJBig2_BitStream::readShortInteger(FX_WORD* dwResult) {
  if (m_dwByteIdx + 1 < m_dwLength) {
    *dwResult = (m_pBuf[m_dwByteIdx] << 8) | m_pBuf[m_dwByteIdx + 1];
    m_dwByteIdx += 2;
    return 0;
  } else {
    return -1;
  }
}
inline void CJBig2_BitStream::alignByte() {
  if (m_dwBitIdx != 0) {
    m_dwByteIdx++;
    m_dwBitIdx = 0;
  }
}
inline void CJBig2_BitStream::align4Byte() {
  if (m_dwBitIdx != 0) {
    m_dwByteIdx++;
    m_dwBitIdx = 0;
  }
  m_dwByteIdx = (m_dwByteIdx + 3) & -4;
}
inline uint8_t CJBig2_BitStream::getAt(FX_DWORD dwOffset) {
  if (dwOffset < m_dwLength) {
    return m_pBuf[dwOffset];
  } else {
    return 0;
  }
}
inline uint8_t CJBig2_BitStream::getCurByte() {
  if (m_dwByteIdx < m_dwLength) {
    return m_pBuf[m_dwByteIdx];
  } else {
    return 0;
  }
}
inline uint8_t CJBig2_BitStream::getNextByte() {
  if (m_dwByteIdx + 1 < m_dwLength) {
    return m_pBuf[m_dwByteIdx + 1];
  } else {
    return 0;
  }
}
inline int32_t CJBig2_BitStream::incByteIdx() {
  if (m_dwByteIdx < m_dwLength) {
    m_dwByteIdx++;
    return 0;
  } else {
    return -1;
  }
}
inline uint8_t CJBig2_BitStream::getCurByte_arith() {
  if (m_dwByteIdx < m_dwLength) {
    return m_pBuf[m_dwByteIdx];
  } else {
    return 0xff;
  }
}
inline uint8_t CJBig2_BitStream::getNextByte_arith() {
  if (m_dwByteIdx + 1 < m_dwLength) {
    return m_pBuf[m_dwByteIdx + 1];
  } else {
    return 0xff;
  }
}
inline FX_DWORD CJBig2_BitStream::getOffset() {
  return m_dwByteIdx;
}
inline void CJBig2_BitStream::setOffset(FX_DWORD dwOffset) {
  if (dwOffset > m_dwLength) {
    dwOffset = m_dwLength;
  }
  m_dwByteIdx = dwOffset;
}
inline FX_DWORD CJBig2_BitStream::getBitPos() {
  return (m_dwByteIdx << 3) + m_dwBitIdx;
}
inline void CJBig2_BitStream::setBitPos(FX_DWORD dwBitPos) {
  m_dwByteIdx = dwBitPos >> 3;
  m_dwBitIdx = dwBitPos & 7;
}
inline const uint8_t* CJBig2_BitStream::getBuf() {
  return m_pBuf;
}
inline const uint8_t* CJBig2_BitStream::getPointer() {
  return m_pBuf + m_dwByteIdx;
}
inline void CJBig2_BitStream::offset(FX_DWORD dwOffset) {
  m_dwByteIdx += dwOffset;
}
inline FX_DWORD CJBig2_BitStream::getByteLeft() {
  return m_dwLength - m_dwByteIdx;
}
#endif
