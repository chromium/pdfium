// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/theme/cfwl_pictureboxtp.h"

#include "xfa/fwl/cfwl_picturebox.h"
#include "xfa/fwl/cfwl_themebackground.h"
#include "xfa/fwl/cfwl_widget.h"

namespace pdfium {

CFWL_PictureBoxTP::CFWL_PictureBoxTP() = default;

CFWL_PictureBoxTP::~CFWL_PictureBoxTP() = default;

void CFWL_PictureBoxTP::DrawBackground(const CFWL_ThemeBackground& pParams) {
  switch (pParams.GetPart()) {
    case CFWL_ThemePart::Part::kBorder:
      DrawBorder(pParams.GetGraphics(), pParams.m_PartRect, pParams.m_matrix);
      break;
    default:
      break;
  }
}

}  // namespace pdfium
