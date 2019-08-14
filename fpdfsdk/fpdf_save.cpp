// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_save.h"

#include <memory>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "core/fpdfapi/edit/cpdf_creator.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/fx_extension.h"
#include "fpdfsdk/cpdfsdk_filewriteadapter.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/fpdf_edit.h"
#include "third_party/base/optional.h"

#ifdef PDF_ENABLE_XFA
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "public/fpdf_formfill.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#endif

#if defined(OS_ANDROID)
#include <time.h>
#else
#include <ctime>
#endif

namespace {

#ifdef PDF_ENABLE_XFA
bool SaveXFADocumentData(CPDFXFA_Context* pContext,
                         std::vector<RetainPtr<IFX_SeekableStream>>* fileList) {
  if (!pContext)
    return false;

  if (!pContext->ContainsExtensionForm())
    return true;

  CXFA_FFDocView* pXFADocView = pContext->GetXFADocView();
  if (!pXFADocView)
    return true;

  CPDF_Document* pPDFDocument = pContext->GetPDFDoc();
  if (!pPDFDocument)
    return false;

  CPDF_Dictionary* pRoot = pPDFDocument->GetRoot();
  if (!pRoot)
    return false;

  CPDF_Dictionary* pAcroForm = pRoot->GetDictFor("AcroForm");
  if (!pAcroForm)
    return false;

  CPDF_Object* pXFA = pAcroForm->GetObjectFor("XFA");
  if (!pXFA)
    return true;

  CPDF_Array* pArray = pXFA->AsArray();
  if (!pArray)
    return false;

  int size = pArray->size();
  int iFormIndex = -1;
  int iDataSetsIndex = -1;
  for (int i = 0; i < size - 1; i++) {
    const CPDF_Object* pPDFObj = pArray->GetObjectAt(i);
    if (!pPDFObj->IsString())
      continue;
    if (pPDFObj->GetString() == "form")
      iFormIndex = i + 1;
    else if (pPDFObj->GetString() == "datasets")
      iDataSetsIndex = i + 1;
  }

  CPDF_Stream* pFormStream = nullptr;
  if (iFormIndex != -1) {
    // Get form CPDF_Stream
    CPDF_Object* pFormPDFObj = pArray->GetObjectAt(iFormIndex);
    if (pFormPDFObj->IsReference()) {
      CPDF_Object* pFormDirectObj = pFormPDFObj->GetDirect();
      if (pFormDirectObj && pFormDirectObj->IsStream()) {
        pFormStream = pFormDirectObj->AsStream();
      }
    } else if (pFormPDFObj->IsStream()) {
      pFormStream = pFormPDFObj->AsStream();
    }
  }

  CPDF_Stream* pDataSetsStream = nullptr;
  if (iDataSetsIndex != -1) {
    // Get datasets CPDF_Stream
    CPDF_Object* pDataSetsPDFObj = pArray->GetObjectAt(iDataSetsIndex);
    if (pDataSetsPDFObj->IsReference()) {
      CPDF_Reference* pDataSetsRefObj = pDataSetsPDFObj->AsReference();
      CPDF_Object* pDataSetsDirectObj = pDataSetsRefObj->GetDirect();
      if (pDataSetsDirectObj && pDataSetsDirectObj->IsStream()) {
        pDataSetsStream = pDataSetsDirectObj->AsStream();
      }
    } else if (pDataSetsPDFObj->IsStream()) {
      pDataSetsStream = pDataSetsPDFObj->AsStream();
    }
  }
  // L"datasets"
  {
    RetainPtr<IFX_SeekableStream> pDsfileWrite =
        pdfium::MakeRetain<CFX_MemoryStream>();
    CXFA_FFDoc* ffdoc = pXFADocView->GetDoc();
    if (ffdoc->SavePackage(
            ToNode(ffdoc->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Datasets)),
            pDsfileWrite) &&
        pDsfileWrite->GetSize() > 0) {
      auto pDataDict = pPDFDocument->New<CPDF_Dictionary>();
      if (iDataSetsIndex != -1) {
        if (pDataSetsStream) {
          pDataSetsStream->InitStreamFromFile(pDsfileWrite,
                                              std::move(pDataDict));
        }
      } else {
        CPDF_Stream* pData = pPDFDocument->NewIndirect<CPDF_Stream>();
        pData->InitStreamFromFile(pDsfileWrite, std::move(pDataDict));
        int iLast = pArray->size() - 2;
        pArray->InsertNewAt<CPDF_String>(iLast, "datasets", false);
        pArray->InsertNewAt<CPDF_Reference>(iLast + 1, pPDFDocument,
                                            pData->GetObjNum());
      }
      fileList->push_back(std::move(pDsfileWrite));
    }
  }
  // L"form"
  {
    RetainPtr<IFX_SeekableStream> pfileWrite =
        pdfium::MakeRetain<CFX_MemoryStream>();

    CXFA_FFDoc* ffdoc = pXFADocView->GetDoc();
    if (ffdoc->SavePackage(
            ToNode(ffdoc->GetXFADoc()->GetXFAObject(XFA_HASHCODE_Form)),
            pfileWrite) &&
        pfileWrite->GetSize() > 0) {
      auto pDataDict = pPDFDocument->New<CPDF_Dictionary>();
      if (iFormIndex != -1) {
        if (pFormStream)
          pFormStream->InitStreamFromFile(pfileWrite, std::move(pDataDict));
      } else {
        CPDF_Stream* pData = pPDFDocument->NewIndirect<CPDF_Stream>();
        pData->InitStreamFromFile(pfileWrite, std::move(pDataDict));
        int iLast = pArray->size() - 2;
        pArray->InsertNewAt<CPDF_String>(iLast, "form", false);
        pArray->InsertNewAt<CPDF_Reference>(iLast + 1, pPDFDocument,
                                            pData->GetObjNum());
      }
      fileList->push_back(std::move(pfileWrite));
    }
  }
  return true;
}
#endif  // PDF_ENABLE_XFA

