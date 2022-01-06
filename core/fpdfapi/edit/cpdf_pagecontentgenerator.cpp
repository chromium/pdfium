// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/edit/cpdf_pagecontentgenerator.h"

#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <utility>

#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"
#include "core/fpdfapi/edit/cpdf_pagecontentmanager.h"
#include "core/fpdfapi/edit/cpdf_stringarchivestream.h"
#include "core/fpdfapi/font/cpdf_truetypefont.h"
#include "core/fpdfapi/font/cpdf_type1font.h"
#include "core/fpdfapi/page/cpdf_contentmarks.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_path.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"

namespace {

bool GetColor(const CPDF_Color* pColor, float* rgb) {
  int intRGB[3];
  if (!pColor || !pColor->IsColorSpaceRGB() ||
      !pColor->GetRGB(&intRGB[0], &intRGB[1], &intRGB[2])) {
    return false;
  }
  rgb[0] = intRGB[0] / 255.0f;
  rgb[1] = intRGB[1] / 255.0f;
  rgb[2] = intRGB[2] / 255.0f;
  return true;
}

}  // namespace

CPDF_PageContentGenerator::CPDF_PageContentGenerator(
    CPDF_PageObjectHolder* pObjHolder)
    : m_pObjHolder(pObjHolder), m_pDocument(pObjHolder->GetDocument()) {
  for (const auto& pObj : *pObjHolder) {
    if (pObj)
      m_pageObjects.emplace_back(pObj.get());
  }
}

CPDF_PageContentGenerator::~CPDF_PageContentGenerator() = default;

void CPDF_PageContentGenerator::GenerateContent() {
  DCHECK(m_pObjHolder->IsPage());
  UpdateContentStreams(GenerateModifiedStreams());
}

std::map<int32_t, std::ostringstream>
CPDF_PageContentGenerator::GenerateModifiedStreams() {
  // Make sure default graphics are created.
  GetOrCreateDefaultGraphics();

  // Figure out which streams are dirty.
  std::set<int32_t> all_dirty_streams;
  for (auto& pPageObj : m_pageObjects) {
    if (pPageObj->IsDirty())
      all_dirty_streams.insert(pPageObj->GetContentStream());
  }
  std::set<int32_t> marked_dirty_streams = m_pObjHolder->TakeDirtyStreams();
  all_dirty_streams.insert(marked_dirty_streams.begin(),
                           marked_dirty_streams.end());

  // Start regenerating dirty streams.
  std::map<int32_t, std::ostringstream> streams;
  std::set<int32_t> empty_streams;
  std::unique_ptr<const CPDF_ContentMarks> empty_content_marks =
      std::make_unique<CPDF_ContentMarks>();
  std::map<int32_t, const CPDF_ContentMarks*> current_content_marks;

  for (int32_t dirty_stream : all_dirty_streams) {
    std::ostringstream buf;

    // Set the default graphic state values
    buf << "q\n";
    if (!m_pObjHolder->GetLastCTM().IsIdentity())
      buf << m_pObjHolder->GetLastCTM().GetInverse() << " cm\n";

    ProcessDefaultGraphics(&buf);
    streams[dirty_stream] = std::move(buf);
    empty_streams.insert(dirty_stream);
    current_content_marks[dirty_stream] = empty_content_marks.get();
  }

  // Process the page objects, write into each dirty stream.
  for (auto& pPageObj : m_pageObjects) {
    int stream_index = pPageObj->GetContentStream();
    auto it = streams.find(stream_index);
    if (it == streams.end())
      continue;

    std::ostringstream* buf = &it->second;
    empty_streams.erase(stream_index);
    current_content_marks[stream_index] = ProcessContentMarks(
        buf, pPageObj.Get(), current_content_marks[stream_index]);
    ProcessPageObject(buf, pPageObj.Get());
  }

  // Finish dirty streams.
  for (int32_t dirty_stream : all_dirty_streams) {
    std::ostringstream* buf = &streams[dirty_stream];
    if (pdfium::Contains(empty_streams, dirty_stream)) {
      // Clear to show that this stream needs to be deleted.
      buf->str("");
    } else {
      FinishMarks(buf, current_content_marks[dirty_stream]);

      // Return graphics to original state
      *buf << "Q\n";
    }
  }

  return streams;
}

