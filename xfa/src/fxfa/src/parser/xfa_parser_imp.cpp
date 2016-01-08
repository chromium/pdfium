// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_utils.h"
#include "xfa/src/fxfa/src/common/xfa_object.h"
#include "xfa/src/fxfa/src/common/xfa_document.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/common/xfa_script.h"
#include "xfa/src/fxfa/src/common/xfa_docdata.h"
#include "xfa/src/fxfa/src/common/xfa_doclayout.h"
#include "xfa/src/fxfa/src/common/xfa_localemgr.h"
#include "xfa/src/fxfa/src/common/xfa_fm2jsapi.h"
#include "xfa_basic_imp.h"
#include "xfa_parser_imp.h"
IXFA_Parser* IXFA_Parser::Create(IXFA_ObjFactory* pFactory,
                                 FX_BOOL bDocumentParser) {
  return new CXFA_SimpleParser(pFactory, bDocumentParser);
}
CXFA_SimpleParser::CXFA_SimpleParser(IXFA_ObjFactory* pFactory,
                                     FX_BOOL bDocumentParser)
    : m_pXMLParser(nullptr),
      m_pXMLDoc(nullptr),
      m_pStream(nullptr),
      m_pFileRead(nullptr),
      m_pFactory(pFactory),
      m_pRootNode(nullptr),
      m_ePacketID(XFA_XDPPACKET_UNKNOWN),
      m_bDocumentParser(bDocumentParser) {
}
CXFA_SimpleParser::~CXFA_SimpleParser() {
  CloseParser();
}
void CXFA_SimpleParser::SetFactory(IXFA_ObjFactory* pFactory) {
  m_pFactory = pFactory;
}
static IFDE_XMLNode* XFA_FDEExtension_GetDocumentNode(
    IFDE_XMLDoc* pXMLDoc,
    FX_BOOL bVerifyWellFormness = FALSE) {
  if (!pXMLDoc) {
    return NULL;
  }
  IFDE_XMLNode* pXMLFakeRoot = pXMLDoc->GetRoot();
  for (IFDE_XMLNode* pXMLNode =
           pXMLFakeRoot->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
      if (bVerifyWellFormness) {
        for (IFDE_XMLNode* pNextNode =
                 pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling);
             pNextNode;
             pNextNode = pNextNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
          if (pNextNode->GetType() == FDE_XMLNODE_Element) {
            return FALSE;
          }
        }
      }
      return pXMLNode;
    }
  }
  return NULL;
}
int32_t CXFA_SimpleParser::StartParse(IFX_FileRead* pStream,
                                      XFA_XDPPACKET ePacketID) {
  CloseParser();
  m_pFileRead = pStream;
  m_pStream = IFX_Stream::CreateStream(
      pStream, FX_STREAMACCESS_Read | FX_STREAMACCESS_Text);
  if (m_pStream == NULL) {
    return XFA_PARSESTATUS_StreamErr;
  }
  FX_WORD wCodePage = m_pStream->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    m_pStream->SetCodePage(FX_CODEPAGE_UTF8);
  }
  m_pXMLDoc = IFDE_XMLDoc::Create();
  if (m_pXMLDoc == NULL) {
    return XFA_PARSESTATUS_StatusErr;
  }
  m_pXMLParser = new CXFA_XMLParser(m_pXMLDoc->GetRoot(), m_pStream);
  if (m_pXMLParser == NULL) {
    return XFA_PARSESTATUS_StatusErr;
  }
  if (!m_pXMLDoc->LoadXML(m_pXMLParser)) {
    return XFA_PARSESTATUS_StatusErr;
  }
  m_ePacketID = ePacketID;
  return XFA_PARSESTATUS_Ready;
}
int32_t CXFA_SimpleParser::DoParse(IFX_Pause* pPause) {
  if (m_pXMLDoc == NULL || m_ePacketID == XFA_XDPPACKET_UNKNOWN) {
    return XFA_PARSESTATUS_StatusErr;
  }
  int32_t iRet = m_pXMLDoc->DoLoad(pPause);
  if (iRet < 0) {
    return XFA_PARSESTATUS_SyntaxErr;
  }
  if (iRet < 100) {
    return iRet / 2;
  }
  m_pRootNode = ParseAsXDPPacket(XFA_FDEExtension_GetDocumentNode(m_pXMLDoc),
                                 m_ePacketID);
  m_pXMLDoc->CloseXML();
  if (m_pStream) {
    m_pStream->Release();
    m_pStream = NULL;
  }
  if (!m_pRootNode) {
    return XFA_PARSESTATUS_StatusErr;
  }
  return XFA_PARSESTATUS_Done;
}
int32_t CXFA_SimpleParser::ParseXMLData(const CFX_WideString& wsXML,
                                        IFDE_XMLNode*& pXMLNode,
                                        IFX_Pause* pPause) {
  CloseParser();
  pXMLNode = NULL;
  IFX_Stream* pStream = XFA_CreateWideTextRead(wsXML);
  if (!pStream) {
    return XFA_PARSESTATUS_StreamErr;
  }
  m_pStream = pStream;
  m_pXMLDoc = IFDE_XMLDoc::Create();
  if (m_pXMLDoc == NULL) {
    return XFA_PARSESTATUS_StatusErr;
  }
  CXFA_XMLParser* pParser = new CXFA_XMLParser(m_pXMLDoc->GetRoot(), m_pStream);
  if (pParser == NULL) {
    return XFA_PARSESTATUS_StatusErr;
  }
#ifdef _XFA_VERIFY_Checksum_
  pParser->m_dwCheckStatus = 0x03;
#endif
  if (!m_pXMLDoc->LoadXML(pParser)) {
    return XFA_PARSESTATUS_StatusErr;
  }
  int32_t iRet = m_pXMLDoc->DoLoad(pPause);
  if (iRet < 0 || iRet >= 100) {
    m_pXMLDoc->CloseXML();
  }
  if (iRet < 0) {
    return XFA_PARSESTATUS_SyntaxErr;
  }
  if (iRet < 100) {
    return iRet / 2;
  }
  if (m_pStream) {
    m_pStream->Release();
    m_pStream = NULL;
  }
  pXMLNode = XFA_FDEExtension_GetDocumentNode(m_pXMLDoc);
  return XFA_PARSESTATUS_Done;
}
void CXFA_SimpleParser::ConstructXFANode(CXFA_Node* pXFANode,
                                         IFDE_XMLNode* pXMLNode) {
  XFA_XDPPACKET ePacketID = (XFA_XDPPACKET)pXFANode->GetPacketID();
  if (ePacketID == XFA_XDPPACKET_Datasets) {
    if (pXFANode->GetClassID() == XFA_ELEMENT_DataValue) {
      for (IFDE_XMLNode* pXMLChild =
               pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
           pXMLChild;
           pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
        FDE_XMLNODETYPE eNodeType = pXMLChild->GetType();
        if (eNodeType == FDE_XMLNODE_Instruction) {
          continue;
        }
        if (eNodeType == FDE_XMLNODE_Element) {
          CXFA_Node* pXFAChild = m_pFactory->CreateNode(XFA_XDPPACKET_Datasets,
                                                        XFA_ELEMENT_DataValue);
          if (pXFAChild == NULL) {
            return;
          }
          CFX_WideString wsNodeStr;
          ((IFDE_XMLElement*)pXMLChild)->GetLocalTagName(wsNodeStr);
          pXFAChild->SetCData(XFA_ATTRIBUTE_Name, wsNodeStr);
          CFX_WideString wsChildValue;
#ifdef XFA_PARSE_HAS_LINEIDENTIFIER
          XFA_GetPlainTextFromRichText((IFDE_XMLElement*)pXMLChild,
                                       wsChildValue);
#else
          XFA_ConvertRichTextToPlainText((IFDE_XMLElement*)pXMLChild,
                                         wsChildValue);
#endif
          if (!wsChildValue.IsEmpty()) {
            pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsChildValue);
          }
          pXFANode->InsertChild(pXFAChild);
          pXFAChild->SetXMLMappingNode(pXMLChild);
          pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
          break;
        }
      }
      m_pRootNode = pXFANode;
    } else {
      m_pRootNode = DataLoader(pXFANode, pXMLNode, TRUE);
    }
  } else {
    if (pXFANode->GetObjectType() == XFA_OBJECTTYPE_ContentNode) {
      ParseContentNode(pXFANode, pXMLNode, ePacketID);
      m_pRootNode = pXFANode;
    } else {
      m_pRootNode = NormalLoader(pXFANode, pXMLNode, ePacketID);
    }
  }
}
FX_BOOL XFA_FDEExtension_ResolveNamespaceQualifier(
    IFDE_XMLElement* pNode,
    const CFX_WideStringC& wsQualifier,
    CFX_WideString& wsNamespaceURI) {
  if (!pNode) {
    return FALSE;
  }
  IFDE_XMLNode* pFakeRoot = pNode->GetNodeItem(IFDE_XMLNode::Root);
  CFX_WideString wsNSAttribute;
  FX_BOOL bRet = FALSE;
  if (wsQualifier.IsEmpty()) {
    wsNSAttribute = FX_WSTRC(L"xmlns");
    bRet = TRUE;
  } else {
    wsNSAttribute = FX_WSTRC(L"xmlns:") + wsQualifier;
  }
  for (; pNode != pFakeRoot;
       pNode = (IFDE_XMLElement*)pNode->GetNodeItem(IFDE_XMLNode::Parent)) {
    if (pNode->GetType() != FDE_XMLNODE_Element) {
      continue;
    }
    if (pNode->HasAttribute(wsNSAttribute)) {
      pNode->GetString(wsNSAttribute, wsNamespaceURI);
      return TRUE;
    }
  }
  wsNamespaceURI.Empty();
  return bRet;
}
static inline void XFA_FDEExtension_GetElementTagNamespaceURI(
    IFDE_XMLElement* pElement,
    CFX_WideString& wsNamespaceURI) {
  CFX_WideString wsNodeStr;
  pElement->GetNamespacePrefix(wsNodeStr);
  if (!XFA_FDEExtension_ResolveNamespaceQualifier(pElement, wsNodeStr,
                                                  wsNamespaceURI)) {
    wsNamespaceURI.Empty();
  }
}
static FX_BOOL XFA_FDEExtension_MatchNodeName(
    IFDE_XMLNode* pNode,
    const CFX_WideStringC& wsLocalTagName,
    const CFX_WideStringC& wsNamespaceURIPrefix,
    FX_DWORD eMatchFlags = XFA_XDPPACKET_FLAGS_NOMATCH) {
  if (!pNode || pNode->GetType() != FDE_XMLNODE_Element) {
    return FALSE;
  }
  IFDE_XMLElement* pElement = reinterpret_cast<IFDE_XMLElement*>(pNode);
  CFX_WideString wsNodeStr;
  pElement->GetLocalTagName(wsNodeStr);
  if (wsNodeStr != wsLocalTagName) {
    return FALSE;
  }
  XFA_FDEExtension_GetElementTagNamespaceURI(pElement, wsNodeStr);
  if (eMatchFlags & XFA_XDPPACKET_FLAGS_NOMATCH) {
    return TRUE;
  }
  if (eMatchFlags & XFA_XDPPACKET_FLAGS_PREFIXMATCH) {
    return wsNodeStr.Left(wsNamespaceURIPrefix.GetLength()) ==
           wsNamespaceURIPrefix;
  }
  return wsNodeStr == wsNamespaceURIPrefix;
}
static FX_BOOL XFA_FDEExtension_GetAttributeLocalName(
    const CFX_WideStringC& wsAttributeName,
    CFX_WideString& wsLocalAttrName) {
  CFX_WideString wsAttrName(wsAttributeName);
  FX_STRSIZE iFind = wsAttrName.Find(L':', 0);
  if (iFind < 0) {
    wsLocalAttrName = wsAttrName;
    return FALSE;
  } else {
    wsLocalAttrName = wsAttrName.Right(wsAttrName.GetLength() - iFind - 1);
    return TRUE;
  }
}
static FX_BOOL XFA_FDEExtension_ResolveAttribute(
    IFDE_XMLElement* pElement,
    const CFX_WideStringC& wsAttributeName,
    CFX_WideString& wsLocalAttrName,
    CFX_WideString& wsNamespaceURI) {
  CFX_WideString wsAttrName(wsAttributeName);
  CFX_WideString wsNSPrefix;
  if (XFA_FDEExtension_GetAttributeLocalName(wsAttributeName,
                                             wsLocalAttrName)) {
    wsNSPrefix = wsAttrName.Left(wsAttributeName.GetLength() -
                                 wsLocalAttrName.GetLength() - 1);
  }
  if (wsLocalAttrName == FX_WSTRC(L"xmlns") ||
      wsNSPrefix == FX_WSTRC(L"xmlns") || wsNSPrefix == FX_WSTRC(L"xml")) {
    return FALSE;
  }
  if (!XFA_FDEExtension_ResolveNamespaceQualifier(pElement, wsNSPrefix,
                                                  wsNamespaceURI)) {
    wsNamespaceURI.Empty();
    return FALSE;
  }
  return TRUE;
}
static FX_BOOL XFA_FDEExtension_FindAttributeWithNS(
    IFDE_XMLElement* pElement,
    const CFX_WideStringC& wsLocalAttributeName,
    const CFX_WideStringC& wsNamespaceURIPrefix,
    CFX_WideString& wsValue,
    FX_BOOL bMatchNSAsPrefix = FALSE) {
  if (!pElement) {
    return FALSE;
  }
  CFX_WideString wsAttrName;
  CFX_WideString wsAttrValue;
  CFX_WideString wsAttrNS;
  for (int32_t iAttrCount = pElement->CountAttributes(), i = 0; i < iAttrCount;
       i++) {
    pElement->GetAttribute(i, wsAttrName, wsAttrValue);
    FX_STRSIZE iFind = wsAttrName.Find(L':', 0);
    CFX_WideString wsNSPrefix;
    if (iFind < 0) {
      if (wsLocalAttributeName != wsAttrName) {
        continue;
      }
    } else {
      if (wsLocalAttributeName !=
          wsAttrName.Right(wsAttrName.GetLength() - iFind - 1)) {
        continue;
      }
      wsNSPrefix = wsAttrName.Left(iFind);
    }
    if (!XFA_FDEExtension_ResolveNamespaceQualifier(pElement, wsNSPrefix,
                                                    wsAttrNS)) {
      continue;
    }
    if (bMatchNSAsPrefix) {
      if (wsAttrNS.Left(wsNamespaceURIPrefix.GetLength()) !=
          wsNamespaceURIPrefix) {
        continue;
      }
    } else {
      if (wsAttrNS != wsNamespaceURIPrefix) {
        continue;
      }
    }
    wsValue = wsAttrValue;
    return TRUE;
  }
  return FALSE;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket(IFDE_XMLNode* pXMLDocumentNode,
                                               XFA_XDPPACKET ePacketID) {
  switch (ePacketID) {
    case XFA_XDPPACKET_UNKNOWN:
      return NULL;
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
  return NULL;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_XDP(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (!XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_XDP)->pName,
          XFA_GetPacketByIndex(XFA_PACKET_XDP)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_XDP)->eFlags)) {
    return nullptr;
  }
  CXFA_Node* pXFARootNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_XDP, XFA_ELEMENT_Xfa);
  if (!pXFARootNode) {
    return nullptr;
  }
  m_pRootNode = pXFARootNode;
  pXFARootNode->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L"xfa"));
  {
    IFDE_XMLElement* pElement = (IFDE_XMLElement*)pXMLDocumentNode;
    int32_t iAttributeCount = pElement->CountAttributes();
    for (int32_t i = 0; i < iAttributeCount; i++) {
      CFX_WideString wsAttriName, wsAttriValue;
      pElement->GetAttribute(i, wsAttriName, wsAttriValue);
      if (wsAttriName == FX_WSTRC(L"uuid")) {
        pXFARootNode->SetCData(XFA_ATTRIBUTE_Uuid, wsAttriValue);
      } else if (wsAttriName == FX_WSTRC(L"timeStamp")) {
        pXFARootNode->SetCData(XFA_ATTRIBUTE_TimeStamp, wsAttriValue);
      }
    }
  }
  IFDE_XMLNode* pXMLConfigDOMRoot = nullptr;
  CXFA_Node* pXFAConfigDOMRoot = nullptr;
  {
    for (IFDE_XMLNode* pChildItem =
             pXMLDocumentNode->GetNodeItem(IFDE_XMLNode::FirstChild);
         pChildItem;
         pChildItem = pChildItem->GetNodeItem(IFDE_XMLNode::NextSibling)) {
      XFA_LPCPACKETINFO pPacketInfo = XFA_GetPacketByIndex(XFA_PACKET_Config);
      if (!XFA_FDEExtension_MatchNodeName(pChildItem, pPacketInfo->pName,
                                          pPacketInfo->pURI,
                                          pPacketInfo->eFlags)) {
        continue;
      }
      if (pXFARootNode->GetFirstChildByName(pPacketInfo->uHash)) {
        return nullptr;
      }
      pXMLConfigDOMRoot = pChildItem;
      pXFAConfigDOMRoot =
          ParseAsXDPPacket_Config(pXMLConfigDOMRoot, XFA_XDPPACKET_Config);
      pXFARootNode->InsertChild(pXFAConfigDOMRoot, NULL);
    }
  }
  IFDE_XMLNode* pXMLDatasetsDOMRoot = nullptr;
  IFDE_XMLNode* pXMLFormDOMRoot = nullptr;
  IFDE_XMLNode* pXMLTemplateDOMRoot = nullptr;
  {
    for (IFDE_XMLNode* pChildItem =
             pXMLDocumentNode->GetNodeItem(IFDE_XMLNode::FirstChild);
         pChildItem;
         pChildItem = pChildItem->GetNodeItem(IFDE_XMLNode::NextSibling)) {
      if (!pChildItem || pChildItem->GetType() != FDE_XMLNODE_Element) {
        continue;
      }
      if (pChildItem == pXMLConfigDOMRoot) {
        continue;
      }
      IFDE_XMLElement* pElement =
          reinterpret_cast<IFDE_XMLElement*>(pChildItem);
      CFX_WideString wsPacketName;
      pElement->GetLocalTagName(wsPacketName);
      XFA_LPCPACKETINFO pPacketInfo = XFA_GetPacketByName(wsPacketName);
      if (pPacketInfo && pPacketInfo->pURI) {
        if (!XFA_FDEExtension_MatchNodeName(pElement, pPacketInfo->pName,
                                            pPacketInfo->pURI,
                                            pPacketInfo->eFlags)) {
          pPacketInfo = nullptr;
        }
      }
      XFA_XDPPACKET ePacket =
          pPacketInfo ? pPacketInfo->eName : XFA_XDPPACKET_USER;
      if (ePacket == XFA_XDPPACKET_XDP) {
        continue;
      }
      if (ePacket == XFA_XDPPACKET_Datasets) {
        if (pXMLDatasetsDOMRoot) {
          return nullptr;
        }
        pXMLDatasetsDOMRoot = pElement;
      } else if (ePacket == XFA_XDPPACKET_Form) {
        if (pXMLFormDOMRoot) {
          return nullptr;
        }
        pXMLFormDOMRoot = pElement;
      } else if (ePacket == XFA_XDPPACKET_Template) {
        if (pXMLTemplateDOMRoot) {
          // Found a duplicate template packet.
          return nullptr;
        }
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
  }
  if (!pXMLTemplateDOMRoot) {
    // No template is found.
    return nullptr;
  }
  if (pXMLDatasetsDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLDatasetsDOMRoot, XFA_XDPPACKET_Datasets);
    if (pPacketNode) {
      pXFARootNode->InsertChild(pPacketNode);
    }
  }
  if (pXMLFormDOMRoot) {
    CXFA_Node* pPacketNode =
        ParseAsXDPPacket(pXMLFormDOMRoot, XFA_XDPPACKET_Form);
    if (pPacketNode) {
      pXFARootNode->InsertChild(pPacketNode);
    }
  }
  pXFARootNode->SetXMLMappingNode(pXMLDocumentNode);
  return pXFARootNode;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Config(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (!XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_Config)->pName,
          XFA_GetPacketByIndex(XFA_PACKET_Config)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_Config)->eFlags)) {
    return NULL;
  }
  CXFA_Node* pNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_Config, XFA_ELEMENT_Config);
  if (!pNode) {
    return NULL;
  }
  pNode->SetCData(XFA_ATTRIBUTE_Name,
                  XFA_GetPacketByIndex(XFA_PACKET_Config)->pName);
  if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID)) {
    return NULL;
  }
  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_TemplateForm(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode = NULL;
  if (ePacketID == XFA_XDPPACKET_Template) {
    if (XFA_FDEExtension_MatchNodeName(
            pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_Template)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_Template)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_Template)->eFlags)) {
      pNode =
          m_pFactory->CreateNode(XFA_XDPPACKET_Template, XFA_ELEMENT_Template);
      if (!pNode) {
        return NULL;
      }
      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_Template)->pName);
      if (m_bDocumentParser) {
        CFX_WideString wsNamespaceURI;
        IFDE_XMLElement* pXMLDocumentElement =
            (IFDE_XMLElement*)pXMLDocumentNode;
        pXMLDocumentElement->GetNamespaceURI(wsNamespaceURI);
        if (wsNamespaceURI.IsEmpty()) {
          pXMLDocumentElement->GetString(L"xmlns:xfa", wsNamespaceURI);
        }
        pNode->GetDocument()->RecognizeXFAVersionNumber(wsNamespaceURI);
      }
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID)) {
        return NULL;
      }
    }
  } else if (ePacketID == XFA_XDPPACKET_Form) {
    if (XFA_FDEExtension_MatchNodeName(
            pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_Form)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_Form)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_Form)->eFlags)) {
      IFDE_XMLElement* pXMLDocumentElement = (IFDE_XMLElement*)pXMLDocumentNode;
      CFX_WideString wsChecksum;
      pXMLDocumentElement->GetString(L"checksum", wsChecksum);
#ifdef _XFA_VERIFY_Checksum_
      if (wsChecksum.GetLength() != 28 ||
          m_pXMLParser->m_dwCheckStatus != 0x03) {
        return NULL;
      }
      IXFA_ChecksumContext* pChecksum = XFA_Checksum_Create();
      pChecksum->StartChecksum();
      pChecksum->UpdateChecksum(m_pFileRead, m_pXMLParser->m_nStart[0],
                                m_pXMLParser->m_nSize[0]);
      pChecksum->UpdateChecksum(m_pFileRead, m_pXMLParser->m_nStart[1],
                                m_pXMLParser->m_nSize[1]);
      pChecksum->FinishChecksum();
      CFX_ByteString bsCheck;
      pChecksum->GetChecksum(bsCheck);
      pChecksum->Release();
      if (bsCheck != wsChecksum.UTF8Encode()) {
        return NULL;
      }
#endif
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_Form, XFA_ELEMENT_Form);
      if (!pNode) {
        return NULL;
      }
      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_Form)->pName);
      pNode->SetAttribute(XFA_ATTRIBUTE_Checksum, wsChecksum);
      CXFA_Node* pTemplateRoot =
          m_pRootNode->GetFirstChildByClass(XFA_ELEMENT_Template);
      CXFA_Node* pTemplateChosen =
          pTemplateRoot
              ? pTemplateRoot->GetFirstChildByClass(XFA_ELEMENT_Subform)
              : NULL;
      FX_BOOL bUseAttribute = TRUE;
      if (pTemplateChosen &&
          pTemplateChosen->GetEnum(XFA_ATTRIBUTE_RestoreState) !=
              XFA_ATTRIBUTEENUM_Auto) {
        bUseAttribute = FALSE;
      }
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID, bUseAttribute)) {
        return NULL;
      }
    }
  }
  if (pNode) {
    pNode->SetXMLMappingNode(pXMLDocumentNode);
  }
  return pNode;
}
static IFDE_XMLNode* XFA_GetDataSetsFromXDP(IFDE_XMLNode* pXMLDocumentNode) {
  if (XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName,
          XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
    return pXMLDocumentNode;
  }
  if (!XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_XDP)->pName,
          XFA_GetPacketByIndex(XFA_PACKET_XDP)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_XDP)->eFlags)) {
    return NULL;
  }
  for (IFDE_XMLNode* pDatasetsNode =
           pXMLDocumentNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pDatasetsNode;
       pDatasetsNode = pDatasetsNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    if (!XFA_FDEExtension_MatchNodeName(
            pDatasetsNode, XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
      continue;
    }
    return pDatasetsNode;
  }
  return NULL;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Data(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  IFDE_XMLNode* pDatasetsXMLNode = XFA_GetDataSetsFromXDP(pXMLDocumentNode);
  if (pDatasetsXMLNode) {
    CXFA_Node* pNode =
        m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataModel);
    if (!pNode) {
      return NULL;
    }
    pNode->SetCData(XFA_ATTRIBUTE_Name,
                    XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pName);
    if (!DataLoader(pNode, pDatasetsXMLNode, FALSE)) {
      return NULL;
    }
    pNode->SetXMLMappingNode(pDatasetsXMLNode);
    return pNode;
  }
  IFDE_XMLNode* pDataXMLNode = NULL;
  if (XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, FX_WSTRC(L"data"),
          XFA_GetPacketByIndex(XFA_PACKET_Datasets)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_Datasets)->eFlags)) {
    ((IFDE_XMLElement*)pXMLDocumentNode)->RemoveAttribute(L"xmlns:xfa");
    pDataXMLNode = pXMLDocumentNode;
  } else {
    IFDE_XMLElement* pDataElement =
        IFDE_XMLElement::Create(FX_WSTRC(L"xfa:data"));
    IFDE_XMLNode* pParentXMLNode =
        pXMLDocumentNode->GetNodeItem(IFDE_XMLNode::Parent);
    if (pParentXMLNode) {
      pParentXMLNode->RemoveChildNode(pXMLDocumentNode);
    }
    FXSYS_assert(pXMLDocumentNode->GetType() == FDE_XMLNODE_Element);
    if (pXMLDocumentNode->GetType() == FDE_XMLNODE_Element) {
      ((IFDE_XMLElement*)pXMLDocumentNode)->RemoveAttribute(L"xmlns:xfa");
    }
    pDataElement->InsertChildNode(pXMLDocumentNode);
    pDataXMLNode = pDataElement;
  }
  if (pDataXMLNode) {
    CXFA_Node* pNode =
        m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataGroup);
    if (!pNode) {
      if (pDataXMLNode != pXMLDocumentNode) {
        pDataXMLNode->Release();
      }
      return NULL;
    }
    CFX_WideString wsLocalName;
    ((IFDE_XMLElement*)pDataXMLNode)->GetLocalTagName(wsLocalName);
    pNode->SetCData(XFA_ATTRIBUTE_Name, wsLocalName);
    if (!DataLoader(pNode, pDataXMLNode, TRUE)) {
      return NULL;
    }
    pNode->SetXMLMappingNode(pDataXMLNode);
    if (pDataXMLNode != pXMLDocumentNode) {
      pNode->SetFlag(XFA_NODEFLAG_OwnXMLNode, TRUE, FALSE);
    }
    return pNode;
  }
  return NULL;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_LocaleConnectionSourceSet(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode = NULL;
  if (ePacketID == XFA_XDPPACKET_LocaleSet) {
    if (XFA_FDEExtension_MatchNodeName(
            pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_LocaleSet,
                                     XFA_ELEMENT_LocaleSet);
      if (!pNode) {
        return NULL;
      }
      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_LocaleSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID)) {
        return NULL;
      }
    }
  } else if (ePacketID == XFA_XDPPACKET_ConnectionSet) {
    if (XFA_FDEExtension_MatchNodeName(
            pXMLDocumentNode,
            XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_ConnectionSet,
                                     XFA_ELEMENT_ConnectionSet);
      if (!pNode) {
        return NULL;
      }
      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_ConnectionSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID)) {
        return NULL;
      }
    }
  } else if (ePacketID == XFA_XDPPACKET_SourceSet) {
    if (XFA_FDEExtension_MatchNodeName(
            pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pName,
            XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pURI,
            XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->eFlags)) {
      pNode = m_pFactory->CreateNode(XFA_XDPPACKET_SourceSet,
                                     XFA_ELEMENT_SourceSet);
      if (!pNode) {
        return NULL;
      }
      pNode->SetCData(XFA_ATTRIBUTE_Name,
                      XFA_GetPacketByIndex(XFA_PACKET_SourceSet)->pName);
      if (!NormalLoader(pNode, pXMLDocumentNode, ePacketID)) {
        return NULL;
      }
    }
  }
  if (pNode) {
    pNode->SetXMLMappingNode(pXMLDocumentNode);
  }
  return pNode;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_Xdc(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  if (XFA_FDEExtension_MatchNodeName(
          pXMLDocumentNode, XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pName,
          XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pURI,
          XFA_GetPacketByIndex(XFA_PACKET_Xdc)->eFlags)) {
    CXFA_Node* pNode =
        m_pFactory->CreateNode(XFA_XDPPACKET_Xdc, XFA_ELEMENT_Xdc);
    if (!pNode) {
      return NULL;
    }
    pNode->SetCData(XFA_ATTRIBUTE_Name,
                    XFA_GetPacketByIndex(XFA_PACKET_Xdc)->pName);
    pNode->SetXMLMappingNode(pXMLDocumentNode);
    return pNode;
  }
  return NULL;
}
CXFA_Node* CXFA_SimpleParser::ParseAsXDPPacket_User(
    IFDE_XMLNode* pXMLDocumentNode,
    XFA_XDPPACKET ePacketID) {
  CXFA_Node* pNode =
      m_pFactory->CreateNode(XFA_XDPPACKET_XDP, XFA_ELEMENT_Packet);
  if (!pNode) {
    return NULL;
  }
  CFX_WideString wsName;
  ((IFDE_XMLElement*)pXMLDocumentNode)->GetLocalTagName(wsName);
  pNode->SetCData(XFA_ATTRIBUTE_Name, wsName);
  if (!UserPacketLoader(pNode, pXMLDocumentNode)) {
    return NULL;
  }
  pNode->SetXMLMappingNode(pXMLDocumentNode);
  return pNode;
}
CXFA_Node* CXFA_SimpleParser::UserPacketLoader(CXFA_Node* pXFANode,
                                               IFDE_XMLNode* pXMLDoc) {
  return pXFANode;
}
static FX_BOOL XFA_FDEExtension_IsStringAllWhitespace(CFX_WideString wsText) {
  wsText.TrimRight(L"\x20\x9\xD\xA");
  return wsText.IsEmpty();
}
CXFA_Node* CXFA_SimpleParser::DataLoader(CXFA_Node* pXFANode,
                                         IFDE_XMLNode* pXMLDoc,
                                         FX_BOOL bDoTransform) {
  ParseDataGroup(pXFANode, pXMLDoc, XFA_XDPPACKET_Datasets);
  return pXFANode;
}
CXFA_Node* CXFA_SimpleParser::NormalLoader(CXFA_Node* pXFANode,
                                           IFDE_XMLNode* pXMLDoc,
                                           XFA_XDPPACKET ePacketID,
                                           FX_BOOL bUseAttribute) {
  FX_BOOL bOneOfPropertyFound = FALSE;
  for (IFDE_XMLNode* pXMLChild = pXMLDoc->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    switch (pXMLChild->GetType()) {
      case FDE_XMLNODE_Element: {
        IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLChild;
        CFX_WideString wsTagName;
        pXMLElement->GetLocalTagName(wsTagName);
        XFA_LPCELEMENTINFO pElemInfo = XFA_GetElementByName(wsTagName);
        if (!pElemInfo) {
          continue;
        }
        XFA_LPCPROPERTY pPropertyInfo = XFA_GetPropertyOfElement(
            pXFANode->GetClassID(), pElemInfo->eName, ePacketID);
        if (pPropertyInfo &&
            ((pPropertyInfo->uFlags &
              (XFA_PROPERTYFLAG_OneOf | XFA_PROPERTYFLAG_DefaultOneOf)) != 0)) {
          if (bOneOfPropertyFound) {
            break;
          }
          bOneOfPropertyFound = TRUE;
        }
        CXFA_Node* pXFAChild =
            m_pFactory->CreateNode(ePacketID, pElemInfo->eName);
        if (pXFAChild == NULL) {
          return NULL;
        }
        if (ePacketID == XFA_XDPPACKET_Config) {
          pXFAChild->SetAttribute(XFA_ATTRIBUTE_Name, wsTagName);
        }
        FX_BOOL IsNeedValue = TRUE;
        for (int32_t i = 0, count = pXMLElement->CountAttributes(); i < count;
             i++) {
          CFX_WideString wsAttrQualifiedName;
          CFX_WideString wsAttrName;
          CFX_WideString wsAttrValue;
          pXMLElement->GetAttribute(i, wsAttrQualifiedName, wsAttrValue);
          XFA_FDEExtension_GetAttributeLocalName(wsAttrQualifiedName,
                                                 wsAttrName);
          if (wsAttrName == FX_WSTRC(L"nil") &&
              wsAttrValue == FX_WSTRC(L"true")) {
            IsNeedValue = FALSE;
          }
          XFA_LPCATTRIBUTEINFO lpAttrInfo = XFA_GetAttributeByName(wsAttrName);
          if (!lpAttrInfo) {
            continue;
          }
          if (!bUseAttribute && lpAttrInfo->eName != XFA_ATTRIBUTE_Name &&
              lpAttrInfo->eName != XFA_ATTRIBUTE_Save) {
            continue;
          }
          pXFAChild->SetAttribute(lpAttrInfo->eName, wsAttrValue);
        }
        pXFANode->InsertChild(pXFAChild);
        if (pElemInfo->eName == XFA_ELEMENT_Validate ||
            pElemInfo->eName == XFA_ELEMENT_Locale) {
          if (ePacketID == XFA_XDPPACKET_Config) {
            ParseContentNode(pXFAChild, pXMLElement, ePacketID);
          } else {
            NormalLoader(pXFAChild, pXMLElement, ePacketID, bUseAttribute);
          }
          break;
        }
        switch (pXFAChild->GetObjectType()) {
          case XFA_OBJECTTYPE_ContentNode:
          case XFA_OBJECTTYPE_TextNode:
          case XFA_OBJECTTYPE_NodeC:
          case XFA_OBJECTTYPE_NodeV:
            if (IsNeedValue) {
              ParseContentNode(pXFAChild, pXMLElement, ePacketID);
            }
            break;
          default:
            NormalLoader(pXFAChild, pXMLElement, ePacketID, bUseAttribute);
            break;
        }
      } break;
      case FDE_XMLNODE_Instruction:
        ParseInstruction(pXFANode, (IFDE_XMLInstruction*)pXMLChild, ePacketID);
        break;
      default:
        break;
    }
  }
  return pXFANode;
}
FX_BOOL XFA_RecognizeRichText(IFDE_XMLElement* pRichTextXMLNode) {
  if (pRichTextXMLNode) {
    CFX_WideString wsNamespaceURI;
    XFA_FDEExtension_GetElementTagNamespaceURI(pRichTextXMLNode,
                                               wsNamespaceURI);
    if (wsNamespaceURI == FX_WSTRC(L"http://www.w3.org/1999/xhtml")) {
      return TRUE;
    }
  }
  return FALSE;
}
class RichTextNodeVisitor {
 public:
  static inline IFDE_XMLNode* GetFirstChild(IFDE_XMLNode* pNode) {
    return pNode->GetNodeItem(IFDE_XMLNode::FirstChild);
  }
  static inline IFDE_XMLNode* GetNextSibling(IFDE_XMLNode* pNode) {
    return pNode->GetNodeItem(IFDE_XMLNode::NextSibling);
  }
  static inline IFDE_XMLNode* GetParent(IFDE_XMLNode* pNode) {
    return pNode->GetNodeItem(IFDE_XMLNode::Parent);
  }
};
#ifndef XFA_PARSE_HAS_LINEIDENTIFIER
void XFA_ConvertRichTextToPlainText(IFDE_XMLElement* pRichTextXMLNode,
                                    CFX_WideString& wsOutput) {
  CXFA_NodeIteratorTemplate<IFDE_XMLNode, RichTextNodeVisitor> sIterator(
      pRichTextXMLNode);
  CFX_WideTextBuf wsPlainTextBuf;
  for (IFDE_XMLNode* pNode = sIterator.GetCurrent(); pNode;
       pNode = sIterator.MoveToNext()) {
    switch (pNode->GetType()) {
      case FDE_XMLNODE_Text: {
        CFX_WideString wsText;
        ((IFDE_XMLText*)pNode)->GetText(wsText);
        wsPlainTextBuf << wsText;
      } break;
      case FDE_XMLNODE_CharData: {
        CFX_WideString wsText;
        ((IFDE_XMLCharData*)pNode)->GetCharData(wsText);
        wsPlainTextBuf << wsText;
      } break;
      default:
        break;
    }
  }
  wsOutput = wsPlainTextBuf.GetWideString();
}
#endif
void XFA_ConvertXMLToPlainText(IFDE_XMLElement* pRootXMLNode,
                               CFX_WideString& wsOutput) {
  for (IFDE_XMLNode* pXMLChild =
           pRootXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
#ifdef _DEBUG
    FDE_XMLNODETYPE nodeType = pXMLChild->GetType();
#endif
    switch (pXMLChild->GetType()) {
      case FDE_XMLNODE_Element: {
        CFX_WideString wsTextData;
        ((IFDE_XMLElement*)pXMLChild)->GetTextData(wsTextData);
        wsTextData += FX_WSTRC(L"\n");
        wsOutput += wsTextData;
      } break;
      case FDE_XMLNODE_Text: {
        CFX_WideString wsText;
        ((IFDE_XMLText*)pXMLChild)->GetText(wsText);
        if (XFA_FDEExtension_IsStringAllWhitespace(wsText)) {
          continue;
        } else {
          wsOutput = wsText;
        }
      } break;
      case FDE_XMLNODE_CharData: {
        CFX_WideString wsCharData;
        ((IFDE_XMLCharData*)pXMLChild)->GetCharData(wsCharData);
        if (XFA_FDEExtension_IsStringAllWhitespace(wsCharData)) {
          continue;
        } else {
          wsOutput = wsCharData;
        }
      } break;
      default:
        FXSYS_assert(FALSE);
        break;
    }
  }
}
void CXFA_SimpleParser::ParseContentNode(CXFA_Node* pXFANode,
                                         IFDE_XMLNode* pXMLNode,
                                         XFA_XDPPACKET ePacketID) {
  XFA_ELEMENT element = XFA_ELEMENT_Sharptext;
  if (pXFANode->GetClassID() == XFA_ELEMENT_ExData) {
    CFX_WideStringC wsContentType =
        pXFANode->GetCData(XFA_ATTRIBUTE_ContentType);
    if (wsContentType == FX_WSTRC(L"text/html")) {
      element = XFA_ELEMENT_SharpxHTML;
    } else if (wsContentType == FX_WSTRC(L"text/xml")) {
      element = XFA_ELEMENT_Sharpxml;
    }
  }
  if (element == XFA_ELEMENT_SharpxHTML) {
    pXFANode->SetXMLMappingNode(pXMLNode);
  }
  CFX_WideString wsValue;
  for (IFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    FDE_XMLNODETYPE eNodeType = pXMLChild->GetType();
    if (eNodeType == FDE_XMLNODE_Instruction) {
      continue;
    }
    if (element == XFA_ELEMENT_SharpxHTML) {
      if (eNodeType != FDE_XMLNODE_Element) {
        break;
      }
      if (XFA_RecognizeRichText((IFDE_XMLElement*)pXMLChild)) {
#ifdef XFA_PARSE_HAS_LINEIDENTIFIER
        XFA_GetPlainTextFromRichText((IFDE_XMLElement*)pXMLChild, wsValue);
#else
        XFA_ConvertRichTextToPlainText((IFDE_XMLElement*)pXMLChild, wsValue);
#endif
      }
    } else if (element == XFA_ELEMENT_Sharpxml) {
      if (eNodeType != FDE_XMLNODE_Element) {
        break;
      }
      XFA_ConvertXMLToPlainText((IFDE_XMLElement*)pXMLChild, wsValue);
    } else {
      if (eNodeType == FDE_XMLNODE_Element) {
        break;
      }
      if (eNodeType == FDE_XMLNODE_Text) {
        ((IFDE_XMLText*)pXMLChild)->GetText(wsValue);
      } else if (eNodeType == FDE_XMLNODE_CharData) {
        ((IFDE_XMLCharData*)pXMLChild)->GetCharData(wsValue);
      }
    }
    break;
  }
  if (!wsValue.IsEmpty()) {
    if (pXFANode->GetObjectType() == XFA_OBJECTTYPE_ContentNode) {
      CXFA_Node* pContentRawDataNode =
          m_pFactory->CreateNode(ePacketID, element);
      FXSYS_assert(pContentRawDataNode);
      pContentRawDataNode->SetCData(XFA_ATTRIBUTE_Value, wsValue);
      pXFANode->InsertChild(pContentRawDataNode);
    } else {
      pXFANode->SetCData(XFA_ATTRIBUTE_Value, wsValue);
    }
  }
}
void CXFA_SimpleParser::ParseDataGroup(CXFA_Node* pXFANode,
                                       IFDE_XMLNode* pXMLNode,
                                       XFA_XDPPACKET ePacketID) {
  for (IFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    switch (pXMLChild->GetType()) {
      case FDE_XMLNODE_Element: {
        IFDE_XMLElement* pXMLElement = (IFDE_XMLElement*)pXMLChild;
        {
          CFX_WideString wsNamespaceURI;
          XFA_FDEExtension_GetElementTagNamespaceURI(pXMLElement,
                                                     wsNamespaceURI);
          if (wsNamespaceURI ==
                  FX_WSTRC(L"http://www.xfa.com/schema/xfa-package/") ||
              wsNamespaceURI ==
                  FX_WSTRC(L"http://www.xfa.org/schema/xfa-package/") ||
              wsNamespaceURI ==
                  FX_WSTRC(L"http://www.w3.org/2001/XMLSchema-instance")) {
            continue;
          }
          if (0) {
            continue;
          }
        }
        XFA_ELEMENT eNodeType = XFA_ELEMENT_DataModel;
        if (eNodeType == XFA_ELEMENT_DataModel) {
          CFX_WideString wsDataNodeAttr;
          if (XFA_FDEExtension_FindAttributeWithNS(
                  pXMLElement, FX_WSTRC(L"dataNode"),
                  FX_WSTRC(L"http://www.xfa.org/schema/xfa-data/1.0/"),
                  wsDataNodeAttr)) {
            if (wsDataNodeAttr == FX_WSTRC(L"dataGroup")) {
              eNodeType = XFA_ELEMENT_DataGroup;
            } else if (wsDataNodeAttr == FX_WSTRC(L"dataValue")) {
              eNodeType = XFA_ELEMENT_DataValue;
            }
          }
        }
        CFX_WideString wsContentType;
        if (eNodeType == XFA_ELEMENT_DataModel) {
          if (XFA_FDEExtension_FindAttributeWithNS(
                  pXMLElement, FX_WSTRC(L"contentType"),
                  FX_WSTRC(L"http://www.xfa.org/schema/xfa-data/1.0/"),
                  wsContentType)) {
            if (!wsContentType.IsEmpty()) {
              eNodeType = XFA_ELEMENT_DataValue;
            }
          }
        }
        if (eNodeType == XFA_ELEMENT_DataModel) {
          for (IFDE_XMLNode* pXMLDataChild =
                   pXMLElement->GetNodeItem(IFDE_XMLNode::FirstChild);
               pXMLDataChild; pXMLDataChild = pXMLDataChild->GetNodeItem(
                                  IFDE_XMLNode::NextSibling)) {
            if (pXMLDataChild->GetType() == FDE_XMLNODE_Element) {
              if (!XFA_RecognizeRichText((IFDE_XMLElement*)pXMLDataChild)) {
                eNodeType = XFA_ELEMENT_DataGroup;
                break;
              }
            }
          }
        }
        if (eNodeType == XFA_ELEMENT_DataModel) {
          eNodeType = XFA_ELEMENT_DataValue;
        }
        CXFA_Node* pXFAChild =
            m_pFactory->CreateNode(XFA_XDPPACKET_Datasets, eNodeType);
        if (pXFAChild == NULL) {
          return;
        }
        CFX_WideString wsNodeName;
        pXMLElement->GetLocalTagName(wsNodeName);
        pXFAChild->SetCData(XFA_ATTRIBUTE_Name, wsNodeName);
        FX_BOOL bNeedValue = TRUE;
        if (1) {
          for (int32_t i = 0, count = pXMLElement->CountAttributes(); i < count;
               i++) {
            CFX_WideString wsAttrQualifiedName;
            CFX_WideString wsAttrValue;
            CFX_WideString wsAttrName;
            CFX_WideString wsAttrNamespaceURI;
            pXMLElement->GetAttribute(i, wsAttrQualifiedName, wsAttrValue);
            if (!XFA_FDEExtension_ResolveAttribute(
                    pXMLElement, wsAttrQualifiedName, wsAttrName,
                    wsAttrNamespaceURI)) {
              continue;
            }
            if (wsAttrName == FX_WSTRC(L"nil") &&
                wsAttrValue == FX_WSTRC(L"true")) {
              bNeedValue = FALSE;
              continue;
            }
            if (wsAttrNamespaceURI ==
                    FX_WSTRC(L"http://www.xfa.com/schema/xfa-package/") ||
                wsAttrNamespaceURI ==
                    FX_WSTRC(L"http://www.xfa.org/schema/xfa-package/") ||
                wsAttrNamespaceURI ==
                    FX_WSTRC(L"http://www.w3.org/2001/XMLSchema-instance") ||
                wsAttrNamespaceURI ==
                    FX_WSTRC(L"http://www.xfa.org/schema/xfa-data/1.0/")) {
              continue;
            }
            if (0) {
              continue;
            }
            CXFA_Node* pXFAMetaData = m_pFactory->CreateNode(
                XFA_XDPPACKET_Datasets, XFA_ELEMENT_DataValue);
            if (pXFAMetaData == NULL) {
              return;
            }
            pXFAMetaData->SetCData(XFA_ATTRIBUTE_Name, wsAttrName);
            pXFAMetaData->SetCData(XFA_ATTRIBUTE_QualifiedName,
                                   wsAttrQualifiedName);
            pXFAMetaData->SetCData(XFA_ATTRIBUTE_Value, wsAttrValue);
            pXFAMetaData->SetEnum(XFA_ATTRIBUTE_Contains,
                                  XFA_ATTRIBUTEENUM_MetaData);
            pXFAChild->InsertChild(pXFAMetaData);
            pXFAMetaData->SetXMLMappingNode(pXMLElement);
            pXFAMetaData->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
          }
          if (!bNeedValue) {
            CFX_WideString wsNilName = FX_WSTRC(L"xsi:nil");
            pXMLElement->RemoveAttribute(wsNilName);
          }
        }
        pXFANode->InsertChild(pXFAChild);
        if (eNodeType == XFA_ELEMENT_DataGroup) {
          ParseDataGroup(pXFAChild, pXMLElement, ePacketID);
        } else {
          if (bNeedValue) {
            ParseDataValue(pXFAChild, pXMLChild, XFA_XDPPACKET_Datasets);
          }
        }
        pXFAChild->SetXMLMappingNode(pXMLElement);
        pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
      }
        continue;
      case FDE_XMLNODE_CharData: {
        IFDE_XMLCharData* pXMLCharData = (IFDE_XMLCharData*)pXMLChild;
        CFX_WideString wsCharData;
        pXMLCharData->GetCharData(wsCharData);
        if (XFA_FDEExtension_IsStringAllWhitespace(wsCharData)) {
          continue;
        }
        CXFA_Node* pXFAChild = m_pFactory->CreateNode(XFA_XDPPACKET_Datasets,
                                                      XFA_ELEMENT_DataValue);
        if (pXFAChild == NULL) {
          return;
        }
        pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsCharData);
        pXFANode->InsertChild(pXFAChild);
        pXFAChild->SetXMLMappingNode(pXMLCharData);
        pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
      }
        continue;
      case FDE_XMLNODE_Text: {
        IFDE_XMLText* pXMLText = (IFDE_XMLText*)pXMLChild;
        CFX_WideString wsText;
        pXMLText->GetText(wsText);
        if (XFA_FDEExtension_IsStringAllWhitespace(wsText)) {
          continue;
        }
        CXFA_Node* pXFAChild = m_pFactory->CreateNode(XFA_XDPPACKET_Datasets,
                                                      XFA_ELEMENT_DataValue);
        if (pXFAChild == NULL) {
          return;
        }
        pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsText);
        pXFANode->InsertChild(pXFAChild);
        pXFAChild->SetXMLMappingNode(pXMLText);
        pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
      }
        continue;
      case FDE_XMLNODE_Instruction:
        continue;
      default:
        continue;
    }
  }
}
void CXFA_SimpleParser::ParseDataValue(CXFA_Node* pXFANode,
                                       IFDE_XMLNode* pXMLNode,
                                       XFA_XDPPACKET ePacketID) {
  CFX_WideTextBuf wsValueTextBuf;
  CFX_WideTextBuf wsCurValueTextBuf;
  FX_BOOL bMarkAsCompound = FALSE;
  IFDE_XMLNode* pXMLCurValueNode = NULL;
  for (IFDE_XMLNode* pXMLChild =
           pXMLNode->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLChild;
       pXMLChild = pXMLChild->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    FDE_XMLNODETYPE eNodeType = pXMLChild->GetType();
    if (eNodeType == FDE_XMLNODE_Instruction) {
      continue;
    }
    CFX_WideString wsText;
    if (eNodeType == FDE_XMLNODE_Text) {
      ((IFDE_XMLText*)pXMLChild)->GetText(wsText);
      if (!pXMLCurValueNode) {
        pXMLCurValueNode = pXMLChild;
      }
      wsCurValueTextBuf << wsText;
    } else if (eNodeType == FDE_XMLNODE_CharData) {
      ((IFDE_XMLCharData*)pXMLChild)->GetCharData(wsText);
      if (!pXMLCurValueNode) {
        pXMLCurValueNode = pXMLChild;
      }
      wsCurValueTextBuf << wsText;
    } else if (XFA_RecognizeRichText((IFDE_XMLElement*)pXMLChild)) {
#ifdef XFA_PARSE_HAS_LINEIDENTIFIER
      XFA_GetPlainTextFromRichText((IFDE_XMLElement*)pXMLChild, wsText);
#else
      XFA_ConvertRichTextToPlainText((IFDE_XMLElement*)pXMLChild, wsText);
#endif
      if (!pXMLCurValueNode) {
        pXMLCurValueNode = pXMLChild;
      }
      wsCurValueTextBuf << wsText;
    } else {
      bMarkAsCompound = TRUE;
      if (pXMLCurValueNode) {
        CFX_WideStringC wsCurValue = wsCurValueTextBuf.GetWideString();
        if (!wsCurValue.IsEmpty()) {
          CXFA_Node* pXFAChild =
              m_pFactory->CreateNode(ePacketID, XFA_ELEMENT_DataValue);
          if (pXFAChild == NULL) {
            return;
          }
          pXFAChild->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L""));
          pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsCurValue);
          pXFANode->InsertChild(pXFAChild);
          pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
          pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
          wsValueTextBuf << wsCurValue;
          wsCurValueTextBuf.Clear();
        }
        pXMLCurValueNode = NULL;
      }
      CXFA_Node* pXFAChild =
          m_pFactory->CreateNode(ePacketID, XFA_ELEMENT_DataValue);
      if (pXFAChild == NULL) {
        return;
      }
      CFX_WideString wsNodeStr;
      ((IFDE_XMLElement*)pXMLChild)->GetLocalTagName(wsNodeStr);
      pXFAChild->SetCData(XFA_ATTRIBUTE_Name, wsNodeStr);
      ParseDataValue(pXFAChild, pXMLChild, ePacketID);
      pXFANode->InsertChild(pXFAChild);
      pXFAChild->SetXMLMappingNode(pXMLChild);
      pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
      CFX_WideStringC wsCurValue = pXFAChild->GetCData(XFA_ATTRIBUTE_Value);
      wsValueTextBuf << wsCurValue;
    }
  }
  if (pXMLCurValueNode) {
    CFX_WideStringC wsCurValue = wsCurValueTextBuf.GetWideString();
    if (!wsCurValue.IsEmpty()) {
      if (bMarkAsCompound) {
        CXFA_Node* pXFAChild =
            m_pFactory->CreateNode(ePacketID, XFA_ELEMENT_DataValue);
        if (pXFAChild == NULL) {
          return;
        }
        pXFAChild->SetCData(XFA_ATTRIBUTE_Name, FX_WSTRC(L""));
        pXFAChild->SetCData(XFA_ATTRIBUTE_Value, wsCurValue);
        pXFANode->InsertChild(pXFAChild);
        pXFAChild->SetXMLMappingNode(pXMLCurValueNode);
        pXFAChild->SetFlag(XFA_NODEFLAG_Initialized, TRUE, FALSE);
      }
      wsValueTextBuf << wsCurValue;
      wsCurValueTextBuf.Clear();
    }
    pXMLCurValueNode = NULL;
  }
  CFX_WideStringC wsNodeValue = wsValueTextBuf.GetWideString();
  pXFANode->SetCData(XFA_ATTRIBUTE_Value, wsNodeValue);
}
void CXFA_SimpleParser::ParseInstruction(CXFA_Node* pXFANode,
                                         IFDE_XMLInstruction* pXMLInstruction,
                                         XFA_XDPPACKET ePacketID) {
  if (!m_bDocumentParser) {
    return;
  }
  CFX_WideString wsTargetName;
  pXMLInstruction->GetTargetName(wsTargetName);
  if (wsTargetName == FX_WSTRC(L"originalXFAVersion")) {
    CFX_WideString wsData;
    if (pXMLInstruction->GetData(0, wsData) &&
        (pXFANode->GetDocument()->RecognizeXFAVersionNumber(wsData) !=
         XFA_VERSION_UNKNOWN)) {
      wsData.Empty();
      if (pXMLInstruction->GetData(1, wsData) &&
          wsData == FX_WSTRC(L"v2.7-scripting:1")) {
        pXFANode->GetDocument()->SetFlag(XFA_DOCFLAG_Scripting, TRUE);
      }
    }
  } else if (wsTargetName == FX_WSTRC(L"acrobat")) {
    CFX_WideString wsData;
    if (pXMLInstruction->GetData(0, wsData) &&
        wsData == FX_WSTRC(L"JavaScript")) {
      if (pXMLInstruction->GetData(1, wsData) &&
          wsData == FX_WSTRC(L"strictScoping")) {
        pXFANode->GetDocument()->SetFlag(XFA_DOCFLAG_StrictScoping, TRUE);
      }
    }
  }
}
void CXFA_SimpleParser::CloseParser() {
  if (m_pXMLDoc) {
    m_pXMLDoc->Release();
    m_pXMLDoc = NULL;
  }
  if (m_pStream) {
    m_pStream->Release();
    m_pStream = NULL;
  }
}
IXFA_DocParser* IXFA_DocParser::Create(IXFA_Notify* pNotify) {
  return new CXFA_DocumentParser(pNotify);
}
CXFA_DocumentParser::CXFA_DocumentParser(IXFA_Notify* pNotify)
    : m_nodeParser(NULL, TRUE), m_pNotify(pNotify), m_pDocument(NULL) {
}
CXFA_DocumentParser::~CXFA_DocumentParser() {
  CloseParser();
}
int32_t CXFA_DocumentParser::StartParse(IFX_FileRead* pStream,
                                        XFA_XDPPACKET ePacketID) {
  CloseParser();
  int32_t nRetStatus = m_nodeParser.StartParse(pStream, ePacketID);
  if (nRetStatus == XFA_PARSESTATUS_Ready) {
    m_pDocument = new CXFA_Document(this);
    m_nodeParser.SetFactory(m_pDocument);
  }
  return nRetStatus;
}
int32_t CXFA_DocumentParser::DoParse(IFX_Pause* pPause) {
  int32_t nRetStatus = m_nodeParser.DoParse(pPause);
  if (nRetStatus >= XFA_PARSESTATUS_Done) {
    FXSYS_assert(m_pDocument);
    m_pDocument->SetRoot(m_nodeParser.GetRootNode());
  }
  return nRetStatus;
}
int32_t CXFA_DocumentParser::ParseXMLData(const CFX_WideString& wsXML,
                                          IFDE_XMLNode*& pXMLNode,
                                          IFX_Pause* pPause) {
  CloseParser();
  int32_t nRetStatus = m_nodeParser.ParseXMLData(wsXML, pXMLNode, NULL);
  if (nRetStatus == XFA_PARSESTATUS_Done && pXMLNode) {
    m_pDocument = new CXFA_Document(this);
    m_nodeParser.SetFactory(m_pDocument);
  }
  return nRetStatus;
}
void CXFA_DocumentParser::ConstructXFANode(CXFA_Node* pXFANode,
                                           IFDE_XMLNode* pXMLNode) {
  if (!pXFANode || !pXMLNode) {
    return;
  }
  m_nodeParser.ConstructXFANode(pXFANode, pXMLNode);
  CXFA_Node* pRootNode = m_nodeParser.GetRootNode();
  if (m_pDocument && pRootNode) {
    m_pDocument->SetRoot(pRootNode);
  }
}
void CXFA_DocumentParser::CloseParser() {
  if (m_pDocument) {
    delete m_pDocument;
    m_pDocument = NULL;
  }
  m_nodeParser.CloseParser();
}
CXFA_XMLParser::CXFA_XMLParser(IFDE_XMLNode* pRoot, IFX_Stream* pStream)
    :
