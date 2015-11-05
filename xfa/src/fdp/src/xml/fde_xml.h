// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_XML_IMP
#define _FDE_XML_IMP
#define _FDE_BLOCK_BUFFER
#ifdef _FDE_BLOCK_BUFFER
class CFDE_BlockBuffer;
#endif
class CFDE_XMLNode;
class CFDE_XMLInstruction;
class CFDE_XMLElement;
class CFDE_XMLText;
class CFDE_XMLDoc;
class IFDE_XMLParser;
class CFDE_XMLDOMParser;
class CFDE_XMLSAXParser;
class CFDE_XMLSyntaxParser;
class CFDE_XMLNode : public CFX_Target {
 public:
  CFDE_XMLNode();
  virtual void Release() { delete this; }
  virtual FDE_XMLNODETYPE GetType() const { return FDE_XMLNODE_Unknown; }
  virtual int32_t CountChildNodes() const;
  virtual CFDE_XMLNode* GetChildNode(int32_t index) const;
  virtual int32_t GetChildNodeIndex(CFDE_XMLNode* pNode) const;
  virtual CFDE_XMLNode* GetPath(const FX_WCHAR* pPath,
                                int32_t iLength = -1,
                                FX_BOOL bQualifiedName = TRUE) const;
  virtual int32_t InsertChildNode(CFDE_XMLNode* pNode, int32_t index = -1);
  virtual void RemoveChildNode(CFDE_XMLNode* pNode);
  virtual void DeleteChildren();
  virtual CFDE_XMLNode* GetNodeItem(IFDE_XMLNode::NodeItem eItem) const;
  virtual int32_t GetNodeLevel() const;
  virtual FX_BOOL InsertNodeItem(IFDE_XMLNode::NodeItem eItem,
                                 CFDE_XMLNode* pNode);
  virtual CFDE_XMLNode* RemoveNodeItem(IFDE_XMLNode::NodeItem eItem);
  virtual CFDE_XMLNode* Clone(FX_BOOL bRecursive);
  virtual void SaveXMLNode(IFX_Stream* pXMLStream);

 public:
  ~CFDE_XMLNode();
  void CloneChildren(CFDE_XMLNode* pClone);
  CFDE_XMLNode* m_pParent;
  CFDE_XMLNode* m_pChild;
  CFDE_XMLNode* m_pPrior;
  CFDE_XMLNode* m_pNext;
};
class CFDE_XMLInstruction : public CFDE_XMLNode {
 public:
  CFDE_XMLInstruction(const CFX_WideString& wsTarget);
  virtual void Release() { delete this; }
  virtual FDE_XMLNODETYPE GetType() const { return FDE_XMLNODE_Instruction; }
  virtual CFDE_XMLNode* Clone(FX_BOOL bRecursive);
  virtual void GetTargetName(CFX_WideString& wsTarget) const {
    wsTarget = m_wsTarget;
  }
  virtual int32_t CountAttributes() const;
  virtual FX_BOOL GetAttribute(int32_t index,
                               CFX_WideString& wsAttriName,
                               CFX_WideString& wsAttriValue) const;
  virtual FX_BOOL HasAttribute(const FX_WCHAR* pwsAttriName) const;
  virtual void GetString(const FX_WCHAR* pwsAttriName,
                         CFX_WideString& wsAttriValue,
                         const FX_WCHAR* pwsDefValue = NULL) const;
  virtual void SetString(const CFX_WideString& wsAttriName,
                         const CFX_WideString& wsAttriValue);
  virtual int32_t GetInteger(const FX_WCHAR* pwsAttriName,
                             int32_t iDefValue = 0) const;
  virtual void SetInteger(const FX_WCHAR* pwsAttriName, int32_t iAttriValue);
  virtual FX_FLOAT GetFloat(const FX_WCHAR* pwsAttriName,
                            FX_FLOAT fDefValue = 0) const;
  virtual void SetFloat(const FX_WCHAR* pwsAttriName, FX_FLOAT fAttriValue);
  virtual void RemoveAttribute(const FX_WCHAR* pwsAttriName);
  virtual int32_t CountData() const;
  virtual FX_BOOL GetData(int32_t index, CFX_WideString& wsData) const;
  virtual void AppendData(const CFX_WideString& wsData);
  virtual void RemoveData(int32_t index);

