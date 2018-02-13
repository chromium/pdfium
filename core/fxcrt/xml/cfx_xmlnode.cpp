// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

#include <vector>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/stl_util.h"

CFX_XMLNode::CFX_XMLNode()
    : m_pParent(nullptr),
      m_pChild(nullptr),
      m_pPrior(nullptr),
      m_pNext(nullptr) {}

FX_XMLNODETYPE CFX_XMLNode::GetType() const {
  return FX_XMLNODE_Unknown;
}

CFX_XMLNode::~CFX_XMLNode() {
  DeleteChildren();
}

void CFX_XMLNode::DeleteChildren() {
  CFX_XMLNode* pChild = m_pChild;
  while (pChild) {
    CFX_XMLNode* pNext = pChild->m_pNext;
    delete pChild;
    pChild = pNext;
  }
  m_pChild = nullptr;
}

void CFX_XMLNode::AppendChild(CFX_XMLNode* pNode) {
  InsertChildNode(pNode, -1);
}

void CFX_XMLNode::InsertChildNode(CFX_XMLNode* pNode, int32_t index) {
  ASSERT(!pNode->m_pParent);

  pNode->m_pParent = this;
  if (!m_pChild) {
    m_pChild = pNode;
    pNode->m_pPrior = nullptr;
    pNode->m_pNext = nullptr;
    return;
  }
  if (index == 0) {
    pNode->m_pNext = m_pChild;
    pNode->m_pPrior = nullptr;
    m_pChild->m_pPrior = pNode;
    m_pChild = pNode;
    return;
  }

  int32_t iCount = 0;
  CFX_XMLNode* pFind = m_pChild;
  while (++iCount != index && pFind->m_pNext)
    pFind = pFind->m_pNext;

  pNode->m_pPrior = pFind;
  pNode->m_pNext = pFind->m_pNext;
  if (pFind->m_pNext)
    pFind->m_pNext->m_pPrior = pNode;
  pFind->m_pNext = pNode;
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(m_pChild && pNode);

  if (m_pChild == pNode)
    m_pChild = pNode->m_pNext;
  else
    pNode->m_pPrior->m_pNext = pNode->m_pNext;

  if (pNode->m_pNext)
    pNode->m_pNext->m_pPrior = pNode->m_pPrior;

  pNode->m_pParent = nullptr;
  pNode->m_pNext = nullptr;
  pNode->m_pPrior = nullptr;
}

CFX_XMLNode* CFX_XMLNode::GetNodeItem(CFX_XMLNode::NodeItem eItem) const {
  switch (eItem) {
    case CFX_XMLNode::Root: {
      CFX_XMLNode* pParent = (CFX_XMLNode*)this;
      while (pParent->m_pParent)
        pParent = pParent->m_pParent;

      return pParent;
    }
    case CFX_XMLNode::Parent:
      return m_pParent;
    case CFX_XMLNode::NextSibling:
      return m_pNext;
    case CFX_XMLNode::FirstChild:
      return m_pChild;
  }
  return nullptr;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLNode::Clone() {
  return nullptr;
}

void CFX_XMLNode::SaveXMLNode(
    const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream) {
  CFX_XMLNode* pNode = (CFX_XMLNode*)this;
  switch (pNode->GetType()) {
    case FX_XMLNODE_Instruction: {
      WideString ws;
      CFX_XMLInstruction* pInstruction = (CFX_XMLInstruction*)pNode;
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
        pXMLStream->WriteString(ws.AsStringView());
      } else {
        ws = WideString::Format(L"<?%ls", pInstruction->GetName().c_str());
        pXMLStream->WriteString(ws.AsStringView());

        for (auto it : pInstruction->GetAttributes()) {
          WideString wsValue = it.second;
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
          pXMLStream->WriteString(ws.AsStringView());
        }

        for (auto target : pInstruction->GetTargetData()) {
          ws = L" \"";
          ws += target;
          ws += L"\"";
          pXMLStream->WriteString(ws.AsStringView());
        }
        ws = L"?>";
        pXMLStream->WriteString(ws.AsStringView());
      }
      break;
    }
    case FX_XMLNODE_Element: {
      WideString ws;
      ws = L"<";
      ws += static_cast<CFX_XMLElement*>(pNode)->GetName();
      pXMLStream->WriteString(ws.AsStringView());

      for (auto it : static_cast<CFX_XMLElement*>(pNode)->GetAttributes()) {
        WideString wsValue = it.second;
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
        pXMLStream->WriteString(ws.AsStringView());
      }
      if (pNode->m_pChild) {
        ws = L"\n>";
        pXMLStream->WriteString(ws.AsStringView());
        CFX_XMLNode* pChild = pNode->m_pChild;
        while (pChild) {
          pChild->SaveXMLNode(pXMLStream);
          pChild = pChild->m_pNext;
        }
        ws = L"</";
        ws += static_cast<CFX_XMLElement*>(pNode)->GetName();
        ws += L"\n>";
      } else {
        ws = L"\n/>";
      }
      pXMLStream->WriteString(ws.AsStringView());
      break;
    }
    case FX_XMLNODE_Text: {
      WideString ws = static_cast<CFX_XMLText*>(pNode)->GetText();
      ws.Replace(L"&", L"&amp;");
      ws.Replace(L"<", L"&lt;");
      ws.Replace(L">", L"&gt;");
      ws.Replace(L"\'", L"&apos;");
      ws.Replace(L"\"", L"&quot;");
      pXMLStream->WriteString(ws.AsStringView());
      break;
    }
    case FX_XMLNODE_CharData: {
      WideString ws = L"<![CDATA[";
      ws += static_cast<CFX_XMLCharData*>(pNode)->GetText();
      ws += L"]]>";
      pXMLStream->WriteString(ws.AsStringView());
      break;
    }
    case FX_XMLNODE_Unknown:
    default:
      break;
  }
}
