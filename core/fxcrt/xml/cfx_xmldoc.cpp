// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmldoc.h"

#include <utility>
#include <vector>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_XMLDoc::CFX_XMLDoc()
    : m_iStatus(0), m_pRoot(pdfium::MakeUnique<CFX_XMLNode>()) {
  m_pRoot->InsertChildNode(new CFX_XMLInstruction(L"xml"));
}

CFX_XMLDoc::~CFX_XMLDoc() {}

bool CFX_XMLDoc::LoadXML(std::unique_ptr<CFX_XMLParser> pXMLParser) {
  if (!pXMLParser)
    return false;

  m_iStatus = 0;
  m_pStream.Reset();
  m_pRoot->DeleteChildren();
  m_pXMLParser = std::move(pXMLParser);
  return true;
}

int32_t CFX_XMLDoc::DoLoad() {
  if (m_iStatus < 100)
    m_iStatus = m_pXMLParser->DoParser();

  return m_iStatus;
}

void CFX_XMLDoc::CloseXML() {
  m_pXMLParser.reset();
}

void CFX_XMLDoc::SaveXMLNode(
    const RetainPtr<CFX_SeekableStreamProxy>& pXMLStream,
    CFX_XMLNode* pINode) {
  CFX_XMLNode* pNode = (CFX_XMLNode*)pINode;
  switch (pNode->GetType()) {
    case FX_XMLNODE_Instruction: {
      CFX_XMLInstruction* pInstruction = (CFX_XMLInstruction*)pNode;
      if (pInstruction->GetName().CompareNoCase(L"xml") == 0) {
        WideString ws = L"<?xml version=\"1.0\" encoding=\"";
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
        WideString ws =
            WideString::Format(L"<?%ls", pInstruction->GetName().c_str());
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
          SaveXMLNode(pXMLStream, static_cast<CFX_XMLNode*>(pChild));
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
