// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_simple_parser.h"

#include <utility>
#include <vector>

#include "core/fxcrt/cfx_checksumcontext.h"
#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

CFX_XMLNode* GetDocumentNode(CFX_XMLDoc* pXMLDoc,
                             bool bVerifyWellFormness = false) {
  if (!pXMLDoc)
    return nullptr;

  for (CFX_XMLNode* pXMLNode =
           pXMLDoc->GetRoot()->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() != FX_XMLNODE_Element)
      continue;

    if (!bVerifyWellFormness)
      return pXMLNode;

    for (CFX_XMLNode* pNextNode =
             pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling);
         pNextNode;
         pNextNode = pNextNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
      if (pNextNode->GetType() == FX_XMLNODE_Element)
        return nullptr;
    }
    return pXMLNode;
  }
  return nullptr;
}

CFX_WideString GetElementTagNamespaceURI(CFX_XMLElement* pElement) {
  CFX_WideString wsNodeStr = pElement->GetNamespacePrefix();
  CFX_WideString wsNamespaceURI;
  if (!XFA_FDEExtension_ResolveNamespaceQualifier(
          pElement, wsNodeStr.AsStringC(), &wsNamespaceURI)) {
    return CFX_WideString();
  }
  return wsNamespaceURI;
}

bool MatchNodeName(CFX_XMLNode* pNode,
                   const CFX_WideStringC& wsLocalTagName,
                   const CFX_WideStringC& wsNamespaceURIPrefix,
                   uint32_t eMatchFlags = XFA_XDPPACKET_FLAGS_NOMATCH) {
  if (!pNode || pNode->GetType() != FX_XMLNODE_Element)
    return false;

  CFX_XMLElement* pElement = reinterpret_cast<CFX_XMLElement*>(pNode);
  CFX_WideString wsNodeStr = pElement->GetLocalTagName();
  if (wsNodeStr != wsLocalTagName)
    return false;

  wsNodeStr = GetElementTagNamespaceURI(pElement);
  if (eMatchFlags & XFA_XDPPACKET_FLAGS_NOMATCH)
    return true;
  if (eMatchFlags & XFA_XDPPACKET_FLAGS_PREFIXMATCH) {
    return wsNodeStr.Left(wsNamespaceURIPrefix.GetLength()) ==
           wsNamespaceURIPrefix;
  }

  return wsNodeStr == wsNamespaceURIPrefix;
}

bool GetAttributeLocalName(const CFX_WideStringC& wsAttributeName,
                           CFX_WideString& wsLocalAttrName) {
  CFX_WideString wsAttrName(wsAttributeName);
  FX_STRSIZE iFind = wsAttrName.Find(L':', 0);
  if (iFind < 0) {
    wsLocalAttrName = wsAttrName;
    return false;
  }
  wsLocalAttrName = wsAttrName.Right(wsAttrName.GetLength() - iFind - 1);
  return true;
}

bool ResolveAttribute(CFX_XMLElement* pElement,
                      const CFX_WideStringC& wsAttributeName,
                      CFX_WideString& wsLocalAttrName,
                      CFX_WideString& wsNamespaceURI) {
  CFX_WideString wsAttrName(wsAttributeName);
  CFX_WideString wsNSPrefix;
  if (GetAttributeLocalName(wsAttributeName, wsLocalAttrName)) {
    wsNSPrefix = wsAttrName.Left(wsAttributeName.GetLength() -
                                 wsLocalAttrName.GetLength() - 1);
  }
  if (wsLocalAttrName == L"xmlns" || wsNSPrefix == L"xmlns" ||
      wsNSPrefix == L"xml") {
    return false;
  }
  if (!XFA_FDEExtension_ResolveNamespaceQualifier(
          pElement, wsNSPrefix.AsStringC(), &wsNamespaceURI)) {
    wsNamespaceURI.clear();
    return false;
  }
  return true;
}

bool FindAttributeWithNS(CFX_XMLElement* pElement,
                         const CFX_WideStringC& wsLocalAttributeName,
                         const CFX_WideStringC& wsNamespaceURIPrefix,
                         CFX_WideString& wsValue,
                         bool bMatchNSAsPrefix = false) {
  if (!pElement)
    return false;

  CFX_WideString wsAttrNS;
  for (auto it : pElement->GetAttributes()) {
    FX_STRSIZE iFind = it.first.Find(L':', 0);
    CFX_WideString wsNSPrefix;
    if (iFind < 0) {
      if (wsLocalAttributeName != it.first)
        continue;
    } else {
      if (wsLocalAttributeName !=
          it.first.Right(it.first.GetLength() - iFind - 1)) {
        continue;
      }
      wsNSPrefix = it.first.Left(iFind);
    }

    if (!XFA_FDEExtension_ResolveNamespaceQualifier(
            pElement, wsNSPrefix.AsStringC(), &wsAttrNS)) {
      continue;
    }
    if (bMatchNSAsPrefix) {
      if (wsAttrNS.Left(wsNamespaceURIPrefix.GetLength()) !=
          wsNamespaceURIPrefix) {
        continue;
      }
    } else {
      if (wsAttrNS != wsNamespaceURIPrefix)
        continue;
    }
    wsValue = it.second;
    return true;
  }
  return false;
}

