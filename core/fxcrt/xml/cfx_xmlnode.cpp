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

CFX_XMLNode::CFX_XMLNode() = default;

CFX_XMLNode::~CFX_XMLNode() {
  DeleteChildren();
}

FX_XMLNODETYPE CFX_XMLNode::GetType() const {
  return FX_XMLNODE_Unknown;
}

void CFX_XMLNode::DeleteChildren() {
  CFX_XMLNode* pChild = first_child_;
  first_child_ = nullptr;

  while (pChild) {
    CFX_XMLNode* pNext = pChild->next_sibling_;
    delete pChild;
    pChild = pNext;
  }
}

void CFX_XMLNode::AppendChild(CFX_XMLNode* pNode) {
  InsertChildNode(pNode, -1);
}

void CFX_XMLNode::InsertChildNode(CFX_XMLNode* pNode, int32_t index) {
  ASSERT(!pNode->parent_);

  pNode->parent_ = this;
  if (!first_child_) {
    first_child_ = pNode;
    pNode->prev_sibling_ = nullptr;
    pNode->next_sibling_ = nullptr;
    return;
  }
  if (index == 0) {
    pNode->next_sibling_ = first_child_;
    pNode->prev_sibling_ = nullptr;
    first_child_->prev_sibling_ = pNode;
    first_child_ = pNode;
    return;
  }

  int32_t iCount = 0;
  CFX_XMLNode* pFind = first_child_;
  while (++iCount != index && pFind->next_sibling_)
    pFind = pFind->next_sibling_;

  pNode->prev_sibling_ = pFind;
  pNode->next_sibling_ = pFind->next_sibling_;
  if (pFind->next_sibling_)
    pFind->next_sibling_->prev_sibling_ = pNode;
  pFind->next_sibling_ = pNode;
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(first_child_ && pNode);

  if (first_child_ == pNode)
    first_child_ = pNode->next_sibling_;
  else
    pNode->prev_sibling_->next_sibling_ = pNode->next_sibling_;

  if (pNode->next_sibling_)
    pNode->next_sibling_->prev_sibling_ = pNode->prev_sibling_;

  pNode->parent_ = nullptr;
  pNode->next_sibling_ = nullptr;
  pNode->prev_sibling_ = nullptr;
}

CFX_XMLNode* CFX_XMLNode::GetRoot() {
  CFX_XMLNode* pParent = this;
  while (pParent->parent_)
    pParent = pParent->parent_;

  return pParent;
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
      if (pNode->first_child_) {
        ws = L"\n>";
        pXMLStream->WriteString(ws.AsStringView());
        CFX_XMLNode* pChild = pNode->first_child_;
        while (pChild) {
          pChild->SaveXMLNode(pXMLStream);
          pChild = pChild->next_sibling_;
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
