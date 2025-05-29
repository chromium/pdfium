// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_DICTIONARY_H_
#define CORE_FPDFAPI_PARSER_CPDF_DICTIONARY_H_

#include <functional>
#include <map>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/weak_ptr.h"

class CPDF_IndirectObjectHolder;

// Dictionaries never contain nullptr for valid keys, but some of the methods
// will return nullptr to indicate non-existent keys.
class CPDF_Dictionary final : public CPDF_Object {
 public:
  using DictMap = std::map<ByteString, RetainPtr<CPDF_Object>, std::less<>>;
  using const_iterator = DictMap::const_iterator;

  CONSTRUCT_VIA_MAKE_RETAIN;

  // CPDF_Object:
  Type GetType() const override;
  RetainPtr<CPDF_Object> Clone() const override;
  CPDF_Dictionary* AsMutableDictionary() override;
  bool WriteTo(IFX_ArchiveStream* archive,
               const CPDF_Encryptor* encryptor) const override;

  bool IsLocked() const { return !!lock_count_; }

  size_t size() const { return map_.size(); }
  RetainPtr<const CPDF_Object> GetObjectFor(ByteStringView key) const;
  RetainPtr<CPDF_Object> GetMutableObjectFor(ByteStringView key);

  RetainPtr<const CPDF_Object> GetDirectObjectFor(ByteStringView key) const;
  RetainPtr<CPDF_Object> GetMutableDirectObjectFor(ByteStringView key);

  // These will return the string representation of the object specified by
  // |key|, for any object type that has a string representation.
  ByteString GetByteStringFor(ByteStringView key) const;
  ByteString GetByteStringFor(ByteStringView key,
                              ByteStringView default_str) const;
  WideString GetUnicodeTextFor(ByteStringView key) const;

  // This will only return the string representation of a name object specified
  // by |key|. Useful when the PDF spec requires the value to be an object of
  // type name. i.e. /Foo and not (Foo).
  ByteString GetNameFor(ByteStringView key) const;

  bool GetBooleanFor(ByteStringView key, bool bDefault) const;
  int GetIntegerFor(ByteStringView key) const;
  int GetIntegerFor(ByteStringView key, int default_int) const;
  int GetDirectIntegerFor(ByteStringView key) const;
  float GetFloatFor(ByteStringView key) const;
  RetainPtr<const CPDF_Dictionary> GetDictFor(ByteStringView key) const;
  RetainPtr<CPDF_Dictionary> GetMutableDictFor(ByteStringView key);
  RetainPtr<CPDF_Dictionary> GetOrCreateDictFor(ByteStringView key);
  RetainPtr<const CPDF_Array> GetArrayFor(ByteStringView key) const;
  RetainPtr<CPDF_Array> GetMutableArrayFor(ByteStringView key);
  RetainPtr<CPDF_Array> GetOrCreateArrayFor(ByteStringView key);
  RetainPtr<const CPDF_Stream> GetStreamFor(ByteStringView key) const;
  RetainPtr<CPDF_Stream> GetMutableStreamFor(ByteStringView key);
  RetainPtr<const CPDF_Number> GetNumberFor(ByteStringView key) const;
  RetainPtr<const CPDF_String> GetStringFor(ByteStringView key) const;
  CFX_FloatRect GetRectFor(ByteStringView key) const;
  CFX_Matrix GetMatrixFor(ByteStringView key) const;

  bool KeyExist(ByteStringView key) const;
  std::vector<ByteString> GetKeys() const;

  // Creates a new object owned by the dictionary and returns an unowned
  // pointer to it. Invalidates iterators for the element with the key |key|.
  // Prefer using these templates over calls to SetFor(), since by creating
  // a new object with no previous references, they ensure cycles can not be
  // introduced.
  template <typename T, typename... Args>
    requires(!CanInternStrings<T>::value)
  RetainPtr<T> SetNewFor(const ByteString& key, Args&&... args) {
    static_assert(!std::is_same<T, CPDF_Stream>::value,
                  "Cannot set a CPDF_Stream directly. Add it indirectly as a "
                  "`CPDF_Reference` instead.");
    return pdfium::WrapRetain(static_cast<T*>(SetForInternal(
        key, pdfium::MakeRetain<T>(std::forward<Args>(args)...))));
  }
  template <typename T, typename... Args>
    requires(CanInternStrings<T>::value)
  RetainPtr<T> SetNewFor(const ByteString& key, Args&&... args) {
    return pdfium::WrapRetain(static_cast<T*>(SetForInternal(
        key, pdfium::MakeRetain<T>(pool_, std::forward<Args>(args)...))));
  }

