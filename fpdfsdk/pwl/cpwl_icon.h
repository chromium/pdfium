// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_ICON_H_
#define FPDFSDK_PWL_CPWL_ICON_H_

#include "fpdfsdk/pwl/cpwl_wnd.h"

class CPWL_Icon final : public CPWL_Wnd {
 public:
  CPWL_Icon(const CreateParams& cp);
  ~CPWL_Icon() override;
};

#endif  // FPDFSDK_PWL_CPWL_ICON_H_
