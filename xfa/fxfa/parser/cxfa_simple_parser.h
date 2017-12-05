// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_SIMPLE_PARSER_H_
#define XFA_FXFA_PARSER_CXFA_SIMPLE_PARSER_H_

#include <memory>

#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Document;
class CXFA_Node;
class CFX_XMLDoc;
class CFX_XMLInstruction;
class CFX_XMLNode;
class CFX_XMLParser;
class IFX_SeekableStream;
class CFX_SeekableStreamProxy;

class CXFA_SimpleParser {
 public:
  CXFA_SimpleParser();
  explicit CXFA_SimpleParser(CXFA_Document* pFactory);
  ~CXFA_SimpleParser();

  int32_t StartParse(const RetainPtr<IFX_SeekableStream>& pStream,
                     XFA_PacketType ePacketID);
  int32_t DoParse();
  CFX_XMLNode* ParseXMLData(const ByteString& wsXML);
  void ConstructXFANode(CXFA_Node* pXFANode, CFX_XMLNode* pXMLNode);
  CXFA_Node* GetRootNode() const;
  CFX_XMLDoc* GetXMLDoc() const;
  void CloseParser();

  // Called later for the ctor with no parameters.
  void SetFactory(CXFA_Document* pFactory);

 private:
  CXFA_Node* ParseAsXDPPacket(CFX_XMLNode* pXMLDocumentNode,
                              XFA_PacketType ePacketID);
  CXFA_Node* ParseAsXDPPacket_XDP(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_Config(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_Template(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_Form(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_Data(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_LocaleConnectionSourceSet(
      CFX_XMLNode* pXMLDocumentNode,
      XFA_PacketType packet_type,
      XFA_Element element);
  CXFA_Node* ParseAsXDPPacket_Xdc(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* ParseAsXDPPacket_User(CFX_XMLNode* pXMLDocumentNode);
  CXFA_Node* NormalLoader(CXFA_Node* pXFANode,
                          CFX_XMLNode* pXMLDoc,
                          XFA_PacketType ePacketID,
                          bool bUseAttribute);
  CXFA_Node* DataLoader(CXFA_Node* pXFANode,
                        CFX_XMLNode* pXMLDoc,
                        bool bDoTransform);
  CXFA_Node* UserPacketLoader(CXFA_Node* pXFANode, CFX_XMLNode* pXMLDoc);
  void ParseContentNode(CXFA_Node* pXFANode,
                        CFX_XMLNode* pXMLNode,
                        XFA_PacketType ePacketID);
  void ParseDataValue(CXFA_Node* pXFANode,
                      CFX_XMLNode* pXMLNode,
                      XFA_PacketType ePacketID);
  void ParseDataGroup(CXFA_Node* pXFANode,
                      CFX_XMLNode* pXMLNode,
                      XFA_PacketType ePacketID);
  void ParseInstruction(CXFA_Node* pXFANode,
                        CFX_XMLInstruction* pXMLInstruction,
                        XFA_PacketType ePacketID);

  std::unique_ptr<CFX_XMLDoc> m_pXMLDoc;
  UnownedPtr<CFX_XMLParser> m_pXMLParser;  // Owned by |m_pXMLDoc|
  RetainPtr<CFX_SeekableStreamProxy> m_pStream;
  RetainPtr<IFX_SeekableStream> m_pFileRead;
  UnownedPtr<CXFA_Document> m_pFactory;
  // TODO(dsinclair): Figure out who owns this.
  CXFA_Node* m_pRootNode = nullptr;
  XFA_PacketType m_ePacketID = XFA_PacketType::User;
  bool m_bParseStarted = false;
  const bool m_bDocumentParser;
};

#endif  // XFA_FXFA_PARSER_CXFA_SIMPLE_PARSER_H_
