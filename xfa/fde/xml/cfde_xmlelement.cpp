// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlelement.h"

#include "core/fxcrt/fx_ext.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/xml/cfde_xmlchardata.h"
#include "xfa/fde/xml/cfde_xmltext.h"

CFDE_XMLElement::CFDE_XMLElement(const CFX_WideString& wsTag)
    : CFDE_XMLNode(), m_wsTag(wsTag), m_Attributes() {
  ASSERT(m_wsTag.GetLength() > 0);
}

CFDE_XMLElement::~CFDE_XMLElement() {}

FDE_XMLNODETYPE CFDE_XMLElement::GetType() const {
  return FDE_XMLNODE_Element;
}

CFDE_XMLNode* CFDE_XMLElement::Clone(bool bRecursive) {
  CFDE_XMLElement* pClone = new CFDE_XMLElement(m_wsTag);
  if (!pClone)
    return nullptr;

  pClone->m_Attributes = m_Attributes;
  if (bRecursive) {
    CloneChildren(pClone);
  } else {
    CFX_WideString wsText;
    CFDE_XMLNode* pChild = m_pChild;
    while (pChild) {
      switch (pChild->GetType()) {
        case FDE_XMLNODE_Text:
          wsText += ((CFDE_XMLText*)pChild)->m_wsText;
          break;
        default:
          break;
      }
      pChild = pChild->m_pNext;
    }
    pClone->SetTextData(wsText);
  }
  return pClone;
}

void CFDE_XMLElement::GetTagName(CFX_WideString& wsTag) const {
  wsTag = m_wsTag;
}

void CFDE_XMLElement::GetLocalTagName(CFX_WideString& wsTag) const {
  FX_STRSIZE iFind = m_wsTag.Find(L':', 0);
  if (iFind < 0) {
    wsTag = m_wsTag;
  } else {
    wsTag = m_wsTag.Right(m_wsTag.GetLength() - iFind - 1);
  }
}

void CFDE_XMLElement::GetNamespacePrefix(CFX_WideString& wsPrefix) const {
  FX_STRSIZE iFind = m_wsTag.Find(L':', 0);
  if (iFind < 0) {
    wsPrefix.clear();
  } else {
    wsPrefix = m_wsTag.Left(iFind);
  }
}

void CFDE_XMLElement::GetNamespaceURI(CFX_WideString& wsNamespace) const {
  CFX_WideString wsAttri(L"xmlns"), wsPrefix;
  GetNamespacePrefix(wsPrefix);
  if (wsPrefix.GetLength() > 0) {
    wsAttri += L":";
    wsAttri += wsPrefix;
  }
  wsNamespace.clear();
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)this;
  while (pNode) {
    if (pNode->GetType() != FDE_XMLNODE_Element) {
      break;
    }
    CFDE_XMLElement* pElement = (CFDE_XMLElement*)pNode;
    if (!pElement->HasAttribute(wsAttri.c_str())) {
      pNode = pNode->GetNodeItem(CFDE_XMLNode::Parent);
      continue;
    }
    pElement->GetString(wsAttri.c_str(), wsNamespace);
    break;
  }
}

int32_t CFDE_XMLElement::CountAttributes() const {
  return pdfium::CollectionSize<int32_t>(m_Attributes) / 2;
}

bool CFDE_XMLElement::GetAttribute(int32_t index,
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

bool CFDE_XMLElement::HasAttribute(const wchar_t* pwsAttriName) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0)
      return true;
  }
  return false;
}

void CFDE_XMLElement::GetString(const wchar_t* pwsAttriName,
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

void CFDE_XMLElement::SetString(const CFX_WideString& wsAttriName,
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

int32_t CFDE_XMLElement::GetInteger(const wchar_t* pwsAttriName,
                                    int32_t iDefValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wtoi(m_Attributes[i + 1].c_str());
    }
  }
  return iDefValue;
}

void CFDE_XMLElement::SetInteger(const wchar_t* pwsAttriName,
                                 int32_t iAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%d", iAttriValue);
  SetString(pwsAttriName, wsValue);
}

float CFDE_XMLElement::GetFloat(const wchar_t* pwsAttriName,
                                float fDefValue) const {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      return FXSYS_wcstof(m_Attributes[i + 1].c_str(), -1, nullptr);
    }
  }
  return fDefValue;
}

void CFDE_XMLElement::SetFloat(const wchar_t* pwsAttriName, float fAttriValue) {
  CFX_WideString wsValue;
  wsValue.Format(L"%f", fAttriValue);
  SetString(pwsAttriName, wsValue);
}

void CFDE_XMLElement::RemoveAttribute(const wchar_t* pwsAttriName) {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Attributes);
  for (int32_t i = 0; i < iCount; i += 2) {
    if (m_Attributes[i].Compare(pwsAttriName) == 0) {
      m_Attributes.erase(m_Attributes.begin() + i,
                         m_Attributes.begin() + i + 2);
      return;
    }
  }
}

void CFDE_XMLElement::GetTextData(CFX_WideString& wsText) const {
  CFX_WideTextBuf buffer;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    switch (pChild->GetType()) {
      case FDE_XMLNODE_Text:
        buffer << ((CFDE_XMLText*)pChild)->m_wsText;
        break;
      case FDE_XMLNODE_CharData:
        buffer << ((CFDE_XMLCharData*)pChild)->m_wsCharData;
        break;
      default:
        break;
    }
    pChild = pChild->m_pNext;
  }
  wsText = buffer.AsStringC();
}

void CFDE_XMLElement::SetTextData(const CFX_WideString& wsText) {
  if (wsText.GetLength() < 1) {
    return;
  }
  InsertChildNode(new CFDE_XMLText(wsText));
}
