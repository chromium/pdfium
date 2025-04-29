// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/string_template.h"

#include <algorithm>
#include <utility>

#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"

namespace fxcrt {

template <typename T>
pdfium::span<T> StringTemplate<T>::GetBuffer(size_t nMinBufLength) {
  if (!data_) {
    if (nMinBufLength == 0) {
      return pdfium::span<T>();
    }
    data_ = StringData::Create(nMinBufLength);
    data_->data_length_ = 0;
    data_->string_[0] = 0;
    return data_->alloc_span();
  }
  if (data_->CanOperateInPlace(nMinBufLength)) {
    return data_->alloc_span();
  }
  nMinBufLength = std::max(nMinBufLength, data_->data_length_);
  if (nMinBufLength == 0) {
    return pdfium::span<T>();
  }
  RetainPtr<StringData> pNewData = StringData::Create(nMinBufLength);
  pNewData->CopyContents(*data_);
  pNewData->data_length_ = data_->data_length_;
  data_ = std::move(pNewData);
  return data_->alloc_span();
}

template <typename T>
void StringTemplate<T>::ReleaseBuffer(size_t nNewLength) {
  if (!data_) {
    return;
  }
  nNewLength = std::min(nNewLength, data_->alloc_length_);
  if (nNewLength == 0) {
    clear();
    return;
  }
  DCHECK_EQ(data_->refs_, 1);
  data_->data_length_ = nNewLength;
  data_->capacity_span()[nNewLength] = 0;
  if (data_->alloc_length_ - nNewLength >= 32) {
    // Over arbitrary threshold, so pay the price to relocate.  Force copy to
    // always occur by holding a second reference to the string.
    StringTemplate preserve(*this);
    ReallocBeforeWrite(nNewLength);
  }
}

template <typename T>
size_t StringTemplate<T>::Remove(T chRemove) {
  size_t count = std::count(span().begin(), span().end(), chRemove);
  if (count == 0) {
    return 0;
  }
  ReallocBeforeWrite(data_->data_length_);
  auto src_span = data_->span();
  auto dst_span = data_->span();
  // Perform self-intersecting copy in forwards order.
  while (!src_span.empty()) {
    if (src_span[0] != chRemove) {
      dst_span[0] = src_span[0];
      dst_span = dst_span.template subspan<1u>();
    }
    src_span = src_span.template subspan<1u>();
  }
  data_->data_length_ -= count;
  data_->capacity_span()[data_->data_length_] = 0;
  return count;
}

template <typename T>
size_t StringTemplate<T>::Insert(size_t index, T ch) {
  const size_t cur_length = GetLength();
  if (!IsValidLength(index)) {
    return cur_length;
  }
  const size_t new_length = cur_length + 1;
  ReallocBeforeWrite(new_length);
  fxcrt::spanmove(data_->capacity_span().subspan(index + 1),
                  data_->capacity_span().subspan(index, new_length - index));
  data_->capacity_span()[index] = ch;
  data_->data_length_ = new_length;
  return new_length;
}

template <typename T>
size_t StringTemplate<T>::Delete(size_t index, size_t count) {
  if (!data_) {
    return 0;
  }
  size_t old_length = data_->data_length_;
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
      data_->capacity_span().subspan(index),
      data_->capacity_span().subspan(removal_length, chars_to_copy));
  data_->data_length_ = old_length - count;
  return data_->data_length_;
}

template <typename T>
void StringTemplate<T>::SetAt(size_t index, T ch) {
  DCHECK(IsValidIndex(index));
  ReallocBeforeWrite(data_->data_length_);
  data_->span()[index] = ch;
}

template <typename T>
std::optional<size_t> StringTemplate<T>::Find(T ch, size_t start) const {
  return Find(StringView(ch), start);
}

template <typename T>
std::optional<size_t> StringTemplate<T>::Find(StringView str,
                                              size_t start) const {
  if (!data_) {
    return std::nullopt;
  }
  if (!IsValidIndex(start)) {
    return std::nullopt;
  }
  std::optional<size_t> result =
      spanpos(data_->span().subspan(start), str.span());
  if (!result.has_value()) {
    return std::nullopt;
  }
  return start + result.value();
}

template <typename T>
std::optional<size_t> StringTemplate<T>::ReverseFind(T ch) const {
  if (!data_) {
    return std::nullopt;
  }
  size_t nLength = data_->data_length_;
  while (nLength--) {
    if (data_->span()[nLength] == ch) {
      return nLength;
    }
  }
  return std::nullopt;
}

