// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlnode.h"

#include <vector>

#include "third_party/base/stl_util.h"
#include "xfa/fde/xml/cfde_xmlchardata.h"
#include "xfa/fde/xml/cfde_xmlelement.h"
#include "xfa/fde/xml/cfde_xmlinstruction.h"
#include "xfa/fde/xml/cfde_xmltext.h"
#include "xfa/fgas/crt/fgas_codepage.h"

CFDE_XMLNode::CFDE_XMLNode()
    : m_pParent(nullptr),
      m_pChild(nullptr),
      m_pPrior(nullptr),
      m_pNext(nullptr) {}

FDE_XMLNODETYPE CFDE_XMLNode::GetType() const {
  return FDE_XMLNODE_Unknown;
}

CFDE_XMLNode::~CFDE_XMLNode() {
  DeleteChildren();
}

void CFDE_XMLNode::DeleteChildren() {
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    CFDE_XMLNode* pNext = pChild->m_pNext;
    delete pChild;
    pChild = pNext;
  }
  m_pChild = nullptr;
}

int32_t CFDE_XMLNode::CountChildNodes() const {
  int32_t iCount = 0;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    iCount++;
    pChild = pChild->m_pNext;
  }
  return iCount;
}

CFDE_XMLNode* CFDE_XMLNode::GetChildNode(int32_t index) const {
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    if (index == 0) {
      return pChild;
    }
    index--;
    pChild = pChild->m_pNext;
  }
  return nullptr;
}

int32_t CFDE_XMLNode::GetChildNodeIndex(CFDE_XMLNode* pNode) const {
  int32_t index = 0;
  CFDE_XMLNode* pChild = m_pChild;
  while (pChild) {
    if (pChild == pNode) {
      return index;
    }
    index++;
    pChild = pChild->m_pNext;
  }
  return -1;
}

CFDE_XMLNode* CFDE_XMLNode::GetPath(const wchar_t* pPath,
                                    int32_t iLength,
                                    bool bQualifiedName) const {
  ASSERT(pPath);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pPath);
  }
  if (iLength == 0) {
    return nullptr;
  }
  CFX_WideString csPath;
  const wchar_t* pStart = pPath;
  const wchar_t* pEnd = pPath + iLength;
  wchar_t ch;
  while (pStart < pEnd) {
    ch = *pStart++;
    if (ch == L'/') {
      break;
    } else {
      csPath += ch;
    }
  }
  iLength -= pStart - pPath;
  CFDE_XMLNode* pFind = nullptr;
  if (csPath.GetLength() < 1) {
    pFind = GetNodeItem(CFDE_XMLNode::Root);
  } else if (csPath.Compare(L"..") == 0) {
    pFind = m_pParent;
  } else if (csPath.Compare(L".") == 0) {
    pFind = (CFDE_XMLNode*)this;
  } else {
    CFX_WideString wsTag;
    CFDE_XMLNode* pNode = m_pChild;
    while (pNode) {
      if (pNode->GetType() == FDE_XMLNODE_Element) {
        if (bQualifiedName)
          wsTag = static_cast<CFDE_XMLElement*>(pNode)->GetName();
        else
          wsTag = static_cast<CFDE_XMLElement*>(pNode)->GetLocalTagName();

        if (wsTag.Compare(csPath) == 0) {
          if (iLength < 1)
            pFind = pNode;
          else
            pFind = pNode->GetPath(pStart, iLength, bQualifiedName);

          if (pFind)
            return pFind;
        }
      }
      pNode = pNode->m_pNext;
    }
  }
  if (!pFind || iLength < 1)
    return pFind;
  return pFind->GetPath(pStart, iLength, bQualifiedName);
}

int32_t CFDE_XMLNode::InsertChildNode(CFDE_XMLNode* pNode, int32_t index) {
  pNode->m_pParent = this;
  if (!m_pChild) {
    m_pChild = pNode;
    pNode->m_pPrior = nullptr;
    pNode->m_pNext = nullptr;
    return 0;
  }
  if (index == 0) {
    pNode->m_pNext = m_pChild;
    pNode->m_pPrior = nullptr;
    m_pChild->m_pPrior = pNode;
    m_pChild = pNode;
    return 0;
  }
  int32_t iCount = 0;
  CFDE_XMLNode* pFind = m_pChild;
  while (++iCount != index && pFind->m_pNext) {
    pFind = pFind->m_pNext;
  }
  pNode->m_pPrior = pFind;
  pNode->m_pNext = pFind->m_pNext;
  if (pFind->m_pNext)
    pFind->m_pNext->m_pPrior = pNode;
  pFind->m_pNext = pNode;
  return iCount;
}

