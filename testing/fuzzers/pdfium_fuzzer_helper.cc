// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/pdfium_fuzzer_helper.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>

#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_text.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"

namespace {

class FuzzerTestLoader {
 public:
  explicit FuzzerTestLoader(pdfium::span<const char> span) : m_Span(span) {}

  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size) {
    FuzzerTestLoader* pLoader = static_cast<FuzzerTestLoader*>(param);
    if (pos + size < pos || pos + size > pLoader->m_Span.size()) {
      NOTREACHED();
      return 0;
    }

    memcpy(pBuf, &pLoader->m_Span[pos], size);
    return 1;
  }

 private:
  const pdfium::span<const char> m_Span;
};

int ExampleAppAlert(IPDF_JSPLATFORM*,
                    FPDF_WIDESTRING,
                    FPDF_WIDESTRING,
                    int,
                    int) {
  return 0;
}

int ExampleAppResponse(IPDF_JSPLATFORM*,
                       FPDF_WIDESTRING question,
                       FPDF_WIDESTRING title,
                       FPDF_WIDESTRING default_value,
                       FPDF_WIDESTRING label,
                       FPDF_BOOL is_password,
                       void* response,
                       int length) {
  // UTF-16, always LE regardless of platform.
  uint8_t* ptr = static_cast<uint8_t*>(response);
  ptr[0] = 'N';
  ptr[1] = 0;
  ptr[2] = 'o';
  ptr[3] = 0;
  return 4;
}

void ExampleDocGotoPage(IPDF_JSPLATFORM*, int pageNumber) {}

void ExampleDocMail(IPDF_JSPLATFORM*,
                    void* mailData,
                    int length,
                    FPDF_BOOL UI,
                    FPDF_WIDESTRING To,
                    FPDF_WIDESTRING Subject,
                    FPDF_WIDESTRING CC,
                    FPDF_WIDESTRING BCC,
                    FPDF_WIDESTRING Msg) {}

FPDF_BOOL Is_Data_Avail(FX_FILEAVAIL* pThis, size_t offset, size_t size) {
  return true;
}

void Add_Segment(FX_DOWNLOADHINTS* pThis, size_t offset, size_t size) {}

std::pair<int, int> GetRenderingAndFormFlagFromData(const char* data,
                                                    size_t len) {
  std::string data_str = std::string(data, len);
  size_t data_hash = std::hash<std::string>()(data_str);

  // The largest flag value is 0x4FFF, so just take 16 bits from |data_hash| at
  // a time.
  int render_flags = data_hash & 0xffff;
  int form_flags = (data_hash >> 16) & 0xffff;
  return std::make_pair(render_flags, form_flags);
}

}  // namespace

PDFiumFuzzerHelper::PDFiumFuzzerHelper() = default;

PDFiumFuzzerHelper::~PDFiumFuzzerHelper() = default;

bool PDFiumFuzzerHelper::OnFormFillEnvLoaded(FPDF_DOCUMENT doc) {
  return true;
}

void PDFiumFuzzerHelper::RenderPdf(const char* data, size_t len) {
  int render_flags;
  int form_flags;
  std::tie(render_flags, form_flags) =
      GetRenderingAndFormFlagFromData(data, len);

  IPDF_JSPLATFORM platform_callbacks;
  memset(&platform_callbacks, '\0', sizeof(platform_callbacks));
  platform_callbacks.version = 3;
  platform_callbacks.app_alert = ExampleAppAlert;
  platform_callbacks.app_response = ExampleAppResponse;
  platform_callbacks.Doc_gotoPage = ExampleDocGotoPage;
  platform_callbacks.Doc_mail = ExampleDocMail;

  FPDF_FORMFILLINFO form_callbacks;
  memset(&form_callbacks, '\0', sizeof(form_callbacks));
  form_callbacks.version = GetFormCallbackVersion();
  form_callbacks.m_pJsPlatform = &platform_callbacks;

  FuzzerTestLoader loader({data, len});
  FPDF_FILEACCESS file_access;
  memset(&file_access, '\0', sizeof(file_access));
  file_access.m_FileLen = static_cast<unsigned long>(len);
  file_access.m_GetBlock = FuzzerTestLoader::GetBlock;
  file_access.m_Param = &loader;

  FX_FILEAVAIL file_avail;
  memset(&file_avail, '\0', sizeof(file_avail));
  file_avail.version = 1;
  file_avail.IsDataAvail = Is_Data_Avail;

  FX_DOWNLOADHINTS hints;
  memset(&hints, '\0', sizeof(hints));
  hints.version = 1;
  hints.AddSegment = Add_Segment;

  ScopedFPDFAvail pdf_avail(FPDFAvail_Create(&file_avail, &file_access));

  int nRet = PDF_DATA_NOTAVAIL;
  bool bIsLinearized = false;
  ScopedFPDFDocument doc;
  if (FPDFAvail_IsLinearized(pdf_avail.get()) == PDF_LINEARIZED) {
    doc.reset(FPDFAvail_GetDocument(pdf_avail.get(), nullptr));
    if (doc) {
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);

      if (nRet == PDF_DATA_ERROR)
        return;

      nRet = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
      if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL)
        return;

      bIsLinearized = true;
    }
  } else {
    doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
  }

  if (!doc)
    return;

  (void)FPDF_GetDocPermissions(doc.get());

  ScopedFPDFFormHandle form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  if (!OnFormFillEnvLoaded(doc.get()))
    return;

  FPDF_SetFormFieldHighlightColor(form.get(), FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form.get(), 100);
  FORM_DoDocumentJSAction(form.get());
  FORM_DoDocumentOpenAction(form.get());

  int page_count = FPDF_GetPageCount(doc.get());
  for (int i = 0; i < page_count; ++i) {
    if (bIsLinearized) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL)
        nRet = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);

      if (nRet == PDF_DATA_ERROR)
        return;
    }
    RenderPage(doc.get(), form.get(), i, render_flags, form_flags);
  }
  OnRenderFinished(doc.get());
  FORM_DoDocumentAAction(form.get(), FPDFDOC_AACTION_WC);
}

bool PDFiumFuzzerHelper::RenderPage(FPDF_DOCUMENT doc,
                                    FPDF_FORMHANDLE form,
                                    int page_index,
                                    int render_flags,
                                    int form_flags) {
  ScopedFPDFPage page(FPDF_LoadPage(doc, page_index));
  if (!page)
    return false;

  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
  FORM_OnAfterLoadPage(page.get(), form);
  FORM_DoPageAAction(page.get(), form, FPDFPAGE_AACTION_OPEN);

  const double scale = 1.0;
  int width = static_cast<int>(FPDF_GetPageWidthF(page.get()) * scale);
  int height = static_cast<int>(FPDF_GetPageHeightF(page.get()) * scale);
  ScopedFPDFBitmap bitmap(FPDFBitmap_Create(width, height, 0));
  if (bitmap) {
    FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, 0xFFFFFFFF);
    FPDF_RenderPageBitmap(bitmap.get(), page.get(), 0, 0, width, height, 0,
                          render_flags);
    FPDF_FFLDraw(form, bitmap.get(), page.get(), 0, 0, width, height, 0,
                 form_flags);
  }
  FORM_DoPageAAction(page.get(), form, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page.get(), form);
  return !!bitmap;
}
