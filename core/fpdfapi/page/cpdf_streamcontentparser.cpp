// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_streamcontentparser.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_allstates.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_meshstream.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_shadingobject.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_streamparser.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/autonuller.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/scoped_set_insertion.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/cxx17_backports.h"
#include "third_party/base/no_destructor.h"
#include "third_party/base/notreached.h"
#include "third_party/base/span.h"

namespace {

const int kMaxFormLevel = 40;

const int kSingleCoordinatePair = 1;
const int kTensorCoordinatePairs = 16;
const int kCoonsCoordinatePairs = 12;
const int kSingleColorPerPatch = 1;
const int kQuadColorsPerPatch = 4;

const char kPathOperatorSubpath = 'm';
const char kPathOperatorLine = 'l';
const char kPathOperatorCubicBezier1 = 'c';
const char kPathOperatorCubicBezier2 = 'v';
const char kPathOperatorCubicBezier3 = 'y';
const char kPathOperatorClosePath = 'h';
const char kPathOperatorRectangle[] = "re";

CFX_FloatRect GetShadingBBox(CPDF_ShadingPattern* pShading,
                             const CFX_Matrix& matrix) {
  ShadingType type = pShading->GetShadingType();
  const CPDF_Stream* pStream = ToStream(pShading->GetShadingObject());
  RetainPtr<CPDF_ColorSpace> pCS = pShading->GetCS();
  if (!pStream || !pCS)
    return CFX_FloatRect();

  CPDF_MeshStream stream(type, pShading->GetFuncs(), pStream, pCS);
  if (!stream.Load())
    return CFX_FloatRect();

  CFX_FloatRect rect;
  bool update_rect = false;
  bool bGouraud = type == kFreeFormGouraudTriangleMeshShading ||
                  type == kLatticeFormGouraudTriangleMeshShading;

  int point_count;
  if (type == kTensorProductPatchMeshShading)
    point_count = kTensorCoordinatePairs;
  else if (type == kCoonsPatchMeshShading)
    point_count = kCoonsCoordinatePairs;
  else
    point_count = kSingleCoordinatePair;

  int color_count;
  if (type == kCoonsPatchMeshShading || type == kTensorProductPatchMeshShading)
    color_count = kQuadColorsPerPatch;
  else
    color_count = kSingleColorPerPatch;

  while (!stream.BitStream()->IsEOF()) {
    uint32_t flag = 0;
    if (type != kLatticeFormGouraudTriangleMeshShading) {
      if (!stream.CanReadFlag())
        break;
      flag = stream.ReadFlag();
    }

    if (!bGouraud && flag) {
      point_count -= 4;
      color_count -= 2;
    }

    for (int i = 0; i < point_count; ++i) {
      if (!stream.CanReadCoords())
        break;

      CFX_PointF origin = stream.ReadCoords();
      if (update_rect) {
        rect.UpdateRect(origin);
      } else {
        rect = CFX_FloatRect(origin);
        update_rect = true;
      }
    }
    FX_SAFE_UINT32 nBits = stream.Components();
    nBits *= stream.ComponentBits();
    nBits *= color_count;
    if (!nBits.IsValid())
      break;

    stream.BitStream()->SkipBits(nBits.ValueOrDie());
    if (bGouraud)
      stream.BitStream()->ByteAlign();
  }
  return matrix.TransformRect(rect);
}

struct AbbrPair {
  const char* abbr;
  const char* full_name;
};

const AbbrPair kInlineKeyAbbr[] = {
    {"BPC", "BitsPerComponent"}, {"CS", "ColorSpace"}, {"D", "Decode"},
    {"DP", "DecodeParms"},       {"F", "Filter"},      {"H", "Height"},
    {"IM", "ImageMask"},         {"I", "Interpolate"}, {"W", "Width"},
};

const AbbrPair kInlineValueAbbr[] = {
    {"G", "DeviceGray"},       {"RGB", "DeviceRGB"},
    {"CMYK", "DeviceCMYK"},    {"I", "Indexed"},
    {"AHx", "ASCIIHexDecode"}, {"A85", "ASCII85Decode"},
    {"LZW", "LZWDecode"},      {"Fl", "FlateDecode"},
    {"RL", "RunLengthDecode"}, {"CCF", "CCITTFaxDecode"},
    {"DCT", "DCTDecode"},
};

struct AbbrReplacementOp {
  bool is_replace_key;
  ByteString key;
  ByteStringView replacement;
};

ByteStringView FindFullName(const AbbrPair* table,
                            size_t count,
                            ByteStringView abbr) {
  auto* it = std::find_if(table, table + count, [abbr](const AbbrPair& pair) {
    return pair.abbr == abbr;
  });
  return it != table + count ? ByteStringView(it->full_name) : ByteStringView();
}

void ReplaceAbbr(CPDF_Object* pObj);

void ReplaceAbbrInDictionary(CPDF_Dictionary* pDict) {
  std::vector<AbbrReplacementOp> replacements;
  {
    CPDF_DictionaryLocker locker(pDict);
    for (const auto& it : locker) {
      ByteString key = it.first;
      CPDF_Object* value = it.second.Get();
      ByteStringView fullname = FindFullName(
          kInlineKeyAbbr, pdfium::size(kInlineKeyAbbr), key.AsStringView());
      if (!fullname.IsEmpty()) {
        AbbrReplacementOp op;
        op.is_replace_key = true;
        op.key = std::move(key);
        op.replacement = fullname;
        replacements.push_back(op);
        key = fullname;
      }

      if (value->IsName()) {
        ByteString name = value->GetString();
        fullname =
            FindFullName(kInlineValueAbbr, pdfium::size(kInlineValueAbbr),
                         name.AsStringView());
        if (!fullname.IsEmpty()) {
          AbbrReplacementOp op;
          op.is_replace_key = false;
          op.key = key;
          op.replacement = fullname;
          replacements.push_back(op);
        }
      } else {
        ReplaceAbbr(value);
      }
    }
  }
  for (const auto& op : replacements) {
    if (op.is_replace_key)
      pDict->ReplaceKey(op.key, ByteString(op.replacement));
    else
      pDict->SetNewFor<CPDF_Name>(op.key, ByteString(op.replacement));
  }
}

void ReplaceAbbrInArray(CPDF_Array* pArray) {
  for (size_t i = 0; i < pArray->size(); ++i) {
    CPDF_Object* pElement = pArray->GetObjectAt(i);
    if (pElement->IsName()) {
      ByteString name = pElement->GetString();
      ByteStringView fullname =
          FindFullName(kInlineValueAbbr, pdfium::size(kInlineValueAbbr),
                       name.AsStringView());
      if (!fullname.IsEmpty())
        pArray->SetNewAt<CPDF_Name>(i, ByteString(fullname));
    } else {
      ReplaceAbbr(pElement);
    }
  }
}

void ReplaceAbbr(CPDF_Object* pObj) {
  CPDF_Dictionary* pDict = pObj->AsDictionary();
  if (pDict) {
    ReplaceAbbrInDictionary(pDict);
    return;
  }

  CPDF_Array* pArray = pObj->AsArray();
  if (pArray)
    ReplaceAbbrInArray(pArray);
}

}  // namespace