CFX_XMLNode* GetDataSetsFromXDP(CFX_XMLNode* pXMLDocumentNode) {
  if (MatchNodeName(pXMLDocumentNode,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
    return pXMLDocumentNode;
  }
  if (!MatchNodeName(pXMLDocumentNode,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->pName,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->pURI,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->eFlags)) {
    return nullptr;
  }
  for (CFX_XMLNode* pDatasetsNode =
           pXMLDocumentNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pDatasetsNode;
       pDatasetsNode = pDatasetsNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (!MatchNodeName(pDatasetsNode,
                       XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName,
                       XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
                       XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
      continue;
    }
    return pDatasetsNode;
  }
  return nullptr;
}

bool IsStringAllWhitespace(CFX_WideString wsText) {
  wsText.TrimRight(L"\x20\x9\xD\xA");
  return wsText.IsEmpty();
}

void ConvertXMLToPlainText(CFX_XMLElement* pRootXMLNode,
                           CFX_WideString& wsOutput) {
  for (CFX_XMLNode* pXMLChild =
           pRootXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    switch (pXMLChild->GetType()) {
      case FX_XMLNODE_Element: {
        CFX_WideString wsTextData =
            static_cast<CFX_XMLElement*>(pXMLChild)->GetTextData();
        wsTextData += L"\n";
        wsOutput += wsTextData;
        break;
      }
      case FX_XMLNODE_Text:
      case FX_XMLNODE_CharData: {
        CFX_WideString wsText = static_cast<CFX_XMLText*>(pXMLChild)->GetText();
        if (IsStringAllWhitespace(wsText))
          continue;

        wsOutput = wsText;
        break;
      }
      default:
        NOTREACHED();
        break;
    }
  }
}

const XFA_PACKETINFO* GetPacketByName(const CFX_WideStringC& wsName) {
  if (wsName.IsEmpty())
    return nullptr;

  uint32_t uHash = FX_HashCode_GetW(wsName, false);
  int32_t iStart = 0;
  int32_t iEnd = g_iXFAPacketCount - 1;
  do {
    int32_t iMid = (iStart + iEnd) / 2;
    const XFA_PACKETINFO* pInfo = g_XFAPacketData + iMid;
    if (uHash == pInfo->uHash)
      return pInfo;
    if (uHash < pInfo->uHash)
      iEnd = iMid - 1;
    else
      iStart = iMid + 1;
  } while (iStart <= iEnd);
  return nullptr;
}

}  // namespace

bool XFA_RecognizeRichText(CFX_XMLElement* pRichTextXMLNode) {
  return pRichTextXMLNode && GetElementTagNamespaceURI(pRichTextXMLNode) ==
                                 L"http://www.w3.org/1999/xhtml";
}

CXFA_SimpleParser::CXFA_SimpleParser(CXFA_Document* pFactory,
                                     bool bDocumentParser)
    : m_pXMLParser(nullptr),
      m_pXMLDoc(nullptr),
      m_pStream(nullptr),
      m_pFileRead(nullptr),
      m_pFactory(pFactory),
      m_pRootNode(nullptr),
      m_ePacketID(XFA_XDPPACKET_UNKNOWN),
      m_bDocumentParser(bDocumentParser) {}

CXFA_SimpleParser::~CXFA_SimpleParser() {}

void CXFA_SimpleParser::SetFactory(CXFA_Document* pFactory) {
  m_pFactory = pFactory;
}

int32_t CXFA_SimpleParser::StartParse(
    const CFX_RetainPtr<IFX_SeekableStream>& pStream,
    XFA_XDPPACKET ePacketID) {
  CloseParser();
  m_pFileRead = pStream;
  m_pStream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pStream, false);
  uint16_t wCodePage = m_pStream->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    m_pStream->SetCodePage(FX_CODEPAGE_UTF8);
  }
  m_pXMLDoc = pdfium::MakeUnique<CFX_XMLDoc>();
  auto pNewParser =
      pdfium::MakeUnique<CFX_XMLParser>(m_pXMLDoc->GetRoot(), m_pStream);
  m_pXMLParser = pNewParser.get();
  if (!m_pXMLDoc->LoadXML(std::move(pNewParser)))
    return XFA_PARSESTATUS_StatusErr;

  m_ePacketID = ePacketID;
  return XFA_PARSESTATUS_Ready;
}

int32_t CXFA_SimpleParser::DoParse() {
  if (!m_pXMLDoc || m_ePacketID == XFA_XDPPACKET_UNKNOWN)
    return XFA_PARSESTATUS_StatusErr;

  int32_t iRet = m_pXMLDoc->DoLoad();
  if (iRet < 0)
    return XFA_PARSESTATUS_SyntaxErr;
  if (iRet < 100)
    return iRet / 2;

  m_pRootNode = ParseAsXDPPacket(GetDocumentNode(m_pXMLDoc.get()), m_ePacketID);
  m_pXMLDoc->CloseXML();
  m_pStream.Reset();

  if (!m_pRootNode)
    return XFA_PARSESTATUS_StatusErr;

  return XFA_PARSESTATUS_Done;
}

CFX_XMLNode* CXFA_SimpleParser::ParseXMLData(const CFX_ByteString& wsXML) {
  CloseParser();
  m_pXMLDoc = pdfium::MakeUnique<CFX_XMLDoc>();

  auto pStream = pdfium::MakeRetain<CFX_SeekableStreamProxy>(
      const_cast<uint8_t*>(wsXML.raw_str()), wsXML.GetLength());
  auto pParser =
      pdfium::MakeUnique<CFX_XMLParser>(m_pXMLDoc->GetRoot(), pStream);
  pParser->m_dwCheckStatus = 0x03;
  if (!m_pXMLDoc->LoadXML(std::move(pParser)))
    return nullptr;

  int32_t iRet = m_pXMLDoc->DoLoad();
  if (iRet < 0 || iRet >= 100)
    m_pXMLDoc->CloseXML();
  return iRet < 100 ? nullptr : GetDocumentNode(m_pXMLDoc.get());
}

