// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_
#define CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Dictionary;

class CPDF_ContentMarkItem final : public Retainable {
 public:
  enum ParamType { kNone, kPropertiesDict, kDirectDict };

  CONSTRUCT_VIA_MAKE_RETAIN;

  const ByteString& GetName() const { return mark_name_; }
  ParamType GetParamType() const { return param_type_; }
  RetainPtr<const CPDF_Dictionary> GetParam() const;
  RetainPtr<CPDF_Dictionary> GetParam();
  const ByteString& GetPropertyName() const { return property_name_; }

  void SetDirectDict(RetainPtr<CPDF_Dictionary> dict);
  void SetPropertiesHolder(RetainPtr<CPDF_Dictionary> pHolder,
                           const ByteString& property_name);

 private:
  explicit CPDF_ContentMarkItem(ByteString name);
  ~CPDF_ContentMarkItem() override;

  ParamType param_type_ = kNone;
  ByteString mark_name_;
  ByteString property_name_;
  RetainPtr<CPDF_Dictionary> properties_holder_;
  RetainPtr<CPDF_Dictionary> direct_dict_;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_
