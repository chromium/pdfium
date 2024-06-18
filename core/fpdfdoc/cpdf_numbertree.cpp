// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_numbertree.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"

namespace {

RetainPtr<const CPDF_Object> FindNumberNode(const CPDF_Dictionary* node_dict,
                                            int num) {
  RetainPtr<const CPDF_Array> limits_array = node_dict->GetArrayFor("Limits");
  if (limits_array && (num < limits_array->GetIntegerAt(0) ||
                       num > limits_array->GetIntegerAt(1))) {
    return nullptr;
  }
  RetainPtr<const CPDF_Array> numbers_array = node_dict->GetArrayFor("Nums");
  if (numbers_array) {
    for (size_t i = 0; i < numbers_array->size() / 2; i++) {
      int index = numbers_array->GetIntegerAt(i * 2);
      if (num == index) {
        return numbers_array->GetDirectObjectAt(i * 2 + 1);
      }
      if (index > num) {
        break;
      }
    }
    return nullptr;
  }

  RetainPtr<const CPDF_Array> kids_array = node_dict->GetArrayFor("Kids");
  if (!kids_array) {
    return nullptr;
  }

  for (size_t i = 0; i < kids_array->size(); i++) {
    RetainPtr<const CPDF_Dictionary> kid_dict = kids_array->GetDictAt(i);
    if (!kid_dict) {
      continue;
    }

    RetainPtr<const CPDF_Object> result = FindNumberNode(kid_dict.Get(), num);
    if (result) {
      return result;
    }
  }
  return nullptr;
}

}  // namespace

CPDF_NumberTree::CPDF_NumberTree(RetainPtr<const CPDF_Dictionary> root)
    : root_(std::move(root)) {}

CPDF_NumberTree::~CPDF_NumberTree() = default;

RetainPtr<const CPDF_Object> CPDF_NumberTree::LookupValue(int num) const {
  return FindNumberNode(root_.Get(), num);
}
