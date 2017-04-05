// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlparser.h"

#include "core/fxcrt/fx_basic.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/xml/cfde_xmlchardata.h"
#include "xfa/fde/xml/cfde_xmlelement.h"
#include "xfa/fde/xml/cfde_xmlinstruction.h"
#include "xfa/fde/xml/cfde_xmlnode.h"
#include "xfa/fde/xml/cfde_xmltext.h"

CFDE_XMLParser::CFDE_XMLParser(CFDE_XMLNode* pParent,
                               const CFX_RetainPtr<IFGAS_Stream>& pStream)
    : m_nElementStart(0),
      m_dwCheckStatus(0),
      m_dwCurrentCheckStatus(0),
      m_pStream(pStream),
      m_pParser(pdfium::MakeUnique<CFDE_XMLSyntaxParser>(m_pStream)),
      m_pParent(pParent),
      m_pChild(nullptr),
      m_syntaxParserResult(FDE_XmlSyntaxResult::None) {
  ASSERT(m_pParent && m_pStream);
  m_NodeStack.push(m_pParent);
}

CFDE_XMLParser::~CFDE_XMLParser() {}

int32_t CFDE_XMLParser::DoParser(IFX_Pause* pPause) {
  if (m_syntaxParserResult == FDE_XmlSyntaxResult::Error)
    return -1;
  if (m_syntaxParserResult == FDE_XmlSyntaxResult::EndOfString)
    return 100;

  int32_t iCount = 0;
  while (true) {
    m_syntaxParserResult = m_pParser->DoSyntaxParse();
    switch (m_syntaxParserResult) {
      case FDE_XmlSyntaxResult::InstructionOpen:
        break;
      case FDE_XmlSyntaxResult::InstructionClose:
        if (m_pChild) {
          if (m_pChild->GetType() != FDE_XMLNODE_Instruction) {
            m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
            break;
          }
        }
        m_pChild = m_pParent;
        break;
      case FDE_XmlSyntaxResult::ElementOpen:
        if (m_dwCheckStatus != 0x03 && m_NodeStack.size() == 2)
          m_nElementStart = m_pParser->GetCurrentPos() - 1;
        break;
      case FDE_XmlSyntaxResult::ElementBreak:
        break;
      case FDE_XmlSyntaxResult::ElementClose:
        if (m_pChild->GetType() != FDE_XMLNODE_Element) {
          m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
          break;
        }
        m_ws1 = m_pParser->GetTagName();
        m_ws2 = static_cast<CFDE_XMLElement*>(m_pChild)->GetName();
        if (m_ws1.GetLength() > 0 && m_ws1 != m_ws2) {
          m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
          break;
        }
        if (!m_NodeStack.empty())
          m_NodeStack.pop();
        if (m_NodeStack.empty()) {
          m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
          break;
        } else if (m_dwCurrentCheckStatus != 0 && m_NodeStack.size() == 2) {
          m_nSize[m_dwCurrentCheckStatus - 1] =
              m_pParser->GetCurrentBinaryPos() -
              m_nStart[m_dwCurrentCheckStatus - 1];
          m_dwCurrentCheckStatus = 0;
        }
        m_pParent = m_NodeStack.top();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FDE_XmlSyntaxResult::TargetName:
        m_ws1 = m_pParser->GetTargetName();
        if (m_ws1 == L"originalXFAVersion" || m_ws1 == L"acrobat") {
          m_pChild = new CFDE_XMLInstruction(m_ws1);
          m_pParent->InsertChildNode(m_pChild);
        } else {
          m_pChild = nullptr;
        }
        m_ws1.clear();
        break;
      case FDE_XmlSyntaxResult::TagName:
        m_ws1 = m_pParser->GetTagName();
        m_pChild = new CFDE_XMLElement(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_NodeStack.push(m_pChild);
        m_pParent = m_pChild;

        if (m_dwCheckStatus != 0x03 && m_NodeStack.size() == 3) {
          CFX_WideString wsTag =
              static_cast<CFDE_XMLElement*>(m_pChild)->GetLocalTagName();
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
      case FDE_XmlSyntaxResult::AttriName:
        m_ws1 = m_pParser->GetAttributeName();
        break;
      case FDE_XmlSyntaxResult::AttriValue:
        if (m_pChild) {
          m_ws2 = m_pParser->GetAttributeName();
          if (m_pChild->GetType() == FDE_XMLNODE_Element)
            static_cast<CFDE_XMLElement*>(m_pChild)->SetString(m_ws1, m_ws2);
        }
        m_ws1.clear();
        break;
      case FDE_XmlSyntaxResult::Text:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFDE_XMLText(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XmlSyntaxResult::CData:
        m_ws1 = m_pParser->GetTextData();
        m_pChild = new CFDE_XMLCharData(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XmlSyntaxResult::TargetData:
        if (m_pChild) {
          if (m_pChild->GetType() != FDE_XMLNODE_Instruction) {
            m_syntaxParserResult = FDE_XmlSyntaxResult::Error;
            break;
          }
          auto* instruction = static_cast<CFDE_XMLInstruction*>(m_pChild);
          if (!m_ws1.IsEmpty())
            instruction->AppendData(m_ws1);
          instruction->AppendData(m_pParser->GetTargetData());
        }
        m_ws1.clear();
        break;
      default:
        break;
    }
    if (m_syntaxParserResult == FDE_XmlSyntaxResult::Error ||
        m_syntaxParserResult == FDE_XmlSyntaxResult::EndOfString) {
      break;
    }
    if (pPause && iCount > 500 && pPause->NeedToPauseNow()) {
      break;
    }
  }
  return (m_syntaxParserResult == FDE_XmlSyntaxResult::Error ||
          m_NodeStack.size() != 1)
             ? -1
             : m_pParser->GetStatus();
}
