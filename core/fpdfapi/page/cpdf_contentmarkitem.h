// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_
#define CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_

#include <memory>

#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CPDF_Dictionary;

class CPDF_ContentMarkItem final : public Retainable {
 public:
  enum ParamType { None, PropertiesDict, DirectDict };

  explicit CPDF_ContentMarkItem(ByteString name);
  ~CPDF_ContentMarkItem() override;

  const ByteString& GetName() const { return m_MarkName; }
  ParamType GetParamType() const { return m_ParamType; }
  const CPDF_Dictionary* GetParam() const;
  CPDF_Dictionary* GetParam();
  const ByteString& GetPropertyName() const { return m_PropertyName; }
  bool HasMCID() const;

  void SetDirectDict(std::unique_ptr<CPDF_Dictionary> pDict);
  void SetPropertiesDict(CPDF_Dictionary* pDict,
                         const ByteString& property_name);

 private:
  ByteString m_MarkName;
  ParamType m_ParamType = None;
  UnownedPtr<CPDF_Dictionary> m_pPropertiesDict;
  ByteString m_PropertyName;
  std::unique_ptr<CPDF_Dictionary> m_pDirectDict;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_CONTENTMARKITEM_H_
