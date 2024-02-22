// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_document_builder.h"

#include <optional>
#include <utility>
#include <vector>

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_template.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

CFX_XMLNode* GetDocumentNode(CFX_XMLNode* pRootNode) {
  for (CFX_XMLNode* pXMLNode = pRootNode->GetFirstChild(); pXMLNode;
       pXMLNode = pXMLNode->GetNextSibling()) {
    if (pXMLNode->GetType() == CFX_XMLNode::Type::kElement)
      return pXMLNode;
  }
  return nullptr;
}

bool MatchNodeName(CFX_XMLNode* pNode,
                   ByteStringView bsLocalTagName,
                   ByteStringView bsNamespaceURIPrefix,
                   XFA_PacketMatch eMatch) {
  CFX_XMLElement* pElement = ToXMLElement(pNode);
  if (!pElement)
    return false;

  if (!pElement->GetLocalTagName().EqualsASCII(bsLocalTagName))
    return false;

  if (eMatch == XFA_PacketMatch::kNoMatch)
    return true;

  WideString wsNodeStr = pElement->GetNamespaceURI();
  if (eMatch == XFA_PacketMatch::kPrefixMatch) {
    return wsNodeStr.AsStringView()
        .First(bsNamespaceURIPrefix.GetLength())
        .EqualsASCII(bsNamespaceURIPrefix);
  }
  return wsNodeStr.EqualsASCII(bsNamespaceURIPrefix);
}

bool GetAttributeLocalName(WideStringView wsAttributeName,
                           WideString& wsLocalAttrName) {
  WideString wsAttrName(wsAttributeName);
  auto pos = wsAttrName.Find(L':', 0);
  if (!pos.has_value()) {
    wsLocalAttrName = std::move(wsAttrName);
    return false;
  }
  wsLocalAttrName = wsAttrName.Last(wsAttrName.GetLength() - pos.value() - 1);
  return true;
}

bool ResolveAttribute(CFX_XMLElement* pElement,
                      const WideString& wsAttrName,
                      WideString& wsLocalAttrName,
                      WideString& wsNamespaceURI) {
  WideString wsNSPrefix;
  if (GetAttributeLocalName(wsAttrName.AsStringView(), wsLocalAttrName)) {
    wsNSPrefix = wsAttrName.First(wsAttrName.GetLength() -
                                  wsLocalAttrName.GetLength() - 1);
  }
  if (wsLocalAttrName.EqualsASCII("xmlns") || wsNSPrefix.EqualsASCII("xmlns") ||
      wsNSPrefix.EqualsASCII("xml")) {
    return false;
  }
  if (!XFA_FDEExtension_ResolveNamespaceQualifier(pElement, wsNSPrefix,
                                                  &wsNamespaceURI)) {
    wsNamespaceURI.clear();
    return false;
  }
  return true;
}

std::optional<WideString> FindAttributeWithNS(
    CFX_XMLElement* pElement,
    WideStringView wsLocalAttributeName,
    WideStringView wsNamespaceURIPrefix) {
  WideString wsAttrNS;
  for (auto it : pElement->GetAttributes()) {
    auto pos = it.first.Find(L':', 0);
    WideString wsNSPrefix;
    if (!pos.has_value()) {
      if (wsLocalAttributeName != it.first)
        continue;
    } else {
      if (wsLocalAttributeName !=
          it.first.Last(it.first.GetLength() - pos.value() - 1)) {
        continue;
      }
      wsNSPrefix = it.first.First(pos.value());
    }
    if (!XFA_FDEExtension_ResolveNamespaceQualifier(pElement, wsNSPrefix,
                                                    &wsAttrNS) ||
        wsAttrNS != wsNamespaceURIPrefix) {
      continue;
    }
    return it.second;
  }
  return std::nullopt;
}

CFX_XMLNode* GetDataSetsFromXDP(CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO datasets_packet =
      XFA_GetPacketByIndex(XFA_PacketType::Datasets);
  if (MatchNodeName(pXMLDocumentNode, datasets_packet.name, datasets_packet.uri,
                    datasets_packet.match)) {
    return pXMLDocumentNode;
  }
  XFA_PACKETINFO xdp_packet = XFA_GetPacketByIndex(XFA_PacketType::Xdp);
  if (!MatchNodeName(pXMLDocumentNode, xdp_packet.name, xdp_packet.uri,
                     xdp_packet.match)) {
    return nullptr;
  }
  for (CFX_XMLNode* pDatasetsNode = pXMLDocumentNode->GetFirstChild();
       pDatasetsNode; pDatasetsNode = pDatasetsNode->GetNextSibling()) {
    if (MatchNodeName(pDatasetsNode, datasets_packet.name, datasets_packet.uri,
                      datasets_packet.match)) {
      return pDatasetsNode;
    }
  }
  return nullptr;
}