void CXFA_SimpleParser::ConstructXFANode(CXFA_Node* pXFANode,
                                         CFX_XMLNode* pXMLNode) {
  XFA_XDPPACKET ePacketID = (XFA_XDPPACKET)pXFANode->GetPacketID();
  if (ePacketID == XFA_XDPPACKET_Datasets) {
    if (pXFANode->GetElementType() == XFA_Element::DataValue) {
      for (CFX_XMLNode* pXMLChild =
               pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
           pXMLChild;
           pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
        FX_XMLNODETYPE eNodeType = pXMLChild->GetType();
        if (eNodeType == FX_XMLNODE_Instruction)
          continue;

        if (eNodeType == FX_XMLNODE_Element) {
          CXFA_Node* pXFAChild = m_pFactory->CreateNode(XFA_XDPPACKET_Datasets,
                                                        XFA_Element::DataValue);
          if (!pXFAChild)
            return;

          CFX_XMLElement* child = static_cast<CFX_XMLElement*>(pXMLChild);
          CFX_WideString wsNodeStr = child->GetLocalTagName();
          pXFAChild->SetCData(XFA_ATTRIBUTE_Name, wsNodeStr);
          CFX_WideString wsChildValue;
          XFA_GetPlainTextFromRichText(child, wsChildValue);
          if (!wsChildValue.IsEmpty())
            pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsChildValue);

          pXFANode->InsertChild(pXFAChild);
          pXFAChild->SetXMLMappingNode(pXMLChild);
          pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
          break;
        }
      }
      m_pRootNode = pXFANode;
    } else {
      m_pRootNode = DataLoader(pXFANode, pXMLNode, true);
    }
  } else if (pXFANode->IsContentNode()) {
    ParseContentNode(pXFANode, pXMLNode, ePacketID);
    m_pRootNode = pXFANode;
  } else {
    m_pRootNode = NormalLoader(pXFANode, pXMLNode, ePacketID, true);
  }
}

CXFA_Node* CXFA_SimpleParser::GetRootNode() const {
  return m_pRootNode;
}

CFX_XMLDoc* CXFA_SimpleParser::GetXMLDoc() const {
  return m_pXMLDoc.get();
}

