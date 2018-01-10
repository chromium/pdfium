// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dataexporter.h"

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CXFA_DataExporter::CXFA_DataExporter(CXFA_Document* pDocument)
    : m_pDocument(pDocument) {
  ASSERT(m_pDocument);
}

CXFA_DataExporter::~CXFA_DataExporter() {}

bool CXFA_DataExporter::Export(const RetainPtr<IFX_SeekableStream>& pWrite) {
  return Export(pWrite, m_pDocument->GetRoot(), 0, nullptr);
}

bool CXFA_DataExporter::Export(const RetainPtr<IFX_SeekableStream>& pWrite,
                               CXFA_Node* pNode,
                               uint32_t dwFlag,
                               const char* pChecksum) {
  ASSERT(pWrite);
  if (!pWrite)
    return false;

  auto pStream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pWrite, true);
  pStream->SetCodePage(FX_CODEPAGE_UTF8);
  return Export(pStream, pNode, dwFlag, pChecksum);
}

bool CXFA_DataExporter::Export(
    const RetainPtr<CFX_SeekableStreamProxy>& pStream,
    CXFA_Node* pNode,
    uint32_t dwFlag,
    const char* pChecksum) {
  CFX_XMLDoc* pXMLDoc = m_pDocument->GetXMLDoc();
  if (pNode->IsModelNode()) {
    switch (pNode->GetPacketType()) {
      case XFA_PacketType::Xdp: {
        pStream->WriteString(
            L"<xdp:xdp xmlns:xdp=\"http://ns.adobe.com/xdp/\">");
        for (CXFA_Node* pChild = pNode->GetFirstChild(); pChild;
             pChild = pChild->GetNextSibling()) {
          Export(pStream, pChild, dwFlag, pChecksum);
        }
        pStream->WriteString(L"</xdp:xdp\n>");
        break;
      }
      case XFA_PacketType::Datasets: {
        CFX_XMLElement* pElement =
            static_cast<CFX_XMLElement*>(pNode->GetXMLMappingNode());
        if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
          return false;

        CXFA_Node* pDataNode = pNode->GetFirstChild();
        ASSERT(pDataNode);
        XFA_DataExporter_DealWithDataGroupNode(pDataNode);
        pXMLDoc->SaveXMLNode(pStream, pElement);
        break;
      }
      case XFA_PacketType::Form: {
        XFA_DataExporter_RegenerateFormFile(pNode, pStream, pChecksum, false);
        break;
      }
      case XFA_PacketType::Template:
      default: {
        CFX_XMLElement* pElement =
            static_cast<CFX_XMLElement*>(pNode->GetXMLMappingNode());
        if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
          return false;

        pXMLDoc->SaveXMLNode(pStream, pElement);
        break;
      }
    }
    return true;
  }

  CXFA_Node* pDataNode = pNode->GetParent();
  CXFA_Node* pExportNode = pNode;
  for (CXFA_Node* pChildNode = pDataNode->GetFirstChild(); pChildNode;
       pChildNode = pChildNode->GetNextSibling()) {
    if (pChildNode != pNode) {
      pExportNode = pDataNode;
      break;
    }
  }
  CFX_XMLElement* pElement =
      static_cast<CFX_XMLElement*>(pExportNode->GetXMLMappingNode());
  if (!pElement || pElement->GetType() != FX_XMLNODE_Element)
    return false;

  XFA_DataExporter_DealWithDataGroupNode(pExportNode);
  pElement->SetString(L"xmlns:xfa", L"http://www.xfa.org/schema/xfa-data/1.0/");
  pXMLDoc->SaveXMLNode(pStream, pElement);
  pElement->RemoveAttribute(L"xmlns:xfa");

  return true;
}
