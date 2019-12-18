// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_ICON_H_
#define FPDFSDK_PWL_CPWL_ICON_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"

class CPDF_Icon;
class CPDF_IconFit;

class CPWL_Icon final : public CPWL_Wnd {
 public:
  CPWL_Icon(const CreateParams& cp,
            std::unique_ptr<CPDF_Icon> pIcon,
            CPDF_IconFit* pFit);
  ~CPWL_Icon() override;

  // horizontal scale, vertical scale
  std::pair<float, float> GetScale();

  // x, y
  std::pair<float, float> GetImageOffset();

  CFX_Matrix GetImageMatrix();
  ByteString GetImageAlias();

 private:
  CFX_PointF GetIconPosition();  // left, bottom coordinates.
  CFX_SizeF GetImageSize();

  std::unique_ptr<CPDF_Icon> const m_pIcon;
  UnownedPtr<CPDF_IconFit> const m_pIconFit;
};

#endif  // FPDFSDK_PWL_CPWL_ICON_H_