void CPDF_PageContentGenerator::UpdateContentStreams(
    std::map<int32_t, std::ostringstream>&& new_stream_data) {
  // If no streams were regenerated or removed, nothing to do here.
  if (new_stream_data.empty())
    return;

  CPDF_PageContentManager page_content_manager(m_pObjHolder.Get());

  for (auto& pair : new_stream_data) {
    int32_t stream_index = pair.first;
    std::ostringstream* buf = &pair.second;

    if (stream_index == CPDF_PageObject::kNoContentStream) {
      int new_stream_index = page_content_manager.AddStream(buf);
      UpdateStreamlessPageObjects(new_stream_index);
      continue;
    }

    CPDF_Stream* old_stream =
        page_content_manager.GetStreamByIndex(stream_index);
    DCHECK(old_stream);

    // If buf is now empty, remove the stream instead of setting the data.
    if (buf->tellp() <= 0)
      page_content_manager.ScheduleRemoveStreamByIndex(stream_index);
    else
      old_stream->SetDataFromStringstreamAndRemoveFilter(buf);
  }

  page_content_manager.ExecuteScheduledRemovals();
}

ByteString CPDF_PageContentGenerator::RealizeResource(
    const CPDF_Object* pResource,
    const ByteString& bsType) const {
  DCHECK(pResource);
  if (!m_pObjHolder->GetResources()) {
    m_pObjHolder->SetResources(m_pDocument->NewIndirect<CPDF_Dictionary>());
    m_pObjHolder->GetDict()->SetNewFor<CPDF_Reference>(
        "Resources", m_pDocument.Get(),
        m_pObjHolder->GetResources()->GetObjNum());
  }
  CPDF_Dictionary* pResList = m_pObjHolder->GetResources()->GetDictFor(bsType);
  if (!pResList)
    pResList = m_pObjHolder->GetResources()->SetNewFor<CPDF_Dictionary>(bsType);

  ByteString name;
  int idnum = 1;
  while (1) {
    name = ByteString::Format("FX%c%d", bsType[0], idnum);
    if (!pResList->KeyExist(name))
      break;

    idnum++;
  }
  pResList->SetNewFor<CPDF_Reference>(name, m_pDocument.Get(),
                                      pResource->GetObjNum());
  return name;
}

bool CPDF_PageContentGenerator::ProcessPageObjects(std::ostringstream* buf) {
  bool bDirty = false;
  std::unique_ptr<const CPDF_ContentMarks> empty_content_marks =
      std::make_unique<CPDF_ContentMarks>();
  const CPDF_ContentMarks* content_marks = empty_content_marks.get();

  for (auto& pPageObj : m_pageObjects) {
    if (m_pObjHolder->IsPage() && !pPageObj->IsDirty())
      continue;

    bDirty = true;
    content_marks = ProcessContentMarks(buf, pPageObj.Get(), content_marks);
    ProcessPageObject(buf, pPageObj.Get());
  }
  FinishMarks(buf, content_marks);
  return bDirty;
}

void CPDF_PageContentGenerator::UpdateStreamlessPageObjects(
    int new_content_stream_index) {
  for (auto& pPageObj : m_pageObjects) {
    if (pPageObj->GetContentStream() == CPDF_PageObject::kNoContentStream)
      pPageObj->SetContentStream(new_content_stream_index);
  }
}

