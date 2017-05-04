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
  CXFA_SimpleParser(CXFA_Document* pFactory, bool bDocumentParser);
  ~CXFA_SimpleParser();

  int32_t StartParse(const CFX_RetainPtr<IFX_SeekableStream>& pStream,
                     XFA_XDPPACKET ePacketID);
  int32_t DoParse();
  CFX_XMLNode* ParseXMLData(const CFX_ByteString& wsXML);
  void ConstructXFANode(CXFA_Node* pXFANode, CFX_XMLNode* pXMLNode);
  CXFA_Node* GetRootNode() const;
  CFX_XMLDoc* GetXMLDoc() const;
  void CloseParser();

  void SetFactory(CXFA_Document* pFactory);

 private:
  CXFA_Node* ParseAsXDPPacket(CFX_XMLNode* pXMLDocumentNode,
                              XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_XDP(CFX_XMLNode* pXMLDocumentNode,
                                  XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Config(CFX_XMLNode* pXMLDocumentNode,
                                     XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_TemplateForm(CFX_XMLNode* pXMLDocumentNode,
                                           XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Data(CFX_XMLNode* pXMLDocumentNode,
                                   XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_LocaleConnectionSourceSet(
      CFX_XMLNode* pXMLDocumentNode,
      XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Xdc(CFX_XMLNode* pXMLDocumentNode,
                                  XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_User(CFX_XMLNode* pXMLDocumentNode,
                                   XFA_XDPPACKET ePacketID);
  CXFA_Node* NormalLoader(CXFA_Node* pXFANode,
                          CFX_XMLNode* pXMLDoc,
                          XFA_XDPPACKET ePacketID,
                          bool bUseAttribute);
  CXFA_Node* DataLoader(CXFA_Node* pXFANode,
                        CFX_XMLNode* pXMLDoc,
                        bool bDoTransform);
  CXFA_Node* UserPacketLoader(CXFA_Node* pXFANode, CFX_XMLNode* pXMLDoc);
  void ParseContentNode(CXFA_Node* pXFANode,
                        CFX_XMLNode* pXMLNode,
                        XFA_XDPPACKET ePacketID);
  void ParseDataValue(CXFA_Node* pXFANode,
                      CFX_XMLNode* pXMLNode,
                      XFA_XDPPACKET ePacketID);
  void ParseDataGroup(CXFA_Node* pXFANode,
                      CFX_XMLNode* pXMLNode,
                      XFA_XDPPACKET ePacketID);
  void ParseInstruction(CXFA_Node* pXFANode,
                        CFX_XMLInstruction* pXMLInstruction,
                        XFA_XDPPACKET ePacketID);

  CFX_XMLParser* m_pXMLParser;
  std::unique_ptr<CFX_XMLDoc> m_pXMLDoc;
  CFX_RetainPtr<CFX_SeekableStreamProxy> m_pStream;
  CFX_RetainPtr<IFX_SeekableStream> m_pFileRead;
  CXFA_Document* m_pFactory;
  CXFA_Node* m_pRootNode;
  XFA_XDPPACKET m_ePacketID;
  bool m_bDocumentParser;
};

#endif  // XFA_FXFA_PARSER_CXFA_SIMPLE_PARSER_H_
