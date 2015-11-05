// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_PARSER_IMP
#define _XFA_PARSER_IMP
#define _XFA_VERIFY_Checksum_
class CXFA_XMLParser;
class CXFA_SimpleParser : public IXFA_Parser {
 public:
  CXFA_SimpleParser(IXFA_ObjFactory* pFactory, FX_BOOL bDocumentParser = FALSE);
  ~CXFA_SimpleParser();
  virtual void Release() { delete this; }

  virtual int32_t StartParse(IFX_FileRead* pStream,
                             XFA_XDPPACKET ePacketID = XFA_XDPPACKET_XDP);
  virtual int32_t DoParse(IFX_Pause* pPause = NULL);
  virtual int32_t ParseXMLData(const CFX_WideString& wsXML,
                               IFDE_XMLNode*& pXMLNode,
                               IFX_Pause* pPause = NULL);
  virtual void ConstructXFANode(CXFA_Node* pXFANode, IFDE_XMLNode* pXMLNode);
  virtual IXFA_ObjFactory* GetFactory() const { return m_pFactory; }
  virtual CXFA_Node* GetRootNode() const { return m_pRootNode; }
  virtual IFDE_XMLDoc* GetXMLDoc() const { return m_pXMLDoc; }
  virtual void CloseParser();

 protected:
  CXFA_Node* ParseAsXDPPacket(IFDE_XMLNode* pXMLDocumentNode,
                              XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_XDP(IFDE_XMLNode* pXMLDocumentNode,
                                  XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Config(IFDE_XMLNode* pXMLDocumentNode,
                                     XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_TemplateForm(IFDE_XMLNode* pXMLDocumentNode,
                                           XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Data(IFDE_XMLNode* pXMLDocumentNode,
                                   XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_LocaleConnectionSourceSet(
      IFDE_XMLNode* pXMLDocumentNode,
      XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_Xdc(IFDE_XMLNode* pXMLDocumentNode,
                                  XFA_XDPPACKET ePacketID);
  CXFA_Node* ParseAsXDPPacket_User(IFDE_XMLNode* pXMLDocumentNode,
                                   XFA_XDPPACKET ePacketID);
  CXFA_Node* NormalLoader(CXFA_Node* pXFANode,
                          IFDE_XMLNode* pXMLDoc,
                          XFA_XDPPACKET ePacketID,
                          FX_BOOL bUseAttribute = TRUE);
  CXFA_Node* DataLoader(CXFA_Node* pXFANode,
                        IFDE_XMLNode* pXMLDoc,
                        FX_BOOL bDoTransform);
  CXFA_Node* UserPacketLoader(CXFA_Node* pXFANode, IFDE_XMLNode* pXMLDoc);
  void ParseContentNode(CXFA_Node* pXFANode,
                        IFDE_XMLNode* pXMLNode,
                        XFA_XDPPACKET ePacketID);
  void ParseDataValue(CXFA_Node* pXFANode,
                      IFDE_XMLNode* pXMLNode,
                      XFA_XDPPACKET ePacketID);
  void ParseDataGroup(CXFA_Node* pXFANode,
                      IFDE_XMLNode* pXMLNode,
                      XFA_XDPPACKET ePacketID);
  void ParseInstruction(CXFA_Node* pXFANode,
                        IFDE_XMLInstruction* pXMLInstruction,
                        XFA_XDPPACKET ePacketID);
  void SetFactory(IXFA_ObjFactory* pFactory);

  CXFA_XMLParser* m_pXMLParser;
  IFDE_XMLDoc* m_pXMLDoc;
  IFX_Stream* m_pStream;
  IFX_FileRead* m_pFileRead;
  IXFA_ObjFactory* m_pFactory;
  CXFA_Node* m_pRootNode;
  XFA_XDPPACKET m_ePacketID;
  FX_BOOL m_bDocumentParser;
  friend class CXFA_DocumentParser;
};
class CXFA_DocumentParser : public IXFA_DocParser {
 public:
  CXFA_DocumentParser(IXFA_Notify* pNotify);
  ~CXFA_DocumentParser();
  virtual void Release() { delete this; }
  virtual int32_t StartParse(IFX_FileRead* pStream,
                             XFA_XDPPACKET ePacketID = XFA_XDPPACKET_XDP);
  virtual int32_t DoParse(IFX_Pause* pPause = NULL);
  virtual int32_t ParseXMLData(const CFX_WideString& wsXML,
                               IFDE_XMLNode*& pXMLNode,
                               IFX_Pause* pPause = NULL);
  virtual void ConstructXFANode(CXFA_Node* pXFANode, IFDE_XMLNode* pXMLNode);
  virtual IXFA_ObjFactory* GetFactory() const {
    return m_nodeParser.GetFactory();
  }
  virtual CXFA_Node* GetRootNode() const { return m_nodeParser.GetRootNode(); }
  virtual IFDE_XMLDoc* GetXMLDoc() const { return m_nodeParser.GetXMLDoc(); }
  virtual IXFA_Notify* GetNotify() const { return m_pNotify; }
  virtual CXFA_Document* GetDocument() const { return m_pDocument; }
  virtual void CloseParser();

 protected:
  CXFA_SimpleParser m_nodeParser;
  IXFA_Notify* m_pNotify;
  CXFA_Document* m_pDocument;
};
typedef CFX_StackTemplate<IFDE_XMLNode*> CXFA_XMLNodeStack;
class CXFA_XMLParser : public IFDE_XMLParser {
 public:
  CXFA_XMLParser(IFDE_XMLNode* pRoot, IFX_Stream* pStream);
  ~CXFA_XMLParser();

  virtual void Release() { delete this; }
  virtual int32_t DoParser(IFX_Pause* pPause);

#ifdef _XFA_VERIFY_Checksum_
  FX_FILESIZE m_nStart[2];
  size_t m_nSize[2];
  FX_FILESIZE m_nElementStart;
  FX_WORD m_dwCheckStatus;
  FX_WORD m_dwCurrentCheckStatus;
#endif

 protected:
  IFDE_XMLNode* m_pRoot;
  IFX_Stream* m_pStream;
  IFDE_XMLSyntaxParser* m_pParser;

  IFDE_XMLNode* m_pParent;
  IFDE_XMLNode* m_pChild;
  CXFA_XMLNodeStack m_NodeStack;
  CFX_WideString m_ws1;
  CFX_WideString m_ws2;
  FX_DWORD m_dwStatus;
};
#endif
