// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_

#include "PWL_Wnd.h"

class CPWL_Button : public CPWL_Wnd {
 public:
  CPWL_Button();
  ~CPWL_Button() override;

  // CPWL_Wnd
  CFX_ByteString GetClassName() const override;
  void OnCreate(PWL_CREATEPARAM& cp) override;
  FX_BOOL OnLButtonDown(const CPDF_Point& point, FX_DWORD nFlag) override;
  FX_BOOL OnLButtonUp(const CPDF_Point& point, FX_DWORD nFlag) override;

 protected:
  FX_BOOL m_bMouseDown;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_