void CFDE_XMLNode::RemoveChildNode(CFDE_XMLNode* pNode) {
  ASSERT(m_pChild && pNode);
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
  } else {
    pNode->m_pPrior->m_pNext = pNode->m_pNext;
  }
  if (pNode->m_pNext)
    pNode->m_pNext->m_pPrior = pNode->m_pPrior;
  pNode->m_pParent = nullptr;
  pNode->m_pNext = nullptr;
  pNode->m_pPrior = nullptr;
}

CFDE_XMLNode* CFDE_XMLNode::GetNodeItem(CFDE_XMLNode::NodeItem eItem) const {
  switch (eItem) {
    case CFDE_XMLNode::Root: {
      CFDE_XMLNode* pParent = (CFDE_XMLNode*)this;
      while (pParent->m_pParent) {
        pParent = pParent->m_pParent;
      }
      return pParent;
    }
    case CFDE_XMLNode::Parent:
      return m_pParent;
    case CFDE_XMLNode::FirstSibling: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pPrior) {
        pItem = pItem->m_pPrior;
      }
      return pItem == (CFDE_XMLNode*)this ? nullptr : pItem;
    }
    case CFDE_XMLNode::PriorSibling:
      return m_pPrior;
    case CFDE_XMLNode::NextSibling:
      return m_pNext;
    case CFDE_XMLNode::LastSibling: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pNext)
        pItem = pItem->m_pNext;
      return pItem == (CFDE_XMLNode*)this ? nullptr : pItem;
    }
    case CFDE_XMLNode::FirstNeighbor: {
      CFDE_XMLNode* pParent = (CFDE_XMLNode*)this;
      while (pParent->m_pParent)
        pParent = pParent->m_pParent;
      return pParent == (CFDE_XMLNode*)this ? nullptr : pParent;
    }
    case CFDE_XMLNode::PriorNeighbor: {
      if (!m_pPrior)
        return m_pParent;

      CFDE_XMLNode* pItem = m_pPrior;
      while (pItem->m_pChild) {
        pItem = pItem->m_pChild;
        while (pItem->m_pNext)
          pItem = pItem->m_pNext;
      }
      return pItem;
    }
    case CFDE_XMLNode::NextNeighbor: {
      if (m_pChild)
        return m_pChild;
      if (m_pNext)
        return m_pNext;
      CFDE_XMLNode* pItem = m_pParent;
      while (pItem) {
        if (pItem->m_pNext)
          return pItem->m_pNext;
        pItem = pItem->m_pParent;
      }
      return nullptr;
    }
    case CFDE_XMLNode::LastNeighbor: {
      CFDE_XMLNode* pItem = (CFDE_XMLNode*)this;
      while (pItem->m_pParent) {
        pItem = pItem->m_pParent;
      }
      while (true) {
        while (pItem->m_pNext)
          pItem = pItem->m_pNext;
        if (!pItem->m_pChild)
          break;
        pItem = pItem->m_pChild;
      }
      return pItem == (CFDE_XMLNode*)this ? nullptr : pItem;
    }
    case CFDE_XMLNode::FirstChild:
      return m_pChild;
    case CFDE_XMLNode::LastChild: {
      if (!m_pChild)
        return nullptr;

      CFDE_XMLNode* pChild = m_pChild;
      while (pChild->m_pNext)
        pChild = pChild->m_pNext;
      return pChild;
    }
    default:
      break;
  }
  return nullptr;
}

int32_t CFDE_XMLNode::GetNodeLevel() const {
  int32_t iLevel = 0;
  const CFDE_XMLNode* pItem = m_pParent;
  while (pItem) {
    iLevel++;
    pItem = pItem->m_pParent;
  }
  return iLevel;
}

bool CFDE_XMLNode::InsertNodeItem(CFDE_XMLNode::NodeItem eItem,
                                  CFDE_XMLNode* pNode) {
  switch (eItem) {
    case CFDE_XMLNode::NextSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = m_pNext;
      pNode->m_pPrior = this;
      if (m_pNext) {
        m_pNext->m_pPrior = pNode;
      }
      m_pNext = pNode;
      return true;
    }
    case CFDE_XMLNode::PriorSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = this;
      pNode->m_pPrior = m_pPrior;
      if (m_pPrior) {
        m_pPrior->m_pNext = pNode;
      } else if (m_pParent) {
        m_pParent->m_pChild = pNode;
      }
      m_pPrior = pNode;
      return true;
    }
    default:
      return false;
  }
}

