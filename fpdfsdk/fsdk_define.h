// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FSDK_DEFINE_H_
#define FPDFSDK_FSDK_DEFINE_H_

#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "public/fpdfview.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
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
class IFSDK_PAUSE_Adapter;
class FX_PATHPOINT;

// Layering prevents fxcrt from knowing about FPDF_FILEACCESS, so this can't
// be a static method of IFX_SeekableReadStream.
RetainPtr<IFX_SeekableReadStream> MakeSeekableReadStream(
    FPDF_FILEACCESS* pFileAccess);

#ifdef PDF_ENABLE_XFA
// Layering prevents fxcrt from knowing about FPDF_FILEHANDLER, so this can't
// be a static method of IFX_SeekableStream.
RetainPtr<IFX_SeekableStream> MakeSeekableStream(
    FPDF_FILEHANDLER* pFileHandler);
#endif  // PDF_ENABLE_XFA

// Object types for public FPDF_ types; these correspond to next layer down
// from fpdfsdk. For master, these are CPDF_ types, but for XFA, these are
// CPDFXFA_ types.
#ifndef PDF_ENABLE_XFA
using UnderlyingDocumentType = CPDF_Document;
using UnderlyingPageType = CPDF_Page;
#else   // PDF_ENABLE_XFA
using UnderlyingDocumentType = CPDFXFA_Context;
using UnderlyingPageType = CPDFXFA_Page;
#endif  // PDF_ENABLE_XFA

// Conversions to/from underlying types.
UnderlyingDocumentType* UnderlyingFromFPDFDocument(FPDF_DOCUMENT doc);
FPDF_DOCUMENT FPDFDocumentFromUnderlying(UnderlyingDocumentType* doc);

UnderlyingPageType* UnderlyingFromFPDFPage(FPDF_PAGE page);

// Conversions to/from FPDF_ types.
CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc);
FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc);

CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page);

CPDF_PathObject* CPDFPathObjectFromFPDFPageObject(FPDF_PAGEOBJECT page_object);

CPDF_PageObject* CPDFPageObjectFromFPDFPageObject(FPDF_PAGEOBJECT page_object);

CPDF_Object* CPDFObjectFromFPDFAttachment(FPDF_ATTACHMENT attachment);

ByteString CFXByteStringFromFPDFWideString(FPDF_WIDESTRING wide_string);

CFX_DIBitmap* CFXBitmapFromFPDFBitmap(FPDF_BITMAP bitmap);

CFX_FloatRect CFXFloatRectFromFSRECTF(const FS_RECTF& rect);
void FSRECTFFromCFXFloatRect(const CFX_FloatRect& rect, FS_RECTF* out_rect);

const FX_PATHPOINT* FXPathPointFromFPDFPathSegment(FPDF_PATHSEGMENT segment);

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const WideString& text,
                                                  void* buffer,
                                                  unsigned long buflen);

unsigned long DecodeStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen);

void FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
FPDF_BOOL FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy);
void FPDF_RenderPage_Retail(CPDF_PageRenderContext* pContext,
                            FPDF_PAGE page,
                            int start_x,
                            int start_y,
                            int size_x,
                            int size_y,
                            int rotate,
                            int flags,
                            bool bNeedToRestore,
                            IFSDK_PAUSE_Adapter* pause);

void CheckUnSupportError(CPDF_Document* pDoc, uint32_t err_code);
void CheckUnSupportAnnot(CPDF_Document* pDoc, const CPDF_Annot* pPDFAnnot);
void ProcessParseError(CPDF_Parser::Error err);
FPDF_BOOL FPDFPageObj_SetFillColor(FPDF_PAGEOBJECT page_object,
                                   unsigned int R,
                                   unsigned int G,
                                   unsigned int B,
                                   unsigned int A);

#endif  // FPDFSDK_FSDK_DEFINE_H_
