// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_REFLOW_AUTOREFLOW_H_
#define CORE_SRC_REFLOW_AUTOREFLOW_H_

#include "../../include/reflow/reflowengine.h"
#include "reflowedpage.h"

class CPDF_AutoReflowElement;
class CPDF_AutoReflowLayoutProvider;
typedef CFX_ArrayTemplate<CPDF_AutoReflowElement*> CAR_ElmPtrArray;
typedef CFX_ArrayTemplate<CPDF_PageObject*> CAR_ObjPtrArray;
class CRF_CELL 
{
public:
    CRF_CELL() { };
    ~CRF_CELL() { };
    CFX_PtrList		m_ObjList;
    int			m_CellWritingMode;
    FX_RECT		m_BBox;
};
class CPDF_AutoReflowElement : public IPDF_LayoutElement
{
public:
    CPDF_AutoReflowElement(LayoutType layoutType = LayoutUnknown , CPDF_AutoReflowElement* pParent = NULL) ;
    ~CPDF_AutoReflowElement();
    LayoutType GetType()
    {
        return m_ElmType;
    }
    void	GetRect(CFX_FloatRect& rcRect) {};

    int  CountAttrValues(LayoutAttr attr_type);
    LayoutEnum  GetEnumAttr(LayoutAttr attr_type, int index);
    FX_FLOAT GetNumberAttr(LayoutAttr attr_type, int index);
    FX_COLORREF GetColorAttr(LayoutAttr attr_type, int index);

    int  CountChildren()
    {
        return m_ChildArray.GetSize();
    }
    IPDF_LayoutElement* GetChild(int index)
    {
        return m_ChildArray.GetAt(index);
    }

    IPDF_LayoutElement* GetParent()
    {
        return m_pParentElm;
    }
    int		CountObjects()
    {
        return m_ObjArray.GetSize();
    }
    CPDF_PageObject*	GetObject(int index)
    {
        return m_ObjArray.GetAt(index);
    }
    CPDF_AutoReflowElement* m_pParentElm;
    LayoutType		m_ElmType;
    CAR_ElmPtrArray m_ChildArray;
    CAR_ObjPtrArray m_ObjArray;
    FX_FLOAT		m_SpaceBefore;
};
#define AUTOREFLOW_STEP_GENERATELINE		1
#define AUTOREFLOW_STEP_GENERATEParagraph	2
#define AUTOREFLOW_STEP_CREATEELEMENT		3
#define AUTOREFLOW_STEP_REMOVEDATA			4
class CPDF_AutoReflowLayoutProvider : public IPDF_LayoutProvider
{
public:
    CPDF_AutoReflowLayoutProvider(CPDF_PageObjects* pPage, FX_BOOL bReadOrder);
    ~CPDF_AutoReflowLayoutProvider();
    void	SetLayoutProviderStyle(LAYOUTPROVIDER_STYLE Style)
    {
        m_Style = Style;
    }
    LayoutStatus StartLoad(IFX_Pause* pPause = NULL);
    LayoutStatus Continue();
    int	 		GetPosition();
    IPDF_LayoutElement* GetRoot()
    {
        return m_pRoot;
    }
    FX_FLOAT GetObjMinCell(CPDF_PageObject* pObj);
    void Conver2AppreceOrder(const CPDF_PageObjects* pStreamOrderObjs, CPDF_PageObjects* pAppraceOrderObjs);
    void	ReleaseElm(CPDF_AutoReflowElement*& pElm, FX_BOOL bReleaseChildren = TRUE);
    void GenerateCell();
    void GenerateStructTree();
    void GenerateLine(CFX_PtrArray& cellArray);
    void GenerateParagraph(CFX_PtrArray& cellArray);
    void CreateElement();
    void AddObjectArray(CPDF_AutoReflowElement* pElm, CFX_PtrList& ObjList);
    FX_FLOAT GetLayoutOrderHeight(CPDF_PageObject* pCurObj);
    FX_FLOAT GetLayoutOrderWidth(CPDF_PageObject* pCurObj);
    int GetWritingMode(CPDF_PageObject* pPreObj, CPDF_PageObject* pCurObj);
    int GetRectStart(FX_RECT rect);
    int GetRectEnd(FX_RECT rect);
    int GetRectTop(FX_RECT rect);
    int GetRectBottom(FX_RECT rect);
    int GetRectHeight(FX_RECT rect);
    int GetRectWidth(FX_RECT rect);
    void ProcessObj(CFX_PtrArray& cellArray, CPDF_PageObject* pObj, CFX_AffineMatrix matrix);
    FX_INT32 LogicPreObj(CPDF_PageObject* pObj);

    CPDF_AutoReflowElement* m_pRoot;
    CPDF_AutoReflowElement* m_pCurrElm;
    CPDF_Page*	m_pPDFPage;
    IFX_Pause*	m_pPause;
    CFX_AffineMatrix m_PDFDisplayMatrix;
    CPDF_PageObject* m_pPreObj;
    LayoutStatus m_Status;
    int m_WritingMode;
    CFX_PtrArray m_CellArray;
    FX_BOOL		 m_bReadOrder;
    LAYOUTPROVIDER_STYLE m_Style;
    CFX_PtrArray m_cellArray;
    int			m_Step;
};

#endif  // CORE_SRC_REFLOW_AUTOREFLOW_H_
