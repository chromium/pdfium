// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_ICON_H_
#define FPDFSDK_PDFWINDOW_CPWL_ICON_H_

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"

class CPWL_Image : public CPWL_Wnd {
 public:
  CPWL_Image();
  ~CPWL_Image() override;

  virtual void GetScale(float& fHScale, float& fVScale);
  virtual void GetImageOffset(float& x, float& y);

  CPDF_Stream* GetPDFStream() const;
  void SetPDFStream(CPDF_Stream* pStream);
  void GetImageSize(float& fWidth, float& fHeight);
  CFX_Matrix GetImageMatrix();
  CFX_ByteString GetImageAlias();
  void SetImageAlias(const char* sImageAlias);

 protected:
  CFX_UnownedPtr<CPDF_Stream> m_pPDFStream;
  CFX_ByteString m_sImageAlias;
};

class CPWL_Icon : public CPWL_Image {
 public:
  CPWL_Icon();
  ~CPWL_Icon() override;

  CPDF_IconFit* GetIconFit() const;

  // CPWL_Image
  void GetScale(float& fHScale, float& fVScale) override;
  void GetImageOffset(float& x, float& y) override;

  int32_t GetScaleMethod();
  bool IsProportionalScale();
  void GetIconPosition(float& fLeft, float& fBottom);

  void SetIconFit(CPDF_IconFit* pIconFit) { m_pIconFit = pIconFit; }

 private:
  CFX_UnownedPtr<CPDF_IconFit> m_pIconFit;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_ICON_H_
