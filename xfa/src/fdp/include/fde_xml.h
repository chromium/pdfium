// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_XML
#define _FDE_XML
class IFDE_XMLNode;
class IFDE_XMLInstruction;
class IFDE_XMLDeclaration;
class IFDE_XMLDeclComment;
class IFDE_XMLDeclCharData;
class IFDE_XMLDeclDocType;
class IFDE_XMLDeclElement;
class IFDE_XMLDeclAttriList;
class IFDE_XMLDeclNotition;
class IFDE_XMLDeclEntity;
class IFDE_XMLElement;
class IFDE_XMLText;
class IFDE_XMLDoc;
class IFDE_XMLParser;
class IFDE_XMLSyntaxParser;
#ifdef __cplusplus
extern "C" {
#endif
enum FDE_XMLNODETYPE {
    FDE_XMLNODE_Unknown			=  0,
    FDE_XMLNODE_Instruction			,
    FDE_XMLNODE_Element				,
    FDE_XMLNODE_Text				,
    FDE_XMLNODE_CharData			,
};
typedef struct _FDE_XMLNODE {
    FX_INT32		iNodeNum;
    FDE_XMLNODETYPE	eNodeType;
} FDE_XMLNODE, * FDE_LPXMLNODE;
typedef FDE_XMLNODE const * FDE_LPCXMLNODE;
typedef CFX_StackTemplate<FDE_XMLNODE> CFDE_XMLNodeStack;
FX_BOOL		FDE_IsXMLValidChar(FX_WCHAR ch);
FX_BOOL		FDE_IsXMLWhiteSpace(FX_WCHAR ch);
FX_BOOL		FDE_IsXMLNameChar(FX_WCHAR ch, FX_BOOL bFirstChar);
#ifdef __cplusplus
}
#endif
class IFDE_XMLNode
{
public:
    virtual void			Release() = 0;
    virtual FDE_XMLNODETYPE	GetType() const = 0;
    virtual FX_INT32		CountChildNodes() const = 0;
    virtual IFDE_XMLNode*	GetChildNode(FX_INT32 index) const = 0;
    virtual FX_INT32		GetChildNodeIndex(IFDE_XMLNode *pNode) const = 0;
    virtual IFDE_XMLNode*	GetPath(FX_LPCWSTR pPath, FX_INT32 iLength = -1, FX_BOOL bQualifiedName = TRUE) const = 0;
    virtual FX_INT32		InsertChildNode(IFDE_XMLNode *pNode, FX_INT32 index = -1) = 0;
    virtual void			RemoveChildNode(IFDE_XMLNode *pNode) = 0;
    virtual void			DeleteChildren() = 0;
    enum NodeItem {Root = 0, Parent, FirstSibling, PriorSibling, NextSibling, LastSibling, FirstNeighbor, PriorNeighbor, NextNeighbor, LastNeighbor, FirstChild, LastChild};
    virtual IFDE_XMLNode*	GetNodeItem(NodeItem eItem) const = 0;
    virtual FX_INT32		GetNodeLevel() const = 0;
    virtual FX_BOOL			InsertNodeItem(IFDE_XMLNode::NodeItem eItem, IFDE_XMLNode *pNode) = 0;
    virtual	IFDE_XMLNode*	RemoveNodeItem(IFDE_XMLNode::NodeItem eItem) = 0;
    virtual IFDE_XMLNode*	Clone(FX_BOOL bRecursive) = 0;
    virtual	void			SaveXMLNode(IFX_Stream *pXMLStream) = 0;
};
class IFDE_XMLInstruction : public IFDE_XMLNode
{
public:
    static IFDE_XMLInstruction*	Create(const CFX_WideString &wsTarget);
    virtual void				GetTargetName(CFX_WideString &wsTarget) const = 0;
    virtual FX_INT32		CountAttributes() const = 0;
    virtual FX_BOOL			GetAttribute(FX_INT32 index, CFX_WideString &wsAttriName, CFX_WideString &wsAttriValue) const = 0;
    virtual FX_BOOL			HasAttribute(FX_LPCWSTR pwsAttriName) const = 0;
    virtual void			GetString(FX_LPCWSTR pwsAttriName, CFX_WideString &wsAttriValue, FX_LPCWSTR pwsDefValue = NULL) const = 0;
    virtual void			SetString(const CFX_WideString &wsAttriName, const CFX_WideString &wsAttriValue) = 0;
    virtual FX_INT32		GetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iDefValue = 0) const = 0;
    virtual void			SetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iAttriValue) = 0;
    virtual FX_FLOAT		GetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fDefValue = 0) const = 0;
    virtual void			SetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fAttriValue) = 0;
    virtual void			RemoveAttribute(FX_LPCWSTR pwsAttriName) = 0;
    virtual FX_INT32		CountData() const = 0;
    virtual FX_BOOL			GetData(FX_INT32 index, CFX_WideString &wsData) const = 0;
    virtual void			AppendData(const CFX_WideString &wsData) = 0;
    virtual void			RemoveData(FX_INT32 index) = 0;
};
class IFDE_XMLElement : public IFDE_XMLNode
{
public:
    static IFDE_XMLElement*	Create(const CFX_WideString &wsTag);
    virtual void			GetTagName(CFX_WideString &wsTag) const = 0;
    virtual void			GetLocalTagName(CFX_WideString &wsTag) const = 0;
    virtual void			GetNamespacePrefix(CFX_WideString &wsPrefix) const = 0;
    virtual void			GetNamespaceURI(CFX_WideString &wsNamespace) const = 0;
    virtual FX_INT32		CountAttributes() const = 0;
    virtual FX_BOOL			GetAttribute(FX_INT32 index, CFX_WideString &wsAttriName, CFX_WideString &wsAttriValue) const = 0;
    virtual FX_BOOL			HasAttribute(FX_LPCWSTR pwsAttriName) const = 0;
    virtual void			GetString(FX_LPCWSTR pwsAttriName, CFX_WideString &wsAttriValue, FX_LPCWSTR pwsDefValue = NULL) const = 0;
    virtual void			SetString(const CFX_WideString &wsAttriName, const CFX_WideString &wsAttriValue) = 0;
    virtual FX_INT32		GetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iDefValue = 0) const = 0;
    virtual void			SetInteger(FX_LPCWSTR pwsAttriName, FX_INT32 iAttriValue) = 0;
    virtual FX_FLOAT		GetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fDefValue = 0) const = 0;
    virtual void			SetFloat(FX_LPCWSTR pwsAttriName, FX_FLOAT fAttriValue) = 0;
    virtual void			RemoveAttribute(FX_LPCWSTR pwsAttriName) = 0;
    virtual void			GetTextData(CFX_WideString &wsText) const = 0;
    virtual void			SetTextData(const CFX_WideString &wsText) = 0;
};
class IFDE_XMLText : public IFDE_XMLNode
{
public:
    static IFDE_XMLText*	Create(const CFX_WideString &wsText);
    virtual void			GetText(CFX_WideString &wsText) const = 0;
    virtual void			SetText(const CFX_WideString &wsText) = 0;
};
class IFDE_XMLDeclaration : public IFDE_XMLNode
{
public:
};
class IFDE_XMLCharData : public IFDE_XMLDeclaration
{
public:
    static IFDE_XMLCharData*	Create(const CFX_WideString &wsCData);