 public:
  ~CFDE_XMLInstruction() {}
  CFX_WideString m_wsTarget;
  CFX_WideStringArray m_Attributes;
  CFX_WideStringArray m_TargetData;
};
class CFDE_XMLElement : public CFDE_XMLNode {
 public:
  CFDE_XMLElement(const CFX_WideString& wsTag);
  virtual void Release() { delete this; }
  virtual FDE_XMLNODETYPE GetType() const { return FDE_XMLNODE_Element; }
  virtual CFDE_XMLNode* Clone(FX_BOOL bRecursive);
  virtual void GetTagName(CFX_WideString& wsTag) const;
  virtual void GetLocalTagName(CFX_WideString& wsTag) const;
  virtual void GetNamespacePrefix(CFX_WideString& wsPrefix) const;
  virtual void GetNamespaceURI(CFX_WideString& wsNamespace) const;
  virtual int32_t CountAttributes() const;
  virtual FX_BOOL GetAttribute(int32_t index,
                               CFX_WideString& wsAttriName,
                               CFX_WideString& wsAttriValue) const;
  virtual FX_BOOL HasAttribute(const FX_WCHAR* pwsAttriName) const;
  virtual void GetString(const FX_WCHAR* pwsAttriName,
                         CFX_WideString& wsAttriValue,
                         const FX_WCHAR* pwsDefValue = NULL) const;
  virtual void SetString(const CFX_WideString& wsAttriName,
                         const CFX_WideString& wsAttriValue);
  virtual int32_t GetInteger(const FX_WCHAR* pwsAttriName,
                             int32_t iDefValue = 0) const;
  virtual void SetInteger(const FX_WCHAR* pwsAttriName, int32_t iAttriValue);
  virtual FX_FLOAT GetFloat(const FX_WCHAR* pwsAttriName,
                            FX_FLOAT fDefValue = 0) const;
  virtual void SetFloat(const FX_WCHAR* pwsAttriName, FX_FLOAT fAttriValue);
  virtual void RemoveAttribute(const FX_WCHAR* pwsAttriName);
  virtual void GetTextData(CFX_WideString& wsText) const;
  virtual void SetTextData(const CFX_WideString& wsText);

 public:
  ~CFDE_XMLElement();
  CFX_WideString m_wsTag;
  CFX_WideStringArray m_Attributes;
};
class CFDE_XMLText : public CFDE_XMLNode {
 public:
  CFDE_XMLText(const CFX_WideString& wsText);
  virtual void Release() { delete this; }
  virtual FDE_XMLNODETYPE GetType() const { return FDE_XMLNODE_Text; }
  virtual CFDE_XMLNode* Clone(FX_BOOL bRecursive);
  virtual void GetText(CFX_WideString& wsText) const { wsText = m_wsText; }
  virtual void SetText(const CFX_WideString& wsText) { m_wsText = wsText; }

 public:
  ~CFDE_XMLText() {}
  CFX_WideString m_wsText;
};
class CFDE_XMLDeclaration : public CFDE_XMLNode {
 public:
  CFDE_XMLDeclaration() : CFDE_XMLNode() {}
};
class CFDE_XMLCharData : public CFDE_XMLDeclaration {
 public:
  CFDE_XMLCharData(const CFX_WideString& wsCData);

  virtual void Release() { delete this; }
  virtual FDE_XMLNODETYPE GetType() const { return FDE_XMLNODE_CharData; }
  virtual CFDE_XMLNode* Clone(FX_BOOL bRecursive);
  virtual void GetCharData(CFX_WideString& wsCharData) const {
    wsCharData = m_wsCharData;
  }
  virtual void SetCharData(const CFX_WideString& wsCData) {
    m_wsCharData = wsCData;
  }

 public:
  ~CFDE_XMLCharData() {}

  CFX_WideString m_wsCharData;
};
class CFDE_XMLDoc : public CFX_Target {
 public:
  CFDE_XMLDoc();
  ~CFDE_XMLDoc();
  virtual void Release() { delete this; }
  virtual FX_BOOL LoadXML(IFX_Stream* pXMLStream,
                          int32_t iXMLPlaneSize = 8192,
                          int32_t iTextDataSize = 256,
                          FDE_LPXMLREADERHANDLER pHandler = NULL);
  virtual FX_BOOL LoadXML(IFDE_XMLParser* pXMLParser);
  virtual int32_t DoLoad(IFX_Pause* pPause = NULL);
  virtual void CloseXML();
  virtual CFDE_XMLNode* GetRoot() const { return m_pRoot; }
  virtual void SaveXML(IFX_Stream* pXMLStream = NULL, FX_BOOL bSaveBOM = TRUE);
  virtual void SaveXMLNode(IFX_Stream* pXMLStream, IFDE_XMLNode* pNode);

