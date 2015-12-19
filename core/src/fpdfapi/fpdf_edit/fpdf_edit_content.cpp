// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/include/fpdfapi/fpdf_serial.h"
#include "core/src/fpdfapi/fpdf_page/pageint.h"

CFX_ByteTextBuf& operator<<(CFX_ByteTextBuf& ar, CFX_Matrix& matrix) {
  ar << matrix.a << " " << matrix.b << " " << matrix.c << " " << matrix.d << " "
     << matrix.e << " " << matrix.f;
  return ar;
}
CPDF_PageContentGenerate::CPDF_PageContentGenerate(CPDF_Page* pPage)
    : m_pPage(pPage) {
  m_pDocument = NULL;
  if (m_pPage) {
    m_pDocument = m_pPage->m_pDocument;
  }
  FX_POSITION pos = pPage->GetFirstObjectPosition();
  while (pos) {
    InsertPageObject(pPage->GetNextObject(pos));
  }
}
CPDF_PageContentGenerate::~CPDF_PageContentGenerate() {}
FX_BOOL CPDF_PageContentGenerate::InsertPageObject(
    CPDF_PageObject* pPageObject) {
  if (!pPageObject) {
    return FALSE;
  }
  return m_pageObjects.Add(pPageObject);
}
void CPDF_PageContentGenerate::GenerateContent() {
  CFX_ByteTextBuf buf;
  CPDF_Dictionary* pPageDict = m_pPage->m_pFormDict;
  for (int i = 0; i < m_pageObjects.GetSize(); ++i) {
    CPDF_PageObject* pPageObj = m_pageObjects[i];
    if (!pPageObj || pPageObj->m_Type != PDFPAGE_IMAGE) {
      continue;
    }
    ProcessImage(buf, (CPDF_ImageObject*)pPageObj);
  }
  CPDF_Object* pContent =
      pPageDict ? pPageDict->GetElementValue("Contents") : NULL;
  if (pContent) {
    pPageDict->RemoveAt("Contents");
  }
  CPDF_Stream* pStream = new CPDF_Stream(NULL, 0, NULL);
  pStream->SetData(buf.GetBuffer(), buf.GetLength(), FALSE, FALSE);
  m_pDocument->AddIndirectObject(pStream);
  pPageDict->SetAtReference("Contents", m_pDocument, pStream->GetObjNum());
}
CFX_ByteString CPDF_PageContentGenerate::RealizeResource(
    CPDF_Object* pResourceObj,
    const FX_CHAR* szType) {
  if (!m_pPage->m_pResources) {
    m_pPage->m_pResources = new CPDF_Dictionary;
    int objnum = m_pDocument->AddIndirectObject(m_pPage->m_pResources);
    m_pPage->m_pFormDict->SetAtReference("Resources", m_pDocument, objnum);
  }
  CPDF_Dictionary* pResList = m_pPage->m_pResources->GetDict(szType);
  if (!pResList) {
    pResList = new CPDF_Dictionary;
    m_pPage->m_pResources->SetAt(szType, pResList);
  }
  m_pDocument->AddIndirectObject(pResourceObj);
  CFX_ByteString name;
  int idnum = 1;
  while (1) {
    name.Format("FX%c%d", szType[0], idnum);
    if (!pResList->KeyExist(name)) {
      break;
    }
    idnum++;
  }
  pResList->AddReference(name, m_pDocument, pResourceObj->GetObjNum());
  return name;
}
void CPDF_PageContentGenerate::ProcessImage(CFX_ByteTextBuf& buf,
                                            CPDF_ImageObject* pImageObj) {
  if ((pImageObj->m_Matrix.a == 0 && pImageObj->m_Matrix.b == 0) ||
      (pImageObj->m_Matrix.c == 0 && pImageObj->m_Matrix.d == 0)) {
    return;
  }
  buf << "q " << pImageObj->m_Matrix << " cm ";
  if (!pImageObj->m_pImage->IsInline()) {
    CPDF_Stream* pStream = pImageObj->m_pImage->GetStream();
    FX_DWORD dwSavedObjNum = pStream->GetObjNum();
    CFX_ByteString name = RealizeResource(pStream, "XObject");
    if (dwSavedObjNum == 0) {
      if (pImageObj->m_pImage)
        pImageObj->m_pImage->Release();
      pImageObj->m_pImage = m_pDocument->GetPageData()->GetImage(pStream);
    }
    buf << "/" << PDF_NameEncode(name) << " Do Q\n";
  }
}
void CPDF_PageContentGenerate::ProcessForm(CFX_ByteTextBuf& buf,
                                           const uint8_t* data,
                                           FX_DWORD size,
                                           CFX_Matrix& matrix) {
  if (!data || !size) {
    return;
  }
  CPDF_Stream* pStream = new CPDF_Stream(NULL, 0, NULL);
  CPDF_Dictionary* pFormDict = new CPDF_Dictionary;
  pFormDict->SetAtName("Type", "XObject");
  pFormDict->SetAtName("Subtype", "Form");
  CFX_FloatRect bbox = m_pPage->GetPageBBox();
  matrix.TransformRect(bbox);
  pFormDict->SetAtRect("BBox", bbox);
  pStream->InitStream((uint8_t*)data, size, pFormDict);
  buf << "q " << matrix << " cm ";
  CFX_ByteString name = RealizeResource(pStream, "XObject");
  buf << "/" << PDF_NameEncode(name) << " Do Q\n";
}
void CPDF_PageContentGenerate::TransformContent(CFX_Matrix& matrix) {
  CPDF_Dictionary* pDict = m_pPage->m_pFormDict;
  CPDF_Object* pContent = pDict ? pDict->GetElementValue("Contents") : NULL;
  if (!pContent)
    return;

  CFX_ByteTextBuf buf;
  if (CPDF_Array* pArray = pContent->AsArray()) {
    int iCount = pArray->GetCount();
    CPDF_StreamAcc** pContentArray = FX_Alloc(CPDF_StreamAcc*, iCount);
    int size = 0;
    int i = 0;
    for (i = 0; i < iCount; ++i) {
      pContent = pArray->GetElement(i);
      CPDF_Stream* pStream = ToStream(pContent);
      if (!pStream)
        continue;

      CPDF_StreamAcc* pStreamAcc = new CPDF_StreamAcc();
      pStreamAcc->LoadAllData(pStream);
      pContentArray[i] = pStreamAcc;
      size += pContentArray[i]->GetSize() + 1;
    }
    int pos = 0;
    uint8_t* pBuf = FX_Alloc(uint8_t, size);
    for (i = 0; i < iCount; ++i) {
      FXSYS_memcpy(pBuf + pos, pContentArray[i]->GetData(),
                   pContentArray[i]->GetSize());
      pos += pContentArray[i]->GetSize() + 1;
      pBuf[pos - 1] = ' ';
      delete pContentArray[i];
    }
    ProcessForm(buf, pBuf, size, matrix);
    FX_Free(pBuf);
    FX_Free(pContentArray);
  } else if (CPDF_Stream* pStream = pContent->AsStream()) {
    CPDF_StreamAcc contentStream;
    contentStream.LoadAllData(pStream);
    ProcessForm(buf, contentStream.GetData(), contentStream.GetSize(), matrix);
  }
  CPDF_Stream* pStream = new CPDF_Stream(NULL, 0, NULL);
  pStream->SetData(buf.GetBuffer(), buf.GetLength(), FALSE, FALSE);
  m_pDocument->AddIndirectObject(pStream);
  m_pPage->m_pFormDict->SetAtReference("Contents", m_pDocument,
                                       pStream->GetObjNum());
}
