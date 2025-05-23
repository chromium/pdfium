// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_
#define CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_

#include <stddef.h>

#include <optional>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"

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
  CPDF_Array* AsMutableArray() override;
  bool WriteTo(IFX_ArchiveStream* archive,
               const CPDF_Encryptor* encryptor) const override;

  bool IsEmpty() const { return objects_.empty(); }
  size_t size() const { return objects_.size(); }

  // The Get*ObjectAt() methods tolerate out-of-bounds indices and return
  // nullptr in those cases. Otherwise, for in-bound indices, the result
  // is never nullptr.
  RetainPtr<CPDF_Object> GetMutableObjectAt(size_t index);
  RetainPtr<const CPDF_Object> GetObjectAt(size_t index) const;

  // The Get*DirectObjectAt() methods tolerate out-of-bounds indices and
  // return nullptr in those cases. Furthermore, for reference objects that
  // do not correspond to a valid indirect object, nullptr is returned.
  RetainPtr<CPDF_Object> GetMutableDirectObjectAt(size_t index);
  RetainPtr<const CPDF_Object> GetDirectObjectAt(size_t index) const;

  // The Get*At() methods tolerate out-of-bounds indices and return nullptr
  // in those cases. Furthermore, these safely coerce to the sub-class,
  // returning nullptr if the object at the location is of a different type.
  ByteString GetByteStringAt(size_t index) const;
  WideString GetUnicodeTextAt(size_t index) const;
  bool GetBooleanAt(size_t index, bool bDefault) const;
  int GetIntegerAt(size_t index) const;
  float GetFloatAt(size_t index) const;
  RetainPtr<CPDF_Dictionary> GetMutableDictAt(size_t index);
  RetainPtr<const CPDF_Dictionary> GetDictAt(size_t index) const;
  RetainPtr<CPDF_Stream> GetMutableStreamAt(size_t index);
  RetainPtr<const CPDF_Stream> GetStreamAt(size_t index) const;
  RetainPtr<CPDF_Array> GetMutableArrayAt(size_t index);
  RetainPtr<const CPDF_Array> GetArrayAt(size_t index) const;
  RetainPtr<const CPDF_Number> GetNumberAt(size_t index) const;
  RetainPtr<const CPDF_String> GetStringAt(size_t index) const;

  CFX_FloatRect GetRect() const;
  CFX_Matrix GetMatrix() const;

  std::optional<size_t> Find(const CPDF_Object* pThat) const;
  bool Contains(const CPDF_Object* pThat) const;

  // Creates object owned by the array, and returns a retained pointer to it.
  // We have special cases for objects that can intern strings from
  // a ByteStringPool. Prefer using these templates over direct calls
  // to Append()/SetAt()/InsertAt() since by creating a new object with no
  // previous references, they ensure cycles can not be introduced.
  template <typename T, typename... Args>
    requires(!CanInternStrings<T>::value)
  RetainPtr<T> AppendNew(Args&&... args) {
    static_assert(!std::is_same<T, CPDF_Stream>::value,
                  "Cannot append a CPDF_Stream directly. Add it indirectly as "
                  "a `CPDF_Reference` instead.");
    return pdfium::WrapRetain(static_cast<T*>(
        AppendInternal(pdfium::MakeRetain<T>(std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(CanInternStrings<T>::value)
  RetainPtr<T> AppendNew(Args&&... args) {
    return pdfium::WrapRetain(static_cast<T*>(AppendInternal(
        pdfium::MakeRetain<T>(pool_, std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(!CanInternStrings<T>::value)
  RetainPtr<T> SetNewAt(size_t index, Args&&... args) {
    static_assert(!std::is_same<T, CPDF_Stream>::value,
                  "Cannot set a CPDF_Stream directly. Add it indirectly as a "
                  "`CPDF_Reference` instead.");
    return pdfium::WrapRetain(static_cast<T*>(SetAtInternal(
        index, pdfium::MakeRetain<T>(std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(CanInternStrings<T>::value)
  RetainPtr<T> SetNewAt(size_t index, Args&&... args) {
    return pdfium::WrapRetain(static_cast<T*>(SetAtInternal(
        index, pdfium::MakeRetain<T>(pool_, std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(!CanInternStrings<T>::value)
  RetainPtr<T> InsertNewAt(size_t index, Args&&... args) {
    static_assert(!std::is_same<T, CPDF_Stream>::value,
                  "Cannot insert a CPDF_Stream directly. Add it indirectly as "
                  "a `CPDF_Reference` instead.");
    return pdfium::WrapRetain(static_cast<T*>(InsertAtInternal(
        index, pdfium::MakeRetain<T>(std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(CanInternStrings<T>::value)
  RetainPtr<T> InsertNewAt(size_t index, Args&&... args) {
    return pdfium::WrapRetain(static_cast<T*>(InsertAtInternal(
        index, pdfium::MakeRetain<T>(pool_, std::forward<Args>(args)...))));
  }

  // Adds non-null `object` to the end of the array, growing as appropriate.
  void Append(RetainPtr<CPDF_Object> object);
  void Append(RetainPtr<CPDF_Stream> stream) = delete;

  // Overwrites the object at `index` with non-null `object`, if it is
  // in bounds. Otherwise, `index` is out of bounds, and `object` is
  // not stored.
  void SetAt(size_t index, RetainPtr<CPDF_Object> object);
  void SetAt(size_t index, RetainPtr<CPDF_Stream> stream) = delete;

  // Inserts non-null `object` at `index` and shifts by one position all of the
  // objects beyond it like std::vector::insert(), if `index` is less than or
  // equal to the current array size. Otherwise, `index` is out of bounds,
  // and `object` is not stored.
  void InsertAt(size_t index, RetainPtr<CPDF_Object> object);
  void InsertAt(size_t index, RetainPtr<CPDF_Stream> stream) = delete;

  void Clear();
  void RemoveAt(size_t index);
  void ConvertToIndirectObjectAt(size_t index,
                                 CPDF_IndirectObjectHolder* pHolder);
  bool IsLocked() const { return !!lock_count_; }

 private:
  friend class CPDF_ArrayLocker;

  CPDF_Array();
  explicit CPDF_Array(const WeakPtr<ByteStringPool>& pPool);
  ~CPDF_Array() override;

  // No guarantees about result lifetime, use with caution.
  const CPDF_Object* GetObjectAtInternal(size_t index) const;
  CPDF_Object* GetMutableObjectAtInternal(size_t index);
  CPDF_Object* AppendInternal(RetainPtr<CPDF_Object> pObj);
  CPDF_Object* SetAtInternal(size_t index, RetainPtr<CPDF_Object> pObj);
  CPDF_Object* InsertAtInternal(size_t index, RetainPtr<CPDF_Object> pObj);

  RetainPtr<CPDF_Object> CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* pVisited) const override;

  std::vector<RetainPtr<CPDF_Object>> objects_;
  WeakPtr<ByteStringPool> pool_;
  mutable uint32_t lock_count_ = 0;
};

class CPDF_ArrayLocker {
 public:
  FX_STACK_ALLOCATED();
  using const_iterator = CPDF_Array::const_iterator;

  explicit CPDF_ArrayLocker(const CPDF_Array* pArray);
  explicit CPDF_ArrayLocker(RetainPtr<CPDF_Array> pArray);
  explicit CPDF_ArrayLocker(RetainPtr<const CPDF_Array> pArray);
  ~CPDF_ArrayLocker();

  const_iterator begin() const {
    CHECK(array_->IsLocked());
    return array_->objects_.begin();
  }
  const_iterator end() const {
    CHECK(array_->IsLocked());
    return array_->objects_.end();
  }

 private:
  RetainPtr<const CPDF_Array> const array_;
};

inline CPDF_Array* ToArray(CPDF_Object* obj) {
  return obj ? obj->AsMutableArray() : nullptr;
}

inline const CPDF_Array* ToArray(const CPDF_Object* obj) {
  return obj ? obj->AsArray() : nullptr;
}

inline RetainPtr<CPDF_Array> ToArray(RetainPtr<CPDF_Object> obj) {
  return RetainPtr<CPDF_Array>(ToArray(obj.Get()));
}

inline RetainPtr<const CPDF_Array> ToArray(RetainPtr<const CPDF_Object> obj) {
  return RetainPtr<const CPDF_Array>(ToArray(obj.Get()));
}

#endif  // CORE_FPDFAPI_PARSER_CPDF_ARRAY_H_
