// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_HELPERS_H_
#define FPDFSDK_CPDFSDK_HELPERS_H_

#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "public/fpdf_doc.h"
#include "public/fpdfview.h"

#ifdef PDF_ENABLE_XFA
#include "core/fxcrt/fx_stream.h"
#endif  // PDF_ENABLE_XFA

#ifdef _WIN32
#include <math.h>
#include <tchar.h>
#endif

class CPDF_Annot;
class CPDF_Page;
class CPDF_PageObject;
class CPDF_PageRenderContext;
class CPDF_PathObject;
class CPDF_Stream;
class IPDFSDK_PauseAdapter;
class FX_PATHPOINT;

#ifdef PDF_ENABLE_XFA
class CPDFXFA_Context;
class CPDFXFA_Page;
#endif  // PDF_ENABLE_XFA

// Object types for public FPDF_ types; these correspond to next layer down
// from fpdfsdk. For master, these are CPDF_ types, but for XFA, these are
// CPDFXFA_ types.
#ifndef PDF_ENABLE_XFA
using UnderlyingPageType = CPDF_Page;
#else   // PDF_ENABLE_XFA
using UnderlyingPageType = CPDFXFA_Page;
#endif  // PDF_ENABLE_XFA

// Conversions to/from underlying types.
UnderlyingPageType* UnderlyingFromFPDFPage(FPDF_PAGE page);
FPDF_PAGE FPDFPageFromUnderlying(UnderlyingPageType* page);

// Conversions to/from FPDF_ types.
CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc);
FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc);

CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page);
CPDF_PageObject* CPDFPageObjectFromFPDFPageObject(FPDF_PAGEOBJECT page_object);
ByteString CFXByteStringFromFPDFWideString(FPDF_WIDESTRING wide_string);
CFX_DIBitmap* CFXBitmapFromFPDFBitmap(FPDF_BITMAP bitmap);

#ifdef PDF_ENABLE_XFA
// Layering prevents fxcrt from knowing about FPDF_FILEHANDLER, so this can't
// be a static method of IFX_SeekableStream.
RetainPtr<IFX_SeekableStream> MakeSeekableStream(
    FPDF_FILEHANDLER* pFileHandler);
#endif  // PDF_ENABLE_XFA

CPDF_Array* GetQuadPointsArrayFromDictionary(const CPDF_Dictionary* dict);
CPDF_Array* AddQuadPointsArrayToDictionary(CPDF_Dictionary* dict);
bool IsValidQuadPointsIndex(const CPDF_Array* array, size_t index);
bool GetQuadPointsAtIndex(const CPDF_Array* array,
                          size_t quad_index,
                          FS_QUADPOINTSF* quad_points);

CFX_FloatRect CFXFloatRectFromFSRECTF(const FS_RECTF& rect);
void FSRECTFFromCFXFloatRect(const CFX_FloatRect& rect, FS_RECTF* out_rect);

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const WideString& text,
                                                  void* buffer,
                                                  unsigned long buflen);
unsigned long DecodeStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen);

void FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
FPDF_BOOL FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy);

// TODO(dsinclair): Where should this live?
void FPDF_RenderPage_Retail(CPDF_PageRenderContext* pContext,
                            FPDF_PAGE page,
                            int start_x,
                            int start_y,
                            int size_x,
                            int size_y,
                            int rotate,
                            int flags,
                            bool bNeedToRestore,
                            IPDFSDK_PauseAdapter* pause);

void CheckUnSupportError(CPDF_Document* pDoc, uint32_t err_code);
void CheckUnSupportAnnot(CPDF_Document* pDoc, const CPDF_Annot* pPDFAnnot);

#ifndef _WIN32
void SetLastError(int err);
int GetLastError();
#endif  // _WIN32

void ProcessParseError(CPDF_Parser::Error err);

// TODO(dsinclair): This seems like it should be a public API?
FPDF_BOOL FPDFPageObj_SetFillColor(FPDF_PAGEOBJECT page_object,
                                   unsigned int R,
                                   unsigned int G,
                                   unsigned int B,
                                   unsigned int A);

#endif  // FPDFSDK_CPDFSDK_HELPERS_H_
