// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_HELPERS_H_
#define FPDFSDK_CPDFSDK_HELPERS_H_

#include "build/build_config.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "public/fpdf_doc.h"
#include "public/fpdf_ext.h"
#include "public/fpdfview.h"

#ifdef PDF_ENABLE_XFA
#include "core/fxcrt/fx_stream.h"
#endif  // PDF_ENABLE_XFA

#if defined(OS_WIN)
#include <math.h>
#include <tchar.h>
#endif

class CPDF_Annot;
class CPDF_AnnotContext;
class CPDF_ClipPath;
class CPDF_ContentMarkItem;
class CPDF_Object;
class CPDF_Font;
class CPDF_LinkExtract;
class CPDF_PageObject;
class CPDF_Stream;
class CPDF_StructElement;
class CPDF_StructTree;
class CPDF_TextPage;
class CPDF_TextPageFind;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_InteractiveForm;
class FX_PATHPOINT;
struct CPDF_JavaScript;

// Conversions to/from underlying types.
IPDF_Page* IPDFPageFromFPDFPage(FPDF_PAGE page);
FPDF_PAGE FPDFPageFromIPDFPage(IPDF_Page* page);
CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page);
FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc);
CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc);

// Conversions to/from incomplete FPDF_ API types.
inline FPDF_ACTION FPDFActionFromCPDFDictionary(const CPDF_Dictionary* action) {
  return reinterpret_cast<FPDF_ACTION>(const_cast<CPDF_Dictionary*>(action));
}
inline CPDF_Dictionary* CPDFDictionaryFromFPDFAction(FPDF_ACTION action) {
  return reinterpret_cast<CPDF_Dictionary*>(action);
}

inline FPDF_ANNOTATION FPDFAnnotationFromCPDFAnnotContext(
    CPDF_AnnotContext* annot) {
  return reinterpret_cast<FPDF_ANNOTATION>(annot);
}
inline CPDF_AnnotContext* CPDFAnnotContextFromFPDFAnnotation(
    FPDF_ANNOTATION annot) {
  return reinterpret_cast<CPDF_AnnotContext*>(annot);
}

inline FPDF_ATTACHMENT FPDFAttachmentFromCPDFObject(CPDF_Object* attachment) {
  return reinterpret_cast<FPDF_ATTACHMENT>(attachment);
}
inline CPDF_Object* CPDFObjectFromFPDFAttachment(FPDF_ATTACHMENT attachment) {
  return reinterpret_cast<CPDF_Object*>(attachment);
}

inline FPDF_BITMAP FPDFBitmapFromCFXDIBitmap(CFX_DIBitmap* bitmap) {
  return reinterpret_cast<FPDF_BITMAP>(bitmap);
}
inline CFX_DIBitmap* CFXDIBitmapFromFPDFBitmap(FPDF_BITMAP bitmap) {
  return reinterpret_cast<CFX_DIBitmap*>(bitmap);
}

inline FPDF_BOOKMARK FPDFBookmarkFromCPDFDictionary(
    const CPDF_Dictionary* bookmark) {
  return reinterpret_cast<FPDF_BOOKMARK>(
      const_cast<CPDF_Dictionary*>(bookmark));
}
inline CPDF_Dictionary* CPDFDictionaryFromFPDFBookmark(FPDF_BOOKMARK bookmark) {
  return reinterpret_cast<CPDF_Dictionary*>(bookmark);
}

inline FPDF_CLIPPATH FPDFClipPathFromCPDFClipPath(CPDF_ClipPath* path) {
  return reinterpret_cast<FPDF_CLIPPATH>(path);
}
inline CPDF_ClipPath* CPDFClipPathFromFPDFClipPath(FPDF_CLIPPATH path) {
  return reinterpret_cast<CPDF_ClipPath*>(path);
}

inline FPDF_DEST FPDFDestFromCPDFArray(const CPDF_Array* dest) {
  return reinterpret_cast<FPDF_DEST>(const_cast<CPDF_Array*>(dest));
}
inline CPDF_Array* CPDFArrayFromFPDFDest(FPDF_DEST dest) {
  return reinterpret_cast<CPDF_Array*>(dest);
}

