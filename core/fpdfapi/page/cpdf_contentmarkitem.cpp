// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_contentmarkitem.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"

CPDF_ContentMarkItem::CPDF_ContentMarkItem(ByteString name)
    : mark_name_(std::move(name)) {}

CPDF_ContentMarkItem::~CPDF_ContentMarkItem() = default;

RetainPtr<const CPDF_Dictionary> CPDF_ContentMarkItem::GetParam() const {
  switch (param_type_) {
    case kPropertiesDict:
      return properties_holder_->GetDictFor(property_name_.AsStringView());
    case kDirectDict:
      return direct_dict_;
    case kNone:
      return nullptr;
  }
}

RetainPtr<CPDF_Dictionary> CPDF_ContentMarkItem::GetParam() {
  return pdfium::WrapRetain(
      const_cast<CPDF_Dictionary*>(std::as_const(*this).GetParam().Get()));
}

void CPDF_ContentMarkItem::SetDirectDict(RetainPtr<CPDF_Dictionary> dict) {
  param_type_ = kDirectDict;
  direct_dict_ = std::move(dict);
}

void CPDF_ContentMarkItem::SetPropertiesHolder(
    RetainPtr<CPDF_Dictionary> pHolder,
    const ByteString& property_name) {
  param_type_ = kPropertiesDict;
  properties_holder_ = std::move(pHolder);
  property_name_ = property_name;
}
