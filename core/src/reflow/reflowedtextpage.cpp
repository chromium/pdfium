// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "reflowedtextpage.h"

IPDF_TextPage*	IPDF_TextPage::CreateReflowTextPage(IPDF_ReflowedPage* pRefPage)
{
    return new CRF_TextPage(pRefPage);
}
CRF_TextPage::CRF_TextPage(IPDF_ReflowedPage* pRefPage)
{
    m_pRefPage = (CPDF_ReflowedPage*)(pRefPage);
    m_pDataList = NULL;
    m_CountBSArray = NULL;
}
CRF_TextPage::~CRF_TextPage()
{
    if(m_pDataList) {
        delete m_pDataList;
        m_pDataList = NULL;
    }
    if(m_CountBSArray) {
        delete m_CountBSArray;
        m_CountBSArray = NULL;
    }
}
FX_BOOL CRF_TextPage::ParseTextPage()
{
    if(!m_pRefPage) {
        return FALSE;
    }
    int count = m_pRefPage->m_pReflowed->GetSize();
    m_pDataList = new CRF_CharDataPtrArray(std::min(count, 500));
    for(int i = 0; i < count; i++) {
        CRF_Data* pData = (*(m_pRefPage->m_pReflowed))[i];
        if(pData->GetType() == CRF_Data::Text) {
            m_pDataList->Add((CRF_CharData*)pData);
        }
    }
    m_CountBSArray = new CFX_CountBSINT32Array(20);
    return TRUE;
}
FX_BOOL	CRF_TextPage::IsParsered() const
{
    if(m_pDataList) {
        return TRUE;
    }
    return FALSE;
}
int CRF_TextPage::CharIndexFromTextIndex(int TextIndex) const
{
    return TextIndex;
}
int CRF_TextPage::TextIndexFromCharIndex(int CharIndex) const
{
    return CharIndex;
}