 protected:
  IFX_Stream* m_pStream;
  int32_t m_iStatus;
  CFDE_XMLNode* m_pRoot;
  IFDE_XMLSyntaxParser* m_pSyntaxParser;
  IFDE_XMLParser* m_pXMLParser;
  void Reset(FX_BOOL bInitRoot);
  void ReleaseParser();
};
typedef CFX_StackTemplate<CFDE_XMLNode*> CFDE_XMLDOMNodeStack;
class CFDE_XMLDOMParser : public IFDE_XMLParser, public CFX_Target {
 public:
  CFDE_XMLDOMParser(CFDE_XMLNode* pRoot, IFDE_XMLSyntaxParser* pParser);
  ~CFDE_XMLDOMParser();

  virtual void Release() { delete this; }
  virtual int32_t DoParser(IFX_Pause* pPause);

 private:
  IFDE_XMLSyntaxParser* m_pParser;
  CFDE_XMLNode* m_pParent;
  CFDE_XMLNode* m_pChild;
  CFDE_XMLDOMNodeStack m_NodeStack;
  CFX_WideString m_ws1;
  CFX_WideString m_ws2;
};
class CFDE_XMLTAG : public CFX_Target {
 public:
  CFDE_XMLTAG() : eType(FDE_XMLNODE_Unknown) {}
  CFDE_XMLTAG(const CFDE_XMLTAG& src)
      : wsTagName(src.wsTagName), eType(src.eType) {}
  CFX_WideString wsTagName;
  FDE_XMLNODETYPE eType;
};
typedef CFX_ObjectStackTemplate<CFDE_XMLTAG> CFDE_XMLTagStack;
class CFDE_XMLSAXParser : public IFDE_XMLParser, public CFX_Target {
 public:
  CFDE_XMLSAXParser(FDE_LPXMLREADERHANDLER pHandler,
                    IFDE_XMLSyntaxParser* pParser);
  ~CFDE_XMLSAXParser();

  virtual void Release() { delete this; }
  virtual int32_t DoParser(IFX_Pause* pPause);

 private:
  void Push(const CFDE_XMLTAG& xmlTag);
  void Pop();
  FDE_LPXMLREADERHANDLER m_pHandler;
  IFDE_XMLSyntaxParser* m_pParser;
  CFDE_XMLTagStack m_TagStack;
  CFDE_XMLTAG* m_pTagTop;
  CFX_WideString m_ws1;
  CFX_WideString m_ws2;
};
#ifdef _FDE_BLOCK_BUFFER
class CFDE_BlockBuffer : public CFX_Target {
 public:
  CFDE_BlockBuffer(int32_t iAllocStep = 1024 * 1024);
  ~CFDE_BlockBuffer();

  FX_BOOL InitBuffer(int32_t iBufferSize = 1024 * 1024);
  FX_BOOL IsInitialized() { return m_iBufferSize / m_iAllocStep >= 1; }
  void ReleaseBuffer() { delete this; }
  FX_WCHAR* GetAvailableBlock(int32_t& iIndexInBlock);
  inline int32_t GetAllocStep() const { return m_iAllocStep; }
  inline int32_t& GetDataLengthRef() { return m_iDataLength; }
  inline void Reset(FX_BOOL bReserveData = TRUE) {
    if (!bReserveData) {
      m_iStartPosition = 0;
    }
    m_iDataLength = 0;
  }
  void SetTextChar(int32_t iIndex, FX_WCHAR ch);
  int32_t DeleteTextChars(int32_t iCount, FX_BOOL bDirection = TRUE);
  void GetTextData(CFX_WideString& wsTextData,
                   int32_t iStart = 0,
                   int32_t iLength = -1) const;