CFDE_XMLNode* CFDE_XMLNode::RemoveNodeItem(CFDE_XMLNode::NodeItem eItem) {
  CFDE_XMLNode* pNode = nullptr;
  switch (eItem) {
    case CFDE_XMLNode::NextSibling:
      if (m_pNext) {
        pNode = m_pNext;
        m_pNext = pNode->m_pNext;
        if (m_pNext) {
          m_pNext->m_pPrior = this;
        }
        pNode->m_pParent = nullptr;
        pNode->m_pNext = nullptr;
        pNode->m_pPrior = nullptr;
      }
      break;
    default:
      break;
  }
  return pNode;
}

std::unique_ptr<CFDE_XMLNode> CFDE_XMLNode::Clone() {
  return nullptr;
}

void CFDE_XMLNode::SaveXMLNode(const CFX_RetainPtr<IFGAS_Stream>& pXMLStream) {
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)this;
  switch (pNode->GetType()) {
    case FDE_XMLNODE_Instruction: {
      CFX_WideString ws;
      CFDE_XMLInstruction* pInstruction = (CFDE_XMLInstruction*)pNode;
      if (pInstruction->GetName().CompareNoCase(L"xml") == 0) {
        ws = L"<?xml version=\"1.0\" encoding=\"";
        uint16_t wCodePage = pXMLStream->GetCodePage();
        if (wCodePage == FX_CODEPAGE_UTF16LE) {
          ws += L"UTF-16";
        } else if (wCodePage == FX_CODEPAGE_UTF16BE) {
          ws += L"UTF-16be";
        } else {
          ws += L"UTF-8";
        }
        ws += L"\"?>";
        pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      } else {
        ws.Format(L"<?%s", pInstruction->GetName().c_str());
        pXMLStream->WriteString(ws.c_str(), ws.GetLength());

        for (auto it : pInstruction->GetAttributes()) {
          CFX_WideString wsValue = it.second;
          wsValue.Replace(L"&", L"&amp;");
          wsValue.Replace(L"<", L"&lt;");
          wsValue.Replace(L">", L"&gt;");
          wsValue.Replace(L"\'", L"&apos;");
          wsValue.Replace(L"\"", L"&quot;");

          ws = L" ";
          ws += it.first;
          ws += L"=\"";
          ws += wsValue;
          ws += L"\"";
          pXMLStream->WriteString(ws.c_str(), ws.GetLength());
        }

        for (auto target : pInstruction->GetTargetData()) {
          ws = L" \"";
          ws += target;
          ws += L"\"";
          pXMLStream->WriteString(ws.c_str(), ws.GetLength());
        }
        ws = L"?>";
        pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      }
      break;
    }
    case FDE_XMLNODE_Element: {
      CFX_WideString ws;
      ws = L"<";
      ws += static_cast<CFDE_XMLElement*>(pNode)->GetName();
      pXMLStream->WriteString(ws.c_str(), ws.GetLength());

      for (auto it : static_cast<CFDE_XMLElement*>(pNode)->GetAttributes()) {
        CFX_WideString wsValue = it.second;
        wsValue.Replace(L"&", L"&amp;");
        wsValue.Replace(L"<", L"&lt;");
        wsValue.Replace(L">", L"&gt;");
        wsValue.Replace(L"\'", L"&apos;");
        wsValue.Replace(L"\"", L"&quot;");

        ws = L" ";
        ws += it.first;
        ws += L"=\"";
        ws += wsValue;
        ws += L"\"";
        pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      }
      if (pNode->m_pChild) {
        ws = L"\n>";
        pXMLStream->WriteString(ws.c_str(), ws.GetLength());
        CFDE_XMLNode* pChild = pNode->m_pChild;
        while (pChild) {
          pChild->SaveXMLNode(pXMLStream);
          pChild = pChild->m_pNext;
        }
        ws = L"</";
        ws += static_cast<CFDE_XMLElement*>(pNode)->GetName();
        ws += L"\n>";
      } else {
        ws = L"\n/>";
      }
      pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      break;
    }
    case FDE_XMLNODE_Text: {
      CFX_WideString ws = static_cast<CFDE_XMLText*>(pNode)->GetText();
      ws.Replace(L"&", L"&amp;");
      ws.Replace(L"<", L"&lt;");
      ws.Replace(L">", L"&gt;");
      ws.Replace(L"\'", L"&apos;");
      ws.Replace(L"\"", L"&quot;");
      pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      break;
    }
    case FDE_XMLNODE_CharData: {
      CFX_WideString ws = L"<![CDATA[";
      ws += static_cast<CFDE_XMLCharData*>(pNode)->GetText();
      ws += L"]]>";
      pXMLStream->WriteString(ws.c_str(), ws.GetLength());
      break;
    }
    case FDE_XMLNODE_Unknown:
    default:
      break;
  }
}