CPDF_StreamContentParser::CPDF_StreamContentParser(
    CPDF_Document* pDocument,
    CPDF_Dictionary* pPageResources,
    CPDF_Dictionary* pParentResources,
    const CFX_Matrix* pmtContentToUser,
    CPDF_PageObjectHolder* pObjHolder,
    CPDF_Dictionary* pResources,
    const CFX_FloatRect& rcBBox,
    const CPDF_AllStates* pStates,
    std::set<const uint8_t*>* pParsedSet)
    : m_pDocument(pDocument),
      m_pPageResources(pPageResources),
      m_pParentResources(pParentResources),
      m_pResources(CPDF_Form::ChooseResourcesDict(pResources,
                                                  pParentResources,
                                                  pPageResources)),
      m_pObjectHolder(pObjHolder),
      m_ParsedSet(pParsedSet),
      m_BBox(rcBBox),
      m_pCurStates(std::make_unique<CPDF_AllStates>()) {
  if (pmtContentToUser)
    m_mtContentToUser = *pmtContentToUser;
  if (pStates) {
    m_pCurStates->Copy(*pStates);
  } else {
    m_pCurStates->m_GeneralState.Emplace();
    m_pCurStates->m_GraphState.Emplace();
    m_pCurStates->m_TextState.Emplace();
    m_pCurStates->m_ColorState.Emplace();
  }

  // Add the sentinel.
  m_ContentMarksStack.push(std::make_unique<CPDF_ContentMarks>());
}

CPDF_StreamContentParser::~CPDF_StreamContentParser() {
  ClearAllParams();
}

int CPDF_StreamContentParser::GetNextParamPos() {
  if (m_ParamCount == kParamBufSize) {
    m_ParamStartPos++;
    if (m_ParamStartPos == kParamBufSize) {
      m_ParamStartPos = 0;
    }
    if (m_ParamBuf[m_ParamStartPos].m_Type == ContentParam::Type::kObject)
      m_ParamBuf[m_ParamStartPos].m_pObject.Reset();

    return m_ParamStartPos;
  }
  int index = m_ParamStartPos + m_ParamCount;
  if (index >= kParamBufSize) {
    index -= kParamBufSize;
  }
  m_ParamCount++;
  return index;
}

void CPDF_StreamContentParser::AddNameParam(ByteStringView bsName) {
  ContentParam& param = m_ParamBuf[GetNextParamPos()];
  param.m_Type = ContentParam::Type::kName;
  param.m_Name = PDF_NameDecode(bsName);
}

void CPDF_StreamContentParser::AddNumberParam(ByteStringView str) {
  ContentParam& param = m_ParamBuf[GetNextParamPos()];
  param.m_Type = ContentParam::Type::kNumber;
  param.m_Number = FX_Number(str);
}

void CPDF_StreamContentParser::AddObjectParam(RetainPtr<CPDF_Object> pObj) {
  ContentParam& param = m_ParamBuf[GetNextParamPos()];
  param.m_Type = ContentParam::Type::kObject;
  param.m_pObject = std::move(pObj);
}

void CPDF_StreamContentParser::ClearAllParams() {
  uint32_t index = m_ParamStartPos;
  for (uint32_t i = 0; i < m_ParamCount; i++) {
    if (m_ParamBuf[index].m_Type == ContentParam::Type::kObject)
      m_ParamBuf[index].m_pObject.Reset();
    index++;
    if (index == kParamBufSize)
      index = 0;
  }
  m_ParamStartPos = 0;
  m_ParamCount = 0;
}

CPDF_Object* CPDF_StreamContentParser::GetObject(uint32_t index) {
  if (index >= m_ParamCount) {
    return nullptr;
  }
  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= kParamBufSize) {
    real_index -= kParamBufSize;
  }
  ContentParam& param = m_ParamBuf[real_index];
  if (param.m_Type == ContentParam::Type::kNumber) {
    param.m_Type = ContentParam::Type::kObject;
    param.m_pObject =
        param.m_Number.IsInteger()
            ? pdfium::MakeRetain<CPDF_Number>(param.m_Number.GetSigned())
            : pdfium::MakeRetain<CPDF_Number>(param.m_Number.GetFloat());
    return param.m_pObject.Get();
  }
  if (param.m_Type == ContentParam::Type::kName) {
    param.m_Type = ContentParam::Type::kObject;
    param.m_pObject = m_pDocument->New<CPDF_Name>(param.m_Name);
    return param.m_pObject.Get();
  }
  if (param.m_Type == ContentParam::Type::kObject)
    return param.m_pObject.Get();

  NOTREACHED();
  return nullptr;
}

ByteString CPDF_StreamContentParser::GetString(uint32_t index) const {
  if (index >= m_ParamCount)
    return ByteString();

  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= kParamBufSize)
    real_index -= kParamBufSize;

  const ContentParam& param = m_ParamBuf[real_index];
  if (param.m_Type == ContentParam::Type::kName)
    return param.m_Name;

  if (param.m_Type == ContentParam::Type::kObject && param.m_pObject)
    return param.m_pObject->GetString();

  return ByteString();
}

float CPDF_StreamContentParser::GetNumber(uint32_t index) const {
  if (index >= m_ParamCount)
    return 0;

  int real_index = m_ParamStartPos + m_ParamCount - index - 1;
  if (real_index >= kParamBufSize)
    real_index -= kParamBufSize;

  const ContentParam& param = m_ParamBuf[real_index];
  if (param.m_Type == ContentParam::Type::kNumber)
    return param.m_Number.GetFloat();

  if (param.m_Type == ContentParam::Type::kObject && param.m_pObject)
    return param.m_pObject->GetNumber();

  return 0;
}

std::vector<float> CPDF_StreamContentParser::GetNumbers(size_t count) const {
  std::vector<float> values(count);
  for (size_t i = 0; i < count; ++i)
    values[i] = GetNumber(count - i - 1);
  return values;
}

CFX_PointF CPDF_StreamContentParser::GetPoint(uint32_t index) const {
  return CFX_PointF(GetNumber(index + 1), GetNumber(index));
}

CFX_Matrix CPDF_StreamContentParser::GetMatrix() const {
  return CFX_Matrix(GetNumber(5), GetNumber(4), GetNumber(3), GetNumber(2),
                    GetNumber(1), GetNumber(0));
}

void CPDF_StreamContentParser::SetGraphicStates(CPDF_PageObject* pObj,
                                                bool bColor,
                                                bool bText,
                                                bool bGraph) {
  pObj->m_GeneralState = m_pCurStates->m_GeneralState;
  pObj->m_ClipPath = m_pCurStates->m_ClipPath;
  pObj->SetContentMarks(*m_ContentMarksStack.top());
  if (bColor) {
    pObj->m_ColorState = m_pCurStates->m_ColorState;
  }
  if (bGraph) {
    pObj->m_GraphState = m_pCurStates->m_GraphState;
  }
  if (bText) {
    pObj->m_TextState = m_pCurStates->m_TextState;
  }
}

