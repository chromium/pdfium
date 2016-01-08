// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_save.h"

#include "core/include/fpdfapi/fpdf_serial.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "public/fpdf_edit.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_app.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_doc.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"
#include "public/fpdf_formfill.h"
#endif

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

class CFX_IFileWrite final : public IFX_StreamWrite {
 public:
  CFX_IFileWrite();
  FX_BOOL Init(FPDF_FILEWRITE* pFileWriteStruct);
  FX_BOOL WriteBlock(const void* pData, size_t size) override;
  void Release() override;

 protected:
  ~CFX_IFileWrite() override {}

  FPDF_FILEWRITE* m_pFileWriteStruct;
};

CFX_IFileWrite::CFX_IFileWrite() {
  m_pFileWriteStruct = NULL;
}

FX_BOOL CFX_IFileWrite::Init(FPDF_FILEWRITE* pFileWriteStruct) {
  if (!pFileWriteStruct)
    return FALSE;

  m_pFileWriteStruct = pFileWriteStruct;
  return TRUE;
}

FX_BOOL CFX_IFileWrite::WriteBlock(const void* pData, size_t size) {
  if (!m_pFileWriteStruct)
    return FALSE;

  m_pFileWriteStruct->WriteBlock(m_pFileWriteStruct, pData, size);
  return TRUE;
}

void CFX_IFileWrite::Release() {
  delete this;
}

#ifdef PDF_ENABLE_XFA
#define XFA_DATASETS 0
#define XFA_FORMS 1