const CPDF_ContentMarks* CPDF_PageContentGenerator::ProcessContentMarks(
    std::ostringstream* buf,
    const CPDF_PageObject* pPageObj,
    const CPDF_ContentMarks* pPrev) {
  const CPDF_ContentMarks* pNext = pPageObj->GetContentMarks();
  const size_t first_different = pPrev->FindFirstDifference(pNext);

  // Close all marks that are in prev but not in next.
  // Technically we should iterate backwards to close from the top to the
  // bottom, but since the EMC operators do not identify which mark they are
  // closing, it does not matter.
  for (size_t i = first_different; i < pPrev->CountItems(); ++i)
    *buf << "EMC\n";

  // Open all marks that are in next but not in prev.
  for (size_t i = first_different; i < pNext->CountItems(); ++i) {
    const CPDF_ContentMarkItem* item = pNext->GetItem(i);

    // Write mark tag.
    *buf << "/" << PDF_NameEncode(item->GetName()) << " ";

    // If there are no parameters, write a BMC (begin marked content) operator.
    if (item->GetParamType() == CPDF_ContentMarkItem::kNone) {
      *buf << "BMC\n";
      continue;
    }

    // If there are parameters, write properties, direct or indirect.
    switch (item->GetParamType()) {
      case CPDF_ContentMarkItem::kDirectDict: {
        CPDF_StringArchiveStream archive_stream(buf);
        item->GetParam()->WriteTo(&archive_stream, nullptr);
        *buf << " ";
        break;
      }
      case CPDF_ContentMarkItem::kPropertiesDict: {
        *buf << "/" << item->GetPropertyName() << " ";
        break;
      }
      default:
        NOTREACHED();
        break;
    }

    // Write BDC (begin dictionary content) operator.
    *buf << "BDC\n";
  }

  return pNext;
}

void CPDF_PageContentGenerator::FinishMarks(
    std::ostringstream* buf,
    const CPDF_ContentMarks* pContentMarks) {
  // Technically we should iterate backwards to close from the top to the
  // bottom, but since the EMC operators do not identify which mark they are
  // closing, it does not matter.
  for (size_t i = 0; i < pContentMarks->CountItems(); ++i)
    *buf << "EMC\n";
}

void CPDF_PageContentGenerator::ProcessPageObject(std::ostringstream* buf,
                                                  CPDF_PageObject* pPageObj) {
  if (CPDF_ImageObject* pImageObject = pPageObj->AsImage())
    ProcessImage(buf, pImageObject);
  else if (CPDF_FormObject* pFormObj = pPageObj->AsForm())
    ProcessForm(buf, pFormObj);
  else if (CPDF_PathObject* pPathObj = pPageObj->AsPath())
    ProcessPath(buf, pPathObj);
  else if (CPDF_TextObject* pTextObj = pPageObj->AsText())
    ProcessText(buf, pTextObj);
  pPageObj->SetDirty(false);
}

void CPDF_PageContentGenerator::ProcessImage(std::ostringstream* buf,
                                             CPDF_ImageObject* pImageObj) {
  if ((pImageObj->matrix().a == 0 && pImageObj->matrix().b == 0) ||
      (pImageObj->matrix().c == 0 && pImageObj->matrix().d == 0)) {
    return;
  }

  RetainPtr<CPDF_Image> pImage = pImageObj->GetImage();
  if (pImage->IsInline())
    return;

  CPDF_Stream* pStream = pImage->GetStream();
  if (!pStream)
    return;

  *buf << "q " << pImageObj->matrix() << " cm ";

  bool bWasInline = pStream->IsInline();
  if (bWasInline)
    pImage->ConvertStreamToIndirectObject();

  ByteString name = RealizeResource(pStream, "XObject");
  if (bWasInline) {
    auto* pPageData = CPDF_DocPageData::FromDocument(m_pDocument.Get());
    pImageObj->SetImage(pPageData->GetImage(pStream->GetObjNum()));
  }

  *buf << "/" << PDF_NameEncode(name) << " Do Q\n";
}

void CPDF_PageContentGenerator::ProcessForm(std::ostringstream* buf,
                                            CPDF_FormObject* pFormObj) {
  if ((pFormObj->form_matrix().a == 0 && pFormObj->form_matrix().b == 0) ||
      (pFormObj->form_matrix().c == 0 && pFormObj->form_matrix().d == 0)) {
    return;
  }

  const CPDF_Stream* pStream = pFormObj->form()->GetStream();
  if (!pStream)
    return;

  *buf << "q\n" << pFormObj->form_matrix() << " cm ";

  ByteString name = RealizeResource(pStream, "XObject");
  *buf << "/" << PDF_NameEncode(name) << " Do Q\n";
}

