// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_

#include "PWL_Wnd.h"

class IFX_Edit;

class CPWL_Label : public CPWL_Wnd {
 public:
  CPWL_Label();
  ~CPWL_Label() override;

  void SetText(const FX_WCHAR* csText);
  CFX_WideString GetText() const;
  void SetLimitChar(int32_t nLimitChar);
  void SetHorzScale(int32_t nHorzScale);
  void SetCharSpace(FX_FLOAT fCharSpace);
  CPDF_Rect GetContentRect() const;
  int32_t GetTotalWords();
  CFX_ByteString GetTextAppearanceStream(const CPDF_Point& ptOffset) const;

 protected:
  // CPWL_Wnd
  CFX_ByteString GetClassName() const override;
  void SetFontSize(FX_FLOAT fFontSize) override;
  FX_FLOAT GetFontSize() const override;
  void OnCreated() override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) override;
  void RePosChildWnd() override;

 private:
  void SetParamByFlag();

  IFX_Edit* m_pEdit;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_LABEL_H_
