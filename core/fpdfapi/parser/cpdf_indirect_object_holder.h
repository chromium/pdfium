// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_
#define CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_

#include <stdint.h>

#include <map>
#include <type_traits>
#include <utility>

#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/weak_ptr.h"

class CPDF_IndirectObjectHolder {
 public:
  using const_iterator =
      std::map<uint32_t, RetainPtr<CPDF_Object>>::const_iterator;

  CPDF_IndirectObjectHolder();
  virtual ~CPDF_IndirectObjectHolder();

  RetainPtr<CPDF_Object> GetOrParseIndirectObject(uint32_t objnum);
  RetainPtr<const CPDF_Object> GetIndirectObject(uint32_t objnum) const;
  RetainPtr<CPDF_Object> GetMutableIndirectObject(uint32_t objnum);
  void DeleteIndirectObject(uint32_t objnum);

  // Creates and adds a new object retained by the indirect object holder,
  // and returns a retained pointer to it.
  template <typename T, typename... Args>
  RetainPtr<T> NewIndirect(Args&&... args) {
    auto obj = New<T>(std::forward<Args>(args)...);
    AddIndirectObject(obj);
    return obj;
  }

  // Creates and adds a new object not retained by the indirect object holder,
  // but which can intern strings from it. We have a special cast to handle
  // objects that can intern strings from our ByteStringPool.
  template <typename T, typename... Args>
    requires(CanInternStrings<T>::value)
  RetainPtr<T> New(Args&&... args) {
    return pdfium::MakeRetain<T>(byte_string_pool_,
                                 std::forward<Args>(args)...);
  }
  template <typename T, typename... Args>
    requires(!CanInternStrings<T>::value)
  RetainPtr<T> New(Args&&... args) {
    return pdfium::MakeRetain<T>(std::forward<Args>(args)...);
  }

  // Always Retains |pObj|, returns its new object number.
  uint32_t AddIndirectObject(RetainPtr<CPDF_Object> pObj);

  // If higher generation number, retains |pObj| and returns true.
  bool ReplaceIndirectObjectIfHigherGeneration(uint32_t objnum,
                                               RetainPtr<CPDF_Object> pObj);

  uint32_t GetLastObjNum() const { return last_obj_num_; }
  void SetLastObjNum(uint32_t objnum) { last_obj_num_ = objnum; }

  WeakPtr<ByteStringPool> GetByteStringPool() const {
    return byte_string_pool_;
  }

  const_iterator begin() const { return indirect_objs_.begin(); }
  const_iterator end() const { return indirect_objs_.end(); }

 protected:
  virtual RetainPtr<CPDF_Object> ParseIndirectObject(uint32_t objnum);

 private:
  friend class CPDF_Reference;

  const CPDF_Object* GetIndirectObjectInternal(uint32_t objnum) const;
  CPDF_Object* GetOrParseIndirectObjectInternal(uint32_t objnum);

  uint32_t last_obj_num_ = 0;
  std::map<uint32_t, RetainPtr<CPDF_Object>> indirect_objs_;
  WeakPtr<ByteStringPool> byte_string_pool_;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_INDIRECT_OBJECT_HOLDER_H_