inline FPDF_FONT FPDFFontFromCPDFFont(CPDF_Font* font) {
  return reinterpret_cast<FPDF_FONT>(font);
}
inline CPDF_Font* CPDFFontFromFPDFFont(FPDF_FONT font) {
  return reinterpret_cast<CPDF_Font*>(font);
}

inline FPDF_JAVASCRIPT_ACTION FPDFJavaScriptActionFromCPDFJavaScriptAction(
    CPDF_JavaScript* javascript) {
  return reinterpret_cast<FPDF_JAVASCRIPT_ACTION>(javascript);
}
inline CPDF_JavaScript* CPDFJavaScriptActionFromFPDFJavaScriptAction(
    FPDF_JAVASCRIPT_ACTION javascript) {
  return reinterpret_cast<CPDF_JavaScript*>(javascript);
}

inline FPDF_LINK FPDFLinkFromCPDFDictionary(CPDF_Dictionary* link) {
  return reinterpret_cast<FPDF_LINK>(link);
}
inline CPDF_Dictionary* CPDFDictionaryFromFPDFLink(FPDF_LINK link) {
  return reinterpret_cast<CPDF_Dictionary*>(link);
}

inline FPDF_PAGELINK FPDFPageLinkFromCPDFLinkExtract(CPDF_LinkExtract* link) {
  return reinterpret_cast<FPDF_PAGELINK>(link);
}
inline CPDF_LinkExtract* CPDFLinkExtractFromFPDFPageLink(FPDF_PAGELINK link) {
  return reinterpret_cast<CPDF_LinkExtract*>(link);
}

inline FPDF_PAGEOBJECT FPDFPageObjectFromCPDFPageObject(
    CPDF_PageObject* page_object) {
  return reinterpret_cast<FPDF_PAGEOBJECT>(page_object);
}
inline CPDF_PageObject* CPDFPageObjectFromFPDFPageObject(
    FPDF_PAGEOBJECT page_object) {
  return reinterpret_cast<CPDF_PageObject*>(page_object);
}

inline FPDF_PAGEOBJECTMARK FPDFPageObjectMarkFromCPDFContentMarkItem(
    CPDF_ContentMarkItem* mark) {
  return reinterpret_cast<FPDF_PAGEOBJECTMARK>(mark);
}
inline CPDF_ContentMarkItem* CPDFContentMarkItemFromFPDFPageObjectMark(
    FPDF_PAGEOBJECTMARK mark) {
  return reinterpret_cast<CPDF_ContentMarkItem*>(mark);
}

inline FPDF_PAGERANGE FPDFPageRangeFromCPDFArray(CPDF_Array* range) {
  return reinterpret_cast<FPDF_PAGERANGE>(range);
}
inline CPDF_Array* CPDFArrayFromFPDFPageRange(FPDF_PAGERANGE range) {
  return reinterpret_cast<CPDF_Array*>(range);
}

inline FPDF_PATHSEGMENT FPDFPathSegmentFromFXPathPoint(
    const FX_PATHPOINT* segment) {
  return reinterpret_cast<FPDF_PATHSEGMENT>(segment);
}
inline const FX_PATHPOINT* FXPathPointFromFPDFPathSegment(
    FPDF_PATHSEGMENT segment) {
  return reinterpret_cast<const FX_PATHPOINT*>(segment);
}

inline FPDF_STRUCTTREE FPDFStructTreeFromCPDFStructTree(
    CPDF_StructTree* struct_tree) {
  return reinterpret_cast<FPDF_STRUCTTREE>(struct_tree);
}
inline CPDF_StructTree* CPDFStructTreeFromFPDFStructTree(
    FPDF_STRUCTTREE struct_tree) {
  return reinterpret_cast<CPDF_StructTree*>(struct_tree);
}

inline FPDF_STRUCTELEMENT FPDFStructElementFromCPDFStructElement(
    CPDF_StructElement* struct_element) {
  return reinterpret_cast<FPDF_STRUCTELEMENT>(struct_element);
}
inline CPDF_StructElement* CPDFStructElementFromFPDFStructElement(
    FPDF_STRUCTELEMENT struct_element) {
  return reinterpret_cast<CPDF_StructElement*>(struct_element);
}

