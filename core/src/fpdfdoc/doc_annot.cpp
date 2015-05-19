// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../third_party/base/nonstd_unique_ptr.h"
#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fpdfapi/fpdf_pageobj.h"

CPDF_AnnotList::CPDF_AnnotList(CPDF_Page* pPage)
{
    ASSERT(pPage != NULL);
    m_pPageDict = pPage->m_pFormDict;
    if (m_pPageDict == NULL) {
        return;
    }
    m_pDocument = pPage->m_pDocument;
    CPDF_Array* pAnnots = m_pPageDict->GetArray("Annots");
    if (pAnnots == NULL) {
        return;
    }
    CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
    CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
    FX_BOOL bRegenerateAP = pAcroForm && pAcroForm->GetBoolean("NeedAppearances");
    for (FX_DWORD i = 0; i < pAnnots->GetCount(); i ++) {
        CPDF_Dictionary* pDict = (CPDF_Dictionary*)pAnnots->GetElementValue(i);
        if (pDict == NULL || pDict->GetType() != PDFOBJ_DICTIONARY) {
            continue;
        }
        FX_DWORD dwObjNum = pDict->GetObjNum();
        if (dwObjNum == 0) {
            dwObjNum = m_pDocument->AddIndirectObject(pDict);
            CPDF_Reference* pAction = CPDF_Reference::Create(m_pDocument, dwObjNum);
            if (pAction == NULL) {
                break;
            }
            pAnnots->InsertAt(i, pAction);
            pAnnots->RemoveAt(i + 1);
            pDict = pAnnots->GetDict(i);
        }
        CPDF_Annot* pAnnot = new CPDF_Annot(pDict);
        pAnnot->m_pList = this;
        m_AnnotList.Add(pAnnot);
        if (bRegenerateAP && pDict->GetConstString(FX_BSTRC("Subtype")) == FX_BSTRC("Widget"))
            if (CPDF_InterForm::UpdatingAPEnabled()) {
                FPDF_GenerateAP(m_pDocument, pDict);
            }
    }
}
CPDF_AnnotList::~CPDF_AnnotList()
{
    int i = 0;
    for (i = 0; i < m_AnnotList.GetSize(); i ++) {
        delete (CPDF_Annot*)m_AnnotList[i];
    }
    for (i = 0; i < m_Borders.GetSize(); ++i) {
        delete (CPDF_PageObjects*)m_Borders[i];
    }
}
void CPDF_AnnotList::DisplayPass(const CPDF_Page* pPage, CFX_RenderDevice* pDevice,
                                 CPDF_RenderContext* pContext, FX_BOOL bPrinting, CFX_AffineMatrix* pMatrix,
                                 FX_BOOL bWidgetPass, CPDF_RenderOptions* pOptions, FX_RECT* clip_rect)
{
    for (int i = 0; i < m_AnnotList.GetSize(); i ++) {
        CPDF_Annot* pAnnot = (CPDF_Annot*)m_AnnotList[i];
        FX_BOOL bWidget = pAnnot->GetSubType() == "Widget";
        if ((bWidgetPass && !bWidget) || (!bWidgetPass && bWidget)) {
            continue;
        }
        FX_DWORD annot_flags = pAnnot->GetFlags();
        if (annot_flags & ANNOTFLAG_HIDDEN) {
            continue;
        }
        if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0) {
            continue;
        }
        if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW)) {
            continue;
        }
        if (pOptions != NULL) {
            IPDF_OCContext* pOCContext = pOptions->m_pOCContext;
            CPDF_Dictionary* pAnnotDict = pAnnot->m_pAnnotDict;
            if (pOCContext != NULL && pAnnotDict != NULL &&
                    !pOCContext->CheckOCGVisible(pAnnotDict->GetDict(FX_BSTRC("OC")))) {
                continue;
            }
        }
        CPDF_Rect annot_rect_f;
        pAnnot->GetRect(annot_rect_f);
        CFX_Matrix matrix;
        matrix = *pMatrix;
        if (clip_rect) {
            annot_rect_f.Transform(&matrix);
            FX_RECT annot_rect = annot_rect_f.GetOutterRect();
            annot_rect.Intersect(*clip_rect);
            if (annot_rect.IsEmpty()) {
                continue;
            }
        }
        if (pContext) {
            pAnnot->DrawInContext(pPage, pContext, &matrix, CPDF_Annot::Normal);
        } else if (!pAnnot->DrawAppearance(pPage, pDevice, &matrix, CPDF_Annot::Normal, pOptions)) {
            pAnnot->DrawBorder(pDevice, &matrix, pOptions);
        }
    }
}
void CPDF_AnnotList::DisplayAnnots(const CPDF_Page* pPage, CFX_RenderDevice* pDevice,
                                   CFX_AffineMatrix* pUser2Device,
                                   FX_BOOL bShowWidget, CPDF_RenderOptions* pOptions)
{
    FX_RECT clip_rect;
    if (pDevice) {
        clip_rect = pDevice->GetClipBox();
    }
    FX_BOOL bPrinting = pDevice->GetDeviceClass() == FXDC_PRINTER || (pOptions && (pOptions->m_Flags & RENDER_PRINTPREVIEW));
    DisplayAnnots(pPage, pDevice, NULL, bPrinting, pUser2Device, bShowWidget ? 3 : 1, pOptions, &clip_rect);
}
void CPDF_AnnotList::DisplayAnnots(const CPDF_Page* pPage, CFX_RenderDevice* pDevice, CPDF_RenderContext* pContext,
                                   FX_BOOL bPrinting, CFX_AffineMatrix* pUser2Device, FX_DWORD dwAnnotFlags,
                                   CPDF_RenderOptions* pOptions, FX_RECT* pClipRect)
{
    if (dwAnnotFlags & 0x01) {
        DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, FALSE, pOptions, pClipRect);
    }
    if (dwAnnotFlags & 0x02) {
        DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, TRUE, pOptions, pClipRect);
    }
}
int CPDF_AnnotList::GetIndex(CPDF_Annot* pAnnot)
{
    for (int i = 0; i < m_AnnotList.GetSize(); i ++)
        if (m_AnnotList[i] == (FX_LPVOID)pAnnot) {
            return i;
        }
    return -1;
}
CPDF_Annot::CPDF_Annot(CPDF_Dictionary* pDict)
{
    m_pList = NULL;
    m_pAnnotDict = pDict;
}
CPDF_Annot::~CPDF_Annot()
{
    ClearCachedAP();
}
CPDF_Reference* CPDF_Annot::NewAnnotRef()
{
    if (m_pAnnotDict->GetObjNum() == 0) {
        m_pList->m_pDocument->AddIndirectObject(m_pAnnotDict);
    }
    return CPDF_Reference::Create(m_pList->m_pDocument, m_pAnnotDict->GetObjNum());
}
void CPDF_Annot::ClearCachedAP()
{
    FX_POSITION pos = m_APMap.GetStartPosition();
    while (pos) {
        void* pForm;
        void* pObjects;
        m_APMap.GetNextAssoc(pos, pForm, pObjects);
        delete (CPDF_PageObjects*)pObjects;
    }
    m_APMap.RemoveAll();
}
CFX_ByteString CPDF_Annot::GetSubType() const
{
    return m_pAnnotDict ? m_pAnnotDict->GetConstString(FX_BSTRC("Subtype")) : CFX_ByteStringC();
}
void CPDF_Annot::GetRect(CPDF_Rect& rect) const
{
    if (m_pAnnotDict == NULL) {
        return;
    }
    rect = m_pAnnotDict->GetRect("Rect");
    rect.Normalize();
}
CPDF_Stream* FPDFDOC_GetAnnotAP(CPDF_Dictionary* pAnnotDict, CPDF_Annot::AppearanceMode mode)
{
    CPDF_Dictionary* pAP = pAnnotDict->GetDict("AP");
    if (pAP == NULL) {
        return NULL;
    }
    const FX_CHAR* ap_entry = "N";
    if (mode == CPDF_Annot::Down) {
        ap_entry = "D";
    } else if (mode == CPDF_Annot::Rollover) {
        ap_entry = "R";
    }
    if (!pAP->KeyExist(ap_entry)) {
        ap_entry = "N";
    }
    CPDF_Object* psub = pAP->GetElementValue(ap_entry);
    if (psub == NULL) {
        return NULL;
    }
    CPDF_Stream* pStream = NULL;
    if (psub->GetType() == PDFOBJ_STREAM) {
        pStream = (CPDF_Stream*)psub;
    } else if (psub->GetType() == PDFOBJ_DICTIONARY) {
        CFX_ByteString as = pAnnotDict->GetString("AS");
        if (as.IsEmpty()) {
            CFX_ByteString value = pAnnotDict->GetString(FX_BSTRC("V"));
            if (value.IsEmpty()) {
                CPDF_Dictionary* pDict = pAnnotDict->GetDict(FX_BSTRC("Parent"));
                value = pDict ? pDict->GetString(FX_BSTRC("V")) : CFX_ByteString();
            }
            if (value.IsEmpty() || !((CPDF_Dictionary*)psub)->KeyExist(value)) {
                as = FX_BSTRC("Off");
            } else {
                as = value;
            }
        }
        pStream = ((CPDF_Dictionary*)psub)->GetStream(as);
    }
    return pStream;
}
CPDF_Form* CPDF_Annot::GetAPForm(const CPDF_Page* pPage, AppearanceMode mode)
{
    CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(m_pAnnotDict, mode);
    if (pStream == NULL) {
        return NULL;
    }
    CPDF_Form* pForm;
    if (m_APMap.Lookup(pStream, (void*&)pForm)) {
        return pForm;
    }
    pForm = new CPDF_Form(m_pList->m_pDocument, pPage->m_pResources, pStream);
    pForm->ParseContent(NULL, NULL, NULL, NULL);
    m_APMap.SetAt(pStream, pForm);
    return pForm;
}
static CPDF_Form* FPDFDOC_Annot_GetMatrix(const CPDF_Page* pPage, CPDF_Annot* pAnnot, CPDF_Annot::AppearanceMode mode, const CFX_AffineMatrix* pUser2Device, CFX_Matrix &matrix)
{
    CPDF_Form* pForm = pAnnot->GetAPForm(pPage, mode);
    if (!pForm) {
        return NULL;
    }
    CFX_FloatRect form_bbox = pForm->m_pFormDict->GetRect(FX_BSTRC("BBox"));
    CFX_Matrix form_matrix = pForm->m_pFormDict->GetMatrix(FX_BSTRC("Matrix"));
    form_matrix.TransformRect(form_bbox);
    CPDF_Rect arect;
    pAnnot->GetRect(arect);
    matrix.MatchRect(arect, form_bbox);
    matrix.Concat(*pUser2Device);
    return pForm;
}
FX_BOOL CPDF_Annot::DrawAppearance(const CPDF_Page* pPage, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pUser2Device,
                                   AppearanceMode mode, const CPDF_RenderOptions* pOptions)
{
    CFX_Matrix matrix;
    CPDF_Form* pForm = FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
    if (!pForm) {
        return FALSE;
    }
    CPDF_RenderContext context;
    context.Create((CPDF_Page*)pPage);
    context.DrawObjectList(pDevice, pForm, &matrix, pOptions);
    return TRUE;
}
FX_BOOL CPDF_Annot::DrawInContext(const CPDF_Page* pPage, const CPDF_RenderContext* pContext, const CFX_AffineMatrix* pUser2Device, AppearanceMode mode)
{
    CFX_Matrix matrix;
    CPDF_Form* pForm = FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
    if (!pForm) {
        return FALSE;
    }
    ((CPDF_RenderContext*)pContext)->AppendObjectList(pForm, &matrix);
    return TRUE;
}
CPDF_PageObject* CPDF_Annot::GetBorder(FX_BOOL bPrint, const CPDF_RenderOptions* pOptions)
{
    if (GetSubType() == "Popup") {
        return NULL;
    }
    FX_DWORD annot_flags = GetFlags();
    if (annot_flags & ANNOTFLAG_HIDDEN) {
        return NULL;
    }
    FX_BOOL bPrinting = bPrint || (pOptions && (pOptions->m_Flags & RENDER_PRINTPREVIEW));
    if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0) {
        return NULL;
    }
    if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW)) {
        return NULL;
    }
    CPDF_Dictionary* pBS = m_pAnnotDict->GetDict("BS");
    char style_char;
    FX_FLOAT width;
    CPDF_Array* pDashArray = NULL;
    if (pBS == NULL) {
        CPDF_Array* pBorderArray = m_pAnnotDict->GetArray("Border");
        style_char = 'S';
        if (pBorderArray) {
            width = pBorderArray->GetNumber(2);
            if (pBorderArray->GetCount() == 4) {
                pDashArray = pBorderArray->GetArray(3);
                if (pDashArray == NULL) {
                    return NULL;
                }
                style_char = 'D';
            }
        } else {
            width = 1;
        }
    } else {
        CFX_ByteString style = pBS->GetString("S");
        pDashArray = pBS->GetArray("D");
        style_char = style[1];
        width = pBS->GetNumber("W");
    }
    if (width <= 0) {
        return NULL;
    }
    CPDF_Array* pColor = m_pAnnotDict->GetArray("C");
    FX_DWORD argb = 0xff000000;
    if (pColor != NULL) {
        int R = (FX_INT32)(pColor->GetNumber(0) * 255);
        int G = (FX_INT32)(pColor->GetNumber(1) * 255);
        int B = (FX_INT32)(pColor->GetNumber(2) * 255);
        argb = ArgbEncode(0xff, R, G, B);
    }
    nonstd::unique_ptr<CPDF_PathObject> pPathObject(new CPDF_PathObject());
    CPDF_GraphStateData *pGraphState = pPathObject->m_GraphState.GetModify();
    if (!pGraphState) {
        return NULL;
    }
    pGraphState->m_LineWidth = width;
    CPDF_ColorStateData *pColorData = pPathObject->m_ColorState.GetModify();
    if (!pColorData) {
        return NULL;
    }
    pColorData->m_StrokeRGB = argb;
    pPathObject->m_bStroke = TRUE;
    pPathObject->m_FillType = 0;
    if (style_char == 'D') {
        if (pDashArray) {
            FX_DWORD dash_count = pDashArray->GetCount();
            if (dash_count % 2) {
                dash_count ++;
            }
            pGraphState->m_DashArray = FX_Alloc(FX_FLOAT, dash_count);
            pGraphState->m_DashCount = dash_count;
            FX_DWORD i;
            for (i = 0; i < pDashArray->GetCount(); i ++) {
                pGraphState->m_DashArray[i] = pDashArray->GetNumber(i);
            }
            if (i < dash_count) {
                pGraphState->m_DashArray[i] = pGraphState->m_DashArray[i - 1];
            }
        } else {
            pGraphState->m_DashArray = FX_Alloc(FX_FLOAT, 2);
            pGraphState->m_DashCount = 2;
            pGraphState->m_DashArray[0] = pGraphState->m_DashArray[1] = 3 * 1.0f;
        }
    }
    CFX_FloatRect rect;
    GetRect(rect);
    width /= 2;
    CPDF_PathData *pPathData = pPathObject->m_Path.GetModify();
    if (pPathData) {
        pPathData->AppendRect(rect.left + width, rect.bottom + width, rect.right - width, rect.top - width);
    }
    pPathObject->CalcBoundingBox();
    return pPathObject.release();
}
void CPDF_Annot::DrawBorder(CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pUser2Device, const CPDF_RenderOptions* pOptions)
{
    if (GetSubType() == "Popup") {
        return;
    }
    FX_DWORD annot_flags = GetFlags();
    if (annot_flags & ANNOTFLAG_HIDDEN) {
        return;
    }
    FX_BOOL bPrinting = pDevice->GetDeviceClass() == FXDC_PRINTER || (pOptions && (pOptions->m_Flags & RENDER_PRINTPREVIEW));
    if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0) {
        return;
    }
    if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW)) {
        return;
    }
    CPDF_Dictionary* pBS = m_pAnnotDict->GetDict("BS");
    char style_char;
    FX_FLOAT width;
    CPDF_Array* pDashArray = NULL;
    if (pBS == NULL) {
        CPDF_Array* pBorderArray = m_pAnnotDict->GetArray("Border");
        style_char = 'S';
        if (pBorderArray) {
            width = pBorderArray->GetNumber(2);
            if (pBorderArray->GetCount() == 4) {
                pDashArray = pBorderArray->GetArray(3);
                if (pDashArray == NULL) {
                    return;
                }
                int nLen = pDashArray->GetCount();
                int i = 0;
                for (; i < nLen; ++i) {
                    CPDF_Object*pObj = pDashArray->GetElementValue(i);
                    if (pObj && pObj->GetInteger()) {
                        break;
                    }
                }
                if (i == nLen) {
                    return;
                }
                style_char = 'D';
            }
        } else {
            width = 1;
        }
    } else {
        CFX_ByteString style = pBS->GetString("S");
        pDashArray = pBS->GetArray("D");
        style_char = style[1];
        width = pBS->GetNumber("W");
    }
    if (width <= 0) {
        return;
    }
    CPDF_Array* pColor = m_pAnnotDict->GetArray("C");
    FX_DWORD argb = 0xff000000;
    if (pColor != NULL) {
        int R = (FX_INT32)(pColor->GetNumber(0) * 255);
        int G = (FX_INT32)(pColor->GetNumber(1) * 255);
        int B = (FX_INT32)(pColor->GetNumber(2) * 255);
        argb = ArgbEncode(0xff, R, G, B);
    }
    CPDF_GraphStateData graph_state;
    graph_state.m_LineWidth = width;
    if (style_char == 'D') {
        if (pDashArray) {
            FX_DWORD dash_count = pDashArray->GetCount();
            if (dash_count % 2) {
                dash_count ++;
            }
            graph_state.m_DashArray = FX_Alloc(FX_FLOAT, dash_count);
            graph_state.m_DashCount = dash_count;
            FX_DWORD i;
            for (i = 0; i < pDashArray->GetCount(); i ++) {
                graph_state.m_DashArray[i] = pDashArray->GetNumber(i);
            }
            if (i < dash_count) {
                graph_state.m_DashArray[i] = graph_state.m_DashArray[i - 1];
            }
        } else {
            graph_state.m_DashArray = FX_Alloc(FX_FLOAT, 2);
            graph_state.m_DashCount = 2;
            graph_state.m_DashArray[0] = graph_state.m_DashArray[1] = 3 * 1.0f;
        }
    }
    CFX_FloatRect rect;
    GetRect(rect);
    CPDF_PathData path;
    width /= 2;
    path.AppendRect(rect.left + width, rect.bottom + width, rect.right - width, rect.top - width);
    int fill_type = 0;
    if (pOptions && (pOptions->m_Flags & RENDER_NOPATHSMOOTH)) {
        fill_type |= FXFILL_NOPATHSMOOTH;
    }
    pDevice->DrawPath(&path, pUser2Device, &graph_state, argb, argb, fill_type);
}
int CPDF_Annot::CountIRTNotes()
{
    int count = 0;
    for (int i = 0; i < m_pList->Count(); i ++) {
        CPDF_Annot* pAnnot = m_pList->GetAt(i);
        if (pAnnot == NULL) {
            continue;
        }
        CPDF_Dictionary* pIRT = pAnnot->m_pAnnotDict->GetDict("IRT");
        if (pIRT != m_pAnnotDict) {
            continue;
        }
        count ++;
    }
    return count;
}
CPDF_Annot* CPDF_Annot::GetIRTNote(int index)
{
    int count = 0;
    for (int i = 0; i < m_pList->Count(); i ++) {
        CPDF_Annot* pAnnot = m_pList->GetAt(i);
        if (pAnnot == NULL) {
            continue;
        }
        CPDF_Dictionary* pIRT = pAnnot->m_pAnnotDict->GetDict("IRT");
        if (pIRT != m_pAnnotDict) {
            continue;
        }
        if (count == index) {
            return pAnnot;
        }
        count ++;
    }
    return NULL;
}