template <typename T>
size_t StringTemplate<T>::Replace(StringView oldstr, StringView newstr) {
  if (!data_ || oldstr.IsEmpty()) {
    return 0;
  }
  size_t count = 0;
  {
    // Limit span lifetime.
    pdfium::span<const T> search_span = data_->span();
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
  size_t nNewLength =
      data_->data_length_ + count * (newstr.GetLength() - oldstr.GetLength());
  if (nNewLength == 0) {
    clear();
    return count;
  }
  RetainPtr<StringData> newstr_data = StringData::Create(nNewLength);
  {
    // Spans can't outlive StringData buffers.
    pdfium::span<const T> search_span = data_->span();
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
  data_ = std::move(newstr_data);
  return count;
}

template <typename T>
void StringTemplate<T>::Trim(T ch) {
  TrimFront(ch);
  TrimBack(ch);
}

template <typename T>
void StringTemplate<T>::TrimFront(T ch) {
  TrimFront(StringView(ch));
}

template <typename T>
void StringTemplate<T>::TrimBack(T ch) {
  TrimBack(StringView(ch));
}

template <typename T>
void StringTemplate<T>::Trim(StringView targets) {
  TrimFront(targets);
  TrimBack(targets);
}

template <typename T>
void StringTemplate<T>::TrimFront(StringView targets) {
  if (!data_ || targets.IsEmpty()) {
    return;
  }

  size_t len = GetLength();
  if (len == 0) {
    return;
  }

  size_t pos = 0;
  while (pos < len) {
    size_t i = 0;
    while (i < targets.GetLength() && targets.CharAt(i) != data_->span()[pos]) {
      i++;
    }
    if (i == targets.GetLength()) {
      break;
    }
    pos++;
  }
  if (!pos) {
    return;
  }

  ReallocBeforeWrite(len);
  size_t nDataLength = len - pos;
  // Move the terminating NUL as well.
  fxcrt::spanmove(data_->capacity_span(),
                  data_->capacity_span().subspan(pos, nDataLength + 1));
  data_->data_length_ = nDataLength;
}

template <typename T>
void StringTemplate<T>::TrimBack(StringView targets) {
  if (!data_ || targets.IsEmpty()) {
    return;
  }

  size_t pos = GetLength();
  if (pos == 0) {
    return;
  }

  while (pos) {
    size_t i = 0;
    while (i < targets.GetLength() &&
           targets.CharAt(i) != data_->span()[pos - 1]) {
      i++;
    }
    if (i == targets.GetLength()) {
      break;
    }
    pos--;
  }
  if (pos < data_->data_length_) {
    ReallocBeforeWrite(data_->data_length_);
    data_->data_length_ = pos;
    data_->capacity_span()[data_->data_length_] = 0;
  }
}

template <typename T>
void StringTemplate<T>::ReallocBeforeWrite(size_t nNewLength) {
  if (data_ && data_->CanOperateInPlace(nNewLength)) {
    return;
  }
  if (nNewLength == 0) {
    clear();
    return;
  }
  RetainPtr<StringData> pNewData = StringData::Create(nNewLength);
  if (data_) {
    size_t nCopyLength = std::min(data_->data_length_, nNewLength);
    // SAFETY: copy of no more than data_length_ bytes.
    pNewData->CopyContents(
        UNSAFE_BUFFERS(pdfium::span(data_->string_, nCopyLength)));
    pNewData->data_length_ = nCopyLength;
  } else {
    pNewData->data_length_ = 0;
  }
  pNewData->capacity_span()[pNewData->data_length_] = 0;
  data_ = std::move(pNewData);
}

template <typename T>
void StringTemplate<T>::AllocBeforeWrite(size_t nNewLength) {
  if (data_ && data_->CanOperateInPlace(nNewLength)) {
    return;
  }
  if (nNewLength == 0) {
    clear();
    return;
  }
  data_ = StringData::Create(nNewLength);
}

template <typename T>
void StringTemplate<T>::AssignCopy(const T* pSrcData, size_t nSrcLen) {
  AllocBeforeWrite(nSrcLen);
  // SAFETY: AllocBeforeWrite() ensures `nSrcLen` bytes available.
  data_->CopyContents(UNSAFE_BUFFERS(pdfium::span(pSrcData, nSrcLen)));
  data_->data_length_ = nSrcLen;
}

template <typename T>
void StringTemplate<T>::Concat(const T* pSrcData, size_t nSrcLen) {
  if (!pSrcData || nSrcLen == 0) {
    return;
  }
  // SAFETY: required from caller.
  // TODO(tsepez): should be UNSAFE_BUFFER_USAGE or pass span.
  auto src_span = UNSAFE_BUFFERS(pdfium::span(pSrcData, nSrcLen));
  if (!data_) {
    data_ = StringData::Create(src_span);
    return;
  }
  if (data_->CanOperateInPlace(data_->data_length_ + nSrcLen)) {
    data_->CopyContentsAt(data_->data_length_, src_span);
    data_->data_length_ += nSrcLen;
    return;
  }
  // Increase size by at least 50% to amortize repeated concatenations.
  size_t nGrowLen = std::max(data_->data_length_ / 2, nSrcLen);
  RetainPtr<StringData> pNewData =
      StringData::Create(data_->data_length_ + nGrowLen);
  pNewData->CopyContents(*data_);
  pNewData->CopyContentsAt(data_->data_length_, src_span);
  pNewData->data_length_ = data_->data_length_ + nSrcLen;
  data_ = std::move(pNewData);
}

template <typename T>
void StringTemplate<T>::clear() {
  if (data_ && data_->CanOperateInPlace(0)) {
    data_->data_length_ = 0;
    return;
  }
  data_.Reset();
}

// Instantiate.
template class StringTemplate<char>;
template class StringTemplate<wchar_t>;

}  // namespace fxcrt
