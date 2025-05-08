// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_streamcontentparser.h"

#include <algorithm>
#include <array>
#include <map>
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
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/compiler_specific.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/scoped_set_insertion.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_graphstate.h"
#include "core/fxge/cfx_graphstatedata.h"

namespace {

constexpr int kMaxFormLevel = 40;

constexpr int kSingleCoordinatePair = 1;
constexpr int kTensorCoordinatePairs = 16;
constexpr int kCoonsCoordinatePairs = 12;
constexpr int kSingleColorPerPatch = 1;
constexpr int kQuadColorsPerPatch = 4;

const char kPathOperatorSubpath = 'm';
const char kPathOperatorLine = 'l';
const char kPathOperatorCubicBezier1 = 'c';
const char kPathOperatorCubicBezier2 = 'v';
const char kPathOperatorCubicBezier3 = 'y';
const char kPathOperatorClosePath = 'h';
const char kPathOperatorRectangle[] = "re";

using OpCodes = std::map<uint32_t, void (CPDF_StreamContentParser::*)()>;
OpCodes* g_opcodes = nullptr;

CFX_FloatRect GetShadingBBox(CPDF_ShadingPattern* pShading,
                             const CFX_Matrix& matrix) {
  ShadingType type = pShading->GetShadingType();
  RetainPtr<const CPDF_Stream> pStream = ToStream(pShading->GetShadingObject());
  RetainPtr<CPDF_ColorSpace> pCS = pShading->GetCS();
  if (!pStream || !pCS) {
    return CFX_FloatRect();
  }

  CPDF_MeshStream stream(type, pShading->GetFuncs(), std::move(pStream),
                         std::move(pCS));
  if (!stream.Load()) {
    return CFX_FloatRect();
  }

  CFX_FloatRect rect;
  bool update_rect = false;
  bool bGouraud = type == kFreeFormGouraudTriangleMeshShading ||
                  type == kLatticeFormGouraudTriangleMeshShading;

  int point_count;
  if (type == kTensorProductPatchMeshShading) {
    point_count = kTensorCoordinatePairs;
  } else if (type == kCoonsPatchMeshShading) {
    point_count = kCoonsCoordinatePairs;
  } else {
    point_count = kSingleCoordinatePair;
  }

  int color_count;
  if (type == kCoonsPatchMeshShading ||
      type == kTensorProductPatchMeshShading) {
    color_count = kQuadColorsPerPatch;
  } else {
    color_count = kSingleColorPerPatch;
  }

  while (!stream.IsEOF()) {
    uint32_t flag = 0;
    if (type != kLatticeFormGouraudTriangleMeshShading) {
      if (!stream.CanReadFlag()) {
        break;
      }
      flag = stream.ReadFlag();
    }

    if (!bGouraud && flag) {
      point_count -= 4;
      color_count -= 2;
    }

    for (int i = 0; i < point_count; ++i) {
      if (!stream.CanReadCoords()) {
        break;
      }

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
    if (!nBits.IsValid()) {
      break;
    }

    stream.SkipBits(nBits.ValueOrDie());
    if (bGouraud) {
      stream.ByteAlign();
    }
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

ByteStringView FindFullName(pdfium::span<const AbbrPair> table,
                            ByteStringView abbr) {
  for (const auto& pair : table) {
    if (pair.abbr == abbr) {
      return ByteStringView(pair.full_name);
    }
  }
  return ByteStringView();
}

void ReplaceAbbr(RetainPtr<CPDF_Object> pObj);

void ReplaceAbbrInDictionary(CPDF_Dictionary* pDict) {
  std::vector<AbbrReplacementOp> replacements;
  {
    CPDF_DictionaryLocker locker(pDict);
    for (const auto& it : locker) {
      ByteString key = it.first;
      ByteStringView fullname =
          FindFullName(kInlineKeyAbbr, key.AsStringView());
      if (!fullname.IsEmpty()) {
        AbbrReplacementOp op;
        op.is_replace_key = true;
        op.key = std::move(key);
        op.replacement = fullname;
        replacements.push_back(op);
        key = fullname;
      }
      RetainPtr<CPDF_Object> value = it.second;
      if (value->IsName()) {
        ByteString name = value->GetString();
        fullname = FindFullName(kInlineValueAbbr, name.AsStringView());
        if (!fullname.IsEmpty()) {
          AbbrReplacementOp op;
          op.is_replace_key = false;
          op.key = key;
          op.replacement = fullname;
          replacements.push_back(op);
        }
      } else {
        ReplaceAbbr(std::move(value));
      }
    }
  }
  for (const auto& op : replacements) {
    if (op.is_replace_key) {
      pDict->ReplaceKey(op.key, ByteString(op.replacement));
    } else {
      pDict->SetNewFor<CPDF_Name>(op.key, ByteString(op.replacement));
    }
  }
}

void ReplaceAbbrInArray(CPDF_Array* pArray) {
  for (size_t i = 0; i < pArray->size(); ++i) {
    RetainPtr<CPDF_Object> pElement = pArray->GetMutableObjectAt(i);
    if (pElement->IsName()) {
      ByteString name = pElement->GetString();
      ByteStringView fullname =
          FindFullName(kInlineValueAbbr, name.AsStringView());
      if (!fullname.IsEmpty()) {
        pArray->SetNewAt<CPDF_Name>(i, ByteString(fullname));
      }
    } else {
      ReplaceAbbr(std::move(pElement));
    }
  }
}

void ReplaceAbbr(RetainPtr<CPDF_Object> pObj) {
  CPDF_Dictionary* pDict = pObj->AsMutableDictionary();
  if (pDict) {
    ReplaceAbbrInDictionary(pDict);
    return;
  }

  CPDF_Array* pArray = pObj->AsMutableArray();
  if (pArray) {
    ReplaceAbbrInArray(pArray);
  }
}

}  // namespace

// static
void CPDF_StreamContentParser::InitializeGlobals() {
  CHECK(!g_opcodes);
  g_opcodes = new OpCodes({
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

// static
void CPDF_StreamContentParser::DestroyGlobals() {
  delete g_opcodes;
  g_opcodes = nullptr;
}

CPDF_StreamContentParser::CPDF_StreamContentParser(
    CPDF_Document* pDocument,
    RetainPtr<CPDF_Dictionary> pPageResources,
    RetainPtr<CPDF_Dictionary> pParentResources,
    const CFX_Matrix* pmtContentToUser,
    CPDF_PageObjectHolder* pObjHolder,
    RetainPtr<CPDF_Dictionary> pResources,
    const CFX_FloatRect& rcBBox,
    const CPDF_AllStates* pStates,
    CPDF_Form::RecursionState* recursion_state)
    : document_(pDocument),
      page_resources_(pPageResources),
      parent_resources_(pParentResources),
      resources_(CPDF_Form::ChooseResourcesDict(pResources.Get(),
                                                pParentResources.Get(),
                                                pPageResources.Get())),
      object_holder_(pObjHolder),
      recursion_state_(recursion_state),
      bbox_(rcBBox),
      cur_states_(std::make_unique<CPDF_AllStates>()) {
  if (pmtContentToUser) {
    mt_content_to_user_ = *pmtContentToUser;
  }
  if (pStates) {
    *cur_states_ = *pStates;
  } else {
    cur_states_->mutable_general_state().Emplace();
    cur_states_->mutable_graph_state().Emplace();
    cur_states_->mutable_text_state().Emplace();
    cur_states_->mutable_color_state().Emplace();
  }

  // Add the sentinel.
  content_marks_stack_.push(std::make_unique<CPDF_ContentMarks>());

  // Initialize `all_ctms_`, as there is a CTM, even if the stream contains no
  // cm operators.
  all_ctms_[0] = cur_states_->current_transformation_matrix();
}

CPDF_StreamContentParser::~CPDF_StreamContentParser() {
  ClearAllParams();
}

int CPDF_StreamContentParser::GetNextParamPos() {
  if (param_count_ == kParamBufSize) {
    param_start_pos_++;
    if (param_start_pos_ == kParamBufSize) {
      param_start_pos_ = 0;
    }
    auto& param = param_buf_[param_start_pos_];
    if (std::holds_alternative<RetainPtr<CPDF_Object>>(param)) {
      std::get<RetainPtr<CPDF_Object>>(param).Reset();
    }

    return param_start_pos_;
  }
  int index = param_start_pos_ + param_count_;
  if (index >= kParamBufSize) {
    index -= kParamBufSize;
  }
  param_count_++;
  return index;
}

void CPDF_StreamContentParser::AddNameParam(ByteStringView bsName) {
  param_buf_[GetNextParamPos()] = PDF_NameDecode(bsName);
}

void CPDF_StreamContentParser::AddNumberParam(ByteStringView str) {
  param_buf_[GetNextParamPos()] = FX_Number(str);
}

void CPDF_StreamContentParser::AddObjectParam(RetainPtr<CPDF_Object> pObj) {
  param_buf_[GetNextParamPos()] = std::move(pObj);
}

void CPDF_StreamContentParser::ClearAllParams() {
  uint32_t index = param_start_pos_;
  for (uint32_t i = 0; i < param_count_; i++) {
    if (std::holds_alternative<RetainPtr<CPDF_Object>>(param_buf_[index])) {
      std::get<RetainPtr<CPDF_Object>>(param_buf_[index]).Reset();
    }
    index++;
    if (index == kParamBufSize) {
      index = 0;
    }
  }
  param_start_pos_ = 0;
  param_count_ = 0;
}

RetainPtr<CPDF_Object> CPDF_StreamContentParser::GetObject(uint32_t index) {
  if (index >= param_count_) {
    return nullptr;
  }
  int real_index = param_start_pos_ + param_count_ - index - 1;
  if (real_index >= kParamBufSize) {
    real_index -= kParamBufSize;
  }
  ContentParam& param = param_buf_[real_index];
  if (std::holds_alternative<FX_Number>(param)) {
    const auto& number = std::get<FX_Number>(param);
    param = number.IsInteger()
                ? pdfium::MakeRetain<CPDF_Number>(number.GetSigned())
                : pdfium::MakeRetain<CPDF_Number>(number.GetFloat());
    return std::get<RetainPtr<CPDF_Object>>(param);
  }
  if (std::holds_alternative<ByteString>(param)) {
    const auto& name = std::get<ByteString>(param);
    param = document_->New<CPDF_Name>(name);
    return std::get<RetainPtr<CPDF_Object>>(param);
  }
  CHECK(std::holds_alternative<RetainPtr<CPDF_Object>>(param));
  return std::get<RetainPtr<CPDF_Object>>(param);
}

ByteString CPDF_StreamContentParser::GetString(uint32_t index) const {
  if (index >= param_count_) {
    return ByteString();
  }

  int real_index = param_start_pos_ + param_count_ - index - 1;
  if (real_index >= kParamBufSize) {
    real_index -= kParamBufSize;
  }

  const ContentParam& param = param_buf_[real_index];
  if (std::holds_alternative<ByteString>(param)) {
    return std::get<ByteString>(param);
  }

  if (std::holds_alternative<RetainPtr<CPDF_Object>>(param)) {
    const auto& obj = std::get<RetainPtr<CPDF_Object>>(param);
    if (obj) {
      return obj->GetString();
    }
  }

  return ByteString();
}

float CPDF_StreamContentParser::GetNumber(uint32_t index) const {
  if (index >= param_count_) {
    return 0;
  }

  int real_index = param_start_pos_ + param_count_ - index - 1;
  if (real_index >= kParamBufSize) {
    real_index -= kParamBufSize;
  }

  const ContentParam& param = param_buf_[real_index];
  if (std::holds_alternative<FX_Number>(param)) {
    return std::get<FX_Number>(param).GetFloat();
  }

  if (std::holds_alternative<RetainPtr<CPDF_Object>>(param)) {
    const auto& obj = std::get<RetainPtr<CPDF_Object>>(param);
    if (obj) {
      return obj->GetNumber();
    }
  }

  return 0;
}

std::vector<float> CPDF_StreamContentParser::GetNumbers(size_t count) const {
  std::vector<float> values(count);
  for (size_t i = 0; i < count; ++i) {
    values[i] = GetNumber(count - i - 1);
  }
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
  pObj->mutable_general_state() = cur_states_->general_state();
  pObj->mutable_clip_path() = cur_states_->clip_path();
  pObj->SetContentMarks(*content_marks_stack_.top());
  if (bColor) {
    pObj->mutable_color_state() = cur_states_->color_state();
  }
  if (bGraph) {
    pObj->mutable_graph_state() = cur_states_->graph_state();
  }
  if (bText) {
    pObj->mutable_text_state() = cur_states_->text_state();
  }
}

void CPDF_StreamContentParser::OnOperator(ByteStringView op) {
  auto it = g_opcodes->find(op.GetID());
  if (it != g_opcodes->end()) {
    (this->*it->second)();
  }
}

void CPDF_StreamContentParser::Handle_CloseFillStrokePath() {
  Handle_ClosePath();
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_FillStrokePath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kWinding, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_CloseEOFillStrokePath() {
  AddPathPointAndClose(path_start_, CFX_Path::Point::Type::kLine);
  AddPathObject(CFX_FillRenderOptions::FillType::kEvenOdd, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_EOFillStrokePath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kEvenOdd, RenderType::kStroke);
}

void CPDF_StreamContentParser::Handle_BeginMarkedContent_Dictionary() {
  RetainPtr<CPDF_Object> pProperty = GetObject(0);
  if (!pProperty) {
    return;
  }

  ByteString tag = GetString(1);
  std::unique_ptr<CPDF_ContentMarks> new_marks =
      content_marks_stack_.top()->Clone();

  if (pProperty->IsName()) {
    ByteString property_name = pProperty->GetString();
    RetainPtr<CPDF_Dictionary> pHolder = FindResourceHolder("Properties");
    if (!pHolder || !pHolder->GetDictFor(property_name)) {
      return;
    }
    new_marks->AddMarkWithPropertiesHolder(tag, std::move(pHolder),
                                           property_name);
  } else if (pProperty->IsDictionary()) {
    new_marks->AddMarkWithDirectDict(tag, ToDictionary(pProperty));
  } else {
    return;
  }
  content_marks_stack_.push(std::move(new_marks));
}

void CPDF_StreamContentParser::Handle_BeginImage() {
  FX_FILESIZE savePos = syntax_->GetPos();
  auto pDict = document_->New<CPDF_Dictionary>();
  while (true) {
    CPDF_StreamParser::ElementType type = syntax_->ParseNextElement();
    if (type == CPDF_StreamParser::ElementType::kKeyword) {
      if (syntax_->GetWord() != "ID") {
        syntax_->SetPos(savePos);
        return;
      }
    }
    if (type != CPDF_StreamParser::ElementType::kName) {
      break;
    }
    // Next `syntax_` read below may invalidate `word`. Must save to `key`.
    ByteStringView word = syntax_->GetWord();
    ByteString key(word.Last(word.GetLength() - 1));
    auto pObj = syntax_->ReadNextObject(false, false, 0);
    if (pObj && !pObj->IsInline()) {
      pDict->SetNewFor<CPDF_Reference>(key, document_, pObj->GetObjNum());
    } else {
      pDict->SetFor(key, std::move(pObj));
    }
  }
  ReplaceAbbr(pDict);
  RetainPtr<const CPDF_Object> pCSObj;
  if (pDict->KeyExist("ColorSpace")) {
    pCSObj = pDict->GetDirectObjectFor("ColorSpace");
    if (pCSObj->IsName()) {
      ByteString name = pCSObj->GetString();
      if (name != "DeviceRGB" && name != "DeviceGray" && name != "DeviceCMYK") {
        pCSObj = FindResourceObj("ColorSpace", name);
        if (pCSObj && pCSObj->IsInline()) {
          pDict->SetFor("ColorSpace", pCSObj->Clone());
        }
      }
    }
  }
  pDict->SetNewFor<CPDF_Name>("Subtype", "Image");
  RetainPtr<CPDF_Stream> pStream =
      syntax_->ReadInlineStream(document_, std::move(pDict), pCSObj.Get());
  while (true) {
    CPDF_StreamParser::ElementType type = syntax_->ParseNextElement();
    if (type == CPDF_StreamParser::ElementType::kEndOfData) {
      return;
    }

    if (type == CPDF_StreamParser::ElementType::kKeyword &&
        syntax_->GetWord() == "EI") {
      break;
    }
  }
  CPDF_ImageObject* pObj = AddImageFromStream(std::move(pStream), /*name=*/"");
  // Record the bounding box of this image, so rendering code can draw it
  // properly.
  if (pObj && pObj->GetImage()->IsMask()) {
    object_holder_->AddImageMaskBoundingBox(pObj->GetRect());
  }
}

void CPDF_StreamContentParser::Handle_BeginMarkedContent() {
  std::unique_ptr<CPDF_ContentMarks> new_marks =
      content_marks_stack_.top()->Clone();
  new_marks->AddMark(GetString(0));
  content_marks_stack_.push(std::move(new_marks));
}

void CPDF_StreamContentParser::Handle_BeginText() {
  cur_states_->set_text_matrix(CFX_Matrix());
  OnChangeTextMatrix();
  cur_states_->ResetTextPosition();
}

void CPDF_StreamContentParser::Handle_CurveTo_123() {
  AddPathPoint(GetPoint(4), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(2), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
}

void CPDF_StreamContentParser::Handle_ConcatMatrix() {
  cur_states_->prepend_to_current_transformation_matrix(GetMatrix());
  all_ctms_[GetCurrentStreamIndex()] =
      cur_states_->current_transformation_matrix();
  OnChangeTextMatrix();
}

void CPDF_StreamContentParser::Handle_SetColorSpace_Fill() {
  RetainPtr<CPDF_ColorSpace> pCS = FindColorSpace(GetString(0));
  if (!pCS) {
    return;
  }

  cur_states_->mutable_color_state().GetMutableFillColor()->SetColorSpace(
      std::move(pCS));
}

void CPDF_StreamContentParser::Handle_SetColorSpace_Stroke() {
  RetainPtr<CPDF_ColorSpace> pCS = FindColorSpace(GetString(0));
  if (!pCS) {
    return;
  }

  cur_states_->mutable_color_state().GetMutableStrokeColor()->SetColorSpace(
      std::move(pCS));
}

void CPDF_StreamContentParser::Handle_SetDash() {
  RetainPtr<CPDF_Array> pArray = ToArray(GetObject(1));
  if (!pArray) {
    return;
  }

  cur_states_->SetLineDash(pArray.Get(), GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetCharWidth() {
  type3_data_[0] = GetNumber(1);
  type3_data_[1] = GetNumber(0);
  colored_ = true;
}

void CPDF_StreamContentParser::Handle_SetCachedDevice() {
  for (int i = 0; i < 6; i++) {
    type3_data_[i] = GetNumber(5 - i);
  }
  colored_ = false;
}

void CPDF_StreamContentParser::Handle_ExecuteXObject() {
  ByteString name = GetString(0);
  if (name == last_image_name_ && last_image_ && last_image_->GetStream() &&
      last_image_->GetStream()->GetObjNum()) {
    CPDF_ImageObject* pObj = AddLastImage();
    // Record the bounding box of this image, so rendering code can draw it
    // properly.
    if (pObj && pObj->GetImage()->IsMask()) {
      object_holder_->AddImageMaskBoundingBox(pObj->GetRect());
    }
    return;
  }

  RetainPtr<CPDF_Stream> pXObject(ToStream(FindResourceObj("XObject", name)));
  if (!pXObject) {
    return;
  }

  const ByteString type = pXObject->GetDict()->GetByteStringFor("Subtype");
  if (type == "Form") {
    AddForm(std::move(pXObject), name);
    return;
  }

  if (type == "Image") {
    CPDF_ImageObject* pObj =
        pXObject->IsInline()
            ? AddImageFromStream(ToStream(pXObject->Clone()), name)
            : AddImageFromStreamObjNum(pXObject->GetObjNum(), name);

    last_image_name_ = std::move(name);
    if (pObj) {
      last_image_ = pObj->GetImage();
      if (last_image_->IsMask()) {
        object_holder_->AddImageMaskBoundingBox(pObj->GetRect());
      }
    }
  }
}

void CPDF_StreamContentParser::AddForm(RetainPtr<CPDF_Stream> pStream,
                                       const ByteString& name) {
  CPDF_AllStates status;
  status.mutable_general_state() = cur_states_->general_state();
  status.mutable_graph_state() = cur_states_->graph_state();
  status.mutable_color_state() = cur_states_->color_state();
  status.mutable_text_state() = cur_states_->text_state();
  auto form = std::make_unique<CPDF_Form>(document_, page_resources_,
                                          std::move(pStream), resources_.Get());
  form->ParseContent(&status, nullptr, recursion_state_);

  CFX_Matrix matrix =
      cur_states_->current_transformation_matrix() * mt_content_to_user_;
  auto pFormObj = std::make_unique<CPDF_FormObject>(GetCurrentStreamIndex(),
                                                    std::move(form), matrix);
  pFormObj->SetResourceName(name);
  if (!object_holder_->BackgroundAlphaNeeded() &&
      pFormObj->form()->BackgroundAlphaNeeded()) {
    object_holder_->SetBackgroundAlphaNeeded(true);
  }
  pFormObj->CalcBoundingBox();
  SetGraphicStates(pFormObj.get(), true, true, true);
  object_holder_->AppendPageObject(std::move(pFormObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImageFromStream(
    RetainPtr<CPDF_Stream> pStream,
    const ByteString& name) {
  if (!pStream) {
    return nullptr;
  }

  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetResourceName(name);
  pImageObj->SetImage(
      pdfium::MakeRetain<CPDF_Image>(document_, std::move(pStream)));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImageFromStreamObjNum(
    uint32_t stream_obj_num,
    const ByteString& name) {
  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetResourceName(name);
  pImageObj->SetImage(
      CPDF_DocPageData::FromDocument(document_)->GetImage(stream_obj_num));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddLastImage() {
  DCHECK(last_image_);

  auto pImageObj = std::make_unique<CPDF_ImageObject>(GetCurrentStreamIndex());
  pImageObj->SetResourceName(last_image_name_);
  pImageObj->SetImage(CPDF_DocPageData::FromDocument(document_)->GetImage(
      last_image_->GetStream()->GetObjNum()));

  return AddImageObject(std::move(pImageObj));
}

CPDF_ImageObject* CPDF_StreamContentParser::AddImageObject(
    std::unique_ptr<CPDF_ImageObject> pImageObj) {
  SetGraphicStates(pImageObj.get(), pImageObj->GetImage()->IsMask(), false,
                   false);

  pImageObj->SetInitialImageMatrix(
      cur_states_->current_transformation_matrix() * mt_content_to_user_);

  CPDF_ImageObject* pRet = pImageObj.get();
  object_holder_->AppendPageObject(std::move(pImageObj));
  return pRet;
}

std::vector<float> CPDF_StreamContentParser::GetColors() const {
  DCHECK(param_count_ > 0);
  return GetNumbers(param_count_);
}

std::vector<float> CPDF_StreamContentParser::GetNamedColors() const {
  DCHECK(param_count_ > 0);
  const uint32_t nvalues = param_count_ - 1;
  std::vector<float> values(nvalues);
  for (size_t i = 0; i < nvalues; ++i) {
    values[i] = GetNumber(param_count_ - i - 1);
  }
  return values;
}

void CPDF_StreamContentParser::Handle_MarkPlace_Dictionary() {}

void CPDF_StreamContentParser::Handle_EndImage() {}

void CPDF_StreamContentParser::Handle_EndMarkedContent() {
  // First element is a sentinel, so do not pop it, ever. This may come up if
  // the EMCs are mismatched with the BMC/BDCs.
  if (content_marks_stack_.size() > 1) {
    content_marks_stack_.pop();
  }
}

void CPDF_StreamContentParser::Handle_EndText() {
  if (clip_text_list_.empty()) {
    return;
  }

  if (TextRenderingModeIsClipMode(cur_states_->text_state().GetTextMode())) {
    cur_states_->mutable_clip_path().AppendTexts(&clip_text_list_);
  }

  clip_text_list_.clear();
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
  cur_states_->mutable_color_state().SetFillColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray),
      GetNumbers(1));
}

void CPDF_StreamContentParser::Handle_SetGray_Stroke() {
  cur_states_->mutable_color_state().SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceGray),
      GetNumbers(1));
}

void CPDF_StreamContentParser::Handle_SetExtendGraphState() {
  ByteString name = GetString(0);
  RetainPtr<CPDF_Dictionary> pGS =
      ToDictionary(FindResourceObj("ExtGState", name));
  if (!pGS) {
    return;
  }

  CHECK(!name.IsEmpty());
  cur_states_->mutable_general_state().AppendGraphicsResourceName(
      std::move(name));
  cur_states_->ProcessExtGS(pGS.Get(), this);
}

void CPDF_StreamContentParser::Handle_ClosePath() {
  if (path_points_.empty()) {
    return;
  }

  if (path_start_.x != path_current_.x || path_start_.y != path_current_.y) {
    AddPathPointAndClose(path_start_, CFX_Path::Point::Type::kLine);
  } else {
    path_points_.back().close_figure_ = true;
  }
}

void CPDF_StreamContentParser::Handle_SetFlat() {
  cur_states_->mutable_general_state().SetFlatness(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_BeginImageData() {}

void CPDF_StreamContentParser::Handle_SetLineJoin() {
  cur_states_->mutable_graph_state().SetLineJoin(
      static_cast<CFX_GraphStateData::LineJoin>(GetInteger(0)));
}

void CPDF_StreamContentParser::Handle_SetLineCap() {
  cur_states_->mutable_graph_state().SetLineCap(
      static_cast<CFX_GraphStateData::LineCap>(GetInteger(0)));
}

void CPDF_StreamContentParser::Handle_SetCMYKColor_Fill() {
  if (param_count_ != 4) {
    return;
  }

  cur_states_->mutable_color_state().SetFillColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK),
      GetNumbers(4));
}

void CPDF_StreamContentParser::Handle_SetCMYKColor_Stroke() {
  if (param_count_ != 4) {
    return;
  }

  cur_states_->mutable_color_state().SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK),
      GetNumbers(4));
}

void CPDF_StreamContentParser::Handle_LineTo() {
  if (param_count_ != 2) {
    return;
  }

  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kLine);
}

void CPDF_StreamContentParser::Handle_MoveTo() {
  if (param_count_ != 2) {
    return;
  }

  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kMove);
  ParsePathObject();
}

void CPDF_StreamContentParser::Handle_SetMiterLimit() {
  cur_states_->mutable_graph_state().SetMiterLimit(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_MarkPlace() {}

void CPDF_StreamContentParser::Handle_EndPath() {
  AddPathObject(CFX_FillRenderOptions::FillType::kNoFill, RenderType::kFill);
}

void CPDF_StreamContentParser::Handle_SaveGraphState() {
  state_stack_.push_back(std::make_unique<CPDF_AllStates>(*cur_states_));
}

void CPDF_StreamContentParser::Handle_RestoreGraphState() {
  if (state_stack_.empty()) {
    return;
  }

  *cur_states_ = *state_stack_.back();
  state_stack_.pop_back();
  all_ctms_[GetCurrentStreamIndex()] =
      cur_states_->current_transformation_matrix();
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
  if (param_count_ != 3) {
    return;
  }

  cur_states_->mutable_color_state().SetFillColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB),
      GetNumbers(3));
}

void CPDF_StreamContentParser::Handle_SetRGBColor_Stroke() {
  if (param_count_ != 3) {
    return;
  }

  cur_states_->mutable_color_state().SetStrokeColor(
      CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB),
      GetNumbers(3));
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
  int nargs = std::min(param_count_, 4U);
  cur_states_->mutable_color_state().SetFillColor(nullptr, GetNumbers(nargs));
}

void CPDF_StreamContentParser::Handle_SetColor_Stroke() {
  int nargs = std::min(param_count_, 4U);
  cur_states_->mutable_color_state().SetStrokeColor(nullptr, GetNumbers(nargs));
}

void CPDF_StreamContentParser::Handle_SetColorPS_Fill() {
  RetainPtr<CPDF_Object> pLastParam = GetObject(0);
  if (!pLastParam) {
    return;
  }

  if (!pLastParam->IsName()) {
    cur_states_->mutable_color_state().SetFillColor(nullptr, GetColors());
    return;
  }

  // A valid |pLastParam| implies |param_count_| > 0, so GetNamedColors() call
  // below is safe.
  RetainPtr<CPDF_Pattern> pPattern = FindPattern(GetString(0));
  if (!pPattern) {
    return;
  }

  std::vector<float> values = GetNamedColors();
  cur_states_->mutable_color_state().SetFillPattern(std::move(pPattern),
                                                    values);
}

void CPDF_StreamContentParser::Handle_SetColorPS_Stroke() {
  RetainPtr<CPDF_Object> pLastParam = GetObject(0);
  if (!pLastParam) {
    return;
  }

  if (!pLastParam->IsName()) {
    cur_states_->mutable_color_state().SetStrokeColor(nullptr, GetColors());
    return;
  }

  // A valid |pLastParam| implies |param_count_| > 0, so GetNamedColors() call
  // below is safe.
  RetainPtr<CPDF_Pattern> pPattern = FindPattern(GetString(0));
  if (!pPattern) {
    return;
  }

  std::vector<float> values = GetNamedColors();
  cur_states_->mutable_color_state().SetStrokePattern(std::move(pPattern),
                                                      values);
}

void CPDF_StreamContentParser::Handle_ShadeFill() {
  RetainPtr<CPDF_ShadingPattern> pShading = FindShading(GetString(0));
  if (!pShading) {
    return;
  }

  if (!pShading->IsShadingObject() || !pShading->Load()) {
    return;
  }

  CFX_Matrix matrix =
      cur_states_->current_transformation_matrix() * mt_content_to_user_;
  auto pObj = std::make_unique<CPDF_ShadingObject>(GetCurrentStreamIndex(),
                                                   pShading, matrix);
  SetGraphicStates(pObj.get(), false, false, false);
  CFX_FloatRect bbox =
      pObj->clip_path().HasRef() ? pObj->clip_path().GetClipBox() : bbox_;
  if (pShading->IsMeshShading()) {
    bbox.Intersect(GetShadingBBox(pShading.Get(), pObj->matrix()));
  }
  pObj->SetRect(bbox);
  object_holder_->AppendPageObject(std::move(pObj));
}

void CPDF_StreamContentParser::Handle_SetCharSpace() {
  cur_states_->mutable_text_state().SetCharSpace(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_MoveTextPoint() {
  cur_states_->MoveTextPoint(GetPoint(0));
}

void CPDF_StreamContentParser::Handle_MoveTextPoint_SetLeading() {
  Handle_MoveTextPoint();
  cur_states_->set_text_leading(-GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetFont() {
  cur_states_->mutable_text_state().SetFontSize(GetNumber(0));
  RetainPtr<CPDF_Font> pFont = FindFont(GetString(1));
  if (pFont) {
    cur_states_->mutable_text_state().SetFont(std::move(pFont));
  }
}

RetainPtr<CPDF_Dictionary> CPDF_StreamContentParser::FindResourceHolder(
    const ByteString& type) {
  if (!resources_) {
    return nullptr;
  }

  RetainPtr<CPDF_Dictionary> pDict = resources_->GetMutableDictFor(type);
  if (pDict) {
    return pDict;
  }

  if (resources_ == page_resources_ || !page_resources_) {
    return nullptr;
  }

  return page_resources_->GetMutableDictFor(type);
}

RetainPtr<CPDF_Object> CPDF_StreamContentParser::FindResourceObj(
    const ByteString& type,
    const ByteString& name) {
  RetainPtr<CPDF_Dictionary> pHolder = FindResourceHolder(type);
  return pHolder ? pHolder->GetMutableDirectObjectFor(name) : nullptr;
}

RetainPtr<CPDF_Font> CPDF_StreamContentParser::FindFont(
    const ByteString& name) {
  RetainPtr<CPDF_Dictionary> pFontDict(
      ToDictionary(FindResourceObj("Font", name)));
  if (!pFontDict) {
    return CPDF_Font::GetStockFont(document_, CFX_Font::kDefaultAnsiFontName);
  }
  RetainPtr<CPDF_Font> pFont =
      CPDF_DocPageData::FromDocument(document_)->GetFont(std::move(pFontDict));
  if (pFont) {
    // Save `name` for later retrieval by the CPDF_TextObject that uses the
    // font.
    pFont->SetResourceName(name);
    if (pFont->IsType3Font()) {
      pFont->AsType3Font()->SetPageResources(resources_.Get());
      pFont->AsType3Font()->CheckType3FontMetrics();
    }
  }
  return pFont;
}

CPDF_PageObjectHolder::CTMMap CPDF_StreamContentParser::TakeAllCTMs() {
  CPDF_PageObjectHolder::CTMMap all_ctms;
  all_ctms.swap(all_ctms_);
  return all_ctms;
}

RetainPtr<CPDF_ColorSpace> CPDF_StreamContentParser::FindColorSpace(
    const ByteString& name) {
  if (name == "Pattern") {
    return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kPattern);
  }

  if (name == "DeviceGray" || name == "DeviceCMYK" || name == "DeviceRGB") {
    ByteString defname = "Default";
    defname += name.Last(name.GetLength() - 7);
    RetainPtr<const CPDF_Object> pDefObj =
        FindResourceObj("ColorSpace", defname);
    if (!pDefObj) {
      if (name == "DeviceGray") {
        return CPDF_ColorSpace::GetStockCS(
            CPDF_ColorSpace::Family::kDeviceGray);
      }
      if (name == "DeviceRGB") {
        return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceRGB);
      }

      return CPDF_ColorSpace::GetStockCS(CPDF_ColorSpace::Family::kDeviceCMYK);
    }
    return CPDF_DocPageData::FromDocument(document_)->GetColorSpace(
        pDefObj.Get(), nullptr);
  }
  RetainPtr<const CPDF_Object> pCSObj = FindResourceObj("ColorSpace", name);
  if (!pCSObj) {
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(document_)->GetColorSpace(pCSObj.Get(),
                                                                  nullptr);
}

RetainPtr<CPDF_Pattern> CPDF_StreamContentParser::FindPattern(
    const ByteString& name) {
  RetainPtr<CPDF_Object> pPattern = FindResourceObj("Pattern", name);
  if (!pPattern || (!pPattern->IsDictionary() && !pPattern->IsStream())) {
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(document_)->GetPattern(
      std::move(pPattern), cur_states_->parent_matrix());
}

RetainPtr<CPDF_ShadingPattern> CPDF_StreamContentParser::FindShading(
    const ByteString& name) {
  RetainPtr<CPDF_Object> pPattern = FindResourceObj("Shading", name);
  if (!pPattern || (!pPattern->IsDictionary() && !pPattern->IsStream())) {
    return nullptr;
  }
  return CPDF_DocPageData::FromDocument(document_)->GetShading(
      std::move(pPattern), cur_states_->parent_matrix());
}

void CPDF_StreamContentParser::AddTextObject(
    pdfium::span<const ByteString> strings,
    pdfium::span<const float> kernings,
    float initial_kerning) {
  RetainPtr<CPDF_Font> pFont = cur_states_->text_state().GetFont();
  if (!pFont) {
    return;
  }
  if (initial_kerning != 0) {
    if (pFont->IsVertWriting()) {
      cur_states_->IncrementTextPositionY(
          -GetVerticalTextSize(initial_kerning));
    } else {
      cur_states_->IncrementTextPositionX(
          -GetHorizontalTextSize(initial_kerning));
    }
  }
  if (strings.empty()) {
    return;
  }
  const TextRenderingMode text_mode =
      pFont->IsType3Font() ? TextRenderingMode::MODE_FILL
                           : cur_states_->text_state().GetTextMode();
  {
    auto pText = std::make_unique<CPDF_TextObject>(GetCurrentStreamIndex());
    pText->SetResourceName(pFont->GetResourceName());
    SetGraphicStates(pText.get(), true, true, true);
    if (TextRenderingModeIsStrokeMode(text_mode)) {
      const CFX_Matrix& ctm = cur_states_->current_transformation_matrix();
      pdfium::span<float> text_ctm =
          pText->mutable_text_state().GetMutableCTM();
      text_ctm[0] = ctm.a;
      text_ctm[1] = ctm.c;
      text_ctm[2] = ctm.b;
      text_ctm[3] = ctm.d;
    }
    pText->SetSegments(strings, kernings);
    pText->SetPosition(mt_content_to_user_.Transform(
        cur_states_->GetTransformedTextPosition()));

    const CFX_PointF position =
        pText->CalcPositionData(cur_states_->text_horz_scale());
    cur_states_->IncrementTextPositionX(position.x);
    cur_states_->IncrementTextPositionY(position.y);
    if (TextRenderingModeIsClipMode(text_mode)) {
      clip_text_list_.push_back(pText->Clone());
    }
    object_holder_->AppendPageObject(std::move(pText));
  }
  if (!kernings.empty() && kernings.back() != 0) {
    if (pFont->IsVertWriting()) {
      cur_states_->IncrementTextPositionY(
          -GetVerticalTextSize(kernings.back()));
    } else {
      cur_states_->IncrementTextPositionX(
          -GetHorizontalTextSize(kernings.back()));
    }
  }
}

float CPDF_StreamContentParser::GetHorizontalTextSize(float fKerning) const {
  return GetVerticalTextSize(fKerning) * cur_states_->text_horz_scale();
}

float CPDF_StreamContentParser::GetVerticalTextSize(float fKerning) const {
  return fKerning * cur_states_->text_state().GetFontSize() / 1000;
}

int32_t CPDF_StreamContentParser::GetCurrentStreamIndex() {
  auto it = std::upper_bound(stream_start_offsets_.begin(),
                             stream_start_offsets_.end(),
                             syntax_->GetPos() + start_parse_offset_);
  return (it - stream_start_offsets_.begin()) - 1;
}

void CPDF_StreamContentParser::Handle_ShowText() {
  ByteString str = GetString(0);
  if (!str.IsEmpty()) {
    AddTextObject(pdfium::span_from_ref(str), pdfium::span<float>(), 0.0f);
  }
}

void CPDF_StreamContentParser::Handle_ShowText_Positioning() {
  RetainPtr<CPDF_Array> pArray = ToArray(GetObject(0));
  if (!pArray) {
    return;
  }

  size_t n = pArray->size();
  size_t nsegs = 0;
  for (size_t i = 0; i < n; i++) {
    RetainPtr<const CPDF_Object> pDirectObject = pArray->GetDirectObjectAt(i);
    if (pDirectObject && pDirectObject->IsString()) {
      nsegs++;
    }
  }
  if (nsegs == 0) {
    for (size_t i = 0; i < n; i++) {
      float fKerning = pArray->GetFloatAt(i);
      if (fKerning != 0) {
        cur_states_->IncrementTextPositionX(-GetHorizontalTextSize(fKerning));
      }
    }
    return;
  }
  std::vector<ByteString> strs(nsegs);
  std::vector<float> kernings(nsegs);
  size_t iSegment = 0;
  float fInitKerning = 0;
  for (size_t i = 0; i < n; i++) {
    RetainPtr<const CPDF_Object> pObj = pArray->GetDirectObjectAt(i);
    if (!pObj) {
      continue;
    }

    if (pObj->IsString()) {
      ByteString str = pObj->GetString();
      if (str.IsEmpty()) {
        continue;
      }
      strs[iSegment] = std::move(str);
      kernings[iSegment++] = 0;
    } else {
      float num = pObj->GetNumber();
      if (iSegment == 0) {
        fInitKerning += num;
      } else {
        kernings[iSegment - 1] += num;
      }
    }
  }
  AddTextObject(pdfium::span(strs).first(iSegment), kernings, fInitKerning);
}

void CPDF_StreamContentParser::Handle_SetTextLeading() {
  cur_states_->set_text_leading(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetTextMatrix() {
  cur_states_->set_text_matrix(GetMatrix());
  OnChangeTextMatrix();
  cur_states_->ResetTextPosition();
}

void CPDF_StreamContentParser::OnChangeTextMatrix() {
  CFX_Matrix text_matrix(cur_states_->text_horz_scale(), 0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f);
  text_matrix.Concat(cur_states_->text_matrix());
  text_matrix.Concat(cur_states_->current_transformation_matrix());
  text_matrix.Concat(mt_content_to_user_);
  pdfium::span<float> pTextMatrix =
      cur_states_->mutable_text_state().GetMutableMatrix();
  pTextMatrix[0] = text_matrix.a;
  pTextMatrix[1] = text_matrix.c;
  pTextMatrix[2] = text_matrix.b;
  pTextMatrix[3] = text_matrix.d;
}

void CPDF_StreamContentParser::Handle_SetTextRenderMode() {
  TextRenderingMode mode;
  if (SetTextRenderingModeFromInt(GetInteger(0), &mode)) {
    cur_states_->mutable_text_state().SetTextMode(mode);
  }
}

void CPDF_StreamContentParser::Handle_SetTextRise() {
  cur_states_->set_text_rise(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetWordSpace() {
  cur_states_->mutable_text_state().SetWordSpace(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_SetHorzScale() {
  if (param_count_ != 1) {
    return;
  }
  cur_states_->set_text_horz_scale(GetNumber(0) / 100);
  OnChangeTextMatrix();
}

void CPDF_StreamContentParser::Handle_MoveToNextLine() {
  cur_states_->MoveTextToNextLine();
}

void CPDF_StreamContentParser::Handle_CurveTo_23() {
  AddPathPoint(path_current_, CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(2), CFX_Path::Point::Type::kBezier);
  AddPathPoint(GetPoint(0), CFX_Path::Point::Type::kBezier);
}

void CPDF_StreamContentParser::Handle_SetLineWidth() {
  cur_states_->mutable_graph_state().SetLineWidth(GetNumber(0));
}

void CPDF_StreamContentParser::Handle_Clip() {
  path_clip_type_ = CFX_FillRenderOptions::FillType::kWinding;
}

void CPDF_StreamContentParser::Handle_EOClip() {
  path_clip_type_ = CFX_FillRenderOptions::FillType::kEvenOdd;
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
  cur_states_->mutable_text_state().SetWordSpace(GetNumber(2));
  cur_states_->mutable_text_state().SetCharSpace(GetNumber(1));
  Handle_NextLineShowText();
}

void CPDF_StreamContentParser::Handle_Invalid() {}

void CPDF_StreamContentParser::AddPathPoint(const CFX_PointF& point,
                                            CFX_Path::Point::Type type) {
  // If the path point is the same move as the previous one and neither of them
  // closes the path, then just skip it.
  if (type == CFX_Path::Point::Type::kMove && !path_points_.empty() &&
      !path_points_.back().close_figure_ && path_points_.back().type_ == type &&
      path_current_ == point) {
    return;
  }

  path_current_ = point;
  if (type == CFX_Path::Point::Type::kMove) {
    path_start_ = point;
    if (!path_points_.empty() &&
        path_points_.back().IsTypeAndOpen(CFX_Path::Point::Type::kMove)) {
      path_points_.back().point_ = point;
      return;
    }
  } else if (path_points_.empty()) {
    return;
  }
  path_points_.emplace_back(point, type, /*close=*/false);
}

void CPDF_StreamContentParser::AddPathPointAndClose(
    const CFX_PointF& point,
    CFX_Path::Point::Type type) {
  path_current_ = point;
  if (path_points_.empty()) {
    return;
  }

  path_points_.emplace_back(point, type, /*close=*/true);
}

void CPDF_StreamContentParser::AddPathObject(
    CFX_FillRenderOptions::FillType fill_type,
    RenderType render_type) {
  std::vector<CFX_Path::Point> path_points;
  path_points.swap(path_points_);
  CFX_FillRenderOptions::FillType path_clip_type = path_clip_type_;
  path_clip_type_ = CFX_FillRenderOptions::FillType::kNoFill;

  if (path_points.empty()) {
    return;
  }

  if (path_points.size() == 1) {
    if (path_clip_type != CFX_FillRenderOptions::FillType::kNoFill) {
      CPDF_Path path;
      path.AppendRect(0, 0, 0, 0);
      cur_states_->mutable_clip_path().AppendPathWithAutoMerge(
          path, CFX_FillRenderOptions::FillType::kWinding);
      return;
    }

    CFX_Path::Point& point = path_points.front();
    if (point.type_ != CFX_Path::Point::Type::kMove || !point.close_figure_ ||
        cur_states_->graph_state().GetLineCap() !=
            CFX_GraphStateData::LineCap::kRound) {
      return;
    }

    // For round line cap only: When a path moves to a point and immediately
    // gets closed, we can treat it as drawing a path from this point to itself
    // and closing the path. This should not apply to butt line cap or
    // projecting square line cap since they should not be rendered.
    point.close_figure_ = false;
    path_points.emplace_back(point.point_, CFX_Path::Point::Type::kLine,
                             /*close=*/true);
  }

  if (path_points.back().IsTypeAndOpen(CFX_Path::Point::Type::kMove)) {
    path_points.pop_back();
  }

  CPDF_Path path;
  for (const auto& point : path_points) {
    if (point.close_figure_) {
      path.AppendPointAndClose(point.point_, point.type_);
    } else {
      path.AppendPoint(point.point_, point.type_);
    }
  }

  CFX_Matrix matrix =
      cur_states_->current_transformation_matrix() * mt_content_to_user_;
  bool bStroke = render_type == RenderType::kStroke;
  if (bStroke || fill_type != CFX_FillRenderOptions::FillType::kNoFill) {
    auto pPathObj = std::make_unique<CPDF_PathObject>(GetCurrentStreamIndex());
    pPathObj->set_stroke(bStroke);
    pPathObj->set_filltype(fill_type);
    pPathObj->path() = path;
    SetGraphicStates(pPathObj.get(), true, false, true);
    pPathObj->SetPathMatrix(matrix);
    object_holder_->AppendPageObject(std::move(pPathObj));
  }
  if (path_clip_type != CFX_FillRenderOptions::FillType::kNoFill) {
    if (!matrix.IsIdentity()) {
      path.Transform(matrix);
    }
    cur_states_->mutable_clip_path().AppendPathWithAutoMerge(path,
                                                             path_clip_type);
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
  start_parse_offset_ = start_offset;
  if (recursion_state_->parsed_set.size() > kMaxFormLevel ||
      pdfium::Contains(recursion_state_->parsed_set, pDataStart.data())) {
    return fxcrt::CollectionSize<uint32_t>(pDataStart);
  }

  stream_start_offsets_ = stream_start_offsets;

  ScopedSetInsertion scoped_insert(&recursion_state_->parsed_set,
                                   pDataStart.data());

  uint32_t init_obj_count = object_holder_->GetPageObjectCount();
  AutoNuller<std::unique_ptr<CPDF_StreamParser>> auto_clearer(&syntax_);
  syntax_ = std::make_unique<CPDF_StreamParser>(pDataStart,
                                                document_->GetByteStringPool());

  while (true) {
    uint32_t cost = object_holder_->GetPageObjectCount() - init_obj_count;
    if (max_cost && cost >= max_cost) {
      break;
    }
    switch (syntax_->ParseNextElement()) {
      case CPDF_StreamParser::ElementType::kEndOfData:
        return syntax_->GetPos();
      case CPDF_StreamParser::ElementType::kKeyword:
        OnOperator(syntax_->GetWord());
        ClearAllParams();
        break;
      case CPDF_StreamParser::ElementType::kNumber:
        AddNumberParam(syntax_->GetWord());
        break;
      case CPDF_StreamParser::ElementType::kName: {
        auto word = syntax_->GetWord();
        AddNameParam(word.Last(word.GetLength() - 1));
        break;
      }
      default:
        AddObjectParam(syntax_->GetObject());
    }
  }
  return syntax_->GetPos();
}

void CPDF_StreamContentParser::ParsePathObject() {
  std::array<float, 6> params = {};
  int nParams = 0;
  int last_pos = syntax_->GetPos();
  while (true) {
    CPDF_StreamParser::ElementType type = syntax_->ParseNextElement();
    bool bProcessed = true;
    switch (type) {
      case CPDF_StreamParser::ElementType::kEndOfData:
        return;
      case CPDF_StreamParser::ElementType::kKeyword: {
        ByteStringView strc = syntax_->GetWord();
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
              AddPathPoint(path_current_, CFX_Path::Point::Type::kBezier);
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
          last_pos = syntax_->GetPos();
        }
        break;
      }
      case CPDF_StreamParser::ElementType::kNumber: {
        if (nParams == 6) {
          break;
        }

        FX_Number number(syntax_->GetWord());
        params[nParams++] = number.GetFloat();
        break;
      }
      default:
        bProcessed = false;
    }
    if (!bProcessed) {
      syntax_->SetPos(last_pos);
      return;
    }
  }
}

// static
ByteStringView CPDF_StreamContentParser::FindKeyAbbreviationForTesting(
    ByteStringView abbr) {
  return FindFullName(kInlineKeyAbbr, abbr);
}

// static
ByteStringView CPDF_StreamContentParser::FindValueAbbreviationForTesting(
    ByteStringView abbr) {
  return FindFullName(kInlineValueAbbr, abbr);
}