// static
CPDF_StreamContentParser::OpCodes
CPDF_StreamContentParser::InitializeOpCodes() {
  return OpCodes({
      {FXBSTR_ID('"', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_NextLineShowText_Space},
      {FXBSTR_ID('\'', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_NextLineShowText},
      {FXBSTR_ID('B', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_FillStrokePath},
      {FXBSTR_ID('B', '*', 0, 0),
       &CPDF_StreamContentParser::Handle_EOFillStrokePath},
      {FXBSTR_ID('B', 'D', 'C', 0),
       &CPDF_StreamContentParser::Handle_BeginMarkedContent_Dictionary},
      {FXBSTR_ID('B', 'I', 0, 0), &CPDF_StreamContentParser::Handle_BeginImage},
      {FXBSTR_ID('B', 'M', 'C', 0),
       &CPDF_StreamContentParser::Handle_BeginMarkedContent},
      {FXBSTR_ID('B', 'T', 0, 0), &CPDF_StreamContentParser::Handle_BeginText},
      {FXBSTR_ID('C', 'S', 0, 0),
       &CPDF_StreamContentParser::Handle_SetColorSpace_Stroke},
      {FXBSTR_ID('D', 'P', 0, 0),
       &CPDF_StreamContentParser::Handle_MarkPlace_Dictionary},
      {FXBSTR_ID('D', 'o', 0, 0),
       &CPDF_StreamContentParser::Handle_ExecuteXObject},
      {FXBSTR_ID('E', 'I', 0, 0), &CPDF_StreamContentParser::Handle_EndImage},
      {FXBSTR_ID('E', 'M', 'C', 0),
       &CPDF_StreamContentParser::Handle_EndMarkedContent},
      {FXBSTR_ID('E', 'T', 0, 0), &CPDF_StreamContentParser::Handle_EndText},
      {FXBSTR_ID('F', 0, 0, 0), &CPDF_StreamContentParser::Handle_FillPathOld},
      {FXBSTR_ID('G', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_SetGray_Stroke},
      {FXBSTR_ID('I', 'D', 0, 0),
       &CPDF_StreamContentParser::Handle_BeginImageData},
      {FXBSTR_ID('J', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineCap},
      {FXBSTR_ID('K', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_SetCMYKColor_Stroke},
      {FXBSTR_ID('M', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_SetMiterLimit},
      {FXBSTR_ID('M', 'P', 0, 0), &CPDF_StreamContentParser::Handle_MarkPlace},
      {FXBSTR_ID('Q', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_RestoreGraphState},
      {FXBSTR_ID('R', 'G', 0, 0),
       &CPDF_StreamContentParser::Handle_SetRGBColor_Stroke},
      {FXBSTR_ID('S', 0, 0, 0), &CPDF_StreamContentParser::Handle_StrokePath},
      {FXBSTR_ID('S', 'C', 0, 0),
       &CPDF_StreamContentParser::Handle_SetColor_Stroke},
      {FXBSTR_ID('S', 'C', 'N', 0),
       &CPDF_StreamContentParser::Handle_SetColorPS_Stroke},
      {FXBSTR_ID('T', '*', 0, 0),
       &CPDF_StreamContentParser::Handle_MoveToNextLine},
      {FXBSTR_ID('T', 'D', 0, 0),
       &CPDF_StreamContentParser::Handle_MoveTextPoint_SetLeading},
      {FXBSTR_ID('T', 'J', 0, 0),
       &CPDF_StreamContentParser::Handle_ShowText_Positioning},
      {FXBSTR_ID('T', 'L', 0, 0),
       &CPDF_StreamContentParser::Handle_SetTextLeading},
      {FXBSTR_ID('T', 'c', 0, 0),
       &CPDF_StreamContentParser::Handle_SetCharSpace},
      {FXBSTR_ID('T', 'd', 0, 0),
       &CPDF_StreamContentParser::Handle_MoveTextPoint},
      {FXBSTR_ID('T', 'f', 0, 0), &CPDF_StreamContentParser::Handle_SetFont},
      {FXBSTR_ID('T', 'j', 0, 0), &CPDF_StreamContentParser::Handle_ShowText},
      {FXBSTR_ID('T', 'm', 0, 0),
       &CPDF_StreamContentParser::Handle_SetTextMatrix},
      {FXBSTR_ID('T', 'r', 0, 0),
       &CPDF_StreamContentParser::Handle_SetTextRenderMode},
      {FXBSTR_ID('T', 's', 0, 0),
       &CPDF_StreamContentParser::Handle_SetTextRise},
      {FXBSTR_ID('T', 'w', 0, 0),
       &CPDF_StreamContentParser::Handle_SetWordSpace},
      {FXBSTR_ID('T', 'z', 0, 0),
       &CPDF_StreamContentParser::Handle_SetHorzScale},
      {FXBSTR_ID('W', 0, 0, 0), &CPDF_StreamContentParser::Handle_Clip},
      {FXBSTR_ID('W', '*', 0, 0), &CPDF_StreamContentParser::Handle_EOClip},
      {FXBSTR_ID('b', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_CloseFillStrokePath},
      {FXBSTR_ID('b', '*', 0, 0),
       &CPDF_StreamContentParser::Handle_CloseEOFillStrokePath},
      {FXBSTR_ID('c', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_123},
      {FXBSTR_ID('c', 'm', 0, 0),
       &CPDF_StreamContentParser::Handle_ConcatMatrix},
      {FXBSTR_ID('c', 's', 0, 0),
       &CPDF_StreamContentParser::Handle_SetColorSpace_Fill},
      {FXBSTR_ID('d', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetDash},
      {FXBSTR_ID('d', '0', 0, 0),
       &CPDF_StreamContentParser::Handle_SetCharWidth},
      {FXBSTR_ID('d', '1', 0, 0),
       &CPDF_StreamContentParser::Handle_SetCachedDevice},
      {FXBSTR_ID('f', 0, 0, 0), &CPDF_StreamContentParser::Handle_FillPath},
      {FXBSTR_ID('f', '*', 0, 0), &CPDF_StreamContentParser::Handle_EOFillPath},
      {FXBSTR_ID('g', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetGray_Fill},
      {FXBSTR_ID('g', 's', 0, 0),
       &CPDF_StreamContentParser::Handle_SetExtendGraphState},
      {FXBSTR_ID('h', 0, 0, 0), &CPDF_StreamContentParser::Handle_ClosePath},
      {FXBSTR_ID('i', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetFlat},
      {FXBSTR_ID('j', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineJoin},
      {FXBSTR_ID('k', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_SetCMYKColor_Fill},
      {FXBSTR_ID('l', 0, 0, 0), &CPDF_StreamContentParser::Handle_LineTo},
      {FXBSTR_ID('m', 0, 0, 0), &CPDF_StreamContentParser::Handle_MoveTo},
      {FXBSTR_ID('n', 0, 0, 0), &CPDF_StreamContentParser::Handle_EndPath},
      {FXBSTR_ID('q', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_SaveGraphState},
      {FXBSTR_ID('r', 'e', 0, 0), &CPDF_StreamContentParser::Handle_Rectangle},
      {FXBSTR_ID('r', 'g', 0, 0),
       &CPDF_StreamContentParser::Handle_SetRGBColor_Fill},
      {FXBSTR_ID('r', 'i', 0, 0),
       &CPDF_StreamContentParser::Handle_SetRenderIntent},
      {FXBSTR_ID('s', 0, 0, 0),
       &CPDF_StreamContentParser::Handle_CloseStrokePath},
      {FXBSTR_ID('s', 'c', 0, 0),
       &CPDF_StreamContentParser::Handle_SetColor_Fill},
      {FXBSTR_ID('s', 'c', 'n', 0),
       &CPDF_StreamContentParser::Handle_SetColorPS_Fill},
      {FXBSTR_ID('s', 'h', 0, 0), &CPDF_StreamContentParser::Handle_ShadeFill},
      {FXBSTR_ID('v', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_23},
      {FXBSTR_ID('w', 0, 0, 0), &CPDF_StreamContentParser::Handle_SetLineWidth},
      {FXBSTR_ID('y', 0, 0, 0), &CPDF_StreamContentParser::Handle_CurveTo_13},
  });
}

void CPDF_StreamContentParser::OnOperator(ByteStringView op) {
  static const pdfium::base::NoDestructor<OpCodes> s_OpCodes(
      InitializeOpCodes());

  auto it = s_OpCodes->find(op.GetID());
  if (it != s_OpCodes->end())
    (this->*it->second)();
}

void CPDF_StreamContentParser::Handle_CloseFillStrokePath() {
  Handle_ClosePath();
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_FillStrokePath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_CloseEOFillStrokePath() {
  AddPathPointAndClose(m_PathStart, CFX_Path::Point::Type::kLine);
  AddPathObject(CFX_FillRenderOptions::FillType::kEvenOdd, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_EOFillStrokePath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kEvenOdd, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_BeginMarkedContent_Dictionary() {
  CPDF_Object* pProperty = GetObject(0);
  if (!pProperty)
    return;

  ByteString tag = GetString(1);
  std::unique_ptr<CPDF_ContentMarks> new_marks =
      m_ContentMarksStack.top()->Clone();

  if (pProperty->IsName()) {
    ByteString property_name = pProperty->GetString();
    CPDF_Dictionary* pHolder = FindResourceHolder("Properties");
    if (!pHolder || !pHolder->GetDictFor(property_name))
      return;
    new_marks->AddMarkWithPropertiesHolder(tag, pHolder, property_name);
  } else if (pProperty->IsDictionary()) {
    new_marks->AddMarkWithDirectDict(tag, pProperty->AsDictionary());
  } else {
    return;
  }
  m_ContentMarksStack.push(std::move(new_marks));
}

void CPDF_StreamContentParser::Handle_BeginImage() {
  FX_FILESIZE savePos = m_pSyntax->GetPos();
  auto pDict = m_pDocument->New<CPDF_Dictionary>();
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    if (type == CPDF_StreamParser::Keyword) {
      if (m_pSyntax->GetWord() != "ID") {
        m_pSyntax->SetPos(savePos);
        return;
      }
    }
    if (type != CPDF_StreamParser::Name) {
      break;
    }
    auto word = m_pSyntax->GetWord();
    ByteString key(word.Last(word.GetLength() - 1));
    auto pObj = m_pSyntax->ReadNextObject(false, false, 0);
    if (CPDF_Dictionary::IsValidKey(key)) {
      if (pObj && !pObj->IsInline()) {
        pDict->SetNewFor<CPDF_Reference>(key, m_pDocument.Get(),
                                         pObj->GetObjNum());
      } else {
        pDict->SetFor(key, std::move(pObj));
      }
    }
  }
  ReplaceAbbr(pDict.Get());
  CPDF_Object* pCSObj = nullptr;
  if (pDict->KeyExist("ColorSpace")) {
    pCSObj = pDict->GetDirectObjectFor("ColorSpace");
    if (pCSObj->IsName()) {
      ByteString name = pCSObj->GetString();
      if (name != "DeviceRGB" && name != "DeviceGray" && name != "DeviceCMYK") {
        pCSObj = FindResourceObj("ColorSpace", name);
        if (pCSObj && pCSObj->IsInline())
          pDict->SetFor("ColorSpace", pCSObj->Clone());
      }
    }
  }
  pDict->SetNewFor<CPDF_Name>("Subtype", "Image");
  RetainPtr<CPDF_Stream> pStream =
      m_pSyntax->ReadInlineStream(m_pDocument.Get(), std::move(pDict), pCSObj);
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    if (type == CPDF_StreamParser::EndOfData) {
      break;
    }
    if (type != CPDF_StreamParser::Keyword) {
      continue;
    }
    if (m_pSyntax->GetWord() == "EI") {
      break;
    }
  }
  CPDF_ImageObject* pObj = AddImage(std::move(pStream));
  // Record the bounding box of this image, so rendering code can draw it
  // properly.
  if (pObj && pObj->GetImage()->IsMask())
    m_pObjectHolder->AddImageMaskBoundingBox(pObj->GetRect());
}

void CPDF_StreamContentParser::Handle_BeginMarkedContent() {
  std::unique_ptr<CPDF_ContentMarks> new_marks =
      m_ContentMarksStack.top()->Clone();
  new_marks->AddMark(GetString(0));
  m_ContentMarksStack.push(std::move(new_marks));
}

void CPDF_StreamContentParser::Handle_BeginText() {
  m_pCurStates->m_TextMatrix = CFX_Matrix();
  OnChangeTextMatrix();
  m_pCurStates->m_TextPos = CFX_PointF();
  m_pCurStates->m_TextLinePos = CFX_PointF();
}

void CPDF_StreamContentParser::Handle_CurveTo_123() {
  AddPathPoint(GetPoint(4), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(2), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
}

void CPDF_StreamContentParser::Handle_ConcatMatrix() {
  m_pCurStates->m_CTM = GetMatrix() * m_pCurStates->m_CTM;
  OnChangeTextMatrix();
}

void CPDF_StreamContentParser::Handle_SetColorSpace_Fill() {
  RetainPtr<CPDF_ColorSpace> pCS = FindColorSpace(GetString(0));
  if (!pCS)
    return;

  m_pCurStates->m_ColorState.GetMutableFillColor()->SetColorSpace(pCS);
}

void CPDF_StreamContentParser::Handle_SetColorSpace_Stroke() {
  RetainPtr<CPDF_ColorSpace> pCS = FindColorSpace(GetString(0));
  if (!pCS)
    return;

  m_pCurStates->m_ColorState.GetMutableStrokeColor()->SetColorSpace(pCS);
}

void CPDF_StreamContentParser::Handle_SetDash() {
  CPDF_Array* pArray = ToArray(GetObject(1));
  if (!pArray)
    return;

  m_pCurStates->SetLineDash(pArray, GetNumber(0), 1.0f);
}

void CPDF_StreamContentParser::Handle_SetCharWidth() {
  m_Type3Data[0] = GetNumber(1);
  m_Type3Data[1] = GetNumber(0);
  m_bColored = true;
}

void CPDF_StreamContentParser::Handle_SetCachedDevice() {
  for (int i = 0; i < 6; i++) {
    m_Type3Data[i] = GetNumber(5 - i);
  }
  m_bColored = false;
}

void CPDF_StreamContentParser::Handle_ExecuteXObject() {
  ByteString name = GetString(0);
  if (name == m_LastImageName && m_pLastImage && m_pLastImage->GetStream() &&
      m_pLastImage->GetStream()->GetObjNum()) {
    CPDF_ImageObject* pObj = AddImage(m_pLastImage);
    // Record the bounding box of this image, so rendering code can draw it
    // properly.
    if (pObj && pObj->GetImage()->IsMask())
      m_pObjectHolder->AddImageMaskBoundingBox(pObj->GetRect());
    return;
  }

  CPDF_Stream* pXObject = ToStream(FindResourceObj("XObject", name));
  if (!pXObject) {
    m_bResourceMissing = true;
    return;
  }

  ByteString type;
  if (pXObject->GetDict())
    type = pXObject->GetDict()->GetStringFor("Subtype");

  if (type == "Form") {
    AddForm(pXObject);
    return;
  }

  if (type == "Image") {
    CPDF_ImageObject* pObj = pXObject->IsInline()
                                 ? AddImage(ToStream(pXObject->Clone()))
                                 : AddImage(pXObject->GetObjNum());

    m_LastImageName = std::move(name);
    if (pObj) {
      m_pLastImage = pObj->GetImage();
      if (m_pLastImage->IsMask())
        m_pObjectHolder->AddImageMaskBoundingBox(pObj->GetRect());
    }
  }
}

void CPDF_StreamContentParser::AddForm(CPDF_Stream* pStream) {
  CPDF_AllStates status;
  status.m_GeneralState = m_pCurStates->m_GeneralState;
  status.m_GraphState = m_pCurStates->m_GraphState;
  status.m_ColorState = m_pCurStates->m_ColorState;
  status.m_TextState = m_pCurStates->m_TextState;
  auto form = std::make_unique<CPDF_Form>(
      m_pDocument.Get(), m_pPageResources.Get(), pStream, m_pResources.Get());
  form->ParseContent(&status, nullptr, m_ParsedSet.Get());

  CFX_Matrix matrix = m_pCurStates->m_CTM * m_mtContentToUser;

  auto pFormObj = std::make_unique<CPDF_FormObject>(GetCurrentStreamIndex(),
                                                    std::move(form), matrix);
  if (!m_pObjectHolder->BackgroundAlphaNeeded() &&
      pFormObj->form()->BackgroundAlphaNeeded()) {
    m_pObjectHolder->SetBackgroundAlphaNeeded(true);
  }
  pFormObj->CalcBoundingBox();
  SetGraphicStates(pFormObj.get(), true, true, true);
  m_pObjectHolder->AppendPageObject(std::move(pFormObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImage(
    RetainPtr<CPDF_Stream> pStream) {
  if (!pStream)
    return nullptr;

  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetImage(
      pdfium::MakeRetain<CPDF_Image>(m_pDocument.Get(), std::move(pStream)));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImage(uint32_t streamObjNum) {
  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetImage(CPDF_DocPageData::FromDocument(m_pDocument.Get())
                          ->GetImage(streamObjNum));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImage(
    const RetainPtr<CPDF_Image>& pImage) {
  if (!pImage)
    return nullptr;

  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetImage(CPDF_DocPageData::FromDocument(m_pDocument.Get())
                          ->GetImage(pImage->GetStream()->GetObjNum()));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImageObject(
    std::unique_ptr<CPDF_ImageObject> pImageObj) {
  SetGraphicStates(pImageObj.get(), pImageObj->GetImage()->IsMask(), false,
                   false);

  CFX_Matrix ImageMatrix = m_pCurStates->m_CTM * m_mtContentToUser;
  pImageObj->SetImageMatrix(ImageMatrix);

  CPDF_ImageObject* pRet = pImageObj.get();
  m_pObjectHolder->AppendPageObject(std::move(pImageObj));
  return pRet;
}

std::vector<float> CPDF_StreamContentParser::GetColors() const {
  DCHECK(m_ParamCount > 0);
  return GetNumbers(m_ParamCount);
}

std::vector<float> CPDF_StreamContentParser::GetNamedColors() const {
  DCHECK(m_ParamCount > 0);
  const uint32_t nvalues = m_ParamCount - 1;
  std::vector<float> values(nvalues);
  for (size_t i = 0; i < nvalues; ++i)
    values[i] = GetNumber(m_ParamCount - i - 1);
  return values;
}

void CPDF_StreamContentParser::Handle_MarkPlace_Dictionary() {}

void CPDF_StreamContentParser::Handle_EndImage() {}

void CPDF_StreamContentParser::Handle_EndMarkedContent() {
  // First element is a sentinel, so do not pop it, ever. This may come up if
  // the EMCs are mismatched with the BMC/BDCs.
  if (m_ContentMarksStack.size() > 1)
    m_ContentMarksStack.pop();
}

void CPDF_StreamContentParser::Handle_EndText() {
  if (m_ClipTextList.empty())
    return;

  if (TextRenderingModeIsClipMode(m_pCurStates->m_TextState.GetTextMode()))
    m_pCurStates->m_ClipPath.AppendTexts(&m_ClipTextList);

  m_ClipTextList.clear();
}

void CPDF_StreamContentParser::Handle_FillPath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kFill);
}

void CPDF_StreamContentParser::Handle_FillPathOld() {
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kFill);
}

void CPDF_StreamContentParser::Handle_EOFillPath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kEvenOdd, RenderType::kFill);
}

void CPDF_StreamContentParser::Handle_SetGray_Fill() {
  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray);
  m_pCurStates->m_ColorState.SetFillColor(pCS, GetNumbers(1));
}

void CPDF_StreamContentParser::Handle_SetGray_Stroke() {
  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, GetNumbers(1));
}

void CPDF_StreamContentParser::Handle_SetExtendGraphState() {
  ByteString name = GetString(0);
  CPDF_Dictionary* pGS = ToDictionary(FindResourceObj("ExtGState", name));
  if (!pGS) {
    m_bResourceMissing = true;
    return;
  }
  m_pCurStates->ProcessExtGS(pGS, this);
}

void CPDF_StreamContentParser::Handle_ClosePath() {
  if (m_PathPoints.empty())
    return;

  if (m_PathStart.x != m_PathCurrent.x || m_PathStart.y != m_PathCurrent.y) {
    AddPathPointAndClose(m_PathStart, CFX_Path::Point::Type::kLine);
  } else if (m_PathPoints.back().m_Type != CFX_Path::Point::Type::kMove) {
    m_PathPoints.back().m_CloseFigure = true;
  }
}

void CPDF_StreamContentParser::Handle_SetFlat() {
  m_pCurStates->m_GeneralState.SetFlatness(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_BeginImageData() {}

void CPDF_StreamContentParser::Handle_SetLineJoin() {
  m_pCurStates->m_GraphState.SetLineJoin(
      static_cast<CFX_GraphStateData::LineJoin>(GetInteger(0)));
}

void CPDF_StreamContentParser::Handle_SetLineCap() {
  m_pCurStates->m_GraphState.SetLineCap(
      static_cast<CFX_GraphStateData::LineCap>(GetInteger(0)));
}

void CPDF_StreamContentParser::Handle_SetCMYKColor_Fill() {
  if (m_ParamCount != 4)
    return;

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK);
  m_pCurStates->m_ColorState.SetFillColor(pCS, GetNumbers(4));
}

void CPDF_StreamContentParser::Handle_SetCMYKColor_Stroke() {
  if (m_ParamCount != 4)
    return;

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, GetNumbers(4));
}

void CPDF_StreamContentParser::Handle_LineTo() {
  if (m_ParamCount != 2)
    return;

  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kLine);
}

void CPDF_StreamContentParser::Handle_MoveTo() {
  if (m_ParamCount != 2)
    return;

  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kMove);
  ParsePathObject();
}

void CPDF_StreamContentParser::Handle_SetMiterLimit() {
  m_pCurStates->m_GraphState.SetMiterLimit(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_MarkPlace() {}

void CPDF_StreamContentParser::Handle_EndPath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kNoFill, RenderType::kFill);
}

void CPDF_StreamContentParser::Handle_SaveGraphState() {
  auto pStates = std::make_unique<CPDF_AllStates>();
  pStates->Copy(*m_pCurStates);
  m_StateStack.push_back(std::move(pStates));
}

void CPDF_StreamContentParser::Handle_RestoreGraphState() {
  if (m_StateStack.empty())
    return;
  std::unique_ptr<CPDF_AllStates> pStates = std::move(m_StateStack.back());
  m_StateStack.pop_back();
  m_pCurStates->Copy(*pStates);
}

void CPDF_StreamContentParser::Handle_Rectangle() {
  float x = GetNumber(3);
  float y = GetNumber(2);
  float w = GetNumber(1);
  float h = GetNumber(0);
  AddPathRect(x, y, w, h);
}

void CPDF_StreamContentParser::AddPathRect(float x, float y, float w, float h) {
  AddPathPoint({x, y}, CFX_Path::Point::Type::kMove);
  AddPathPoint({x + w, y}, CFX_Path::Point::Type::kLine);
  AddPathPoint({x + w, y + h}, CFX_Path::Point::Type::kLine);
  AddPathPoint({x, y + h}, CFX_Path::Point::Type::kLine);
  AddPathPointAndClose({x, y}, CFX_Path::Point::Type::kLine);
}

void CPDF_StreamContentParser::Handle_SetRGBColor_Fill() {
  if (m_ParamCount != 3)
    return;

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);
  m_pCurStates->m_ColorState.SetFillColor(pCS, GetNumbers(3));
}

void CPDF_StreamContentParser::Handle_SetRGBColor_Stroke() {
  if (m_ParamCount != 3)
    return;

  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);
  m_pCurStates->m_ColorState.SetStrokeColor(pCS, GetNumbers(3));
}

void CPDF_StreamContentParser::Handle_SetRenderIntent() {}

void CPDF_StreamContentParser::Handle_CloseStrokePath() {
  Handle_ClosePath();
  AddPathObject(CFX_FillRenderOptions::FillType::kNoFill, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_StrokePath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kNoFill, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_SetColor_Fill() {
  int nargs = std::min(m_ParamCount, 4U);
  m_pCurStates->m_ColorState.SetFillColor(nullptr, GetNumbers(nargs));
}

void CPDF_StreamContentParser::Handle_SetColor_Stroke() {
  int nargs = std::min(m_ParamCount, 4U);
  m_pCurStates->m_ColorState.SetStrokeColor(nullptr, GetNumbers(nargs));
}

void CPDF_StreamContentParser::Handle_SetColorPS_Fill() {
  CPDF_Object* pLastParam = GetObject(0);
  if (!pLastParam)
    return;

  if (!pLastParam->IsName()) {
    m_pCurStates->m_ColorState.SetFillColor(nullptr, GetColors());
    return;
  }

  // A valid |pLastParam| implies |m_ParamCount| > 0, so GetNamedColors() call
  // below is safe.
  RetainPtr<CPDF_Pattern> pPattern = FindPattern(GetString(0));
  if (pPattern)
    m_pCurStates->m_ColorState.SetFillPattern(pPattern, GetNamedColors());
}

void CPDF_StreamContentParser::Handle_SetColorPS_Stroke() {
  CPDF_Object* pLastParam = GetObject(0);
  if (!pLastParam)
    return;

  if (!pLastParam->IsName()) {
    m_pCurStates->m_ColorState.SetStrokeColor(nullptr, GetColors());
    return;
  }

  // A valid |pLastParam| implies |m_ParamCount| > 0, so GetNamedColors() call
  // below is safe.
  RetainPtr<CPDF_Pattern> pPattern = FindPattern(GetString(0));
  if (pPattern)
    m_pCurStates->m_ColorState.SetStrokePattern(pPattern, GetNamedColors());
}

void CPDF_StreamContentParser::Handle_ShadeFill() {
  RetainPtr<CPDF_ShadingPattern> pShading = FindShading(GetString(0));
  if (!pShading)
    return;

  if (!pShading->IsShadingObject() || !pShading->Load())
    return;

  CFX_Matrix matrix = m_pCurStates->m_CTM * m_mtContentToUser;
  auto pObj = std::make_unique<CPDF_ShadingObject>(GetCurrentStreamIndex(),
                                                   pShading.Get(), matrix);
  SetGraphicStates(pObj.get(), false, false, false);
  CFX_FloatRect bbox =
      pObj->m_ClipPath.HasRef() ? pObj->m_ClipPath.GetClipBox() : m_BBox;
  if (pShading->IsMeshShading())
    bbox.Intersect(GetShadingBBox(pShading.Get(), pObj->matrix()));
  pObj->SetRect(bbox);
  m_pObjectHolder->AppendPageObject(std::move(pObj));
}

void CPDF_StreamContentParser::Handle_SetCharSpace() {
  m_pCurStates->m_TextState.SetCharSpace(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_MoveTextPoint() {
  m_pCurStates->m_TextLinePos += GetPoint(0);
  m_pCurStates->m_TextPos = m_pCurStates->m_TextLinePos;
}

void CPDF_StreamContentParser::Handle_MoveTextPoint_SetLeading() {
  Handle_MoveTextPoint();
  m_pCurStates->m_TextLeading = -GetNumber(0);
}

void CPDF_StreamContentParser::Handle_SetFont() {
  float fs = GetNumber(0);
  if (fs == 0) {
    constexpr float kDefaultFontSize = 0.0f;
    fs = kDefaultFontSize;
  }

  m_pCurStates->m_TextState.SetFontSize(fs);
  RetainPtr<CPDF_Font> pFont = FindFont(GetString(1));
  if (pFont)
    m_pCurStates->m_TextState.SetFont(pFont);
}

CPDF_Dictionary* CPDF_StreamContentParser::FindResourceHolder(
    const ByteString& type) {
  if (!m_pResources)
    return nullptr;

  CPDF_Dictionary* pDict = m_pResources->GetDictFor(type);
  if (pDict)
    return pDict;

  if (m_pResources == m_pPageResources || !m_pPageResources)
    return nullptr;

  return m_pPageResources->GetDictFor(type);
}

CPDF_Object* CPDF_StreamContentParser::FindResourceObj(const ByteString& type,
                                                       const ByteString& name) {
  CPDF_Dictionary* pHolder = FindResourceHolder(type);
  return pHolder ? pHolder->GetDirectObjectFor(name) : nullptr;
}

RetainPtr<CPDF_Font> CPDF_StreamContentParser::FindFont(
    const ByteString& name) {
  CPDF_Dictionary* pFontDict = ToDictionary(FindResourceObj("Font", name));
  if (!pFontDict) {
    m_bResourceMissing = true;
    return CPDF_Font::GetStockFont(m_pDocument.Get(),
                                   CFX_Font::kDefaultAnsiFontName);
  }
  RetainPtr<CPDF_Font> pFont =
      CPDF_DocPageData::FromDocument(m_pDocument.Get())->GetFont(pFontDict);
  if (pFont && pFont->IsType3Font()) {
    pFont->AsType3Font()->SetPageResources(m_pResources.Get());
    pFont->AsType3Font()->CheckType3FontMetrics();
  }
  return pFont;
}

RetainPtr<CPDF_ColorSpace> CPDF_StreamContentParser::FindColorSpace(
    const ByteString& name) {
  if (name == "Pattern")
    return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kPattern);

  if (name == "DeviceGray" || name == "DeviceCMYK" || name == "DeviceRGB") {
    ByteString defname = "Default";
    defname += name.Last(name.GetLength() - 7);
    const CPDF_Object* pDefObj = FindResourceObj("ColorSpace", defname);
    if (!pDefObj) {
      if (name == "DeviceGray") {
        return CPDF_ColorSpace::GetStockCS(
            CPDF_ColorSpace::Family::kDeviceGray);
      }
      if (name == "DeviceRGB")
        return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);

      return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK);
    }
    return CPDF_DocPageData::FromDocument(m_pDocument.Get())
        ->GetColorSpace(pDefObj, nullptr);
  }
  const CPDF_Object* pCSObj = FindResourceObj("ColorSpace", name);
  if (!pCSObj) {
    m_bResourceMissing = true;
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(m_pDocument.Get())
      ->GetColorSpace(pCSObj, nullptr);
}

RetainPtr<CPDF_Pattern> CPDF_StreamContentParser::FindPattern(
    const ByteString& name) {
  CPDF_Object* pPattern = FindResourceObj("Pattern", name);
  if (!pPattern || (!pPattern->IsDictionary() && !pPattern->IsStream())) {
    m_bResourceMissing = true;
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(m_pDocument.Get())
      ->GetPattern(pPattern, m_pCurStates->m_ParentMatrix);
}

RetainPtr<CPDF_ShadingPattern> CPDF_StreamContentParser::FindShading(
    const ByteString& name) {
  CPDF_Object* pPattern = FindResourceObj("Shading", name);
  if (!pPattern || (!pPattern->IsDictionary() && !pPattern->IsStream())) {
    m_bResourceMissing = true;
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(m_pDocument.Get())
      ->GetShading(pPattern, m_pCurStates->m_ParentMatrix);
}

void CPDF_StreamContentParser::AddTextObject(const ByteString* pStrs,
                                             float fInitKerning,
                                             const std::vector<float>& kernings,
                                             size_t nSegs) {
  RetainPtr<CPDF_Font> pFont = m_pCurStates->m_TextState.GetFont();
  if (!pFont)
    return;

  if (fInitKerning != 0) {
    if (pFont->IsVertWriting())
      m_pCurStates->m_TextPos.y -= GetVerticalTextSize(fInitKerning);
    else
      m_pCurStates->m_TextPos.x -= GetHorizontalTextSize(fInitKerning);
  }
  if (nSegs == 0)
    return;

  const TextRenderingMode text_mode =
      pFont->IsType3Font() ? TextRenderingMode::MODE_FILL
                           : m_pCurStates->m_TextState.GetTextMode();
  {
    auto pText = std::make_unique<CPDF_TextObject>(GetCurrentStreamIndex());
    m_pLastTextObject = pText.get();
    SetGraphicStates(pText.get(), true, true, true);
    if (TextRenderingModeIsStrokeMode(text_mode)) {
      float* pCTM = pText->m_TextState.GetMutableCTM();
      pCTM[0] = m_pCurStates->m_CTM.a;
      pCTM[1] = m_pCurStates->m_CTM.c;
      pCTM[2] = m_pCurStates->m_CTM.b;
      pCTM[3] = m_pCurStates->m_CTM.d;
    }
    pText->SetSegments(pStrs, kernings, nSegs);
    pText->SetPosition(
        m_mtContentToUser.Transform(m_pCurStates->m_CTM.Transform(
            m_pCurStates->m_TextMatrix.Transform(CFX_PointF(
                m_pCurStates->m_TextPos.x,
                m_pCurStates->m_TextPos.y + m_pCurStates->m_TextRise)))));

    m_pCurStates->m_TextPos +=
        pText->CalcPositionData(m_pCurStates->m_TextHorzScale);
    if (TextRenderingModeIsClipMode(text_mode))
      m_ClipTextList.push_back(pText->Clone());
    m_pObjectHolder->AppendPageObject(std::move(pText));
  }
  if (!kernings.empty() && kernings[nSegs - 1] != 0) {
    if (pFont->IsVertWriting())
      m_pCurStates->m_TextPos.y -= GetVerticalTextSize(kernings[nSegs - 1]);
    else
      m_pCurStates->m_TextPos.x -= GetHorizontalTextSize(kernings[nSegs - 1]);
  }
}

float CPDF_StreamContentParser::GetHorizontalTextSize(float fKerning) const {
  return GetVerticalTextSize(fKerning) * m_pCurStates->m_TextHorzScale;
}

float CPDF_StreamContentParser::GetVerticalTextSize(float fKerning) const {
  return fKerning * m_pCurStates->m_TextState.GetFontSize() / 1000;
}

int32_t CPDF_StreamContentParser::GetCurrentStreamIndex() {
  auto it =
      std::upper_bound(m_StreamStartOffsets.begin(), m_StreamStartOffsets.end(),
                       m_pSyntax->GetPos() + m_StartParseOffset);
  return (it - m_StreamStartOffsets.begin()) - 1;
}

void CPDF_StreamContentParser::Handle_ShowText() {
  ByteString str = GetString(0);
  if (!str.IsEmpty())
    AddTextObject(&str, 0, std::vector<float>(), 1);
}

void CPDF_StreamContentParser::Handle_ShowText_Positioning() {
  CPDF_Array* pArray = ToArray(GetObject(0));
  if (!pArray)
    return;

  size_t n = pArray->size();
  size_t nsegs = 0;
  for (size_t i = 0; i < n; i++) {
    const CPDF_Object* pDirectObject = pArray->GetDirectObjectAt(i);
    if (pDirectObject && pDirectObject->IsString())
      nsegs++;
  }
  if (nsegs == 0) {
    for (size_t i = 0; i < n; i++) {
      float fKerning = pArray->GetNumberAt(i);
      if (fKerning != 0)
        m_pCurStates->m_TextPos.x -= GetHorizontalTextSize(fKerning);
    }
    return;
  }
  std::vector<ByteString> strs(nsegs);
  std::vector<float> kernings(nsegs);
  size_t iSegment = 0;
  float fInitKerning = 0;
  for (size_t i = 0; i < n; i++) {
    CPDF_Object* pObj = pArray->GetDirectObjectAt(i);
    if (!pObj)
      continue;

    if (pObj->IsString()) {
      ByteString str = pObj->GetString();
      if (str.IsEmpty())
        continue;
      strs[iSegment] = std::move(str);
      kernings[iSegment++] = 0;
    } else {
      float num = pObj->GetNumber();
      if (iSegment == 0)
        fInitKerning += num;
      else
        kernings[iSegment - 1] += num;
    }
  }
  AddTextObject(strs.data(), fInitKerning, kernings, iSegment);
}

void CPDF_StreamContentParser::Handle_SetTextLeading() {
  m_pCurStates->m_TextLeading = GetNumber(0);
}

void CPDF_StreamContentParser::Handle_SetTextMatrix() {
  m_pCurStates->m_TextMatrix = GetMatrix();
  OnChangeTextMatrix();
  m_pCurStates->m_TextPos = CFX_PointF();
  m_pCurStates->m_TextLinePos = CFX_PointF();
}

void CPDF_StreamContentParser::OnChangeTextMatrix() {
  CFX_Matrix text_matrix(m_pCurStates->m_TextHorzScale, 0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f);
  text_matrix.Concat(m_pCurStates->m_TextMatrix);
  text_matrix.Concat(m_pCurStates->m_CTM);
  text_matrix.Concat(m_mtContentToUser);
  float* pTextMatrix = m_pCurStates->m_TextState.GetMutableMatrix();
  pTextMatrix[0] = text_matrix.a;
  pTextMatrix[1] = text_matrix.c;
  pTextMatrix[2] = text_matrix.b;
  pTextMatrix[3] = text_matrix.d;
}

void CPDF_StreamContentParser::Handle_SetTextRenderMode() {
  TextRenderingMode mode;
  if (SetTextRenderingModeFromInt(GetInteger(0), &mode))
    m_pCurStates->m_TextState.SetTextMode(mode);
}

void CPDF_StreamContentParser::Handle_SetTextRise() {
  m_pCurStates->m_TextRise = GetNumber(0);
}

void CPDF_StreamContentParser::Handle_SetWordSpace() {
  m_pCurStates->m_TextState.SetWordSpace(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetHorzScale() {
  if (m_ParamCount != 1) {
    return;
  }
  m_pCurStates->m_TextHorzScale = GetNumber(0) / 100;
  OnChangeTextMatrix();
}

void CPDF_StreamContentParser::Handle_MoveToNextLine() {
  m_pCurStates->m_TextLinePos.y -= m_pCurStates->m_TextLeading;
  m_pCurStates->m_TextPos = m_pCurStates->m_TextLinePos;
}

void CPDF_StreamContentParser::Handle_CurveTo_23() {
  AddPathPoint(m_PathCurrent, CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(2), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
}

void CPDF_StreamContentParser::Handle_SetLineWidth() {
  m_pCurStates->m_GraphState.SetLineWidth(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_Clip() {
  m_PathClipType = CFX_FillRenderOptions::FillType::kWinding;
}

void CPDF_StreamContentParser::Handle_EOClip() {
  m_PathClipType = CFX_FillRenderOptions::FillType::kEvenOdd;
}

void CPDF_StreamContentParser::Handle_CurveTo_13() {
  AddPathPoint(GetPoint(2), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
}

void CPDF_StreamContentParser::Handle_NextLineShowText() {
  Handle_MoveToNextLine();
  Handle_ShowText();
}

void CPDF_StreamContentParser::Handle_NextLineShowText_Space() {
  m_pCurStates->m_TextState.SetWordSpace(GetNumber(2));
  m_pCurStates->m_TextState.SetCharSpace(GetNumber(1));
  Handle_NextLineShowText();
}

void CPDF_StreamContentParser::Handle_Invalid() {}

void CPDF_StreamContentParser::AddPathPoint(const CFX_PointF& point,
                                            CFX_Path::Point::Type type) {
  // If the path point is the same move as the previous one and neither of them
  // closes the path, then just skip it.
  if (type == CFX_Path::Point::Type::kMove && !m_PathPoints.empty() &&
      !m_PathPoints.back().m_CloseFigure &&
      m_PathPoints.back().m_Type == type && m_PathCurrent == point) {
    return;
  }

  m_PathCurrent = point;
  if (type == CFX_Path::Point::Type::kMove) {
    m_PathStart = point;
    if (!m_PathPoints.empty() &&
        m_PathPoints.back().IsTypeAndOpen(CFX_Path::Point::Type::kMove)) {
      m_PathPoints.back().m_Point = point;
      return;
    }
  } else if (m_PathPoints.empty()) {
    return;
  }
  m_PathPoints.push_back(CFX_Path::Point(point, type, /*close=*/false));
}

void CPDF_StreamContentParser::AddPathPointAndClose(
    const CFX_PointF& point,
    CFX_Path::Point::Type type) {
  m_PathCurrent = point;
  if (m_PathPoints.empty())
    return;

  m_PathPoints.push_back(CFX_Path::Point(point, type, /*close=*/true));
}

void CPDF_StreamContentParser::AddPathObject(
    CFX_FillRenderOptions::FillType fill_type,
    RenderType render_type) {
  std::vector<CFX_Path::Point> path_points;
  path_points.swap(m_PathPoints);
  CFX_FillRenderOptions::FillType path_clip_type = m_PathClipType;
  m_PathClipType = CFX_FillRenderOptions::FillType::kNoFill;

  if (path_points.empty())
    return;

  if (path_points.size() == 1) {
    if (path_clip_type != CFX_FillRenderOptions::FillType::kNoFill) {
      CPDF_Path path;
      path.AppendRect(0, 0, 0, 0);
      m_pCurStates->m_ClipPath.AppendPathWithAutoMerge(
          path, CFX_FillRenderOptions::FillType::kWinding);
    }
    return;
  }

  if (path_points.back().IsTypeAndOpen(CFX_Path::Point::Type::kMove))
    path_points.pop_back();

  CPDF_Path path;
  for (const auto& point : path_points) {
    if (point.m_CloseFigure)
      path.AppendPointAndClose(point.m_Point, point.m_Type);
    else
      path.AppendPoint(point.m_Point, point.m_Type);
  }

  CFX_Matrix matrix = m_pCurStates->m_CTM * m_mtContentToUser;
  bool bStroke = render_type == RenderType::kStroke;
  if (bStroke || fill_type != CFX_FillRenderOptions::FillType::kNoFill) {
    auto pPathObj = std::make_unique<CPDF_PathObject>(GetCurrentStreamIndex());
    pPathObj->set_stroke(bStroke);
    pPathObj->set_filltype(fill_type);
    pPathObj->path() = path;
    SetGraphicStates(pPathObj.get(), true, false, true);
    pPathObj->SetPathMatrix(matrix);
    m_pObjectHolder->AppendPageObject(std::move(pPathObj));
  }
  if (path_clip_type != CFX_FillRenderOptions::FillType::kNoFill) {
    if (!matrix.IsIdentity())
      path.Transform(matrix);
    m_pCurStates->m_ClipPath.AppendPathWithAutoMerge(path, path_clip_type);
  }
}

uint32_t CPDF_StreamContentParser::Parse(
    pdfium::span<const uint8_t> pData,
    uint32_t start_offset,
    uint32_t max_cost,
    const std::vector<uint32_t>& stream_start_offsets) {
  DCHECK(start_offset < pData.size());

  // Parsing will be done from within |pDataStart|.
  pdfium::span<const uint8_t> pDataStart = pData.subspan(start_offset);
  m_StartParseOffset = start_offset;
  if (m_ParsedSet->size() > kMaxFormLevel ||
      pdfium::Contains(*m_ParsedSet, pDataStart.data())) {
    return pDataStart.size();
  }

  m_StreamStartOffsets = stream_start_offsets;

  ScopedSetInsertion<const uint8_t*> scopedInsert(m_ParsedSet.Get(),
                                                  pDataStart.data());

  uint32_t init_obj_count = m_pObjectHolder->GetPageObjectCount();
  AutoNuller<std::unique_ptr<CPDF_StreamParser>> auto_clearer(&m_pSyntax);
  m_pSyntax = std::make_unique<CPDF_StreamParser>(
      pDataStart, m_pDocument->GetByteStringPool());

  while (1) {
    uint32_t cost = m_pObjectHolder->GetPageObjectCount() - init_obj_count;
    if (max_cost && cost >= max_cost) {
      break;
    }
    switch (m_pSyntax->ParseNextElement()) {
      case CPDF_StreamParser::EndOfData:
        return m_pSyntax->GetPos();
      case CPDF_StreamParser::Keyword:
        OnOperator(m_pSyntax->GetWord());
        ClearAllParams();
        break;
      case CPDF_StreamParser::Number:
        AddNumberParam(m_pSyntax->GetWord());
        break;
      case CPDF_StreamParser::Name: {
        auto word = m_pSyntax->GetWord();
        AddNameParam(word.Last(word.GetLength() - 1));
        break;
      }
      default:
        AddObjectParam(m_pSyntax->GetObject());
    }
  }
  return m_pSyntax->GetPos();
}

void CPDF_StreamContentParser::ParsePathObject() {
  float params[6] = {};
  int nParams = 0;
  int last_pos = m_pSyntax->GetPos();
  while (1) {
    CPDF_StreamParser::SyntaxType type = m_pSyntax->ParseNextElement();
    bool bProcessed = true;
    switch (type) {
      case CPDF_StreamParser::EndOfData:
        return;
      case CPDF_StreamParser::Keyword: {
        ByteStringView strc = m_pSyntax->GetWord();
        int len = strc.GetLength();
        if (len == 1) {
          switch (strc[0]) {
            case kPathOperatorSubpath:
              AddPathPoint({params[0], params[1]},
                           CFX_Path::Point::Type::kMove);
              nParams = 0;
              break;
            case kPathOperatorLine:
              AddPathPoint({params[0], params[1]},
                           CFX_Path::Point::Type::kLine);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier1:
              AddPathPoint({params[0], params[1]},
                           CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[2], params[3]},
                           CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[4], params[5]},
                           CFX_Path::Point::Type::kBezier);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier2:
              AddPathPoint(m_PathCurrent, CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[0], params[1]},
                           CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[2], params[3]},
                           CFX_Path::Point::Type::kBezier);
              nParams = 0;
              break;
            case kPathOperatorCubicBezier3:
              AddPathPoint({params[0], params[1]},
                           CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[2], params[3]},
                           CFX_Path::Point::Type::kBezier);
              AddPathPoint({params[2], params[3]},
                           CFX_Path::Point::Type::kBezier);
              nParams = 0;
              break;
            case kPathOperatorClosePath:
              Handle_ClosePath();
              nParams = 0;
              break;
            default:
              bProcessed = false;
              break;
          }
        } else if (len == 2) {
          if (strc[0] == kPathOperatorRectangle[0] &&
              strc[1] == kPathOperatorRectangle[1]) {
            AddPathRect(params[0], params[1], params[2], params[3]);
            nParams = 0;
          } else {
            bProcessed = false;
          }
        } else {
          bProcessed = false;
        }
        if (bProcessed) {
          last_pos = m_pSyntax->GetPos();
        }
        break;
      }
      case CPDF_StreamParser::Number: {
        if (nParams == 6)
          break;

        FX_Number number(m_pSyntax->GetWord());
        params[nParams++] = number.GetFloat();
        break;
      }
      default:
        bProcessed = false;
    }
    if (!bProcessed) {
      m_pSyntax->SetPos(last_pos);
      return;
    }
  }
}

// static
ByteStringView CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
    ByteStringView abbr) {
  return FindFullName(kInlineKeyAbbr, pdfium::size(kInlineKeyAbbr), abbr);
}

// static
ByteStringView CPDF_StreamContentParser::FindValueAbbreviationForTesting(
    ByteStringView abbr) {
  return FindFullName(kInlineValueAbbr, pdfium::size(kInlineValueAbbr), abbr);
}

CPDF_StreamContentParser::ContentParam::ContentParam() = default;

CPDF_StreamContentParser::ContentParam::~ContentParam() = default;
