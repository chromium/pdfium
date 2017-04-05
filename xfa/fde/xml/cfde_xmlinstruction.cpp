// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlinstruction.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFDE_XMLInstruction::CFDE_XMLInstruction(const CFX_WideString& wsTarget)
    : CFDE_XMLAttributeNode(wsTarget) {}

CFDE_XMLInstruction::~CFDE_XMLInstruction() {}

FDE_XMLNODETYPE CFDE_XMLInstruction::GetType() const {
  return FDE_XMLNODE_Instruction;
}

std::unique_ptr<CFDE_XMLNode> CFDE_XMLInstruction::Clone() {
  auto pClone = pdfium::MakeUnique<CFDE_XMLInstruction>(GetName());
  pClone->SetAttributes(GetAttributes());
  pClone->m_TargetData = m_TargetData;
  return pClone;
}

void CFDE_XMLInstruction::AppendData(const CFX_WideString& wsData) {
  m_TargetData.push_back(wsData);
}

void CFDE_XMLInstruction::RemoveData(int32_t index) {
  if (pdfium::IndexInBounds(m_TargetData, index))
    m_TargetData.erase(m_TargetData.begin() + index);
}