bool IsStringAllWhitespace(WideString wsText) {
  wsText.TrimBack(L"\x20\x9\xD\xA");
  return wsText.IsEmpty();
}

void ConvertXMLToPlainText(CFX_XMLElement* pRootXMLNode, WideString& wsOutput) {
  for (CFX_XMLNode* pXMLChild = pRootXMLNode->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    switch (pXMLChild->GetType()) {
      case CFX_XMLNode::Type::kElement: {
        WideString wsTextData = ToXMLElement(pXMLChild)->GetTextData();
        wsTextData += L"\n";
        wsOutput += wsTextData;
        break;
      }
      case CFX_XMLNode::Type::kText:
      case CFX_XMLNode::Type::kCharData: {
        WideString wsText = ToXMLText(pXMLChild)->GetText();
        if (IsStringAllWhitespace(wsText))
          continue;
        wsOutput = std::move(wsText);
        break;
      }
      default:
        NOTREACHED_NORETURN();
    }
  }
}

WideString GetPlainTextFromRichText(CFX_XMLNode* pXMLNode) {
  if (!pXMLNode)
    return WideString();

  WideString wsPlainText;
  switch (pXMLNode->GetType()) {
    case CFX_XMLNode::Type::kElement: {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
      WideString wsTag = pXMLElement->GetLocalTagName();
      uint32_t uTag = FX_HashCode_GetLoweredW(wsTag.AsStringView());
      if (uTag == 0x0001f714) {
        wsPlainText += L"\n";
      } else if (uTag == 0x00000070) {
        if (!wsPlainText.IsEmpty()) {
          wsPlainText += L"\n";
        }
      } else if (uTag == 0xa48ac63) {
        if (!wsPlainText.IsEmpty() && wsPlainText.Back() != '\n') {
          wsPlainText += L"\n";
        }
      }
      break;
    }
    case CFX_XMLNode::Type::kText:
    case CFX_XMLNode::Type::kCharData: {
      WideString wsContent = ToXMLText(pXMLNode)->GetText();
      wsPlainText += wsContent;
      break;
    }
    default:
      break;
  }
  for (CFX_XMLNode* pChildXML = pXMLNode->GetFirstChild(); pChildXML;
       pChildXML = pChildXML->GetNextSibling()) {
    wsPlainText += GetPlainTextFromRichText(pChildXML);
  }

  return wsPlainText;
}

}  // namespace

bool XFA_RecognizeRichText(CFX_XMLElement* pRichTextXMLNode) {
  return pRichTextXMLNode && pRichTextXMLNode->GetNamespaceURI().EqualsASCII(
                                 "http://www.w3.org/1999/xhtml");
}

CXFA_DocumentBuilder::CXFA_DocumentBuilder(CXFA_Document* pNodeFactory)
    : node_factory_(pNodeFactory) {}

CXFA_DocumentBuilder::~CXFA_DocumentBuilder() = default;

bool CXFA_DocumentBuilder::BuildDocument(CFX_XMLDocument* pXML,
                                         XFA_PacketType ePacketID) {
  DCHECK(pXML);

  CFX_XMLNode* root = Build(pXML);
  if (!root)
    return false;

  root_node_ = ParseAsXDPPacket(root, ePacketID);
  return !!root_node_;
}

CFX_XMLNode* CXFA_DocumentBuilder::Build(CFX_XMLDocument* pXML) {
  if (!pXML)
    return nullptr;

  xml_doc_ = pXML;
  xml_doc_->GetRoot()->InsertChildNode(
      xml_doc_->CreateNode<CFX_XMLInstruction>(L"xml"), 0);

  return GetDocumentNode(xml_doc_->GetRoot());
}

void CXFA_DocumentBuilder::ConstructXFANode(CXFA_Node* pXFANode,
                                            CFX_XMLNode* pXMLNode) {
  XFA_PacketType ePacketID = pXFANode->GetPacketType();
  if (ePacketID == XFA_PacketType::Datasets) {
    if (pXFANode->GetElementType() == XFA_Element::DataValue) {
      for (CFX_XMLNode* pXMLChild = pXMLNode->GetFirstChild(); pXMLChild;
           pXMLChild = pXMLChild->GetNextSibling()) {
        CFX_XMLNode::Type eNodeType = pXMLChild->GetType();
        if (eNodeType == CFX_XMLNode::Type::kInstruction)
          continue;

        if (eNodeType == CFX_XMLNode::Type::kElement) {
          CXFA_Node* pXFAChild = node_factory_->CreateNode(
              XFA_PacketType::Datasets, XFA_Element::DataValue);
          if (!pXFAChild)
            return;

          CFX_XMLElement* child = static_cast<CFX_XMLElement*>(pXMLChild);
          WideString wsNodeStr = child->GetLocalTagName();
          pXFAChild->JSObject()->SetCData(XFA_Attribute::Name, wsNodeStr);
          WideString wsChildValue = GetPlainTextFromRichText(child);
          if (!wsChildValue.IsEmpty())
            pXFAChild->JSObject()->SetCData(XFA_Attribute::Value, wsChildValue);

          pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
          pXFAChild->SetXMLMappingNode(pXMLChild);
          pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
          break;
        }
      }
      root_node_ = pXFANode;
    } else {
      root_node_ = DataLoader(pXFANode, pXMLNode);
    }
  } else if (pXFANode->IsContentNode()) {
    ParseContentNode(pXFANode, pXMLNode, ePacketID);
    root_node_ = pXFANode;
  } else {
    root_node_ = NormalLoader(pXFANode, pXMLNode, ePacketID, true);
  }
}

