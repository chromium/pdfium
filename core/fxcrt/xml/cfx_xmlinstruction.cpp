// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlinstruction.h"

#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLInstruction::CFX_XMLInstruction(const WideString& wsTarget)
    : CFX_XMLNode(), name_(wsTarget) {}

CFX_XMLInstruction::~CFX_XMLInstruction() = default;

FX_XMLNODETYPE CFX_XMLInstruction::GetType() const {
  return FX_XMLNODE_Instruction;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLInstruction::Clone() {
  auto pClone = pdfium::MakeUnique<CFX_XMLInstruction>(name_);
  pClone->m_TargetData = m_TargetData;
  return std::move(pClone);
}

void CFX_XMLInstruction::AppendData(const WideString& wsData) {
  m_TargetData.push_back(wsData);
}

bool CFX_XMLInstruction::IsOriginalXFAVersion() const {
  return name_ == L"originalXFAVersion";
}

bool CFX_XMLInstruction::IsAcrobat() const {
  return name_ == L"acrobat";
}

void CFX_XMLInstruction::Save(
    const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream) {
  if (name_.CompareNoCase(L"xml") == 0) {
    WideString ws = L"<?xml version=\"1.0\" encoding=\"";
    uint16_t wCodePage = pXMLStream->GetCodePage();
    if (wCodePage == FX_CODEPAGE_UTF16LE)
      ws += L"UTF-16";
    else if (wCodePage == FX_CODEPAGE_UTF16BE)
      ws += L"UTF-16be";
    else
      ws += L"UTF-8";

    ws += L"\"?>";
    pXMLStream->WriteString(ws.AsStringView());
    return;
  }

  pXMLStream->WriteString(
      WideString::Format(L"<?%ls", name_.c_str()).AsStringView());

  for (const WideString& target : m_TargetData) {
    pXMLStream->WriteString(L"\"");
    pXMLStream->WriteString(target.AsStringView());
    pXMLStream->WriteString(L"\"");
  }

  pXMLStream->WriteString(WideStringView(L"?>"));
}
