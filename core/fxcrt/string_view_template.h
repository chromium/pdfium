// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_STRING_VIEW_TEMPLATE_H_
#define CORE_FXCRT_STRING_VIEW_TEMPLATE_H_

#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/optional.h"
#include "third_party/base/stl_util.h"

namespace fxcrt {

// An immutable string with caller-provided storage which must outlive the
// string itself. These are not necessarily nul-terminated, so that substring
// extraction (via the Mid(), Left(), and Right() methods) is copy-free.
template <typename T>
class StringViewTemplate {
 public:
  using CharType = T;
  using UnsignedType = typename std::make_unsigned<CharType>::type;
  using const_iterator = const CharType*;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  StringViewTemplate() : m_Ptr(nullptr), m_Length(0) {}

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  StringViewTemplate(const CharType* ptr)
      : m_Ptr(reinterpret_cast<const UnsignedType*>(ptr)),
        m_Length(ptr ? FXSYS_len(ptr) : 0) {}

  StringViewTemplate(const CharType* ptr, size_t len)
      : m_Ptr(reinterpret_cast<const UnsignedType*>(ptr)), m_Length(len) {}

  template <typename U = UnsignedType>
  StringViewTemplate(
      const UnsignedType* ptr,
      size_t size,
      typename std::enable_if<!std::is_same<U, CharType>::value>::type* = 0)
      : m_Ptr(ptr), m_Length(size) {}

  // Deliberately implicit to avoid calling on every string literal.
  // |ch| must be an lvalue that outlives the StringViewTemplate.
  // NOLINTNEXTLINE(runtime/explicit)
  StringViewTemplate(CharType& ch) {
    m_Ptr = reinterpret_cast<const UnsignedType*>(&ch);
    m_Length = 1;
  }

  StringViewTemplate(const StringViewTemplate& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  // Any changes to |vec| invalidate the string.
  explicit StringViewTemplate(const std::vector<UnsignedType>& vec) {
    m_Length = vec.size();
    m_Ptr = m_Length ? vec.data() : nullptr;
  }

  StringViewTemplate& operator=(const CharType* src) {
    m_Ptr = reinterpret_cast<const UnsignedType*>(src);
    m_Length = src ? FXSYS_len(src) : 0;
    return *this;
  }

  StringViewTemplate& operator=(const StringViewTemplate& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
    return *this;
  }

  const_iterator begin() const {
    return reinterpret_cast<const CharType*>(m_Ptr.Get());
  }
  const_iterator end() const {
    return m_Ptr ? reinterpret_cast<const CharType*>(m_Ptr.Get()) + m_Length
                 : nullptr;
  }

  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }

  bool operator==(const CharType* ptr) const {
    return FXSYS_len(ptr) == m_Length &&
           FXSYS_cmp(ptr, reinterpret_cast<const CharType*>(m_Ptr.Get()),
                     m_Length) == 0;
  }
  bool operator==(const StringViewTemplate& other) const {
    return other.m_Length == m_Length &&
           FXSYS_cmp(reinterpret_cast<const CharType*>(other.m_Ptr.Get()),
                     reinterpret_cast<const CharType*>(m_Ptr.Get()),
                     m_Length) == 0;
  }
  bool operator!=(const CharType* ptr) const { return !(*this == ptr); }
  bool operator!=(const StringViewTemplate& other) const {
    return !(*this == other);
  }

  uint32_t GetID() const {
    if (m_Length == 0)
      return 0;

    uint32_t strid = 0;
    size_t size = std::min(static_cast<size_t>(4), m_Length);
    for (size_t i = 0; i < size; i++)
      strid = strid * 256 + m_Ptr.Get()[i];

    return strid << ((4 - size) * 8);
  }

  const UnsignedType* raw_str() const { return m_Ptr.Get(); }
  const CharType* unterminated_c_str() const {
    return reinterpret_cast<const CharType*>(m_Ptr.Get());
  }