bool XFA_FDEExtension_ResolveNamespaceQualifier(
    CFX_XMLElement* pNode,
    const CFX_WideStringC& wsQualifier,
    CFX_WideString* wsNamespaceURI) {
  if (!pNode)
    return false;

  CFX_XMLNode* pFakeRoot = pNode->GetNodeItem(CFX_XMLNode::Root);
  CFX_WideString wsNSAttribute;
  bool bRet = false;
  if (wsQualifier.IsEmpty()) {
    wsNSAttribute = L"xmlns";
    bRet = true;
  } else {
    wsNSAttribute = L"xmlns:" + wsQualifier;
  }
  for (CFX_XMLNode* pParent = pNode; pParent != pFakeRoot;
       pParent = pParent->GetNodeItem(CFX_XMLNode::Parent)) {
    if (pParent->GetType() != FX_XMLNODE_Element)
      continue;

    auto* pElement = static_cast<CFX_XMLElement*>(pParent);
    if (pElement->HasAttribute(wsNSAttribute.c_str())) {
      *wsNamespaceURI = pElement->GetString(wsNSAttribute.c_str());
      return true;
    }
  }
  wsNamespaceURI->clear();
  return bRet;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket(CFX_XMLNode* pXMLDocumentNode,
                                               XFA_XDPPACKET ePacketID) {
  switch (ePacketID) {
    case XFA_XDPPACKET_UNKNOWN:
      return nullptr;
    case XFA_XDPPACKET_XDP:
      return ParseAsXDPPacket_XDP(pXMLDocumentNode, ePacketID);
    case XFA_XDPPACKET_Config:
      return ParseAsXDPPacket_Config(pXMLDocumentNode, ePacketID);
    case XFA_XDPPACKET_Template:
    case XFA_XDPPACKET_Form:
      return ParseAsXDPPacket_TemplateForm(pXMLDocumentNode, ePacketID);
    case XFA_XDPPACKET_Datasets:
      return ParseAsXDPPacket_Data(pXMLDocumentNode, ePacketID);
    case XFA_XDPPACKET_Xdc:
      return ParseAsXDPPacket_Xdc(pXMLDocumentNode, ePacketID);
    case XFA_XDPPACKET_LocaleSet:
    case XFA_XDPPACKET_ConnectionSet:
    case XFA_XDPPACKET_SourceSet:
      return ParseAsXDPPacket_LocaleConnectionSourceSet(pXMLDocumentNode,
                                                        ePacketID);
    default:
      return ParseAsXDPPacket_User(pXMLDocumentNode, ePacketID);
  }
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_XDP(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (!MatchNodeName(pXMLDocumentNode,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->pName,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->pURI,
                     XFA_GetPacketByIndex(XFA_PACKET_XDP)->eFlags)) {
    return nullptr;
  }

  CXFA_Node* pXFARootNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_XDP, XFA_Element::Xfa);
  if (!pXFARootNode)
    return nullptr;

  m_pRootNode = pXFARootNode;
  pXFARootNode->SetCData(XFA_ATTRIBUTE_Name, L"xfa");

  CFX_XMLElement* pElement = static_cast<CFX_XMLElement*>(pXMLDocumentNode);
  for (auto it : pElement->GetAttributes()) {
    if (it.first == L"uuid")
      pXFARootNode->SetCData(XFA_ATTRIBUTE_Uuid, it.second);
    else if (it.first == L"timeStamp")
      pXFARootNode->SetCData(XFA_ATTRIBUTE_TimeStamp, it.second);
  }

  CFX_XMLNode* pXMLConfigDOMRoot = nullptr;
  CXFA_Node* pXFAConfigDOMRoot = nullptr;
  for (CFX_XMLNode* pChildItem =
           pXMLDocumentNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pChildItem;
       pChildItem = pChildItem->GetNodeItem(CFX_XMLNode::NextSibling)) {
    const XFA_PACKETINFO* pPacketInfo = XFA_GetPacketByIndex(XFA_PACKET_Config);
    if (!MatchNodeName(pChildItem, pPacketInfo->pName, pPacketInfo->pURI,
                       pPacketInfo->eFlags)) {
      continue;
    }
    if (pXFARootNode->GetFirstChildByName(pPacketInfo->uHash))
      return nullptr;

    pXMLConfigDOMRoot = pChildItem;
    pXFAConfigDOMRoot =
        ParseAsXDPPacket_Config(pXMLConfigDOMRoot, XFA_XDPPACKET_Config);
    if (pXFAConfigDOMRoot)
      pXFARootNode->InsertChild(pXFAConfigDOMRoot, nullptr);
  }

  CFX_XMLNode* pXMLDatasetsDOMRoot = nullptr;
  CFX_XMLNode* pXMLFormDOMRoot = nullptr;
  CFX_XMLNode* pXMLTemplateDOMRoot = nullptr;
  for (CFX_XMLNode* pChildItem =
           pXMLDocumentNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pChildItem;
       pChildItem = pChildItem->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (!pChildItem || pChildItem->GetType() != FX_XMLNODE_Element)
      continue;
    if (pChildItem == pXMLConfigDOMRoot)
      continue;

    CFX_XMLElement* pElement = reinterpret_cast<CFX_XMLElement*>(pChildItem);
    CFX_WideString wsPacketName = pElement->GetLocalTagName();
    const XFA_PACKETINFO* pPacketInfo =
        GetPacketByName(wsPacketName.AsStringC());
    if (pPacketInfo && pPacketInfo->pURI) {
      if (!MatchNodeName(pElement, pPacketInfo->pName, pPacketInfo->pURI,
                         pPacketInfo->eFlags)) {
        pPacketInfo = nullptr;
      }
    }
    XFA_XDPPACKET ePacket =
        pPacketInfo ? pPacketInfo->eName : XFA_XDPPACKET_USER;
    if (ePacket == XFA_XDPPACKET_XDP)
      continue;
    if (ePacket == XFA_XDPPACKET_Datasets) {
      if (pXMLDatasetsDOMRoot)
        return nullptr;

      pXMLDatasetsDOMRoot = pElement;
    } else if (ePacket == XFA_XDPPACKET_Form) {
      if (pXMLFormDOMRoot)
        return nullptr;

      pXMLFormDOMRoot = pElement;
    } else if (ePacket == XFA_XDPPACKET_Template) {
      // Found a duplicate template packet.
      if (pXMLTemplateDOMRoot)
        return nullptr;

      CXFA_Node* pPacketNode = ParseAsXDPPacket(pElement, ePacket);
      if (pPacketNode) {
        pXMLTemplateDOMRoot = pElement;
        pXFARootNode->InsertChild(pPacketNode);
      }
    } else {
      CXFA_Node* pPacketNode = ParseAsXDPPacket(pElement, ePacket);
      if (pPacketNode) {
        if (pPacketInfo &&
            (pPacketInfo->eFlags & XFA_XDPPACKET_FLAGS_SUPPORTONE) &&
            pXFARootNode->GetFirstChildByName(pPacketInfo->uHash)) {
          return nullptr;
        }
        pXFARootNode->InsertChild(pPacketNode);
      }
    }
  }

  // No template is found.
  if (!pXMLTemplateDOMRoot)
    return nullptr;

  if (pXMLDatasetsDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLDatasetsDOMRoot, XFA_XDPPACKET_Datasets);
    if (pPacketNode)
      pXFARootNode->InsertChild(pPacketNode);
  }
  if (pXMLFormDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLFormDOMRoot, XFA_XDPPACKET_Form);
    if (pPacketNode)
      pXFARootNode->InsertChild(pPacketNode);
  }

  pXFARootNode->SetXMLMappingNode(pXMLDocumentNode);
  return pXFARootNode;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Config(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (!MatchNodeName(pXMLDocumentNode,
                     XFA_GetPacketByIndex(XFA_PACKET_Config)->pName,
                     XFA_GetPacketByIndex(XFA_PACKET_Config)->pURI,
                     XFA_GetPacketByIndex(XFA_PACKET_Config)->eFlags)) {
    return nullptr;
  }
  CXFA_Node* pNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_Config, XFA_Element::Config);
  if (!pNode)
    return nullptr;

  pNode->SetCData(XFA_ATTRIBUTE_Name,
                  XFA_GetPacketByIndex(XFA_PACKET_Config)->pName);
  if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, true))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_TemplateForm(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode = nullptr;
  if (ePacketID == XFA_XDPPACKET_Template) {
    if (MatchNodeName(pXMLDocumentNode,
                      XFA_GetPacketByIndex(XFA_PACKET_Template)->pName,
                      XFA_GetPacketByIndex(XFA_PACKET_Template)->pURI,
                      XFA_GetPacketByIndex(XFA_PACKET_Template)->eFlags)) {
      pNode =
          m_pFactory->CreateNode(XFA_XDPPACKET_Template, XFA_Element::Template);
      if (!pNode)
        return nullptr;

      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_Template)->pName);
      if (m_bDocumentParser) {
        CFX_XMLElement* pXMLDocumentElement =
            static_cast<CFX_XMLElement*>(pXMLDocumentNode);
        CFX_WideString wsNamespaceURI = pXMLDocumentElement->GetNamespaceURI();
        if (wsNamespaceURI.IsEmpty())
          wsNamespaceURI = pXMLDocumentElement->GetString(L"xmlns:xfa");

        pNode->GetDocument()->RecognizeXFAVersionNumber(wsNamespaceURI);
      }
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, true))
        return nullptr;
    }
  } else if (ePacketID == XFA_XDPPACKET_Form) {
    if (MatchNodeName(pXMLDocumentNode,
                      XFA_GetPacketByIndex(XFA_PACKET_Form)->pName,
                      XFA_GetPacketByIndex(XFA_PACKET_Form)->pURI,
                      XFA_GetPacketByIndex(XFA_PACKET_Form)->eFlags)) {
      CFX_XMLElement* pXMLDocumentElement =
          static_cast<CFX_XMLElement*>(pXMLDocumentNode);
      CFX_WideString wsChecksum = pXMLDocumentElement->GetString(L"checksum");
      if (wsChecksum.GetLength() != 28 ||
          m_pXMLParser->m_dwCheckStatus != 0x03) {
        return nullptr;
      }

      auto pChecksum = pdfium::MakeUnique<CFX_ChecksumContext>();
      pChecksum->StartChecksum();
      pChecksum->UpdateChecksum(m_pFileRead, m_pXMLParser->m_nStart[0],
                                m_pXMLParser->m_nSize[0]);
      pChecksum->UpdateChecksum(m_pFileRead, m_pXMLParser->m_nStart[1],
                                m_pXMLParser->m_nSize[1]);
      pChecksum->FinishChecksum();
      CFX_ByteString bsCheck = pChecksum->GetChecksum();
      if (bsCheck != wsChecksum.UTF8Encode())
        return nullptr;

      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_Form, XFA_Element::Form);
      if (!pNode)
        return nullptr;

      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_Form)->pName);
      pNode->SetAttribute(XFA_ATTRIBUTE_Checksum, wsChecksum.AsStringC());
      CXFA_Node* pTemplateRoot =
          m_pRootNode->GetFirstChildByClass(XFA_Element::Template);
      CXFA_Node* pTemplateChosen =
          pTemplateRoot
              ? pTemplateRoot->GetFirstChildByClass(XFA_Element::Subform)
              : nullptr;
      bool bUseAttribute = true;
      if (pTemplateChosen &&
          pTemplateChosen->GetEnum(XFA_ATTRIBUTE_RestoreState) !=
              XFA_ATTRIBUTEENUM_Auto) {
        bUseAttribute = false;
      }
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, bUseAttribute))
        return nullptr;
    }
  }
  if (pNode)
    pNode->SetXMLMappingNode(pXMLDocumentNode);

  return pNode;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Data(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CFX_XMLNode* pDatasetsXMLNode = GetDataSetsFromXDP(pXMLDocumentNode);
  if (pDatasetsXMLNode) {
    CXFA_Node* pNode =
        m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, XFA_Element::DataModel);
    if (!pNode)
      return nullptr;

    pNode->SetCData(XFA_ATTRIBUTE_Name,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName);
    if (!DataLoader(pNode, pDatasetsXMLNode, false))
      return nullptr;

    pNode->SetXMLMappingNode(pDatasetsXMLNode);
    return pNode;
  }

  CFX_XMLNode* pDataXMLNode = nullptr;
  if (MatchNodeName(pXMLDocumentNode, L"data",
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
    static_cast<CFX_XMLElement*>(pXMLDocumentNode)
        ->RemoveAttribute(L"xmlns:xfa");
    pDataXMLNode = pXMLDocumentNode;
  } else {
    CFX_XMLElement* pDataElement = new CFX_XMLElement(L"xfa:data");
    CFX_XMLNode* pParentXMLNode =
        pXMLDocumentNode->GetNodeItem(CFX_XMLNode::Parent);
    if (pParentXMLNode)
      pParentXMLNode->RemoveChildNode(pXMLDocumentNode);

    ASSERT(pXMLDocumentNode->GetType() == FX_XMLNODE_Element);
    if (pXMLDocumentNode->GetType() == FX_XMLNODE_Element) {
      static_cast<CFX_XMLElement*>(pXMLDocumentNode)
          ->RemoveAttribute(L"xmlns:xfa");
    }
    pDataElement->InsertChildNode(pXMLDocumentNode);
    pDataXMLNode = pDataElement;
  }

  if (pDataXMLNode) {
    CXFA_Node* pNode =
        m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, XFA_Element::DataGroup);
    if (!pNode) {
      if (pDataXMLNode != pXMLDocumentNode)
        delete pDataXMLNode;
      return nullptr;
    }
    CFX_WideString wsLocalName =
        static_cast<CFX_XMLElement*>(pDataXMLNode)->GetLocalTagName();
    pNode->SetCData(XFA_ATTRIBUTE_Name, wsLocalName);
    if (!DataLoader(pNode, pDataXMLNode, true))
      return nullptr;

    pNode->SetXMLMappingNode(pDataXMLNode);
    if (pDataXMLNode != pXMLDocumentNode)
      pNode->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
    return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_LocaleConnectionSourceSet(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode = nullptr;
  if (ePacketID == XFA_XDPPACKET_LocaleSet) {
    if (MatchNodeName(pXMLDocumentNode,
                      XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pName,
                      XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pURI,
                      XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_LocaleSet,
                                     XFA_Element::LocaleSet);
      if (!pNode)
        return nullptr;

      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, true))
        return nullptr;
    }
  } else if (ePacketID == XFA_XDPPACKET_ConnectionSet) {
    if (MatchNodeName(pXMLDocumentNode,
                      XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pName,
                      XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pURI,
                      XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_ConnectionSet,
                                     XFA_Element::ConnectionSet);
      if (!pNode)
        return nullptr;

      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, true))
        return nullptr;
    }
  } else if (ePacketID == XFA_XDPPACKET_SourceSet) {
    if (MatchNodeName(pXMLDocumentNode,
                      XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pName,
                      XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pURI,
                      XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_SourceSet,
                                     XFA_Element::SourceSet);
      if (!pNode)
        return nullptr;

      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, true))
        return nullptr;
    }
  }
  if (pNode)
    pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Xdc(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (!MatchNodeName(pXMLDocumentNode,
                     XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pName,
                     XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pURI,
                     XFA_GetPacketByIndex(XFA_PACKET_Xdc)->eFlags))
    return nullptr;

  CXFA_Node* pNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_Xdc, XFA_Element::Xdc);
  if (!pNode)
    return nullptr;

  pNode->SetCData(XFA_ATTRIBUTE_Name,
                  XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pName);
  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_User(
    CFX_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_XDP, XFA_Element::Packet);
  if (!pNode)
    return nullptr;

  CFX_WideString wsName =
      static_cast<CFX_XMLElement*>(pXMLDocumentNode)->GetLocalTagName();
  pNode->SetCData(XFA_ATTRIBUTE_Name, wsName);
  if (!UserPacketLoader(pNode, pXMLDocumentNode))
    return nullptr;

  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}

