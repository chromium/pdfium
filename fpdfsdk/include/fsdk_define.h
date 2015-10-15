// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_DEFINE_H_
#define FPDFSDK_INCLUDE_FSDK_DEFINE_H_

#include "../../core/include/fpdfapi/fpdf_module.h"
#include "../../core/include/fpdfapi/fpdf_pageobj.h"
#include "../../core/include/fpdfapi/fpdf_parser.h"
#include "../../core/include/fpdfapi/fpdf_render.h"
#include "../../core/include/fpdfapi/fpdf_serial.h"
#include "../../core/include/fpdfapi/fpdfapi.h"
#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fpdfdoc/fpdf_vt.h"
#include "../../core/include/fxge/fx_ge.h"
#include "../../core/include/fxge/fx_ge_win32.h"
#include "../../public/fpdfview.h"

#ifdef _WIN32
#include <tchar.h>
#include <math.h>
#endif

#ifndef FX_ARGBTOCOLORREF
/** @brief Convert a #FX_ARGB to a #FX_COLORREF. */
#define FX_ARGBTOCOLORREF(argb)                                            \
  ((((FX_DWORD)argb & 0x00FF0000) >> 16) | ((FX_DWORD)argb & 0x0000FF00) | \
   (((FX_DWORD)argb & 0x000000FF) << 16))
#endif

#ifndef FX_COLORREFTOARGB
/** @brief Convert a #FX_COLORREF to a #FX_ARGB. */
#define FX_COLORREFTOARGB(rgb)                                   \
  ((FX_DWORD)0xFF000000 | (((FX_DWORD)rgb & 0x000000FF) << 16) | \
   ((FX_DWORD)rgb & 0x0000FF00) | (((FX_DWORD)rgb & 0x00FF0000) >> 16))
#endif

typedef unsigned int FX_UINT;
class CRenderContext;
class IFSDK_PAUSE_Adapter;

class CPDF_CustomAccess final : public IFX_FileRead {
 public:
  CPDF_CustomAccess(FPDF_FILEACCESS* pFileAccess);
  ~CPDF_CustomAccess() override {}

  // IFX_FileRead
  FX_FILESIZE GetSize() override { return m_FileAccess.m_FileLen; }
  void Release() override { delete this; }
  FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

 private:
  FPDF_FILEACCESS m_FileAccess;
};

// Conversions from FPDF_ types.
CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc);
CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page);

void DropContext(void* data);
void FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
FPDF_BOOL FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy);
void FPDF_RenderPage_Retail(CRenderContext* pContext,
                            FPDF_PAGE page,
                            int start_x,
                            int start_y,
                            int size_x,
                            int size_y,
                            int rotate,
                            int flags,
                            FX_BOOL bNeedToRestore,
                            IFSDK_PAUSE_Adapter* pause);

void CheckUnSupportError(CPDF_Document* pDoc, FX_DWORD err_code);
void CheckUnSupportAnnot(CPDF_Document* pDoc, CPDF_Annot* pPDFAnnot);
void ProcessParseError(FX_DWORD err_code);

#endif  // FPDFSDK_INCLUDE_FSDK_DEFINE_H_
