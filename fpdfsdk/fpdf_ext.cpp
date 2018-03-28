// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ext.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "fpdfsdk/cpdfsdk_helpers.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#endif  // PDF_ENABLE_XFA

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FSDK_SetUnSpObjProcessHandler(UNSUPPORT_INFO* unsp_info) {
  if (!unsp_info || unsp_info->version != 1)
    return false;

  CPDF_ModuleMgr::Get()->SetUnsupportInfoAdapter(
      pdfium::MakeUnique<CFSDK_UnsupportInfo_Adapter>(unsp_info));
  return true;
}

FPDF_EXPORT int FPDF_CALLCONV FPDFDoc_GetPageMode(FPDF_DOCUMENT document) {
  CPDF_Document* pDoc = CPDFDocumentFromFPDFDocument(document);
  if (!pDoc)
    return PAGEMODE_UNKNOWN;

  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return PAGEMODE_UNKNOWN;

  CPDF_Object* pName = pRoot->GetObjectFor("PageMode");
  if (!pName)
    return PAGEMODE_USENONE;

  ByteString strPageMode = pName->GetString();
  if (strPageMode.IsEmpty() || strPageMode.EqualNoCase("UseNone"))
    return PAGEMODE_USENONE;
  if (strPageMode.EqualNoCase("UseOutlines"))
    return PAGEMODE_USEOUTLINES;
  if (strPageMode.EqualNoCase("UseThumbs"))
    return PAGEMODE_USETHUMBS;
  if (strPageMode.EqualNoCase("FullScreen"))
    return PAGEMODE_FULLSCREEN;
  if (strPageMode.EqualNoCase("UseOC"))
    return PAGEMODE_USEOC;
  if (strPageMode.EqualNoCase("UseAttachments"))
    return PAGEMODE_USEATTACHMENTS;

  return PAGEMODE_UNKNOWN;
}