CXFA_Node* CXFA_SimpleParser::UserPacketLoader(CXFA_Node* pXFANode,
                                               CFX_XMLNode* pXMLDoc) {
  return pXFANode;
}

CXFA_Node* CXFA_SimpleParser::DataLoader(CXFA_Node* pXFANode,
                                         CFX_XMLNode* pXMLDoc,
                                         bool bDoTransform) {
  ParseDataGroup(pXFANode, pXMLDoc, XFA_XDPPACKET_Datasets);
  return pXFANode;
}

CXFA_Node* CXFA_SimpleParser::NormalLoader(CXFA_Node* pXFANode,
                                           CFX_XMLNode* pXMLDoc,
                                           XFA_XDPPACKET ePacketID,
                                           bool bUseAttribute) {
  bool bOneOfPropertyFound = false;
  for (CFX_XMLNode* pXMLChild = pXMLDoc->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    switch (pXMLChild->GetType()) {
      case FX_XMLNODE_Element: {
        CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLChild);
        CFX_WideString wsTagName = pXMLElement->GetLocalTagName();
        XFA_Element eType = XFA_GetElementTypeForName(wsTagName.AsStringC());
        if (eType == XFA_Element::Unknown)
          continue;

        const XFA_PROPERTY* pPropertyInfo = XFA_GetPropertyOfElement(
            pXFANode->GetElementType(), eType, ePacketID);
        if (pPropertyInfo &&
            ((pPropertyInfo->uFlags &
              (XFA_PROPERTYFLAG_OneOf | XFA_PROPERTYFLAG_DefaultOneOf)) != 0)) {
          if (bOneOfPropertyFound)
            break;

          bOneOfPropertyFound = true;
        }
        CXFA_Node* pXFAChild = m_pFactory->CreateNode(ePacketID, eType);
        if (!pXFAChild)
          return nullptr;
        if (ePacketID == XFA_XDPPACKET_Config)
          pXFAChild->SetAttribute(XFA_ATTRIBUTE_Name, wsTagName.AsStringC());

        bool IsNeedValue = true;
        for (auto it : pXMLElement->GetAttributes()) {
          CFX_WideString wsAttrName;
          GetAttributeLocalName(it.first.AsStringC(), wsAttrName);
          if (wsAttrName == L"nil" && it.second == L"true")
            IsNeedValue = false;

          const XFA_ATTRIBUTEINFO* lpAttrInfo =
              XFA_GetAttributeByName(wsAttrName.AsStringC());
          if (!lpAttrInfo)
            continue;

          if (!bUseAttribute && lpAttrInfo->eName != XFA_ATTRIBUTE_Name &&
              lpAttrInfo->eName != XFA_ATTRIBUTE_Save) {
            continue;
          }
          pXFAChild->SetAttribute(lpAttrInfo->eName, it.second.AsStringC());
        }
        pXFANode->InsertChild(pXFAChild);
        if (eType == XFA_Element::Validate || eType == XFA_Element::Locale) {
          if (ePacketID == XFA_XDPPACKET_Config)
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
      case FX_XMLNODE_Instruction:
        ParseInstruction(pXFANode, static_cast<CFX_XMLInstruction*>(pXMLChild),
                         ePacketID);
        break;
      default:
        break;
    }
  }
  return pXFANode;
}

void CXFA_SimpleParser::ParseContentNode(CXFA_Node* pXFANode,
                                         CFX_XMLNode* pXMLNode,
                                         XFA_XDPPACKET ePacketID) {
  XFA_Element element = XFA_Element::Sharptext;
  if (pXFANode->GetElementType() == XFA_Element::ExData) {
    CFX_WideStringC wsContentType =
        pXFANode->GetCData(XFA_ATTRIBUTE_ContentType);
    if (wsContentType == L"text/html")
      element = XFA_Element::SharpxHTML;
    else if (wsContentType == L"text/xml")
      element = XFA_Element::Sharpxml;
  }
  if (element == XFA_Element::SharpxHTML)
    pXFANode->SetXMLMappingNode(pXMLNode);

  CFX_WideString wsValue;
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    FX_XMLNODETYPE eNodeType = pXMLChild->GetType();
    if (eNodeType == FX_XMLNODE_Instruction)
      continue;

    if (element == XFA_Element::SharpxHTML) {
      if (eNodeType != FX_XMLNODE_Element)
        break;

      if (XFA_RecognizeRichText(static_cast<CFX_XMLElement*>(pXMLChild)))
        XFA_GetPlainTextFromRichText(static_cast<CFX_XMLElement*>(pXMLChild),
                                     wsValue);
    } else if (element == XFA_Element::Sharpxml) {
      if (eNodeType != FX_XMLNODE_Element)
        break;

      ConvertXMLToPlainText(static_cast<CFX_XMLElement*>(pXMLChild), wsValue);
    } else {
      if (eNodeType == FX_XMLNODE_Element)
        break;
      if (eNodeType == FX_XMLNODE_Text || eNodeType == FX_XMLNODE_CharData)
        wsValue = static_cast<CFX_XMLText*>(pXMLChild)->GetText();
    }
    break;
  }
  if (!wsValue.IsEmpty()) {
    if (pXFANode->IsContentNode()) {
      CXFA_Node* pContentRawDataNode =
          m_pFactory->CreateNode(ePacketID, element);
      ASSERT(pContentRawDataNode);
      pContentRawDataNode->SetCData(XFA_ATTRIBUTE_Value, wsValue);
      pXFANode->InsertChild(pContentRawDataNode);
    } else {
      pXFANode->SetCData(XFA_ATTRIBUTE_Value, wsValue);
    }
  }
}