FX_BOOL _SaveXFADocumentData(CPDFXFA_Document* pDocument,
                             CFX_PtrArray& fileList) {
  if (!pDocument)
    return FALSE;
  if (pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return TRUE;
  if (!CPDFXFA_App::GetInstance()->GetXFAApp())
    return TRUE;

  IXFA_DocView* pXFADocView = pDocument->GetXFADocView();
  if (NULL == pXFADocView)
    return TRUE;

  IXFA_DocHandler* pXFADocHandler =
      CPDFXFA_App::GetInstance()->GetXFAApp()->GetDocHandler();
  CPDF_Document* pPDFDocument = pDocument->GetPDFDoc();
  if (pDocument == NULL)
    return FALSE;

  CPDF_Dictionary* pRoot = pPDFDocument->GetRoot();
  if (pRoot == NULL)
    return FALSE;
  CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
  if (NULL == pAcroForm)
    return FALSE;
  CPDF_Object* pXFA = pAcroForm->GetElement("XFA");
  if (pXFA == NULL)
    return TRUE;
  if (pXFA->GetType() != PDFOBJ_ARRAY)
    return FALSE;
  CPDF_Array* pArray = pXFA->GetArray();
  if (NULL == pArray)
    return FALSE;
  int size = pArray->GetCount();
  int iFormIndex = -1;
  int iDataSetsIndex = -1;
  int iTemplate = -1;
  int iLast = size - 2;
  for (int i = 0; i < size - 1; i++) {
    CPDF_Object* pPDFObj = pArray->GetElement(i);
    if (pPDFObj->GetType() != PDFOBJ_STRING)
      continue;
    if (pPDFObj->GetString() == "form")
      iFormIndex = i + 1;
    else if (pPDFObj->GetString() == "datasets")
      iDataSetsIndex = i + 1;
    else if (pPDFObj->GetString() == "template")
      iTemplate = i + 1;
  }
  IXFA_ChecksumContext* pContext = NULL;
  // Checksum
  pContext = XFA_Checksum_Create();
  FXSYS_assert(pContext);
  pContext->StartChecksum();

  // template
  if (iTemplate > -1) {
    CPDF_Stream* pTemplateStream = pArray->GetStream(iTemplate);
    CPDF_StreamAcc streamAcc;
    streamAcc.LoadAllData(pTemplateStream);
    uint8_t* pData = (uint8_t*)streamAcc.GetData();
    FX_DWORD dwSize2 = streamAcc.GetSize();
    IFX_FileStream* pTemplate = FX_CreateMemoryStream(pData, dwSize2);
    pContext->UpdateChecksum((IFX_FileRead*)pTemplate);
    pTemplate->Release();
  }
  CPDF_Stream* pFormStream = NULL;
  CPDF_Stream* pDataSetsStream = NULL;
  if (iFormIndex != -1) {
    // Get form CPDF_Stream
    CPDF_Object* pFormPDFObj = pArray->GetElement(iFormIndex);
    if (pFormPDFObj->GetType() == PDFOBJ_REFERENCE) {
      CPDF_Object* pFormDircetObj = pFormPDFObj->GetDirect();
      if (NULL != pFormDircetObj &&
          pFormDircetObj->GetType() == PDFOBJ_STREAM) {
        pFormStream = (CPDF_Stream*)pFormDircetObj;
      }
    } else if (pFormPDFObj->GetType() == PDFOBJ_STREAM) {
      pFormStream = (CPDF_Stream*)pFormPDFObj;
    }
  }

  if (iDataSetsIndex != -1) {
    // Get datasets CPDF_Stream
    CPDF_Object* pDataSetsPDFObj = pArray->GetElement(iDataSetsIndex);
    if (pDataSetsPDFObj->GetType() == PDFOBJ_REFERENCE) {
      CPDF_Reference* pDataSetsRefObj = (CPDF_Reference*)pDataSetsPDFObj;
      CPDF_Object* pDataSetsDircetObj = pDataSetsRefObj->GetDirect();
      if (NULL != pDataSetsDircetObj &&
          pDataSetsDircetObj->GetType() == PDFOBJ_STREAM) {
        pDataSetsStream = (CPDF_Stream*)pDataSetsDircetObj;
      }
    } else if (pDataSetsPDFObj->GetType() == PDFOBJ_STREAM) {
      pDataSetsStream = (CPDF_Stream*)pDataSetsPDFObj;
    }
  }
  // end
  // L"datasets"
  {
    IFX_FileStream* pDsfileWrite = FX_CreateMemoryStream();
    if (NULL == pDsfileWrite) {
      pContext->Release();
      pDsfileWrite->Release();
      return FALSE;
    }
    if (pXFADocHandler->SavePackage(pXFADocView->GetDoc(),
                                    CFX_WideStringC(L"datasets"),
                                    pDsfileWrite) &&
        pDsfileWrite->GetSize() > 0) {
      // Datasets
      pContext->UpdateChecksum((IFX_FileRead*)pDsfileWrite);
      pContext->FinishChecksum();
      CPDF_Dictionary* pDataDict = new CPDF_Dictionary;
      if (iDataSetsIndex != -1) {
        if (pDataSetsStream)
          pDataSetsStream->InitStreamFromFile(pDsfileWrite, pDataDict);
      } else {
        CPDF_Stream* pData = new CPDF_Stream(NULL, 0, NULL);
        pData->InitStreamFromFile(pDsfileWrite, pDataDict);
        pPDFDocument->AddIndirectObject(pData);
        iLast = pArray->GetCount() - 2;
        pArray->InsertAt(iLast, new CPDF_String("datasets", FALSE));
        pArray->InsertAt(iLast + 1, pData, pPDFDocument);
      }
      fileList.Add(pDsfileWrite);
    }
  }

  // L"form"
  {
    IFX_FileStream* pfileWrite = FX_CreateMemoryStream();
    if (NULL == pfileWrite) {
      pContext->Release();
      return FALSE;
    }
    if (pXFADocHandler->SavePackage(pXFADocView->GetDoc(),
                                    CFX_WideStringC(L"form"), pfileWrite,
                                    pContext) &&
        pfileWrite > 0) {
      CPDF_Dictionary* pDataDict = new CPDF_Dictionary;
      if (iFormIndex != -1) {
        if (pFormStream)
          pFormStream->InitStreamFromFile(pfileWrite, pDataDict);
      } else {
        CPDF_Stream* pData = new CPDF_Stream(NULL, 0, NULL);
        pData->InitStreamFromFile(pfileWrite, pDataDict);
        pPDFDocument->AddIndirectObject(pData);
        iLast = pArray->GetCount() - 2;
        pArray->InsertAt(iLast, new CPDF_String("form", FALSE));
        pArray->InsertAt(iLast + 1, pData, pPDFDocument);
      }
      fileList.Add(pfileWrite);
    }
  }
  pContext->Release();
  return TRUE;
}

FX_BOOL _SendPostSaveToXFADoc(CPDFXFA_Document* pDocument) {
  if (!pDocument)
    return FALSE;

  if (pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return TRUE;

  IXFA_DocView* pXFADocView = pDocument->GetXFADocView();
  if (NULL == pXFADocView)
    return FALSE;
  IXFA_WidgetHandler* pWidgetHander = pXFADocView->GetWidgetHandler();

  CXFA_WidgetAcc* pWidgetAcc = NULL;
  IXFA_WidgetAccIterator* pWidgetAccIterator =
      pXFADocView->CreateWidgetAccIterator();
  pWidgetAcc = pWidgetAccIterator->MoveToNext();
  while (pWidgetAcc) {
    CXFA_EventParam preParam;
    preParam.m_eType = XFA_EVENT_PostSave;
    pWidgetHander->ProcessEvent(pWidgetAcc, &preParam);
    pWidgetAcc = pWidgetAccIterator->MoveToNext();
  }
  pWidgetAccIterator->Release();
  pXFADocView->UpdateDocView();
  pDocument->_ClearChangeMark();
  return TRUE;
}

FX_BOOL _SendPreSaveToXFADoc(CPDFXFA_Document* pDocument,
                             CFX_PtrArray& fileList) {
  if (pDocument->GetDocType() != DOCTYPE_DYNAMIC_XFA &&
      pDocument->GetDocType() != DOCTYPE_STATIC_XFA)
    return TRUE;
  IXFA_DocView* pXFADocView = pDocument->GetXFADocView();
  if (NULL == pXFADocView)
    return TRUE;
  IXFA_WidgetHandler* pWidgetHander = pXFADocView->GetWidgetHandler();
  CXFA_WidgetAcc* pWidgetAcc = NULL;
  IXFA_WidgetAccIterator* pWidgetAccIterator =
      pXFADocView->CreateWidgetAccIterator();
  pWidgetAcc = pWidgetAccIterator->MoveToNext();
  while (pWidgetAcc) {
    CXFA_EventParam preParam;
    preParam.m_eType = XFA_EVENT_PreSave;
    pWidgetHander->ProcessEvent(pWidgetAcc, &preParam);
    pWidgetAcc = pWidgetAccIterator->MoveToNext();
  }
  pWidgetAccIterator->Release();
  pXFADocView->UpdateDocView();
  return _SaveXFADocumentData(pDocument, fileList);
}
#endif  // PDF_ENABLE_XFA

FPDF_BOOL _FPDF_Doc_Save(FPDF_DOCUMENT document,
                         FPDF_FILEWRITE* pFileWrite,
                         FPDF_DWORD flags,
                         FPDF_BOOL bSetVersion,
                         int fileVerion) {
  CPDF_Document* pPDFDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pPDFDoc)
    return 0;

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Document* pDoc = (CPDFXFA_Document*)document;
  CFX_PtrArray fileList;
  _SendPreSaveToXFADoc(pDoc, fileList);
#endif  // PDF_ENABLE_XFA

  if (flags < FPDF_INCREMENTAL || flags > FPDF_REMOVE_SECURITY) {
    flags = 0;
  }

  CPDF_Creator FileMaker(pPDFDoc);
  if (bSetVersion)
    FileMaker.SetFileVersion(fileVerion);
  if (flags == FPDF_REMOVE_SECURITY) {
    flags = 0;
    FileMaker.RemoveSecurity();
  }

  CFX_IFileWrite* pStreamWrite = NULL;
  FX_BOOL bRet;
  pStreamWrite = new CFX_IFileWrite;
  pStreamWrite->Init(pFileWrite);
  bRet = FileMaker.Create(pStreamWrite, flags);
#ifdef PDF_ENABLE_XFA
  _SendPostSaveToXFADoc(pDoc);
  for (int i = 0; i < fileList.GetSize(); i++) {
    IFX_FileStream* pFile = (IFX_FileStream*)fileList.GetAt(i);
    pFile->Release();
  }
  fileList.RemoveAll();
#endif  // PDF_ENABLE_XFA
  pStreamWrite->Release();
  return bRet;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_SaveAsCopy(FPDF_DOCUMENT document,
                                            FPDF_FILEWRITE* pFileWrite,
                                            FPDF_DWORD flags) {
  return _FPDF_Doc_Save(document, pFileWrite, flags, FALSE, 0);
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_SaveWithVersion(FPDF_DOCUMENT document,
                                                 FPDF_FILEWRITE* pFileWrite,
                                                 FPDF_DWORD flags,
                                                 int fileVersion) {
  return _FPDF_Doc_Save(document, pFileWrite, flags, TRUE, fileVersion);
}
