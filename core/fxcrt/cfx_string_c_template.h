// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_CFX_STRING_C_TEMPLATE_H_
#define CORE_FXCRT_CFX_STRING_C_TEMPLATE_H_

#include <algorithm>
#include <type_traits>
#include <vector>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/stl_util.h"

// An immutable string with caller-provided storage which must outlive the
// string itself. These are not necessarily nul-terminated, so that substring
// extraction (via the Mid(), Left(), and Right() methods) is copy-free.
template <typename T>
class CFX_StringCTemplate {
 public:
  using CharType = T;
  using UnsignedType = typename std::make_unsigned<CharType>::type;
  using const_iterator = const CharType*;

  CFX_StringCTemplate() : m_Ptr(nullptr), m_Length(0) {}

  // Deliberately implicit to avoid calling on every string literal.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_StringCTemplate(const CharType* ptr)
      : m_Ptr(reinterpret_cast<const UnsignedType*>(ptr)),
        m_Length(ptr ? FXSYS_len(ptr) : 0) {}

  CFX_StringCTemplate(const CharType* ptr, FX_STRSIZE len)
      : m_Ptr(reinterpret_cast<const UnsignedType*>(ptr)),
        m_Length(len < 0 ? FXSYS_len(ptr) : len) {}

  template <typename U = UnsignedType>
  CFX_StringCTemplate(
      const UnsignedType* ptr,
      FX_STRSIZE size,
      typename std::enable_if<!std::is_same<U, CharType>::value>::type* = 0)
      : m_Ptr(ptr), m_Length(size) {}

  // Deliberately implicit to avoid calling on every string literal.
  // |ch| must be an lvalue that outlives the the CFX_StringCTemplate.
  // NOLINTNEXTLINE(runtime/explicit)
  CFX_StringCTemplate(CharType& ch) {
    m_Ptr = reinterpret_cast<const UnsignedType*>(&ch);
    m_Length = 1;
  }

  CFX_StringCTemplate(const CFX_StringCTemplate& src) {
    m_Ptr = src.m_Ptr;
    m_Length = src.m_Length;
  }

  // Any changes to |vec| invalidate the string.
  explicit CFX_StringCTemplate(const std::vector<UnsignedType>& vec) {
    m_Length = pdfium::CollectionSize<FX_STRSIZE>(vec);
    m_Ptr = m_Length ? vec.data() : nullptr;
  }

  CFX_StringCTemplate& operator=(const CharType* src) {
    m_Ptr = reinterpret_cast<const UnsignedType*>(src);
    m_Length = src ? FXSYS_len(src) : 0;
    return *this;
  }

  CFX_StringCTemplate& operator=(const CFX_StringCTemplate& src) {
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

  bool operator==(const CharType* ptr) const {
    return FXSYS_len(ptr) == m_Length &&
           FXSYS_cmp(ptr, reinterpret_cast<const CharType*>(m_Ptr.Get()),
                     m_Length) == 0;
  }
  bool operator==(const CFX_StringCTemplate& other) const {
    return other.m_Length == m_Length &&
           FXSYS_cmp(reinterpret_cast<const CharType*>(other.m_Ptr.Get()),
                     reinterpret_cast<const CharType*>(m_Ptr.Get()),
                     m_Length) == 0;
  }
  bool operator!=(const CharType* ptr) const { return !(*this == ptr); }
  bool operator!=(const CFX_StringCTemplate& other) const {
    return !(*this == other);
  }

  uint32_t GetID(FX_STRSIZE start_pos = 0) const {
    if (m_Length == 0 || start_pos < 0 || start_pos >= m_Length)
      return 0;

    uint32_t strid = 0;
    FX_STRSIZE size = std::min(4, m_Length - start_pos);
    for (FX_STRSIZE i = 0; i < size; i++)
      strid = strid * 256 + m_Ptr.Get()[start_pos + i];

    return strid << ((4 - size) * 8);
  }

  const UnsignedType* raw_str() const { return m_Ptr.Get(); }
  const CharType* unterminated_c_str() const {
    return reinterpret_cast<const CharType*>(m_Ptr.Get());
  }

  FX_STRSIZE GetLength() const { return m_Length; }
  bool IsEmpty() const { return m_Length == 0; }

  UnsignedType GetAt(FX_STRSIZE index) const { return m_Ptr.Get()[index]; }
  CharType CharAt(FX_STRSIZE index) const {
    return static_cast<CharType>(m_Ptr.Get()[index]);
  }

  FX_STRSIZE Find(CharType ch) const {
    const UnsignedType* found = reinterpret_cast<const UnsignedType*>(FXSYS_chr(
        reinterpret_cast<const CharType*>(m_Ptr.Get()), ch, m_Length));
    return found ? found - m_Ptr.Get() : -1;
  }

  CFX_StringCTemplate Mid(FX_STRSIZE index, FX_STRSIZE count = -1) const {
    index = std::max(0, index);
    if (index > m_Length)
      return CFX_StringCTemplate();

    if (count < 0 || count > m_Length - index)
      count = m_Length - index;

    return CFX_StringCTemplate(m_Ptr.Get() + index, count);
  }

  CFX_StringCTemplate Left(FX_STRSIZE count) const {
    if (count <= 0)
      return CFX_StringCTemplate();

    return CFX_StringCTemplate(m_Ptr.Get(), std::min(count, m_Length));
  }

  CFX_StringCTemplate Right(FX_STRSIZE count) const {
    if (count <= 0)
      return CFX_StringCTemplate();

    count = std::min(count, m_Length);
    return CFX_StringCTemplate(m_Ptr.Get() + m_Length - count, count);
  }

  const UnsignedType& operator[](size_t index) const {
    return m_Ptr.Get()[index];
  }

  bool operator<(const CFX_StringCTemplate& that) const {
    int result = FXSYS_cmp(reinterpret_cast<const CharType*>(m_Ptr.Get()),
                           reinterpret_cast<const CharType*>(that.m_Ptr.Get()),
                           std::min(m_Length, that.m_Length));
    return result < 0 || (result == 0 && m_Length < that.m_Length);
  }

  bool operator>(const CFX_StringCTemplate& that) const {
    int result = FXSYS_cmp(reinterpret_cast<const CharType*>(m_Ptr.Get()),
                           reinterpret_cast<const CharType*>(that.m_Ptr.Get()),
                           std::min(m_Length, that.m_Length));
    return result > 0 || (result == 0 && m_Length > that.m_Length);
  }

 protected:
  CFX_UnownedPtr<const UnsignedType> m_Ptr;
  FX_STRSIZE m_Length;

 private:
  void* operator new(size_t) throw() { return nullptr; }
};

template <typename T>
inline bool operator==(const T* lhs, const CFX_StringCTemplate<T>& rhs) {
  return rhs == lhs;
}

template <typename T>
inline bool operator!=(const T* lhs, const CFX_StringCTemplate<T>& rhs) {
  return rhs != lhs;
}

extern template class CFX_StringCTemplate<char>;
extern template class CFX_StringCTemplate<wchar_t>;

using CFX_ByteStringC = CFX_StringCTemplate<char>;
using CFX_WideStringC = CFX_StringCTemplate<wchar_t>;

#endif  // CORE_FXCRT_CFX_STRING_C_TEMPLATE_H_