void CXFA_SimpleParser::ParseDataGroup(CXFA_Node* pXFANode,
                                       CFX_XMLNode* pXMLNode,
                                       XFA_XDPPACKET ePacketID) {
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    switch (pXMLChild->GetType()) {
      case FX_XMLNODE_Element: {
        CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLChild);
        {
          CFX_WideString wsNamespaceURI =
              GetElementTagNamespaceURI(pXMLElement);
          if (wsNamespaceURI == L"http://www.xfa.com/schema/xfa-package/" ||
              wsNamespaceURI == L"http://www.xfa.org/schema/xfa-package/" ||
              wsNamespaceURI == L"http://www.w3.org/2001/XMLSchema-instance") {
            continue;
          }
        }

        XFA_Element eNodeType = XFA_Element::DataModel;
        if (eNodeType == XFA_Element::DataModel) {
          CFX_WideString wsDataNodeAttr;
          if (FindAttributeWithNS(pXMLElement, L"dataNode",
                                  L"http://www.xfa.org/schema/xfa-data/1.0/",
                                  wsDataNodeAttr)) {
            if (wsDataNodeAttr == L"dataGroup")
              eNodeType = XFA_Element::DataGroup;
            else if (wsDataNodeAttr == L"dataValue")
              eNodeType = XFA_Element::DataValue;
          }
        }
        CFX_WideString wsContentType;
        if (eNodeType == XFA_Element::DataModel) {
          if (FindAttributeWithNS(pXMLElement, L"contentType",
                                  L"http://www.xfa.org/schema/xfa-data/1.0/",
                                  wsContentType)) {
            if (!wsContentType.IsEmpty())
              eNodeType = XFA_Element::DataValue;
          }
        }
        if (eNodeType == XFA_Element::DataModel) {
          for (CFX_XMLNode* pXMLDataChild =
                   pXMLElement->GetNodeItem(CFX_XMLNode::FirstChild);
               pXMLDataChild; pXMLDataChild = pXMLDataChild->GetNodeItem(
                                  CFX_XMLNode::NextSibling)) {
            if (pXMLDataChild->GetType() == FX_XMLNODE_Element) {
              if (!XFA_RecognizeRichText(
                      static_cast<CFX_XMLElement*>(pXMLDataChild))) {
                eNodeType = XFA_Element::DataGroup;
                break;
              }
            }
          }
        }
        if (eNodeType == XFA_Element::DataModel)
          eNodeType = XFA_Element::DataValue;

        CXFA_Node* pXFAChild =
            m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, eNodeType);
        if (!pXFAChild)
          return;

        pXFAChild->SetCData(XFA_ATTRIBUTE_Name, pXMLElement->GetLocalTagName());
        bool bNeedValue = true;

        for (auto it : pXMLElement->GetAttributes()) {
          CFX_WideString wsName;
          CFX_WideString wsNS;
          if (!ResolveAttribute(pXMLElement, it.first.AsStringC(), wsName,
                                wsNS)) {
            continue;
          }
          if (wsName == L"nil" && it.second == L"true") {
            bNeedValue = false;
            continue;
          }
          if (wsNS == L"http://www.xfa.com/schema/xfa-package/" ||
              wsNS == L"http://www.xfa.org/schema/xfa-package/" ||
              wsNS == L"http://www.w3.org/2001/XMLSchema-instance" ||
              wsNS == L"http://www.xfa.org/schema/xfa-data/1.0/") {
            continue;
          }
          CXFA_Node* pXFAMetaData = m_pFactory->CreateNode(
              XFA_XDPPACKET_Datasets, XFA_Element::DataValue);
          if (!pXFAMetaData)
            return;

          pXFAMetaData->SetCData(XFA_ATTRIBUTE_Name, wsName);
          pXFAMetaData->SetCData(XFA_ATTRIBUTE_QualifiedName, it.first);
          pXFAMetaData->SetCData(XFA_ATTRIBUTE_Value, it.second);
          pXFAMetaData->SetEnum(XFA_ATTRIBUTE_Contains,
                                XFA_ATTRIBUTEENUM_MetaData);
          pXFAChild->InsertChild(pXFAMetaData);
          pXFAMetaData->SetXMLMappingNode(pXMLElement);
          pXFAMetaData->SetFlag(XFA_NodeFlag_Initialized, false);
        }

        if (!bNeedValue) {
          CFX_WideString wsNilName(L"xsi:nil");
          pXMLElement->RemoveAttribute(wsNilName.c_str());
        }
        pXFANode->InsertChild(pXFAChild);
        if (eNodeType == XFA_Element::DataGroup)
          ParseDataGroup(pXFAChild, pXMLElement, ePacketID);
        else if (bNeedValue)
          ParseDataValue(pXFAChild, pXMLChild, XFA_XDPPACKET_Datasets);

        pXFAChild->SetXMLMappingNode(pXMLElement);
        pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
        continue;
      }
      case FX_XMLNODE_CharData:
      case FX_XMLNODE_Text: {
        CFX_XMLText* pXMLText = static_cast<CFX_XMLText*>(pXMLChild);
        CFX_WideString wsText = pXMLText->GetText();
        if (IsStringAllWhitespace(wsText))
          continue;

        CXFA_Node* pXFAChild = m_pFactory->CreateNode(XFA_XDPPACKET_Datasets,
                                                      XFA_Element::DataValue);
        if (!pXFAChild)
          return;

        pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsText);
        pXFANode->InsertChild(pXFAChild);
        pXFAChild->SetXMLMappingNode(pXMLText);
        pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
        continue;
      }
      default:
        continue;
    }
  }
}

