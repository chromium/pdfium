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
class CFDE_XMLNode : public CFX_Target
{
public:
    CFDE_XMLNode();
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FDE_XMLNODETYPE	GetType() const
    {
        return FDE_XMLNODE_Unknown;
    }
    virtual FX_INT32		CountChildNodes() const;
    virtual CFDE_XMLNode*	GetChildNode(FX_INT32 index) const;
    virtual FX_INT32		GetChildNodeIndex(CFDE_XMLNode *pNode) const;
    virtual CFDE_XMLNode*	GetPath(FX_LPCWSTR pPath, FX_INT32 iLength = -1, FX_BOOL bQualifiedName = TRUE) const;
    virtual FX_INT32		InsertChildNode(CFDE_XMLNode *pNode, FX_INT32 index = -1);
    virtual void			RemoveChildNode(CFDE_XMLNode *pNode);
    virtual void			DeleteChildren();
    virtual CFDE_XMLNode*	GetNodeItem(IFDE_XMLNode::NodeItem eItem) const;
    virtual FX_INT32		GetNodeLevel() const;
    virtual FX_BOOL			InsertNodeItem(IFDE_XMLNode::NodeItem eItem, CFDE_XMLNode *pNode);
    virtual	CFDE_XMLNode*	RemoveNodeItem(IFDE_XMLNode::NodeItem eItem);
    virtual CFDE_XMLNode*	Clone(FX_BOOL bRecursive);
    virtual void			SaveXMLNode(IFX_Stream *pXMLStream);
public:
    ~CFDE_XMLNode();
    void					CloneChildren(CFDE_XMLNode* pClone);
    CFDE_XMLNode	*m_pParent;
    CFDE_XMLNode	*m_pChild;
    CFDE_XMLNode	*m_pPrior;
    CFDE_XMLNode	*m_pNext;
};
class CFDE_XMLInstruction : public CFDE_XMLNode
{
public:
    CFDE_XMLInstruction(const CFX_WideString &wsTarget);
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FDE_XMLNODETYPE	GetType() const
    {
        return FDE_XMLNODE_Instruction;
    }
    virtual CFDE_XMLNode*	Clone(FX_BOOL bRecursive);
    virtual void			GetTargetName(CFX_WideString &wsTarget) const
    {
        wsTarget = m_wsTarget;
    }
    virtual FX_INT32		CountAttributes() const;
    virtual FX_BOOL			GetAttribute(FX_INT32 index, CFX_WideString &wsAttriName, CFX_WideString &wsAttriValue) const;
    virtual FX_BOOL			HasAttribute(FX_LPCWSTR pwsAttriName) const;
    virtual void			GetString(FX_LPCWSTR pwsAttriName, CFX_WideString &wsAttriValue, FX_LPCWSTR pwsDefValue = NULL) const;
    virtual void			SetString(const CFX_WideString &wsAttriName, const CFX_WideString &wsAttriValue);
    virtual FX_INT32		GetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iDefValue = 0) const;
    virtual void			SetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iAttriValue);
    virtual FX_FLOAT		GetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fDefValue = 0) const;
    virtual void			SetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fAttriValue);
    virtual void			RemoveAttribute(FX_LPCWSTR pwsAttriName);
    virtual FX_INT32		CountData() const;
    virtual FX_BOOL			GetData(FX_INT32 index, CFX_WideString &wsData) const;
    virtual void			AppendData(const CFX_WideString &wsData);
    virtual void			RemoveData(FX_INT32 index);
public:
    ~CFDE_XMLInstruction() {}
    CFX_WideString			m_wsTarget;
    CFX_WideStringArray		m_Attributes;
    CFX_WideStringArray		m_TargetData;
};
class CFDE_XMLElement : public CFDE_XMLNode
{
public:
    CFDE_XMLElement(const CFX_WideString &wsTag);
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FDE_XMLNODETYPE	GetType() const
    {
        return FDE_XMLNODE_Element;
    }
    virtual CFDE_XMLNode*	Clone(FX_BOOL bRecursive);
    virtual void			GetTagName(CFX_WideString &wsTag) const;
    virtual void			GetLocalTagName(CFX_WideString &wsTag) const;
    virtual void			GetNamespacePrefix(CFX_WideString &wsPrefix) const;
    virtual void			GetNamespaceURI(CFX_WideString &wsNamespace) const;
    virtual FX_INT32		CountAttributes() const;
    virtual FX_BOOL			GetAttribute(FX_INT32 index, CFX_WideString &wsAttriName, CFX_WideString &wsAttriValue) const;
    virtual FX_BOOL			HasAttribute(FX_LPCWSTR pwsAttriName) const;
    virtual void			GetString(FX_LPCWSTR pwsAttriName, CFX_WideString &wsAttriValue, FX_LPCWSTR pwsDefValue = NULL) const;
    virtual void			SetString(const CFX_WideString &wsAttriName, const CFX_WideString &wsAttriValue);
    virtual FX_INT32		GetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iDefValue = 0) const;
    virtual void			SetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iAttriValue);
    virtual FX_FLOAT		GetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fDefValue = 0) const;
    virtual void			SetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fAttriValue);
    virtual void			RemoveAttribute(FX_LPCWSTR pwsAttriName);
    virtual void			GetTextData(CFX_WideString &wsText) const;
    virtual void			SetTextData(const CFX_WideString &wsText);
