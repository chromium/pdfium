// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_BASIC_H_
#define CORE_FXCRT_FX_BASIC_H_

#include <algorithm>
#include <memory>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

#ifdef PDF_ENABLE_XFA
#define FX_IsOdd(a) ((a)&1)
#endif  // PDF_ENABLE_XFA

class CFX_BinaryBuf {
 public:
  CFX_BinaryBuf();
  explicit CFX_BinaryBuf(FX_STRSIZE size);
  ~CFX_BinaryBuf();

  uint8_t* GetBuffer() const { return m_pBuffer.get(); }
  FX_STRSIZE GetSize() const { return m_DataSize; }

  void Clear();
  void EstimateSize(FX_STRSIZE size, FX_STRSIZE alloc_step = 0);
  void AppendBlock(const void* pBuf, FX_STRSIZE size);
  void AppendString(const CFX_ByteString& str) {
    AppendBlock(str.c_str(), str.GetLength());
  }

  void AppendByte(uint8_t byte) {
    ExpandBuf(1);
    m_pBuffer.get()[m_DataSize++] = byte;
  }

  void InsertBlock(FX_STRSIZE pos, const void* pBuf, FX_STRSIZE size);
  void Delete(int start_index, int count);

  // Releases ownership of |m_pBuffer| and returns it.
  std::unique_ptr<uint8_t, FxFreeDeleter> DetachBuffer();

 protected:
  void ExpandBuf(FX_STRSIZE size);

  FX_STRSIZE m_AllocStep;
  FX_STRSIZE m_AllocSize;
  FX_STRSIZE m_DataSize;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pBuffer;
};

class CFX_ByteTextBuf : public CFX_BinaryBuf {
 public:
  FX_STRSIZE GetLength() const { return m_DataSize; }
  CFX_ByteString MakeString() const {
    return CFX_ByteString(m_pBuffer.get(), m_DataSize);
  }
  CFX_ByteStringC AsStringC() const {
    return CFX_ByteStringC(m_pBuffer.get(), m_DataSize);
  }

  void AppendChar(int ch) { AppendByte(static_cast<uint8_t>(ch)); }
  CFX_ByteTextBuf& operator<<(int i);
  CFX_ByteTextBuf& operator<<(uint32_t i);
  CFX_ByteTextBuf& operator<<(double f);
  CFX_ByteTextBuf& operator<<(const char* pStr) {
    return *this << CFX_ByteStringC(pStr);
  }
  CFX_ByteTextBuf& operator<<(const CFX_ByteString& str) {
    return *this << str.AsStringC();
  }
  CFX_ByteTextBuf& operator<<(const CFX_ByteStringC& lpsz);
  CFX_ByteTextBuf& operator<<(const CFX_ByteTextBuf& buf);
};

class CFX_WideTextBuf : public CFX_BinaryBuf {
 public:
  void AppendChar(wchar_t wch);
  FX_STRSIZE GetLength() const { return m_DataSize / sizeof(wchar_t); }
  wchar_t* GetBuffer() const {
    return reinterpret_cast<wchar_t*>(m_pBuffer.get());
  }

  CFX_WideStringC AsStringC() const {
    return CFX_WideStringC(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                           m_DataSize / sizeof(wchar_t));
  }
  CFX_WideString MakeString() const {
    return CFX_WideString(reinterpret_cast<const wchar_t*>(m_pBuffer.get()),
                          m_DataSize / sizeof(wchar_t));
  }

  void Delete(int start_index, int count) {
    CFX_BinaryBuf::Delete(start_index * sizeof(wchar_t),
                          count * sizeof(wchar_t));
  }

  CFX_WideTextBuf& operator<<(int i);
  CFX_WideTextBuf& operator<<(double f);
  CFX_WideTextBuf& operator<<(const wchar_t* lpsz);
  CFX_WideTextBuf& operator<<(const CFX_WideStringC& str);
  CFX_WideTextBuf& operator<<(const CFX_WideString& str);
  CFX_WideTextBuf& operator<<(const CFX_WideTextBuf& buf);
};

