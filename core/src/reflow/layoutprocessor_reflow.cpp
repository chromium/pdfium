// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/reflow/reflowengine.h"
#include "reflowedpage.h"
#include "layoutprovider_taggedpdf.h"
IPDF_LayoutProcessor* IPDF_LayoutProcessor::Create_LayoutProcessor_Reflow(FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, void* pReflowedPage, int flags, FX_FLOAT lineSpace )
{
    if(pReflowedPage == NULL || fWidth <= 20) {
        return NULL;
    }
    CPDF_LayoutProcessor_Reflow* pReflowEngine = new CPDF_LayoutProcessor_Reflow();
    pReflowEngine->Init(TopIndent, fWidth, fHeight, (CPDF_ReflowedPage*)pReflowedPage, flags, lineSpace);
    return pReflowEngine;
}
CPDF_LayoutProcessor_Reflow::CPDF_LayoutProcessor_Reflow()
{
    m_pPause = NULL;
    m_pLayoutElement = NULL;
    m_fRefWidth = 0;
    m_fRefWidth = 0;
    m_fCurrLineWidth = 0;
    m_fCurrLineHeight = 0;
    m_bIllustration = FALSE;
    m_pPreObj = NULL;
    m_pCurrLine = new CRF_DataPtrArray(50);
    m_pTempLine = new CRF_DataPtrArray(50);
    m_StartIndent = 0;
    m_PausePosition = 0;
}
CPDF_LayoutProcessor_Reflow::~CPDF_LayoutProcessor_Reflow()
{
    if (m_pCurrLine) {
        m_pCurrLine->RemoveAll();
        delete m_pCurrLine;
    }
    m_pCurrLine = NULL;
    if (m_pTempLine) {
        m_pTempLine->RemoveAll();
        delete m_pTempLine;
    }
    m_pTempLine = NULL;
}
void CPDF_LayoutProcessor_Reflow::Init(FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, CPDF_ReflowedPage* pReflowedPage, int flags, FX_FLOAT lineSpace)
{
    m_pLayoutElement = NULL;
    m_TopIndent = TopIndent;
    m_Status = LayoutReady;
    m_flags = flags;
    m_pReflowedPage = pReflowedPage;
    m_fScreenHeight = fHeight;
    m_fRefWidth = fWidth;
    m_fCurrLineHeight = 0;
    m_fCurrLineWidth = 0;
    m_fLineSpace = lineSpace;
    pReflowedPage->m_PageWidth = fWidth;
    pReflowedPage->m_PageHeight = TopIndent;
}
void CPDF_LayoutProcessor_Reflow::FitPageMode()
{
    if(m_flags & RF_PARSER_PAGEMODE && m_fScreenHeight > 20) {
        float fitPageHeight = m_fScreenHeight;
        CPDF_ReflowedPage* pRFPage = m_pReflowedPage;
        int count = pRFPage->m_pReflowed->GetSize();
        CFX_WordArray dy;
        dy.Add(0);
        int pos = 0;
        int screenCount = 1;
        FX_FLOAT h = pRFPage->GetPageHeight();
        while (h > screenCount * fitPageHeight) {
            FX_FLOAT tempPageHeight = screenCount * fitPageHeight;
            int j = 0;
            FX_FLOAT tempDy = 0;
            for(int i = 0; i < count; i++) {
                CRF_Data* pData = (*pRFPage->m_pReflowed)[i];
                FX_FLOAT posY;
                posY = pData->m_PosY;
                if(FXSYS_fabs(posY) > tempPageHeight &&
                        FXSYS_fabs(posY + pData->m_Height) < tempPageHeight) {
                    if(j == 0) {
                        j = i;
                    }
                    if(pData->m_Height > fitPageHeight) {
                        FX_FLOAT zoom;
                        FX_FLOAT spaceh = screenCount * fitPageHeight + posY + pData->m_Height;
                        if(spaceh < fitPageHeight / 3 * 2) {
                            spaceh = fitPageHeight;
                        }
                        zoom = spaceh / pData->m_Height;
                        tempDy = spaceh - pData->m_Height;
                        pData->m_Height = spaceh;
                        pData->m_Width *= zoom;
                        break;
                    }
                    FX_FLOAT dy = pData->m_PosY + pData->m_Height + tempPageHeight;
                    if(dy > tempDy) {
                        tempDy = dy;
                    }
                } else if(FXSYS_fabs(posY + pData->m_Height) > tempPageHeight) {
                    break;
                }
            }
            for(; j < count; j++) {
                CRF_Data* pData = (*pRFPage->m_pReflowed)[j];
                FX_FLOAT posY;
                posY = pData->m_PosY;
                if(FXSYS_fabs(posY) > tempPageHeight ) {
                    pData->m_PosY -= tempDy;
                }
                if(pData->m_Height >= fitPageHeight) {
                    pData->m_Height = fitPageHeight - 1;
                    if(pData->GetType() == CRF_Data::Text) {
                        CRF_CharData* pCharData = (CRF_CharData*)pData;
                        pCharData->m_pCharState->m_fFontSize = pData->m_Height;
                    }
                }
            }
            pRFPage->m_PageHeight += tempDy;
            h += tempDy;
            screenCount++;
        }
    }
}
LayoutStatus CPDF_LayoutProcessor_Reflow::StartProcess(IPDF_LayoutElement* pElement, IFX_Pause* pPause, const CFX_AffineMatrix* pPDFMatrix)
{
    if(!pElement) {
        return LayoutError;
    }
    m_pPause = pPause;
    m_PDFMatrix = *pPDFMatrix;
    m_pRootElement = pElement;
    ProcessElement(m_pRootElement, m_fRefWidth);
    if(m_Status == LayoutToBeContinued) {
        return LayoutToBeContinued;
    }
    m_Status = LayoutFinished;
    FitPageMode();
    return LayoutFinished;
}
LayoutStatus CPDF_LayoutProcessor_Reflow::Continue()
{
    int size = m_pReflowedPage->m_pReflowed->GetSize();
    ProcessElement(m_pRootElement, m_CurrRefWidth);
    size = m_pReflowedPage->m_pReflowed->GetSize();
    if(m_Status == LayoutReady) {
        m_Status = LayoutFinished;
        FitPageMode();
    }
    return m_Status;
}
int CPDF_LayoutProcessor_Reflow::GetPosition()
{
    return m_PausePosition;
}
FX_BOOL	CPDF_LayoutProcessor_Reflow::IsCanBreakAfter(FX_DWORD unicode)
{
    if(unicode == -1) {
        return FALSE;
    }
    switch(unicode) {
        case 40:
        case 91:
        case 123:
            return FALSE;
    }
    if(unicode >= 256) {
        return TRUE;
    } else if(unicode >= 48 && unicode <= 57) {
        return FALSE;
    } else if(unicode >= 64 && unicode <= 90) {
        return FALSE;
    } else if(unicode >= 97 && unicode <= 122) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL	CPDF_LayoutProcessor_Reflow::IsCanBreakBefore(FX_DWORD unicode)
{
    if(unicode == -1) {
        return FALSE;
    }
    switch(unicode) {
        case 33:
        case 41:
        case 44:
        case 46:
        case 59:
        case 63:
        case 93:
        case 125:
            return FALSE;
    }
    if(unicode >= 256) {
        return TRUE;
    } else if(unicode >= 48 && unicode <= 57) {
        return FALSE;
    } else if(unicode >= 64 && unicode <= 90) {
        return FALSE;
    } else if(unicode >= 97 && unicode <= 122) {
        return FALSE;
    }
    return TRUE;
}
void CPDF_LayoutProcessor_Reflow::ProcessTable(FX_FLOAT dx)
{
    if(m_pReflowedPage->m_pReflowed->GetSize() == 0) {
        return;
    }
    CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
    int rowCount = pTable->m_nCell.GetSize();
    int n = 0;
    FX_FLOAT* dyRow = FX_Alloc(FX_FLOAT, rowCount + 1);
    FXSYS_memset32(dyRow, 0, sizeof(FX_FLOAT) * (rowCount + 1));
    dyRow[0] = 0 ;
    dyRow[0] = - pTable->m_ReflowPageHeight;
    int tableColCount = 0;
    int i;
    for(i = 0; i < rowCount; i++) {
        int colCount = pTable->m_nCell.GetAt(i);
        if(colCount > tableColCount) {
            tableColCount = colCount;
        }
    }
    int cellCount = tableColCount * rowCount;
    RF_TableCell** pVirtualTable = FX_Alloc(RF_TableCell*, cellCount);
    FXSYS_memset32(pVirtualTable, 0, sizeof(RF_TableCell*) * cellCount);
    for(i = 0; i < rowCount; i++) {
        int colCount = pTable->m_nCell.GetAt(i);
        FX_FLOAT rowWidth = 0;
        int j = 0;
        int s = pTable->m_pCellArray.GetSize();
        for(j = 0; j < colCount; j++) {
            RF_TableCell* pCell = (RF_TableCell*)pTable->m_pCellArray.GetAt(n++);
            if(pCell->m_EndPos < pCell->m_BeginPos) {
                continue;
            }
            int pos = i * tableColCount;
            while(pos < cellCount && pVirtualTable[pos] != NULL) {
                pos++;
            }
            if(pos >= (i + 1) * tableColCount) {
                pos = i * tableColCount + j;
            }
            int RowSpan = pCell->m_RowSpan;
            int ColSpan = pCell->m_ColSpan;
            if(RowSpan + i > rowCount) {
                RowSpan = rowCount - i;
            }
            if(ColSpan + j > colCount) {
                ColSpan = colCount - j;
            }
            for(int m = 0; m < RowSpan; m++) {
                for(int nn = 0; nn < ColSpan; nn++) {
                    if(pos + nn >= cellCount) {
                        break;
                    }
                    pVirtualTable[pos + nn] = pCell;
                }
                pos += tableColCount;
            }
            FX_FLOAT dxCell = dx;
            for(pos = i * tableColCount; pVirtualTable[pos] != pCell && pos < cellCount; pos++) {
                dxCell += (pVirtualTable[pos])->m_MaxWidth;
            }
            CRF_Data* pData = (*m_pReflowedPage->m_pReflowed)[pCell->m_BeginPos];
            FX_FLOAT dy = dyRow[i] - pData->m_Height - pData->m_PosY;
            CFX_AffineMatrix matrix(1, 0, 0, 1, dxCell, dy);
            Transform(&matrix, m_pReflowedPage->m_pReflowed, pCell->m_BeginPos, pCell->m_EndPos - pCell->m_BeginPos + 1);
            if(pCell->m_RowSpan + i <= rowCount) {
                if(FXSYS_fabs(dyRow[pCell->m_RowSpan + i]) < FXSYS_fabs(dyRow[i] - pCell->m_CellHeight)) {
                    dyRow[pCell->m_RowSpan + i] = dyRow[i] - pCell->m_CellHeight;
                }
            }
        }
    }
    n = 0;
    for(i = 0; i < rowCount; i++) {
        int colCount = pTable->m_nCell.GetAt(i);
        for(int j = 0; j < colCount; j++) {
            RF_TableCell* pCell = (RF_TableCell*)pTable->m_pCellArray.GetAt(n++);
            switch(pCell->m_BlockAlign) {
                case LayoutAfter: {
                        FX_FLOAT dy = dyRow[i + pCell->m_RowSpan] - pCell->m_CellHeight - dyRow[i];
                        CFX_AffineMatrix matrix(1, 0, 0, 1, 0, dy);
                        Transform(&matrix, m_pReflowedPage->m_pReflowed, pCell->m_BeginPos, pCell->m_EndPos - pCell->m_BeginPos + 1);
                    }
                    break;
                case LayoutMiddle:
                case LayoutJustify: {
                        FX_FLOAT dy = (dyRow[i + pCell->m_RowSpan] + pCell->m_CellHeight - dyRow[i]) / 2;
                        CFX_AffineMatrix matrix(1, 0, 0, 1, 0, dy);
                        Transform(&matrix, m_pReflowedPage->m_pReflowed, pCell->m_BeginPos, pCell->m_EndPos - pCell->m_BeginPos + 1);
                        break;
                    }
                default:
                    break;
            }
        }
    }
    CRF_Data* pData = (*m_pReflowedPage->m_pReflowed)[m_pReflowedPage->m_pReflowed->GetSize() - 1];
    m_pReflowedPage->m_PageHeight = - dyRow[rowCount] + pData->m_Height;
    FX_Free(pVirtualTable);
    FX_Free(dyRow);
    int size = pTable->m_pCellArray.GetSize();
    for(i = 0; i < size; i++) {
        RF_TableCell* pCell = pTable->m_pCellArray.GetAt(i);
        FX_Free(pCell);
    }
    pTable->m_pCellArray.RemoveAll();
    pTable->m_nCell.RemoveAll();
    int s = sizeof(CRF_Table);
    delete pTable;
    m_TableArray.RemoveAt(m_TableArray.GetSize() - 1);
}
CFX_FloatRect CPDF_LayoutProcessor_Reflow::GetElmBBox(IPDF_LayoutElement* pElement)
{
    CFX_FloatRect rect;
    int objCount = pElement->CountObjects();
    int count = pElement->CountChildren();
    if(objCount == 0 && count == 0) {
        return rect;
    }
    CFX_AffineMatrix matrix;
    int i;
    for(i = 0; i < objCount; i++) {
        CPDF_PageObject* pObj = pElement->GetObject(0);
        if(!pObj) {
            continue;
        }
        if( rect.Height() == 0 ) {
            rect = pObj->GetBBox(&matrix);
        } else {
            rect.Union(pObj->GetBBox(&matrix));
        }
    }
    for(i = 0; i < count; i++) {
        IPDF_LayoutElement* pChildElement = pElement->GetChild(i);
        if( rect.Height() == 0 ) {
            rect = GetElmBBox(pChildElement);
        } else {
            rect.Union(GetElmBBox(pChildElement));
        }
    }
    return rect;
}
FX_FLOAT CPDF_LayoutProcessor_Reflow::GetElmWidth(IPDF_LayoutElement* pElement)
{
    if(!pElement) {
        return 0;
    }
    LayoutType layoutType = pElement->GetType();
    FX_FLOAT width = 0;
    if(layoutType == LayoutTable || layoutType == LayoutTableDataCell || layoutType == LayoutTableHeaderCell) {
        width = pElement->GetNumberAttr(LayoutWidth);
        if(width > 0) {
            return width;
        }
    } else if( layoutType == LayoutTableRow) {
        int count = pElement->CountChildren();
        for(int i = 0; i < count; i++) {
            IPDF_LayoutElement* pElm = pElement->GetChild(i);
            width += pElm->GetNumberAttr(LayoutWidth);
        }
        if(width > 0) {
            return width;
        }
    }
    CFX_FloatRect rect = GetElmBBox(pElement);
    return rect.Width();
}
FX_BOOL GetIntersection(FX_FLOAT low1, FX_FLOAT high1, FX_FLOAT low2, FX_FLOAT high2,
                        FX_FLOAT& interlow, FX_FLOAT& interhigh);
FX_BOOL IsSameLine(FX_BOOL bHorizontal, CFX_FloatRect Rect1, CFX_FloatRect Rect2)
{
    if(bHorizontal) {
        FX_FLOAT inter_top, inter_bottom;
        if (!GetIntersection(Rect1.bottom, Rect1.top, Rect2.bottom, Rect2.top,
                             inter_bottom, inter_top)) {
            return FALSE;
        }
        FX_FLOAT lineHeight = Rect1.top - Rect1.bottom;
        if(lineHeight > 20 && lineHeight > Rect2.Height() * 2) {
            return FALSE;
        }
        if(lineHeight > 5 && Rect2.Height() / 2 > lineHeight) {
            return FALSE;
        }
        FX_FLOAT inter_h = inter_top - inter_bottom;
        if (inter_h < (lineHeight) / 2 && inter_h < Rect2.Height() / 2) {
            return FALSE;
        }
    } else {
        FX_FLOAT inter_left, inter_right;
        if(!GetIntersection(Rect1.left, Rect1.right, Rect2.left, Rect2.right, inter_left, inter_right)) {
            return FALSE;
        }
        FX_FLOAT inter_w = inter_right - inter_left;
        if (inter_w < (Rect1.right - Rect1.left) / 2 && inter_w < (Rect2.right - Rect2.left) / 2) {
            return FALSE;
        }
    }
    return TRUE;
}
FX_INT32 IsCanMergeParagraph(IPDF_LayoutElement* pPrevElement, IPDF_LayoutElement* pNextElement)
{
    FX_INT32 analogial = 100;
    FX_INT32 nPrevObj = pPrevElement->CountObjects(), i;
    CPDF_PageObject* pPrevObj = NULL;
    CFX_FloatRect prevRect, rect;
    CFX_PtrArray prevLine, line;
    FX_BOOL bParagraphStart = FALSE;
    for(i = 0; i < nPrevObj; i++) {
        CPDF_PageObject* pObj = pPrevElement->GetObject(i);
        if(!pPrevObj) {
            pPrevObj = pObj;
            rect = CFX_FloatRect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
            line.Add(pObj);
            continue;
        }
        CFX_FloatRect objRect = CFX_FloatRect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
        if(IsSameLine(TRUE, rect, objRect)) {
            line.Add(pObj);
            rect.Union(objRect);
        } else {
            prevLine.RemoveAll();
            prevLine.Append(line);
            prevRect = rect;
            line.RemoveAll();
            line.Add(pObj);
            rect = objRect;
            if(!bParagraphStart) {
                if (prevRect.left > rect.left + rect.Height() * 1.5) {
                    bParagraphStart = TRUE;
                }
            }
        }
    }
    if(prevLine.GetSize()) {
        if(FXSYS_fabs(rect.right - prevRect.right) > rect.Height()) {
            analogial -= 50;
        }
    }
    CPDF_PageObject* pObj = pPrevElement->GetObject(nPrevObj - 1);
    if(pObj->m_Type == PDFPAGE_TEXT) {
        CPDF_TextObject* pText = (CPDF_TextObject*)pObj;
        FX_INT32 nItem = pText->CountItems();
        CPDF_TextObjectItem item;
        pText->GetItemInfo(nItem - 1, &item);
        CFX_WideString wStr = pText->GetFont()->UnicodeFromCharCode(item.m_CharCode);
        if(wStr.IsEmpty()) {
            wStr = (FX_WCHAR)item.m_CharCode;
        }
        FX_WCHAR wch = wStr.GetAt(wStr.GetLength() - 1);
        switch(wch) {
            case '.':
            case 12290:
            case 65311:
            case 63:
            case 33:
            case 65281:
                analogial -= 50;
                break;
        }
    }
    prevLine.RemoveAll();
    prevLine.Append(line);
    line.RemoveAll();
    FX_INT32 nNextObj = pNextElement->CountObjects();
    pPrevObj = NULL;
    FX_BOOL bFirst = TRUE;
    for(i = 0; i < nNextObj; i++) {
        CPDF_PageObject* pObj = pNextElement->GetObject(i);
        if(!pPrevObj) {
            pPrevObj = pObj;
            rect = CFX_FloatRect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
            line.Add(pObj);
            continue;
        }
        CFX_FloatRect objRect = CFX_FloatRect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
        if(IsSameLine(TRUE, rect, objRect)) {
            line.Add(pObj);
            rect.Union(objRect);
        } else {
            if(FXSYS_fabs(rect.right - prevRect.right) < rect.Height() && FXSYS_fabs(rect.left - prevRect.left) < rect.Height()) {
                analogial += 50;
            }
            prevLine.RemoveAll();
            prevLine.Append(line);
            prevRect = rect;
            line.RemoveAll();
            line.Add(pObj);
            rect = objRect;
            if(!bFirst) {
                break;
            }
            bFirst = FALSE;
        }
    }
    if(prevLine.GetSize()) {
        if(bParagraphStart) {
            if(prevRect.left - rect.left > rect.Height() && prevRect.left - rect.left < rect.Height() * 3) {
                analogial -= 50;
            }
        } else {
            if(FXSYS_fabs(prevRect.left - rect.left) < rect.Height()) {
                analogial -= 50;
            }
        }
    }
    return analogial;
}
void CPDF_LayoutProcessor_Reflow::ProcessElement(IPDF_LayoutElement* pElement, FX_FLOAT reflowWidth)
{
    if(pElement == NULL) {
        return;
    }
    if(m_Status == LayoutReady) {
        LayoutType layoutType = pElement->GetType();
        FX_INT32 ElementType = GetElementTypes(layoutType);
        switch(ElementType) {
            case SST_IE:
                m_bIllustration = TRUE;
                break;
            case SST_BLSE:
                FinishedCurrLine();
                FX_FLOAT StartIndent = 0;
                if(IPDF_LayoutElement* pParent = pElement->GetParent()) {
                    StartIndent = pParent->GetNumberAttr(LayoutStartIndent);
                }
                FX_FLOAT currStartIndent = pElement->GetNumberAttr(LayoutStartIndent);
                m_StartIndent = ConverWidth(currStartIndent);
                FX_FLOAT width = reflowWidth;
                if(StartIndent != currStartIndent) {
                    reflowWidth -= m_StartIndent;
                }
                FX_FLOAT spaceBefore = pElement->GetNumberAttr(LayoutSpaceBefore);
                m_pReflowedPage->m_PageHeight += spaceBefore;
                m_TextAlign = pElement->GetEnumAttr(LayoutTextAlign);
                if(IPDF_LayoutElement* pParent = pElement->GetParent()) {
                    StartIndent = pParent->GetNumberAttr(LayoutEndIndent);
                    FX_FLOAT currEndIndent = pElement->GetNumberAttr(LayoutEndIndent);
                    if(StartIndent != currStartIndent) {
                        reflowWidth -= ConverWidth(currEndIndent);
                    }
                }
                if(reflowWidth * 2 < width) {
                    reflowWidth = width;
                    m_StartIndent = 0;
                }
                break;
        }
        switch(layoutType) {
            case LayoutTable: {
                    CRF_Table* pTable = new CRF_Table;
                    m_TableArray.Add(pTable);
                    pTable->m_ReflowPageHeight = m_pReflowedPage->m_PageHeight;
                    pTable->m_TableWidth = GetElmWidth(pElement);
                    break;
                }
            case LayoutTableRow: {
                    if(!m_TableArray.GetSize()) {
                        break;
                    }
                    int count = pElement->CountChildren();
                    CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
                    int f = 0;
                    for(int i = 0; i < count; i++) {
                        IPDF_LayoutElement* pChildElement = pElement->GetChild(i);
                        LayoutType type = pChildElement->GetType();
                        if(type == LayoutTableDataCell || type == LayoutTableHeaderCell) {
                            f++;
                        }
                    }
                    pTable->m_nCell.Add(f);
                    break;
                }
            case LayoutTableDataCell:
            case LayoutTableHeaderCell: {
                    if(!m_TableArray.GetSize()) {
                        break;
                    }
                    RF_TableCell* pCell = FX_Alloc(RF_TableCell, 1);
                    FXSYS_memset32(pCell, 0 , sizeof(RF_TableCell));
                    CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
                    int pos = pTable->m_nCell.GetSize() - 1;
                    pCell->m_BeginPos = m_pReflowedPage->m_pReflowed->GetSize();
                    FX_FLOAT cellWidth = pElement->GetNumberAttr(LayoutWidth);
                    if(cellWidth == 0 || pCell->m_MaxWidth > pTable->m_TableWidth) {
                        CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
                        pCell->m_MaxWidth = reflowWidth / pTable->m_nCell.GetAt(pTable->m_nCell.GetSize() - 1);
                    } else {
                        pCell->m_MaxWidth = pElement->GetNumberAttr(LayoutWidth) * reflowWidth / pTable->m_TableWidth;
                    }
                    pCell->m_ColSpan = (int)(pElement->GetNumberAttr(LayoutColSpan));
                    pCell->m_RowSpan = (int)(pElement->GetNumberAttr(LayoutRowSpan));
                    if(!pCell->m_ColSpan) {
                        pCell->m_ColSpan = 1;
                    }
                    if(!pCell->m_RowSpan ) {
                        pCell->m_RowSpan = 1;
                    }
                    pCell->m_BlockAlign = pElement->GetEnumAttr(LayoutBlockAlign);
                    m_TextAlign = pElement->GetEnumAttr(LayoutInlineAlign);
                    pCell->m_PosX = 0;
                    pCell->m_PosY = 0;
                    reflowWidth = pCell->m_MaxWidth;
                    pTable->m_pCellArray.Add(pCell);
                    break;
                }
            default:
                break;
        }
        m_fLineHeight = pElement->GetNumberAttr(LayoutLineHeight);
        int ReflowedSize = m_pReflowedPage->m_pReflowed->GetSize();
        if(pElement->CountObjects()) {
            ProcessObjs(pElement, reflowWidth);
        }
    }
    int count = pElement->CountChildren();
    for(int i = 0; i < count; i++) {
        IPDF_LayoutElement* pChildElement = pElement->GetChild(i);
        ProcessElement(pChildElement, reflowWidth);
        if(m_pPause && m_pRootElement == pElement && m_Status != LayoutToBeContinued ) {
            if(m_pPause->NeedToPauseNow()) {
                m_pLayoutElement = pChildElement;
                m_Status = LayoutToBeContinued;
                m_CurrRefWidth = reflowWidth;
                m_PausePosition = (i + 1) * 100 / (count + 1);
                return ;
            }
        }
        if(m_Status == LayoutToBeContinued && m_pLayoutElement == pChildElement) {
            m_Status = LayoutReady;
        }
    }
    if(m_Status == LayoutReady) {
        FX_FLOAT dx = 0;
        LayoutType layoutType = pElement->GetType();
        FX_INT32 ElementType = GetElementTypes(layoutType);
        switch(ElementType) {
            case SST_IE:
                m_bIllustration = FALSE;
                FinishedCurrLine();
                break;
            case SST_BLSE:
                FinishedCurrLine();
                FX_FLOAT StartIndent = 0;
                if(IPDF_LayoutElement* pParent = pElement->GetParent()) {
                    StartIndent = pParent->GetNumberAttr(LayoutStartIndent);
                }
                FX_FLOAT currStartIndent = pElement->GetNumberAttr(LayoutStartIndent);
                if(StartIndent != currStartIndent) {
                    reflowWidth += ConverWidth(currStartIndent);
                    dx += ConverWidth(currStartIndent);
                }
                FX_FLOAT spaceAfter = pElement->GetNumberAttr(LayoutSpaceAfter);
                m_pReflowedPage->m_PageHeight += spaceAfter;
                break;
        }
        switch(layoutType) {
            case LayoutTableDataCell:
            case LayoutTableHeaderCell: {
                    if(!m_TableArray.GetSize()) {
                        break;
                    }
                    CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
                    RF_TableCell* pCell = pTable->m_pCellArray.GetAt(pTable->m_pCellArray.GetSize() - 1);
                    pCell->m_EndPos = m_pReflowedPage->m_pReflowed->GetSize() - 1;
                    if(pCell->m_EndPos < pCell->m_BeginPos) {
                        pCell->m_CellHeight = 0;
                    } else {
                        CRF_Data* pBeginData = (*m_pReflowedPage->m_pReflowed)[pCell->m_BeginPos];
                        CRF_Data* pEndData = (*m_pReflowedPage->m_pReflowed)[pCell->m_EndPos];
                        pCell->m_CellHeight = pBeginData->m_Height > pEndData->m_Height ? pBeginData->m_Height : pEndData->m_Height;
                        pCell->m_CellHeight -= pEndData->m_PosY - pBeginData->m_PosY;
                    }
                    break;
                }
            case LayoutTableRow: {
                    if(!m_TableArray.GetSize()) {
                        break;
                    }
                    CRF_Table* pTable = m_TableArray.GetAt(m_TableArray.GetSize() - 1);
                    if(pTable->m_nCol == 0) {
                        pTable->m_nCol = pTable->m_pCellArray.GetSize();
                    }
                    break;
                }
            case LayoutTable: {
                    ProcessTable(dx);
                    break;
                }
            default:
                if(dx) {
                    CFX_AffineMatrix matrix(1, 0, 0, 1, dx, 0);
                    int ReflowedSize = m_pReflowedPage->m_pReflowed->GetSize();
                    Transform(&matrix, m_pReflowedPage->m_pReflowed, ReflowedSize, m_pReflowedPage->m_pReflowed->GetSize() - ReflowedSize);
                }
        }
    }
    if(m_pRootElement == pElement) {
        m_PausePosition = 100;
    }
}
FX_INT32 CPDF_LayoutProcessor_Reflow::GetElementTypes(LayoutType layoutType)
{
    switch(layoutType) {
        case LayoutParagraph:
        case LayoutHeading:
        case LayoutHeading1:
        case LayoutHeading2:
        case LayoutHeading3:
        case LayoutHeading4:
        case LayoutHeading5:
        case LayoutHeading6:
        case LayoutList:
        case LayoutListItem:
        case LayoutListLabel:
        case LayoutListBody:
        case LayoutTable:
        case LayoutTableHeaderCell:
        case LayoutTableDataCell:
        case LayoutTableRow:
        case LayoutTableHeaderGroup:
        case LayoutTableBodyGroup:
        case LayoutTableFootGroup:
        case LayoutTOCI:
        case LayoutCaption:
            return SST_BLSE;
        case LayoutFigure:
        case LayoutFormula:
        case LayoutForm:
            return SST_IE;
        case LayoutSpan:
        case LayoutQuote:
        case LayoutNote:
        case LayoutReference:
        case LayoutBibEntry:
        case LayoutCode:
        case LayoutLink:
        case LayoutAnnot:
        case LayoutRuby:
        case LayoutWarichu:
            return SST_ILSE;
        default:
            return SST_GE;
    }
    return FALSE;
}
FX_FLOAT	CPDF_LayoutProcessor_Reflow::ConverWidth(FX_FLOAT width)
{
    return width;
}
void CPDF_LayoutProcessor_Reflow::ProcessObject(CPDF_PageObject* pObj, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix)
{
    if(!pObj) {
        return;
    }
    if(pObj->m_Type == PDFPAGE_TEXT) {
        ProcessTextObject( (CPDF_TextObject *)pObj, reflowWidth, objMatrix);
    } else if(pObj->m_Type == PDFPAGE_IMAGE) {
        if(!(m_flags & RF_PARSER_IMAGE)) {
            return;
        }
        CPDF_PageObjects* pObjs = new CPDF_PageObjects(FALSE);
        FX_POSITION pos = pObjs->GetLastObjectPosition();
        pos = pObjs->InsertObject(pos, pObj);
        CFX_AffineMatrix matrix;
        FX_RECT rect = pObj->GetBBox(&matrix);
        CPDF_ImageObject* ImageObj = (CPDF_ImageObject*)pObj;
        ProcessUnitaryObjs(pObjs, reflowWidth, objMatrix);
        delete pObjs;
    } else if(pObj->m_Type == PDFPAGE_PATH) {
    } else if(pObj->m_Type == PDFPAGE_FORM) {
        CPDF_FormObject* pForm = (CPDF_FormObject*)pObj;
        FX_POSITION pos = pForm->m_pForm->GetFirstObjectPosition();
        objMatrix.Concat(pForm->m_FormMatrix);
        while (pos) {
            CPDF_PageObject* pObj1 = pForm->m_pForm->GetNextObject(pos);
            ProcessObject(pObj1, reflowWidth, objMatrix);
        }
    }
}
void CPDF_LayoutProcessor_Reflow::ProcessObjs(IPDF_LayoutElement* pElement, FX_FLOAT reflowWidth)
{
    m_fCurrMaxWidth = reflowWidth;
    int ObjCount = pElement->CountObjects();
    for(int i = 0; i < ObjCount; i++) {
        CPDF_PageObject* pObj = pElement->GetObject(i);
        ProcessObject(pObj, reflowWidth, m_PDFMatrix);
        continue;
    }
}
void CPDF_LayoutProcessor_Reflow::AddTemp2CurrLine(int begin, int count)
{
    if(begin < 0 || count <= 0 || !m_pReflowedPage || !m_pReflowedPage->m_pReflowed || !m_pTempLine) {
        return;
    } else {
        count += begin;
    }
    int size = m_pReflowedPage->m_pReflowed->GetSize();
    int temps = m_pTempLine->GetSize();
    for(int i = begin; i < count; i++) {
        CRF_Data* pData = (*m_pTempLine)[i];
        AddData2CurrLine(pData);
    }
}
void CPDF_LayoutProcessor_Reflow::AddData2CurrLine(CRF_Data* pData)
{
    if(pData == NULL || m_pCurrLine == NULL) {
        return;
    }
    m_pCurrLine->Add(pData);
    m_fCurrLineWidth = pData->m_PosX + pData->m_Width;
    if(pData->m_Height > m_fCurrLineHeight) {
        m_fCurrLineHeight = pData->m_Height;
    }
}
void CPDF_LayoutProcessor_Reflow::UpdateCurrLine()
{
}
void CPDF_LayoutProcessor_Reflow::Transform(const CFX_AffineMatrix* pMatrix, CRF_DataPtrArray* pDataArray, int beginPos, int count)
{
    if (!pDataArray) {
        return;
    }
    if(count == 0) {
        count = pDataArray->GetSize();
    } else {
        count += beginPos;
    }
    for(int i = beginPos; i < count; i++) {
        CRF_Data* pData = (*pDataArray)[i];
        Transform(pMatrix, pData);
    }
}
void CPDF_LayoutProcessor_Reflow::Transform(const CFX_AffineMatrix* pMatrix, CRF_Data* pData)
{
    if(pData->GetType() == CRF_Data::Path) {
        CRF_PathData* pPathData = (CRF_PathData*)pData;
        pPathData->m_pPath2Device.Concat(*pMatrix);
    }
    pMatrix->Transform(pData->m_PosX, pData->m_PosY, pData->m_PosX, pData->m_PosY);
}
FX_BOOL CPDF_LayoutProcessor_Reflow::FinishedCurrLine()
{
    if (NULL == m_pCurrLine) {
        return FALSE;
    }
    int count = m_pCurrLine->GetSize();
    if(count == 0) {
        return FALSE;
    }
    if(m_fLineHeight > m_fCurrLineHeight) {
        m_fCurrLineHeight = m_fLineHeight;
    } else {
        m_fCurrLineHeight += 2;
    }
    if(m_pReflowedPage->m_pReflowed->GetSize() > 0) {
        m_fCurrLineHeight += m_fLineSpace;
    }
    FX_FLOAT height = m_pReflowedPage->m_PageHeight + m_fCurrLineHeight;
    FX_FLOAT lineHeight = m_fLineHeight;
    if(lineHeight == 0) {
        lineHeight = m_fCurrLineHeight;
    }
    FX_FLOAT dx = 0;
    switch(m_TextAlign) {
        case LayoutCenter:
            dx = (m_fCurrMaxWidth - m_fCurrLineWidth) / 2;
            break;
        case LayoutEnd:
            dx = m_fCurrMaxWidth - m_fCurrLineWidth;
            break;
        case LayoutJustify:
            break;
        default:
            break;
    }
    FX_FLOAT dy = - height;
    int refedSize = m_pReflowedPage->m_pReflowed->GetSize();
    if(count == 13) {
        int a = 0;
    }
    for(int i = 0; i < count; i++) {
        CRF_Data* pData = (*m_pCurrLine)[i];
        m_pReflowedPage->m_pReflowed->Add(pData);
        FX_FLOAT x = m_StartIndent + dx * (m_TextAlign == LayoutJustify ? i + 1 : 1);
        CFX_AffineMatrix matrix(1, 0, 0, 1, x, dy);
        Transform(&matrix, pData);
    }
    m_pCurrLine->RemoveAll();
    m_fCurrLineWidth = 0;
    m_pReflowedPage->m_PageHeight += m_fCurrLineHeight;
    m_fCurrLineHeight = 0;
    return TRUE;
}
CRF_CharState* CPDF_LayoutProcessor_Reflow::GetCharState(CPDF_TextObject* pObj, CPDF_Font* pFont, FX_FLOAT fHeight, FX_ARGB color)
{
    if (NULL == m_pReflowedPage->m_pCharState) {
        return NULL;
    }
    int count = m_pReflowedPage->m_pCharState->GetSize();
    for(int i = count - 1; i >= 0; i--) {
        CRF_CharState* pState = (CRF_CharState*)m_pReflowedPage->m_pCharState->GetAt(i);
        if(pState->m_Color == color && pState->m_fFontSize == fHeight && pState->m_pFont == pFont && pState->m_pTextObj == pObj) {
            return pState;
        }
    }
    CRF_CharState pState;
    pState.m_pTextObj = pObj;
    pState.m_Color = color;
    pState.m_pFont = pFont;
    pState.m_fFontSize = fHeight;
    int ascent = pFont->GetTypeAscent();
    int descent = pFont->GetTypeDescent();
    pState.m_fAscent = ascent * fHeight / (ascent - descent);
    if(descent == 0) {
        pState.m_fDescent = 0;
    } else {
        pState.m_fDescent = descent * fHeight / (ascent - descent);
    }
    pState.m_bVert = FALSE;
    CPDF_CIDFont *pCIDFont = pFont->GetCIDFont();
    if(pCIDFont) {
        pState.m_bVert = pCIDFont->IsVertWriting();
    }
    m_pReflowedPage->m_pCharState->Add(pState);
    return (CRF_CharState*)m_pReflowedPage->m_pCharState->GetAt(count);
}
int CPDF_LayoutProcessor_Reflow::GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont) const
{
    if(charCode == -1) {
        return 0;
    }
    int w = pFont->GetCharWidthF(charCode);
    if(w == 0) {
        CFX_ByteString str;
        pFont->AppendChar(str, charCode);
        w = pFont->GetStringWidth(str, 1);
        if(w == 0) {
            FX_RECT BBox;
            pFont->GetCharBBox(charCode, BBox);
            w = BBox.right - BBox.left;
        }
    }
    return w;
}
void CPDF_LayoutProcessor_Reflow::CreateRFData(CPDF_PageObject* pObj, CFX_AffineMatrix* pObjMatrix)
{
    if (NULL == m_pReflowedPage->m_pMemoryPool) {
        return;
    }
    if(pObj->m_Type == PDFPAGE_TEXT) {
        CPDF_TextObject* pTextObj = (CPDF_TextObject* )pObj;
        int count = pTextObj->CountItems();
        if(!count) {
            return;
        }
        if(count == 1) {
            CPDF_TextObjectItem Item;
            pTextObj->GetItemInfo(0, &Item);
            if(Item.m_CharCode == 49) {
                int a = 0;
            }
        }
        CPDF_Font * pFont = pTextObj->GetFont();
        FX_FLOAT fs = pTextObj->GetFontSize();
        FX_FLOAT* pmatrix = pTextObj->m_TextState.GetMatrix();
        FX_FLOAT matrix1 = pmatrix[1];
        if(pmatrix[2] == 0) {
            matrix1 = 0;
        }
        CFX_AffineMatrix textMatrix(pmatrix[0], matrix1, pmatrix[2], pmatrix[3], 0, 0);
        FX_FLOAT height = FXSYS_fabs(textMatrix.TransformDistance(fs));
        if(pObjMatrix) {
            height = FXSYS_fabs(pObjMatrix->TransformDistance(height));
        }
        int r = 0, g = 0, b = 0;
        pTextObj->m_ColorState.GetFillColor()->GetRGB(r, g, b);
        FX_ARGB col = r * 0x10000;
        col += g * 0x100;
        col += b;
        CRF_CharState* pState = GetCharState(pTextObj, pFont, height, col);
        FX_FLOAT dx = 0, dy = 0;
        FX_RECT ObjBBox;
        if(pObjMatrix) {
            ObjBBox = pTextObj->GetBBox(pObjMatrix);
            dx = (float)ObjBBox.left;
            dy = (float)ObjBBox.bottom;
        } else {
            CFX_AffineMatrix matrix;
            ObjBBox = pTextObj->GetBBox(&matrix);
        }
        FX_FLOAT objWidth = 0;
        CFX_ByteString str;
        FX_BOOL bOrder = TRUE;
        CFX_PtrArray tempArray;
        int i = 0;
        CPDF_TextObjectItem Item;
        pTextObj->GetItemInfo(i, &Item);
        dx = Item.m_OriginX;
        dy = Item.m_OriginY;
        textMatrix.Transform(Item.m_OriginX, Item.m_OriginY, dx, dy);
        CRF_CharData* pLastData = NULL;
        FX_FLOAT horzScale = pTextObj->m_TextState.GetFontSizeV() / pTextObj->m_TextState.GetFontSizeH();
        while(i < count) {
            pTextObj->GetItemInfo(i, &Item);
            if(Item.m_CharCode == -1) {
                i++;
                continue;
            }
            FX_FLOAT OriginX, OriginY;
            textMatrix.Transform(Item.m_OriginX, Item.m_OriginY, OriginX, OriginY);
            CRF_CharData* pData = (CRF_CharData*)m_pReflowedPage->m_pMemoryPool->Alloc(sizeof(CRF_CharData));
            if (NULL == pData) {
                continue;
            }
            pData->m_Type = CRF_Data::Text;
            if(FXSYS_fabs(OriginY - dy) > FXSYS_fabs(OriginX - dx)) {
                pData->m_PosY = dy;
                pData->m_PosX = pLastData->m_PosX + pLastData->m_Width + textMatrix.TransformDistance(pTextObj->m_TextState.GetObject()->m_CharSpace);
            } else {
                pData->m_PosY = OriginY;
                pData->m_PosX = OriginX;
            }
            int size = tempArray.GetSize();
            if(size && pData->m_PosX < pLastData->m_PosX ) {
                for (int j = 0; j < size; j++) {
                    CRF_CharData* pData1 = (CRF_CharData*)tempArray.GetAt(j);
                    if(pData1->m_PosX > pData->m_PosX) {
                        tempArray.InsertAt(j, pData);
                        break;
                    }
                }
            } else {
                tempArray.Add(pData);
            }
            pLastData = pData;
            pData->m_CharCode = Item.m_CharCode;
            pData->m_Height = FXSYS_fabs(height);
            int w = GetCharWidth(Item.m_CharCode, pFont);
            pData->m_Width = FXSYS_fabs(fs * textMatrix.TransformDistance((FX_FLOAT)w) / 1000);
            if(horzScale) {
                pData->m_Width /= horzScale;
            }
            pData->m_pCharState = pState;
            i++;
        }
        count = tempArray.GetSize();
        for (int j = 0; j < count; j++) {
            CRF_CharData* pData = (CRF_CharData*)tempArray.GetAt(j);
            if (m_pTempLine) {
                m_pTempLine->Add(pData);
            }
        }
        tempArray.RemoveAll();
    } else if(pObj->m_Type == PDFPAGE_IMAGE) {
        CPDF_ImageObject* pImageObj = (CPDF_ImageObject* )pObj;
        CRF_ImageData* pRFImage = (CRF_ImageData*)m_pReflowedPage->m_pMemoryPool->Alloc(sizeof(CRF_ImageData));
        if (NULL == pRFImage) {
            return;
        }
        pRFImage->m_pBitmap = NULL;
        pRFImage->m_Type = CRF_Data::Image;
        if (m_pTempLine) {
            m_pTempLine->Add(pRFImage);
        }
        CPDF_Image *pImage = pImageObj->m_pImage;
        if (!pImage->m_pDIBSource || !pImage->m_pMask) {
            if(pImage->StartLoadDIBSource(m_pReflowedPage->GetFormResDict(pImageObj), m_pReflowedPage->m_pPDFPage->m_pResources, 0, 0, TRUE)) {
                pImage->Continue(NULL);
            }
        }
        CFX_DIBSource* pDibSource = pImage->DetachBitmap();
        if (pDibSource) {
            pRFImage->m_pBitmap = pDibSource->Clone();
            delete pDibSource;
        }
        CFX_DIBSource* pMask = pImage->DetachMask();
        if (pMask) {
            if (!pMask->IsAlphaMask()) {
                CFX_DIBitmap* pMaskBmp = pMask->Clone();
                pMaskBmp->ConvertFormat(FXDIB_8bppMask);
                pRFImage->m_pBitmap->MultiplyAlpha(pMaskBmp);
                delete pMaskBmp;
            } else {
                pRFImage->m_pBitmap->MultiplyAlpha(pMask);
            }
            delete pMask;
        }
        CFX_FloatRect ObjBBox;
        if(pObjMatrix) {
            ObjBBox = pImageObj->GetBBox(pObjMatrix);
        } else {
            CFX_AffineMatrix matrix;
            ObjBBox = pImageObj->GetBBox(&matrix);
        }
        pRFImage->m_Width = ObjBBox.Width();
        pRFImage->m_Height = ObjBBox.Height();
        pRFImage->m_PosX = 0;
        pRFImage->m_PosY = 0;
        CFX_AffineMatrix matrix(1, 0, 0, -1, 0, 0);
        matrix.Concat(pImageObj->m_Matrix);
        matrix.Concat(*pObjMatrix);
        pRFImage->m_Matrix.Set(matrix.a == 0 ? 0 : matrix.a / FXSYS_fabs(matrix.a),
                               matrix.b == 0 ? 0 : matrix.b / FXSYS_fabs(matrix.b),
                               matrix.c == 0 ? 0 : matrix.c / FXSYS_fabs(matrix.c),
                               matrix.d == 0 ? 0 : matrix.d / FXSYS_fabs(matrix.d), 0, 0);
    } else if(pObj->m_Type == PDFPAGE_PATH) {
    }
}
FX_FLOAT CPDF_LayoutProcessor_Reflow:: GetDatasWidth(int beginPos, int endpos)
{
    if(endpos < beginPos || !m_pTempLine) {
        return 0;
    }
    if(endpos > m_pTempLine->GetSize() - 1) {
        endpos = m_pTempLine->GetSize() - 1;
    }
    CRF_Data* pBeginData = (*m_pTempLine)[beginPos];
    CRF_Data* pEndData = (*m_pTempLine)[endpos];
    return pEndData->m_PosX - pBeginData->m_PosX + pEndData->m_Width;
}
FX_WCHAR CPDF_LayoutProcessor_Reflow::GetPreChar()
{
    if (NULL == m_pCurrLine) {
        return -1;
    }
    int index = m_pCurrLine->GetSize() - 1;
    CRF_CharData* pCharData = NULL;
    while (index >= 0 && !pCharData) {
        CRF_Data* pData = (*m_pCurrLine)[index];
        if(pData->GetType() == CRF_Data::Text) {
            pCharData = (CRF_CharData*)pData;
        } else {
            return -1;
        }
        index --;
    }
    if(m_pReflowedPage) {
        index = m_pReflowedPage->m_pReflowed->GetSize() - 1;
    }
    while(!pCharData && index >= 0) {
        CRF_Data* pData = (*m_pReflowedPage->m_pReflowed)[index];
        if(pData->GetType() == CRF_Data::Text) {
            pCharData = (CRF_CharData*)pData;
        } else {
            return -1;
        }
        index --;
    }
    if(pCharData) {
        CFX_WideString str = pCharData->m_pCharState->m_pFont->UnicodeFromCharCode(pCharData->m_CharCode);
        return str.GetAt(0);
    }
    return -1;
}
int CPDF_LayoutProcessor_Reflow::ProcessInsertObject(CPDF_TextObject* pObj, CFX_AffineMatrix formMatrix)
{
    if(!pObj || !m_pPreObj || !m_pCurrLine) {
        return 0;
    }
    if(m_pCurrLine->GetSize() == 0) {
        return 0;
    }
    CPDF_TextObjectItem item;
    int nItem = m_pPreObj->CountItems();
    m_pPreObj->GetItemInfo(nItem - 1, &item);
    FX_FLOAT last_pos = item.m_OriginX;
    FX_FLOAT last_width = GetCharWidth(item.m_CharCode, m_pPreObj->GetFont()) * m_pPreObj->GetFontSize() / 1000;
    last_width = FXSYS_fabs(last_width);
    pObj->GetItemInfo(0, &item);
    FX_FLOAT this_width = GetCharWidth(item.m_CharCode, pObj->GetFont()) * pObj->GetFontSize() / 1000;
    this_width = FXSYS_fabs(this_width);
    FX_FLOAT threshold = last_width > this_width ? last_width / 4 : this_width / 4;
    CFX_AffineMatrix prev_matrix, prev_reverse;
    m_pPreObj->GetTextMatrix(&prev_matrix);
    prev_matrix.Concat(m_perMatrix);
    prev_reverse.SetReverse(prev_matrix);
    FX_FLOAT x = pObj->GetPosX(), y = pObj->GetPosY();
    formMatrix.Transform(x, y);
    prev_reverse.Transform(x, y);
    FX_WCHAR preChar  = GetPreChar();
    CFX_WideString wstrItem = pObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
    FX_WCHAR curChar = wstrItem.GetAt(0);
    if (FXSYS_fabs(y) > threshold * 2) {
        if (preChar == L'-') {
            return 3;
        }
        if (preChar != L' ') {
            return 1;
        }
        return 2;
    }
    if ((x - last_pos - last_width) > threshold && curChar != L' ' && preChar != L' ') {
        return 1;
    }
    return 0;
}
FX_INT32 CPDF_LayoutProcessor_Reflow::LogicPreObj(CPDF_TextObject* pObj)
{
    CPDF_TextObject* pPreObj = m_pPreObj;
    m_pPreObj = pObj;
    if(!pObj || !pPreObj) {
        return 0;
    }
    CPDF_TextObjectItem item;
    pPreObj->GetItemInfo(pPreObj->CountItems() - 1, &item);
    FX_FLOAT last_pos = item.m_OriginX;
    FX_FLOAT last_width = pPreObj->GetFont()->GetCharWidthF(item.m_CharCode) * pPreObj->GetFontSize() / 1000;
    last_width = FXSYS_fabs(last_width);
    pObj->GetItemInfo(0, &item);
    FX_FLOAT this_width = pObj->GetFont()->GetCharWidthF(item.m_CharCode) * pObj->GetFontSize() / 1000;
    this_width = FXSYS_fabs(this_width);
    FX_FLOAT threshold = last_width > this_width ? last_width / 4 : this_width / 4;
    CFX_AffineMatrix prev_matrix, prev_reverse;
    pPreObj->GetTextMatrix(&prev_matrix);
    prev_reverse.SetReverse(prev_matrix);
    FX_FLOAT x = pObj->GetPosX(), y = pObj->GetPosY();
    prev_reverse.Transform(x, y);
    CFX_WideString wstrItem = pObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
    FX_WCHAR curChar = wstrItem.GetAt(0);
    if (FXSYS_fabs(y) > threshold * 2) {
        return 2;
    }
    FX_WCHAR preChar = 0;
    if (FXSYS_fabs(last_pos + last_width - x) > threshold && curChar != L' ') {
        return 1;
    }
    return 0;
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
FX_BOOL CPDF_LayoutProcessor_Reflow::IsSameTextObject(CPDF_TextObject* pTextObj1, CPDF_TextObject* pTextObj2)
{
    if (!pTextObj1 || !pTextObj2) {
        return FALSE;
    }
    CFX_FloatRect rcPreObj(pTextObj2->m_Left, pTextObj2->m_Bottom, pTextObj2->m_Right, pTextObj2->m_Top);
    CFX_FloatRect rcCurObj(pTextObj1->m_Left, pTextObj1->m_Bottom, pTextObj1->m_Right, pTextObj1->m_Top);
    if (rcPreObj.IsEmpty() && rcCurObj.IsEmpty()) {
        return FALSE;
    }
    if (!rcPreObj.IsEmpty() || !rcCurObj.IsEmpty()) {
        rcPreObj.Intersect(rcCurObj);
        if (rcPreObj.IsEmpty()) {
            return FALSE;
        }
        if (FXSYS_fabs(rcPreObj.Width() - rcCurObj.Width()) > rcCurObj.Width() / 2) {
            return FALSE;
        }
        if (pTextObj2->GetFontSize() != pTextObj1->GetFontSize()) {
            return FALSE;
        }
    }
    int nPreCount = pTextObj2->CountItems();
    int nCurCount = pTextObj1->CountItems();
    if (nPreCount != nCurCount) {
        return FALSE;
    }
    for (int i = 0; i < nPreCount; i++) {
        CPDF_TextObjectItem itemPer, itemCur;
        pTextObj2->GetItemInfo(i, &itemPer);
        pTextObj1->GetItemInfo(i, &itemCur);
        if (itemCur.m_CharCode != itemPer.m_CharCode) {
            return FALSE;
        }
    }
    return TRUE;
}
void CPDF_LayoutProcessor_Reflow::ProcessTextObject(CPDF_TextObject *pTextObj, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix)
{
    if(reflowWidth < 0 || !m_pCurrLine || !m_pTempLine) {
        return;
    }
    if(IsSameTextObject(pTextObj, m_pPreObj)) {
        return;
    }
    CPDF_PageObject* pPreObj = m_pPreObj;
    FX_INT32 logic = ProcessInsertObject(pTextObj, objMatrix);
    m_pPreObj = pTextObj;
    m_perMatrix.Copy(objMatrix);
    int size = m_pTempLine->GetSize();
    int curs = m_pCurrLine->GetSize();
    CreateRFData(pTextObj);
    size = m_pTempLine->GetSize();
    int reds = m_pReflowedPage->m_pReflowed->GetSize();
    if(size == 0) {
        return;
    }
    if(logic == 1) {
        m_fCurrLineWidth += pTextObj->GetBBox(&objMatrix).Height() / 3;
    } else if(logic == 3 && curs) {
        m_fCurrLineWidth -= (*m_pCurrLine)[curs - 1]->m_Width;
        m_pCurrLine->Delete(curs - 1);
    }
    int beginPos = 0, endPos = m_pTempLine->GetSize() - 1;
    while(beginPos <= endPos) {
        int tempBeginPos = beginPos;
        int tempEndPos = endPos;
        FX_FLOAT all_width = GetDatasWidth( beginPos, endPos);
        if(all_width < reflowWidth - m_fCurrLineWidth) {
            CRF_CharData* pBeginData = (CRF_CharData*)(*m_pTempLine)[beginPos];
            CFX_AffineMatrix matrix(1, 0, 0, 1, -pBeginData->m_PosX + m_fCurrLineWidth, -pBeginData->m_PosY);
            Transform(&matrix, m_pTempLine, beginPos, endPos - beginPos + 1);
            AddTemp2CurrLine(beginPos, endPos - beginPos + 1);
            m_pTempLine->RemoveAll();
            return;
        }
        int	midPos ;
        if(tempBeginPos >= tempEndPos && tempEndPos != 0) {
            midPos = tempEndPos;
        } else {
            while (tempBeginPos < tempEndPos ) {
                midPos = (tempEndPos - tempBeginPos) / 2 + tempBeginPos;
                if(midPos == tempBeginPos || midPos == tempEndPos) {
                    break;
                }
                FX_FLOAT w = GetDatasWidth( beginPos, midPos);
                if(w < reflowWidth - m_fCurrLineWidth) {
                    tempBeginPos = midPos;
                } else {
                    tempEndPos = midPos;
                }
            }
            midPos = tempBeginPos;
            if(midPos == 0) {
                FX_FLOAT w = GetDatasWidth( beginPos, 1);
                if(w > reflowWidth - m_fCurrLineWidth) {
                    midPos = -1;
                }
            }
        }
        if(midPos == -1) {
            int count = m_pCurrLine->GetSize();
            if(count == 0) {
                midPos = 0;
            }
        }
        int f = -1;
        int i = 0;
        for(i = midPos; i >= beginPos; i--) {
            CRF_CharData* pData = (CRF_CharData*)(*m_pTempLine)[i];
            CFX_WideString Wstr = pData->m_pCharState->m_pFont->UnicodeFromCharCode(pData->m_CharCode);
            FX_WCHAR cha = Wstr.GetAt(0);
            if(i < m_pTempLine->GetSize() - 1) {
                CRF_CharData* pNextData = (CRF_CharData*)(*m_pTempLine)[i + 1];
                if(pNextData->m_PosX - (pData->m_PosX + pData->m_Width) >= pData->m_Height / 4) {
                    f = i;
                    i++;
                }
            }
            if(f == -1) {
                if(IsCanBreakAfter((FX_DWORD)cha)) {
                    f = i;
                    i++;
                } else if(IsCanBreakBefore((FX_DWORD)cha)) {
                    f = i - 1;
                    if(f < beginPos) {
                        f = -1;
                    }
                }
            }
            if(f != -1) {
                CRF_CharData* pBeginData = (CRF_CharData*)(*m_pTempLine)[beginPos];
                CFX_AffineMatrix matrix(1, 0, 0, 1, -pBeginData->m_PosX + m_fCurrLineWidth, -pBeginData->m_PosY);
                Transform(&matrix, m_pTempLine, beginPos, f - beginPos + 1);
                CRF_Data* pData = (*m_pTempLine)[0];
                AddTemp2CurrLine(beginPos, f - beginPos + 1);
                beginPos = i;
                FinishedCurrLine();
                f = 1;
                break;
            }
        }
        if(f == -1 && i < beginPos) {
            if( m_pCurrLine->GetSize()) {
                int count = m_pCurrLine->GetSize();
                f = -1;
                for(int i = count - 1; i >= 0; i--) {
                    CRF_Data* pData = (*m_pCurrLine)[i];
                    if(pData->GetType() != CRF_Data::Text) {
                        f = i + 1;
                    } else {
                        CRF_CharData* pCharData = (CRF_CharData*)pData;
                        CFX_WideString Wstr = pCharData->m_pCharState->m_pFont->UnicodeFromCharCode(pCharData->m_CharCode);
                        FX_WCHAR cha = Wstr.GetAt(0);
                        if(IsCanBreakAfter(cha)) {
                            f = i + 1;
                            i++;
                        } else if(IsCanBreakBefore(cha)) {
                            f = i;
                        }
                        if(f == 0) {
                            f = -1;
                        }
                    }
                    if(f != -1) {
                        FinishedCurrLine();
                        if(f < count) {
                            int reflowdCount = m_pReflowedPage->m_pReflowed->GetSize();
                            int pos = reflowdCount + f - count;
                            CRF_CharData* pData = (CRF_CharData*)(*m_pReflowedPage->m_pReflowed)[pos];
                            CFX_AffineMatrix matrix(1, 0, 0, 1, -pData->m_PosX + m_fCurrLineWidth, -pData->m_PosY);
                            Transform(&matrix, m_pReflowedPage->m_pReflowed, pos, reflowdCount - pos);
                            for(int j = pos; j < reflowdCount; j++) {
                                AddData2CurrLine((*m_pReflowedPage->m_pReflowed)[j]);
                            }
                            m_pReflowedPage->m_pReflowed->Delete(pos, count - f);
                            if(logic == 3) {
                                m_fCurrLineWidth += pTextObj->GetBBox(&objMatrix).Height() / 3;
                            }
                        }
                        break;
                    }
                }
            }
            if(f == -1) {
                CRF_CharData* pData = (CRF_CharData*)(*m_pTempLine)[beginPos];
                CFX_AffineMatrix matrix(1, 0, 0, 1, -pData->m_PosX + m_fCurrLineWidth, -pData->m_PosY);
                if(beginPos == midPos) {
                    Transform(&matrix, pData);
                    FX_RECT rect;
                    pData->m_pCharState->m_pFont->GetFontBBox(rect);
                    FX_FLOAT* pmatrix = pTextObj->m_TextState.GetMatrix();
                    CFX_AffineMatrix textMatrix(pmatrix[0], pmatrix[1], pmatrix[2], pmatrix[3], 0, 0);
                    FX_FLOAT width = pData->m_Height * (rect.right - rect.left) / 1000;
                    FX_FLOAT f = (reflowWidth - m_fCurrLineWidth) / width;
                    pData->m_PosY *= f;
                    pData->m_Width *= f;
                    pData->m_Height *= f;
                    pData->m_pCharState = GetCharState(pData->m_pCharState->m_pTextObj, pData->m_pCharState->m_pFont, pData->m_Height, pData->m_pCharState->m_Color);
                    AddData2CurrLine(pData);
                } else {
                    for(int m = beginPos; m <= midPos; m++) {
                        CRF_CharData* pData = (CRF_CharData*)(*m_pTempLine)[m];
                        Transform(&matrix, pData);
                        AddData2CurrLine(pData);
                    }
                }
                FinishedCurrLine();
                beginPos = midPos + 1;
            }
        }
    }
    m_pTempLine->RemoveAll();
    return;
}
void CPDF_LayoutProcessor_Reflow::ProcessUnitaryObjs(CPDF_PageObjects *pObjs, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix)
{
    if(!pObjs) {
        return;
    }
    CFX_FloatRect ObjBBox = pObjs->CalcBoundingBox();
    objMatrix.TransformRect(ObjBBox);
    FX_FLOAT ObjWidth = ObjBBox.Width();
    FX_FLOAT ObjHeight = ObjBBox.Height();
    CFX_AffineMatrix matrix;
    if(ObjWidth <= reflowWidth - m_fCurrLineWidth) {
        matrix.Set(1, 0, 0, 1, m_fCurrLineWidth , 0);
    } else if(ObjWidth <= reflowWidth) {
        FinishedCurrLine();
        matrix.Set(1, 0, 0, 1, 0, 0);
    } else {
        FinishedCurrLine();
        FX_FLOAT f = reflowWidth / ObjWidth ;
        matrix.Set(f, 0, 0, f, 0, 0);
    }
    CFX_AffineMatrix tempMatrix = matrix;
    matrix.Concat(objMatrix);
    FX_POSITION pos = pObjs->GetFirstObjectPosition();
    while(pos) {
        CPDF_PageObject* pObj = pObjs->GetNextObject(pos);
        if(pObj->m_Type == PDFPAGE_TEXT) {
            FX_INT32 ret = LogicPreObj((CPDF_TextObject*)pObj);
            if(ret == 1 || ret == 2) {
                continue;
            }
        }
        CreateRFData(pObj, &matrix);
    }
    if (m_pTempLine) {
        Transform(&tempMatrix, m_pTempLine, 0, m_pTempLine->GetSize());
        AddTemp2CurrLine(0, m_pTempLine->GetSize());
        m_pTempLine->RemoveAll();
    }
}
void CPDF_LayoutProcessor_Reflow::ProcessPathObject(CPDF_PathObject *pObj, FX_FLOAT reflowWidth)
{
}