  size_t GetLength() const { return m_Length; }
  bool IsEmpty() const { return m_Length == 0; }
  bool IsValidIndex(size_t index) const { return index < GetLength(); }
  bool IsValidLength(size_t length) const { return length <= GetLength(); }

  const UnsignedType& operator[](const size_t index) const {
    ASSERT(IsValidIndex(index));
    return m_Ptr.Get()[index];
  }

  UnsignedType First() const { return GetLength() ? (*this)[0] : 0; }

  UnsignedType Last() const {
    return GetLength() ? (*this)[GetLength() - 1] : 0;
  }

  const CharType CharAt(const size_t index) const {
    ASSERT(IsValidIndex(index));
    return static_cast<CharType>(m_Ptr.Get()[index]);
  }

  Optional<size_t> Find(CharType ch) const {
    const auto* found = reinterpret_cast<const UnsignedType*>(FXSYS_chr(
        reinterpret_cast<const CharType*>(m_Ptr.Get()), ch, m_Length));

    return found ? Optional<size_t>(found - m_Ptr.Get()) : Optional<size_t>();
  }

  bool Contains(CharType ch) const { return Find(ch).has_value(); }

  StringViewTemplate Mid(size_t first, size_t count) const {
    if (!m_Ptr.Get())
      return StringViewTemplate();

    if (!IsValidIndex(first))
      return StringViewTemplate();

    if (count == 0 || !IsValidLength(count))
      return StringViewTemplate();

    if (!IsValidIndex(first + count - 1))
      return StringViewTemplate();

    return StringViewTemplate(m_Ptr.Get() + first, count);
  }

  StringViewTemplate Left(size_t count) const {
    if (count == 0 || !IsValidLength(count))
      return StringViewTemplate();
    return Mid(0, count);
  }

  StringViewTemplate Right(size_t count) const {
    if (count == 0 || !IsValidLength(count))
      return StringViewTemplate();
    return Mid(GetLength() - count, count);
  }

  StringViewTemplate TrimmedRight(CharType ch) const {
    if (IsEmpty())
      return StringViewTemplate();

    size_t pos = GetLength();
    while (pos && CharAt(pos - 1) == ch)
      pos--;

    if (pos == 0)
      return StringViewTemplate();

    return StringViewTemplate(m_Ptr.Get(), pos);
  }

  bool operator<(const StringViewTemplate& that) const {
    int result = FXSYS_cmp(reinterpret_cast<const CharType*>(m_Ptr.Get()),
                           reinterpret_cast<const CharType*>(that.m_Ptr.Get()),
                           std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

  bool operator>(const StringViewTemplate& that) const {
    int result = FXSYS_cmp(reinterpret_cast<const CharType*>(m_Ptr.Get()),
                           reinterpret_cast<const CharType*>(that.m_Ptr.Get()),
                           std::min(m_Length, that.m_Length));
    return result > 0 || (result == 0 && m_Length > that.m_Length);
  }

 protected:
  UnownedPtr<const UnsignedType> m_Ptr;
  size_t m_Length;

 private:
  void* operator new(size_t) throw() { return nullptr; }
};

template <typename T>
inline bool operator==(const T* lhs, const StringViewTemplate<T>& rhs) {
  return rhs == lhs;
}
template <typename T>
inline bool operator!=(const T* lhs, const StringViewTemplate<T>& rhs) {
  return rhs != lhs;
}
template <typename T>
inline bool operator<(const T* lhs, const StringViewTemplate<T>& rhs) {
  return rhs > lhs;
}

extern template class StringViewTemplate<char>;
extern template class StringViewTemplate<wchar_t>;

using ByteStringView = StringViewTemplate<char>;
using WideStringView = StringViewTemplate<wchar_t>;

}  // namespace fxcrt

using ByteStringView = fxcrt::ByteStringView;
using WideStringView = fxcrt::WideStringView;

#endif  // CORE_FXCRT_STRING_VIEW_TEMPLATE_H_
