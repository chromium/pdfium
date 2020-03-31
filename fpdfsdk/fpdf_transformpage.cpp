// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_transformpage.h"

#include <memory>
#include <sstream>
#include <vector>

#include "constants/page_object.h"
#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"
#include "core/fpdfapi/page/cpdf_clippath.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/render_defines.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/span.h"

namespace {

void SetBoundingBox(CPDF_Page* page,
                    const ByteString& key,
                    const CFX_FloatRect& rect) {
  if (!page)
    return;

  page->GetDict()->SetRectFor(key, rect);
  page->UpdateDimensions();
}

bool GetBoundingBox(CPDF_Page* page,
                    const ByteString& key,
                    float* left,
                    float* bottom,
                    float* right,
                    float* top) {
  if (!page || !left || !bottom || !right || !top)
    return false;

  CPDF_Array* pArray = page->GetDict()->GetArrayFor(key);
  if (!pArray)
    return false;

  *left = pArray->GetNumberAt(0);
  *bottom = pArray->GetNumberAt(1);
  *right = pArray->GetNumberAt(2);
  *top = pArray->GetNumberAt(3);
  return true;
}

CPDF_Object* GetPageContent(CPDF_Dictionary* pPageDict) {
  return pPageDict->GetDirectObjectFor(pdfium::page_object::kContents);
}

void OutputPath(std::ostringstream& buf, CPDF_Path path) {
  const CFX_PathData* pPathData = path.GetObject();
  if (!pPathData)
    return;

  pdfium::span<const FX_PATHPOINT> points = pPathData->GetPoints();
  if (path.IsRect()) {
    CFX_PointF diff = points[2].m_Point - points[0].m_Point;
    buf << points[0].m_Point.x << " " << points[0].m_Point.y << " " << diff.x
        << " " << diff.y << " re\n";
    return;
  }

  for (size_t i = 0; i < points.size(); ++i) {
    buf << points[i].m_Point.x << " " << points[i].m_Point.y;
    FXPT_TYPE point_type = points[i].m_Type;
    if (point_type == FXPT_TYPE::MoveTo) {
      buf << " m\n";
    } else if (point_type == FXPT_TYPE::BezierTo) {
      buf << " " << points[i + 1].m_Point.x << " " << points[i + 1].m_Point.y
          << " " << points[i + 2].m_Point.x << " " << points[i + 2].m_Point.y;
      buf << " c";
      if (points[i + 2].m_CloseFigure)
        buf << " h";
      buf << "\n";

      i += 2;
    } else if (point_type == FXPT_TYPE::LineTo) {
      buf << " l";
      if (points[i].m_CloseFigure)
        buf << " h";
      buf << "\n";
    }
  }
}

}  // namespace

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetMediaBox(FPDF_PAGE page,
                                                    float left,
                                                    float bottom,
                                                    float right,
                                                    float top) {
  SetBoundingBox(CPDFPageFromFPDFPage(page), pdfium::page_object::kMediaBox,
                 CFX_FloatRect(left, bottom, right, top));
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetCropBox(FPDF_PAGE page,
                                                   float left,
                                                   float bottom,
                                                   float right,
                                                   float top) {
  SetBoundingBox(CPDFPageFromFPDFPage(page), pdfium::page_object::kCropBox,
                 CFX_FloatRect(left, bottom, right, top));
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetBleedBox(FPDF_PAGE page,
                                                    float left,
                                                    float bottom,
                                                    float right,
                                                    float top) {
  SetBoundingBox(CPDFPageFromFPDFPage(page), pdfium::page_object::kBleedBox,
                 CFX_FloatRect(left, bottom, right, top));
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetTrimBox(FPDF_PAGE page,
                                                   float left,
                                                   float bottom,
                                                   float right,
                                                   float top) {
  SetBoundingBox(CPDFPageFromFPDFPage(page), pdfium::page_object::kTrimBox,
                 CFX_FloatRect(left, bottom, right, top));
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_SetArtBox(FPDF_PAGE page,
                                                  float left,
                                                  float bottom,
                                                  float right,
                                                  float top) {
  SetBoundingBox(CPDFPageFromFPDFPage(page), pdfium::page_object::kArtBox,
                 CFX_FloatRect(left, bottom, right, top));
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetMediaBox(FPDF_PAGE page,
                                                         float* left,
                                                         float* bottom,
                                                         float* right,
                                                         float* top) {
  return GetBoundingBox(CPDFPageFromFPDFPage(page),
                        pdfium::page_object::kMediaBox, left, bottom, right,
                        top);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetCropBox(FPDF_PAGE page,
                                                        float* left,
                                                        float* bottom,
                                                        float* right,
                                                        float* top) {
  return GetBoundingBox(CPDFPageFromFPDFPage(page),
                        pdfium::page_object::kCropBox, left, bottom, right,
                        top);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetBleedBox(FPDF_PAGE page,
                                                         float* left,
                                                         float* bottom,
                                                         float* right,
                                                         float* top) {
  return GetBoundingBox(CPDFPageFromFPDFPage(page),
                        pdfium::page_object::kBleedBox, left, bottom, right,
                        top);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetTrimBox(FPDF_PAGE page,
                                                        float* left,
                                                        float* bottom,
                                                        float* right,
                                                        float* top) {
  return GetBoundingBox(CPDFPageFromFPDFPage(page),
                        pdfium::page_object::kTrimBox, left, bottom, right,
                        top);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDFPage_GetArtBox(FPDF_PAGE page,
                                                       float* left,
                                                       float* bottom,
                                                       float* right,
                                                       float* top) {
  return GetBoundingBox(CPDFPageFromFPDFPage(page),
                        pdfium::page_object::kArtBox, left, bottom, right, top);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDFPage_TransFormWithClip(FPDF_PAGE page,
                           const FS_MATRIX* matrix,
                           const FS_RECTF* clipRect) {
  if (!matrix && !clipRect)
    return false;

  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return false;

  CPDF_Dictionary* pPageDict = pPage->GetDict();
  CPDF_Object* pContentObj = GetPageContent(pPageDict);
  if (!pContentObj)
    return false;

  CPDF_Document* pDoc = pPage->GetDocument();
  if (!pDoc)
    return false;

  std::ostringstream text_buf;
  text_buf << "q ";

  if (clipRect) {
    CFX_FloatRect rect = CFXFloatRectFromFSRectF(*clipRect);
    rect.Normalize();

    WriteFloat(text_buf, rect.left) << " ";
    WriteFloat(text_buf, rect.bottom) << " ";
    WriteFloat(text_buf, rect.Width()) << " ";
    WriteFloat(text_buf, rect.Height()) << " re W* n ";
  }
  if (matrix) {
    CFX_Matrix m = CFXMatrixFromFSMatrix(*matrix);
    text_buf << m << " cm ";
  }

  CPDF_Stream* pStream =
      pDoc->NewIndirect<CPDF_Stream>(nullptr, 0, pDoc->New<CPDF_Dictionary>());
  pStream->SetDataFromStringstream(&text_buf);

  CPDF_Stream* pEndStream =
      pDoc->NewIndirect<CPDF_Stream>(nullptr, 0, pDoc->New<CPDF_Dictionary>());
  pEndStream->SetData(ByteStringView(" Q").raw_span());

  if (CPDF_Array* pContentArray = ToArray(pContentObj)) {
    pContentArray->InsertNewAt<CPDF_Reference>(0, pDoc, pStream->GetObjNum());
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pEndStream->GetObjNum());
  } else if (pContentObj->IsStream() && !pContentObj->IsInline()) {
    pContentArray = pDoc->NewIndirect<CPDF_Array>();
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pStream->GetObjNum());
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pContentObj->GetObjNum());
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pEndStream->GetObjNum());
    pPageDict->SetNewFor<CPDF_Reference>(pdfium::page_object::kContents, pDoc,
                                         pContentArray->GetObjNum());
  }

  // Need to transform the patterns as well.
  CPDF_Dictionary* pRes =
      pPageDict->GetDictFor(pdfium::page_object::kResources);
  if (!pRes)
    return true;

  CPDF_Dictionary* pPatternDict = pRes->GetDictFor("Pattern");
  if (!pPatternDict)
    return true;

  CPDF_DictionaryLocker locker(pPatternDict);
  for (const auto& it : locker) {
    CPDF_Object* pObj = it.second.Get();
    if (pObj->IsReference())
      pObj = pObj->GetDirect();

    CPDF_Dictionary* pDict = nullptr;
    if (pObj->IsDictionary())
      pDict = pObj->AsDictionary();
    else if (CPDF_Stream* pObjStream = pObj->AsStream())
      pDict = pObjStream->GetDict();
    else
      continue;

    if (matrix) {
      CFX_Matrix m = CFXMatrixFromFSMatrix(*matrix);
      pDict->SetMatrixFor("Matrix", pDict->GetMatrixFor("Matrix") * m);
    }
  }

  return true;
}

FPDF_EXPORT void FPDF_CALLCONV
FPDFPageObj_TransformClipPath(FPDF_PAGEOBJECT page_object,
                              double a,
                              double b,
                              double c,
                              double d,
                              double e,
                              double f) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return;

  CFX_Matrix matrix((float)a, (float)b, (float)c, (float)d, (float)e, (float)f);

  // Special treatment to shading object, because the ClipPath for shading
  // object is already transformed.
  if (!pPageObj->IsShading())
    pPageObj->TransformClipPath(matrix);
  pPageObj->TransformGeneralState(matrix);
}

FPDF_EXPORT FPDF_CLIPPATH FPDF_CALLCONV
FPDFPageObj_GetClipPath(FPDF_PAGEOBJECT page_object) {
  CPDF_PageObject* pPageObj = CPDFPageObjectFromFPDFPageObject(page_object);
  if (!pPageObj)
    return nullptr;

  return FPDFClipPathFromCPDFClipPath(&pPageObj->m_ClipPath);
}

FPDF_EXPORT int FPDF_CALLCONV FPDFClipPath_CountPaths(FPDF_CLIPPATH clip_path) {
  CPDF_ClipPath* pClipPath = CPDFClipPathFromFPDFClipPath(clip_path);
  if (!pClipPath || !pClipPath->HasRef())
    return -1;

  return pClipPath->GetPathCount();
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFClipPath_CountPathSegments(FPDF_CLIPPATH clip_path, int path_index) {
  CPDF_ClipPath* pClipPath = CPDFClipPathFromFPDFClipPath(clip_path);
  if (!pClipPath || !pClipPath->HasRef())
    return -1;

  if (path_index < 0 ||
      static_cast<size_t>(path_index) >= pClipPath->GetPathCount()) {
    return -1;
  }

  return pdfium::CollectionSize<int>(
      pClipPath->GetPath(path_index).GetPoints());
}

FPDF_EXPORT FPDF_PATHSEGMENT FPDF_CALLCONV
FPDFClipPath_GetPathSegment(FPDF_CLIPPATH clip_path,
                            int path_index,
                            int segment_index) {
  CPDF_ClipPath* pClipPath = CPDFClipPathFromFPDFClipPath(clip_path);
  if (!pClipPath || !pClipPath->HasRef())
    return nullptr;

  if (path_index < 0 ||
      static_cast<size_t>(path_index) >= pClipPath->GetPathCount()) {
    return nullptr;
  }

  pdfium::span<const FX_PATHPOINT> points =
      pClipPath->GetPath(path_index).GetPoints();
  if (!pdfium::IndexInBounds(points, segment_index))
    return nullptr;

  return FPDFPathSegmentFromFXPathPoint(&points[segment_index]);
}

FPDF_EXPORT FPDF_CLIPPATH FPDF_CALLCONV FPDF_CreateClipPath(float left,
                                                            float bottom,
                                                            float right,
                                                            float top) {
  CPDF_Path Path;
  Path.AppendRect(left, bottom, right, top);

  auto pNewClipPath = pdfium::MakeUnique<CPDF_ClipPath>();
  pNewClipPath->AppendPath(Path, FXFILL_ALTERNATE, false);

  // Caller takes ownership.
  return FPDFClipPathFromCPDFClipPath(pNewClipPath.release());
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_DestroyClipPath(FPDF_CLIPPATH clipPath) {
  // Take ownership back from caller and destroy.
  std::unique_ptr<CPDF_ClipPath>(CPDFClipPathFromFPDFClipPath(clipPath));
}

FPDF_EXPORT void FPDF_CALLCONV FPDFPage_InsertClipPath(FPDF_PAGE page,
                                                       FPDF_CLIPPATH clipPath) {
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return;

  CPDF_Dictionary* pPageDict = pPage->GetDict();
  CPDF_Object* pContentObj = GetPageContent(pPageDict);
  if (!pContentObj)
    return;

  std::ostringstream strClip;
  CPDF_ClipPath* pClipPath = CPDFClipPathFromFPDFClipPath(clipPath);
  for (size_t i = 0; i < pClipPath->GetPathCount(); ++i) {
    CPDF_Path path = pClipPath->GetPath(i);
    if (path.GetPoints().empty()) {
      // Empty clipping (totally clipped out)
      strClip << "0 0 m W n ";
    } else {
      OutputPath(strClip, path);
      if (pClipPath->GetClipType(i) == FXFILL_WINDING)
        strClip << "W n\n";
      else
        strClip << "W* n\n";
    }
  }
  CPDF_Document* pDoc = pPage->GetDocument();
  if (!pDoc)
    return;

  CPDF_Stream* pStream =
      pDoc->NewIndirect<CPDF_Stream>(nullptr, 0, pDoc->New<CPDF_Dictionary>());
  pStream->SetDataFromStringstream(&strClip);

  if (CPDF_Array* pArray = ToArray(pContentObj)) {
    pArray->InsertNewAt<CPDF_Reference>(0, pDoc, pStream->GetObjNum());
  } else if (pContentObj->IsStream() && !pContentObj->IsInline()) {
    CPDF_Array* pContentArray = pDoc->NewIndirect<CPDF_Array>();
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pStream->GetObjNum());
    pContentArray->AppendNew<CPDF_Reference>(pDoc, pContentObj->GetObjNum());
    pPageDict->SetNewFor<CPDF_Reference>(pdfium::page_object::kContents, pDoc,
                                         pContentArray->GetObjNum());
  }
}
