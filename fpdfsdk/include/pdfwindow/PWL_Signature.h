// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_

#include "PWL_Icon.h"
#include "PWL_Wnd.h"

class CFX_DIBSource;
class CPWL_Label;

class CPWL_Signature_Image : public CPWL_Image {
 public:
  CPWL_Signature_Image();
  ~CPWL_Signature_Image() override;

  void SetImage(CFX_DIBSource* pImage);
  CFX_DIBSource* GetImage();

 protected:
  // CPWL_Image
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) override;
  void GetScale(FX_FLOAT& fHScale, FX_FLOAT& fVScale) override;

 private:
  CFX_DIBSource* m_pImage;
};

class CPWL_Signature : public CPWL_Wnd {
 public:
  CPWL_Signature();
  ~CPWL_Signature() override;

  void SetText(const FX_WCHAR* sText);
  void SetDescription(const FX_WCHAR* string);
  void SetImage(CFX_DIBSource* pImage);
  void SetImageStream(CPDF_Stream* pStream, const FX_CHAR* sImageAlias);

  void SetTextFlag(FX_BOOL bTextExist);
  void SetImageFlag(FX_BOOL bImageExist);
  void SetFoxitFlag(FX_BOOL bFlagExist);

 protected:
  // CPWL_Wnd
  void RePosChildWnd() override;
  void CreateChildWnd(const PWL_CREATEPARAM& cp) override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          CFX_Matrix* pUser2Device) override;
  void GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) override;

 private:
  CPWL_Label* m_pText;
  CPWL_Label* m_pDescription;
  CPWL_Signature_Image* m_pImage;

  FX_BOOL m_bTextExist;
  FX_BOOL m_bImageExist;
  FX_BOOL m_bFlagExist;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_SIGNATURE_H_
