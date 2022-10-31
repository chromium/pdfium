// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_occur.h"

#include "fxjs/xfa/cjx_occur.h"
#include "xfa/fxfa/parser/cxfa_document.h"

namespace {

const CXFA_Node::PropertyData kOccurPropertyData[] = {
    {XFA_Element::Extras, 1, {}},
};

const CXFA_Node::AttributeData kOccurAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Max, XFA_AttributeType::Integer, (void*)1},
    {XFA_Attribute::Min, XFA_AttributeType::Integer, (void*)1},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Initial, XFA_AttributeType::Integer, (void*)1},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
};

}  // namespace

CXFA_Occur::CXFA_Occur(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(doc,
                packet,
                {XFA_XDPPACKET::kTemplate, XFA_XDPPACKET::kForm},
                XFA_ObjectType::Node,
                XFA_Element::Occur,
                kOccurPropertyData,
                kOccurAttributeData,
                cppgc::MakeGarbageCollected<CJX_Occur>(
                    doc->GetHeap()->GetAllocationHandle(),
                    this)) {}

CXFA_Occur::~CXFA_Occur() = default;

int32_t CXFA_Occur::GetMax() {
  absl::optional<int32_t> max =
      JSObject()->TryInteger(XFA_Attribute::Max, true);
  return max.has_value() ? max.value() : GetMin();
}

int32_t CXFA_Occur::GetMin() {
  absl::optional<int32_t> min =
      JSObject()->TryInteger(XFA_Attribute::Min, true);
  return min.has_value() && min.value() >= 0 ? min.value() : 1;
}

std::tuple<int32_t, int32_t, int32_t> CXFA_Occur::GetOccurInfo() {
  int32_t iMin = GetMin();
  int32_t iMax = GetMax();

  absl::optional<int32_t> init =
      JSObject()->TryInteger(XFA_Attribute::Initial, false);
  return {iMin, iMax,
          init.has_value() && init.value() >= iMin ? init.value() : iMin};
}

void CXFA_Occur::SetMax(int32_t iMax) {
  iMax = (iMax != -1 && iMax < 1) ? 1 : iMax;
  JSObject()->SetInteger(XFA_Attribute::Max, iMax, false);

  int32_t iMin = GetMin();
  if (iMax != -1 && iMax < iMin) {
    iMin = iMax;
    JSObject()->SetInteger(XFA_Attribute::Min, iMin, false);
  }
}

void CXFA_Occur::SetMin(int32_t iMin) {
  iMin = (iMin < 0) ? 1 : iMin;
  JSObject()->SetInteger(XFA_Attribute::Min, iMin, false);

  int32_t iMax = GetMax();
  if (iMax > 0 && iMax < iMin) {
    iMax = iMin;
    JSObject()->SetInteger(XFA_Attribute::Max, iMax, false);
  }
}