CXFA_Node* CXFA_DocumentBuilder::GetRootNode() const {
  return root_node_;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket(CFX_XMLNode* pXMLDocumentNode,
                                                  XFA_PacketType ePacketID) {
  switch (ePacketID) {
    case XFA_PacketType::Xdp:
      return ParseAsXDPPacket_XDP(pXMLDocumentNode);
    case XFA_PacketType::Config:
      return ParseAsXDPPacket_Config(pXMLDocumentNode);
    case XFA_PacketType::Template:
      return ParseAsXDPPacket_Template(pXMLDocumentNode);
    case XFA_PacketType::Form:
      return ParseAsXDPPacket_Form(pXMLDocumentNode);
    case XFA_PacketType::Datasets:
      return ParseAsXDPPacket_Data(pXMLDocumentNode);
    case XFA_PacketType::Xdc:
      return ParseAsXDPPacket_Xdc(pXMLDocumentNode);
    case XFA_PacketType::LocaleSet:
      return ParseAsXDPPacket_LocaleConnectionSourceSet(
          pXMLDocumentNode, XFA_PacketType::LocaleSet, XFA_Element::LocaleSet);
    case XFA_PacketType::ConnectionSet:
      return ParseAsXDPPacket_LocaleConnectionSourceSet(
          pXMLDocumentNode, XFA_PacketType::ConnectionSet,
          XFA_Element::ConnectionSet);
    case XFA_PacketType::SourceSet:
      return ParseAsXDPPacket_LocaleConnectionSourceSet(
          pXMLDocumentNode, XFA_PacketType::SourceSet, XFA_Element::SourceSet);
    default:
      return ParseAsXDPPacket_User(pXMLDocumentNode);
  }
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_XDP(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Xdp);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pXFARootNode =
      node_factory_->CreateNode(XFA_PacketType::Xdp, XFA_Element::Xfa);
  if (!pXFARootNode)
    return nullptr;

  root_node_ = pXFARootNode;
  pXFARootNode->JSObject()->SetCData(XFA_Attribute::Name, L"xfa");

  for (auto it : ToXMLElement(pXMLDocumentNode)->GetAttributes()) {
    if (it.first.EqualsASCII("uuid"))
      pXFARootNode->JSObject()->SetCData(XFA_Attribute::Uuid, it.second);
    else if (it.first.EqualsASCII("timeStamp"))
      pXFARootNode->JSObject()->SetCData(XFA_Attribute::TimeStamp, it.second);
  }

  CFX_XMLNode* pXMLConfigDOMRoot = nullptr;
  CXFA_Node* pXFAConfigDOMRoot = nullptr;
  XFA_PACKETINFO config_packet = XFA_GetPacketByIndex(XFA_PacketType::Config);
  for (CFX_XMLNode* pChildItem = pXMLDocumentNode->GetFirstChild(); pChildItem;
       pChildItem = pChildItem->GetNextSibling()) {
    if (!MatchNodeName(pChildItem, config_packet.name, config_packet.uri,
                       config_packet.match)) {
      continue;
    }
    // TODO(tsepez): make GetFirstChildByName() take a name.
    uint32_t hash = FX_HashCode_GetAsIfW(config_packet.name);
    if (pXFARootNode->GetFirstChildByName(hash))
      return nullptr;

    pXMLConfigDOMRoot = pChildItem;
    pXFAConfigDOMRoot = ParseAsXDPPacket_Config(pXMLConfigDOMRoot);
    if (pXFAConfigDOMRoot)
      pXFARootNode->InsertChildAndNotify(pXFAConfigDOMRoot, nullptr);
  }

  CFX_XMLNode* pXMLDatasetsDOMRoot = nullptr;
  CFX_XMLNode* pXMLFormDOMRoot = nullptr;
  CFX_XMLNode* pXMLTemplateDOMRoot = nullptr;
  for (CFX_XMLNode* pChildItem = pXMLDocumentNode->GetFirstChild(); pChildItem;
       pChildItem = pChildItem->GetNextSibling()) {
    CFX_XMLElement* pElement = ToXMLElement(pChildItem);
    if (!pElement || pElement == pXMLConfigDOMRoot)
      continue;

    WideString wsPacketName = pElement->GetLocalTagName();
    std::optional<XFA_PACKETINFO> packet_info =
        XFA_GetPacketByName(wsPacketName.AsStringView());
    if (packet_info.has_value() && packet_info.value().uri &&
        !MatchNodeName(pElement, packet_info.value().name,
                       packet_info.value().uri, packet_info.value().match)) {
      packet_info = {};
    }
    XFA_PacketType ePacket = XFA_PacketType::User;
    if (packet_info.has_value())
      ePacket = packet_info.value().packet_type;
    if (ePacket == XFA_PacketType::Xdp)
      continue;
    if (ePacket == XFA_PacketType::Datasets) {
      if (pXMLDatasetsDOMRoot)
        return nullptr;

      pXMLDatasetsDOMRoot = pElement;
    } else if (ePacket == XFA_PacketType::Form) {
      if (pXMLFormDOMRoot)
        return nullptr;

      pXMLFormDOMRoot = pElement;
    } else if (ePacket == XFA_PacketType::Template) {
      // Found a duplicate template packet.
      if (pXMLTemplateDOMRoot)
        return nullptr;

      CXFA_Node* pPacketNode = ParseAsXDPPacket_Template(pElement);
      if (pPacketNode) {
        pXMLTemplateDOMRoot = pElement;
        pXFARootNode->InsertChildAndNotify(pPacketNode, nullptr);
      }
    } else {
      CXFA_Node* pPacketNode = ParseAsXDPPacket(pElement, ePacket);
      if (pPacketNode) {
        if (packet_info.has_value() &&
            (packet_info.value().support == XFA_PacketSupport::kSupportOne) &&
            pXFARootNode->GetFirstChildByName(
                FX_HashCode_GetAsIfW(packet_info.value().name))) {
          return nullptr;
        }
        pXFARootNode->InsertChildAndNotify(pPacketNode, nullptr);
      }
    }
  }

  // No template is found.
  if (!pXMLTemplateDOMRoot)
    return nullptr;

  if (pXMLDatasetsDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLDatasetsDOMRoot, XFA_PacketType::Datasets);
    if (pPacketNode)
      pXFARootNode->InsertChildAndNotify(pPacketNode, nullptr);
  }
  if (pXMLFormDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLFormDOMRoot, XFA_PacketType::Form);
    if (pPacketNode)
      pXFARootNode->InsertChildAndNotify(pPacketNode, nullptr);
  }

  pXFARootNode->SetXMLMappingNode(pXMLDocumentNode);
  return pXFARootNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_Config(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Config);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pNode =
      node_factory_->CreateNode(XFA_PacketType::Config, XFA_Element::Config);
  if (!pNode)
    return nullptr;

  pNode->JSObject()->SetCData(XFA_Attribute::Name,
                              WideString::FromASCII(packet.name));
  if (!NormalLoader(pNode, pXMLDocumentNode, XFA_PacketType::Config, true))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_Template(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Template);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pNode = node_factory_->CreateNode(XFA_PacketType::Template,
                                               XFA_Element::Template);
  if (!pNode)
    return nullptr;

  pNode->JSObject()->SetCData(XFA_Attribute::Name,
                              WideString::FromASCII(packet.name));

  CFX_XMLElement* pXMLDocumentElement = ToXMLElement(pXMLDocumentNode);
  WideString wsNamespaceURI = pXMLDocumentElement->GetNamespaceURI();
  if (wsNamespaceURI.IsEmpty())
    wsNamespaceURI = pXMLDocumentElement->GetAttribute(L"xmlns:xfa");

  pNode->GetDocument()->RecognizeXFAVersionNumber(wsNamespaceURI);

  if (!NormalLoader(pNode, pXMLDocumentNode, XFA_PacketType::Template, true))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_Form(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Form);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pNode =
      node_factory_->CreateNode(XFA_PacketType::Form, XFA_Element::Form);
  if (!pNode)
    return nullptr;

  pNode->JSObject()->SetCData(XFA_Attribute::Name,
                              WideString::FromASCII(packet.name));
  CXFA_Template* pTemplateRoot =
      root_node_->GetFirstChildByClass<CXFA_Template>(XFA_Element::Template);
  CXFA_Subform* pTemplateChosen =
      pTemplateRoot ? pTemplateRoot->GetFirstChildByClass<CXFA_Subform>(
                          XFA_Element::Subform)
                    : nullptr;
  bool bUseAttribute = true;
  if (pTemplateChosen &&
      pTemplateChosen->JSObject()->GetEnum(XFA_Attribute::RestoreState) !=
          XFA_AttributeValue::Auto) {
    bUseAttribute = false;
  }
  if (!NormalLoader(pNode, pXMLDocumentNode, XFA_PacketType::Form,
                    bUseAttribute))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_Data(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Datasets);
  CFX_XMLNode* pDatasetsXMLNode = GetDataSetsFromXDP(pXMLDocumentNode);
  if (pDatasetsXMLNode) {
    CXFA_Node* pNode = node_factory_->CreateNode(XFA_PacketType::Datasets,
                                                 XFA_Element::DataModel);
    if (!pNode)
      return nullptr;

    pNode->JSObject()->SetCData(XFA_Attribute::Name,
                                WideString::FromASCII(packet.name));
    if (!DataLoader(pNode, pDatasetsXMLNode))
      return nullptr;

    pNode->SetXMLMappingNode(pDatasetsXMLNode);
    return pNode;
  }

  CFX_XMLNode* pDataXMLNode = nullptr;
  if (MatchNodeName(pXMLDocumentNode, "data", packet.uri, packet.match)) {
    ToXMLElement(pXMLDocumentNode)->RemoveAttribute(L"xmlns:xfa");
    pDataXMLNode = pXMLDocumentNode;
  } else {
    auto* pDataElement = xml_doc_->CreateNode<CFX_XMLElement>(L"xfa:data");
    pXMLDocumentNode->RemoveSelfIfParented();

    CFX_XMLElement* pElement = ToXMLElement(pXMLDocumentNode);
    pElement->RemoveAttribute(L"xmlns:xfa");

    // The node was either removed from the parent above, or already has no
    // parent so we can take ownership.
    pDataElement->AppendLastChild(pXMLDocumentNode);
    pDataXMLNode = pDataElement;
  }
  if (!pDataXMLNode)
    return nullptr;

  CXFA_Node* pNode = node_factory_->CreateNode(XFA_PacketType::Datasets,
                                               XFA_Element::DataGroup);
  if (!pNode)
    return nullptr;

  WideString wsLocalName = ToXMLElement(pDataXMLNode)->GetLocalTagName();
  pNode->JSObject()->SetCData(XFA_Attribute::Name, wsLocalName);
  if (!DataLoader(pNode, pDataXMLNode))
    return nullptr;

  pNode->SetXMLMappingNode(pDataXMLNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_LocaleConnectionSourceSet(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_PacketType packet_type,
    XFA_Element element) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(packet_type);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pNode = node_factory_->CreateNode(packet_type, element);
  if (!pNode)
    return nullptr;

  pNode->JSObject()->SetCData(XFA_Attribute::Name,
                              WideString::FromASCII(packet.name));
  if (!NormalLoader(pNode, pXMLDocumentNode, packet_type, true))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_Xdc(
    CFX_XMLNode* pXMLDocumentNode) {
  XFA_PACKETINFO packet = XFA_GetPacketByIndex(XFA_PacketType::Xdc);
  if (!MatchNodeName(pXMLDocumentNode, packet.name, packet.uri, packet.match))
    return nullptr;

  CXFA_Node* pNode =
      node_factory_->CreateNode(XFA_PacketType::Xdc, XFA_Element::Xdc);
  if (!pNode)
    return nullptr;

  pNode->JSObject()->SetCData(XFA_Attribute::Name,
                              WideString::FromASCII(packet.name));
  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::ParseAsXDPPacket_User(
    CFX_XMLNode* pXMLDocumentNode) {
  CXFA_Node* pNode =
      node_factory_->CreateNode(XFA_PacketType::Xdp, XFA_Element::Packet);
  if (!pNode)
    return nullptr;

  WideString wsName = ToXMLElement(pXMLDocumentNode)->GetLocalTagName();
  pNode->JSObject()->SetCData(XFA_Attribute::Name, wsName);
  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_DocumentBuilder::DataLoader(CXFA_Node* pXFANode,
                                            CFX_XMLNode* pXMLDoc) {
  ParseDataGroup(pXFANode, pXMLDoc, XFA_PacketType::Datasets);
  return pXFANode;
}

CXFA_Node* CXFA_DocumentBuilder::NormalLoader(CXFA_Node* pXFANode,
                                              CFX_XMLNode* pXMLDoc,
                                              XFA_PacketType ePacketID,
                                              bool bUseAttribute) {
  constexpr size_t kMaxExecuteRecursion = 1000;
  if (execute_recursion_depth_ > kMaxExecuteRecursion)
    return nullptr;
  AutoRestorer<size_t> restorer(&execute_recursion_depth_);
  ++execute_recursion_depth_;

  bool bOneOfPropertyFound = false;
  for (CFX_XMLNode* pXMLChild = pXMLDoc->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    switch (pXMLChild->GetType()) {
      case CFX_XMLNode::Type::kElement: {
        CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLChild);
        WideString wsTagName = pXMLElement->GetLocalTagName();
        XFA_Element eType = XFA_GetElementByName(wsTagName.AsStringView());
        if (eType == XFA_Element::Unknown)
          continue;

        if (pXFANode->HasPropertyFlag(eType, XFA_PropertyFlag::kOneOf) ||
            pXFANode->HasPropertyFlag(eType, XFA_PropertyFlag::kDefaultOneOf)) {
          if (bOneOfPropertyFound)
            break;
          bOneOfPropertyFound = true;
        }

        CXFA_Node* pXFAChild = node_factory_->CreateNode(ePacketID, eType);
        if (!pXFAChild)
          return nullptr;
        if (ePacketID == XFA_PacketType::Config) {
          pXFAChild->JSObject()->SetAttributeByEnum(XFA_Attribute::Name,
                                                    wsTagName, false);
        }

        bool IsNeedValue = true;
        for (auto it : pXMLElement->GetAttributes()) {
          WideString wsAttrName;
          GetAttributeLocalName(it.first.AsStringView(), wsAttrName);
          if (wsAttrName.EqualsASCII("nil") && it.second.EqualsASCII("true"))
            IsNeedValue = false;

          std::optional<XFA_ATTRIBUTEINFO> attr =
              XFA_GetAttributeByName(wsAttrName.AsStringView());
          if (!attr.has_value())
            continue;

          if (!bUseAttribute && attr.value().attribute != XFA_Attribute::Name &&
              attr.value().attribute != XFA_Attribute::Save) {
            continue;
          }
          pXFAChild->JSObject()->SetAttributeByEnum(attr.value().attribute,
                                                    it.second, false);
        }
        pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
        if (eType == XFA_Element::Validate || eType == XFA_Element::Locale) {
          if (ePacketID == XFA_PacketType::Config)
            ParseContentNode(pXFAChild, pXMLElement, ePacketID);
          else
            NormalLoader(pXFAChild, pXMLElement, ePacketID, bUseAttribute);

          break;
        }
        switch (pXFAChild->GetObjectType()) {
          case XFA_ObjectType::ContentNode:
          case XFA_ObjectType::TextNode:
          case XFA_ObjectType::NodeC:
          case XFA_ObjectType::NodeV:
            if (IsNeedValue)
              ParseContentNode(pXFAChild, pXMLElement, ePacketID);
            break;
          default:
            NormalLoader(pXFAChild, pXMLElement, ePacketID, bUseAttribute);
            break;
        }
      } break;
      case CFX_XMLNode::Type::kInstruction:
        ParseInstruction(pXFANode, ToXMLInstruction(pXMLChild), ePacketID);
        break;
      default:
        break;
    }
  }
  return pXFANode;
}

void CXFA_DocumentBuilder::ParseContentNode(CXFA_Node* pXFANode,
                                            CFX_XMLNode* pXMLNode,
                                            XFA_PacketType ePacketID) {
  XFA_Element element = XFA_Element::Sharptext;
  if (pXFANode->GetElementType() == XFA_Element::ExData) {
    WideString wsContentType =
        pXFANode->JSObject()->GetCData(XFA_Attribute::ContentType);
    if (wsContentType.EqualsASCII("text/html"))
      element = XFA_Element::SharpxHTML;
    else if (wsContentType.EqualsASCII("text/xml"))
      element = XFA_Element::Sharpxml;
  }
  if (element == XFA_Element::SharpxHTML)
    pXFANode->SetXMLMappingNode(pXMLNode);

  WideString wsValue;
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    CFX_XMLNode::Type eNodeType = pXMLChild->GetType();
    if (eNodeType == CFX_XMLNode::Type::kInstruction)
      continue;

    CFX_XMLElement* pElement = ToXMLElement(pXMLChild);
    if (element == XFA_Element::SharpxHTML) {
      if (!pElement)
        break;
      if (XFA_RecognizeRichText(pElement))
        wsValue += GetPlainTextFromRichText(pElement);
    } else if (element == XFA_Element::Sharpxml) {
      if (!pElement)
        break;
      ConvertXMLToPlainText(pElement, wsValue);
    } else {
      if (pElement)
        break;
      CFX_XMLText* pText = ToXMLText(pXMLChild);
      if (pText)
        wsValue = pText->GetText();
    }
    break;
  }
  if (!wsValue.IsEmpty()) {
    if (pXFANode->IsContentNode()) {
      CXFA_Node* pContentRawDataNode =
          node_factory_->CreateNode(ePacketID, element);
      DCHECK(pContentRawDataNode);
      pContentRawDataNode->JSObject()->SetCData(XFA_Attribute::Value, wsValue);
      pXFANode->InsertChildAndNotify(pContentRawDataNode, nullptr);
    } else {
      pXFANode->JSObject()->SetCData(XFA_Attribute::Value, wsValue);
    }
  }
}

void CXFA_DocumentBuilder::ParseDataGroup(CXFA_Node* pXFANode,
                                          CFX_XMLNode* pXMLNode,
                                          XFA_PacketType ePacketID) {
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    switch (pXMLChild->GetType()) {
      case CFX_XMLNode::Type::kElement: {
        CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLChild);
        WideString wsNamespaceURI = pXMLElement->GetNamespaceURI();
        if (wsNamespaceURI.EqualsASCII(
                "http://www.xfa.com/schema/xfa-package/") ||
            wsNamespaceURI.EqualsASCII(
                "http://www.xfa.org/schema/xfa-package/") ||
            wsNamespaceURI.EqualsASCII(
                "http://www.w3.org/2001/XMLSchema-instance")) {
          continue;
        }

        XFA_Element eNodeType = XFA_Element::DataModel;
        if (eNodeType == XFA_Element::DataModel) {
          std::optional<WideString> wsDataNodeAttr =
              FindAttributeWithNS(pXMLElement, L"dataNode",
                                  L"http://www.xfa.org/schema/xfa-data/1.0/");
          if (wsDataNodeAttr.has_value()) {
            if (wsDataNodeAttr.value().EqualsASCII("dataGroup"))
              eNodeType = XFA_Element::DataGroup;
            else if (wsDataNodeAttr.value().EqualsASCII("dataValue"))
              eNodeType = XFA_Element::DataValue;
          }
        }
        if (eNodeType == XFA_Element::DataModel) {
          std::optional<WideString> wsContentType =
              FindAttributeWithNS(pXMLElement, L"contentType",
                                  L"http://www.xfa.org/schema/xfa-data/1.0/");
          if (wsContentType.has_value() && !wsContentType.value().IsEmpty())
            eNodeType = XFA_Element::DataValue;
        }
        if (eNodeType == XFA_Element::DataModel) {
          for (CFX_XMLNode* pXMLDataChild = pXMLElement->GetFirstChild();
               pXMLDataChild; pXMLDataChild = pXMLDataChild->GetNextSibling()) {
            CFX_XMLElement* pElement = ToXMLElement(pXMLDataChild);
            if (pElement && !XFA_RecognizeRichText(pElement)) {
              eNodeType = XFA_Element::DataGroup;
              break;
            }
          }
        }
        if (eNodeType == XFA_Element::DataModel)
          eNodeType = XFA_Element::DataValue;

        CXFA_Node* pXFAChild =
            node_factory_->CreateNode(XFA_PacketType::Datasets, eNodeType);
        if (!pXFAChild)
          return;

        pXFAChild->JSObject()->SetCData(XFA_Attribute::Name,
                                        pXMLElement->GetLocalTagName());
        bool bNeedValue = true;

        for (auto it : pXMLElement->GetAttributes()) {
          WideString wsName;
          WideString wsNS;
          if (!ResolveAttribute(pXMLElement, it.first, wsName, wsNS)) {
            continue;
          }
          if (wsName.EqualsASCII("nil") && it.second.EqualsASCII("true")) {
            bNeedValue = false;
            continue;
          }
          if (wsNS.EqualsASCII("http://www.xfa.com/schema/xfa-package/") ||
              wsNS.EqualsASCII("http://www.xfa.org/schema/xfa-package/") ||
              wsNS.EqualsASCII("http://www.w3.org/2001/XMLSchema-instance") ||
              wsNS.EqualsASCII("http://www.xfa.org/schema/xfa-data/1.0/")) {
            continue;
          }
          CXFA_Node* pXFAMetaData = node_factory_->CreateNode(
              XFA_PacketType::Datasets, XFA_Element::DataValue);
          if (!pXFAMetaData)
            return;

          pXFAMetaData->JSObject()->SetCData(XFA_Attribute::Name, wsName);
          pXFAMetaData->JSObject()->SetCData(XFA_Attribute::QualifiedName,
                                             it.first);
          pXFAMetaData->JSObject()->SetCData(XFA_Attribute::Value, it.second);
          pXFAMetaData->JSObject()->SetEnum(
              XFA_Attribute::Contains, XFA_AttributeValue::MetaData, false);
          pXFAChild->InsertChildAndNotify(pXFAMetaData, nullptr);
          pXFAMetaData->SetXMLMappingNode(pXMLElement);
          pXFAMetaData->SetFlag(XFA_NodeFlag::kInitialized);
        }

        if (!bNeedValue)
          pXMLElement->RemoveAttribute(L"xsi:nil");

        pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
        if (eNodeType == XFA_Element::DataGroup)
          ParseDataGroup(pXFAChild, pXMLElement, ePacketID);
        else if (bNeedValue)
          ParseDataValue(pXFAChild, pXMLChild, XFA_PacketType::Datasets);

        pXFAChild->SetXMLMappingNode(pXMLElement);
        pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
        continue;
      }
      case CFX_XMLNode::Type::kCharData:
      case CFX_XMLNode::Type::kText: {
        CFX_XMLText* pXMLText = ToXMLText(pXMLChild);
        WideString wsText = pXMLText->GetText();
        if (IsStringAllWhitespace(wsText))
          continue;

        CXFA_Node* pXFAChild = node_factory_->CreateNode(
            XFA_PacketType::Datasets, XFA_Element::DataValue);
        if (!pXFAChild)
          return;

        pXFAChild->JSObject()->SetCData(XFA_Attribute::Value, wsText);
        pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
        pXFAChild->SetXMLMappingNode(pXMLText);
        pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
        continue;
      }
      default:
        continue;
    }
  }
}

