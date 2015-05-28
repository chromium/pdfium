// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_REFLOW_LAYOUTPROVIDER_TAGGEDPDF_H_
#define CORE_SRC_REFLOW_LAYOUTPROVIDER_TAGGEDPDF_H_

#include "../../include/reflow/reflowengine.h"

class CPDF_LayoutElement : public IPDF_LayoutElement
{
public:
    CPDF_LayoutElement();
    ~CPDF_LayoutElement();

    LayoutType	GetType();
    void	GetRect(CFX_FloatRect& rcRect) {};

    int		CountAttrValues(LayoutAttr attr_type);

    LayoutEnum	GetEnumAttr(LayoutAttr attr_type, int index);
    FX_FLOAT	GetNumberAttr(LayoutAttr attr_type, int index);
    FX_COLORREF	GetColorAttr(LayoutAttr attr_type, int index);

    int		CountChildren();

    IPDF_LayoutElement* GetChild(int index);

    IPDF_LayoutElement* GetParent();

    int	CountObjects();
    CPDF_PageObject*	GetObject(int index);
    FX_BOOL AddObject(CPDF_PageObject* pObj);
    CPDF_StructElement* m_pTaggedElement;
    CPDF_LayoutElement* m_pParentElement;
    CFX_PtrArray	m_ChildArray;
    LayoutType ConvertLayoutType(FX_BSTR name);
    CFX_ByteStringC ConvertLayoutType(LayoutType type);
    CFX_ByteStringC ConvertLayoutAttr(LayoutAttr attr);
    LayoutEnum ConvertLayoutEnum(CFX_ByteStringC Enum);

protected:
    FX_BOOL		IsInheritable(LayoutAttr attr_type);
    CFX_ByteStringC GetAttrOwner(LayoutAttr attr_type);
    CFX_ByteStringC GetDefaultNameValue(LayoutAttr attr_type);
    FX_FLOAT		GetDefaultFloatValue(LayoutAttr attr_type);
    FX_COLORREF		GetDefaultColorValue(LayoutAttr attr_type);
    CFX_PtrArray	m_ObjArray;
};
class CPDF_LayoutProvider_TaggedPDF : public IPDF_LayoutProvider
{
public:
    CPDF_LayoutProvider_TaggedPDF();
    ~CPDF_LayoutProvider_TaggedPDF();
    void			SetLayoutProviderStyle(LAYOUTPROVIDER_STYLE style) {};

    void	Init(CPDF_PageObjects* pPage)
    {
        m_pPage = pPage;
        m_Status = LayoutReady;
    };

    LayoutStatus	StartLoad(IFX_Pause* pPause = NULL);
    LayoutStatus	Continue();
    int		 		GetPosition();

    IPDF_LayoutElement* GetRoot()
    {
        return m_pRoot;
    };

protected:
    void ProcessElement(CPDF_LayoutElement*pParent, CPDF_StructElement* pTaggedElement);
    LayoutStatus	m_Status;
    CPDF_StructElement* m_pCurTaggedElement;
    CPDF_LayoutElement* m_pRoot;
    IFX_Pause*			m_pPause;
    CPDF_PageObjects*	m_pPage;
    CPDF_StructTree*	m_pPageTree;
    int					m_TopElementIndex;
};

#endif  // CORE_SRC_REFLOW_LAYOUTPROVIDER_TAGGEDPDF_H_