bool DoDocSave(FPDF_DOCUMENT document,
               FPDF_FILEWRITE* pFileWrite,
               FPDF_DWORD flags,
               Optional<int> version) {
  CPDF_Document* pPDFDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pPDFDoc)
    return 0;

#ifdef PDF_ENABLE_XFA
  auto* pContext = static_cast<CPDFXFA_Context*>(pPDFDoc->GetExtension());
  if (pContext) {
    std::vector<RetainPtr<IFX_SeekableStream>> fileList;
    pContext->SendPreSaveToXFADoc(&fileList);
    SaveXFADocumentData(pContext, &fileList);
  }
#endif  // PDF_ENABLE_XFA

  if (flags < FPDF_INCREMENTAL || flags > FPDF_REMOVE_SECURITY)
    flags = 0;

  CPDF_Creator fileMaker(
      pPDFDoc, pdfium::MakeRetain<CPDFSDK_FileWriteAdapter>(pFileWrite));
  if (version.has_value())
    fileMaker.SetFileVersion(version.value());
  if (flags == FPDF_REMOVE_SECURITY) {
    flags = 0;
    fileMaker.RemoveSecurity();
  }

  bool bRet = fileMaker.Create(flags);

#ifdef PDF_ENABLE_XFA
  if (pContext)
    pContext->SendPostSaveToXFADoc();
#endif  // PDF_ENABLE_XFA

  return bRet;
}

}  // namespace

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_SaveAsCopy(FPDF_DOCUMENT document,
                                                    FPDF_FILEWRITE* pFileWrite,
                                                    FPDF_DWORD flags) {
  return DoDocSave(document, pFileWrite, flags, {});
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_SaveWithVersion(FPDF_DOCUMENT document,
                     FPDF_FILEWRITE* pFileWrite,
                     FPDF_DWORD flags,
                     int fileVersion) {
  return DoDocSave(document, pFileWrite, flags, fileVersion);
}