#ifdef _XFA_VERIFY_Checksum_
      m_nElementStart(0),
      m_dwCheckStatus(0),
      m_dwCurrentCheckStatus(0),
#endif
      m_pRoot(pRoot),
      m_pStream(pStream),
      m_pParser(nullptr),
      m_pParent(pRoot),
      m_pChild(nullptr),
      m_NodeStack(16),
      m_dwStatus(FDE_XMLSYNTAXSTATUS_None) {
  ASSERT(m_pParent && m_pStream);
  m_NodeStack.Push(m_pParent);
  m_pParser = IFDE_XMLSyntaxParser::Create();
  m_pParser->Init(m_pStream, 32 * 1024, 1024 * 1024);
}
CXFA_XMLParser::~CXFA_XMLParser() {
  if (m_pParser) {
    m_pParser->Release();
  }
  m_NodeStack.RemoveAll();
  m_ws1.Empty();
  m_ws2.Empty();
}
int32_t CXFA_XMLParser::DoParser(IFX_Pause* pPause) {
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error) {
    return -1;
  }
  if (m_dwStatus == FDE_XMLSYNTAXSTATUS_EOS) {
    return 100;
  }
  int32_t iCount = 0;
  while (TRUE) {
    m_dwStatus = m_pParser->DoSyntaxParse();
    switch (m_dwStatus) {
      case FDE_XMLSYNTAXSTATUS_InstructionOpen:
        break;
      case FDE_XMLSYNTAXSTATUS_InstructionClose:
        if (m_pChild) {
          if (m_pChild->GetType() != FDE_XMLNODE_Instruction) {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            break;
          }
        }
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_ElementOpen:
#ifdef _XFA_VERIFY_Checksum_
        if (m_dwCheckStatus != 0x03 && m_NodeStack.GetSize() == 2) {
          m_nElementStart = m_pParser->GetCurrentPos() - 1;
        }
#endif
        break;
      case FDE_XMLSYNTAXSTATUS_ElementBreak:
        break;
      case FDE_XMLSYNTAXSTATUS_ElementClose:
        if (m_pChild->GetType() != FDE_XMLNODE_Element) {
          m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_pParser->GetTagName(m_ws1);
        ((IFDE_XMLElement*)m_pChild)->GetTagName(m_ws2);
        if (m_ws1.GetLength() > 0 && !m_ws1.Equal(m_ws2)) {
          m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
        m_NodeStack.Pop();
        if (m_NodeStack.GetSize() < 1) {
          m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
          break;
        }
#ifdef _XFA_VERIFY_Checksum_
        else if (m_dwCurrentCheckStatus != 0 && m_NodeStack.GetSize() == 2) {
          m_nSize[m_dwCurrentCheckStatus - 1] =
              m_pParser->GetCurrentBinaryPos() -
              m_nStart[m_dwCurrentCheckStatus - 1];
          m_dwCurrentCheckStatus = 0;
        }
#endif
        m_pParent = (IFDE_XMLNode*)*m_NodeStack.GetTopElement();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FDE_XMLSYNTAXSTATUS_TargetName:
        m_pParser->GetTargetName(m_ws1);
        if (m_ws1 == FX_WSTRC(L"originalXFAVersion") ||
            m_ws1 == FX_WSTRC(L"acrobat")) {
          m_pChild = IFDE_XMLInstruction::Create(m_ws1);
          m_pParent->InsertChildNode(m_pChild);
        } else {
          m_pChild = NULL;
        }
        m_ws1.Empty();
        break;
      case FDE_XMLSYNTAXSTATUS_TagName:
        m_pParser->GetTagName(m_ws1);
        m_pChild = IFDE_XMLElement::Create(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_NodeStack.Push(m_pChild);
        m_pParent = m_pChild;
#ifdef _XFA_VERIFY_Checksum_
        if (m_dwCheckStatus != 0x03 && m_NodeStack.GetSize() == 3) {
          CFX_WideString wsTag;
          ((IFDE_XMLElement*)m_pChild)->GetLocalTagName(wsTag);
          if (wsTag == FX_WSTRC(L"template")) {
            m_dwCheckStatus |= 0x01;
            m_dwCurrentCheckStatus = 0x01;
            m_nStart[0] = m_pParser->GetCurrentBinaryPos() -
                          (m_pParser->GetCurrentPos() - m_nElementStart);
          } else if (wsTag == FX_WSTRC(L"datasets")) {
            m_dwCheckStatus |= 0x02;
            m_dwCurrentCheckStatus = 0x02;
            m_nStart[1] = m_pParser->GetCurrentBinaryPos() -
                          (m_pParser->GetCurrentPos() - m_nElementStart);
          }
        }
#endif
        break;
      case FDE_XMLSYNTAXSTATUS_AttriName:
        m_pParser->GetAttributeName(m_ws1);
        break;
      case FDE_XMLSYNTAXSTATUS_AttriValue:
        if (m_pChild) {
          m_pParser->GetAttributeName(m_ws2);
          if (m_pChild->GetType() == FDE_XMLNODE_Element) {
            ((IFDE_XMLElement*)m_pChild)->SetString(m_ws1, m_ws2);
          }
        }
        m_ws1.Empty();
        break;
      case FDE_XMLSYNTAXSTATUS_Text:
        m_pParser->GetTextData(m_ws1);
        m_pChild = IFDE_XMLText::Create(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_CData:
        m_pParser->GetTextData(m_ws1);
        m_pChild = IFDE_XMLCharData::Create(m_ws1);
        m_pParent->InsertChildNode(m_pChild);
        m_pChild = m_pParent;
        break;
      case FDE_XMLSYNTAXSTATUS_TargetData:
        if (m_pChild) {
          if (m_pChild->GetType() != FDE_XMLNODE_Instruction) {
            m_dwStatus = FDE_XMLSYNTAXSTATUS_Error;
            break;
          }
          if (!m_ws1.IsEmpty()) {
            ((IFDE_XMLInstruction*)m_pChild)->AppendData(m_ws1);
          }
          m_pParser->GetTargetData(m_ws1);
          ((IFDE_XMLInstruction*)m_pChild)->AppendData(m_ws1);
        }
        m_ws1.Empty();
        break;
      default:
        break;
    }
    if (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error ||
        m_dwStatus == FDE_XMLSYNTAXSTATUS_EOS) {
      break;
    }
    if (pPause != NULL && iCount > 500 && pPause->NeedToPauseNow()) {
      break;
    }
  }
  return (m_dwStatus == FDE_XMLSYNTAXSTATUS_Error || m_NodeStack.GetSize() != 1)
             ? -1
             : m_pParser->GetStatus();
}
