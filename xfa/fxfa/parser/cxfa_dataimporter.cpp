// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_dataimporter.h"

#include <memory>

#include "core/fxcrt/fx_stream.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_simple_parser.h"

CXFA_DataImporter::CXFA_DataImporter(CXFA_Document* pDocument)
    : m_pDocument(pDocument) {
  ASSERT(m_pDocument);
}

CXFA_DataImporter::~CXFA_DataImporter() {}

bool CXFA_DataImporter::ImportData(
    const RetainPtr<IFX_SeekableStream>& pDataDocument) {
  auto pDataDocumentParser =
      pdfium::MakeUnique<CXFA_SimpleParser>(m_pDocument.Get());
  if (pDataDocumentParser->StartParse(
          pDataDocument, XFA_PacketType::Datasets) != XFA_PARSESTATUS_Ready) {
    return false;
  }
  if (pDataDocumentParser->DoParse() < XFA_PARSESTATUS_Done)
    return false;

  CXFA_Node* pImportDataRoot = pDataDocumentParser->GetRootNode();
  if (!pImportDataRoot)
    return false;

  CXFA_Node* pDataModel =
      ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Datasets));
  if (!pDataModel)
    return false;

  CXFA_Node* pDataNode = ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Data));
  if (pDataNode)
    pDataModel->RemoveChild(pDataNode, true);

  if (pImportDataRoot->GetElementType() == XFA_Element::DataModel) {
    while (CXFA_Node* pChildNode = pImportDataRoot->GetFirstChild()) {
      pImportDataRoot->RemoveChild(pChildNode, true);
      pDataModel->InsertChild(pChildNode, nullptr);
    }
  } else {
    CFX_XMLNode* pXMLNode = pImportDataRoot->GetXMLMappingNode();
    CFX_XMLNode* pParentXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::Parent);
    if (pParentXMLNode)
      pParentXMLNode->RemoveChildNode(pXMLNode);
    pDataModel->InsertChild(pImportDataRoot, nullptr);
  }
  m_pDocument->DoDataRemerge(false);
  return true;
}