// Processing path construction with operators from Table 4.9 of PDF spec 1.7:
// "re" appends a rectangle (here, used only if the whole path is a rectangle)
// "m" moves current point to the given coordinates
// "l" creates a line from current point to the new point
// "c" adds a Bezier curve from current to last point, using the two other
// points as the Bezier control points
// Note: "l", "c" change the current point
// "h" closes the subpath (appends a line from current to starting point)
void CPDF_PageContentGenerator::ProcessPathPoints(std::ostringstream* buf,
                                                  CPDF_Path* pPath) {
  pdfium::span<const CFX_Path::Point> points = pPath->GetPoints();
  if (pPath->IsRect()) {
    CFX_PointF diff = points[2].m_Point - points[0].m_Point;
    *buf << points[0].m_Point << " " << diff << " re";
    return;
  }
  for (size_t i = 0; i < points.size(); ++i) {
    if (i > 0)
      *buf << " ";

    *buf << points[i].m_Point;

    CFX_Path::Point::Type point_type = points[i].m_Type;
    if (point_type == CFX_Path::Point::Type::kMove) {
      *buf << " m";
    } else if (point_type == CFX_Path::Point::Type::kLine) {
      *buf << " l";
    } else if (point_type == CFX_Path::Point::Type::kBezier) {
      if (i + 2 >= points.size() ||
          !points[i].IsTypeAndOpen(CFX_Path::Point::Type::kBezier) ||
          !points[i + 1].IsTypeAndOpen(CFX_Path::Point::Type::kBezier) ||
          points[i + 2].m_Type != CFX_Path::Point::Type::kBezier) {
        // If format is not supported, close the path and paint
        *buf << " h";
        break;
      }
      *buf << " ";
      *buf << points[i + 1].m_Point << " ";
      *buf << points[i + 2].m_Point << " c";
      i += 2;
    }
    if (points[i].m_CloseFigure)
      *buf << " h";
  }
}

// Processing path painting with operators from Table 4.10 of PDF spec 1.7:
// Path painting operators: "S", "n", "B", "f", "B*", "f*", depending on
// the filling mode and whether we want stroking the path or not.
// "Q" restores the graphics state imposed by the ProcessGraphics method.
void CPDF_PageContentGenerator::ProcessPath(std::ostringstream* buf,
                                            CPDF_PathObject* pPathObj) {
  ProcessGraphics(buf, pPathObj);

  *buf << pPathObj->matrix() << " cm ";
  ProcessPathPoints(buf, &pPathObj->path());

  if (pPathObj->has_no_filltype())
    *buf << (pPathObj->stroke() ? " S" : " n");
  else if (pPathObj->has_winding_filltype())
    *buf << (pPathObj->stroke() ? " B" : " f");
  else if (pPathObj->has_alternate_filltype())
    *buf << (pPathObj->stroke() ? " B*" : " f*");
  *buf << " Q\n";
}

