// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/string_data_template.h"

#include <string.h>

#include <new>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

namespace fxcrt {

// static
template <typename CharType>
StringDataTemplate<CharType>* StringDataTemplate<CharType>::Create(
    size_t nLen) {
  DCHECK_GT(nLen, 0);

  // Calculate space needed for the fixed portion of the struct plus the
  // NUL char that is not included in |m_nAllocLength|.
  int overhead = offsetof(StringDataTemplate, m_String) + sizeof(CharType);
  FX_SAFE_SIZE_T nSize = nLen;
  nSize *= sizeof(CharType);
  nSize += overhead;

  // Now round to an 16-byte boundary, assuming the underlying allocator is most
  // likely PartitionAlloc, which has 16 byte chunks. This will help with cases
  // where we can save a re-alloc when adding a few characters to a string by
  // using this otherwise wasted space.
  nSize += 15;
  nSize &= ~15;
  size_t totalSize = nSize.ValueOrDie();
  size_t usableLen = (totalSize - overhead) / sizeof(CharType);
  DCHECK(usableLen >= nLen);

  void* pData = FX_StringAlloc(char, totalSize);
  return new (pData) StringDataTemplate(nLen, usableLen);
}

// static
template <typename CharType>
StringDataTemplate<CharType>* StringDataTemplate<CharType>::Create(
    const CharType* pStr,
    size_t nLen) {
  StringDataTemplate* result = Create(nLen);
  result->CopyContents(pStr, nLen);
  return result;
}

template <typename CharType>
void StringDataTemplate<CharType>::Release() {
  if (--m_nRefs <= 0)
    FX_Free(this);
}

template <typename CharType>
void StringDataTemplate<CharType>::CopyContents(
    const StringDataTemplate& other) {
  DCHECK(other.m_nDataLength <= m_nAllocLength);
  memcpy(m_String, other.m_String,
         (other.m_nDataLength + 1) * sizeof(CharType));
}

template <typename CharType>
void StringDataTemplate<CharType>::CopyContents(const CharType* pStr,
                                                size_t nLen) {
  DCHECK_GE(nLen, 0);
  DCHECK_LE(nLen, m_nAllocLength);
  memcpy(m_String, pStr, nLen * sizeof(CharType));
  m_String[nLen] = 0;
}

template <typename CharType>
void StringDataTemplate<CharType>::CopyContentsAt(size_t offset,
                                                  const CharType* pStr,
                                                  size_t nLen) {
  DCHECK_GE(offset, 0);
  DCHECK_GE(nLen, 0);
  DCHECK_LE(offset + nLen, m_nAllocLength);
  memcpy(m_String + offset, pStr, nLen * sizeof(CharType));
  m_String[offset + nLen] = 0;
}

template <typename CharType>
StringDataTemplate<CharType>::StringDataTemplate(size_t dataLen,
                                                 size_t allocLen)
    : m_nDataLength(dataLen), m_nAllocLength(allocLen) {
  DCHECK_GE(dataLen, 0);
  DCHECK_LE(dataLen, allocLen);
  m_String[dataLen] = 0;
}

template class StringDataTemplate<char>;
template class StringDataTemplate<wchar_t>;

}  // namespace fxcrt
