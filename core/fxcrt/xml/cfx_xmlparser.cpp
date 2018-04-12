// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlparser.h"

#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"

CFX_XMLParser::CFX_XMLParser(CFX_XMLNode* pParent,
                             const RetainPtr<CFX_SeekableStreamProxy>& pStream)
    : m_pParser(pdfium::MakeUnique<CFX_XMLSyntaxParser>(pStream)),
      m_pParent(pParent),
      m_pChild(nullptr) {
  ASSERT(m_pParent && pStream);
  m_NodeStack.push(m_pParent);
}

CFX_XMLParser::~CFX_XMLParser() {}

int32_t CFX_XMLParser::Parse() {
  int32_t iCount = 0;
  while (true) {
    FX_XmlSyntaxResult result = m_pParser->DoSyntaxParse();
    if (result == FX_XmlSyntaxResult::Error)
      return -1;
    if (result == FX_XmlSyntaxResult::EndOfString)
      break;

    switch (result) {
      case FX_XmlSyntaxResult::InstructionClose:
        if (m_pChild && m_pChild->GetType() != FX_XMLNODE_Instruction)
          return -1;

        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::ElementClose:
        if (m_pChild->GetType() != FX_XMLNODE_Element)
          return -1;

        m_ws1 = m_pParser->GetTagName();
        if (m_ws1.GetLength() > 0 &&
            m_ws1 != static_cast<CFX_XMLElement*>(m_pChild)->GetName()) {
          return -1;
        }

        if (!m_NodeStack.empty())
          m_NodeStack.pop();
        if (m_NodeStack.empty())
          return -1;

        m_pParent = m_NodeStack.top();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FX_XmlSyntaxResult::TargetName:
        m_ws1 = m_pParser->GetTargetName();
        if (m_ws1 == L"originalXFAVersion" || m_ws1 == L"acrobat") {
          m_pChild = new CFX_XMLInstruction(m_ws1);
          m_pParent->AppendChild(m_pChild);
        } else {
          m_pChild = nullptr;
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::TagName:
        m_ws1 = m_pParser->GetTagName();
        m_pChild = new CFX_XMLElement(m_ws1);
        m_pParent->AppendChild(m_pChild);
        m_NodeStack.push(m_pChild);
        m_pParent = m_pChild;
        break;
      case FX_XmlSyntaxResult::AttriName:
        m_ws1 = m_pParser->GetAttributeName();
        break;
      case FX_XmlSyntaxResult::AttriValue:
        if (m_pChild && m_pChild->GetType() == FX_XMLNODE_Element) {
          static_cast<CFX_XMLElement*>(m_pChild)->SetString(
              m_ws1, m_pParser->GetAttributeName());
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::Text:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFX_XMLText(m_ws1);
        m_pParent->AppendChild(m_pChild);
        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::CData:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFX_XMLCharData(m_ws1);
        m_pParent->AppendChild(m_pChild);
        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::TargetData:
        if (m_pChild) {
          if (m_pChild->GetType() != FX_XMLNODE_Instruction)
            return -1;

          auto* instruction = static_cast<CFX_XMLInstruction*>(m_pChild);
          if (!m_ws1.IsEmpty())
            instruction->AppendData(m_ws1);

          instruction->AppendData(m_pParser->GetTargetData());
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::ElementOpen:
      case FX_XmlSyntaxResult::ElementBreak:
      case FX_XmlSyntaxResult::InstructionOpen:
      default:
        break;
    }
  }
  return m_NodeStack.size() != 1 ? -1 : m_pParser->GetStatus();
}
