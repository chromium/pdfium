// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/string_template.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

namespace fxcrt {

template <typename T>
pdfium::span<T> StringTemplate<T>::GetBuffer(size_t nMinBufLength) {
  if (!m_pData) {
    if (nMinBufLength == 0) {
      return pdfium::span<T>();
    }
    m_pData = StringData::Create(nMinBufLength);
    m_pData->m_nDataLength = 0;
    m_pData->m_String[0] = 0;
    return pdfium::span<T>(m_pData->m_String, m_pData->m_nAllocLength);
  }
  if (m_pData->CanOperateInPlace(nMinBufLength)) {
    return pdfium::span<T>(m_pData->m_String, m_pData->m_nAllocLength);
  }
  nMinBufLength = std::max(nMinBufLength, m_pData->m_nDataLength);
  if (nMinBufLength == 0) {
    return pdfium::span<T>();
  }
  RetainPtr<StringData> pNewData = StringData::Create(nMinBufLength);
  pNewData->CopyContents(*m_pData);
  pNewData->m_nDataLength = m_pData->m_nDataLength;
  m_pData = std::move(pNewData);
  return pdfium::span<T>(m_pData->m_String, m_pData->m_nAllocLength);
}

template <typename T>
void StringTemplate<T>::ReleaseBuffer(size_t nNewLength) {
  if (!m_pData) {
    return;
  }
  nNewLength = std::min(nNewLength, m_pData->m_nAllocLength);
  if (nNewLength == 0) {
    clear();
    return;
  }
  DCHECK_EQ(m_pData->m_nRefs, 1);
  m_pData->m_nDataLength = nNewLength;
  m_pData->m_String[nNewLength] = 0;
  if (m_pData->m_nAllocLength - nNewLength >= 32) {
    // Over arbitrary threshold, so pay the price to relocate.  Force copy to
    // always occur by holding a second reference to the string.
    StringTemplate preserve(*this);
    ReallocBeforeWrite(nNewLength);
  }
}

template <typename T>
size_t StringTemplate<T>::Remove(T chRemove) {
  if (IsEmpty()) {
    return 0;
  }

  T* pstrSource = m_pData->m_String;
  T* pstrEnd = m_pData->m_String + m_pData->m_nDataLength;
  while (pstrSource < pstrEnd) {
    if (*pstrSource == chRemove) {
      break;
    }
    pstrSource++;
  }
  if (pstrSource == pstrEnd) {
    return 0;
  }

  ptrdiff_t copied = pstrSource - m_pData->m_String;
  ReallocBeforeWrite(m_pData->m_nDataLength);
  pstrSource = m_pData->m_String + copied;
  pstrEnd = m_pData->m_String + m_pData->m_nDataLength;

  T* pstrDest = pstrSource;
  while (pstrSource < pstrEnd) {
    if (*pstrSource != chRemove) {
      *pstrDest = *pstrSource;
      pstrDest++;
    }
    pstrSource++;
  }

  *pstrDest = 0;
  size_t nCount = static_cast<size_t>(pstrSource - pstrDest);
  m_pData->m_nDataLength -= nCount;
  return nCount;
}

template <typename T>
size_t StringTemplate<T>::Insert(size_t index, T ch) {
  const size_t cur_length = GetLength();
  if (!IsValidLength(index)) {
    return cur_length;
  }
  const size_t new_length = cur_length + 1;
  ReallocBeforeWrite(new_length);
  fxcrt::spanmove(m_pData->capacity_span().subspan(index + 1),
                  m_pData->capacity_span().subspan(index, new_length - index));
  m_pData->m_String[index] = ch;
  m_pData->m_nDataLength = new_length;
  return new_length;
}

template <typename T>
size_t StringTemplate<T>::Delete(size_t index, size_t count) {
  if (!m_pData) {
    return 0;
  }
  size_t old_length = m_pData->m_nDataLength;
  if (count == 0 || index != std::clamp<size_t>(index, 0, old_length)) {
    return old_length;
  }
  size_t removal_length = index + count;
  if (removal_length > old_length) {
    return old_length;
  }
  ReallocBeforeWrite(old_length);
  // Include the NUL char not accounted for in lengths.
  size_t chars_to_copy = old_length - removal_length + 1;
  fxcrt::spanmove(
      m_pData->capacity_span().subspan(index),
      m_pData->capacity_span().subspan(removal_length, chars_to_copy));
  m_pData->m_nDataLength = old_length - count;
  return m_pData->m_nDataLength;
}

template <typename T>
void StringTemplate<T>::SetAt(size_t index, T ch) {
  DCHECK(IsValidIndex(index));
  ReallocBeforeWrite(m_pData->m_nDataLength);
  m_pData->span()[index] = ch;
}

template <typename T>
std::optional<size_t> StringTemplate<T>::Find(T ch, size_t start) const {
  return Find(StringView(ch), start);
}

template <typename T>
std::optional<size_t> StringTemplate<T>::Find(StringView str,
                                              size_t start) const {
  if (!m_pData) {
    return std::nullopt;
  }
  if (!IsValidIndex(start)) {
    return std::nullopt;
  }
  std::optional<size_t> result =
      spanpos(m_pData->span().subspan(start), str.span());
  if (!result.has_value()) {
    return std::nullopt;
  }
  return start + result.value();
}

template <typename T>
std::optional<size_t> StringTemplate<T>::ReverseFind(T ch) const {
  if (!m_pData) {
    return std::nullopt;
  }
  size_t nLength = m_pData->m_nDataLength;
  while (nLength--) {
    if (m_pData->m_String[nLength] == ch) {
      return nLength;
    }
  }
  return std::nullopt;
}

template <typename T>
size_t StringTemplate<T>::Replace(StringView oldstr, StringView newstr) {
  if (!m_pData || oldstr.IsEmpty()) {
    return 0;
  }
  size_t count = 0;
  {
    // Limit span lifetime.
    pdfium::span<const T> search_span = m_pData->span();
    while (true) {
      std::optional<size_t> found = spanpos(search_span, oldstr.span());
      if (!found.has_value()) {
        break;
      }
      ++count;
      search_span = search_span.subspan(found.value() + oldstr.GetLength());
    }
  }
  if (count == 0) {
    return 0;
  }
  size_t nNewLength = m_pData->m_nDataLength +
                      count * (newstr.GetLength() - oldstr.GetLength());
  if (nNewLength == 0) {
    clear();
    return count;
  }
  RetainPtr<StringData> newstr_data = StringData::Create(nNewLength);
  {
    // Spans can't outlive StringData buffers.
    pdfium::span<const T> search_span = m_pData->span();
    pdfium::span<T> dest_span = newstr_data->span();
    for (size_t i = 0; i < count; i++) {
      size_t found = spanpos(search_span, oldstr.span()).value();
      dest_span = spancpy(dest_span, search_span.first(found));
      dest_span = spancpy(dest_span, newstr.span());
      search_span = search_span.subspan(found + oldstr.GetLength());
    }
    dest_span = spancpy(dest_span, search_span);
    CHECK(dest_span.empty());
  }
  m_pData = std::move(newstr_data);
  return count;
}

template <typename T>
void StringTemplate<T>::ReallocBeforeWrite(size_t nNewLength) {
  if (m_pData && m_pData->CanOperateInPlace(nNewLength)) {
    return;
  }
  if (nNewLength == 0) {
    clear();
    return;
  }
  RetainPtr<StringData> pNewData = StringData::Create(nNewLength);
  if (m_pData) {
    size_t nCopyLength = std::min(m_pData->m_nDataLength, nNewLength);
    pNewData->CopyContents({m_pData->m_String, nCopyLength});
    pNewData->m_nDataLength = nCopyLength;
  } else {
    pNewData->m_nDataLength = 0;
  }
  pNewData->m_String[pNewData->m_nDataLength] = 0;
  m_pData = std::move(pNewData);
}

template <typename T>
void StringTemplate<T>::AllocBeforeWrite(size_t nNewLength) {
  if (m_pData && m_pData->CanOperateInPlace(nNewLength)) {
    return;
  }
  if (nNewLength == 0) {
    clear();
    return;
  }
  m_pData = StringData::Create(nNewLength);
}

template <typename T>
void StringTemplate<T>::AssignCopy(const T* pSrcData, size_t nSrcLen) {
  AllocBeforeWrite(nSrcLen);
  m_pData->CopyContents({pSrcData, nSrcLen});
  m_pData->m_nDataLength = nSrcLen;
}

template <typename T>
void StringTemplate<T>::Concat(const T* pSrcData, size_t nSrcLen) {
  if (!pSrcData || nSrcLen == 0) {
    return;
  }

  if (!m_pData) {
    m_pData = StringData::Create({pSrcData, nSrcLen});
    return;
  }

  if (m_pData->CanOperateInPlace(m_pData->m_nDataLength + nSrcLen)) {
    m_pData->CopyContentsAt(m_pData->m_nDataLength, {pSrcData, nSrcLen});
    m_pData->m_nDataLength += nSrcLen;
    return;
  }

  size_t nConcatLen = std::max(m_pData->m_nDataLength / 2, nSrcLen);
  RetainPtr<StringData> pNewData =
      StringData::Create(m_pData->m_nDataLength + nConcatLen);
  pNewData->CopyContents(*m_pData);
  pNewData->CopyContentsAt(m_pData->m_nDataLength, {pSrcData, nSrcLen});
  pNewData->m_nDataLength = m_pData->m_nDataLength + nSrcLen;
  m_pData = std::move(pNewData);
}

template <typename T>
void StringTemplate<T>::clear() {
  if (m_pData && m_pData->CanOperateInPlace(0)) {
    m_pData->m_nDataLength = 0;
    return;
  }
  m_pData.Reset();
}

// Instantiate.
template class StringTemplate<char>;
template class StringTemplate<wchar_t>;

}  // namespace fxcrt
