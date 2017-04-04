// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlinstruction.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFDE_XMLInstruction::CFDE_XMLInstruction(const CFX_WideString& wsTarget)
    : m_wsTarget(wsTarget) {
  ASSERT(m_wsTarget.GetLength() > 0);
}

CFDE_XMLInstruction::~CFDE_XMLInstruction() {}

FDE_XMLNODETYPE CFDE_XMLInstruction::GetType() const {
  return FDE_XMLNODE_Instruction;
}

std::unique_ptr<CFDE_XMLNode> CFDE_XMLInstruction::Clone() {
  auto pClone = pdfium::MakeUnique<CFDE_XMLInstruction>(m_wsTarget);
  pClone->m_Attributes = m_Attributes;
  pClone->m_TargetData = m_TargetData;
  return pClone;
}

int32_t CFDE_XMLInstruction::CountAttributes() const {
  return pdfium::CollectionSize<int32_t>(m_Attributes) / 2;
}

bool CFDE_XMLInstruction::GetAttribute(int32_t index,
                                       CFX_WideString& wsAttriName,
                                       CFX_WideString& wsAttriValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  ASSERT(index > -1 && index < iCount / 2);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (index == 0) {
      wsAttriName = m_Attributes[i];
      wsAttriValue = m_Attributes[i + 1];
      return true;
    }
    index--;
  }
  return false;
}

bool CFDE_XMLInstruction::HasAttribute(const wchar_t* pwsAttriName) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return true;
    }
  }
  return false;
}

void CFDE_XMLInstruction::GetString(const wchar_t* pwsAttriName,
                                    CFX_WideString& wsAttriValue,
                                    const wchar_t* pwsDefValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      wsAttriValue = m_Attributes[i + 1];
      return;
    }
  }
  wsAttriValue = pwsDefValue;
}

void CFDE_XMLInstruction::SetString(const CFX_WideString& wsAttriName,
                                    const CFX_WideString& wsAttriValue) {
  ASSERT(wsAttriName.GetLength() > 0);
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(wsAttriName) == 0) {
      m_Attributes[i] = wsAttriName;
      m_Attributes[i + 1] = wsAttriValue;
      return;
    }
  }
  m_Attributes.push_back(wsAttriName);
  m_Attributes.push_back(wsAttriValue);
}

int32_t CFDE_XMLInstruction::GetInteger(const wchar_t* pwsAttriName,
                                        int32_t iDefValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wtoi(m_Attributes[i + 1].c_str());
    }
  }
  return iDefValue;
}

void CFDE_XMLInstruction::SetInteger(const wchar_t* pwsAttriName,
                                     int32_t iAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", iAttriValue);
  SetString(pwsAttriName, wsValue);
}

float CFDE_XMLInstruction::GetFloat(const wchar_t* pwsAttriName,
                                    float fDefValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wcstof(m_Attributes[i + 1].c_str(), -1, nullptr);
    }
  }
  return fDefValue;
}

void CFDE_XMLInstruction::SetFloat(const wchar_t* pwsAttriName,
                                   float fAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%f", fAttriValue);
  SetString(pwsAttriName, wsValue);
}

void CFDE_XMLInstruction::RemoveAttribute(const wchar_t* pwsAttriName) {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      m_Attributes.erase(m_Attributes.begin() + i,
                         m_Attributes.begin() + i + 2);
      return;
    }
  }
}

int32_t CFDE_XMLInstruction::CountData() const {
  return pdfium::CollectionSize<int32_t>(m_TargetData);
}

bool CFDE_XMLInstruction::GetData(int32_t index, CFX_WideString& wsData) const {
  if (!pdfium::IndexInBounds(m_TargetData, index))
    return false;

  wsData = m_TargetData[index];
  return true;
}

void CFDE_XMLInstruction::AppendData(const CFX_WideString& wsData) {
  m_TargetData.push_back(wsData);
}

void CFDE_XMLInstruction::RemoveData(int32_t index) {
  if (pdfium::IndexInBounds(m_TargetData, index))
    m_TargetData.erase(m_TargetData.begin() + index);
}
