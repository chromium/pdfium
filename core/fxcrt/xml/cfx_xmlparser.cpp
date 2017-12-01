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
    : m_nElementStart(0),
      m_dwCheckStatus(0),
      m_dwCurrentCheckStatus(0),
      m_pStream(pStream),
      m_pParser(pdfium::MakeUnique<CFX_XMLSyntaxParser>(m_pStream)),
      m_pParent(pParent),
      m_pChild(nullptr),
      m_syntaxParserResult(FX_XmlSyntaxResult::None) {
  ASSERT(m_pParent && m_pStream);
  m_NodeStack.push(m_pParent);
}

CFX_XMLParser::~CFX_XMLParser() {}

int32_t CFX_XMLParser::DoParser() {
  if (m_syntaxParserResult == FX_XmlSyntaxResult::Error)
    return -1;
  if (m_syntaxParserResult == FX_XmlSyntaxResult::EndOfString)
    return 100;

  int32_t iCount = 0;
  while (true) {
    m_syntaxParserResult = m_pParser->DoSyntaxParse();
    switch (m_syntaxParserResult) {
      case FX_XmlSyntaxResult::InstructionOpen:
        break;
      case FX_XmlSyntaxResult::InstructionClose:
        if (m_pChild) {
          if (m_pChild->GetType() != FX_XMLNODE_Instruction) {
            m_syntaxParserResult = FX_XmlSyntaxResult::Error;
            break;
          }
        }
        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::ElementOpen:
        if (m_dwCheckStatus != 0x03 && m_NodeStack.size() == 2)
          m_nElementStart = m_pParser->GetCurrentPos() - 1;
        break;
      case FX_XmlSyntaxResult::ElementBreak:
        break;
      case FX_XmlSyntaxResult::ElementClose:
        if (m_pChild->GetType() != FX_XMLNODE_Element) {
          m_syntaxParserResult = FX_XmlSyntaxResult::Error;
          break;
        }
        m_ws1 = m_pParser->GetTagName();
        m_ws2 = static_cast<CFX_XMLElement*>(m_pChild)->GetName();
        if (m_ws1.GetLength() > 0 && m_ws1 != m_ws2) {
          m_syntaxParserResult = FX_XmlSyntaxResult::Error;
          break;
        }
        if (!m_NodeStack.empty())
          m_NodeStack.pop();
        if (m_NodeStack.empty()) {
          m_syntaxParserResult = FX_XmlSyntaxResult::Error;
          break;
        }
        if (m_dwCurrentCheckStatus != 0 && m_NodeStack.size() == 2) {
          m_nSize[m_dwCurrentCheckStatus - 1] =
              m_pParser->GetCurrentBinaryPos() -
              m_nStart[m_dwCurrentCheckStatus - 1];
          m_dwCurrentCheckStatus = 0;
        }
        m_pParent = m_NodeStack.top();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FX_XmlSyntaxResult::TargetName:
        m_ws1 = m_pParser->GetTargetName();
        if (m_ws1 == L"originalXFAVersion" || m_ws1 == L"acrobat") {
          m_pChild = new CFX_XMLInstruction(m_ws1);
          m_pParent->InsertChildNode(m_pChild);
        } else {
          m_pChild = nullptr;
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::TagName:
        m_ws1 = m_pParser->GetTagName();
        m_pChild = new CFX_XMLElement(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_NodeStack.push(m_pChild);
        m_pParent = m_pChild;

        if (m_dwCheckStatus != 0x03 && m_NodeStack.size() == 3) {
          WideString wsTag =
              static_cast<CFX_XMLElement*>(m_pChild)->GetLocalTagName();
          if (wsTag == L"template") {
            m_dwCheckStatus |= 0x01;
            m_dwCurrentCheckStatus = 0x01;
            m_nStart[0] = m_pParser->GetCurrentBinaryPos() -
                          (m_pParser->GetCurrentPos() - m_nElementStart);
          } else if (wsTag == L"datasets") {
            m_dwCheckStatus |= 0x02;
            m_dwCurrentCheckStatus = 0x02;
            m_nStart[1] = m_pParser->GetCurrentBinaryPos() -
                          (m_pParser->GetCurrentPos() - m_nElementStart);
          }
        }
        break;
      case FX_XmlSyntaxResult::AttriName:
        m_ws1 = m_pParser->GetAttributeName();
        break;
      case FX_XmlSyntaxResult::AttriValue:
        if (m_pChild) {
          m_ws2 = m_pParser->GetAttributeName();
          if (m_pChild->GetType() == FX_XMLNODE_Element)
            static_cast<CFX_XMLElement*>(m_pChild)->SetString(m_ws1, m_ws2);
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::Text:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFX_XMLText(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::CData:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFX_XMLCharData(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::TargetData:
        if (m_pChild) {
          if (m_pChild->GetType() != FX_XMLNODE_Instruction) {
            m_syntaxParserResult = FX_XmlSyntaxResult::Error;
            break;
          }
          auto* instruction = static_cast<CFX_XMLInstruction*>(m_pChild);
          if (!m_ws1.IsEmpty())
            instruction->AppendData(m_ws1);
          instruction->AppendData(m_pParser->GetTargetData());
        }
        m_ws1.clear();
        break;
      default:
        break;
    }
    if (m_syntaxParserResult == FX_XmlSyntaxResult::Error ||
        m_syntaxParserResult == FX_XmlSyntaxResult::EndOfString) {
      break;
    }
  }
  return (m_syntaxParserResult == FX_XmlSyntaxResult::Error ||
          m_NodeStack.size() != 1)
             ? -1
             : m_pParser->GetStatus();
}