class CFX_FileBufferArchive {
 public:
  CFX_FileBufferArchive();
  ~CFX_FileBufferArchive();

  void Clear();
  bool Flush();
  int32_t AppendBlock(const void* pBuf, size_t size);
  int32_t AppendByte(uint8_t byte);
  int32_t AppendDWord(uint32_t i);
  int32_t AppendString(const CFX_ByteStringC& lpsz);
  void AttachFile(const CFX_RetainPtr<IFX_WriteStream>& pFile);

 private:
  static const size_t kBufSize = 32768;

  size_t m_Length;
  std::unique_ptr<uint8_t, FxFreeDeleter> m_pBuffer;
  CFX_RetainPtr<IFX_WriteStream> m_pFile;
};

class CFX_CharMap {
 public:
  static CFX_ByteString GetByteString(uint16_t codepage,
                                      const CFX_WideStringC& wstr);

  static CFX_WideString GetWideString(uint16_t codepage,
                                      const CFX_ByteStringC& bstr);

  CFX_CharMap() = delete;
};

class CFX_UTF8Decoder {
 public:
  CFX_UTF8Decoder() { m_PendingBytes = 0; }

  void Clear();

  void Input(uint8_t byte);

  void AppendChar(uint32_t ch);

  void ClearStatus() { m_PendingBytes = 0; }

  CFX_WideStringC GetResult() const { return m_Buffer.AsStringC(); }

 protected:
  int m_PendingBytes;

  uint32_t m_PendingChar;

  CFX_WideTextBuf m_Buffer;
};

class CFX_UTF8Encoder {
 public:
  CFX_UTF8Encoder() {}

  void Input(wchar_t unicode);
  void AppendStr(const CFX_ByteStringC& str) { m_Buffer << str; }
  CFX_ByteStringC GetResult() const { return m_Buffer.AsStringC(); }

 protected:
  CFX_ByteTextBuf m_Buffer;
};

class CFX_BasicArray {
 protected:
  explicit CFX_BasicArray(int unit_size);
  CFX_BasicArray(const CFX_BasicArray&) = delete;
  ~CFX_BasicArray();

  bool SetSize(int nNewSize);
  bool Append(const CFX_BasicArray& src);
  bool Copy(const CFX_BasicArray& src);
  uint8_t* InsertSpaceAt(int nIndex, int nCount);
  bool RemoveAt(int nIndex, int nCount);
  bool InsertAt(int nStartIndex, const CFX_BasicArray* pNewArray);
  const void* GetDataPtr(int index) const;

 protected:
  uint8_t* m_pData;
  int m_nSize;
  int m_nMaxSize;
  int m_nUnitSize;
};

template <class TYPE>
class CFX_ArrayTemplate : public CFX_BasicArray {
 public:
  CFX_ArrayTemplate() : CFX_BasicArray(sizeof(TYPE)) {}

  int GetSize() const { return m_nSize; }

  int GetUpperBound() const { return m_nSize - 1; }

  bool SetSize(int nNewSize) { return CFX_BasicArray::SetSize(nNewSize); }

  void RemoveAll() { SetSize(0); }

  const TYPE GetAt(int nIndex) const {
    if (nIndex < 0 || nIndex >= m_nSize) {
      PDFIUM_IMMEDIATE_CRASH();
    }
    return ((const TYPE*)m_pData)[nIndex];
  }

  bool SetAt(int nIndex, TYPE newElement) {
    if (nIndex < 0 || nIndex >= m_nSize) {
      return false;
    }
    ((TYPE*)m_pData)[nIndex] = newElement;
    return true;
  }

  TYPE& ElementAt(int nIndex) {
    if (nIndex < 0 || nIndex >= m_nSize) {
      PDFIUM_IMMEDIATE_CRASH();
    }
    return ((TYPE*)m_pData)[nIndex];
  }

  const TYPE* GetData() const { return (const TYPE*)m_pData; }

