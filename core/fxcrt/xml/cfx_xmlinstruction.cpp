// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlinstruction.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLInstruction::CFX_XMLInstruction(const WideString& wsTarget)
    : CFX_XMLAttributeNode(wsTarget) {}

CFX_XMLInstruction::~CFX_XMLInstruction() {}

FX_XMLNODETYPE CFX_XMLInstruction::GetType() const {
  return FX_XMLNODE_Instruction;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLInstruction::Clone() {
  auto pClone = pdfium::MakeUnique<CFX_XMLInstruction>(GetName());
  pClone->SetAttributes(GetAttributes());
  pClone->m_TargetData = m_TargetData;
  return std::move(pClone);
}

void CFX_XMLInstruction::AppendData(const WideString& wsData) {
  m_TargetData.push_back(wsData);
}

void CFX_XMLInstruction::RemoveData(int32_t index) {
  if (pdfium::IndexInBounds(m_TargetData, index))
    m_TargetData.erase(m_TargetData.begin() + index);
}
