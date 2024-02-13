// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/string_template.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/span_util.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/containers/span.h"

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
