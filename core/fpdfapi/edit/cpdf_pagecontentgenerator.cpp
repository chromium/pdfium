// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"

namespace {

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& ar, const CFX_Matrix& matrix) {
  ar << matrix.a << " " << matrix.b << " " << matrix.c << " " << matrix.d << " "
     << matrix.e << " " << matrix.f;
  return ar;
}

}  // namespace

CPDF_PageContentGenerator::CPDF_PageContentGenerator(CPDF_Page* pPage)
    : m_pPage(pPage), m_pDocument(m_pPage->m_pDocument) {
  for (const auto& pObj : *pPage->GetPageObjectList()) {
    if (pObj)
      m_pageObjects.push_back(pObj.get());
  }
}

CPDF_PageContentGenerator::~CPDF_PageContentGenerator() {}

void CPDF_PageContentGenerator::GenerateContent() {
  CFX_ByteTextBuf buf;
  for (CPDF_PageObject* pPageObj : m_pageObjects) {
    CPDF_ImageObject* pImageObject = pPageObj->AsImage();
    if (pImageObject)
      ProcessImage(&buf, pImageObject);
  }
  CPDF_Dictionary* pPageDict = m_pPage->m_pFormDict;
  CPDF_Object* pContent =
      pPageDict ? pPageDict->GetDirectObjectFor("Contents") : nullptr;
  if (pContent)
    pPageDict->RemoveFor("Contents");

  CPDF_Stream* pStream = m_pDocument->NewIndirect<CPDF_Stream>();
  pStream->SetData(buf.GetBuffer(), buf.GetLength());
  pPageDict->SetNewFor<CPDF_Reference>("Contents", m_pDocument,
                                       pStream->GetObjNum());
}

CFX_ByteString CPDF_PageContentGenerator::RealizeResource(
    uint32_t dwResourceObjNum,
    const CFX_ByteString& bsType) {
  ASSERT(dwResourceObjNum);
  if (!m_pPage->m_pResources) {
    m_pPage->m_pResources = m_pDocument->NewIndirect<CPDF_Dictionary>();
    m_pPage->m_pFormDict->SetNewFor<CPDF_Reference>(
        "Resources", m_pDocument, m_pPage->m_pResources->GetObjNum());
  }
  CPDF_Dictionary* pResList = m_pPage->m_pResources->GetDictFor(bsType);
  if (!pResList)
    pResList = m_pPage->m_pResources->SetNewFor<CPDF_Dictionary>(bsType);

  CFX_ByteString name;
  int idnum = 1;
  while (1) {
    name.Format("FX%c%d", bsType[0], idnum);
    if (!pResList->KeyExist(name)) {
      break;
    }
    idnum++;
  }
  pResList->SetNewFor<CPDF_Reference>(name, m_pDocument, dwResourceObjNum);
  return name;
}

void CPDF_PageContentGenerator::ProcessImage(CFX_ByteTextBuf* buf,
                                             CPDF_ImageObject* pImageObj) {
  if ((pImageObj->matrix().a == 0 && pImageObj->matrix().b == 0) ||
      (pImageObj->matrix().c == 0 && pImageObj->matrix().d == 0)) {
    return;
  }
  *buf << "q " << pImageObj->matrix() << " cm ";

  CPDF_Image* pImage = pImageObj->GetImage();
  if (pImage->IsInline())
    return;

  CPDF_Stream* pStream = pImage->GetStream();
  if (!pStream)
    return;

  bool bWasInline = pStream->IsInline();
  if (bWasInline)
    pImage->ConvertStreamToIndirectObject();

  uint32_t dwObjNum = pStream->GetObjNum();
  CFX_ByteString name = RealizeResource(dwObjNum, "XObject");
  if (bWasInline)
    pImageObj->SetUnownedImage(m_pDocument->GetPageData()->GetImage(dwObjNum));

  *buf << "/" << PDF_NameEncode(name) << " Do Q\n";
}
