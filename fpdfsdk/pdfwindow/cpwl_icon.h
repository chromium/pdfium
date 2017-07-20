// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_ICON_H_
#define FPDFSDK_PDFWINDOW_CPWL_ICON_H_

#include <utility>

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

class CPWL_Icon : public CPWL_Wnd {
 public:
  CPWL_Icon();
  ~CPWL_Icon() override;

  void SetIconFit(CPDF_IconFit* pIconFit) { m_pIconFit = pIconFit; }
  void SetPDFStream(CPDF_Stream* pStream) { m_pPDFStream = pStream; }

  // horizontal scale, vertical scale
  std::pair<float, float> GetScale();

  // x, y
  std::pair<float, float> GetImageOffset();

  CFX_Matrix GetImageMatrix();
  CFX_ByteString GetImageAlias();

 private:
  // left, bottom
  std::pair<float, float> GetIconPosition();

  // width, height
  std::pair<float, float> GetImageSize();

  CFX_UnownedPtr<CPDF_Stream> m_pPDFStream;
  CFX_UnownedPtr<CPDF_IconFit> m_pIconFit;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_ICON_H_