int	CRF_TextPage::CountChars() const
{
    if (NULL == m_pDataList) {
        return -1;
    }
    return m_pDataList->GetSize();
}
void CRF_TextPage::GetCharInfo(int index, FPDF_CHAR_INFO & info) const
{
    if(index >= CountChars() || index < 0 || !m_pDataList) {
        return;
    }
    CRF_CharData* pData = (*m_pDataList)[index];
    FX_FLOAT ReltiveCorddDs = pData->m_pCharState->m_fDescent;
    FX_FLOAT ReltiveCorddAs = pData->m_pCharState->m_fAscent;
    info.m_Flag		= CHAR_NORMAL;
    info.m_pTextObj	= pData->m_pCharState->m_pTextObj;
    info.m_OriginX	= pData->m_PosX;
    info.m_OriginY	= pData->m_PosY - ReltiveCorddDs;
    info.m_FontSize	= pData->m_pCharState->m_fFontSize;
    CFX_FloatRect FloatRectTmp(pData->m_PosX, pData->m_PosY, pData->m_PosX + pData->m_Width, pData->m_PosY + ReltiveCorddAs - ReltiveCorddDs);
    info.m_CharBox	= FloatRectTmp;
    CFX_WideString str = pData->m_pCharState->m_pFont->UnicodeFromCharCode(pData->m_CharCode);
    if(!str.IsEmpty()) {
        info.m_Unicode	= str.GetAt(0);
    } else {
        info.m_Unicode = -1;
    }
    info.m_Charcode = (FX_WCHAR)pData->m_CharCode;
    info.m_Matrix = CFX_Matrix(1, 0, 0, 1, 0, 0);
}
extern FX_BOOL GetIntersection(FX_FLOAT low1, FX_FLOAT high1, FX_FLOAT low2, FX_FLOAT high2, FX_FLOAT& interlow, FX_FLOAT& interhigh);
inline FX_BOOL _IsInsameline(const CFX_FloatRect& rectA, const CFX_FloatRect& rectB)
{
    if((rectA.top >= rectB.bottom && rectB.top >= rectA.bottom)) {
        return TRUE;
    } else {
        return FALSE;
    }
}
inline FX_BOOL _IsIntersect(const CFX_FloatRect& rectA, const CFX_FloatRect& rectB)
{
    FX_FLOAT interlow = .0f, interhigh = .0f;
    if(GetIntersection(rectA.bottom, rectA.top, rectB.bottom, rectB.top, interlow, interhigh)) {
        if(GetIntersection(rectA.left, rectA.right, rectB.left, rectB.right, interlow, interhigh)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
    return FALSE;
}
void CRF_TextPage::GetRectArray(int start, int nCount, CFX_RectArray& rectArray) const
{
    int indexlen = start + nCount;
    FPDF_CHAR_INFO info;
    FX_BOOL bstart = TRUE;
    CFX_FloatRect recttmp;
    int i;
    for(i = start; i < indexlen; i++) {
        GetCharInfo(i, info);
        if(bstart) {
            recttmp = info.m_CharBox;
            bstart = FALSE;
        } else if(_IsInsameline(recttmp, info.m_CharBox)) {
            recttmp.right = info.m_CharBox.right;
            if(info.m_CharBox.top > recttmp.top) {
                recttmp.top = info.m_CharBox.top;
            }
            if(info.m_CharBox.bottom < recttmp.bottom) {
                recttmp.bottom = info.m_CharBox.bottom;
            }
        } else {
            rectArray.Add(recttmp);
            recttmp = info.m_CharBox;
        }
    }
    rectArray.Add(recttmp);
}
inline FX_FLOAT _GetDistance(CFX_FloatRect floatRect, CPDF_Point point)
{
    if(floatRect.right < point.x && floatRect.bottom > point.y) {
        return FXSYS_sqrt(FXSYS_pow(point.x - floatRect.right, 2) + FXSYS_pow(floatRect.bottom - point.y, 2));
    }
    if (floatRect.right < point.x && floatRect.top < point.y) {
        return FXSYS_sqrt(FXSYS_pow(point.x - floatRect.right, 2) + FXSYS_pow(point.y - floatRect.top, 2));
    }
    if(floatRect.left > point.x && floatRect.bottom > point.y) {
        return FXSYS_sqrt(FXSYS_pow(floatRect.bottom - point.y, 2) + FXSYS_pow(floatRect.left - point.x, 2));
    }
    if((floatRect.right > point.x || FXSYS_fabs(floatRect.right - point.x) <= 0.0001f) &&
            (floatRect.left < point.x || FXSYS_fabs(floatRect.left - point.x) <= 0.0001f) && floatRect.bottom > point.y) {
        return FXSYS_fabs(floatRect.bottom - point.y);
    }
    if(floatRect.left > point.x && (floatRect.bottom < point.y || FXSYS_fabs(floatRect.bottom - point.y) <= 0.0001f) &&
            (floatRect.top > point.y || FXSYS_fabs(floatRect.top - point.y) <= 0.0001f)) {
        return FXSYS_fabs(floatRect.left - point.x);
    }
    if(floatRect.left > point.x && floatRect.top < point.y) {
        return FXSYS_sqrt(FXSYS_pow(floatRect.left - point.x, 2) + FXSYS_pow(point.y - floatRect.top, 2));
    }
    if ((floatRect.left < point.x || FXSYS_fabs(floatRect.left - point.x) <= 0.0001f) &&
            (floatRect.right > point.x || FXSYS_fabs(floatRect.right - point.x) <= 0.0001f) && floatRect.top < point.y) {
        return FXSYS_fabs(point.y - floatRect.top);
    }
    if(floatRect.right < point.x && (floatRect.top > point.y || FXSYS_fabs(floatRect.top - point.y) <= 0.0001f) &&
            (floatRect.bottom < point.y || FXSYS_fabs(floatRect.bottom - point.y) <= 0.0001f)) {
        return point.x - floatRect.right;
    }
    return .0f;
}
int CRF_TextPage::GetIndexAtPos(CPDF_Point point, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const
{
    int index = -1, i = 0, j = 0;
    FPDF_CHAR_INFO info;
    CFX_FloatRect rectTmp;
    FX_FLOAT MinDistance = 1000, DistanceTmp = 0;
    FX_FLOAT rect_bottom = point.x - xTorelance;
    CFX_FloatRect TorelanceRect(rect_bottom <= 0 ? 0 : rect_bottom, point.y - yTorelance, point.x + xTorelance, point.y + yTorelance);
    int count = CountChars();
    for(i = 0; i < count; i++) {
        GetCharInfo(i, info);
        rectTmp = info.m_CharBox;
        if(rectTmp.Contains(point.x, point.y)) {
            index = i;
            break;
        } else if(_IsIntersect(rectTmp, TorelanceRect)) {
            DistanceTmp = _GetDistance(rectTmp, point);
            if(DistanceTmp < MinDistance) {
                MinDistance = DistanceTmp;
                index = i;
            }
        }
    }
    return index;
}
int CRF_TextPage::GetIndexAtPos(FX_FLOAT x, FX_FLOAT y, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const
{
    int index = 0;
    CPDF_Point point(x, y);
    if((index = GetIndexAtPos(point, xTorelance, yTorelance)) < 0) {
        return -1;
    } else {
        return index;
    }
}
int CRF_TextPage::GetOrderByDirection(int index, int direction) const
{
    return -1;
}
CFX_WideString CRF_TextPage::GetTextByRect(CFX_FloatRect rect) const
{
    int count;
    FPDF_CHAR_INFO info;
    CFX_WideString str;
    CFX_FloatRect  Recttmp;
    FX_BOOL bstart = TRUE;
    count = CountChars();
    if(rect.IsEmpty()) {
        return L"";
    }
    for(int i = 0; i < count; i++) {
        GetCharInfo(i, info);
        if(_IsIntersect(rect, info.m_CharBox)) {
            if(bstart) {
                Recttmp = info.m_CharBox;
                str += info.m_Unicode;
                bstart = FALSE;
            } else if(_IsInsameline(Recttmp, info.m_CharBox)) {
                str += info.m_Unicode;
            } else {
                str += L"\r\n";
                Recttmp = info.m_CharBox;
                str += info.m_Unicode;
            }
        }
    }
    if(str.IsEmpty()) {
        return L"";
    } else {
        return str;
    }
}
void CRF_TextPage::GetRectsArrayByRect(CFX_FloatRect rect, CFX_RectArray& resRectArray) const
{
    int count, i;
    FX_BOOL bstart = TRUE;
    FPDF_CHAR_INFO info;
    CFX_FloatRect recttmp;
    count = CountChars();
    for(i = 0; i < count; i++) {
        GetCharInfo(i, info);
        if(_IsIntersect(rect, info.m_CharBox)) {
            if(bstart) {
                recttmp = info.m_CharBox;
                bstart = FALSE;
            } else if(_IsInsameline(recttmp, info.m_CharBox)) {
                recttmp.right = info.m_CharBox.right;
                if(info.m_CharBox.top > recttmp.top) {
                    recttmp.top = info.m_CharBox.top;
                }
                if(info.m_CharBox.bottom < recttmp.bottom) {
                    recttmp.bottom = info.m_CharBox.bottom;
                }
            } else {
                resRectArray.Add(recttmp);
                recttmp = info.m_CharBox;
            }
        }
    }
    resRectArray.Add(recttmp);
}
int CRF_TextPage::CountRects(int start, int nCount)
{
    m_rectArray.RemoveAll();
    GetRectArray(start, nCount, m_rectArray);
    return m_rectArray.GetSize();
}
void CRF_TextPage::GetRect(int rectIndex, FX_FLOAT& left, FX_FLOAT& top, FX_FLOAT& right, FX_FLOAT &bottom) const
{
    if(m_rectArray.GetSize() <= rectIndex) {
        return;
    }
    left   = m_rectArray[rectIndex].left;
    top    = m_rectArray[rectIndex].top;
    right  = m_rectArray[rectIndex].right;
    bottom = m_rectArray[rectIndex].bottom;
}
FX_BOOL CRF_TextPage::GetBaselineRotate(int rectIndex, int& Rotate)
{
    Rotate = 0;
    return TRUE;
}
FX_BOOL CRF_TextPage::GetBaselineRotate(CFX_FloatRect rect, int& Rotate)
{
    Rotate = 0;
    return TRUE;
}
int CRF_TextPage::CountBoundedSegments(FX_FLOAT left, FX_FLOAT top, FX_FLOAT right, FX_FLOAT bottom, FX_BOOL bContains)
{
    if (!m_CountBSArray) {
        return -1;
    }
    m_CountBSArray->RemoveAll();
    CFX_FloatRect floatrect(left, bottom, right, top);
    int totalcount, i, j = 0, counttmp = 0;
    FX_BOOL bstart = TRUE;
    FPDF_CHAR_INFO info;
    CFX_FloatRect recttmp;
    totalcount = CountChars();
    for(i = 0; i < totalcount; i++) {
        GetCharInfo(i, info);
        if(_IsIntersect(floatrect, info.m_CharBox)) {
            if(bstart) {
                m_CountBSArray->Add(i);
                counttmp = 1;
                recttmp = info.m_CharBox;
                bstart = FALSE;
            } else if(_IsInsameline(recttmp, info.m_CharBox)) {
                recttmp.right = info.m_CharBox.right;
                if(info.m_CharBox.top > recttmp.top) {
                    recttmp.top = info.m_CharBox.top;
                }
                if(info.m_CharBox.bottom < recttmp.bottom) {
                    recttmp.bottom = info.m_CharBox.bottom;
                }
                counttmp ++;
            } else {
                m_CountBSArray->Add(counttmp);
                m_CountBSArray->Add(i);
                counttmp = 1;
                j++;
                recttmp = info.m_CharBox;
            }
        }
    }
    m_CountBSArray->Add(counttmp);
    j++;
    return j;
}
void CRF_TextPage::GetBoundedSegment(int index, int& start, int& count) const
{
    if (!m_CountBSArray) {
        return;
    }
    if(m_CountBSArray->GetSize() <= index * 2) {
        start = 0;
        count = 0;
        return;
    }
    start = *(int *)m_CountBSArray->GetAt(index * 2);
    count = *(int *)m_CountBSArray->GetAt(index * 2 + 1);
}

int CRF_TextPage::GetWordBreak(int index, int direction) const
{
    return -1;
}
CFX_WideString CRF_TextPage::GetPageText(int start, int nCount ) const
{
    if(nCount == -1) {
        nCount = CountChars();
        start = 0;
    } else if(nCount < 1) {
        return L"";
    } else if(start >= CountChars()) {
        return L"";
    }
    int i, index = start + nCount;
    FPDF_CHAR_INFO info;
    CFX_WideString str;
    CFX_FloatRect recttmp;
    FX_BOOL bstart = TRUE;
    for(i = start; i < index; i++) {
        GetCharInfo(i, info);
        if(bstart) {
            recttmp = info.m_CharBox;
            str += info.m_Unicode;
            bstart = FALSE;
        } else if (_IsInsameline(recttmp, info.m_CharBox)) {
            str += info.m_Unicode;
        } else {
            str += L"\r\n";
            recttmp = info.m_CharBox;
            str += info.m_Unicode;
        }
    }
    if(str.IsEmpty()) {
        return L"";
    }
    return str;
}