  // If `object` is null, then `key` is erased from the map. Otherwise, takes
  // ownership of `object` and stores in in the map. Invalidates iterators for
  // the element with the key `key`.
  void SetFor(const ByteString& key, RetainPtr<CPDF_Object> object);
  // A stream must be indirect and added as a `CPDF_Reference` instead.
  void SetFor(const ByteString& key, RetainPtr<CPDF_Stream> stream) = delete;

  // Convenience functions to convert native objects to array form.
  void SetRectFor(const ByteString& key, const CFX_FloatRect& rect);
  void SetMatrixFor(const ByteString& key, const CFX_Matrix& matrix);

  void ConvertToIndirectObjectFor(const ByteString& key,
                                  CPDF_IndirectObjectHolder* pHolder);

  // Invalidates iterators for the element with the key |key|.
  RetainPtr<CPDF_Object> RemoveFor(ByteStringView key);

  // Invalidates iterators for the element with the key |oldkey|.
  void ReplaceKey(const ByteString& oldkey, const ByteString& newkey);

  WeakPtr<ByteStringPool> GetByteStringPool() const { return pool_; }

 private:
  friend class CPDF_DictionaryLocker;

  CPDF_Dictionary();
  explicit CPDF_Dictionary(const WeakPtr<ByteStringPool>& pPool);
  ~CPDF_Dictionary() override;

  // No guarantees about result lifetime, use with caution.
  const CPDF_Object* GetObjectForInternal(ByteStringView key) const;
  const CPDF_Object* GetDirectObjectForInternal(ByteStringView key) const;
  const CPDF_Array* GetArrayForInternal(ByteStringView key) const;
  const CPDF_Dictionary* GetDictForInternal(ByteStringView key) const;
  const CPDF_Number* GetNumberForInternal(ByteStringView key) const;
  const CPDF_Stream* GetStreamForInternal(ByteStringView key) const;
  const CPDF_String* GetStringForInternal(ByteStringView key) const;
  CPDF_Object* SetForInternal(const ByteString& key,
                              RetainPtr<CPDF_Object> pObj);

  ByteString MaybeIntern(const ByteString& str);
  const CPDF_Dictionary* GetDictInternal() const override;
  RetainPtr<CPDF_Object> CloneNonCyclic(
      bool bDirect,
      std::set<const CPDF_Object*>* visited) const override;

  mutable uint32_t lock_count_ = 0;
  WeakPtr<ByteStringPool> pool_;
  DictMap map_;
};

class CPDF_DictionaryLocker {
 public:
  FX_STACK_ALLOCATED();
  using const_iterator = CPDF_Dictionary::const_iterator;

  explicit CPDF_DictionaryLocker(const CPDF_Dictionary* dict);
  explicit CPDF_DictionaryLocker(RetainPtr<CPDF_Dictionary> dict);
  explicit CPDF_DictionaryLocker(RetainPtr<const CPDF_Dictionary> dict);
  ~CPDF_DictionaryLocker();

  const_iterator begin() const {
    CHECK(dict_->IsLocked());
    return dict_->map_.begin();
  }
  const_iterator end() const {
    CHECK(dict_->IsLocked());
    return dict_->map_.end();
  }

 private:
  RetainPtr<const CPDF_Dictionary> const dict_;
};

inline CPDF_Dictionary* ToDictionary(CPDF_Object* obj) {
  return obj ? obj->AsMutableDictionary() : nullptr;
}

inline const CPDF_Dictionary* ToDictionary(const CPDF_Object* obj) {
  return obj ? obj->AsDictionary() : nullptr;
}

inline RetainPtr<CPDF_Dictionary> ToDictionary(RetainPtr<CPDF_Object> obj) {
  return RetainPtr<CPDF_Dictionary>(ToDictionary(obj.Get()));
}

inline RetainPtr<const CPDF_Dictionary> ToDictionary(
    RetainPtr<const CPDF_Object> obj) {
  return RetainPtr<const CPDF_Dictionary>(ToDictionary(obj.Get()));
}

#endif  // CORE_FPDFAPI_PARSER_CPDF_DICTIONARY_H_
