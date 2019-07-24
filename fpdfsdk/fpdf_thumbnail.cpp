// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/fpdf_thumbnail.h"

#include <vector>

#include "core/fpdfapi/page/cpdf_dibbase.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "public/fpdfview.h"

namespace {

const CPDF_Stream* CPDFStreamForThumbnailFromPage(FPDF_PAGE page) {
  const CPDF_Page* p_page = CPDFPageFromFPDFPage(page);
  if (!p_page)
    return nullptr;

  const CPDF_Dictionary* page_dict = p_page->GetDict();
  if (!page_dict || !page_dict->KeyExist("Type"))
    return nullptr;

  return page_dict->GetStreamFor("Thumb");
}

}  // namespace

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFPage_GetDecodedThumbnailData(FPDF_PAGE page,
                                 void* buffer,
                                 unsigned long buflen) {
  const CPDF_Stream* thumb_stream = CPDFStreamForThumbnailFromPage(page);
  if (!thumb_stream)
    return 0u;

  return DecodeStreamMaybeCopyAndReturnLength(thumb_stream, buffer, buflen);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FPDFPage_GetRawThumbnailData(FPDF_PAGE page,
                             void* buffer,
                             unsigned long buflen) {
  const CPDF_Stream* thumb_stream = CPDFStreamForThumbnailFromPage(page);
  if (!thumb_stream)
    return 0u;

  return GetRawStreamMaybeCopyAndReturnLength(thumb_stream, buffer, buflen);
}

FPDF_EXPORT FPDF_BITMAP FPDF_CALLCONV
FPDFPage_GetThumbnailAsBitmap(FPDF_PAGE page) {
  const CPDF_Stream* thumb_stream = CPDFStreamForThumbnailFromPage(page);
  if (!thumb_stream)
    return nullptr;

  const CPDF_Page* p_page = CPDFPageFromFPDFPage(page);

  auto p_source = pdfium::MakeRetain<CPDF_DIBBase>();
  const CPDF_DIBBase::LoadState start_status = p_source->StartLoadDIBBase(
      p_page->GetDocument(), thumb_stream, false, nullptr,
      p_page->m_pPageResources.Get(), false, 0, false);
  if (start_status == CPDF_DIBBase::LoadState::kFail)
    return nullptr;

  auto thumb_bitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!thumb_bitmap->Copy(p_source))
    return nullptr;

  return FPDFBitmapFromCFXDIBitmap(thumb_bitmap.Leak());
}
