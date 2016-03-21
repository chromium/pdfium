// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/fpdf_page/pageint.h"

#include "core/fpdfapi/fpdf_parser/include/cpdf_dictionary.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"

void CPDF_Image::Release() {
  if (m_bInline || (m_pStream && m_pStream->GetObjNum() == 0)) {
    delete this;
  }
}
CPDF_Image* CPDF_Image::Clone() {
  if (m_pStream->GetObjNum())
    return m_pDocument->GetPageData()->GetImage(m_pStream);

  CPDF_Image* pImage = new CPDF_Image(m_pDocument);
  pImage->LoadImageF(ToStream(m_pStream->Clone()), m_bInline);
  if (m_bInline)
    pImage->SetInlineDict(ToDictionary(m_pInlineDict->Clone(TRUE)));

  return pImage;
}
CPDF_Image::CPDF_Image(CPDF_Document* pDoc) {
  m_pDocument = pDoc;
  m_pStream = NULL;
  m_pOC = NULL;
  m_bInline = FALSE;
  m_pInlineDict = NULL;
  m_pDIBSource = NULL;
  m_pMask = NULL;
  m_MatteColor = 0;
}
CPDF_Image::~CPDF_Image() {
  if (m_bInline) {
    if (m_pStream) {
      m_pStream->Release();
    }
    if (m_pInlineDict) {
      m_pInlineDict->Release();
    }
  }
}
FX_BOOL CPDF_Image::LoadImageF(CPDF_Stream* pStream, FX_BOOL bInline) {
  m_pStream = pStream;
  if (m_bInline && m_pInlineDict) {
    m_pInlineDict->Release();
    m_pInlineDict = NULL;
  }
  m_bInline = bInline;
  CPDF_Dictionary* pDict = pStream->GetDict();
  if (m_bInline) {
    m_pInlineDict = ToDictionary(pDict->Clone());
  }
  m_pOC = pDict->GetDictBy("OC");
  m_bIsMask =
      !pDict->KeyExist("ColorSpace") || pDict->GetIntegerBy("ImageMask");
  m_bInterpolate = pDict->GetIntegerBy("Interpolate");
  m_Height = pDict->GetIntegerBy("Height");
  m_Width = pDict->GetIntegerBy("Width");
  return TRUE;
}
