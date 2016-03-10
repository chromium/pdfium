// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_CPDF_BOOLEAN_H_
#define CORE_INCLUDE_FPDFAPI_CPDF_BOOLEAN_H_

#include "core/include/fpdfapi/cpdf_object.h"
#include "core/include/fxcrt/fx_string.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_Boolean : public CPDF_Object {
 public:
  CPDF_Boolean();
  explicit CPDF_Boolean(FX_BOOL value);

  // CPDF_Object.
  Type GetType() const override;
  CPDF_Object* Clone(FX_BOOL bDirect = FALSE) const override;
  CFX_ByteString GetString() const override;
  int GetInteger() const override;
  void SetString(const CFX_ByteString& str) override;
  bool IsBoolean() const override;
  CPDF_Boolean* AsBoolean() override;
  const CPDF_Boolean* AsBoolean() const override;

 protected:
  ~CPDF_Boolean() override;

  FX_BOOL m_bValue;
};

#endif  // CORE_INCLUDE_FPDFAPI_CPDF_BOOLEAN_H_
