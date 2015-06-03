// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFDOC_FPDF_TAGGED_H_
#define CORE_INCLUDE_FPDFDOC_FPDF_TAGGED_H_

class CPDF_Document;
class CPDF_Page;
class CPDF_StructElement;
class CPDF_StructTree;
class IPDF_ReflowedPage;
struct CPDF_StructKid;

class CPDF_StructTree 
{
public:

    static CPDF_StructTree* LoadDoc(const CPDF_Document* pDoc);

    static CPDF_StructTree* LoadPage(const CPDF_Document* pDoc, const CPDF_Dictionary* pPageDict);

    virtual ~CPDF_StructTree() {}

    virtual int			CountTopElements() const = 0;

    virtual CPDF_StructElement*	GetTopElement(int i) const = 0;
};
struct CPDF_StructKid {
    enum {
        Invalid,
        Element,
        PageContent,
        StreamContent,
        Object
    } m_Type;

    union {
        struct {

            CPDF_StructElement*	m_pElement;

            CPDF_Dictionary*	m_pDict;
        } m_Element;
        struct {

            FX_DWORD			m_PageObjNum;

            FX_DWORD			m_ContentId;
        } m_PageContent;
        struct {

            FX_DWORD			m_PageObjNum;

            FX_DWORD			m_ContentId;

            FX_DWORD			m_RefObjNum;
        } m_StreamContent;
        struct {

            FX_DWORD			m_PageObjNum;

            FX_DWORD			m_RefObjNum;
        } m_Object;
    };
};
class CPDF_StructElement 
{
public:
    virtual ~CPDF_StructElement() { }

    virtual CPDF_StructTree*	GetTree() const = 0;

    virtual const CFX_ByteString&	GetType() const = 0;

    virtual CPDF_StructElement*	GetParent() const = 0;

    virtual CPDF_Dictionary *	GetDict() const = 0;

    virtual int					CountKids() const = 0;

    virtual const CPDF_StructKid&	GetKid(int index) const = 0;

    virtual CFX_PtrArray*		GetObjectArray() = 0;

    virtual CPDF_Object*		GetAttr(FX_BSTR owner, FX_BSTR name, FX_BOOL bInheritable = FALSE, FX_FLOAT fLevel = 0.0F) = 0;



    virtual CFX_ByteString		GetName(FX_BSTR owner, FX_BSTR name, FX_BSTR default_value, FX_BOOL bInheritable = FALSE, int subindex = -1) = 0;

    virtual FX_ARGB				GetColor(FX_BSTR owner, FX_BSTR name, FX_ARGB default_value, FX_BOOL bInheritable = FALSE, int subindex = -1) = 0;

    virtual FX_FLOAT			GetNumber(FX_BSTR owner, FX_BSTR name, FX_FLOAT default_value, FX_BOOL bInheritable = FALSE, int subindex = -1) = 0;

    virtual int					GetInteger(FX_BSTR owner, FX_BSTR name, int default_value, FX_BOOL bInheritable = FALSE, int subindex = -1) = 0;

};

#endif  // CORE_INCLUDE_FPDFDOC_FPDF_TAGGED_H_