 protected:
  inline void TextDataIndex2BufIndex(const int32_t iIndex,
                                     int32_t& iBlockIndex,
                                     int32_t& iInnerIndex) const;
  void ClearBuffer();
  CFX_PtrArray m_BlockArray;
  int32_t m_iDataLength;
  int32_t m_iBufferSize;
  int32_t m_iAllocStep;
  int32_t m_iStartPosition;
};
#endif
#define FDE_XMLSYNTAXMODE_Text 0
#define FDE_XMLSYNTAXMODE_Node 1
#define FDE_XMLSYNTAXMODE_Target 2
#define FDE_XMLSYNTAXMODE_Tag 3
#define FDE_XMLSYNTAXMODE_AttriName 4
#define FDE_XMLSYNTAXMODE_AttriEqualSign 5
#define FDE_XMLSYNTAXMODE_AttriQuotation 6
#define FDE_XMLSYNTAXMODE_AttriValue 7
#define FDE_XMLSYNTAXMODE_Entity 8
#define FDE_XMLSYNTAXMODE_EntityDecimal 9
#define FDE_XMLSYNTAXMODE_EntityHex 10
#define FDE_XMLSYNTAXMODE_CloseInstruction 11
#define FDE_XMLSYNTAXMODE_BreakElement 12
#define FDE_XMLSYNTAXMODE_CloseElement 13
#define FDE_XMLSYNTAXMODE_SkipDeclNode 14
#define FDE_XMLSYNTAXMODE_DeclCharData 15
#define FDE_XMLSYNTAXMODE_SkipComment 16
#define FDE_XMLSYNTAXMODE_SkipCommentOrDecl 17
#define FDE_XMLSYNTAXMODE_TargetData 18
class CFDE_XMLSyntaxParser : public IFDE_XMLSyntaxParser, public CFX_Target {
 public:
  CFDE_XMLSyntaxParser();
  ~CFDE_XMLSyntaxParser();
  virtual void Release() { delete this; }
  virtual void Init(IFX_Stream* pStream,
                    int32_t iXMLPlaneSize,
                    int32_t iTextDataSize = 256);
  virtual FX_DWORD DoSyntaxParse();
  virtual int32_t GetStatus() const;
  virtual int32_t GetCurrentPos() const {
    return m_iParsedChars + (m_pStart - m_pBuffer);
  }
  virtual FX_FILESIZE GetCurrentBinaryPos() const;
  virtual int32_t GetCurrentNodeNumber() const { return m_iCurrentNodeNum; }
  virtual int32_t GetLastNodeNumber() const { return m_iLastNodeNum; }
#ifdef _FDE_BLOCK_BUFFER
  virtual void GetTargetName(CFX_WideString& wsTarget) const {
    m_BlockBuffer.GetTextData(wsTarget, 0, m_iTextDataLength);
  }
  virtual void GetTagName(CFX_WideString& wsTag) const {
    m_BlockBuffer.GetTextData(wsTag, 0, m_iTextDataLength);
  }
  virtual void GetAttributeName(CFX_WideString& wsAttriName) const {
    m_BlockBuffer.GetTextData(wsAttriName, 0, m_iTextDataLength);
  }
  virtual void GetAttributeValue(CFX_WideString& wsAttriValue) const {
    m_BlockBuffer.GetTextData(wsAttriValue, 0, m_iTextDataLength);
  }
  virtual void GetTextData(CFX_WideString& wsText) const {
    m_BlockBuffer.GetTextData(wsText, 0, m_iTextDataLength);
  }
  virtual void GetTargetData(CFX_WideString& wsData) const {
    m_BlockBuffer.GetTextData(wsData, 0, m_iTextDataLength);
  }
#else
  virtual void GetTargetName(CFX_WideString& wsTarget) const {
    GetData(wsTarget);
  }
  virtual void GetTagName(CFX_WideString& wsTag) const { GetData(wsTag); }
  virtual void GetAttributeName(CFX_WideString& wsAttriName) const {
    GetData(wsAttriName);
  }
  virtual void GetAttributeValue(CFX_WideString& wsAttriValue) const {
    GetData(wsAttriValue);
  }
  virtual void GetTextData(CFX_WideString& wsText) const { GetData(wsText); }
  virtual void GetTargetData(CFX_WideString& wsData) const { GetData(wsData); }
#endif
 protected:
  IFX_Stream* m_pStream;
  int32_t m_iXMLPlaneSize;
  int32_t m_iCurrentPos;
  int32_t m_iCurrentNodeNum;
  int32_t m_iLastNodeNum;
  int32_t m_iParsedChars;
  int32_t m_iParsedBytes;
  FX_WCHAR* m_pBuffer;
  int32_t m_iBufferChars;
  FX_BOOL m_bEOS;
  FX_WCHAR* m_pStart;
  FX_WCHAR* m_pEnd;
  FDE_XMLNODE m_CurNode;
  CFDE_XMLNodeStack m_XMLNodeStack;
#ifdef _FDE_BLOCK_BUFFER
  CFDE_BlockBuffer m_BlockBuffer;
  int32_t m_iAllocStep;
  int32_t& m_iDataLength;
  FX_WCHAR* m_pCurrentBlock;
  int32_t m_iIndexInBlock;
#else
  int32_t m_iTextDataSize;
  FX_WCHAR* m_pwsTextData;
  int32_t m_iDataPos;
#endif
  int32_t m_iTextDataLength;
  FX_DWORD m_dwStatus;
  FX_DWORD m_dwMode;
  FX_WCHAR m_wQuotationMark;
  int32_t m_iEntityStart;
  CFX_DWordStack m_SkipStack;
  FX_WCHAR m_SkipChar;
  inline void ParseTextChar(FX_WCHAR ch);
#ifndef _FDE_BLOCK_BUFFER
  void ReallocTextDataBuffer();
  void GetData(CFX_WideString& wsData) const;
#endif
};
#endif
