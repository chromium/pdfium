// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_shadingpattern.h"

#include <algorithm>
#include <utility>

#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/notreached.h"

namespace {

ShadingType ToShadingType(int type) {
  return (type > kInvalidShading && type < kMaxShading)
             ? static_cast<ShadingType>(type)
             : kInvalidShading;
}

}  // namespace

CPDF_ShadingPattern::CPDF_ShadingPattern(CPDF_Document* doc,
                                         RetainPtr<CPDF_Object> pPatternObj,
                                         bool bShading,
                                         const CFX_Matrix& parentMatrix)
    : CPDF_Pattern(doc, std::move(pPatternObj), parentMatrix),
      shading_(bShading) {
  DCHECK(document());
  if (!bShading) {
    SetPatternToFormMatrix();
  }
}

CPDF_ShadingPattern::~CPDF_ShadingPattern() = default;

CPDF_ShadingPattern* CPDF_ShadingPattern::AsShadingPattern() {
  return this;
}

bool CPDF_ShadingPattern::Load() {
  if (shading_type_ != kInvalidShading) {
    return true;
  }

  RetainPtr<const CPDF_Object> pShadingObj = GetShadingObject();
  RetainPtr<const CPDF_Dictionary> pShadingDict =
      pShadingObj ? pShadingObj->GetDict() : nullptr;
  if (!pShadingDict) {
    return false;
  }

  functions_.clear();
  RetainPtr<const CPDF_Object> pFunc =
      pShadingDict->GetDirectObjectFor("Function");
  if (pFunc) {
    if (const CPDF_Array* pArray = pFunc->AsArray()) {
      functions_.resize(std::min<size_t>(pArray->size(), 4));
      for (size_t i = 0; i < functions_.size(); ++i) {
        functions_[i] = CPDF_Function::Load(pArray->GetDirectObjectAt(i));
      }
    } else {
      functions_.push_back(CPDF_Function::Load(std::move(pFunc)));
    }
  }
  RetainPtr<const CPDF_Object> pCSObj =
      pShadingDict->GetDirectObjectFor("ColorSpace");
  if (!pCSObj) {
    return false;
  }

  auto* pDocPageData = CPDF_DocPageData::FromDocument(document());
  cs_ = pDocPageData->GetColorSpace(pCSObj.Get(), nullptr);

  // The color space is required and cannot be a Pattern space, according to the
  // PDF 1.7 spec, page 305.
  if (!cs_ || cs_->GetFamily() == CPDF_ColorSpace::Family::kPattern) {
    return false;
  }

  shading_type_ = ToShadingType(pShadingDict->GetIntegerFor("ShadingType"));
  return Validate();
}

RetainPtr<const CPDF_Object> CPDF_ShadingPattern::GetShadingObject() const {
  return shading_ ? pattern_obj()
                  : pattern_obj()->GetDict()->GetDirectObjectFor("Shading");
}

bool CPDF_ShadingPattern::Validate() const {
  if (shading_type_ == kInvalidShading) {
    return false;
  }

  // We expect to have a stream if our shading type is a mesh.
  if (IsMeshShading() && !ToStream(GetShadingObject())) {
    return false;
  }

  // Validate color space
  switch (shading_type_) {
    case kFunctionBasedShading:
    case kAxialShading:
    case kRadialShading: {
      if (cs_->GetFamily() == CPDF_ColorSpace::Family::kIndexed) {
        return false;
      }
      break;
    }
    case kFreeFormGouraudTriangleMeshShading:
    case kLatticeFormGouraudTriangleMeshShading:
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      if (!functions_.empty() &&
          cs_->GetFamily() == CPDF_ColorSpace::Family::kIndexed) {
        return false;
      }
      break;
    }
    default: {
      NOTREACHED();
    }
  }

  uint32_t nNumColorSpaceComponents = cs_->ComponentCount();
  switch (shading_type_) {
    case kFunctionBasedShading: {
      // Either one 2-to-N function or N 2-to-1 functions.
      return ValidateFunctions(1, 2, nNumColorSpaceComponents) ||
             ValidateFunctions(nNumColorSpaceComponents, 2, 1);
    }
    case kAxialShading:
    case kRadialShading: {
      // Either one 1-to-N function or N 1-to-1 functions.
      return ValidateFunctions(1, 1, nNumColorSpaceComponents) ||
             ValidateFunctions(nNumColorSpaceComponents, 1, 1);
    }
    case kFreeFormGouraudTriangleMeshShading:
    case kLatticeFormGouraudTriangleMeshShading:
    case kCoonsPatchMeshShading:
    case kTensorProductPatchMeshShading: {
      // Either no function, one 1-to-N function, or N 1-to-1 functions.
      return functions_.empty() ||
             ValidateFunctions(1, 1, nNumColorSpaceComponents) ||
             ValidateFunctions(nNumColorSpaceComponents, 1, 1);
    }
    default:
      NOTREACHED();
  }
}

bool CPDF_ShadingPattern::ValidateFunctions(
    uint32_t nExpectedNumFunctions,
    uint32_t nExpectedNumInputs,
    uint32_t nExpectedNumOutputs) const {
  if (functions_.size() != nExpectedNumFunctions) {
    return false;
  }

  FX_SAFE_UINT32 nTotalOutputs = 0;
  for (const auto& function : functions_) {
    if (!function) {
      return false;
    }

    if (function->InputCount() != nExpectedNumInputs ||
        function->OutputCount() != nExpectedNumOutputs) {
      return false;
    }

    nTotalOutputs += function->OutputCount();
  }

  return nTotalOutputs.IsValid();
}
