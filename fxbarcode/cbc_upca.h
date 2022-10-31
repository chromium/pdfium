// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_CBC_UPCA_H_
#define FXBARCODE_CBC_UPCA_H_

#include <stddef.h>

#include "fxbarcode/BC_Library.h"
#include "fxbarcode/cbc_eancode.h"

class CBC_UPCA final : public CBC_EANCode {
 public:
  CBC_UPCA();
  ~CBC_UPCA() override;

  // CBC_EANCode:
  BC_TYPE GetType() override;
  size_t GetMaxLength() const override;
};

#endif  // FXBARCODE_CBC_UPCA_H_