  TYPE* GetData() { return (TYPE*)m_pData; }

  bool SetAtGrow(int nIndex, TYPE newElement) {
    if (nIndex < 0)
      return false;

    if (nIndex >= m_nSize && !SetSize(nIndex + 1))
      return false;

    ((TYPE*)m_pData)[nIndex] = newElement;
    return true;
  }

  bool Add(TYPE newElement) {
    if (m_nSize < m_nMaxSize) {
      m_nSize++;
    } else if (!SetSize(m_nSize + 1)) {
      return false;
    }
    ((TYPE*)m_pData)[m_nSize - 1] = newElement;
    return true;
  }

  bool Append(const CFX_ArrayTemplate& src) {
    return CFX_BasicArray::Append(src);
  }

  bool Copy(const CFX_ArrayTemplate& src) { return CFX_BasicArray::Copy(src); }

  TYPE* GetDataPtr(int index) {
    return (TYPE*)CFX_BasicArray::GetDataPtr(index);
  }

  TYPE* AddSpace() { return (TYPE*)CFX_BasicArray::InsertSpaceAt(m_nSize, 1); }

  TYPE* InsertSpaceAt(int nIndex, int nCount) {
    return (TYPE*)CFX_BasicArray::InsertSpaceAt(nIndex, nCount);
  }

  const TYPE operator[](int nIndex) const {
    if (nIndex < 0 || nIndex >= m_nSize) {
      *(volatile char*)0 = '\0';
    }
    return ((const TYPE*)m_pData)[nIndex];
  }

  TYPE& operator[](int nIndex) {
    if (nIndex < 0 || nIndex >= m_nSize) {
      *(volatile char*)0 = '\0';
    }
    return ((TYPE*)m_pData)[nIndex];
  }

  bool InsertAt(int nIndex, TYPE newElement, int nCount = 1) {
    if (!InsertSpaceAt(nIndex, nCount)) {
      return false;
    }
    while (nCount--) {
      ((TYPE*)m_pData)[nIndex++] = newElement;
    }
    return true;
  }

  bool RemoveAt(int nIndex, int nCount = 1) {
    return CFX_BasicArray::RemoveAt(nIndex, nCount);
  }

  bool InsertAt(int nStartIndex, const CFX_BasicArray* pNewArray) {
    return CFX_BasicArray::InsertAt(nStartIndex, pNewArray);
  }

  int Find(TYPE data, int iStart = 0) const {
    if (iStart < 0) {
      return -1;
    }
    for (; iStart < (int)m_nSize; iStart++)
      if (((TYPE*)m_pData)[iStart] == data) {
        return iStart;
      }
    return -1;
  }
};

class CFX_BitStream {
 public:
  void Init(const uint8_t* pData, uint32_t dwSize);

  void ByteAlign();
  bool IsEOF() const { return m_BitPos >= m_BitSize; }
  uint32_t GetBits(uint32_t nBits);
  void SkipBits(uint32_t nBits) { m_BitPos += nBits; }
  void Rewind() { m_BitPos = 0; }
  uint32_t GetPos() const { return m_BitPos; }
  uint32_t BitsRemaining() const {
    return m_BitSize >= m_BitPos ? m_BitSize - m_BitPos : 0;
  }

 protected:
  uint32_t m_BitPos;
  uint32_t m_BitSize;
  const uint8_t* m_pData;
};

class IFX_Pause {
 public:
  virtual ~IFX_Pause() {}
  virtual bool NeedToPauseNow() = 0;
};

template <typename T>
class CFX_AutoRestorer {
 public:
  explicit CFX_AutoRestorer(T* location)
      : m_Location(location), m_OldValue(*location) {}
  ~CFX_AutoRestorer() { *m_Location = m_OldValue; }

 private:
  T* const m_Location;
  const T m_OldValue;
};

uint32_t GetBits32(const uint8_t* pData, int bitpos, int nbits);

#endif  // CORE_FXCRT_FX_BASIC_H_
