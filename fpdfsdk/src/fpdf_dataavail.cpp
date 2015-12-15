// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_dataavail.h"

#include "fpdfsdk/include/fsdk_define.h"
#include "public/fpdf_formfill.h"

// These checks are here because core/ and public/ cannot depend on each other.
static_assert(IPDF_DataAvail::DataError == PDF_DATA_ERROR,
              "IPDF_DataAvail::DataError value mismatch");
static_assert(IPDF_DataAvail::DataNotAvailable == PDF_DATA_NOTAVAIL,
              "IPDF_DataAvail::DataNotAvailable value mismatch");
static_assert(IPDF_DataAvail::DataAvailable == PDF_DATA_AVAIL,
              "IPDF_DataAvail::DataAvailable value mismatch");

static_assert(IPDF_DataAvail::LinearizationUnknown == PDF_LINEARIZATION_UNKNOWN,
              "IPDF_DataAvail::LinearizationUnknown value mismatch");
static_assert(IPDF_DataAvail::NotLinearized == PDF_NOT_LINEARIZED,
              "IPDF_DataAvail::NotLinearized value mismatch");
static_assert(IPDF_DataAvail::Linearized == PDF_LINEARIZED,
              "IPDF_DataAvail::Linearized value mismatch");

static_assert(IPDF_DataAvail::FormError == PDF_FORM_ERROR,
              "IPDF_DataAvail::FormError value mismatch");
static_assert(IPDF_DataAvail::FormNotAvailable == PDF_FORM_NOTAVAIL,
              "IPDF_DataAvail::FormNotAvailable value mismatch");
static_assert(IPDF_DataAvail::FormAvailable == PDF_FORM_AVAIL,
              "IPDF_DataAvail::FormAvailable value mismatch");
static_assert(IPDF_DataAvail::FormNotExist == PDF_FORM_NOTEXIST,
              "IPDF_DataAvail::FormNotExist value mismatch");

class CFPDF_FileAvailWrap : public IFX_FileAvail {
 public:
  CFPDF_FileAvailWrap() { m_pfileAvail = NULL; }
  ~CFPDF_FileAvailWrap() override {}

  void Set(FX_FILEAVAIL* pfileAvail) { m_pfileAvail = pfileAvail; }

  // IFX_FileAvail
  FX_BOOL IsDataAvail(FX_FILESIZE offset, FX_DWORD size) override {
    return m_pfileAvail->IsDataAvail(m_pfileAvail, offset, size);
  }

 private:
  FX_FILEAVAIL* m_pfileAvail;
};

class CFPDF_FileAccessWrap : public IFX_FileRead {
 public:
  CFPDF_FileAccessWrap() { m_pFileAccess = NULL; }
  ~CFPDF_FileAccessWrap() override {}

  void Set(FPDF_FILEACCESS* pFile) { m_pFileAccess = pFile; }

  // IFX_FileRead
  FX_FILESIZE GetSize() override { return m_pFileAccess->m_FileLen; }

  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override {
    return m_pFileAccess->m_GetBlock(m_pFileAccess->m_Param, offset,
                                     (uint8_t*)buffer, size);
  }

  void Release() override {}

 private:
  FPDF_FILEACCESS* m_pFileAccess;
};

class CFPDF_DownloadHintsWrap : public IFX_DownloadHints {
 public:
  explicit CFPDF_DownloadHintsWrap(FX_DOWNLOADHINTS* pDownloadHints) {
    m_pDownloadHints = pDownloadHints;
  }
  ~CFPDF_DownloadHintsWrap() override {}

 public:
  // IFX_DownloadHints
  void AddSegment(FX_FILESIZE offset, FX_DWORD size) override {
    m_pDownloadHints->AddSegment(m_pDownloadHints, offset, size);
  }

 private:
  FX_DOWNLOADHINTS* m_pDownloadHints;
};

class CFPDF_DataAvail {
 public:
  CFPDF_DataAvail() { m_pDataAvail = NULL; }

  ~CFPDF_DataAvail() { delete m_pDataAvail; }

  IPDF_DataAvail* m_pDataAvail;
  CFPDF_FileAvailWrap m_FileAvail;
  CFPDF_FileAccessWrap m_FileRead;
};

DLLEXPORT FPDF_AVAIL STDCALL FPDFAvail_Create(FX_FILEAVAIL* file_avail,
                                              FPDF_FILEACCESS* file) {
  CFPDF_DataAvail* pAvail = new CFPDF_DataAvail;
  pAvail->m_FileAvail.Set(file_avail);
  pAvail->m_FileRead.Set(file);
  pAvail->m_pDataAvail =
      IPDF_DataAvail::Create(&pAvail->m_FileAvail, &pAvail->m_FileRead);
  return pAvail;
}

DLLEXPORT void STDCALL FPDFAvail_Destroy(FPDF_AVAIL avail) {
  delete (CFPDF_DataAvail*)avail;
}

DLLEXPORT int STDCALL
FPDFAvail_IsDocAvail(FPDF_AVAIL avail, FX_DOWNLOADHINTS* hints) {
  if (!avail || !hints)
    return PDF_DATA_ERROR;
  CFPDF_DownloadHintsWrap hints_wrap(hints);
  return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsDocAvail(&hints_wrap);
}

DLLEXPORT FPDF_DOCUMENT STDCALL
FPDFAvail_GetDocument(FPDF_AVAIL avail, FPDF_BYTESTRING password) {
  if (!avail)
    return NULL;
  CPDF_Parser* pParser = new CPDF_Parser;
  pParser->SetPassword(password);

  FX_DWORD err_code = pParser->StartAsynParse(
      ((CFPDF_DataAvail*)avail)->m_pDataAvail->GetFileRead());
  if (err_code) {
    delete pParser;
    ProcessParseError(err_code);
    return NULL;
  }
  ((CFPDF_DataAvail*)avail)->m_pDataAvail->SetDocument(pParser->GetDocument());
  CheckUnSupportError(pParser->GetDocument(), FPDF_ERR_SUCCESS);
  return FPDFDocumentFromCPDFDocument(pParser->GetDocument());
}

DLLEXPORT int STDCALL FPDFAvail_GetFirstPageNum(FPDF_DOCUMENT doc) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(doc);
  return pDoc ? pDoc->GetParser()->GetFirstPageNo() : 0;
}

DLLEXPORT int STDCALL FPDFAvail_IsPageAvail(FPDF_AVAIL avail,
                                            int page_index,
                                            FX_DOWNLOADHINTS* hints) {
  if (!avail || !hints)
    return PDF_DATA_ERROR;
  CFPDF_DownloadHintsWrap hints_wrap(hints);
  return ((CFPDF_DataAvail*)avail)
      ->m_pDataAvail->IsPageAvail(page_index, &hints_wrap);
}

DLLEXPORT int STDCALL FPDFAvail_IsFormAvail(FPDF_AVAIL avail,
                                            FX_DOWNLOADHINTS* hints) {
  if (!avail || !hints)
    return PDF_FORM_ERROR;
  CFPDF_DownloadHintsWrap hints_wrap(hints);
  return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsFormAvail(&hints_wrap);
}

DLLEXPORT int STDCALL FPDFAvail_IsLinearized(FPDF_AVAIL avail) {
  if (!avail)
    return PDF_LINEARIZATION_UNKNOWN;
  return ((CFPDF_DataAvail*)avail)->m_pDataAvail->IsLinearizedPDF();
}