// This method supports color operators rg and RGB from Table 4.24 of PDF spec
// 1.7. A color will not be set if the colorspace is not DefaultRGB or the RGB
// values cannot be obtained. The method also adds an external graphics
// dictionary, as described in Section 4.3.4.
// "rg" sets the fill color, "RG" sets the stroke color (using DefaultRGB)
// "w" sets the stroke line width.
// "ca" sets the fill alpha, "CA" sets the stroke alpha.
// "W" and "W*" modify the clipping path using the nonzero winding rule and
// even-odd rules, respectively.
// "q" saves the graphics state, so that the settings can later be reversed
void CPDF_PageContentGenerator::ProcessGraphics(std::ostringstream* buf,
                                                CPDF_PageObject* pPageObj) {
  *buf << "q ";
  float fillColor[3];
  if (GetColor(pPageObj->m_ColorState.GetFillColor(), fillColor)) {
    *buf << fillColor[0] << " " << fillColor[1] << " " << fillColor[2]
         << " rg ";
  }
  float strokeColor[3];
  if (GetColor(pPageObj->m_ColorState.GetStrokeColor(), strokeColor)) {
    *buf << strokeColor[0] << " " << strokeColor[1] << " " << strokeColor[2]
         << " RG ";
  }
  float lineWidth = pPageObj->m_GraphState.GetLineWidth();
  if (lineWidth != 1.0f)
    WriteFloat(*buf, lineWidth) << " w ";
  CFX_GraphStateData::LineCap lineCap = pPageObj->m_GraphState.GetLineCap();
  if (lineCap != CFX_GraphStateData::LineCap::kButt)
    *buf << static_cast<int>(lineCap) << " J ";
  CFX_GraphStateData::LineJoin lineJoin = pPageObj->m_GraphState.GetLineJoin();
  if (lineJoin != CFX_GraphStateData::LineJoin::kMiter)
    *buf << static_cast<int>(lineJoin) << " j ";

  const CPDF_ClipPath& clip_path = pPageObj->m_ClipPath;
  if (clip_path.HasRef()) {
    for (size_t i = 0; i < clip_path.GetPathCount(); ++i) {
      CPDF_Path path = clip_path.GetPath(i);
      ProcessPathPoints(buf, &path);
      switch (clip_path.GetClipType(i)) {
        case CFX_FillRenderOptions::FillType::kWinding:
          *buf << " W ";
          break;
        case CFX_FillRenderOptions::FillType::kEvenOdd:
          *buf << " W* ";
          break;
        case CFX_FillRenderOptions::FillType::kNoFill:
          NOTREACHED();
          break;
      }

      // Use a no-op path-painting operator to terminate the path without
      // causing any marks to be placed on the page.
      *buf << "n ";
    }
  }

  GraphicsData graphD;
  graphD.fillAlpha = pPageObj->m_GeneralState.GetFillAlpha();
  graphD.strokeAlpha = pPageObj->m_GeneralState.GetStrokeAlpha();
  graphD.blendType = pPageObj->m_GeneralState.GetBlendType();
  if (graphD.fillAlpha == 1.0f && graphD.strokeAlpha == 1.0f &&
      graphD.blendType == BlendMode::kNormal) {
    return;
  }

  ByteString name;
  absl::optional<ByteString> maybe_name =
      m_pObjHolder->GraphicsMapSearch(graphD);
  if (maybe_name.has_value()) {
    name = std::move(maybe_name.value());
  } else {
    auto gsDict = pdfium::MakeRetain<CPDF_Dictionary>();
    if (graphD.fillAlpha != 1.0f)
      gsDict->SetNewFor<CPDF_Number>("ca", graphD.fillAlpha);

    if (graphD.strokeAlpha != 1.0f)
      gsDict->SetNewFor<CPDF_Number>("CA", graphD.strokeAlpha);

    if (graphD.blendType != BlendMode::kNormal) {
      gsDict->SetNewFor<CPDF_Name>("BM",
                                   pPageObj->m_GeneralState.GetBlendMode());
    }
    CPDF_Object* pDict = m_pDocument->AddIndirectObject(gsDict);
    name = RealizeResource(pDict, "ExtGState");
    m_pObjHolder->GraphicsMapInsert(graphD, name);
  }
  *buf << "/" << PDF_NameEncode(name) << " gs ";
}

void CPDF_PageContentGenerator::ProcessDefaultGraphics(
    std::ostringstream* buf) {
  *buf << "0 0 0 RG 0 0 0 rg 1 w "
       << static_cast<int>(CFX_GraphStateData::LineCap::kButt) << " J "
       << static_cast<int>(CFX_GraphStateData::LineJoin::kMiter) << " j\n";
  ByteString name = GetOrCreateDefaultGraphics();
  *buf << "/" << PDF_NameEncode(name) << " gs ";
}