inline FPDF_TEXTPAGE FPDFTextPageFromCPDFTextPage(CPDF_TextPage* page) {
  return reinterpret_cast<FPDF_TEXTPAGE>(page);
}
inline CPDF_TextPage* CPDFTextPageFromFPDFTextPage(FPDF_TEXTPAGE page) {
  return reinterpret_cast<CPDF_TextPage*>(page);
}

inline FPDF_SCHHANDLE FPDFSchHandleFromCPDFTextPageFind(
    CPDF_TextPageFind* handle) {
  return reinterpret_cast<FPDF_SCHHANDLE>(handle);
}
inline CPDF_TextPageFind* CPDFTextPageFindFromFPDFSchHandle(
    FPDF_SCHHANDLE handle) {
  return reinterpret_cast<CPDF_TextPageFind*>(handle);
}

inline FPDF_FORMHANDLE FPDFFormHandleFromCPDFSDKFormFillEnvironment(
    CPDFSDK_FormFillEnvironment* handle) {
  return reinterpret_cast<FPDF_FORMHANDLE>(handle);
}
inline CPDFSDK_FormFillEnvironment*
CPDFSDKFormFillEnvironmentFromFPDFFormHandle(FPDF_FORMHANDLE handle) {
  return reinterpret_cast<CPDFSDK_FormFillEnvironment*>(handle);
}

CPDFSDK_InteractiveForm* FormHandleToInteractiveForm(FPDF_FORMHANDLE hHandle);

ByteString ByteStringFromFPDFWideString(FPDF_WIDESTRING wide_string);
WideString WideStringFromFPDFWideString(FPDF_WIDESTRING wide_string);

#ifdef PDF_ENABLE_XFA
// Layering prevents fxcrt from knowing about FPDF_FILEHANDLER, so this can't
// be a static method of IFX_SeekableStream.
RetainPtr<IFX_SeekableStream> MakeSeekableStream(
    FPDF_FILEHANDLER* pFileHandler);
#endif  // PDF_ENABLE_XFA

const CPDF_Array* GetQuadPointsArrayFromDictionary(const CPDF_Dictionary* dict);
CPDF_Array* GetQuadPointsArrayFromDictionary(CPDF_Dictionary* dict);
CPDF_Array* AddQuadPointsArrayToDictionary(CPDF_Dictionary* dict);
bool IsValidQuadPointsIndex(const CPDF_Array* array, size_t index);
bool GetQuadPointsAtIndex(const CPDF_Array* array,
                          size_t quad_index,
                          FS_QUADPOINTSF* quad_points);

CFX_PointF CFXPointFFromFSPointF(const FS_POINTF& point);

CFX_FloatRect CFXFloatRectFromFSRectF(const FS_RECTF& rect);
FS_RECTF FSRectFFromCFXFloatRect(const CFX_FloatRect& rect);

CFX_Matrix CFXMatrixFromFSMatrix(const FS_MATRIX& matrix);
FS_MATRIX FSMatrixFromCFXMatrix(const CFX_Matrix& matrix);

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const WideString& text,
                                                  void* buffer,
                                                  unsigned long buflen);

// Returns the length of the raw stream data from |stream|. The raw data is the
// stream's data as stored in the PDF without applying any filters. If |buffer|
// is non-nullptr and |buflen| is large enough to contain the raw data, then
// the raw data is copied into |buffer|.
unsigned long GetRawStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen);

// Return the length of the decoded stream data of |stream|. The decoded data is
// the uncompressed stream data, i.e. the raw stream data after having all
// filters applied. If |buffer| is non-nullptr and |buflen| is large enough to
// contain the decoded data, then the decoded data is copied into |buffer|.
unsigned long DecodeStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen);

void SetPDFSandboxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
FPDF_BOOL IsPDFSandboxPolicyEnabled(FPDF_DWORD policy);

void SetPDFUnsupportInfo(UNSUPPORT_INFO* unsp_info);
void ReportUnsupportedFeatures(const CPDF_Document* pDoc);
void ReportUnsupportedXFA(const CPDF_Document* pDoc);
void CheckForUnsupportedAnnot(const CPDF_Annot* pAnnot);
void ProcessParseError(CPDF_Parser::Error err);

#endif  // FPDFSDK_CPDFSDK_HELPERS_H_
