// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/fuzzers/pdfium_fuzzer_helper.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>

#include "core/fxcrt/check_op.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/numerics/checked_math.h"
#include "core/fxcrt/span.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_dataavail.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_text.h"

namespace {

class FuzzerTestLoader {
 public:
  explicit FuzzerTestLoader(pdfium::span<const char> span) : span_(span) {}

  static int GetBlock(void* param,
                      unsigned long pos,
                      unsigned char* pBuf,
                      unsigned long size) {
    FuzzerTestLoader* pLoader = static_cast<FuzzerTestLoader*>(param);
    pdfium::CheckedNumeric<size_t> end = pos;
    end += size;
    CHECK_LE(end.ValueOrDie(), pLoader->span_.size());

    FXSYS_memcpy(pBuf, &pLoader->span_[pos], size);
    return 1;
  }

 private:
  const pdfium::span<const char> span_;
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

PDFiumFuzzerHelper::RenderingOptions GetRenderingOptionsFromData(
    pdfium::span<const char> span) {
  // Assume size_t may be 32 bits, so split `span` in 2 and hash them separately
  // to get enough bits.
  auto span_parts = span.split_at(span.size() / 2);
  std::string_view view1(span_parts.first.begin(), span_parts.first.end());
  std::string_view view2(span_parts.second.begin(), span_parts.second.end());
  const size_t data_hash1 = std::hash<std::string_view>()(view1);
  const size_t data_hash2 = std::hash<std::string_view>()(view2);

  // The largest flag value is 0x7FFF, so just take 15 bits from `data_hash1` at
  // a time.
  static constexpr int kFlagMask = 0x7fff;
  const int render_flags = data_hash1 & kFlagMask;
  const int form_flags = (data_hash1 >> 16) & kFlagMask;

  // Separately, use `data_hash2` for the bitmap format. The valid values are
  // [1, 5]. Avoid using the bytes at the start or end of `span`, as the header
  // and footer in PDFs usually have fixed data.
  const int bitmap_format = (data_hash2 % 5) + 1;

  return {
      .render_flags = render_flags,
      .form_flags = form_flags,
      .bitmap_format = bitmap_format,
  };
}

}  // namespace

PDFiumFuzzerHelper::PDFiumFuzzerHelper() = default;

PDFiumFuzzerHelper::~PDFiumFuzzerHelper() = default;

bool PDFiumFuzzerHelper::OnFormFillEnvLoaded(FPDF_DOCUMENT doc) {
  return true;
}

void PDFiumFuzzerHelper::RenderPdf(const char* data, size_t len) {
  // SAFETY: trusted arguments from fuzzer,
  auto span = UNSAFE_BUFFERS(pdfium::span(data, len));
  RenderingOptions options = GetRenderingOptionsFromData(span);

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

  FuzzerTestLoader loader(span);
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
      while (nRet == PDF_DATA_NOTAVAIL) {
        nRet = FPDFAvail_IsDocAvail(pdf_avail.get(), &hints);
      }

      if (nRet == PDF_DATA_ERROR) {
        return;
      }

      nRet = FPDFAvail_IsFormAvail(pdf_avail.get(), &hints);
      if (nRet == PDF_FORM_ERROR || nRet == PDF_FORM_NOTAVAIL) {
        return;
      }

      bIsLinearized = true;
    }
  } else {
    doc.reset(FPDF_LoadCustomDocument(&file_access, nullptr));
  }

  if (!doc) {
    return;
  }

  ScopedFPDFFormHandle form(
      FPDFDOC_InitFormFillEnvironment(doc.get(), &form_callbacks));
  if (!OnFormFillEnvLoaded(doc.get())) {
    return;
  }

  FPDF_SetFormFieldHighlightColor(form.get(), FPDF_FORMFIELD_UNKNOWN, 0xFFE4DD);
  FPDF_SetFormFieldHighlightAlpha(form.get(), 100);
  FORM_DoDocumentJSAction(form.get());
  FORM_DoDocumentOpenAction(form.get());

  int page_count = FPDF_GetPageCount(doc.get());
  for (int i = 0; i < page_count; ++i) {
    if (bIsLinearized) {
      nRet = PDF_DATA_NOTAVAIL;
      while (nRet == PDF_DATA_NOTAVAIL) {
        nRet = FPDFAvail_IsPageAvail(pdf_avail.get(), i, &hints);
      }

      if (nRet == PDF_DATA_ERROR) {
        return;
      }
    }
    RenderPage(doc.get(), form.get(), i, options);
  }
  OnRenderFinished(doc.get());
  FORM_DoDocumentAAction(form.get(), FPDFDOC_AACTION_WC);
}

bool PDFiumFuzzerHelper::RenderPage(FPDF_DOCUMENT doc,
                                    FPDF_FORMHANDLE form,
                                    int page_index,
                                    const RenderingOptions& options) {
  ScopedFPDFPage page(FPDF_LoadPage(doc, page_index));
  if (!page) {
    return false;
  }

  ScopedFPDFTextPage text_page(FPDFText_LoadPage(page.get()));
  FORM_OnAfterLoadPage(page.get(), form);
  FORM_DoPageAAction(page.get(), form, FPDFPAGE_AACTION_OPEN);

  FormActionHandler(form, doc, page.get());

  const double scale = 1.0;
  int width = static_cast<int>(FPDF_GetPageWidthF(page.get()) * scale);
  int height = static_cast<int>(FPDF_GetPageHeightF(page.get()) * scale);
  ScopedFPDFBitmap bitmap(FPDFBitmap_CreateEx(
      width, height, options.bitmap_format, nullptr, /*stride=*/0));
  if (bitmap) {
    if (!FPDFBitmap_FillRect(bitmap.get(), 0, 0, width, height, 0xFFFFFFFF)) {
      return false;
    }
    FPDF_RenderPageBitmap(bitmap.get(), page.get(), 0, 0, width, height, 0,
                          options.render_flags);
    FPDF_FFLDraw(form, bitmap.get(), page.get(), 0, 0, width, height, 0,
                 options.form_flags);
  }
  FORM_DoPageAAction(page.get(), form, FPDFPAGE_AACTION_CLOSE);
  FORM_OnBeforeClosePage(page.get(), form);
  return !!bitmap;
}