void CXFA_DocumentBuilder::ParseDataValue(CXFA_Node* pXFANode,
                                          CFX_XMLNode* pXMLNode,
                                          XFA_PacketType ePacketID) {
  WideString wsValue;
  WideString wsCurValue;
  bool bMarkAsCompound = false;
  CFX_XMLNode* pXMLCurValueNode = nullptr;
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetFirstChild(); pXMLChild;
       pXMLChild = pXMLChild->GetNextSibling()) {
    CFX_XMLNode::Type eNodeType = pXMLChild->GetType();
    if (eNodeType == CFX_XMLNode::Type::kInstruction)
      continue;

    CFX_XMLText* pText = ToXMLText(pXMLChild);
    if (pText) {
      WideString wsText = pText->GetText();
      if (!pXMLCurValueNode)
        pXMLCurValueNode = pXMLChild;
      wsCurValue += wsText;
      continue;
    }
    if (XFA_RecognizeRichText(ToXMLElement(pXMLChild))) {
      WideString wsText = GetPlainTextFromRichText(ToXMLElement(pXMLChild));
      if (!pXMLCurValueNode)
        pXMLCurValueNode = pXMLChild;
      wsCurValue += wsText;
      continue;
    }
    bMarkAsCompound = true;
    if (pXMLCurValueNode) {
      if (!wsCurValue.IsEmpty()) {
        CXFA_Node* pXFAChild =
            node_factory_->CreateNode(ePacketID, XFA_Element::DataValue);
        if (!pXFAChild)
          return;

        pXFAChild->JSObject()->SetCData(XFA_Attribute::Name, WideString());
        pXFAChild->JSObject()->SetCData(XFA_Attribute::Value, wsCurValue);
        pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
        pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
        pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
        wsValue += wsCurValue;
        wsCurValue.clear();
      }
      pXMLCurValueNode = nullptr;
    }
    CXFA_Node* pXFAChild =
        node_factory_->CreateNode(ePacketID, XFA_Element::DataValue);
    if (!pXFAChild)
      return;

    WideString wsNodeStr = ToXMLElement(pXMLChild)->GetLocalTagName();
    pXFAChild->JSObject()->SetCData(XFA_Attribute::Name, wsNodeStr);
    ParseDataValue(pXFAChild, pXMLChild, ePacketID);
    pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
    pXFAChild->SetXMLMappingNode(pXMLChild);
    pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
    wsValue += pXFAChild->JSObject()->GetCData(XFA_Attribute::Value);
  }

  if (pXMLCurValueNode) {
    if (!wsCurValue.IsEmpty()) {
      if (bMarkAsCompound) {
        CXFA_Node* pXFAChild =
            node_factory_->CreateNode(ePacketID, XFA_Element::DataValue);
        if (!pXFAChild)
          return;

        pXFAChild->JSObject()->SetCData(XFA_Attribute::Name, WideString());
        pXFAChild->JSObject()->SetCData(XFA_Attribute::Value, wsCurValue);
        pXFANode->InsertChildAndNotify(pXFAChild, nullptr);
        pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
        pXFAChild->SetFlag(XFA_NodeFlag::kInitialized);
      }
      wsValue += wsCurValue;
      wsCurValue.clear();
    }
    pXMLCurValueNode = nullptr;
  }
  pXFANode->JSObject()->SetCData(XFA_Attribute::Value, wsValue);
}

void CXFA_DocumentBuilder::ParseInstruction(CXFA_Node* pXFANode,
                                            CFX_XMLInstruction* pXMLInstruction,
                                            XFA_PacketType ePacketID) {
  const std::vector<WideString>& target_data = pXMLInstruction->GetTargetData();
  if (pXMLInstruction->IsOriginalXFAVersion()) {
    if (target_data.size() > 1 &&
        (pXFANode->GetDocument()->RecognizeXFAVersionNumber(target_data[0]) !=
         XFA_VERSION_UNKNOWN) &&
        target_data[1].EqualsASCII("v2.7-scripting:1")) {
      pXFANode->GetDocument()->set_is_scripting();
    }
    return;
  }
  if (pXMLInstruction->IsAcrobat()) {
    if (target_data.size() > 1 && target_data[0].EqualsASCII("JavaScript") &&
        target_data[1].EqualsASCII("strictScoping")) {
      pXFANode->GetDocument()->set_is_strict_scoping();
    }
  }
}
