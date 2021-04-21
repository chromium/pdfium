// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_icon.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "core/fpdfdoc/cpdf_icon.h"
#include "core/fpdfdoc/cpdf_iconfit.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "third_party/base/check.h"

CPWL_Icon::CPWL_Icon(const CreateParams& cp) : CPWL_Wnd(cp, nullptr) {}

CPWL_Icon::~CPWL_Icon() = default;