    virtual void				GetCharData(CFX_WideString &wsCData) const = 0;
    virtual void				SetCharData(const CFX_WideString &wsCData) = 0;
};
typedef struct _FDE_XMLREADERHANDLER {
    FX_LPVOID pData;

    void	(*OnTagEnter)(_FDE_XMLREADERHANDLER *pThis, FDE_XMLNODETYPE eType, const CFX_WideString &wsTagName);
    void	(*OnTagBreak)(_FDE_XMLREADERHANDLER *pThis, const CFX_WideString &wsTagName);
    void	(*OnTagClose)(_FDE_XMLREADERHANDLER *pThis, const CFX_WideString &wsTagName);
    void	(*OnAttribute)(_FDE_XMLREADERHANDLER *pThis, const CFX_WideString &wsName, const CFX_WideString &wsValue);
    void	(*OnData)(_FDE_XMLREADERHANDLER *pThis, FDE_XMLNODETYPE eType, const CFX_WideString &wsValue);
} FDE_XMLREADERHANDLER, * FDE_LPXMLREADERHANDLER;
class IFDE_XMLDoc
{
public:
    static IFDE_XMLDoc*		Create();
    virtual void			Release() = 0;
    virtual FX_BOOL			LoadXML(IFX_Stream *pXMLStream, FX_INT32 iXMLPlaneSize = 8192, FX_INT32 iTextDataSize = 256, FDE_LPXMLREADERHANDLER pHandler = NULL) = 0;
    virtual FX_BOOL			LoadXML(IFDE_XMLParser *pXMLParser) = 0;
    virtual FX_INT32		DoLoad(IFX_Pause *pPause = NULL) = 0;
    virtual void			CloseXML() = 0;
    virtual IFDE_XMLNode*	GetRoot() const = 0;
    virtual void			SaveXML(IFX_Stream *pXMLStream = NULL, FX_BOOL bSaveBOM = TRUE) = 0;
    virtual void			SaveXMLNode(IFX_Stream *pXMLStream, IFDE_XMLNode *pNode) = 0;
};
class IFDE_XMLParser
{
public:
    virtual	void				Release() = 0;
    virtual FX_INT32			DoParser(IFX_Pause *pPause) = 0;
};
#define FDE_XMLSYNTAXSTATUS_None				0x00
#define FDE_XMLSYNTAXSTATUS_InstructionOpen		0x01
#define FDE_XMLSYNTAXSTATUS_InstructionClose	0x02
#define FDE_XMLSYNTAXSTATUS_ElementOpen			0x03
#define FDE_XMLSYNTAXSTATUS_ElementBreak		0x04
#define FDE_XMLSYNTAXSTATUS_ElementClose		0x05
#define FDE_XMLSYNTAXSTATUS_TargetName			0x06
#define FDE_XMLSYNTAXSTATUS_TagName				0x07
#define FDE_XMLSYNTAXSTATUS_AttriName			0x08
#define FDE_XMLSYNTAXSTATUS_AttriValue			0x09
#define FDE_XMLSYNTAXSTATUS_Text				0x0A
#define FDE_XMLSYNTAXSTATUS_CData				0x0B
#define FDE_XMLSYNTAXSTATUS_TargetData			0x0C
#define FDE_XMLSYNTAXSTATUS_Error				0xFE
#define FDE_XMLSYNTAXSTATUS_EOS					0xFF
class IFDE_XMLSyntaxParser
{
public:
    static IFDE_XMLSyntaxParser*	Create();
    virtual void			Release() = 0;
    virtual void			Init(IFX_Stream *pStream, FX_INT32 iXMLPlaneSize, FX_INT32 iTextDataSize = 256) = 0;
    virtual FX_DWORD		DoSyntaxParse() = 0;
    virtual FX_INT32		GetStatus() const = 0;
    virtual FX_INT32		GetCurrentPos() const = 0;
    virtual FX_FILESIZE		GetCurrentBinaryPos() const = 0;
    virtual FX_INT32		GetCurrentNodeNumber() const = 0;
    virtual FX_INT32		GetLastNodeNumber() const = 0;
    virtual void			GetTargetName(CFX_WideString &wsTarget) const = 0;
    virtual void			GetTagName(CFX_WideString &wsTag) const = 0;
    virtual void			GetAttributeName(CFX_WideString &wsAttriName) const = 0;
    virtual void			GetAttributeValue(CFX_WideString &wsAttriValue) const = 0;
    virtual void			GetTextData(CFX_WideString &wsText) const = 0;
    virtual void			GetTargetData(CFX_WideString &wsData) const = 0;
};
#endif
