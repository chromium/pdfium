// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/pdfwindow/PWL_Icon.h"
#include "fpdfsdk/include/pdfwindow/PWL_Label.h"
#include "fpdfsdk/include/pdfwindow/PWL_Signature.h"
#include "fpdfsdk/include/pdfwindow/PWL_Utils.h"
#include "fpdfsdk/include/pdfwindow/PWL_Wnd.h"

CPWL_Signature_Image::CPWL_Signature_Image() : m_pImage(NULL) {}

CPWL_Signature_Image::~CPWL_Signature_Image() {}

void CPWL_Signature_Image::SetImage(CFX_DIBSource* pImage) {
  m_pImage = pImage;
}

CFX_DIBSource* CPWL_Signature_Image::GetImage() {
  return m_pImage;
}

void CPWL_Signature_Image::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                              CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  if (m_pImage) {
    CPDF_Rect rcClient = GetClientRect();

    FX_FLOAT x, y;
    pUser2Device->Transform(rcClient.left, rcClient.top, x, y);

    pDevice->StretchDIBits(m_pImage, (int32_t)x, (int32_t)y,
                           (int32_t)rcClient.Width(),
                           (int32_t)rcClient.Height());
  }
}

void CPWL_Signature_Image::GetThisAppearanceStream(
    CFX_ByteTextBuf& sAppStream) {
  sAppStream << CPWL_Image::GetImageAppStream();
}

void CPWL_Signature_Image::GetScale(FX_FLOAT& fHScale, FX_FLOAT& fVScale) {
  FX_FLOAT fImageW, fImageH;

  GetImageSize(fImageW, fImageH);

  CPDF_Rect rcClient = GetClientRect();

  fHScale = rcClient.Width() / fImageW;
  fVScale = rcClient.Height() / fImageH;
}

CPWL_Signature::CPWL_Signature()
    : m_pText(NULL),
      m_pDescription(NULL),
      m_pImage(NULL),
      m_bTextExist(TRUE),
      m_bImageExist(FALSE),
      m_bFlagExist(TRUE) {}

CPWL_Signature::~CPWL_Signature() {}

void CPWL_Signature::SetTextFlag(FX_BOOL bTextExist) {
  m_bTextExist = bTextExist;

  RePosChildWnd();
}

void CPWL_Signature::SetImageFlag(FX_BOOL bImageExist) {
  m_bImageExist = bImageExist;

  RePosChildWnd();
}

void CPWL_Signature::SetFoxitFlag(FX_BOOL bFlagExist) {
  m_bFlagExist = bFlagExist;
}

void CPWL_Signature::SetText(const FX_WCHAR* sText) {
  m_pText->SetText(sText);

  RePosChildWnd();
}

void CPWL_Signature::SetDescription(const FX_WCHAR* string) {
  m_pDescription->SetText(string);

  RePosChildWnd();
}

void CPWL_Signature::SetImage(CFX_DIBSource* pImage) {
  m_pImage->SetImage(pImage);

  RePosChildWnd();
}

void CPWL_Signature::SetImageStream(CPDF_Stream* pStream,
                                    const FX_CHAR* sImageAlias) {
  m_pImage->SetPDFStream(pStream);
  m_pImage->SetImageAlias(sImageAlias);

  RePosChildWnd();
}

void CPWL_Signature::RePosChildWnd() {
  CPDF_Rect rcClient = GetClientRect();

  CPDF_Rect rcText = rcClient;
  CPDF_Rect rcDescription = rcClient;

  FX_BOOL bTextVisible = m_bTextExist && m_pText->GetText().GetLength() > 0;

  if ((bTextVisible || m_bImageExist) &&
      m_pDescription->GetText().GetLength() > 0) {
    if (rcClient.Width() >= rcClient.Height()) {
      rcText.right = rcText.left + rcClient.Width() / 2.0f;
      rcDescription.left = rcDescription.right - rcClient.Width() / 2.0f;
    } else {
      rcText.bottom = rcText.top - rcClient.Height() / 2.0f;
      rcDescription.top = rcDescription.bottom + rcClient.Height() / 2.0f;
    }
  }

  m_pText->SetVisible(bTextVisible);
  m_pImage->SetVisible(m_bImageExist);

  m_pText->Move(rcText, TRUE, FALSE);
  m_pImage->Move(rcText, TRUE, FALSE);
  m_pDescription->Move(rcDescription, TRUE, FALSE);
}

void CPWL_Signature::CreateChildWnd(const PWL_CREATEPARAM& cp) {
  m_pImage = new CPWL_Signature_Image;
  PWL_CREATEPARAM icp = cp;
  icp.pParentWnd = this;
  icp.dwFlags = PWS_CHILD | PWS_VISIBLE;
  icp.sTextColor = CPWL_Color(COLORTYPE_GRAY, 0);
  m_pImage->Create(icp);

  m_pText = new CPWL_Label;
  PWL_CREATEPARAM acp = cp;
  acp.pParentWnd = this;
  acp.dwFlags = PWS_CHILD | PWS_VISIBLE | PWS_AUTOFONTSIZE | PES_MULTILINE |
                PES_AUTORETURN | PES_MIDDLE | PES_CENTER;
  acp.sTextColor = CPWL_Color(COLORTYPE_GRAY, 0);
  m_pText->Create(acp);

  m_pDescription = new CPWL_Label;
  PWL_CREATEPARAM dcp = cp;
  dcp.pParentWnd = this;
  dcp.dwFlags = PWS_CHILD | PWS_VISIBLE | PWS_AUTOFONTSIZE | PES_MULTILINE |
                PES_AUTORETURN | PES_LEFT | PES_CENTER;
  dcp.sTextColor = CPWL_Color(COLORTYPE_GRAY, 0);
  m_pDescription->Create(dcp);
}

void CPWL_Signature::DrawThisAppearance(CFX_RenderDevice* pDevice,
                                        CFX_Matrix* pUser2Device) {
  CPWL_Wnd::DrawThisAppearance(pDevice, pUser2Device);

  if (m_bFlagExist) {
    CPWL_Utils::DrawIconAppStream(
        pDevice, pUser2Device, PWL_ICONTYPE_FOXIT,
        CPWL_Utils::GetCenterSquare(GetClientRect()),
        CPWL_Color(COLORTYPE_RGB, 0.91f, 0.855f, 0.92f),
        CPWL_Color(COLORTYPE_TRANSPARENT), 255);
  }
}

void CPWL_Signature::GetThisAppearanceStream(CFX_ByteTextBuf& sAppStream) {
  CPWL_Wnd::GetThisAppearanceStream(sAppStream);
}
