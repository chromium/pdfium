// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_PARSER_H
#define _XFA_PARSER_H
class IFX_Stream;
class IXFA_Parser {
 public:
  static IXFA_Parser* Create(IXFA_ObjFactory* pFactory,
                             FX_BOOL bDocumentParser = FALSE);
  virtual ~IXFA_Parser() {}
  virtual void Release() = 0;
  virtual int32_t StartParse(IFX_FileRead* pStream,
                             XFA_XDPPACKET ePacketID = XFA_XDPPACKET_XDP) = 0;
  virtual int32_t DoParse(IFX_Pause* pPause = NULL) = 0;
  virtual int32_t ParseXMLData(const CFX_WideString& wsXML,
                               IFDE_XMLNode*& pXMLNode,
                               IFX_Pause* pPause = NULL) = 0;
  virtual void ConstructXFANode(CXFA_Node* pXFANode,
                                IFDE_XMLNode* pXMLNode) = 0;
  virtual IXFA_ObjFactory* GetFactory() const = 0;
  virtual CXFA_Node* GetRootNode() const = 0;
  virtual IFDE_XMLDoc* GetXMLDoc() const = 0;
  virtual void CloseParser() = 0;
};
class IXFA_DocParser : public IXFA_Parser {
 public:
  static IXFA_DocParser* Create(IXFA_Notify* pNotify);
  virtual CXFA_Document* GetDocument() const = 0;
  virtual IXFA_Notify* GetNotify() const = 0;
};
#endif