ByteString CPDF_PageContentGenerator::GetOrCreateDefaultGraphics() const {
  GraphicsData defaultGraphics;
  defaultGraphics.fillAlpha = 1.0f;
  defaultGraphics.strokeAlpha = 1.0f;
  defaultGraphics.blendType = BlendMode::kNormal;

  absl::optional<ByteString> maybe_name =
      m_pObjHolder->GraphicsMapSearch(defaultGraphics);
  if (maybe_name.has_value())
    return maybe_name.value();

  auto gsDict = pdfium::MakeRetain<CPDF_Dictionary>();
  gsDict->SetNewFor<CPDF_Number>("ca", defaultGraphics.fillAlpha);
  gsDict->SetNewFor<CPDF_Number>("CA", defaultGraphics.strokeAlpha);
  gsDict->SetNewFor<CPDF_Name>("BM", "Normal");
  CPDF_Object* pDict = m_pDocument->AddIndirectObject(gsDict);
  ByteString name = RealizeResource(pDict, "ExtGState");
  m_pObjHolder->GraphicsMapInsert(defaultGraphics, name);
  return name;
}

// This method adds text to the buffer, BT begins the text object, ET ends it.
// Tm sets the text matrix (allows positioning and transforming text).
// Tf sets the font name (from Font in Resources) and font size.
// Tr sets the text rendering mode.
// Tj sets the actual text, <####...> is used when specifying charcodes.
void CPDF_PageContentGenerator::ProcessText(std::ostringstream* buf,
                                            CPDF_TextObject* pTextObj) {
  ProcessGraphics(buf, pTextObj);
  *buf << "BT " << pTextObj->GetTextMatrix() << " Tm ";
  RetainPtr<CPDF_Font> pFont(pTextObj->GetFont());
  if (!pFont)
    pFont = CPDF_Font::GetStockFont(m_pDocument.Get(), "Helvetica");

  FontData data;
  const CPDF_FontEncoding* pEncoding = nullptr;
  if (pFont->IsType1Font()) {
    data.type = "Type1";
    pEncoding = pFont->AsType1Font()->GetEncoding();
  } else if (pFont->IsTrueTypeFont()) {
    data.type = "TrueType";
    pEncoding = pFont->AsTrueTypeFont()->GetEncoding();
  } else if (pFont->IsCIDFont()) {
    data.type = "Type0";
  } else {
    return;
  }
  data.baseFont = pFont->GetBaseFontName();

  ByteString dictName;
  absl::optional<ByteString> maybe_name = m_pObjHolder->FontsMapSearch(data);
  if (maybe_name.has_value()) {
    dictName = std::move(maybe_name.value());
  } else {
    CPDF_Object* pIndirectFont = pFont->GetFontDict();
    if (pIndirectFont->IsInline()) {
      // In this case we assume it must be a standard font
      auto pFontDict = pdfium::MakeRetain<CPDF_Dictionary>();
      pFontDict->SetNewFor<CPDF_Name>("Type", "Font");
      pFontDict->SetNewFor<CPDF_Name>("Subtype", data.type);
      pFontDict->SetNewFor<CPDF_Name>("BaseFont", data.baseFont);
      if (pEncoding) {
        pFontDict->SetFor("Encoding",
                          pEncoding->Realize(m_pDocument->GetByteStringPool()));
      }
      pIndirectFont = m_pDocument->AddIndirectObject(pFontDict);
    }
    dictName = RealizeResource(pIndirectFont, "Font");
    m_pObjHolder->FontsMapInsert(data, dictName);
  }
  *buf << "/" << PDF_NameEncode(dictName) << " ";
  WriteFloat(*buf, pTextObj->GetFontSize()) << " Tf ";
  *buf << static_cast<int>(pTextObj->GetTextRenderMode()) << " Tr ";
  ByteString text;
  for (uint32_t charcode : pTextObj->GetCharCodes()) {
    if (charcode != CPDF_Font::kInvalidCharCode)
      pFont->AppendChar(&text, charcode);
  }
  *buf << PDF_HexEncodeString(text.AsStringView()) << " Tj ET";
  *buf << " Q\n";
}
