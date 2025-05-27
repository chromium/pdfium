// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_allstates.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_pageobjectholder.h"
#include "core/fpdfapi/page/cpdf_streamcontentparser.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxge/cfx_graphstatedata.h"

CPDF_AllStates::CPDF_AllStates() = default;

CPDF_AllStates::CPDF_AllStates(const CPDF_AllStates& that) = default;

CPDF_AllStates& CPDF_AllStates::operator=(const CPDF_AllStates& that) = default;

CPDF_AllStates::~CPDF_AllStates() = default;

void CPDF_AllStates::SetDefaultStates() {
  graphic_states_.SetDefaultStates();
}

void CPDF_AllStates::SetLineDash(const CPDF_Array* pArray, float phase) {
  std::vector<float> dashes = ReadArrayElementsToVector(pArray, pArray->size());
  mutable_graph_state().SetLineDash(std::move(dashes), phase);
}

void CPDF_AllStates::ProcessExtGS(const CPDF_Dictionary* pGS,
                                  CPDF_StreamContentParser* pParser) {
  CPDF_DictionaryLocker locker(pGS);
  for (const auto& it : locker) {
    RetainPtr<CPDF_Object> pObject = it.second->GetMutableDirect();
    if (!pObject) {
      continue;
    }

    uint32_t key = it.first.GetID();
    switch (key) {
      case FXBSTR_ID('L', 'W', 0, 0):
        mutable_graph_state().SetLineWidth(pObject->GetNumber());
        break;
      case FXBSTR_ID('L', 'C', 0, 0):
        mutable_graph_state().SetLineCap(
            static_cast<CFX_GraphStateData::LineCap>(pObject->GetInteger()));
        break;
      case FXBSTR_ID('L', 'J', 0, 0):
        mutable_graph_state().SetLineJoin(
            static_cast<CFX_GraphStateData::LineJoin>(pObject->GetInteger()));
        break;
      case FXBSTR_ID('M', 'L', 0, 0):
        mutable_graph_state().SetMiterLimit(pObject->GetNumber());
        break;
      case FXBSTR_ID('D', 0, 0, 0): {
        const CPDF_Array* pDash = pObject->AsArray();
        if (!pDash) {
          break;
        }

        RetainPtr<const CPDF_Array> pArray = pDash->GetArrayAt(0);
        if (!pArray) {
          break;
        }

        SetLineDash(pArray.Get(), pDash->GetFloatAt(1));
        break;
      }
      case FXBSTR_ID('R', 'I', 0, 0):
        mutable_general_state().SetRenderIntent(pObject->GetString());
        break;
      case FXBSTR_ID('F', 'o', 'n', 't'): {
        const CPDF_Array* font = pObject->AsArray();
        if (!font) {
          break;
        }

        mutable_text_state().SetFontSize(font->GetFloatAt(1));
        mutable_text_state().SetFont(
            pParser->FindFont(font->GetByteStringAt(0)));
        break;
      }
      case FXBSTR_ID('T', 'R', 0, 0):
        if (pGS->KeyExist("TR2")) {
          continue;
        }
        [[fallthrough]];
      case FXBSTR_ID('T', 'R', '2', 0):
        mutable_general_state().SetTR(!pObject->IsName() ? std::move(pObject)
                                                         : nullptr);
        break;
      case FXBSTR_ID('B', 'M', 0, 0): {
        const CPDF_Array* pArray = pObject->AsArray();
        mutable_general_state().SetBlendMode(pArray ? pArray->GetByteStringAt(0)
                                                    : pObject->GetString());
        if (general_state().GetBlendType() > BlendMode::kMultiply) {
          pParser->GetPageObjectHolder()->SetBackgroundAlphaNeeded(true);
        }
        break;
      }
      case FXBSTR_ID('S', 'M', 'a', 's'): {
        RetainPtr<CPDF_Dictionary> pMaskDict = ToDictionary(pObject);
        mutable_general_state().SetSoftMask(pMaskDict);
        if (pMaskDict) {
          mutable_general_state().SetSMaskMatrix(pParser->GetCurStates()->ctm_);
        }
        break;
      }
      case FXBSTR_ID('C', 'A', 0, 0):
        mutable_general_state().SetStrokeAlpha(
            std::clamp(pObject->GetNumber(), 0.0f, 1.0f));
        break;
      case FXBSTR_ID('c', 'a', 0, 0):
        mutable_general_state().SetFillAlpha(
            std::clamp(pObject->GetNumber(), 0.0f, 1.0f));
        break;
      case FXBSTR_ID('O', 'P', 0, 0):
        mutable_general_state().SetStrokeOP(!!pObject->GetInteger());
        if (!pGS->KeyExist("op")) {
          mutable_general_state().SetFillOP(!!pObject->GetInteger());
        }
        break;
      case FXBSTR_ID('o', 'p', 0, 0):
        mutable_general_state().SetFillOP(!!pObject->GetInteger());
        break;
      case FXBSTR_ID('O', 'P', 'M', 0):
        mutable_general_state().SetOPMode(pObject->GetInteger());
        break;
      case FXBSTR_ID('B', 'G', 0, 0):
        if (pGS->KeyExist("BG2")) {
          continue;
        }
        [[fallthrough]];
      case FXBSTR_ID('B', 'G', '2', 0):
        mutable_general_state().SetBG(std::move(pObject));
        break;
      case FXBSTR_ID('U', 'C', 'R', 0):
        if (pGS->KeyExist("UCR2")) {
          continue;
        }
        [[fallthrough]];
      case FXBSTR_ID('U', 'C', 'R', '2'):
        mutable_general_state().SetUCR(std::move(pObject));
        break;
      case FXBSTR_ID('H', 'T', 0, 0):
        mutable_general_state().SetHT(std::move(pObject));
        break;
      case FXBSTR_ID('F', 'L', 0, 0):
        mutable_general_state().SetFlatness(pObject->GetNumber());
        break;
      case FXBSTR_ID('S', 'M', 0, 0):
        mutable_general_state().SetSmoothness(pObject->GetNumber());
        break;
      case FXBSTR_ID('S', 'A', 0, 0):
        mutable_general_state().SetStrokeAdjust(!!pObject->GetInteger());
        break;
      case FXBSTR_ID('A', 'I', 'S', 0):
        mutable_general_state().SetAlphaSource(!!pObject->GetInteger());
        break;
      case FXBSTR_ID('T', 'K', 0, 0):
        mutable_general_state().SetTextKnockout(!!pObject->GetInteger());
        break;
    }
  }
}

void CPDF_AllStates::ResetTextPosition() {
  text_line_pos_ = CFX_PointF();
  text_pos_ = CFX_PointF();
}

CFX_PointF CPDF_AllStates::GetTransformedTextPosition() const {
  return ctm_.Transform(text_matrix_.Transform(
      CFX_PointF(text_pos_.x, text_pos_.y + text_rise_)));
}

void CPDF_AllStates::MoveTextPoint(const CFX_PointF& point) {
  text_line_pos_ += point;
  text_pos_ = text_line_pos_;
}

void CPDF_AllStates::MoveTextToNextLine() {
  text_line_pos_.y -= text_leading_;
  text_pos_ = text_line_pos_;
}

void CPDF_AllStates::IncrementTextPositionX(float value) {
  text_pos_.x += value;
}

void CPDF_AllStates::IncrementTextPositionY(float value) {
  text_pos_.y += value;
}
