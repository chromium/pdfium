// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "autoreflow.h"
#define approachto(a,b,c) (FXSYS_fabs((float)((a)-(b)))>(c) ? 0 : 1)
int FPDF_ProcessInterObj(const CPDF_PageObject* pPrevObj, const CPDF_PageObject* pObj)
{
    CFX_AffineMatrix matrix;
    FX_RECT PreRect = pPrevObj->GetBBox(&matrix);
    FX_RECT rect = pObj->GetBBox(&matrix);
    int flag = 0;
    if(PreRect.top > rect.bottom) {
        flag = 0;
    } else if(rect.top > PreRect.bottom) {
        flag = 1;
    } else if(PreRect.right < rect.left) {
        flag = 0;
    } else if(PreRect.left > rect.right) {
        flag = 1;
    } else if(pObj->m_Type != PDFPAGE_TEXT) {
        flag = 1;
    } else if(pPrevObj->m_Type != PDFPAGE_TEXT) {
        flag = 0;
    } else {
        if((PreRect.top < rect.top && PreRect.bottom > rect.bottom) ||
                (PreRect.top > rect.top && PreRect.bottom < rect.bottom)) {
            if(PreRect.left > rect.left) {
                flag = 1;
            } else {
                flag = 0;
            }
        } else {
            CPDF_TextObject* pPrevTextObj = (CPDF_TextObject* )pPrevObj;
            CPDF_TextObject* pTextObj = (CPDF_TextObject* )pObj;
            CPDF_TextObjectItem item, prevItem;
            pPrevTextObj->GetItemInfo(0, &prevItem);
            pTextObj->GetItemInfo(0, &item);
            CFX_AffineMatrix TextMatrix;
            pTextObj->GetTextMatrix(&TextMatrix);
            FX_FLOAT originX, originY, prevOriginX, preOriginY;
            TextMatrix.Transform(item.m_OriginX, item.m_OriginY, originX, originY);
            pPrevTextObj->GetTextMatrix(&TextMatrix);
            TextMatrix.Transform(prevItem.m_OriginX, prevItem.m_OriginY, prevOriginX, preOriginY);
            if(preOriginY > originY) {
                flag = 0;
            } else {
                flag = 1;
            }
        }
    }
    return flag;
}
void CPDF_AutoReflowLayoutProvider::Conver2AppreceOrder(const CPDF_PageObjects* pStreamOrderObjs, CPDF_PageObjects* pAppraceOrderObjs)
{
    FX_POSITION pos = pStreamOrderObjs->GetFirstObjectPosition();
    CFX_AffineMatrix matrix;
    while(pos) {
        CPDF_PageObject* pObj = pStreamOrderObjs->GetNextObject(pos);
        CFX_AffineMatrix matrix;
        if(pObj->m_Type != PDFPAGE_TEXT) {
            continue;
        }
        FX_POSITION pos1 = pAppraceOrderObjs->GetLastObjectPosition();
        while(pos1) {
            CPDF_PageObject* pTempObj = pAppraceOrderObjs->GetPrevObject(pos1);
            if(FPDF_ProcessInterObj(pObj, pTempObj) == 1) {
                if(!pos1) {
                    pos1 = pAppraceOrderObjs->GetFirstObjectPosition();
                } else {
                    pAppraceOrderObjs->GetNextObject(pos1);
                }
                break;
            }
        }
        pAppraceOrderObjs->InsertObject(pos1, pObj);
    }
    pos = pStreamOrderObjs->GetFirstObjectPosition();
    while(pos) {
        CPDF_PageObject* pObj = pStreamOrderObjs->GetNextObject(pos);
        if(pObj->m_Type != PDFPAGE_IMAGE) {
            continue;
        }
        FX_POSITION pos1 = pAppraceOrderObjs->GetLastObjectPosition();
        while(pos1) {
            CPDF_PageObject* pTempObj = pAppraceOrderObjs->GetPrevObject(pos1);
            if(FPDF_ProcessInterObj(pObj, pTempObj) == 1) {
                if(!pos1) {
                    pos1 = pAppraceOrderObjs->GetFirstObjectPosition();
                } else {
                    pAppraceOrderObjs->GetNextObject(pos1);
                }
                break;
            }
        }
        pAppraceOrderObjs->InsertObject(pos1, pObj);
    }
}
IPDF_LayoutProvider* IPDF_LayoutProvider::Create_LayoutProvider_AutoReflow(CPDF_PageObjects* pPage, FX_BOOL bReadOrder)
{
    return new CPDF_AutoReflowLayoutProvider(pPage, bReadOrder);
}
CPDF_AutoReflowElement::CPDF_AutoReflowElement(LayoutType layoutType , CPDF_AutoReflowElement* pParent)
{
    m_ElmType = layoutType;
    m_pParentElm = pParent;
    if(pParent) {
        pParent->m_ChildArray.Add(this);
    }
    m_SpaceBefore = 0;
}
CPDF_AutoReflowElement::~CPDF_AutoReflowElement()
{
    m_ChildArray.RemoveAll();
    m_ObjArray.RemoveAll();
}
int	CPDF_AutoReflowElement::CountAttrValues(LayoutAttr attr_type)
{
    return 1;
}
LayoutEnum CPDF_AutoReflowElement::GetEnumAttr(LayoutAttr attr_type, int index )
{
    return LayoutInvalid;
}
FX_FLOAT	CPDF_AutoReflowElement::GetNumberAttr(LayoutAttr attr_type, int index )
{
    switch (attr_type) {
        case LayoutSpaceBefore:
            return m_SpaceBefore;
        default:
            return 0;
    }
}
FX_COLORREF	CPDF_AutoReflowElement::GetColorAttr(LayoutAttr attr_type, int index )
{
    return 0;
}
#define WritingMode_UNKNOW	0
#define WritingMode_LRTB	1
#define WritingMode_RLTB	2
#define WritingMode_TBRL	3
CPDF_AutoReflowLayoutProvider::CPDF_AutoReflowLayoutProvider(CPDF_PageObjects* pPage, FX_BOOL bReadOrder)
{
    m_pPDFPage = (CPDF_Page*)pPage;
    FX_FLOAT width = m_pPDFPage->GetPageWidth();
    FX_FLOAT height = m_pPDFPage->GetPageHeight();
    m_pPDFPage->GetDisplayMatrix(m_PDFDisplayMatrix, 0, 0, (int)(m_pPDFPage->GetPageWidth()), (int)(m_pPDFPage->GetPageHeight()), 0);
    m_bReadOrder = bReadOrder;
    m_Status = LayoutReady;
    m_pRoot = NULL;
    m_pCurrElm = NULL;
    m_pPreObj = NULL;
    m_Step = 0;
    m_WritingMode = WritingMode_UNKNOW;
}
CPDF_AutoReflowLayoutProvider::~CPDF_AutoReflowLayoutProvider()
{
    m_pPDFPage = NULL;
    ReleaseElm(m_pRoot);
}
void CPDF_AutoReflowLayoutProvider::ReleaseElm(CPDF_AutoReflowElement*& pElm, FX_BOOL bReleaseChildren)
{
    if(bReleaseChildren) {
        int count = pElm->CountChildren();
        for(int i = 0; i < count; i++) {
            CPDF_AutoReflowElement* pChild = (CPDF_AutoReflowElement*)pElm->GetChild(i);
            ReleaseElm(pChild);
        }
    }
    delete pElm;
    pElm = NULL;
}
void CPDF_AutoReflowLayoutProvider::AddObjectArray(CPDF_AutoReflowElement* pElm, CFX_PtrList& ObjList)
{
    if(!pElm) {
        return;
    }
    FX_POSITION pos = ObjList.GetHeadPosition();
    while (pos) {
        pElm->m_ObjArray.Add((CPDF_PageObject*)ObjList.GetNext(pos));
    }
}
void CPDF_AutoReflowLayoutProvider::GenerateStructTree()
{
    if (m_Step < AUTOREFLOW_STEP_GENERATELINE) {
        GenerateLine(m_cellArray);
        if(m_cellArray.GetSize() == 0) {
            m_Status = LayoutError;
            return;
        }
        if(m_pPause && m_pPause->NeedToPauseNow()) {
            m_Step = AUTOREFLOW_STEP_GENERATELINE;
            m_Status = LayoutToBeContinued;
            return;
        }
    }
    if (m_Step < AUTOREFLOW_STEP_GENERATEParagraph) {
        GenerateParagraph(m_cellArray);
        if(m_pPause && m_pPause->NeedToPauseNow()) {
            m_Step = AUTOREFLOW_STEP_GENERATEParagraph;
            m_Status = LayoutToBeContinued;
            return;
        }
    }
    if (m_Step < AUTOREFLOW_STEP_CREATEELEMENT) {
        CreateElement();
        if(m_pPause && m_pPause->NeedToPauseNow()) {
            m_Step = AUTOREFLOW_STEP_CREATEELEMENT;
            m_Status = LayoutToBeContinued;
            return;
        }
    }
    if (m_Step < AUTOREFLOW_STEP_REMOVEDATA) {
        int count = m_cellArray.GetSize();
        for(int i = 0; i < count; i++) {
            CRF_CELL* pCell = (CRF_CELL*)m_cellArray.GetAt(i);
            if(pCell) {
                pCell->m_ObjList.RemoveAll();
                delete pCell;
            }
        }
        m_cellArray.RemoveAll();
        if(m_pPause && m_pPause->NeedToPauseNow()) {
            m_Step = AUTOREFLOW_STEP_REMOVEDATA;
            m_Status = LayoutToBeContinued;
            return;
        }
    }
    m_Step = AUTOREFLOW_STEP_REMOVEDATA;
    m_Status = LayoutFinished;
    return;
}
void CPDF_AutoReflowLayoutProvider::CreateElement()
{
    int count = m_cellArray.GetSize();
    CRF_CELL* plastCell = NULL;
    CRF_CELL* pCell = NULL;
    CRF_CELL* pNextCell = NULL;
    CPDF_AutoReflowElement* pParent = m_pRoot;
    CPDF_AutoReflowElement* pCurrElm = NULL;
    int i;
    for(i = 0; i < count; i++) {
        pCell = (CRF_CELL*)m_cellArray.GetAt(i);
        if(!pCell) {
            continue;
        }
        if(i < count - 1) {
            pNextCell = (CRF_CELL*)m_cellArray.GetAt(i + 1);
        } else {
            pNextCell = NULL;
        }
        pCurrElm = new CPDF_AutoReflowElement(LayoutParagraph, pParent);
        if(pCurrElm->GetType() == LayoutParagraph && plastCell) {
            int SpaceBefore = 0;
            if(pCell->m_CellWritingMode != plastCell->m_CellWritingMode ) {
                SpaceBefore = 20;
            } else if(pCell->m_CellWritingMode == WritingMode_LRTB) {
                SpaceBefore = plastCell->m_BBox.bottom - pCell->m_BBox.top;
            } else if(pCell->m_CellWritingMode == WritingMode_TBRL) {
                SpaceBefore = plastCell->m_BBox.left - pCell->m_BBox.right;
            }
            if(SpaceBefore > 0) {
                pCurrElm->m_SpaceBefore = SpaceBefore > 50 ? 50.0f : SpaceBefore;
            }
        }
        AddObjectArray(pCurrElm, pCell->m_ObjList);
        plastCell = pCell;
    }
}
void CPDF_AutoReflowLayoutProvider::GenerateParagraph(CFX_PtrArray& cellArray)
{
    int count = cellArray.GetSize();
    if(count <= 1) {
        return;
    }
    CRF_CELL* plastCell = (CRF_CELL*)cellArray.GetAt(0);
    if(plastCell->m_BBox.Height() > plastCell->m_BBox.Width()) {
        m_WritingMode = WritingMode_TBRL;
    } else {
        m_WritingMode = WritingMode_LRTB;
    }
    FX_BOOL bEnforce = FALSE;
    int i = 0;
    for(i = 1; i < count; i++) {
        CRF_CELL* pCell = (CRF_CELL*)cellArray.GetAt(i);
        if(!pCell) {
            continue;
        }
        int c = pCell->m_ObjList.GetCount();
        FX_BOOL bMerge = FALSE;
        FX_POSITION pos1 = plastCell->m_ObjList.GetTailPosition();
        CPDF_PageObject* pLastObj = (CPDF_PageObject*)plastCell->m_ObjList.GetPrev(pos1);
        pos1 = pCell->m_ObjList.GetHeadPosition();
        CPDF_PageObject* pCurObj = (CPDF_PageObject*)pCell->m_ObjList.GetNext(pos1);
        int WritingMode = GetRectEnd(pCell->m_BBox);
        if(pCell->m_CellWritingMode == WritingMode_UNKNOW) {
            if(pCell->m_BBox.Height() > pCell->m_BBox.Width()) {
                pCell->m_CellWritingMode = WritingMode_TBRL;
            } else {
                pCell->m_CellWritingMode = WritingMode_LRTB;
            }
        }
        WritingMode = pCell->m_CellWritingMode;
        if(WritingMode == WritingMode_LRTB && (m_Style.m_Language & LP_Lang_ChinesePRC || m_Style.m_Language & LP_Lang_ChineseTaiwan
                                               || m_Style.m_Language & LP_Lang_Japanese || m_Style.m_Language & LP_Lang_Korean)) {
            if(pCurObj->m_Type == PDFPAGE_TEXT) {
                CPDF_TextObject* pText;
                pText = (CPDF_TextObject*)pCurObj;
                if(pText->CountItems()) {
                    CPDF_TextObjectItem item;
                    pText->GetItemInfo(0, &item);
                    CFX_WideString str = pText->GetFont()->UnicodeFromCharCode(item.m_CharCode);
                    FX_WCHAR unicode = str.GetAt(0);
                    if(unicode == 32) {
                        plastCell = pCell;
                        bMerge = FALSE;
                        bEnforce = FALSE;
                        continue;
                    }
                }
            }
        }
        if(m_WritingMode == WritingMode) {
            if(bEnforce) {
                bMerge = FALSE;
                bEnforce = FALSE;
                if(pCurObj->m_Type == PDFPAGE_TEXT) {
                    CPDF_TextObject* pText;
                    pText = (CPDF_TextObject*)pCurObj;
                    if(pText->CountItems()) {
                        CPDF_TextObjectItem item;
                        pText->GetItemInfo(0, &item);
                        CFX_WideString str = pText->GetFont()->UnicodeFromCharCode(item.m_CharCode);
                        FX_WCHAR unicode = str.GetAt(0);
                        if(unicode > 96 && unicode < 123) {
                            bMerge = TRUE;
                        }
                    }
                } else {
                    CPDF_ImageObject* pImage = (CPDF_ImageObject*)pCurObj;
                    FX_RECT imageBBox = pImage->GetBBox(&m_PDFDisplayMatrix);
                    if(GetRectEnd(plastCell->m_BBox) - GetRectEnd(pCell->m_BBox) < GetRectWidth(imageBBox)) {
                        bMerge = TRUE;
                    }
                }
            } else {
                if(!approachto(GetRectStart(pCell->m_BBox), GetRectStart(plastCell->m_BBox), GetRectHeight(pCell->m_BBox) / 4)) {
                    if(approachto(GetRectStart(plastCell->m_BBox), GetRectStart(pCell->m_BBox), GetRectHeight(pCell->m_BBox) * 2.3) &&
                            GetRectStart(plastCell->m_BBox) - GetRectStart(pCell->m_BBox) > 0) {
                        if(pCurObj->m_Type == PDFPAGE_TEXT || pLastObj->m_Type == PDFPAGE_TEXT) {
                            CPDF_TextObject* pText;
                            if(pCurObj->m_Type == PDFPAGE_TEXT) {
                                pText = (CPDF_TextObject*)pCurObj;
                            } else {
                                pText = (CPDF_TextObject*)pLastObj;
                            }
                            CPDF_TextObjectItem item;
                            pText->GetItemInfo(0, &item);
                            CFX_WideString str = pText->GetFont()->UnicodeFromCharCode(item.m_CharCode);
                            FX_WCHAR unicode = str.GetAt(0);
                            if(unicode > 255) {
                                bMerge = TRUE;
                            }
                        }
                    }
                } else if(!approachto(GetRectEnd(pCell->m_BBox), GetRectEnd(plastCell->m_BBox), GetRectHeight(pCell->m_BBox) * 3)) {
                    FX_RECT rect = pLastObj->GetBBox(&m_PDFDisplayMatrix);
                    if(approachto(GetRectStart(pCell->m_BBox), GetRectStart(plastCell->m_BBox), GetRectHeight(pCell->m_BBox) / 4)) {
                        if(GetRectEnd(rect) - GetRectEnd(pCell->m_BBox) > 0) {
                            bMerge = TRUE;
                            bEnforce = TRUE;
                        } else if(GetRectEnd(rect) - GetRectEnd(pCell->m_BBox) <= 0 &&
                                  GetRectEnd(rect) - GetRectEnd(pCell->m_BBox) > GetRectHeight(pCell->m_BBox) * -3) {
                            if(pCurObj->m_Type == PDFPAGE_TEXT) {
                                CPDF_TextObject* pText = (CPDF_TextObject*)pCurObj;
                                CPDF_TextObjectItem item;
                                pText->GetItemInfo(0, &item);
                                CFX_WideString str = pText->GetFont()->UnicodeFromCharCode(item.m_CharCode);
                                FX_WCHAR unicode = str.GetAt(0);
                                if(unicode > 96 && unicode < 123) {
                                    bMerge = TRUE;
                                }
                            }
                        }
                    }
                } else {
                    bMerge = TRUE;
                }
            }
        } else {
            m_WritingMode = WritingMode;
            bEnforce = FALSE;
        }
        if(bMerge) {
            if(GetRectEnd(plastCell->m_BBox) - GetRectEnd(pCell->m_BBox) > 30) {
                bEnforce = TRUE;
            }
            FX_POSITION pos = pCell->m_ObjList.GetHeadPosition();
            while(pos) {
                plastCell->m_ObjList.AddTail(pCell->m_ObjList.GetNext(pos));
            }
            plastCell->m_BBox.Union(pCell->m_BBox);
            pCell->m_ObjList.RemoveAll();
            delete pCell;
            cellArray.RemoveAt(i);
            i--;
            count--;
        } else {
            plastCell = pCell;
        }
    }
}
void CPDF_AutoReflowLayoutProvider::ProcessObj(CFX_PtrArray& cellArray, CPDF_PageObject* pObj, CFX_AffineMatrix matrix)
{
}
int32_t CPDF_AutoReflowLayoutProvider::LogicPreObj(CPDF_PageObject* pObj)
{
    CPDF_PageObject* pPreObj = m_pPreObj;
    m_pPreObj = pObj;
    if(!pPreObj) {
        return 0;
    }
    if(pPreObj->m_Type != pObj->m_Type) {
        return 0;
    }
    CFX_FloatRect rcCurObj(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
    CFX_FloatRect rcPreObj(pPreObj->m_Left, pPreObj->m_Bottom, pPreObj->m_Right, pPreObj->m_Top);
    if(pObj->m_Type == PDFPAGE_IMAGE) {
        if(rcPreObj.Contains(rcCurObj)) {
            return 2;
        }
        if(rcCurObj.Contains(rcPreObj)) {
            return 2;
        }
        return 0;
    }
    if(pObj->m_Type == PDFPAGE_TEXT) {
        if(!((rcPreObj.bottom > rcCurObj.top) || (rcPreObj.top < rcCurObj.bottom))) {
            FX_FLOAT height = FX_MIN(rcPreObj.Height(), rcCurObj.Height());
            if((rcCurObj.left - rcPreObj.right) > height / 3) {
                return 3;
            }
        }
        if(FXSYS_fabs(rcPreObj.Width() - rcCurObj.Width()) >= 2 || FXSYS_fabs(rcPreObj.Height() - rcCurObj.Height()) >= 2 ) {
            return 0;
        }
        CPDF_TextObject* pPreTextObj = (CPDF_TextObject*)pPreObj;
        CPDF_TextObject* pCurTextObj = (CPDF_TextObject*)pObj;
        int nPreCount = pPreTextObj->CountItems();
        int nCurCount = pCurTextObj->CountItems();
        if (nPreCount != nCurCount) {
            return 0;
        }
        FX_BOOL bSame = TRUE;
        for (int i = 0; i < nPreCount; i++) {
            CPDF_TextObjectItem itemPer, itemCur;
            pPreTextObj->GetItemInfo(i, &itemPer);
            pCurTextObj->GetItemInfo(i, &itemCur);
            if (itemCur.m_CharCode != itemPer.m_CharCode) {
                return 0;
            }
            if (itemCur.m_OriginX != itemPer.m_OriginX) {
                bSame = FALSE;
            }
            if (itemCur.m_OriginY != itemPer.m_OriginY) {
                bSame = FALSE;
            }
        }
        if(rcPreObj.left == rcCurObj.left && rcPreObj.top == rcCurObj.top) {
            return 1;
        }
        if(FXSYS_fabs(rcPreObj.left - rcCurObj.left) < rcPreObj.Width() / 3
                && FXSYS_fabs(rcPreObj.top - rcCurObj.top) < rcPreObj.Height() / 3) {
            return 2;
        }
    }
    return 0;
}
void CPDF_AutoReflowLayoutProvider::GenerateLine(CFX_PtrArray& cellArray)
{
    CRF_CELL* pCell = NULL;
    CFX_AffineMatrix matrix;
    FX_POSITION pos = m_pPDFPage->GetFirstObjectPosition();
    if(!pos) {
        return;
    }
    FX_FLOAT PDFWidth = m_pPDFPage->GetPageWidth();
    FX_FLOAT PDFHeight = m_pPDFPage->GetPageHeight();
    m_pPDFPage->GetDisplayMatrix(m_PDFDisplayMatrix, 0, 0, (int)PDFWidth, (int)PDFHeight, 0);
    CPDF_PageObject* pPerObj = NULL;
    int a = 0;
    CFX_FloatRect pageBBox = m_pPDFPage->m_BBox;
    FX_FLOAT PrevX = 0 , PrevY = 0, PosX, PosY;
    while(pos) {
        CPDF_PageObject* pObj = m_pPDFPage->GetNextObject(pos);
        if(!pObj || pObj->m_Type == PDFPAGE_PATH) {
            continue;
        }
        int logic = LogicPreObj(pObj);
        if(logic == 2) {
            if(pCell) {
                pCell->m_ObjList.SetAt(pCell->m_ObjList.GetTailPosition(), pObj);
            }
            continue;
        }
        if (pObj->m_Type == PDFPAGE_TEXT) {
            CPDF_TextObject* pTextObj = (CPDF_TextObject*)pObj;
            int textmode = pTextObj->m_TextState.GetObject()->m_TextMode;
            if(m_Style.m_bIgnoreInvisibleText && pTextObj->m_TextState.GetObject()->m_TextMode == 3) {
                continue;
            }
            PosX = pTextObj->GetPosX();
            PosY = pTextObj->GetPosY();
            m_PDFDisplayMatrix.Transform(PosX, PosY);
        } else {
            PosX = 0;
            PosY = 0;
        }
        FX_BOOL bNewLine = TRUE;
        FX_RECT ObjBBox = pObj->GetBBox(&m_PDFDisplayMatrix);
        if(ObjBBox.left > PDFWidth || ObjBBox.right < 0 ||
                ObjBBox.bottom < 0 || ObjBBox.top > PDFHeight) {
            continue;
        }
        if(ObjBBox.IsEmpty()) {
            continue;
        }
        a++;
        if(!pCell) {
            bNewLine = TRUE;
            m_WritingMode = GetWritingMode(NULL, pObj);
        } else {
            int WritingMode = GetWritingMode(pPerObj, pObj);
            if(m_WritingMode == WritingMode || m_WritingMode == WritingMode_UNKNOW || WritingMode == WritingMode_UNKNOW) {
                if(WritingMode != WritingMode_UNKNOW) {
                    m_WritingMode = WritingMode;
                }
                if(m_WritingMode == WritingMode_TBRL) {
                    if(!(GetRectBottom(ObjBBox) > GetRectTop(pCell->m_BBox) ||
                            GetRectTop(ObjBBox) < GetRectBottom(pCell->m_BBox))) {
                        bNewLine = FALSE;
                    }
                } else {
                    if(!(GetRectBottom(ObjBBox) < GetRectTop(pCell->m_BBox) ||
                            GetRectTop(ObjBBox) > GetRectBottom(pCell->m_BBox))) {
                        bNewLine = FALSE;
                    }
                    if (pObj->m_Type == PDFPAGE_TEXT) {
                        if(FXSYS_fabs(PrevY - PosY) < 1 ) {
                            bNewLine = FALSE;
                        }
                    }
                }
            } else {
                m_WritingMode = WritingMode;
            }
        }
        pPerObj = pObj;
        if(bNewLine) {
            int c = pCell ? pCell->m_ObjList.GetCount() : 0;
            pCell = new CRF_CELL;
            pCell->m_CellWritingMode = m_WritingMode;
            pCell->m_BBox = ObjBBox;
            if(pObj->m_Type == PDFPAGE_TEXT) {
                FX_FLOAT x = ((CPDF_TextObject*)pObj)->GetPosX(), y = ((CPDF_TextObject*)pObj)->GetPosY();
                m_PDFDisplayMatrix.Transform(x, y);
                if(x < ObjBBox.left) {
                    pCell->m_BBox.left = (int)x;
                }
            }
            pCell->m_ObjList.AddTail(pObj);
            cellArray.Add(pCell);
        } else {
            pCell->m_ObjList.AddTail(pObj);
            pCell->m_BBox.Union(ObjBBox);
        }
        PrevX = PosX;
        PrevY = PosY;
    }
}
FX_FLOAT CPDF_AutoReflowLayoutProvider::GetLayoutOrderHeight(CPDF_PageObject* pCurObj)
{
    CFX_FloatRect rcCurObj(pCurObj->m_Left, pCurObj->m_Bottom, pCurObj->m_Right, pCurObj->m_Top);
    if (m_WritingMode == WritingMode_TBRL) {
        return rcCurObj.Width();
    }
    return rcCurObj.Height();
}
FX_FLOAT CPDF_AutoReflowLayoutProvider::GetLayoutOrderWidth(CPDF_PageObject* pCurObj)
{
    CFX_FloatRect rcCurObj(pCurObj->m_Left, pCurObj->m_Bottom, pCurObj->m_Right, pCurObj->m_Top);
    if (m_WritingMode == WritingMode_TBRL) {
        return rcCurObj.Height();
    }
    return rcCurObj.Width();
}
int CPDF_AutoReflowLayoutProvider:: GetRectWidth(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.Height();
    }
    return rect.Width();
}
int CPDF_AutoReflowLayoutProvider:: GetRectHeight(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.Width();
    }
    return rect.Height();
}
int CPDF_AutoReflowLayoutProvider:: GetRectStart(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.top;
    }
    return rect.left;
}
int CPDF_AutoReflowLayoutProvider:: GetRectEnd(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.bottom;
    }
    return rect.right;
}
int CPDF_AutoReflowLayoutProvider:: GetRectTop(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.right;
    }
    return rect.top;
}
int CPDF_AutoReflowLayoutProvider:: GetRectBottom(FX_RECT rect)
{
    if(m_WritingMode == WritingMode_TBRL) {
        return rect.left;
    }
    return rect.bottom;
}
int CPDF_AutoReflowLayoutProvider::GetWritingMode(CPDF_PageObject* pPreObj, CPDF_PageObject* pCurObj)
{
    CFX_FloatRect rcCurObj(pCurObj->m_Left, pCurObj->m_Bottom, pCurObj->m_Right, pCurObj->m_Top);
    if(pCurObj->m_Type == PDFPAGE_TEXT) {
        CPDF_TextObject* ptextObj = (CPDF_TextObject* )pCurObj;
        int count = ptextObj->CountItems();
        if(count > 1) {
            CPDF_TextObjectItem Item1, Item2;
            ptextObj->GetItemInfo(0, &Item1);
            ptextObj->GetItemInfo(count - 1, &Item2);
            if(Item2.m_CharCode == -1 && count > 2) {
                ptextObj->GetItemInfo(2, &Item2);
            }
            CFX_AffineMatrix textMatrix;
            ptextObj->GetTextMatrix(&textMatrix);
            textMatrix.Transform(Item1.m_OriginX, Item1.m_OriginY);
            textMatrix.Transform(Item2.m_OriginX, Item2.m_OriginY);
            FX_FLOAT dx = FXSYS_fabs(Item1.m_OriginX - Item2.m_OriginX);
            FX_FLOAT dy = FXSYS_fabs(Item1.m_OriginY - Item2.m_OriginY);
            return dx >= dy ? WritingMode_LRTB : WritingMode_TBRL;
        } else {
            if(m_WritingMode != WritingMode_UNKNOW) {
                return m_WritingMode;
            }
        }
    }
    if(pPreObj) {
        FX_FLOAT threshold = rcCurObj.Width() / 4;
        if(m_WritingMode == WritingMode_LRTB) {
            if(FXSYS_fabs(pPreObj->m_Bottom - pCurObj->m_Bottom) < threshold * 2
                    && FXSYS_fabs(pPreObj->m_Top - pCurObj->m_Top) < threshold * 2) {
                return m_WritingMode;
            }
            FX_FLOAT mid = (pCurObj->m_Bottom + pCurObj->m_Top) / 2;
            if(mid > pPreObj->m_Bottom && mid < pPreObj->m_Top && pCurObj->m_Right > pPreObj->m_Right) {
                return m_WritingMode;
            }
        } else if(m_WritingMode == WritingMode_TBRL) {
            if(FXSYS_fabs(pPreObj->m_Left - pCurObj->m_Left) < threshold * 2
                    && FXSYS_fabs(pPreObj->m_Right - pCurObj->m_Right) < threshold * 2) {
                return m_WritingMode;
            }
            FX_FLOAT mid = (pCurObj->m_Right + pCurObj->m_Left) / 2;
            if(mid > pPreObj->m_Left && mid < pPreObj->m_Right && pCurObj->m_Bottom < pPreObj->m_Bottom) {
                return m_WritingMode;
            }
        }
        if(FXSYS_fabs(pPreObj->m_Left - pCurObj->m_Left) < threshold &&
                FXSYS_fabs(pPreObj->m_Bottom - pCurObj->m_Bottom) > threshold * 2) {
            return WritingMode_TBRL;
        }
        if(FXSYS_fabs(pPreObj->m_Left - pCurObj->m_Left) > threshold &&
                FXSYS_fabs(pPreObj->m_Bottom - pCurObj->m_Bottom) < threshold * 2) {
            return WritingMode_LRTB;
        }
        int count = 0;
        if(pPreObj->m_Type == PDFPAGE_TEXT) {
            CPDF_TextObject* ptextObj = (CPDF_TextObject* )pCurObj;
            count = ptextObj->CountItems();
        }
        if(pPreObj->m_Type != PDFPAGE_TEXT || count == 1) {
            if(pCurObj->m_Left > pPreObj->m_Right) {
                FX_FLOAT mid = (pCurObj->m_Top + pCurObj->m_Bottom) / 2;
                if(mid < pPreObj->m_Top && mid > pPreObj->m_Bottom) {
                    return WritingMode_LRTB;
                }
            }
            if(pCurObj->m_Top < pPreObj->m_Bottom) {
                FX_FLOAT mid = (pCurObj->m_Left + pCurObj->m_Right) / 2;
                if(mid < pPreObj->m_Right && mid > pPreObj->m_Left) {
                    return WritingMode_TBRL;
                }
            }
        }
    }
    return WritingMode_UNKNOW;
}
LayoutStatus CPDF_AutoReflowLayoutProvider::StartLoad(IFX_Pause* pPause)
{
    m_pPause = pPause;
    m_pRoot = new CPDF_AutoReflowElement(LayoutDocument);
    m_Step = 0;
    return Continue();
}
LayoutStatus CPDF_AutoReflowLayoutProvider::Continue()
{
    GenerateStructTree();
    return m_Status;
}
int	CPDF_AutoReflowLayoutProvider::GetPosition()
{
    if(m_Step == 0) {
        return 0;
    } else {
        return m_Step * 100 / AUTOREFLOW_STEP_REMOVEDATA;
    }
}
FX_FLOAT CPDF_AutoReflowLayoutProvider::GetObjMinCell(CPDF_PageObject* pObj)
{
    if(!pObj) {
        return 0;
    }
    if(pObj->m_Type != PDFPAGE_TEXT) {
        CFX_AffineMatrix matrix;
        FX_RECT rect = pObj->GetBBox(&matrix);
        return (FX_FLOAT)(rect.Width());
    }
    CPDF_TextObject* pTextObj = (CPDF_TextObject* )pObj;
    int count = pTextObj->CountItems();
    for(int i = 0; i < count; i++) {
        CPDF_TextObjectItem Item;
        pTextObj->GetItemInfo(i, &Item);
        if(Item.m_CharCode == -1) {
            continue;
        }
        if((Item.m_CharCode > 47 && Item.m_CharCode < 58) || (Item.m_CharCode > 64 && Item.m_CharCode < 91)
                || (Item.m_CharCode > 96 && Item.m_CharCode < 123)) {
            continue;
        }
        if(Item.m_CharCode > 127 || (Item.m_CharCode > 32 && Item.m_CharCode < 35) || Item.m_CharCode == 37 ||
                (Item.m_CharCode > 38 && Item.m_CharCode < 42) || Item.m_CharCode == 44 || Item.m_CharCode == 46 ||
                Item.m_CharCode == 58 || Item.m_CharCode == 59 || Item.m_CharCode == 63 || Item.m_CharCode == 93) {
            if(i == count - 1) {
                CFX_AffineMatrix matrix;
                FX_RECT rect = pObj->GetBBox(&matrix);
                return (FX_FLOAT)(rect.Width());
            } else {
                pTextObj->GetItemInfo(i + 1, &Item);
                return Item.m_OriginX;
            }
        }
        return Item.m_OriginX;
    }
    CFX_AffineMatrix matrix;
    FX_RECT rect = pObj->GetBBox(&matrix);
    return (FX_FLOAT)(rect.Width());
}
