// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmldoc.h"

#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/xml/cfde_xmlchardata.h"
#include "xfa/fde/xml/cfde_xmlelement.h"
#include "xfa/fde/xml/cfde_xmlinstruction.h"
#include "xfa/fde/xml/cfde_xmlnode.h"
#include "xfa/fde/xml/cfde_xmltext.h"
#include "xfa/fgas/crt/fgas_codepage.h"

CFDE_XMLDoc::CFDE_XMLDoc()
    : m_iStatus(0), m_pRoot(pdfium::MakeUnique<CFDE_XMLNode>()) {
  m_pRoot->InsertChildNode(new CFDE_XMLInstruction(L"xml"));
}

CFDE_XMLDoc::~CFDE_XMLDoc() {}

bool CFDE_XMLDoc::LoadXML(std::unique_ptr<CFDE_XMLParser> pXMLParser) {
  if (!pXMLParser)
    return false;

  m_iStatus = 0;
  m_pStream.Reset();
  m_pRoot->DeleteChildren();
  m_pXMLParser = std::move(pXMLParser);
  return true;
}

int32_t CFDE_XMLDoc::DoLoad(IFX_Pause* pPause) {
  if (m_iStatus < 100)
    m_iStatus = m_pXMLParser->DoParser(pPause);

  return m_iStatus;
}

void CFDE_XMLDoc::CloseXML() {
  m_pXMLParser.reset();
}

void CFDE_XMLDoc::SaveXMLNode(const CFX_RetainPtr<IFGAS_Stream>& pXMLStream,
                              CFDE_XMLNode* pINode) {
  CFDE_XMLNode* pNode = (CFDE_XMLNode*)pINode;
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
          SaveXMLNode(pXMLStream, static_cast<CFDE_XMLNode*>(pChild));
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
