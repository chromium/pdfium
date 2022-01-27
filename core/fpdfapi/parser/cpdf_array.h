// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_
#define CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_

#include <stddef.h>

#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/check.h"

// Arrays never contain nullptrs for objects within bounds, but some of the
// methods will tolerate out-of-bounds indices and return nullptr for those
// cases.
class CPDF_Array final : public CPDF_Object {
 public:
  using const_iterator = std::vector<RetainPtr<CPDF_Object>>::const_iterator;

  CONSTRUCT_VIA_MAKE_RETAIN;

  // CPDF_Object:
  Type GetType() const override;
  RetainPtr<CPDF_Object> Clone() const override;
  bool IsArray() const override;
  CPDF_Array* AsArray() override;
  const CPDF_Array* AsArray() const override;
  bool WriteTo(IFX_ArchiveStream* archive,
               const CPDF_Encryptor* encryptor) const override;

  bool IsEmpty() const { return m_Objects.empty(); }
  size_t size() const { return m_Objects.size(); }

  // The GetObjectAt() methods tolerate out-of-bounds indices and return
  // nullptr in those cases. Otherwise, for in-bound indices, the result
  // is never nullptr.
  CPDF_Object* GetObjectAt(size_t index);
  const CPDF_Object* GetObjectAt(size_t index) const;

  // The GetDirectObjectAt() methods tolerate out-of-bounds indices and
  // return nullptr in those cases. Furthermore, for reference objects that
  // do not correspond to a valid indirect object, nullptr is returned.
  CPDF_Object* GetDirectObjectAt(size_t index);
  const CPDF_Object* GetDirectObjectAt(size_t index) const;

  // The Get*At() methods tolerate out-of-bounds indices and return nullptr
  // in those cases. Furthermore, these safely coerce to the sub-class,
  // returning nullptr if the object at the location is of a different type.
  ByteString GetStringAt(size_t index) const;
  WideString GetUnicodeTextAt(size_t index) const;
  bool GetBooleanAt(size_t index, bool bDefault) const;
  int GetIntegerAt(size_t index) const;
  float GetNumberAt(size_t index) const;
  CPDF_Dictionary* GetDictAt(size_t index);
  const CPDF_Dictionary* GetDictAt(size_t index) const;
  CPDF_Stream* GetStreamAt(size_t index);
  const CPDF_Stream* GetStreamAt(size_t index) const;
  CPDF_Array* GetArrayAt(size_t index);
  const CPDF_Array* GetArrayAt(size_t index) const;

  CFX_FloatRect GetRect() const;
  CFX_Matrix GetMatrix() const;

  absl::optional<size_t> Find(const CPDF_Object* pThat) const;
  bool Contains(const CPDF_Object* pThat) const;

  // Creates object owned by the array, returns unowned pointer to it.
  // We have special cases for objects that can intern strings from
  // a ByteStringPool. Prefer using these templates over direct calls
  // to Append()/SetAt()/InsertAt() since by creating a new object with no
  // previous references, they ensure cycles can not be introduced.
  template <typename T, typename... Args>
  typename std::enable_if<!CanInternStrings<T>::value, T*>::type AppendNew(
      Args&&... args) {
    return static_cast<T*>(
        Append(pdfium::MakeRetain<T>(std::forward<Args>(args)...)));
  }
  template <typename T, typename... Args>
  typename std::enable_if<CanInternStrings<T>::value, T*>::type AppendNew(
      Args&&... args) {
    return static_cast<T*>(
        Append(pdfium::MakeRetain<T>(m_pPool, std::forward<Args>(args)...)));
  }
  template <typename T, typename... Args>
  typename std::enable_if<!CanInternStrings<T>::value, T*>::type SetNewAt(
      size_t index,
      Args&&... args) {
    return static_cast<T*>(
        SetAt(index, pdfium::MakeRetain<T>(std::forward<Args>(args)...)));
  }
  template <typename T, typename... Args>
  typename std::enable_if<CanInternStrings<T>::value, T*>::type SetNewAt(
      size_t index,
      Args&&... args) {
    return static_cast<T*>(SetAt(
        index, pdfium::MakeRetain<T>(m_pPool, std::forward<Args>(args)...)));
  }
  template <typename T, typename... Args>
  typename std::enable_if<!CanInternStrings<T>::value, T*>::type InsertNewAt(
      size_t index,
      Args&&... args) {
    return static_cast<T*>(
        InsertAt(index, pdfium::MakeRetain<T>(std::forward<Args>(args)...)));
  }
  template <typename T, typename... Args>
  typename std::enable_if<CanInternStrings<T>::value, T*>::type InsertNewAt(
      size_t index,
      Args&&... args) {
    return static_cast<T*>(InsertAt(
        index, pdfium::MakeRetain<T>(m_pPool, std::forward<Args>(args)...)));
  }

  // Adds non-null `pObj` to the end of the array, growing as appropriate.
  // Retains reference to `pObj`, and returns raw pointer for convenience.
  CPDF_Object* Append(RetainPtr<CPDF_Object> pObj);

  // Overwrites the object at `index` with non-null `pObj`. If `index` is
  // less than the array size, then retains reference to `pObj`, and returns
  // raw pointer for convenience. Otherwise, `index` is out of bounds, and
  // `pObj` is neither stored nor retained, and nullptr is returned.
  CPDF_Object* SetAt(size_t index, RetainPtr<CPDF_Object> pObj);

  // Inserts non-null `pObj` at `index` and shifts by one position all of the
  // objects beyond it like std::vector::insert(). If `index` is less than or
  // equal to the current array size, then retains reference to `pObj`, and
  // returns raw pointer for convenience. Otherwise, `index` is out of bounds,
  // and `pObj` is neither stored nor retained, and nullptr is returned.
  CPDF_Object* InsertAt(size_t index, RetainPtr<CPDF_Object> pObj);

  void Clear();
  void RemoveAt(size_t index);
  void ConvertToIndirectObjectAt(size_t index,
                                 CPDF_IndirectObjectHolder* pHolder);
  bool IsLocked() const { return !!m_LockCount; }

 private:
  friend class CPDF_ArrayLocker;

  CPDF_Array();
  explicit CPDF_Array(const WeakPtr<ByteStringPool>& pPool);
  ~CPDF_Array() override;

  RetainPtr<CPDF_Object> CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* pVisited) const override;

  std::vector<RetainPtr<CPDF_Object>> m_Objects;
  WeakPtr<ByteStringPool> m_pPool;
  mutable uint32_t m_LockCount = 0;
};

class CPDF_ArrayLocker {
 public:
  FX_STACK_ALLOCATED();
  using const_iterator = CPDF_Array::const_iterator;

  explicit CPDF_ArrayLocker(const CPDF_Array* pArray);
  ~CPDF_ArrayLocker();

  const_iterator begin() const {
    CHECK(m_pArray->IsLocked());
    return m_pArray->m_Objects.begin();
  }
  const_iterator end() const {
    CHECK(m_pArray->IsLocked());
    return m_pArray->m_Objects.end();
  }

 private:
  RetainPtr<const CPDF_Array> const m_pArray;
};

inline CPDF_Array* ToArray(CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

inline const CPDF_Array* ToArray(const CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

inline RetainPtr<CPDF_Array> ToArray(RetainPtr<CPDF_Object> obj) {
  return RetainPtr<CPDF_Array>(ToArray(obj.Get()));
}

#endif  // CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_
