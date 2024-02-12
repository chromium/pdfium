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
