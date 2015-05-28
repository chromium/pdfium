// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXCRT_FX_XML_H_
#define CORE_INCLUDE_FXCRT_FX_XML_H_

#include "fx_basic.h"

class CXML_AttrItem 
{
public:
    CFX_ByteString	m_QSpaceName;
    CFX_ByteString	m_AttrName;
    CFX_WideString	m_Value;
};
class CXML_AttrMap 
{
public:
    CXML_AttrMap()
    {
        m_pMap = NULL;
    }
    ~CXML_AttrMap()
    {
        RemoveAll();
    }
    const CFX_WideString*	Lookup(FX_BSTR space, FX_BSTR name) const;
    void					SetAt(FX_BSTR space, FX_BSTR name, FX_WSTR value);
    void					RemoveAt(FX_BSTR space, FX_BSTR name);
    void					RemoveAll();
    int						GetSize() const;
    CXML_AttrItem&			GetAt(int index) const;
    CFX_ObjectArray<CXML_AttrItem>*	m_pMap;
};
class CXML_Content 
{
public:
    CXML_Content() : m_bCDATA(FALSE), m_Content() {}
    void	Set(FX_BOOL bCDATA, FX_WSTR content)
    {
        m_bCDATA = bCDATA;
        m_Content = content;
    }
    FX_BOOL			m_bCDATA;
    CFX_WideString	m_Content;
};
class CXML_Element 
{
public:
    static CXML_Element*	Parse(const void* pBuffer, size_t size, FX_BOOL bSaveSpaceChars = FALSE, FX_FILESIZE* pParsedSize = NULL);
    static CXML_Element*	Parse(IFX_FileRead *pFile, FX_BOOL bSaveSpaceChars = FALSE, FX_FILESIZE* pParsedSize = NULL);
    static CXML_Element*	Parse(IFX_BufferRead *pBuffer, FX_BOOL bSaveSpaceChars = FALSE, FX_FILESIZE* pParsedSize = NULL);
    CXML_Element(FX_BSTR qSpace, FX_BSTR tagName);
    CXML_Element(FX_BSTR qTagName);
    CXML_Element();

    ~CXML_Element();

    void	Empty();



    CFX_ByteString			GetTagName(FX_BOOL bQualified = FALSE) const;

    CFX_ByteString			GetNamespace(FX_BOOL bQualified = FALSE) const;

    CFX_ByteString			GetNamespaceURI(FX_BSTR qName) const;

    CXML_Element*			GetParent() const
    {
        return m_pParent;
    }

    FX_DWORD				CountAttrs() const
    {
        return m_AttrMap.GetSize();
    }

    void					GetAttrByIndex(int index, CFX_ByteString &space, CFX_ByteString &name, CFX_WideString &value) const;

    FX_BOOL					HasAttr(FX_BSTR qName) const;

    FX_BOOL					GetAttrValue(FX_BSTR name, CFX_WideString& attribute) const;
    CFX_WideString			GetAttrValue(FX_BSTR name) const
    {
        CFX_WideString attr;
        GetAttrValue(name, attr);
        return attr;
    }

    FX_BOOL					GetAttrValue(FX_BSTR space, FX_BSTR name, CFX_WideString& attribute) const;
    CFX_WideString			GetAttrValue(FX_BSTR space, FX_BSTR name) const
    {
        CFX_WideString attr;
        GetAttrValue(space, name, attr);
        return attr;
    }

    FX_BOOL					GetAttrInteger(FX_BSTR name, int& attribute) const;
    int						GetAttrInteger(FX_BSTR name) const
    {
        int attr = 0;
        GetAttrInteger(name, attr);
        return attr;
    }

    FX_BOOL					GetAttrInteger(FX_BSTR space, FX_BSTR name, int& attribute) const;
    int						GetAttrInteger(FX_BSTR space, FX_BSTR name) const
    {
        int attr = 0;
        GetAttrInteger(space, name, attr);
        return attr;
    }

    FX_BOOL					GetAttrFloat(FX_BSTR name, FX_FLOAT& attribute) const;
    FX_FLOAT				GetAttrFloat(FX_BSTR name) const
    {
        FX_FLOAT attr = 0;
        GetAttrFloat(name, attr);
        return attr;
    }

    FX_BOOL					GetAttrFloat(FX_BSTR space, FX_BSTR name, FX_FLOAT& attribute) const;
    FX_FLOAT				GetAttrFloat(FX_BSTR space, FX_BSTR name) const
    {
        FX_FLOAT attr = 0;
        GetAttrFloat(space, name, attr);
        return attr;
    }

    FX_DWORD				CountChildren() const;

    enum ChildType { Invalid, Element, Content};

    ChildType				GetChildType(FX_DWORD index) const;

    CFX_WideString			GetContent(FX_DWORD index) const;

    CXML_Element*			GetElement(FX_DWORD index) const;

    CXML_Element*			GetElement(FX_BSTR space, FX_BSTR tag) const
    {
        return GetElement(space, tag, 0);
    }

    FX_DWORD				CountElements(FX_BSTR space, FX_BSTR tag) const;

    CXML_Element*			GetElement(FX_BSTR space, FX_BSTR tag, int index) const;

    FX_DWORD				FindElement(CXML_Element *pChild) const;




    void					SetTag(FX_BSTR qSpace, FX_BSTR tagname);

    void					SetTag(FX_BSTR qTagName);

    void					RemoveChildren();

    void					RemoveChild(FX_DWORD index);


protected:

    CXML_Element*			m_pParent;
    CFX_ByteString			m_QSpaceName;
    CFX_ByteString			m_TagName;

    CXML_AttrMap			m_AttrMap;

    CFX_PtrArray			m_Children;
    friend class CXML_Parser;
    friend class CXML_Composer;
};

#endif  // CORE_INCLUDE_FXCRT_FX_XML_H_