public:
    ~CFDE_XMLElement();
    CFX_WideString			m_wsTag;
    CFX_WideStringArray		m_Attributes;
};
class CFDE_XMLText : public CFDE_XMLNode
{
public:
    CFDE_XMLText(const CFX_WideString &wsText);
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FDE_XMLNODETYPE	GetType() const
    {
        return FDE_XMLNODE_Text;
    }
    virtual CFDE_XMLNode*	Clone(FX_BOOL bRecursive);
    virtual void			GetText(CFX_WideString &wsText) const
    {
        wsText = m_wsText;
    }
    virtual void			SetText(const CFX_WideString &wsText)
    {
        m_wsText = wsText;
    }
public:
    ~CFDE_XMLText() {}
    CFX_WideString		m_wsText;
};
class CFDE_XMLDeclaration : public CFDE_XMLNode
{
public:
    CFDE_XMLDeclaration() : CFDE_XMLNode() { }
};
class CFDE_XMLCharData : public CFDE_XMLDeclaration
{
public:
    CFDE_XMLCharData(const CFX_WideString &wsCData);

    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FDE_XMLNODETYPE	GetType() const
    {
        return FDE_XMLNODE_CharData;
    }
    virtual CFDE_XMLNode*	Clone(FX_BOOL bRecursive);
    virtual void			GetCharData(CFX_WideString &wsCharData) const
    {
        wsCharData = m_wsCharData;
    }
    virtual void			SetCharData(const CFX_WideString &wsCData)
    {
        m_wsCharData = wsCData;
    }

public:
    ~CFDE_XMLCharData() {}

    CFX_WideString		m_wsCharData;
};
class CFDE_XMLDoc : public CFX_Target
{
public:
    CFDE_XMLDoc();
    ~CFDE_XMLDoc();
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FX_BOOL			LoadXML(IFX_Stream *pXMLStream, FX_INT32 iXMLPlaneSize = 8192, FX_INT32 iTextDataSize = 256, FDE_LPXMLREADERHANDLER pHandler = NULL);
    virtual FX_BOOL			LoadXML(IFDE_XMLParser *pXMLParser);
    virtual FX_INT32		DoLoad(IFX_Pause *pPause = NULL);
    virtual void			CloseXML();
    virtual CFDE_XMLNode*	GetRoot() const
    {
        return m_pRoot;
    }
    virtual void			SaveXML(IFX_Stream *pXMLStream = NULL, FX_BOOL bSaveBOM = TRUE);
    virtual void			SaveXMLNode(IFX_Stream *pXMLStream, IFDE_XMLNode *pNode);
protected:
    IFX_Stream				*m_pStream;
    FX_INT32				m_iStatus;
    CFDE_XMLNode			*m_pRoot;
    IFDE_XMLSyntaxParser	*m_pSyntaxParser;
    IFDE_XMLParser			*m_pXMLParser;
    void			Reset(FX_BOOL bInitRoot);
    void			ReleaseParser();
};
typedef CFX_StackTemplate<CFDE_XMLNode*>	CFDE_XMLDOMNodeStack;
class CFDE_XMLDOMParser : public IFDE_XMLParser, public CFX_Target
{
public:
    CFDE_XMLDOMParser(CFDE_XMLNode *pRoot, IFDE_XMLSyntaxParser	*pParser);
    ~CFDE_XMLDOMParser();

    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FX_INT32		DoParser(IFX_Pause *pPause);
private:
    CFDE_XMLNode			*m_pRoot;
    IFDE_XMLSyntaxParser	*m_pParser;
    CFDE_XMLNode			*m_pParent;
    CFDE_XMLNode			*m_pChild;
    CFDE_XMLDOMNodeStack	m_NodeStack;
    CFX_WideString			m_ws1;
    CFX_WideString			m_ws2;
};
class CFDE_XMLTAG : public CFX_Target
{
public:
    CFDE_XMLTAG() : eType(FDE_XMLNODE_Unknown) {}
    CFDE_XMLTAG(const CFDE_XMLTAG &src) : wsTagName(src.wsTagName), eType(src.eType) {}
    CFX_WideString	wsTagName;
    FDE_XMLNODETYPE	eType;
};
typedef CFX_ObjectStackTemplate<CFDE_XMLTAG>	CFDE_XMLTagStack;
class CFDE_XMLSAXParser : public IFDE_XMLParser, public CFX_Target
{
public:
    CFDE_XMLSAXParser(FDE_LPXMLREADERHANDLER pHandler, IFDE_XMLSyntaxParser	*pParser);
    ~CFDE_XMLSAXParser();

    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual FX_INT32		DoParser(IFX_Pause *pPause);
private:
    void					Push(const CFDE_XMLTAG &xmlTag);
    void					Pop();
    FDE_LPXMLREADERHANDLER	m_pHandler;
    IFDE_XMLSyntaxParser	*m_pParser;
    CFDE_XMLTagStack		m_TagStack;
    CFDE_XMLTAG				*m_pTagTop;
    CFX_WideString			m_ws1;
    CFX_WideString			m_ws2;
};
#ifdef _FDE_BLOCK_BUFFER
class CFDE_BlockBuffer : public CFX_Target
{
public:
    CFDE_BlockBuffer(FX_INT32 iAllocStep = 1024 * 1024);
    ~CFDE_BlockBuffer();

