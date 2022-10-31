// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DOCUMENT_BUILDER_H_
#define XFA_FXFA_PARSER_CXFA_DOCUMENT_BUILDER_H_

#include "core/fxcrt/unowned_ptr.h"
#include "v8/include/cppgc/macros.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFX_XMLDocument;
class CFX_XMLNode;
class CXFA_Document;
class CXFA_Node;
class CFX_XMLInstruction;

class CXFA_DocumentBuilder {
  CPPGC_STACK_ALLOCATED();  // Allow Raw/Unowned pointers.

 public:
  explicit CXFA_DocumentBuilder(CXFA_Document* pNodeFactory);
  ~CXFA_DocumentBuilder();

  CFX_XMLNode* Build(CFX_XMLDocument* pXML);
  bool BuildDocument(CFX_XMLDocument* pXML, XFA_PacketType ePacketID);
  void ConstructXFANode(CXFA_Node* pXFANode, CFX_XMLNode* pXMLNode);
  CXFA_Node* GetRootNode() const;

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
  CXFA_Node* DataLoader(CXFA_Node* pXFANode, CFX_XMLNode* pXMLDoc);
  CXFA_Node* NormalLoader(CXFA_Node* pXFANode,
                          CFX_XMLNode* pXMLDoc,
                          XFA_PacketType ePacketID,
                          bool bUseAttribute);
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

  UnownedPtr<CXFA_Document> node_factory_;  // OK, stack-only.
  UnownedPtr<CXFA_Node> root_node_;         // OK, stack-only.
  UnownedPtr<CFX_XMLDocument> xml_doc_;
  size_t execute_recursion_depth_ = 0;
};

#endif  // XFA_FXFA_PARSER_CXFA_DOCUMENT_BUILDER_H_