void CXFA_SimpleParser::ParseDataValue(CXFA_Node* pXFANode,
                                       CFX_XMLNode* pXMLNode,
                                       XFA_XDPPACKET ePacketID) {
  CFX_WideTextBuf wsValueTextBuf;
  CFX_WideTextBuf wsCurValueTextBuf;
  bool bMarkAsCompound = false;
  CFX_XMLNode* pXMLCurValueNode = nullptr;
  for (CFX_XMLNode* pXMLChild = pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling)) {
    FX_XMLNODETYPE eNodeType = pXMLChild->GetType();
    if (eNodeType == FX_XMLNODE_Instruction)
      continue;

    CFX_WideString wsText;
    if (eNodeType == FX_XMLNODE_Text || eNodeType == FX_XMLNODE_CharData) {
      wsText = static_cast<CFX_XMLText*>(pXMLChild)->GetText();
      if (!pXMLCurValueNode)
        pXMLCurValueNode = pXMLChild;

      wsCurValueTextBuf << wsText;
    } else if (XFA_RecognizeRichText(static_cast<CFX_XMLElement*>(pXMLChild))) {
      XFA_GetPlainTextFromRichText(static_cast<CFX_XMLElement*>(pXMLChild),
                                   wsText);
      if (!pXMLCurValueNode)
        pXMLCurValueNode = pXMLChild;

      wsCurValueTextBuf << wsText;
    } else {
      bMarkAsCompound = true;
      if (pXMLCurValueNode) {
        CFX_WideString wsCurValue = wsCurValueTextBuf.MakeString();
        if (!wsCurValue.IsEmpty()) {
          CXFA_Node* pXFAChild =
              m_pFactory->CreateNode(ePacketID, XFA_Element::DataValue);
          if (!pXFAChild)
            return;

          pXFAChild->SetCData(XFA_ATTRIBUTE_Name, L"");
          pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsCurValue);
          pXFANode->InsertChild(pXFAChild);
          pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
          pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
          wsValueTextBuf << wsCurValue;
          wsCurValueTextBuf.Clear();
        }
        pXMLCurValueNode = nullptr;
      }
      CXFA_Node* pXFAChild =
          m_pFactory->CreateNode(ePacketID, XFA_Element::DataValue);
      if (!pXFAChild)
        return;

      CFX_WideString wsNodeStr =
          static_cast<CFX_XMLElement*>(pXMLChild)->GetLocalTagName();
      pXFAChild->SetCData(XFA_ATTRIBUTE_Name, wsNodeStr);
      ParseDataValue(pXFAChild, pXMLChild, ePacketID);
      pXFANode->InsertChild(pXFAChild);
      pXFAChild->SetXMLMappingNode(pXMLChild);
      pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
      CFX_WideStringC wsCurValue = pXFAChild->GetCData(XFA_ATTRIBUTE_Value);
      wsValueTextBuf << wsCurValue;
    }
  }
  if (pXMLCurValueNode) {
    CFX_WideString wsCurValue = wsCurValueTextBuf.MakeString();
    if (!wsCurValue.IsEmpty()) {
      if (bMarkAsCompound) {
        CXFA_Node* pXFAChild =
            m_pFactory->CreateNode(ePacketID, XFA_Element::DataValue);
        if (!pXFAChild)
          return;

        pXFAChild->SetCData(XFA_ATTRIBUTE_Name, L"");
        pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsCurValue);
        pXFANode->InsertChild(pXFAChild);
        pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
        pXFAChild->SetFlag(XFA_NodeFlag_Initialized, false);
      }
      wsValueTextBuf << wsCurValue;
      wsCurValueTextBuf.Clear();
    }
    pXMLCurValueNode = nullptr;
  }
  CFX_WideString wsNodeValue = wsValueTextBuf.MakeString();
  pXFANode->SetCData(XFA_ATTRIBUTE_Value, wsNodeValue);
}

void CXFA_SimpleParser::ParseInstruction(CXFA_Node* pXFANode,
                                         CFX_XMLInstruction* pXMLInstruction,
                                         XFA_XDPPACKET ePacketID) {
  if (!m_bDocumentParser)
    return;

  CFX_WideString wsTargetName = pXMLInstruction->GetName();
  const std::vector<CFX_WideString>& target_data =
      pXMLInstruction->GetTargetData();
  if (wsTargetName == L"originalXFAVersion") {
    if (target_data.size() > 1 &&
        (pXFANode->GetDocument()->RecognizeXFAVersionNumber(target_data[0]) !=
         XFA_VERSION_UNKNOWN) &&
        target_data[1] == L"v2.7-scripting:1") {
      pXFANode->GetDocument()->SetFlag(XFA_DOCFLAG_Scripting, true);
    }
  } else if (wsTargetName == L"acrobat") {
    if (target_data.size() > 1 && target_data[0] == L"JavaScript" &&
        target_data[1] == L"strictScoping") {
      pXFANode->GetDocument()->SetFlag(XFA_DOCFLAG_StrictScoping, true);
    }
  }
}

void CXFA_SimpleParser::CloseParser() {
  m_pXMLDoc.reset();
  m_pStream.Reset();
}