    FX_BOOL InitBuffer(FX_INT32 iBufferSize = 1024 * 1024);
    FX_BOOL IsInitialized()
    {
        return m_iBufferSize / m_iAllocStep >= 1;
    }
    void ReleaseBuffer()
    {
        FDE_Delete this;
    }
    FX_LPWSTR GetAvailableBlock(FX_INT32& iIndexInBlock);
    inline FX_INT32 GetAllocStep() const
    {
        return m_iAllocStep;
    }
    inline FX_INT32& GetDataLengthRef()
    {
        return m_iDataLength;
    }
    inline void Reset(FX_BOOL bReserveData = TRUE)
    {
        if (!bReserveData) {
            m_iStartPosition = 0;
        }
        m_iDataLength = 0;
    }
    void SetTextChar(FX_INT32 iIndex, FX_WCHAR ch);
    FX_INT32 DeleteTextChars(FX_INT32 iCount, FX_BOOL bDirection = TRUE);
    void  GetTextData(CFX_WideString& wsTextData, FX_INT32 iStart  = 0, FX_INT32 iLength = -1) const;

protected:
    inline void TextDataIndex2BufIndex(const FX_INT32 iIndex, FX_INT32& iBlockIndex, FX_INT32& iInnerIndex) const;
    void ClearBuffer();
    CFX_PtrArray m_BlockArray;
    FX_INT32 m_iDataLength;
    FX_INT32 m_iBufferSize;
    FX_INT32 m_iAllocStep;
    FX_INT32 m_iStartPosition;
};
#endif
#define FDE_XMLSYNTAXMODE_Text				0
#define FDE_XMLSYNTAXMODE_Node				1
#define FDE_XMLSYNTAXMODE_Target			2
#define FDE_XMLSYNTAXMODE_Tag				3
#define FDE_XMLSYNTAXMODE_AttriName			4
#define FDE_XMLSYNTAXMODE_AttriEqualSign	5
#define FDE_XMLSYNTAXMODE_AttriQuotation	6
#define FDE_XMLSYNTAXMODE_AttriValue		7
#define FDE_XMLSYNTAXMODE_Entity			8
#define FDE_XMLSYNTAXMODE_EntityDecimal		9
#define FDE_XMLSYNTAXMODE_EntityHex			10
#define FDE_XMLSYNTAXMODE_CloseInstruction	11
#define FDE_XMLSYNTAXMODE_BreakElement		12
#define FDE_XMLSYNTAXMODE_CloseElement		13
#define FDE_XMLSYNTAXMODE_SkipDeclNode		14
#define FDE_XMLSYNTAXMODE_DeclCharData		15
#define FDE_XMLSYNTAXMODE_SkipComment		16
#define FDE_XMLSYNTAXMODE_SkipCommentOrDecl	17
#define FDE_XMLSYNTAXMODE_TargetData		18
class CFDE_XMLSyntaxParser : public IFDE_XMLSyntaxParser, public CFX_Target
{
public:
    CFDE_XMLSyntaxParser();
    ~CFDE_XMLSyntaxParser();
    virtual void			Release()
    {
        FDE_Delete this;
    }
    virtual void			Init(IFX_Stream *pStream, FX_INT32 iXMLPlaneSize, FX_INT32 iTextDataSize = 256);
    virtual FX_DWORD		DoSyntaxParse();
    virtual FX_INT32		GetStatus() const;
    virtual FX_INT32		GetCurrentPos() const
    {
        return m_iParsedChars + (m_pStart - m_pBuffer);
    }
    virtual FX_FILESIZE		GetCurrentBinaryPos() const;
    virtual FX_INT32		GetCurrentNodeNumber() const
    {
        return m_iCurrentNodeNum;
    }
    virtual FX_INT32		GetLastNodeNumber() const
    {
        return m_iLastNodeNum;
    }
#ifdef _FDE_BLOCK_BUFFER
    virtual void			GetTargetName(CFX_WideString &wsTarget) const
    {
        m_BlockBuffer.GetTextData(wsTarget, 0, m_iTextDataLength);
    }
    virtual void			GetTagName(CFX_WideString &wsTag) const
    {
        m_BlockBuffer.GetTextData(wsTag, 0, m_iTextDataLength);
    }
    virtual void			GetAttributeName(CFX_WideString &wsAttriName) const
    {
        m_BlockBuffer.GetTextData(wsAttriName, 0, m_iTextDataLength);
    }
    virtual void			GetAttributeValue(CFX_WideString &wsAttriValue) const
    {
        m_BlockBuffer.GetTextData(wsAttriValue, 0, m_iTextDataLength);
    }
    virtual void			GetTextData(CFX_WideString &wsText) const
    {
        m_BlockBuffer.GetTextData(wsText, 0, m_iTextDataLength);
    }
    virtual void			GetTargetData(CFX_WideString &wsData) const
    {
        m_BlockBuffer.GetTextData(wsData, 0, m_iTextDataLength);
    }
#else
    virtual void			GetTargetName(CFX_WideString &wsTarget) const
    {
        GetData(wsTarget);
    }
    virtual void			GetTagName(CFX_WideString &wsTag) const
    {
        GetData(wsTag);
    }
    virtual void			GetAttributeName(CFX_WideString &wsAttriName) const
    {
        GetData(wsAttriName);
    }
    virtual void			GetAttributeValue(CFX_WideString &wsAttriValue) const
    {
        GetData(wsAttriValue);
    }
    virtual void			GetTextData(CFX_WideString &wsText) const
    {
        GetData(wsText);
    }
    virtual void			GetTargetData(CFX_WideString &wsData) const
    {
        GetData(wsData);
    }
#endif
protected:
    IFX_Stream			*m_pStream;
    FX_INT32			m_iXMLPlaneSize;
    FX_INT32			m_iCurrentPos;
    FX_INT32			m_iCurrentNodeNum;
    FX_INT32			m_iLastNodeNum;
    FX_INT32			m_iParsedChars;
    FX_INT32			m_iParsedBytes;
    FX_LPWSTR			m_pBuffer;
    FX_INT32			m_iBufferChars;
    FX_BOOL				m_bEOS;
    FX_LPWSTR			m_pStart;
    FX_LPWSTR			m_pEnd;
    FDE_XMLNODE			m_CurNode;
    CFDE_XMLNodeStack	m_XMLNodeStack;
#ifdef _FDE_BLOCK_BUFFER
    CFDE_BlockBuffer	m_BlockBuffer;
    FX_INT32			m_iAllocStep;
    FX_INT32&			m_iDataLength;
    FX_LPWSTR			m_pCurrentBlock;
    FX_INT32			m_iIndexInBlock;
#else
    FX_INT32			m_iTextDataSize;
    FX_LPWSTR			m_pwsTextData;
    FX_INT32			m_iDataPos;
#endif
    FX_INT32			m_iTextDataLength;
    FX_DWORD			m_dwStatus;
    FX_DWORD			m_dwMode;
    FX_WCHAR			m_wQuotationMark;
    FX_INT32			m_iEntityStart;
    CFX_DWordStack		m_SkipStack;
    FX_WCHAR			m_SkipChar;
    inline void				ParseTextChar(FX_WCHAR ch);
#ifndef _FDE_BLOCK_BUFFER
    void				ReallocTextDataBuffer();
    void				GetData(CFX_WideString &wsData) const;
#endif
};
#endif
