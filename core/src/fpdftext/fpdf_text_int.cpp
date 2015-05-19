// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfapi/fpdf_resource.h"
#include "../../include/fpdfapi/fpdf_pageobj.h"
#include "../../include/fpdftext/fpdf_text.h"
#include "../../include/fpdfapi/fpdf_page.h"
#include "../../include/fpdfapi/fpdf_module.h"
#include <ctype.h>
#include <algorithm>
#include "text_int.h"

namespace {

FX_BOOL _IsIgnoreSpaceCharacter(FX_WCHAR curChar)
{
    if(curChar < 255 ) {
        return FALSE;
    }
    if ( (curChar >= 0x0600 && curChar <= 0x06FF)
            || (curChar >= 0xFE70 && curChar <= 0xFEFF)
            || (curChar >= 0xFB50 && curChar <= 0xFDFF)
            || (curChar >= 0x0400 && curChar <= 0x04FF)
            || (curChar >= 0x0500 && curChar <= 0x052F)
            || (curChar >= 0xA640 && curChar <= 0xA69F)
            || (curChar >= 0x2DE0 && curChar <= 0x2DFF)
            || curChar == 8467
            || (curChar >= 0x2000 && curChar <= 0x206F)) {
        return FALSE;
    }
    return TRUE;
}

FX_FLOAT _NormalizeThreshold(FX_FLOAT threshold)
{
    if (threshold < 300) {
        return threshold / 2.0f;
    } else if (threshold < 500) {
        return threshold / 4.0f;
    } else if (threshold < 700) {
        return threshold / 5.0f;
    }
    return threshold / 6.0f;
}

FX_FLOAT _CalculateBaseSpace(const CPDF_TextObject* pTextObj,
                             const CFX_AffineMatrix& matrix)
{
    FX_FLOAT baseSpace = 0.0;
    const int nItems = pTextObj->CountItems();
    if (pTextObj->m_TextState.GetObject()->m_CharSpace && nItems >= 3) {
        FX_BOOL bAllChar = TRUE;
        FX_FLOAT spacing = matrix.TransformDistance(
            pTextObj->m_TextState.GetObject()->m_CharSpace);
        baseSpace = spacing;
        for (int i = 0; i < nItems; i++) {
            CPDF_TextObjectItem item;
            pTextObj->GetItemInfo(i, &item);
            if (item.m_CharCode == (FX_DWORD) - 1) {
                FX_FLOAT fontsize_h = pTextObj->m_TextState.GetFontSizeH();
                FX_FLOAT kerning = -fontsize_h * item.m_OriginX / 1000;
                baseSpace = std::min(baseSpace, kerning + spacing);
                bAllChar = FALSE;
            }
        }
        if (baseSpace < 0.0 || (nItems == 3 && !bAllChar)) {
            baseSpace = 0.0;
        }
    }
    return baseSpace;
}

}  // namespace

CPDFText_ParseOptions::CPDFText_ParseOptions()
    : m_bGetCharCodeOnly(FALSE), m_bNormalizeObjs(TRUE), m_bOutputHyphen(FALSE)
{
}
IPDF_TextPage* IPDF_TextPage::CreateTextPage(const CPDF_Page* pPage, CPDFText_ParseOptions ParserOptions)
{
    return new CPDF_TextPage(pPage, ParserOptions);
}
IPDF_TextPage* IPDF_TextPage::CreateTextPage(const CPDF_Page* pPage, int flags)
{
    return new CPDF_TextPage(pPage, flags);
}
IPDF_TextPage*	IPDF_TextPage::CreateTextPage(const CPDF_PageObjects* pObjs, int flags)
{
    return new CPDF_TextPage(pObjs, flags);
}
IPDF_TextPageFind*	IPDF_TextPageFind::CreatePageFind(const IPDF_TextPage* pTextPage)
{
    if (!pTextPage) {
        return NULL;
    }
    return new CPDF_TextPageFind(pTextPage);
}
IPDF_LinkExtract* IPDF_LinkExtract::CreateLinkExtract()
{
    return new CPDF_LinkExtract();
}
#define  TEXT_BLANK_CHAR		L' '
#define  TEXT_LINEFEED_CHAR		L'\n'
#define	 TEXT_RETURN_CHAR		L'\r'
#define  TEXT_EMPTY				L""
#define  TEXT_BLANK				L" "
#define  TEXT_RETURN_LINEFEED	L"\r\n"
#define  TEXT_LINEFEED			L"\n"
#define	 TEXT_CHARRATIO_GAPDELTA	0.070
CPDF_TextPage::CPDF_TextPage(const CPDF_Page* pPage, int flags)
    : m_charList(512),
      m_TempCharList(50),
      m_pPreTextObj(NULL),
      m_IsParsered(FALSE),
      m_TextlineDir(-1),
      m_CurlineRect(0, 0, 0, 0)
{
    m_pPage = pPage;
    m_parserflag = flags;
    m_TextBuf.EstimateSize(0, 10240);
    pPage->GetDisplayMatrix(m_DisplayMatrix, 0, 0, (int) pPage->GetPageWidth(), (int)pPage->GetPageHeight(), 0);
}
CPDF_TextPage::CPDF_TextPage(const CPDF_Page* pPage, CPDFText_ParseOptions ParserOptions)
    : m_ParseOptions(ParserOptions)
    , m_charList(512)
    , m_TempCharList(50)
    , m_pPreTextObj(NULL)
    , m_IsParsered(FALSE)
    , m_TextlineDir(-1)
    , m_CurlineRect(0, 0, 0, 0)
{
    m_pPage = pPage;
    m_parserflag = 0;
    m_TextBuf.EstimateSize(0, 10240);
    pPage->GetDisplayMatrix(m_DisplayMatrix, 0, 0, (int) pPage->GetPageWidth(), (int)pPage->GetPageHeight(), 0);
}
CPDF_TextPage::CPDF_TextPage(const CPDF_PageObjects* pPage, int flags)
    : m_charList(512),
      m_TempCharList(50),
      m_pPreTextObj(NULL),
      m_IsParsered(FALSE),
      m_TextlineDir(-1),
      m_CurlineRect(0, 0, 0, 0)
{
    m_pPage = pPage;
    m_parserflag = flags;
    m_TextBuf.EstimateSize(0, 10240);
    CFX_FloatRect pageRect = pPage->CalcBoundingBox();
    m_DisplayMatrix = CFX_AffineMatrix(1, 0, 0, -1, pageRect.right, pageRect.top);
}
void CPDF_TextPage::NormalizeObjects(FX_BOOL bNormalize)
{
    m_ParseOptions.m_bNormalizeObjs = bNormalize;
}
FX_BOOL CPDF_TextPage::IsControlChar(PAGECHAR_INFO* pCharInfo)
{
    if(!pCharInfo) {
        return FALSE;
    }
    switch(pCharInfo->m_Unicode) {
        case 0x2:
        case 0x3:
        case 0x93:
        case 0x94:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0xfffe:
            if(pCharInfo->m_Flag == FPDFTEXT_CHAR_HYPHEN) {
                return FALSE;
            } else {
                return TRUE;
            }
        default:
            return FALSE;
    }
}
FX_BOOL CPDF_TextPage::ParseTextPage()
{
    if (!m_pPage) {
        m_IsParsered = FALSE;
        return FALSE;
    }
    m_IsParsered = FALSE;
    m_TextBuf.Clear();
    m_charList.RemoveAll();
    m_pPreTextObj = NULL;
    ProcessObject();
    m_IsParsered = TRUE;
    if(!m_ParseOptions.m_bGetCharCodeOnly) {
        m_CharIndex.RemoveAll();
        int nCount = m_charList.GetSize();
        if(nCount) {
            m_CharIndex.Add(0);
        }
        for(int i = 0; i < nCount; i++) {
            int indexSize = m_CharIndex.GetSize();
            FX_BOOL bNormal = FALSE;
            PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(i);
            if(charinfo.m_Flag == FPDFTEXT_CHAR_GENERATED) {
                bNormal = TRUE;
            }
            else if(charinfo.m_Unicode == 0 || IsControlChar(&charinfo))
                bNormal = FALSE;
            else {
                bNormal = TRUE;
            }
            if(bNormal) {
                if(indexSize % 2) {
                    m_CharIndex.Add(1);
                } else {
                    if(indexSize <= 0) {
                        continue;
                    }
                    m_CharIndex.SetAt(indexSize - 1, m_CharIndex.GetAt(indexSize - 1) + 1);
                }
            } else {
                if(indexSize % 2) {
                    if(indexSize <= 0) {
                        continue;
                    }
                    m_CharIndex.SetAt(indexSize - 1, i + 1);
                } else {
                    m_CharIndex.Add(i + 1);
                }
            }
        }
        int indexSize = m_CharIndex.GetSize();
        if(indexSize % 2) {
            m_CharIndex.RemoveAt(indexSize - 1);
        }
    }
    return TRUE;
}
int	CPDF_TextPage::CountChars() const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return m_TextBuf.GetSize();
    }
    return m_charList.GetSize();
}
int CPDF_TextPage::CharIndexFromTextIndex(int TextIndex) const
{
    int indexSize = m_CharIndex.GetSize();
    int count = 0;
    for(int i = 0; i < indexSize; i += 2) {
        count += m_CharIndex.GetAt(i + 1);
        if(count > TextIndex) {
            return 	TextIndex - count + m_CharIndex.GetAt(i + 1) + m_CharIndex.GetAt(i);
        }
    }
    return -1;
}
int CPDF_TextPage::TextIndexFromCharIndex(int CharIndex) const
{
    int indexSize = m_CharIndex.GetSize();
    int count = 0;
    for(int i = 0; i < indexSize; i += 2) {
        count += m_CharIndex.GetAt(i + 1);
        if(m_CharIndex.GetAt(i + 1) + m_CharIndex.GetAt(i) > CharIndex) {
            if(CharIndex - m_CharIndex.GetAt(i) < 0) {
                return -1;
            }
            return 	CharIndex - m_CharIndex.GetAt(i) + count - m_CharIndex.GetAt(i + 1);
        }
    }
    return -1;
}
void CPDF_TextPage::GetRectArray(int start, int nCount, CFX_RectArray& rectArray) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return;
    }
    if(start < 0 || nCount == 0) {
        return;
    }
    if (!m_IsParsered)	{
        return;
    }
    PAGECHAR_INFO		info_curchar;
    CPDF_TextObject*	pCurObj = NULL;
    CFX_FloatRect		rect;
    int					curPos = start;
    FX_BOOL				flagNewRect = TRUE;
    if (nCount + start > m_charList.GetSize() || nCount == -1) {
        nCount = m_charList.GetSize() - start;
    }
    while (nCount--) {
        info_curchar = *(PAGECHAR_INFO*)m_charList.GetAt(curPos++);
        if (info_curchar.m_Flag == FPDFTEXT_CHAR_GENERATED) {
            continue;
        }
        if(info_curchar.m_CharBox.Width() < 0.01 || info_curchar.m_CharBox.Height() < 0.01) {
            continue;
        }
        if(!pCurObj) {
            pCurObj = info_curchar.m_pTextObj;
        }
        if (pCurObj != info_curchar.m_pTextObj) {
            rectArray.Add(rect);
            pCurObj = info_curchar.m_pTextObj;
            flagNewRect = TRUE;
        }
        if (flagNewRect) {
            FX_FLOAT orgX = info_curchar.m_OriginX, orgY = info_curchar.m_OriginY;
            CFX_AffineMatrix matrix, matrix_reverse;
            info_curchar.m_pTextObj->GetTextMatrix(&matrix);
            matrix.Concat(info_curchar.m_Matrix);
            matrix_reverse.SetReverse(matrix);
            matrix_reverse.Transform(orgX, orgY);
            rect.left = info_curchar.m_CharBox.left;
            rect.right = info_curchar.m_CharBox.right;
            if (pCurObj->GetFont()->GetTypeDescent()) {
                rect.bottom = orgY + pCurObj->GetFont()->GetTypeDescent() * pCurObj->GetFontSize() / 1000;
                FX_FLOAT xPosTemp = orgX;
                matrix.Transform(xPosTemp, rect.bottom);
            } else {
                rect.bottom = info_curchar.m_CharBox.bottom;
            }
            if (pCurObj->GetFont()->GetTypeAscent()) {
                rect.top = orgY + pCurObj->GetFont()->GetTypeAscent() * pCurObj->GetFontSize() / 1000;
                FX_FLOAT xPosTemp = orgX + GetCharWidth(info_curchar.m_CharCode, pCurObj->GetFont()) * pCurObj->GetFontSize() / 1000;
                matrix.Transform(xPosTemp, rect.top);
            } else {
                rect.top = info_curchar.m_CharBox.top;
            }
            flagNewRect = FALSE;
            rect = info_curchar.m_CharBox;
            rect.Normalize();
        } else {
            info_curchar.m_CharBox.Normalize();
            if (rect.left > info_curchar.m_CharBox.left) {
                rect.left = info_curchar.m_CharBox.left;
            }
            if (rect.right < info_curchar.m_CharBox.right) {
                rect.right = info_curchar.m_CharBox.right;
            }
            if ( rect.top < info_curchar.m_CharBox.top) {
                rect.top = info_curchar.m_CharBox.top;
            }
            if (rect.bottom > info_curchar.m_CharBox.bottom) {
                rect.bottom = info_curchar.m_CharBox.bottom;
            }
        }
    }
    rectArray.Add(rect);
    return;
}
int CPDF_TextPage::GetIndexAtPos(CPDF_Point point , FX_FLOAT xTorelance, FX_FLOAT yTorelance) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -3;
    }
    if (!m_IsParsered)	{
        return	-3;
    }
    int pos = 0;
    int NearPos = -1;
    double xdif = 5000, ydif = 5000;
    while(pos < m_charList.GetSize()) {
        PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)(m_charList.GetAt(pos));
        CFX_FloatRect charrect = charinfo.m_CharBox;
        if (charrect.Contains(point.x, point.y)) {
            break;
        }
        if (xTorelance > 0 || yTorelance > 0) {
            CFX_FloatRect charRectExt;
            charrect.Normalize();
            charRectExt.left = charrect.left - xTorelance / 2;
            charRectExt.right = charrect.right + xTorelance / 2;
            charRectExt.top = charrect.top + yTorelance / 2;
            charRectExt.bottom = charrect.bottom - yTorelance / 2;
            if (charRectExt.Contains(point.x, point.y)) {
                double curXdif, curYdif;
                curXdif = FXSYS_fabs(point.x - charrect.left) < FXSYS_fabs(point.x - charrect.right) ? FXSYS_fabs(point.x - charrect.left) : FXSYS_fabs(point.x - charrect.right);
                curYdif = FXSYS_fabs(point.y - charrect.bottom) < FXSYS_fabs(point.y - charrect.top	) ? FXSYS_fabs(point.y - charrect.bottom) : FXSYS_fabs(point.y - charrect.top);
                if (curYdif + curXdif < xdif + ydif) {
                    ydif = curYdif;
                    xdif = curXdif;
                    NearPos = pos;
                }
            }
        }
        ++pos;
    }
    if (pos >= m_charList.GetSize()) {
        pos = NearPos;
    }
    return pos;
}
CFX_WideString CPDF_TextPage::GetTextByRect(const CFX_FloatRect& rect) const
{
    CFX_WideString strText;
    if(m_ParseOptions.m_bGetCharCodeOnly || !m_IsParsered) {
        return strText;
    }
    int nCount = m_charList.GetSize();
    int pos = 0;
    FX_FLOAT posy = 0;
    FX_BOOL IsContainPreChar = FALSE;
    FX_BOOL	ISAddLineFeed = FALSE;
    while (pos < nCount) {
        PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(pos++);
        if (IsRectIntersect(rect, charinfo.m_CharBox)) {
            if (FXSYS_fabs(posy - charinfo.m_OriginY) > 0 && !IsContainPreChar && ISAddLineFeed) {
                posy = charinfo.m_OriginY;
                if (strText.GetLength() > 0) {
                    strText += L"\r\n";
                }
            }
            IsContainPreChar = TRUE;
            ISAddLineFeed = FALSE;
            if (charinfo.m_Unicode) {
                strText += charinfo.m_Unicode;
            }
        } else if (charinfo.m_Unicode == 32) {
            if (IsContainPreChar && charinfo.m_Unicode) {
                strText += charinfo.m_Unicode;
                IsContainPreChar = FALSE;
                ISAddLineFeed = FALSE;
            }
        } else {
            IsContainPreChar = FALSE;
            ISAddLineFeed = TRUE;
        }
    }
    return strText;
}
void CPDF_TextPage::GetRectsArrayByRect(const CFX_FloatRect& rect, CFX_RectArray& resRectArray) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return;
    }
    if (!m_IsParsered)	{
        return;
    }
    CFX_FloatRect		curRect;
    FX_BOOL				flagNewRect = TRUE;
    CPDF_TextObject*	pCurObj = NULL;
    int nCount = m_charList.GetSize();
    int pos = 0;
    while (pos < nCount) {
        PAGECHAR_INFO info_curchar = *(PAGECHAR_INFO*)m_charList.GetAt(pos++);
        if (info_curchar.m_Flag == FPDFTEXT_CHAR_GENERATED) {
            continue;
        }
        if (IsRectIntersect(rect, info_curchar.m_CharBox)) {
            if(!pCurObj) {
                pCurObj = info_curchar.m_pTextObj;
            }
            if (pCurObj != info_curchar.m_pTextObj) {
                resRectArray.Add(curRect);
                pCurObj = info_curchar.m_pTextObj;
                flagNewRect = TRUE;
            }
            if (flagNewRect) {
                curRect = info_curchar.m_CharBox;
                flagNewRect = FALSE;
                curRect.Normalize();
            } else {
                info_curchar.m_CharBox.Normalize();
                if (curRect.left > info_curchar.m_CharBox.left) {
                    curRect.left = info_curchar.m_CharBox.left;
                }
                if (curRect.right < info_curchar.m_CharBox.right) {
                    curRect.right = info_curchar.m_CharBox.right;
                }
                if ( curRect.top < info_curchar.m_CharBox.top) {
                    curRect.top = info_curchar.m_CharBox.top;
                }
                if (curRect.bottom > info_curchar.m_CharBox.bottom) {
                    curRect.bottom = info_curchar.m_CharBox.bottom;
                }
            }
        }
    }
    resRectArray.Add(curRect);
    return;
}
int	CPDF_TextPage::GetIndexAtPos(FX_FLOAT x, FX_FLOAT y, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -3;
    }
    CPDF_Point point(x, y);
    return GetIndexAtPos(point, xTorelance, yTorelance);
}
int CPDF_TextPage::GetOrderByDirection(int order, int direction) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -3;
    }
    if (!m_IsParsered) {
        return -3;
    }
    if (direction == FPDFTEXT_RIGHT || direction == FPDFTEXT_LEFT) {
        order += direction;
        while(order >= 0 && order < m_charList.GetSize()) {
            PAGECHAR_INFO cinfo = *(PAGECHAR_INFO*)m_charList.GetAt(order);
            if (cinfo.m_Flag != FPDFTEXT_CHAR_GENERATED) {
                break;
            } else {
                if (cinfo.m_Unicode == TEXT_LINEFEED_CHAR || cinfo.m_Unicode == TEXT_RETURN_CHAR) {
                    order += direction;
                } else {
                    break;
                }
            }
        }
        if (order >= m_charList.GetSize()) {
            order = -2;
        }
        return order;
    }
    PAGECHAR_INFO charinfo;
    charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(order);
    CPDF_Point curPos(charinfo.m_OriginX, charinfo.m_OriginY);
    FX_FLOAT difPosY = 0.0, minXdif = 1000;
    int	minIndex = -2;
    int index = order;
    FX_FLOAT height = charinfo.m_CharBox.Height();
    if (direction == FPDFTEXT_UP) {
        minIndex = -1;
        while (1) {
            if (--index < 0)	{
                return -1;
            }
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
            if (FXSYS_fabs(charinfo.m_OriginY - curPos.y) > FX_MAX(height, charinfo.m_CharBox.Height()) / 2) {
                difPosY = charinfo.m_OriginY;
                minIndex = index;
                break;
            }
        }
        FX_FLOAT PreXdif = charinfo.m_OriginX - curPos.x;
        minXdif = PreXdif;
        if (PreXdif == 0)	{
            return index;
        }
        FX_FLOAT curXdif = 0;
        while (--index >= 0) {
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
            if (difPosY != charinfo.m_OriginY) {
                break;
            }
            curXdif = charinfo.m_OriginX - curPos.x;
            if (curXdif == 0) {
                return index;
            }
            int signflag = 0;
            if (curXdif > 0) {
                signflag = 1;
            } else {
                signflag = -1;
            }
            if (signflag * PreXdif < 0) {
                if (FXSYS_fabs(PreXdif) < FXSYS_fabs(curXdif)) {
                    return index + 1;
                } else {
                    return index;
                }
            }
            if (FXSYS_fabs(curXdif) < FXSYS_fabs(minXdif)) {
                minIndex = index;
                minXdif = curXdif;
            }
            PreXdif = curXdif;
            if (difPosY != charinfo.m_OriginY) {
                break;
            }
        }
        return minIndex;
    } else if(FPDFTEXT_DOWN) {
        minIndex = -2;
        while (1) {
            if (++index > m_charList.GetSize() - 1)	{
                return minIndex;
            }
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
            if (FXSYS_fabs(charinfo.m_OriginY - curPos.y) > FX_MAX(height, charinfo.m_CharBox.Height()) / 2) {
                difPosY = charinfo.m_OriginY;
                minIndex = index;
                break;
            }
        }
        FX_FLOAT PreXdif = charinfo.m_OriginX - curPos.x;
        minXdif = PreXdif;
        if (PreXdif == 0)	{
            return index;
        }
        FX_FLOAT curXdif = 0;
        while (++index < m_charList.GetSize()) {
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
            if (difPosY != charinfo.m_OriginY) {
                break;
            }
            curXdif = charinfo.m_OriginX - curPos.x;
            if (curXdif == 0) {
                return index;
            }
            int signflag = 0;
            if (curXdif > 0) {
                signflag = 1;
            } else {
                signflag = -1;
            }
            if (signflag * PreXdif < 0) {
                if (FXSYS_fabs(PreXdif) < FXSYS_fabs(curXdif)) {
                    return index - 1;
                } else {
                    return index;
                }
            }
            if (FXSYS_fabs(curXdif) < FXSYS_fabs(minXdif)) {
                minXdif = curXdif;
                minIndex = index;
            }
            PreXdif = curXdif;
        }
        return minIndex;
    }
}
void CPDF_TextPage::GetCharInfo(int index, FPDF_CHAR_INFO & info) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return;
    }
    if (!m_IsParsered)	{
        return;
    }
    if (index < 0 || index >= m_charList.GetSize())	{
        return;
    }
    PAGECHAR_INFO charinfo;
    charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
    info.m_Charcode = charinfo.m_CharCode;
    info.m_OriginX = charinfo.m_OriginX;
    info.m_OriginY = charinfo.m_OriginY;
    info.m_Unicode = charinfo.m_Unicode;
    info.m_Flag = charinfo.m_Flag;
    info.m_CharBox = charinfo.m_CharBox;
    info.m_pTextObj = charinfo.m_pTextObj;
    if (charinfo.m_pTextObj && charinfo.m_pTextObj->GetFont()) {
        info.m_FontSize = charinfo.m_pTextObj->GetFontSize();
    }
    info.m_Matrix.Copy(charinfo.m_Matrix);
    return;
}
void CPDF_TextPage::CheckMarkedContentObject(FX_INT32& start, FX_INT32& nCount) const
{
    PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(start);
    PAGECHAR_INFO charinfo2 = *(PAGECHAR_INFO*)m_charList.GetAt(start + nCount - 1);
    if (FPDFTEXT_CHAR_PIECE != charinfo.m_Flag && FPDFTEXT_CHAR_PIECE != charinfo2.m_Flag) {
        return;
    }
    if (FPDFTEXT_CHAR_PIECE == charinfo.m_Flag) {
        PAGECHAR_INFO charinfo1 = charinfo;
        int startIndex = start;
        while(FPDFTEXT_CHAR_PIECE == charinfo1.m_Flag && charinfo1.m_Index == charinfo.m_Index) {
            startIndex--;
            if (startIndex < 0)	{
                break;
            }
            charinfo1 = *(PAGECHAR_INFO*)m_charList.GetAt(startIndex);
        }
        startIndex++;
        start = startIndex;
    }
    if (FPDFTEXT_CHAR_PIECE == charinfo2.m_Flag) {
        PAGECHAR_INFO charinfo3 = charinfo2;
        int endIndex = start + nCount - 1;
        while(FPDFTEXT_CHAR_PIECE == charinfo3.m_Flag && charinfo3.m_Index == charinfo2.m_Index) {
            endIndex++;
            if (endIndex >= m_charList.GetSize())	{
                break;
            }
            charinfo3 = *(PAGECHAR_INFO*)m_charList.GetAt(endIndex);
        }
        endIndex--;
        nCount = endIndex - start + 1;
    }
}
CFX_WideString CPDF_TextPage::GetPageText(int start , int nCount) const
{
    if (!m_IsParsered || nCount == 0) {
        return L"";
    }
    if (start < 0) {
        start = 0;
    }
    if	(nCount == -1) {
        nCount = m_charList.GetSize() - start;
        return m_TextBuf.GetWideString().Mid(start, m_TextBuf.GetWideString().GetLength());
    }
    if(nCount <= 0 || m_charList.GetSize() <= 0) {
        return L"";
    }
    if(nCount + start > m_charList.GetSize() - 1) {
        nCount = m_charList.GetSize() - start;
    }
    if (nCount <= 0) {
        return L"";
    }
    CheckMarkedContentObject(start, nCount);
    int startindex = 0;
    PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(start);
    int startOffset = 0;
    while(charinfo.m_Index == -1) {
        startOffset++;
        if (startOffset > nCount || start + startOffset >= m_charList.GetSize())	{
            return L"";
        }
        charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(start + startOffset);
    }
    startindex = charinfo.m_Index;
    charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(start + nCount - 1);
    int nCountOffset = 0;
    while (charinfo.m_Index == -1) {
        nCountOffset++;
        if (nCountOffset >= nCount) {
            return L"";
        }
        charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(start + nCount - nCountOffset - 1);
    }
    nCount = start + nCount - nCountOffset - startindex;
    if(nCount <= 0) {
        return L"";
    }
    return m_TextBuf.GetWideString().Mid(startindex, nCount);
}
int CPDF_TextPage::CountRects(int start, int nCount)
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -1;
    }
    if (!m_IsParsered)	{
        return -1;
    }
    if (start < 0) {
        return -1;
    }
    if (nCount == -1 || nCount + start > m_charList.GetSize() ) {
        nCount = m_charList.GetSize() - start;
    }
    m_SelRects.RemoveAll();
    GetRectArray(start, nCount, m_SelRects);
    return m_SelRects.GetSize();
}
void CPDF_TextPage::GetRect(int rectIndex, FX_FLOAT& left, FX_FLOAT& top, FX_FLOAT& right, FX_FLOAT &bottom) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return ;
    }
    if (!m_IsParsered || rectIndex < 0 || rectIndex >= m_SelRects.GetSize()) {
        return;
    }
    left = m_SelRects.GetAt(rectIndex).left;
    top = m_SelRects.GetAt(rectIndex).top;
    right = m_SelRects.GetAt(rectIndex).right;
    bottom = m_SelRects.GetAt(rectIndex).bottom;
}
FX_BOOL CPDF_TextPage::GetBaselineRotate(int start, int end, int& Rotate)
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return FALSE;
    }
    if(end == start) {
        return FALSE;
    }
    FX_FLOAT dx, dy;
    FPDF_CHAR_INFO info1, info2;
    GetCharInfo(start, info1);
    GetCharInfo(end, info2);
    while(info2.m_CharBox.Width() == 0 || info2.m_CharBox.Height() == 0) {
        end--;
        if(end <= start) {
            return FALSE;
        }
        GetCharInfo(end, info2);
    }
    dx = (info2.m_OriginX - info1.m_OriginX);
    dy = (info2.m_OriginY - info1.m_OriginY);
    if(dx == 0) {
        if(dy > 0) {
            Rotate = 90;
        } else if (dy < 0) {
            Rotate = 270;
        } else {
            Rotate = 0;
        }
    } else {
        float a = FXSYS_atan2(dy, dx);
        Rotate = (int)(a * 180 / FX_PI + 0.5);
    }
    if(Rotate < 0) {
        Rotate = -Rotate;
    } else if(Rotate > 0) {
        Rotate = 360 - Rotate;
    }
    return TRUE;
}
FX_BOOL	CPDF_TextPage::GetBaselineRotate(const CFX_FloatRect& rect , int& Rotate)
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return FALSE;
    }
    int start, end, count, n = CountBoundedSegments(rect.left, rect.top, rect.right, rect.bottom, TRUE);
    if(n < 1) {
        return FALSE;
    }
    if(n > 1) {
        GetBoundedSegment(n - 1, start, count);
        end = start + count - 1;
        GetBoundedSegment(0, start, count);
    } else {
        GetBoundedSegment(0, start, count);
        end = start + count - 1;
    }
    return GetBaselineRotate(start, end, Rotate);
}
FX_BOOL	CPDF_TextPage::GetBaselineRotate(int rectIndex, int& Rotate)
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return FALSE;
    }
    if (!m_IsParsered || rectIndex < 0 || rectIndex > m_SelRects.GetSize()) {
        return FALSE;
    }
    CFX_FloatRect rect = m_SelRects.GetAt(rectIndex);
    return GetBaselineRotate(rect , Rotate);
}
int	CPDF_TextPage::CountBoundedSegments(FX_FLOAT left, FX_FLOAT top, FX_FLOAT right, FX_FLOAT bottom, FX_BOOL bContains )
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -1;
    }
    m_Segment.RemoveAll();
    if (!m_IsParsered)	{
        return -1;
    }
    CFX_FloatRect rect(left, bottom, right, top);
    rect.Normalize();
    int nCount = m_charList.GetSize();
    int pos = 0;
    FPDF_SEGMENT	segment;
    segment.m_Start = 0;
    segment.m_nCount = 0;
    FX_BOOL		segmentStatus = 0;
    FX_BOOL		IsContainPreChar = FALSE;
    while (pos < nCount) {
        PAGECHAR_INFO charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(pos);
        if(bContains && rect.Contains(charinfo.m_CharBox)) {
            if (segmentStatus == 0 || segmentStatus == 2) {
                segment.m_Start = pos;
                segment.m_nCount = 1;
                segmentStatus = 1;
            } else if (segmentStatus == 1) {
                segment.m_nCount++;
            }
            IsContainPreChar = TRUE;
        } else if (!bContains && (IsRectIntersect(rect, charinfo.m_CharBox) || rect.Contains(charinfo.m_OriginX, charinfo.m_OriginY))) {
            if (segmentStatus == 0 || segmentStatus == 2) {
                segment.m_Start = pos;
                segment.m_nCount = 1;
                segmentStatus = 1;
            } else if (segmentStatus == 1) {
                segment.m_nCount++;
            }
            IsContainPreChar = TRUE;
        } else if (charinfo.m_Unicode == 32) {
            if (IsContainPreChar == TRUE) {
                if (segmentStatus == 0 || segmentStatus == 2) {
                    segment.m_Start = pos;
                    segment.m_nCount = 1;
                    segmentStatus = 1;
                } else if (segmentStatus == 1) {
                    segment.m_nCount++;
                }
                IsContainPreChar = FALSE;
            } else {
                if (segmentStatus == 1) {
                    segmentStatus = 2;
                    m_Segment.Add(segment);
                    segment.m_Start = 0;
                    segment.m_nCount = 0;
                }
            }
        } else {
            if (segmentStatus == 1) {
                segmentStatus = 2;
                m_Segment.Add(segment);
                segment.m_Start = 0;
                segment.m_nCount = 0;
            }
            IsContainPreChar = FALSE;
        }
        pos++;
    }
    if (segmentStatus == 1) {
        segmentStatus = 2;
        m_Segment.Add(segment);
        segment.m_Start = 0;
        segment.m_nCount = 0;
    }
    return m_Segment.GetSize();
}
void CPDF_TextPage::GetBoundedSegment(int index, int& start, int& count) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return ;
    }
    if (index < 0 || index >= m_Segment.GetSize()) {
        return;
    }
    start = m_Segment.GetAt(index).m_Start;
    count = m_Segment.GetAt(index).m_nCount;
}
int CPDF_TextPage::GetWordBreak(int index, int direction) const
{
    if(m_ParseOptions.m_bGetCharCodeOnly) {
        return -1;
    }
    if (!m_IsParsered)	{
        return -1;
    }
    if (direction != FPDFTEXT_LEFT && direction != FPDFTEXT_RIGHT) {
        return -1;
    }
    if (index < 0 || index >= m_charList.GetSize()) {
        return -1;
    }
    PAGECHAR_INFO charinfo;
    charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(index);
    if (charinfo.m_Index == -1 || charinfo.m_Flag == FPDFTEXT_CHAR_GENERATED)	{
        return index;
    }
    if (!IsLetter(charinfo.m_Unicode)) {
        return index;
    }
    int breakPos = index;
    if (direction == FPDFTEXT_LEFT) {
        while (--breakPos > 0) {
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(breakPos);
            if (!IsLetter(charinfo.m_Unicode)) {
                return breakPos;
            }
        }
        return breakPos;
    } else if (direction == FPDFTEXT_RIGHT) {
        while (++breakPos < m_charList.GetSize()) {
            charinfo = *(PAGECHAR_INFO*)m_charList.GetAt(breakPos);
            if (!IsLetter(charinfo.m_Unicode)) {
                return breakPos;
            }
        }
        return breakPos;
    }
    return breakPos;
}
FX_INT32 CPDF_TextPage::FindTextlineFlowDirection()
{
    if (!m_pPage)	{
        return -1;
    }
    const FX_INT32 nPageWidth = (FX_INT32)((CPDF_Page*)m_pPage)->GetPageWidth();
    const FX_INT32 nPageHeight = (FX_INT32)((CPDF_Page*)m_pPage)->GetPageHeight();
    CFX_ByteArray nHorizontalMask;
    if (!nHorizontalMask.SetSize(nPageWidth)) {
        return -1;
    }
    FX_BYTE* pDataH = nHorizontalMask.GetData();
    CFX_ByteArray nVerticalMask;
    if (!nVerticalMask.SetSize(nPageHeight)) {
        return -1;
    }
    FX_BYTE* pDataV = nVerticalMask.GetData();
    FX_INT32 index = 0;
    FX_FLOAT fLineHeight = 0.0f;
    CPDF_PageObject* pPageObj = NULL;
    FX_POSITION	pos = NULL;
    pos = m_pPage->GetFirstObjectPosition();
    if(!pos) {
        return -1;
    }
    while(pos) {
        pPageObj = m_pPage->GetNextObject(pos);
        if(NULL == pPageObj) {
            continue;
        }
        if(PDFPAGE_TEXT != pPageObj->m_Type) {
            continue;
        }
        FX_INT32 minH = (FX_INT32)pPageObj->m_Left < 0 ? 0 : (FX_INT32)pPageObj->m_Left;
        FX_INT32 maxH = (FX_INT32)pPageObj->m_Right > nPageWidth ? nPageWidth : (FX_INT32)pPageObj->m_Right;
        FX_INT32 minV = (FX_INT32)pPageObj->m_Bottom < 0 ? 0 : (FX_INT32)pPageObj->m_Bottom;
        FX_INT32 maxV = (FX_INT32)pPageObj->m_Top > nPageHeight ? nPageHeight : (FX_INT32)pPageObj->m_Top;
        if (minH >= maxH || minV >= maxV) {
            continue;
        }
        FXSYS_memset8(pDataH + minH, 1, maxH - minH);
        FXSYS_memset8(pDataV + minV, 1, maxV - minV);
        if (fLineHeight <= 0.0f) {
            fLineHeight = pPageObj->m_Top - pPageObj->m_Bottom;
        }
        pPageObj = NULL;
    }
    FX_INT32 nStartH = 0;
    FX_INT32 nEndH = 0;
    FX_FLOAT nSumH = 0.0f;
    for (index = 0; index < nPageWidth; index++)
        if(1 == nHorizontalMask[index]) {
            break;
        }
    nStartH = index;
    for (index = nPageWidth; index > 0; index--)
        if(1 == nHorizontalMask[index - 1]) {
            break;
        }
    nEndH = index;
    for (index = nStartH; index < nEndH; index++) {
        nSumH += nHorizontalMask[index];
    }
    nSumH /= nEndH - nStartH;
    FX_INT32 nStartV = 0;
    FX_INT32 nEndV = 0;
    FX_FLOAT nSumV = 0.0f;
    for (index = 0; index < nPageHeight; index++)
        if(1 == nVerticalMask[index]) {
            break;
        }
    nStartV = index;
    for (index = nPageHeight; index > 0; index--)
        if(1 == nVerticalMask[index - 1]) {
            break;
        }
    nEndV = index;
    for (index = nStartV; index < nEndV; index++) {
        nSumV += nVerticalMask[index];
    }
    nSumV /= nEndV - nStartV;
    if ((nEndV - nStartV) < (FX_INT32)(2 * fLineHeight)) {
        return 0;
    }
    if ((nEndH - nStartH) < (FX_INT32)(2 * fLineHeight)) {
        return 1;
    }
    if (nSumH > 0.8f) {
        return 0;
    }
    if (nSumH - nSumV > 0.0f) {
        return 0;
    }
    if (nSumV - nSumH > 0.0f) {
        return 1;
    }
    return -1;
}
void CPDF_TextPage::ProcessObject()
{
    CPDF_PageObject*	pPageObj = NULL;
    if (!m_pPage)	{
        return;
    }
    FX_POSITION	pos;
    pos = m_pPage->GetFirstObjectPosition();
    if (!pos)	{
        return;
    }
    m_TextlineDir = FindTextlineFlowDirection();
    int nCount = 0;
    while (pos) {
        pPageObj = m_pPage->GetNextObject(pos);
        if(pPageObj) {
            if(pPageObj->m_Type == PDFPAGE_TEXT) {
                CFX_AffineMatrix matrix;
                ProcessTextObject((CPDF_TextObject*)pPageObj, matrix, pos);
                nCount++;
            } else if (pPageObj->m_Type == PDFPAGE_FORM) {
                CFX_AffineMatrix formMatrix(1, 0, 0, 1, 0, 0);
                ProcessFormObject((CPDF_FormObject*)pPageObj, formMatrix);
            }
        }
        pPageObj = NULL;
    }
    int count = m_LineObj.GetSize();
    for(int i = 0; i < count; i++) {
        ProcessTextObject(m_LineObj.GetAt(i));
    }
    m_LineObj.RemoveAll();
    CloseTempLine();
}
void CPDF_TextPage::ProcessFormObject(CPDF_FormObject* pFormObj, const CFX_AffineMatrix& formMatrix)
{
    CPDF_PageObject*	pPageObj = NULL;
    FX_POSITION	pos;
    if (!pFormObj)	{
        return;
    }
    pos = pFormObj->m_pForm->GetFirstObjectPosition();
    if (!pos)	{
        return;
    }
    CFX_AffineMatrix curFormMatrix;
    curFormMatrix.Copy(pFormObj->m_FormMatrix);
    curFormMatrix.Concat(formMatrix);
    while (pos) {
        pPageObj = pFormObj->m_pForm->GetNextObject(pos);
        if(pPageObj) {
            if(pPageObj->m_Type == PDFPAGE_TEXT) {
                ProcessTextObject((CPDF_TextObject*)pPageObj, curFormMatrix, pos);
            } else if (pPageObj->m_Type == PDFPAGE_FORM) {
                ProcessFormObject((CPDF_FormObject*)pPageObj, curFormMatrix);
            }
        }
        pPageObj = NULL;
    }
}
int CPDF_TextPage::GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont) const
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
void CPDF_TextPage::OnPiece(IFX_BidiChar* pBidi, CFX_WideString& str)
{
    FX_INT32 start, count;
    FX_INT32 ret = pBidi->GetBidiInfo(start, count);
    if(ret == 2) {
        for(int i = start + count - 1; i >= start; i--) {
            m_TextBuf.AppendChar(str.GetAt(i));
            m_charList.Add(*(PAGECHAR_INFO*)m_TempCharList.GetAt(i));
        }
    } else {
        int end = start + count ;
        for(int i = start; i < end; i++) {
            m_TextBuf.AppendChar(str.GetAt(i));
            m_charList.Add(*(PAGECHAR_INFO*)m_TempCharList.GetAt(i));
        }
    }
}
void CPDF_TextPage::AddCharInfoByLRDirection(CFX_WideString& str, int i)
{
    PAGECHAR_INFO Info = *(PAGECHAR_INFO*)m_TempCharList.GetAt(i);
    FX_WCHAR wChar = str.GetAt(i);
    if(!IsControlChar(&Info)) {
        Info.m_Index = m_TextBuf.GetLength();
        if (wChar >= 0xFB00 && wChar <= 0xFB06) {
            FX_LPWSTR pDst = NULL;
            FX_STRSIZE nCount = FX_Unicode_GetNormalization(wChar, pDst);
            if (nCount >= 1) {
                pDst = FX_Alloc(FX_WCHAR, nCount);
                FX_Unicode_GetNormalization(wChar, pDst);
                for (int nIndex = 0; nIndex < nCount; nIndex++) {
                    PAGECHAR_INFO Info2 = Info;
                    Info2.m_Unicode = pDst[nIndex];
                    Info2.m_Flag = FPDFTEXT_CHAR_PIECE;
                    m_TextBuf.AppendChar(Info2.m_Unicode);
                    if( !m_ParseOptions.m_bGetCharCodeOnly) {
                        m_charList.Add(Info2);
                    }
                }
                FX_Free(pDst);
                return;
            }
        }
        m_TextBuf.AppendChar(wChar);
    } else {
        Info.m_Index = -1;
    }
    if( !m_ParseOptions.m_bGetCharCodeOnly) {
        m_charList.Add(Info);
    }
}
void CPDF_TextPage::AddCharInfoByRLDirection(CFX_WideString& str, int i)
{
    PAGECHAR_INFO Info = *(PAGECHAR_INFO*)m_TempCharList.GetAt(i);
    if(!IsControlChar(&Info)) {
        Info.m_Index = m_TextBuf.GetLength();
        FX_WCHAR wChar = FX_GetMirrorChar(str.GetAt(i), TRUE, FALSE);
        FX_LPWSTR pDst = NULL;
        FX_STRSIZE nCount = FX_Unicode_GetNormalization(wChar, pDst);
        if (nCount >= 1) {
            pDst = FX_Alloc(FX_WCHAR, nCount);
            FX_Unicode_GetNormalization(wChar, pDst);
            for (int nIndex = 0; nIndex < nCount; nIndex++) {
                PAGECHAR_INFO Info2 = Info;
                Info2.m_Unicode = pDst[nIndex];
                Info2.m_Flag = FPDFTEXT_CHAR_PIECE;
                m_TextBuf.AppendChar(Info2.m_Unicode);
                if( !m_ParseOptions.m_bGetCharCodeOnly) {
                    m_charList.Add(Info2);
                }
            }
            FX_Free(pDst);
            return;
        } else {
            Info.m_Unicode = wChar;
        }
        m_TextBuf.AppendChar(Info.m_Unicode);
    } else {
        Info.m_Index = -1;
    }
    if( !m_ParseOptions.m_bGetCharCodeOnly) {
        m_charList.Add(Info);
    }
}
void CPDF_TextPage::CloseTempLine()
{
    int count1 = m_TempCharList.GetSize();
    if (count1 <= 0) {
        return;
    }
    IFX_BidiChar* BidiChar = IFX_BidiChar::Create();
    CFX_WideString str = m_TempTextBuf.GetWideString();
    CFX_WordArray order;
    FX_BOOL bR2L = FALSE;
    FX_INT32 start = 0, count = 0;
    int nR2L = 0, nL2R = 0;
    FX_BOOL bPrevSpace = FALSE;
    for (int i = 0; i < str.GetLength(); i++) {
        if(str.GetAt(i) == 32) {
            if(bPrevSpace) {
                m_TempTextBuf.Delete(i, 1);
                m_TempCharList.Delete(i);
                str.Delete(i);
                count1--;
                i--;
                continue;
            }
            bPrevSpace = TRUE;
        } else {
            bPrevSpace = FALSE;
        }
        if(BidiChar && BidiChar->AppendChar(str.GetAt(i))) {
            FX_INT32 ret = BidiChar->GetBidiInfo(start, count);
            order.Add(start);
            order.Add(count);
            order.Add(ret);
            if(!bR2L) {
                if(ret == 2) {
                    nR2L++;
                } else if (ret == 1) {
                    nL2R++;
                }
            }
        }
    }
    if(BidiChar && BidiChar->EndChar()) {
        FX_INT32 ret = BidiChar->GetBidiInfo(start, count);
        order.Add(start);
        order.Add(count);
        order.Add(ret);
        if(!bR2L) {
            if(ret == 2) {
                nR2L++;
            } else if(ret == 1) {
                nL2R++;
            }
        }
    }
    if(nR2L > 0 && nR2L >= nL2R) {
        bR2L = TRUE;
    }
    if(this->m_parserflag == FPDFTEXT_RLTB || bR2L) {
        int count = order.GetSize();
        for(int i = count - 1; i > 0; i -= 3) {
            int ret = order.GetAt(i);
            int start = order.GetAt(i - 2);
            int count1 = order.GetAt(i - 1);
            if(ret == 2 || ret == 0) {
                for(int j = start + count1 - 1; j >= start; j--) {
                    AddCharInfoByRLDirection(str, j);
                }
            } else {
                int j = i;
                FX_BOOL bSymbol = FALSE;
                while(j > 0 && order.GetAt(j) != 2) {
                    bSymbol = !order.GetAt(j);
                    j -= 3;
                }
                int end = start + count1 ;
                int n = 0;
                if(bSymbol) {
                    n = j + 6;
                } else {
                    n = j + 3;
                }
                if(n >= i) {
                    for(int m = start; m < end; m++) {
                        AddCharInfoByLRDirection(str, m);
                    }
                } else {
                    j = i;
                    i = n;
                    for(; n <= j; n += 3) {
                        int start = order.GetAt(n - 2);
                        int count1 = order.GetAt(n - 1);
                        int end = start + count1 ;
                        for(int m = start; m < end; m++) {
                            AddCharInfoByLRDirection(str, m);
                        }
                    }
                }
            }
        }
    } else {
        int count = order.GetSize();
        FX_BOOL bL2R = FALSE;
        for(int i = 0; i < count; i += 3) {
            int ret = order.GetAt(i + 2);
            int start = order.GetAt(i);
            int count1 = order.GetAt(i + 1);
            if(ret == 2 || (i == 0 && ret == 0 && !bL2R)) {
                int j = i + 3;
                while(bR2L && j < count) {
                    if(order.GetAt(j + 2) == 1) {
                        break;
                    } else {
                        j += 3;
                    }
                }
                if(j == 3) {
                    i = -3;
                    bL2R = TRUE;
                    continue;
                }
                int end = m_TempCharList.GetSize() - 1;
                if(j < count) {
                    end = order.GetAt(j) - 1;
                }
                i = j - 3;
                for(int n = end; n >= start; n--) {
                    AddCharInfoByRLDirection(str, n);
                }
            } else {
                int end = start + count1 ;
                for(int n = start; n < end; n++) {
                    AddCharInfoByLRDirection(str, n);
                }
            }
        }
    }
    order.RemoveAll();
    m_TempCharList.RemoveAll();
    m_TempTextBuf.Delete(0, m_TempTextBuf.GetLength());
    BidiChar->Release();
}
void CPDF_TextPage::ProcessTextObject(CPDF_TextObject*	pTextObj, const CFX_AffineMatrix& formMatrix, FX_POSITION ObjPos)
{
    CFX_FloatRect re(pTextObj->m_Left, pTextObj->m_Bottom, pTextObj->m_Right, pTextObj->m_Top);
    if(FXSYS_fabs(pTextObj->m_Right - pTextObj->m_Left) < 0.01f ) {
        return;
    }
    int count = m_LineObj.GetSize();
    PDFTEXT_Obj Obj;
    Obj.m_pTextObj = pTextObj;
    Obj.m_formMatrix = formMatrix;
    if(count == 0) {
        m_LineObj.Add(Obj);
        return;
    }
    if (IsSameAsPreTextObject(pTextObj, ObjPos)) {
        return;
    }
    PDFTEXT_Obj prev_Obj = m_LineObj.GetAt(count - 1);
    CPDF_TextObjectItem item;
    int nItem = prev_Obj.m_pTextObj->CountItems();
    prev_Obj.m_pTextObj->GetItemInfo(nItem - 1, &item);
    FX_FLOAT prev_width = GetCharWidth(item.m_CharCode, prev_Obj.m_pTextObj->GetFont()) * prev_Obj.m_pTextObj->GetFontSize() / 1000;
    CFX_AffineMatrix prev_matrix;
    prev_Obj.m_pTextObj->GetTextMatrix(&prev_matrix);
    prev_width = FXSYS_fabs(prev_width);
    prev_matrix.Concat(prev_Obj.m_formMatrix);
    prev_width = prev_matrix.TransformDistance(prev_width);
    pTextObj->GetItemInfo(0, &item);
    FX_FLOAT this_width = GetCharWidth(item.m_CharCode, pTextObj->GetFont()) * pTextObj->GetFontSize() / 1000;
    this_width = FXSYS_fabs(this_width);
    CFX_AffineMatrix this_matrix;
    pTextObj->GetTextMatrix(&this_matrix);
    this_width = FXSYS_fabs(this_width);
    this_matrix.Concat(formMatrix);
    this_width = this_matrix.TransformDistance(this_width);
    FX_FLOAT threshold = prev_width > this_width ? prev_width / 4 : this_width / 4;
    FX_FLOAT prev_x = prev_Obj.m_pTextObj->GetPosX(), prev_y = prev_Obj.m_pTextObj->GetPosY();
    prev_Obj.m_formMatrix.Transform(prev_x, prev_y);
    m_DisplayMatrix.Transform(prev_x, prev_y);
    FX_FLOAT this_x = pTextObj->GetPosX(), this_y = pTextObj->GetPosY();
    formMatrix.Transform(this_x, this_y);
    m_DisplayMatrix.Transform(this_x, this_y);
    if (FXSYS_fabs(this_y - prev_y) > threshold * 2) {
        for(int i = 0; i < count; i++) {
            ProcessTextObject(m_LineObj.GetAt(i));
        }
        m_LineObj.RemoveAll();
        m_LineObj.Add(Obj);
        return;
    }
    int i = 0;
    if(m_ParseOptions.m_bNormalizeObjs) {
        for(i = count - 1; i >= 0; i--) {
            PDFTEXT_Obj prev_Obj = m_LineObj.GetAt(i);
            CFX_AffineMatrix prev_matrix;
            prev_Obj.m_pTextObj->GetTextMatrix(&prev_matrix);
            FX_FLOAT Prev_x = prev_Obj.m_pTextObj->GetPosX(), Prev_y = prev_Obj.m_pTextObj->GetPosY();
            prev_Obj.m_formMatrix.Transform(Prev_x, Prev_y);
            m_DisplayMatrix.Transform(Prev_x, Prev_y);
            if(this_x >= Prev_x) {
                if(i == count - 1) {
                    m_LineObj.Add(Obj);
                } else {
                    m_LineObj.InsertAt(i + 1, Obj);
                }
                break;
            }
        }
        if(i < 0) {
            m_LineObj.InsertAt(0, Obj);
        }
    } else {
        m_LineObj.Add(Obj);
    }
}
FX_INT32 CPDF_TextPage::PreMarkedContent(PDFTEXT_Obj Obj)
{
    CPDF_TextObject* pTextObj = Obj.m_pTextObj;
    CPDF_ContentMarkData* pMarkData = (CPDF_ContentMarkData*)pTextObj->m_ContentMark.GetObject();
    if(!pMarkData) {
        return FPDFTEXT_MC_PASS;
    }
    int nContentMark = pMarkData->CountItems();
    if (nContentMark < 1) {
        return FPDFTEXT_MC_PASS;
    }
    CFX_WideString actText;
    FX_BOOL bExist = FALSE;
    CPDF_Dictionary* pDict = NULL;
    int n = 0;
    for (n = 0; n < nContentMark; n++) {
        CPDF_ContentMarkItem& item = pMarkData->GetItem(n);
        CFX_ByteString tagStr = (CFX_ByteString)item.GetName();
        pDict = (CPDF_Dictionary*)item.GetParam();
        CPDF_String* temp = (CPDF_String*)(pDict ? pDict->GetElement(FX_BSTRC("ActualText")) : NULL);
        if (temp) {
            bExist = TRUE;
            actText = temp->GetUnicodeText();
        }
    }
    if (!bExist) {
        return FPDFTEXT_MC_PASS;
    }
    if (m_pPreTextObj) {
        if (CPDF_ContentMarkData* pPreMarkData = (CPDF_ContentMarkData*)m_pPreTextObj->m_ContentMark.GetObject()) {
            if (pPreMarkData->CountItems() == n) {
                CPDF_ContentMarkItem& item = pPreMarkData->GetItem(n - 1);
                if (pDict == item.GetParam()) {
                    return FPDFTEXT_MC_DONE;
                }
            }
        }
    }
    CPDF_Font*	pFont = pTextObj->GetFont();
    FX_STRSIZE nItems = actText.GetLength();
    if (nItems < 1) {
        return FPDFTEXT_MC_PASS;
    }
    bExist = FALSE;
    for (FX_STRSIZE i = 0; i < nItems; i++) {
        FX_WCHAR wChar = actText.GetAt(i);
        if (-1 == pFont->CharCodeFromUnicode(wChar)) {
            continue;
        } else {
            bExist = TRUE;
            break;
        }
    }
    if (!bExist) {
        return FPDFTEXT_MC_PASS;
    }
    bExist = FALSE;
    for (FX_STRSIZE i = 0; i < nItems; i++) {
        FX_WCHAR wChar = actText.GetAt(i);
        if ((wChar > 0x80 && wChar < 0xFFFD) || (wChar <= 0x80 && isprint(wChar))) {
            bExist = TRUE;
            break;
        }
    }
    if (!bExist) {
        return FPDFTEXT_MC_DONE;
    }
    return FPDFTEXT_MC_DELAY;
}
void CPDF_TextPage::ProcessMarkedContent(PDFTEXT_Obj Obj)
{
    CPDF_TextObject* pTextObj = Obj.m_pTextObj;
    CPDF_ContentMarkData* pMarkData = (CPDF_ContentMarkData*)pTextObj->m_ContentMark.GetObject();
    if(!pMarkData) {
        return;
    }
    int nContentMark = pMarkData->CountItems();
    if (nContentMark < 1) {
        return;
    }
    CFX_WideString actText;
    CPDF_Dictionary* pDict = NULL;
    int n = 0;
    for (n = 0; n < nContentMark; n++) {
        CPDF_ContentMarkItem& item = pMarkData->GetItem(n);
        CFX_ByteString tagStr = (CFX_ByteString)item.GetName();
        pDict = (CPDF_Dictionary*)item.GetParam();
        CPDF_String* temp = (CPDF_String*)(pDict ? pDict->GetElement(FX_BSTRC("ActualText")) : NULL);
        if (temp) {
            actText = temp->GetUnicodeText();
        }
    }
    FX_STRSIZE nItems = actText.GetLength();
    if (nItems < 1) {
        return;
    }
    CPDF_Font*	pFont = pTextObj->GetFont();
    CFX_AffineMatrix formMatrix = Obj.m_formMatrix;
    CFX_AffineMatrix matrix;
    pTextObj->GetTextMatrix(&matrix);
    matrix.Concat(formMatrix);
    FX_FLOAT fPosX = pTextObj->GetPosX();
    FX_FLOAT fPosY = pTextObj->GetPosY();
    int nCharInfoIndex = m_TextBuf.GetLength();
    CFX_FloatRect charBox;
    charBox.top = pTextObj->m_Top;
    charBox.left = pTextObj->m_Left;
    charBox.right = pTextObj->m_Right;
    charBox.bottom = pTextObj->m_Bottom;
    for (FX_STRSIZE k = 0; k < nItems; k++) {
        FX_WCHAR wChar = actText.GetAt(k);
        if (wChar <= 0x80 && !isprint(wChar)) {
            wChar = 0x20;
        }
        if (wChar >= 0xFFFD) {
            continue;
        }
        PAGECHAR_INFO charinfo;
        charinfo.m_OriginX = fPosX;
        charinfo.m_OriginY = fPosY;
        charinfo.m_Index = nCharInfoIndex;
        charinfo.m_Unicode = wChar;
        charinfo.m_CharCode = pFont->CharCodeFromUnicode(wChar);
        charinfo.m_Flag = FPDFTEXT_CHAR_PIECE;
        charinfo.m_pTextObj = pTextObj;
        charinfo.m_CharBox.top = charBox.top;
        charinfo.m_CharBox.left = charBox.left;
        charinfo.m_CharBox.right = charBox.right;
        charinfo.m_CharBox.bottom = charBox.bottom;
        charinfo.m_Matrix.Copy(matrix);
        m_TempTextBuf.AppendChar(wChar);
        m_TempCharList.Add(charinfo);
    }
}
void CPDF_TextPage::FindPreviousTextObject(void)
{
    if (m_TempCharList.GetSize() < 1 && m_charList.GetSize() < 1) {
        return;
    }
    PAGECHAR_INFO preChar;
    if (m_TempCharList.GetSize() >= 1) {
        preChar = *(PAGECHAR_INFO*)m_TempCharList.GetAt(m_TempCharList.GetSize() - 1);
    } else {
        preChar = *(PAGECHAR_INFO*)m_charList.GetAt(m_charList.GetSize() - 1);
    }
    if (preChar.m_pTextObj) {
        m_pPreTextObj = preChar.m_pTextObj;
    }
}
void CPDF_TextPage::ProcessTextObject(PDFTEXT_Obj Obj)
{
    CPDF_TextObject* pTextObj = Obj.m_pTextObj;
    if(FXSYS_fabs(pTextObj->m_Right - pTextObj->m_Left) < 0.01f ) {
        return;
    }
    CFX_AffineMatrix formMatrix = Obj.m_formMatrix;
    CPDF_Font*	pFont = pTextObj->GetFont();
    CFX_AffineMatrix matrix;
    pTextObj->GetTextMatrix(&matrix);
    matrix.Concat(formMatrix);
    FX_INT32 bPreMKC = PreMarkedContent(Obj);
    if (FPDFTEXT_MC_DONE == bPreMKC) {
        m_pPreTextObj = pTextObj;
        m_perMatrix.Copy(formMatrix);
        return;
    }
    int result = 0;
    if (m_pPreTextObj) {
        result = ProcessInsertObject(pTextObj, formMatrix);
        if (2 == result) {
            m_CurlineRect = CFX_FloatRect(Obj.m_pTextObj->m_Left, Obj.m_pTextObj->m_Bottom, Obj.m_pTextObj->m_Right, Obj.m_pTextObj->m_Top);
        } else {
            m_CurlineRect.Union(CFX_FloatRect(Obj.m_pTextObj->m_Left, Obj.m_pTextObj->m_Bottom, Obj.m_pTextObj->m_Right, Obj.m_pTextObj->m_Top));
        }
        PAGECHAR_INFO generateChar;
        if (result == 1) {
            if (GenerateCharInfo(TEXT_BLANK_CHAR, generateChar)) {
                if (!formMatrix.IsIdentity()) {
                    generateChar.m_Matrix.Copy(formMatrix);
                }
                m_TempTextBuf.AppendChar(TEXT_BLANK_CHAR);
                m_TempCharList.Add(generateChar);
            }
        } else if(result == 2) {
            CloseTempLine();
            if(m_TextBuf.GetSize()) {
                if(m_ParseOptions.m_bGetCharCodeOnly) {
                    m_TextBuf.AppendChar(TEXT_RETURN_CHAR);
                    m_TextBuf.AppendChar(TEXT_LINEFEED_CHAR);
                } else {
                    if(GenerateCharInfo(TEXT_RETURN_CHAR, generateChar)) {
                        m_TextBuf.AppendChar(TEXT_RETURN_CHAR);
                        if (!formMatrix.IsIdentity()) {
                            generateChar.m_Matrix.Copy(formMatrix);
                        }
                        m_charList.Add(generateChar);
                    }
                    if(GenerateCharInfo(TEXT_LINEFEED_CHAR, generateChar)) {
                        m_TextBuf.AppendChar(TEXT_LINEFEED_CHAR);
                        if (!formMatrix.IsIdentity()) {
                            generateChar.m_Matrix.Copy(formMatrix);
                        }
                        m_charList.Add(generateChar);
                    }
                }
            }
        } else if (result == 3 && !m_ParseOptions.m_bOutputHyphen) {
            FX_INT32 nChars = pTextObj->CountChars();
            if (nChars == 1) {
                CPDF_TextObjectItem item;
                pTextObj->GetCharInfo(0, &item);
                CFX_WideString wstrItem = pTextObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
                if(wstrItem.IsEmpty()) {
                    wstrItem += (FX_WCHAR)item.m_CharCode;
                }
                FX_WCHAR curChar = wstrItem.GetAt(0);
                if (0x2D == curChar || 0xAD == curChar) {
                    return;
                }
            }
            while (m_TempTextBuf.GetSize() > 0 && m_TempTextBuf.GetWideString().GetAt(m_TempTextBuf.GetLength() - 1) == 0x20) {
                m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
                m_TempCharList.Delete(m_TempCharList.GetSize() - 1);
            }
            PAGECHAR_INFO* cha = (PAGECHAR_INFO*)m_TempCharList.GetAt(m_TempCharList.GetSize() - 1);
            m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
            cha->m_Unicode = 0x2;
            cha->m_Flag = FPDFTEXT_CHAR_HYPHEN;
            m_TempTextBuf.AppendChar(0xfffe);
        }
    } else {
        m_CurlineRect = CFX_FloatRect(Obj.m_pTextObj->m_Left, Obj.m_pTextObj->m_Bottom, Obj.m_pTextObj->m_Right, Obj.m_pTextObj->m_Top);
    }
    if (FPDFTEXT_MC_DELAY == bPreMKC) {
        ProcessMarkedContent(Obj);
        m_pPreTextObj = pTextObj;
        m_perMatrix.Copy(formMatrix);
        return;
    }
    m_pPreTextObj = pTextObj;
    m_perMatrix.Copy(formMatrix);
    int nItems = pTextObj->CountItems();
    FX_FLOAT baseSpace = _CalculateBaseSpace(pTextObj, matrix);

    const FX_BOOL bR2L = IsRightToLeft(pTextObj, pFont, nItems);
    const FX_BOOL bIsBidiAndMirrorInverse =
        bR2L && (matrix.a * matrix.d - matrix.b * matrix.c) < 0;
    FX_INT32 iBufStartAppend = m_TempTextBuf.GetLength();
    FX_INT32 iCharListStartAppend = m_TempCharList.GetSize();

    FX_FLOAT spacing = 0;
    for (int i = 0; i < nItems; i++) {
        CPDF_TextObjectItem item;
        PAGECHAR_INFO charinfo;
        charinfo.m_OriginX = 0;
        charinfo.m_OriginY = 0;
        pTextObj->GetItemInfo(i, &item);
        if (item.m_CharCode == (FX_DWORD) - 1) {
            CFX_WideString str = m_TempTextBuf.GetWideString();
            if(str.IsEmpty()) {
                str = m_TextBuf.GetWideString();
            }
            if (str.IsEmpty() || str.GetAt(str.GetLength() - 1) == TEXT_BLANK_CHAR) {
                continue;
            }
            FX_FLOAT fontsize_h = pTextObj->m_TextState.GetFontSizeH();
            spacing = -fontsize_h * item.m_OriginX / 1000;
            continue;
        }
        FX_FLOAT charSpace = pTextObj->m_TextState.GetObject()->m_CharSpace;
        if (charSpace > 0.001) {
            spacing += matrix.TransformDistance(charSpace);
        } else if(charSpace < -0.001) {
            spacing -= matrix.TransformDistance(FXSYS_fabs(charSpace));
        }
        spacing -= baseSpace;
        if (spacing && i > 0) {
            int last_width = 0;
            FX_FLOAT fontsize_h = pTextObj->m_TextState.GetFontSizeH();
            FX_DWORD space_charcode = pFont->CharCodeFromUnicode(' ');
            FX_FLOAT threshold = 0;
            if (space_charcode != -1) {
                threshold = fontsize_h * pFont->GetCharWidthF(space_charcode) / 1000 ;
            }
            if (threshold > fontsize_h / 3) {
                threshold = 0;
            } else {
                threshold /= 2;
            }
            if (threshold == 0) {
                threshold = fontsize_h;
                int this_width = FXSYS_abs(GetCharWidth(item.m_CharCode, pFont));
                threshold = this_width > last_width ? (FX_FLOAT)this_width : (FX_FLOAT)last_width;
                threshold = _NormalizeThreshold(threshold);
                threshold = fontsize_h * threshold / 1000;
            }
            if (threshold && (spacing && spacing >= threshold) ) {
                charinfo.m_Unicode = TEXT_BLANK_CHAR;
                charinfo.m_Flag = FPDFTEXT_CHAR_GENERATED;
                charinfo.m_pTextObj = pTextObj;
                charinfo.m_Index = m_TextBuf.GetLength();
                m_TempTextBuf.AppendChar(TEXT_BLANK_CHAR);
                charinfo.m_CharCode = -1;
                charinfo.m_Matrix.Copy(formMatrix);
                matrix.Transform(item.m_OriginX, item.m_OriginY, charinfo.m_OriginX, charinfo.m_OriginY);
                charinfo.m_CharBox = CFX_FloatRect(charinfo.m_OriginX, charinfo.m_OriginY, charinfo.m_OriginX, charinfo.m_OriginY);
                m_TempCharList.Add(charinfo);
            }
            if (item.m_CharCode == (FX_DWORD) - 1) {
                continue;
            }
        }
        spacing = 0;
        CFX_WideString wstrItem = pFont->UnicodeFromCharCode(item.m_CharCode);
        FX_BOOL bNoUnicode = FALSE;
        FX_WCHAR wChar = wstrItem.GetAt(0);
        if ((wstrItem.IsEmpty() || wChar == 0) && item.m_CharCode) {
            if(wstrItem.IsEmpty()) {
                wstrItem += (FX_WCHAR)item.m_CharCode;
            } else {
                wstrItem.SetAt(0, (FX_WCHAR)item.m_CharCode);
            }
            bNoUnicode = TRUE;
        }
        charinfo.m_Index = -1;
        charinfo.m_CharCode = item.m_CharCode;
        if(bNoUnicode) {
            charinfo.m_Flag = FPDFTEXT_CHAR_UNUNICODE;
        } else {
            charinfo.m_Flag = FPDFTEXT_CHAR_NORMAL;
        }
        charinfo.m_pTextObj = pTextObj;
        charinfo.m_OriginX = 0, charinfo.m_OriginY = 0;
        matrix.Transform(item.m_OriginX, item.m_OriginY, charinfo.m_OriginX, charinfo.m_OriginY);
        FX_RECT rect(0, 0, 0, 0);
        rect.Intersect(0, 0, 0, 0);
        charinfo.m_pTextObj->GetFont()->GetCharBBox(charinfo.m_CharCode, rect);
        charinfo.m_CharBox.top = rect.top * pTextObj->GetFontSize() / 1000 + item.m_OriginY;
        charinfo.m_CharBox.left = rect.left * pTextObj->GetFontSize() / 1000 + item.m_OriginX;
        charinfo.m_CharBox.right = rect.right * pTextObj->GetFontSize() / 1000 + item.m_OriginX;
        charinfo.m_CharBox.bottom = rect.bottom * pTextObj->GetFontSize() / 1000 + item.m_OriginY;
        if (fabsf(charinfo.m_CharBox.top - charinfo.m_CharBox.bottom) < 0.01f) {
            charinfo.m_CharBox.top = charinfo.m_CharBox.bottom + pTextObj->GetFontSize();
        }
        if (fabsf(charinfo.m_CharBox.right - charinfo.m_CharBox.left) < 0.01f) {
            charinfo.m_CharBox.right = charinfo.m_CharBox.left + pTextObj->GetCharWidth(charinfo.m_CharCode);
        }
        matrix.TransformRect(charinfo.m_CharBox);
        charinfo.m_Matrix.Copy(matrix);
        if (wstrItem.IsEmpty()) {
            charinfo.m_Unicode = 0;
            m_TempCharList.Add(charinfo);
            m_TempTextBuf.AppendChar(0xfffe);
            continue;
        } else {
            int nTotal = wstrItem.GetLength();
            FX_BOOL bDel = FALSE;
            const int count = std::min(m_TempCharList.GetSize(), 7);
            FX_FLOAT threshold = charinfo.m_Matrix.TransformXDistance((FX_FLOAT)TEXT_CHARRATIO_GAPDELTA * pTextObj->GetFontSize());
            for (int n = m_TempCharList.GetSize();
                 n > m_TempCharList.GetSize() - count;
                 n--) {
                PAGECHAR_INFO* charinfo1 = (PAGECHAR_INFO*)m_TempCharList.GetAt(n - 1);
                if(charinfo1->m_CharCode == charinfo.m_CharCode &&
                        charinfo1->m_pTextObj->GetFont() == charinfo.m_pTextObj->GetFont()  &&
                        FXSYS_fabs(charinfo1->m_OriginX - charinfo.m_OriginX) < threshold  &&
                        FXSYS_fabs(charinfo1->m_OriginY - charinfo.m_OriginY) < threshold) {
                    bDel = TRUE;
                    break;
                }
            }
            if(!bDel) {
                for (int nIndex = 0; nIndex < nTotal; nIndex++) {
                    charinfo.m_Unicode = wstrItem.GetAt(nIndex);
                    if (charinfo.m_Unicode) {
                        charinfo.m_Index = m_TextBuf.GetLength();
                        m_TempTextBuf.AppendChar(charinfo.m_Unicode);
                    } else {
                        m_TempTextBuf.AppendChar(0xfffe);
                    }
                    m_TempCharList.Add(charinfo);
                }
            } else if(i == 0) {
                CFX_WideString str = m_TempTextBuf.GetWideString();
                if (!str.IsEmpty() && str.GetAt(str.GetLength() - 1) == TEXT_BLANK_CHAR) {
                    m_TempTextBuf.Delete(m_TempTextBuf.GetLength() - 1, 1);
                    m_TempCharList.Delete(m_TempCharList.GetSize() - 1);
                }
            }
        }
    }
    if (bIsBidiAndMirrorInverse) {
        SwapTempTextBuf(iCharListStartAppend, iBufStartAppend);
    }
}
void CPDF_TextPage::SwapTempTextBuf(FX_INT32 iCharListStartAppend,
                                    FX_INT32 iBufStartAppend)
{
    FX_INT32 i, j;
    i = iCharListStartAppend;
    j = m_TempCharList.GetSize() - 1;
    for (; i < j; i++, j--) {
        std::swap(m_TempCharList[i], m_TempCharList[j]);
        std::swap(m_TempCharList[i].m_Index, m_TempCharList[j].m_Index);
    }
    FX_WCHAR * pTempBuffer = m_TempTextBuf.GetBuffer();
    i = iBufStartAppend;
    j = m_TempTextBuf.GetLength() - 1;
    for (; i < j; i++, j--) {
        std::swap(pTempBuffer[i], pTempBuffer[j]);
    }
}
FX_BOOL CPDF_TextPage::IsRightToLeft(const CPDF_TextObject* pTextObj,
                                     const CPDF_Font* pFont,
                                     int nItems) const
{
    IFX_BidiChar* BidiChar = IFX_BidiChar::Create();
    FX_INT32 nR2L = 0;
    FX_INT32 nL2R = 0;
    FX_INT32 start = 0, count = 0;
    CPDF_TextObjectItem item;
    for (FX_INT32 i = 0; i < nItems; i++) {
        pTextObj->GetItemInfo(i, &item);
        if (item.m_CharCode == (FX_DWORD)-1) {
            continue;
        }
        CFX_WideString wstrItem = pFont->UnicodeFromCharCode(item.m_CharCode);
        FX_WCHAR wChar = wstrItem.GetAt(0);
        if ((wstrItem.IsEmpty() || wChar == 0) && item.m_CharCode) {
            wChar = (FX_WCHAR)item.m_CharCode;
        }
        if (!wChar) {
            continue;
        }
        if (BidiChar && BidiChar->AppendChar(wChar)) {
            FX_INT32 ret = BidiChar->GetBidiInfo(start, count);
            if (ret == 2) {
                nR2L++;
            }
            else if (ret == 1) {
                nL2R++;
            }
        }
    }
    if (BidiChar && BidiChar->EndChar()) {
        FX_INT32 ret = BidiChar->GetBidiInfo(start, count);
        if (ret == 2) {
            nR2L++;
        }
        else if (ret == 1) {
            nL2R++;
        }
    }
    if (BidiChar)
      BidiChar->Release();
    return (nR2L > 0 && nR2L >= nL2R);
}
FX_INT32 CPDF_TextPage::GetTextObjectWritingMode(const CPDF_TextObject* pTextObj)
{
    FX_INT32 nChars = pTextObj->CountChars();
    if (nChars == 1) {
        return m_TextlineDir;
    }
    CPDF_TextObjectItem first, last;
    pTextObj->GetCharInfo(0, &first);
    pTextObj->GetCharInfo(nChars - 1, &last);
    CFX_Matrix textMatrix;
    pTextObj->GetTextMatrix(&textMatrix);
    textMatrix.TransformPoint(first.m_OriginX, first.m_OriginY);
    textMatrix.TransformPoint(last.m_OriginX, last.m_OriginY);
    FX_FLOAT dX = FXSYS_fabs(last.m_OriginX - first.m_OriginX);
    FX_FLOAT dY = FXSYS_fabs(last.m_OriginY - first.m_OriginY);
    if (dX <= 0.0001f && dY <= 0.0001f) {
        return -1;
    }
    CFX_VectorF v;
    v.Set(dX, dY);
    v.Normalize();
    if (v.y <= 0.0872f) {
        if (v.x <= 0.0872f) {
            return m_TextlineDir;
        }
        return 0;
    } else if (v.x <= 0.0872f) {
        return 1;
    }
    return m_TextlineDir;
}
FX_BOOL CPDF_TextPage::IsHyphen(FX_WCHAR curChar)
{
    CFX_WideString strCurText = m_TempTextBuf.GetWideString();
    if(strCurText.GetLength() == 0) {
        strCurText = m_TextBuf.GetWideString();
    }
    FX_STRSIZE nCount = strCurText.GetLength();
    int nIndex = nCount - 1;
    FX_WCHAR wcTmp = strCurText.GetAt(nIndex);
    while(wcTmp == 0x20 && nIndex <= nCount - 1 && nIndex >= 0) {
        wcTmp = strCurText.GetAt(--nIndex);
    }
    if (0x2D == wcTmp || 0xAD == wcTmp) {
        if (--nIndex > 0) {
            FX_WCHAR preChar = strCurText.GetAt((nIndex));
            if (((preChar >= L'A' && preChar <= L'Z') || (preChar >= L'a' && preChar <= L'z'))
                    && ((curChar >= L'A' && curChar <= L'Z') || (curChar >= L'a' && curChar <= L'z'))) {
                return TRUE;
            }
        }
        int size = m_TempCharList.GetSize();
        PAGECHAR_INFO preChar;
        if (size) {
            preChar = (PAGECHAR_INFO)m_TempCharList[size - 1];
        } else {
            size = m_charList.GetSize();
            if(size == 0) {
                return FALSE;
            }
            preChar = (PAGECHAR_INFO)m_charList[size - 1];
        }
        if (FPDFTEXT_CHAR_PIECE == preChar.m_Flag)
            if (0xAD == preChar.m_Unicode || 0x2D == preChar.m_Unicode) {
                return TRUE;
            }
    }
    return FALSE;
}
int CPDF_TextPage::ProcessInsertObject(const CPDF_TextObject* pObj, const CFX_AffineMatrix& formMatrix)
{
    FindPreviousTextObject();
    FX_BOOL bNewline = FALSE;
    int WritingMode = GetTextObjectWritingMode(pObj);
    if(WritingMode == -1) {
        WritingMode = GetTextObjectWritingMode(m_pPreTextObj);
    }
    CFX_FloatRect this_rect(pObj->m_Left, pObj->m_Bottom, pObj->m_Right, pObj->m_Top);
    CFX_FloatRect prev_rect(m_pPreTextObj->m_Left, m_pPreTextObj->m_Bottom, m_pPreTextObj->m_Right, m_pPreTextObj->m_Top);
    CPDF_TextObjectItem PrevItem, item;
    int nItem = m_pPreTextObj->CountItems();
    m_pPreTextObj->GetItemInfo(nItem - 1, &PrevItem);
    pObj->GetItemInfo(0, &item);
    CFX_WideString wstrItem = pObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
    if(wstrItem.IsEmpty()) {
        wstrItem += (FX_WCHAR)item.m_CharCode;
    }
    FX_WCHAR curChar = wstrItem.GetAt(0);
    if(WritingMode == 0) {
        if(this_rect.Height() > 4.5 && prev_rect.Height() > 4.5) {
            FX_FLOAT top = this_rect.top < prev_rect.top ? this_rect.top : prev_rect.top;
            FX_FLOAT bottom = this_rect.bottom > prev_rect.bottom ? this_rect.bottom : prev_rect.bottom;
            if(bottom >= top) {
                if(IsHyphen(curChar)) {
                    return 3;
                }
                return 2;
            }
        }
    } else if (WritingMode == 1) {
        if(this_rect.Width() > pObj->GetFontSize() * 0.1f && prev_rect.Width() > m_pPreTextObj->GetFontSize() * 0.1f) {
            FX_FLOAT left = this_rect.left > m_CurlineRect.left ? this_rect.left : m_CurlineRect.left;
            FX_FLOAT right = this_rect.right < m_CurlineRect.right ? this_rect.right : m_CurlineRect.right;
            if(right <= left) {
                if(IsHyphen(curChar)) {
                    return 3;
                }
                return 2;
            }
        }
    }
    FX_FLOAT last_pos = PrevItem.m_OriginX;
    int nLastWidth = GetCharWidth(PrevItem.m_CharCode, m_pPreTextObj->GetFont());
    FX_FLOAT last_width = nLastWidth * m_pPreTextObj->GetFontSize() / 1000;
    last_width = FXSYS_fabs(last_width);
    int nThisWidth = GetCharWidth(item.m_CharCode, pObj->GetFont());
    FX_FLOAT this_width = nThisWidth * pObj->GetFontSize() / 1000;
    this_width = FXSYS_fabs(this_width);
    FX_FLOAT threshold = last_width > this_width ? last_width / 4 : this_width / 4;
    CFX_AffineMatrix prev_matrix, prev_reverse;
    m_pPreTextObj->GetTextMatrix(&prev_matrix);
    prev_matrix.Concat(m_perMatrix);
    prev_reverse.SetReverse(prev_matrix);
    FX_FLOAT x = pObj->GetPosX();
    FX_FLOAT y = pObj->GetPosY();
    formMatrix.Transform(x, y);
    prev_reverse.Transform(x, y);
    if(last_width < this_width) {
        threshold = prev_reverse.TransformDistance(threshold);
    }
    CFX_FloatRect rect1(m_pPreTextObj->m_Left, pObj->m_Bottom, m_pPreTextObj->m_Right, pObj->m_Top);
    CFX_FloatRect rect2(m_pPreTextObj->m_Left, m_pPreTextObj->m_Bottom, m_pPreTextObj->m_Right, m_pPreTextObj->m_Top);
    CFX_FloatRect rect3 = rect1;
    rect1.Intersect(rect2);
    if (WritingMode == 0) {
        if ((rect1.IsEmpty() && rect2.Height() > 5 && rect3.Height() > 5)
                || ((y > threshold * 2 || y < threshold * -3) && (FXSYS_fabs(y) < 1 ? FXSYS_fabs(x) < FXSYS_fabs(y) : TRUE))) {
            bNewline = TRUE;
            if(nItem > 1 ) {
                CPDF_TextObjectItem tempItem;
                m_pPreTextObj->GetItemInfo(0, &tempItem);
                CFX_AffineMatrix m;
                m_pPreTextObj->GetTextMatrix(&m);
                if(PrevItem.m_OriginX > tempItem.m_OriginX &&
                        m_DisplayMatrix.a > 0.9 && m_DisplayMatrix.b < 0.1 &&
                        m_DisplayMatrix.c < 0.1 && m_DisplayMatrix.d < -0.9
                        && m.b < 0.1 && m.c < 0.1 ) {
                    CFX_FloatRect re(0, m_pPreTextObj->m_Bottom, 1000, m_pPreTextObj->m_Top);
                    if(re.Contains(pObj->GetPosX(), pObj->GetPosY())) {
                        bNewline = FALSE;
                    } else {
                        CFX_FloatRect re(0, pObj->m_Bottom, 1000, pObj->m_Top);
                        if(re.Contains(m_pPreTextObj->GetPosX(), m_pPreTextObj->GetPosY())) {
                            bNewline = FALSE;
                        }
                    }
                }
            }
        }
    }
    if(bNewline) {
        if(IsHyphen(curChar)) {
            return 3;
        }
        return 2;
    }
    FX_INT32 nChars = pObj->CountChars();
    if (nChars == 1 && ( 0x2D == curChar || 0xAD == curChar))
        if (IsHyphen(curChar)) {
            return 3;
        }
    CFX_WideString PrevStr = m_pPreTextObj->GetFont()->UnicodeFromCharCode(PrevItem.m_CharCode);
    FX_WCHAR preChar = PrevStr.GetAt(PrevStr.GetLength() - 1);
    CFX_AffineMatrix matrix;
    pObj->GetTextMatrix(&matrix);
    matrix.Concat(formMatrix);
    threshold = (FX_FLOAT)(nLastWidth > nThisWidth ? nLastWidth : nThisWidth);
    threshold = threshold > 400 ? (threshold < 700 ? threshold / 4 :  (threshold > 800 ? threshold / 6 : threshold / 5)) : (threshold / 2);
    if(nLastWidth >= nThisWidth) {
        threshold *= FXSYS_fabs(m_pPreTextObj->GetFontSize());
    } else {
        threshold *= FXSYS_fabs(pObj->GetFontSize());
        threshold = matrix.TransformDistance(threshold);
        threshold = prev_reverse.TransformDistance(threshold);
    }
    threshold /= 1000;
    if((threshold < 1.4881 && threshold > 1.4879)
            || (threshold < 1.39001 && threshold > 1.38999)) {
        threshold *= 1.5;
    }
    if (FXSYS_fabs(last_pos + last_width - x) > threshold && curChar != L' ' && preChar != L' ')
        if (curChar != L' ' && preChar != L' ') {
            if((x - last_pos - last_width) > threshold || (last_pos - x - last_width) > threshold) {
                return 1;
            }
            if(x < 0 && (last_pos - x - last_width) > threshold) {
                return 1;
            }
            if((x - last_pos - last_width) > this_width || (x - last_pos - this_width) > last_width ) {
                return 1;
            }
        }
    return 0;
}
FX_BOOL CPDF_TextPage::IsSameTextObject(CPDF_TextObject* pTextObj1, CPDF_TextObject* pTextObj2)
{
    if (!pTextObj1 || !pTextObj2) {
        return FALSE;
    }
    CFX_FloatRect rcPreObj(pTextObj2->m_Left, pTextObj2->m_Bottom, pTextObj2->m_Right, pTextObj2->m_Top);
    CFX_FloatRect rcCurObj(pTextObj1->m_Left, pTextObj1->m_Bottom, pTextObj1->m_Right, pTextObj1->m_Top);
    if (rcPreObj.IsEmpty() && rcCurObj.IsEmpty() && !m_ParseOptions.m_bGetCharCodeOnly) {
        FX_FLOAT dbXdif = FXSYS_fabs(rcPreObj.left - rcCurObj.left);
        int nCount = m_charList.GetSize();
        if (nCount >= 2) {
            PAGECHAR_INFO perCharTemp = (PAGECHAR_INFO)m_charList[nCount - 2];
            FX_FLOAT dbSpace = perCharTemp.m_CharBox.Width();
            if (dbXdif > dbSpace) {
                return FALSE;
            }
        }
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
    CPDF_TextObjectItem itemPer, itemCur;
    for (int i = 0; i < nPreCount; i++) {
        pTextObj2->GetItemInfo(i, &itemPer);
        pTextObj1->GetItemInfo(i, &itemCur);
        if (itemCur.m_CharCode != itemPer.m_CharCode) {
            return FALSE;
        }
    }
    if(FXSYS_fabs(pTextObj1->GetPosX() - pTextObj2->GetPosX()) > GetCharWidth(itemPer.m_CharCode, pTextObj2->GetFont())*pTextObj2->GetFontSize() / 1000 * 0.9 ||
            FXSYS_fabs(pTextObj1->GetPosY() - pTextObj2->GetPosY()) >
            FX_MAX(FX_MAX(rcPreObj.Height() , rcPreObj.Width()), pTextObj2->GetFontSize()) / 8) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_TextPage::IsSameAsPreTextObject(CPDF_TextObject* pTextObj, FX_POSITION ObjPos)
{
    if (!pTextObj) {
        return FALSE;
    }
    int i = 0;
    if (!ObjPos) {
        ObjPos = m_pPage->GetLastObjectPosition();
    }
    CPDF_PageObject* pObj = m_pPage->GetPrevObject(ObjPos);
    while (i < 5 && ObjPos) {
        pObj = m_pPage->GetPrevObject(ObjPos);
        if(pObj == pTextObj) {
            continue;
        }
        if(pObj->m_Type != PDFPAGE_TEXT) {
            continue;
        }
        if(IsSameTextObject((CPDF_TextObject*)pObj, pTextObj)) {
            return TRUE;
        }
        i++;
    }
    return FALSE;
}
FX_BOOL CPDF_TextPage::GenerateCharInfo(FX_WCHAR unicode, PAGECHAR_INFO& info)
{
    int size = m_TempCharList.GetSize();
    PAGECHAR_INFO preChar;
    if (size) {
        preChar = (PAGECHAR_INFO)m_TempCharList[size - 1];
    } else {
        size = m_charList.GetSize();
        if(size == 0) {
            return FALSE;
        }
        preChar = (PAGECHAR_INFO)m_charList[size - 1];
    }
    info.m_Index = m_TextBuf.GetLength();
    info.m_Unicode = unicode;
    info.m_pTextObj = NULL;
    info.m_CharCode = -1;
    info.m_Flag = FPDFTEXT_CHAR_GENERATED;
    int preWidth = 0;
    if (preChar.m_pTextObj && preChar.m_CharCode != (FX_DWORD) - 1) {
        preWidth = GetCharWidth(preChar.m_CharCode, preChar.m_pTextObj->GetFont());
    }
    FX_FLOAT fs = 0;
    if(preChar.m_pTextObj) {
        fs = preChar.m_pTextObj->GetFontSize();
    } else {
        fs = preChar.m_CharBox.Height();
    }
    if(!fs) {
        fs = 1;
    }
    info.m_OriginX = preChar.m_OriginX + preWidth * (fs) / 1000;
    info.m_OriginY = preChar.m_OriginY;
    info.m_CharBox = CFX_FloatRect(info.m_OriginX, info.m_OriginY, info.m_OriginX, info.m_OriginY);
    return TRUE;
}
FX_BOOL CPDF_TextPage::IsRectIntersect(const CFX_FloatRect& rect1, const CFX_FloatRect& rect2)
{
    CFX_FloatRect rect = rect1;
    rect.Intersect(rect2);
    return !rect.IsEmpty();
}
FX_BOOL	CPDF_TextPage::IsLetter(FX_WCHAR unicode)
{
    if (unicode < L'A') {
        return FALSE;
    }
    if (unicode > L'Z' && unicode < L'a') {
        return FALSE;
    }
    if (unicode > L'z') {
        return FALSE;
    }
    return TRUE;
}
CPDF_TextPageFind::CPDF_TextPageFind(const IPDF_TextPage* pTextPage)
    : m_pTextPage(pTextPage),
      m_flags(0),
      m_findNextStart(-1),
      m_findPreStart(-1),
      m_bMatchCase(FALSE),
      m_bMatchWholeWord(FALSE),
      m_resStart(0),
      m_resEnd(-1),
      m_IsFind(FALSE)
{
    m_strText = m_pTextPage->GetPageText();
    int nCount = pTextPage->CountChars();
    if(nCount) {
        m_CharIndex.Add(0);
    }
    for(int i = 0; i < nCount; i++) {
        FPDF_CHAR_INFO info;
        pTextPage->GetCharInfo(i, info);
        int indexSize = m_CharIndex.GetSize();
        if(info.m_Flag == CHAR_NORMAL || info.m_Flag == CHAR_GENERATED) {
            if(indexSize % 2) {
                m_CharIndex.Add(1);
            } else {
                if(indexSize <= 0) {
                    continue;
                }
                m_CharIndex.SetAt(indexSize - 1, m_CharIndex.GetAt(indexSize - 1) + 1);
            }
        } else {
            if(indexSize % 2) {
                if(indexSize <= 0) {
                    continue;
                }
                m_CharIndex.SetAt(indexSize - 1, i + 1);
            } else {
                m_CharIndex.Add(i + 1);
            }
        }
    }
    int indexSize = m_CharIndex.GetSize();
    if(indexSize % 2) {
        m_CharIndex.RemoveAt(indexSize - 1);
    }
}
int CPDF_TextPageFind::GetCharIndex(int index) const
{
    return m_pTextPage->CharIndexFromTextIndex(index);
    int indexSize = m_CharIndex.GetSize();
    int count = 0;
    for(int i = 0; i < indexSize; i += 2) {
        count += m_CharIndex.GetAt(i + 1);
        if(count > index) {
            return 	index - count + m_CharIndex.GetAt(i + 1) + m_CharIndex.GetAt(i);
        }
    }
    return -1;
}
FX_BOOL	CPDF_TextPageFind::FindFirst(const CFX_WideString& findwhat, int flags, int startPos)
{
    if (!m_pTextPage) {
        return FALSE;
    }
    if (m_strText.IsEmpty() || m_bMatchCase != (flags & FPDFTEXT_MATCHCASE)) {
        m_strText = m_pTextPage->GetPageText();
    }
    CFX_WideString findwhatStr = findwhat;
    m_findWhat = findwhatStr;
    m_flags = flags;
    m_bMatchCase = flags & FPDFTEXT_MATCHCASE;
    if (m_strText.IsEmpty()) {
        m_IsFind = FALSE;
        return TRUE;
    }
    FX_STRSIZE len = findwhatStr.GetLength();
    if (!m_bMatchCase) {
        findwhatStr.MakeLower();
        m_strText.MakeLower();
    }
    m_bMatchWholeWord = flags & FPDFTEXT_MATCHWHOLEWORD;
    m_findNextStart = startPos;
    if (startPos == -1) {
        m_findPreStart = m_strText.GetLength() - 1;
    } else {
        m_findPreStart = startPos;
    }
    m_csFindWhatArray.RemoveAll();
    int i = 0;
    while(i < len) {
        if(findwhatStr.GetAt(i) != ' ') {
            break;
        }
        i++;
    }
    if(i < len) {
        ExtractFindWhat(findwhatStr);
    } else {
        m_csFindWhatArray.Add(findwhatStr);
    }
    if(m_csFindWhatArray.GetSize() <= 0) {
        return FALSE;
    }
    m_IsFind = TRUE;
    m_resStart = 0;
    m_resEnd = -1;
    return TRUE;
}
FX_BOOL CPDF_TextPageFind::FindNext()
{
    if (!m_pTextPage) {
        return FALSE;
    }
    m_resArray.RemoveAll();
    if(m_findNextStart == -1) {
        return FALSE;
    }
    if(m_strText.IsEmpty()) {
        m_IsFind = FALSE;
        return m_IsFind;
    }
    int strLen = m_strText.GetLength();
    if (m_findNextStart > strLen - 1) {
        m_IsFind = FALSE;
        return m_IsFind;
    }
    int nCount = m_csFindWhatArray.GetSize();
    int nResultPos = 0;
    int	nStartPos = 0;
    nStartPos = m_findNextStart;
    FX_BOOL bSpaceStart = FALSE;
    for(int iWord = 0; iWord < nCount; iWord++) {
        CFX_WideString csWord = m_csFindWhatArray[iWord];
        if(csWord.IsEmpty()) {
            if(iWord == nCount - 1) {
                FX_WCHAR strInsert = m_strText.GetAt(nStartPos);
                if(strInsert == TEXT_LINEFEED_CHAR || strInsert == TEXT_BLANK_CHAR || strInsert == TEXT_RETURN_CHAR || strInsert == 160) {
                    nResultPos = nStartPos + 1;
                    break;
                }
                iWord = -1;
            } else if(iWord == 0) {
                bSpaceStart = TRUE;
            }
            continue;
        }
        int endIndex;
        nResultPos = m_strText.Find(csWord.c_str(), nStartPos);
        if (nResultPos == -1) {
            m_IsFind = FALSE;
            return m_IsFind;
        }
        endIndex = nResultPos + csWord.GetLength() - 1;
        if(iWord == 0) {
            m_resStart = nResultPos;
        }
        FX_BOOL bMatch = TRUE;
        if(iWord != 0 && !bSpaceStart) {
            int PreResEndPos = nStartPos;
            int curChar = csWord.GetAt(0);
            CFX_WideString lastWord = m_csFindWhatArray[iWord - 1];
            int lastChar = lastWord.GetAt(lastWord.GetLength() - 1);
            if(nStartPos == nResultPos && !(_IsIgnoreSpaceCharacter(lastChar) || _IsIgnoreSpaceCharacter(curChar))) {
                bMatch = FALSE;
            }
            for(int d = PreResEndPos; d < nResultPos; d++) {
                FX_WCHAR strInsert = m_strText.GetAt(d);
                if(strInsert != TEXT_LINEFEED_CHAR && strInsert != TEXT_BLANK_CHAR && strInsert != TEXT_RETURN_CHAR && strInsert != 160) {
                    bMatch = FALSE;
                    break;
                }
            }
        } else if(bSpaceStart) {
            if(nResultPos > 0) {
                FX_WCHAR strInsert = m_strText.GetAt(nResultPos - 1);
                if(strInsert != TEXT_LINEFEED_CHAR && strInsert != TEXT_BLANK_CHAR && strInsert != TEXT_RETURN_CHAR && strInsert != 160) {
                    bMatch = FALSE;
                    m_resStart = nResultPos;
                } else {
                    m_resStart = nResultPos - 1;
                }
            }
        }
        if(m_bMatchWholeWord && bMatch) {
            bMatch = IsMatchWholeWord(m_strText, nResultPos, endIndex);
        }
        nStartPos = endIndex + 1;
        if(!bMatch) {
            iWord = -1;
            if(bSpaceStart) {
                nStartPos = m_resStart + m_csFindWhatArray[1].GetLength();
            } else {
                nStartPos = m_resStart + m_csFindWhatArray[0].GetLength();
            }
        }
    }
    m_resEnd = nResultPos + m_csFindWhatArray[m_csFindWhatArray.GetSize() - 1].GetLength() - 1;
    m_IsFind = TRUE;
    int resStart = GetCharIndex(m_resStart);
    int resEnd = GetCharIndex(m_resEnd);
    m_pTextPage->GetRectArray(resStart, resEnd - resStart + 1, m_resArray);
    if(m_flags & FPDFTEXT_CONSECUTIVE) {
        m_findNextStart = m_resStart + 1;
        m_findPreStart = m_resEnd - 1;
    } else {
        m_findNextStart = m_resEnd + 1;
        m_findPreStart = m_resStart - 1;
    }
    return m_IsFind;
}
FX_BOOL CPDF_TextPageFind::FindPrev()
{
    if (!m_pTextPage) {
        return FALSE;
    }
    m_resArray.RemoveAll();
    if(m_strText.IsEmpty() || m_findPreStart < 0) {
        m_IsFind = FALSE;
        return m_IsFind;
    }
    CPDF_TextPageFind findEngine(m_pTextPage);
    FX_BOOL ret = findEngine.FindFirst(m_findWhat, m_flags);
    if(!ret) {
        m_IsFind = FALSE;
        return m_IsFind;
    }
    int	order = -1, MatchedCount = 0;
    while(ret) {
        ret = findEngine.FindNext();
        if(ret) {
            int order1 = findEngine.GetCurOrder() ;
            int	MatchedCount1 = findEngine.GetMatchedCount();
            if(((order1 + MatchedCount1) - 1) > m_findPreStart) {
                break;
            }
            order = order1;
            MatchedCount = MatchedCount1;
        }
    }
    if(order == -1) {
        m_IsFind = FALSE;
        return m_IsFind;
    }
    m_resStart = m_pTextPage->TextIndexFromCharIndex(order);
    m_resEnd = m_pTextPage->TextIndexFromCharIndex(order + MatchedCount - 1);
    m_IsFind = TRUE;
    m_pTextPage->GetRectArray(order, MatchedCount, m_resArray);
    if(m_flags & FPDFTEXT_CONSECUTIVE) {
        m_findNextStart = m_resStart + 1;
        m_findPreStart = m_resEnd - 1;
    } else {
        m_findNextStart = m_resEnd + 1;
        m_findPreStart = m_resStart - 1;
    }
    return m_IsFind;
}
void CPDF_TextPageFind::ExtractFindWhat(const CFX_WideString& findwhat)
{
    if(findwhat.IsEmpty()) {
        return ;
    }
    int index = 0;
    while(1) {
        CFX_WideString csWord = TEXT_EMPTY;
        int ret = ExtractSubString(csWord, findwhat.c_str(), index, TEXT_BLANK_CHAR);
        if(csWord.IsEmpty()) {
            if(ret) {
                m_csFindWhatArray.Add(CFX_WideString(L""));
                index++;
                continue;
            } else {
                break;
            }
        }
        int pos = 0;
        while(pos < csWord.GetLength()) {
            CFX_WideString curStr = csWord.Mid(pos, 1);
            FX_WCHAR curChar = csWord.GetAt(pos);
            if (_IsIgnoreSpaceCharacter(curChar)) {
                if (pos > 0 && curChar == 0x2019) {
                    pos++;
                    continue;
                }
                if (pos > 0 ) {
                    CFX_WideString preStr = csWord.Mid(0, pos);
                    m_csFindWhatArray.Add(preStr);
                }
                m_csFindWhatArray.Add(curStr);
                if (pos == csWord.GetLength() - 1) {
                    csWord.Empty();
                    break;
                }
                csWord = csWord.Right(csWord.GetLength() - pos - 1);
                pos = 0;
                continue;
            }
            pos++;
        }
        if (!csWord.IsEmpty()) {
            m_csFindWhatArray.Add(csWord);
        }
        index++;
    }
}
FX_BOOL CPDF_TextPageFind::IsMatchWholeWord(const CFX_WideString& csPageText, int startPos, int endPos)
{
    int char_left = 0;
    int char_right = 0;
    int char_count = endPos - startPos + 1;
    if(char_count < 1) {
        return FALSE;
    }
    if (char_count == 1 && csPageText.GetAt(startPos) > 255) {
        return TRUE;
    }
    if(startPos - 1 >= 0 ) {
        char_left = csPageText.GetAt(startPos - 1);
    }
    if(startPos + char_count < csPageText.GetLength()) {
        char_right = csPageText.GetAt(startPos + char_count);
    }
    if ((char_left > 'A' && char_left < 'a') || (char_left > 'a' && char_left < 'z') || (char_left > 0xfb00 && char_left < 0xfb06) || (char_left >= '0' && char_left <= '9') ||
            (char_right > 'A' && char_right < 'a') || (char_right > 'a' && char_right < 'z') || (char_right > 0xfb00 && char_right < 0xfb06) || (char_right >= '0' && char_right <= '9')) {
        return FALSE;
    }
    if(!(('A' > char_left || char_left > 'Z')  && ('a' > char_left || char_left > 'z')
            && ('A' > char_right || char_right > 'Z')  && ('a' > char_right || char_right > 'z'))) {
        return FALSE;
    }
    if (char_count > 0) {
        if (csPageText.GetAt(startPos) >= L'0' && csPageText.GetAt(startPos) <= L'9' && char_left >= L'0' && char_left <= L'9') {
            return FALSE;
        }
        if (csPageText.GetAt(endPos) >= L'0' && csPageText.GetAt(endPos) <= L'9' && char_right >= L'0' && char_right <= L'9') {
            return FALSE;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_TextPageFind::ExtractSubString(CFX_WideString& rString, FX_LPCWSTR lpszFullString,
        int iSubString, FX_WCHAR chSep)
{
    if (lpszFullString == NULL) {
        return FALSE;
    }
    while (iSubString--) {
        lpszFullString = FXSYS_wcschr(lpszFullString, chSep);
        if (lpszFullString == NULL) {
            rString.Empty();
            return FALSE;
        }
        lpszFullString++;
        while(*lpszFullString == chSep) {
            lpszFullString++;
        }
    }
    FX_LPCWSTR lpchEnd = FXSYS_wcschr(lpszFullString, chSep);
    int nLen = (lpchEnd == NULL) ?
               (int)FXSYS_wcslen(lpszFullString) : (int)(lpchEnd - lpszFullString);
    ASSERT(nLen >= 0);
    FXSYS_memcpy32(rString.GetBuffer(nLen), lpszFullString, nLen * sizeof(FX_WCHAR));
    rString.ReleaseBuffer();
    return TRUE;
}
CFX_WideString CPDF_TextPageFind::MakeReverse(const CFX_WideString& str)
{
    CFX_WideString str2;
    str2.Empty();
    int nlen = str.GetLength();
    for(int i = nlen - 1; i >= 0; i--) {
        str2 += str.GetAt(i);
    }
    return str2;
}
void CPDF_TextPageFind::GetRectArray(CFX_RectArray& rects) const
{
    rects.Copy(m_resArray);
}
int	CPDF_TextPageFind::GetCurOrder() const
{
    return GetCharIndex(m_resStart);
}
int	CPDF_TextPageFind::GetMatchedCount()const
{
    int resStart = GetCharIndex(m_resStart);
    int resEnd = GetCharIndex(m_resEnd);
    return resEnd - resStart + 1;
}
CPDF_LinkExtract::CPDF_LinkExtract()
    : m_pTextPage(NULL),
      m_IsParserd(FALSE)
{
}
CPDF_LinkExtract::~CPDF_LinkExtract()
{
    DeleteLinkList();
}
FX_BOOL CPDF_LinkExtract::ExtractLinks(const IPDF_TextPage* pTextPage)
{
    if (!pTextPage || !pTextPage->IsParsered()) {
        return FALSE;
    }
    m_pTextPage = (const CPDF_TextPage*)pTextPage;
    m_strPageText = m_pTextPage->GetPageText(0, -1);
    DeleteLinkList();
    if (m_strPageText.IsEmpty()) {
        return FALSE;
    }
    parserLink();
    m_IsParserd = TRUE;
    return TRUE;
}
void CPDF_LinkExtract::DeleteLinkList()
{
    while (m_LinkList.GetSize()) {
        CPDF_LinkExt* linkinfo = NULL;
        linkinfo = m_LinkList.GetAt(0);
        m_LinkList.RemoveAt(0);
        delete linkinfo;
    }
    m_LinkList.RemoveAll();
}
int CPDF_LinkExtract::CountLinks() const
{
    if (!m_IsParserd)	{
        return -1;
    }
    return m_LinkList.GetSize();
}
void CPDF_LinkExtract::parserLink()
{
    int start = 0, pos = 0;
    int TotalChar = m_pTextPage->CountChars();
    while (pos < TotalChar) {
        FPDF_CHAR_INFO pageChar;
        m_pTextPage->GetCharInfo(pos, pageChar);
        if (pageChar.m_Flag == CHAR_GENERATED || pageChar.m_Unicode == 0x20 || pos == TotalChar - 1) {
            int nCount = pos - start;
            if(pos == TotalChar - 1) {
                nCount++;
            }
            CFX_WideString strBeCheck;
            strBeCheck = m_pTextPage->GetPageText(start, nCount);
            if (strBeCheck.GetLength() > 5) {
                while(strBeCheck.GetLength() > 0) {
                    FX_WCHAR ch = strBeCheck.GetAt(strBeCheck.GetLength() - 1);
                    if (ch == L')' || ch == L',' || ch == L'>' || ch == L'.') {
                        strBeCheck = strBeCheck.Mid(0, strBeCheck.GetLength() - 1);
                        nCount--;
                    } else {
                        break;
                    }
                }
                if (nCount > 5 && (CheckWebLink(strBeCheck) || CheckMailLink(strBeCheck))) {
                    if (!AppendToLinkList(start, nCount, strBeCheck)) {
                        break;
                    }
                }
            }
            start = ++pos;
        } else {
            pos++;
        }
    }
}
FX_BOOL CPDF_LinkExtract::CheckWebLink(CFX_WideString& strBeCheck)
{
    CFX_WideString str = strBeCheck;
    str.MakeLower();
    if (str.Find(L"http://www.") != -1) {
        strBeCheck = strBeCheck.Right(str.GetLength() - str.Find(L"http://www."));
        return TRUE;
    } else if (str.Find(L"http://") != -1) {
        strBeCheck = strBeCheck.Right(str.GetLength() - str.Find(L"http://"));
        return TRUE;
    } else if (str.Find(L"https://www.") != -1) {
        strBeCheck = strBeCheck.Right(str.GetLength() - str.Find(L"https://www."));
        return TRUE;
    } else if (str.Find(L"https://") != -1) {
        strBeCheck = strBeCheck.Right(str.GetLength() - str.Find(L"https://"));
        return TRUE;
    } else if (str.Find(L"www.") != -1) {
        strBeCheck = strBeCheck.Right(str.GetLength() - str.Find(L"www."));
        strBeCheck = L"http://" + strBeCheck;
        return TRUE;
    } else {
        return FALSE;
    }
}
FX_BOOL CPDF_LinkExtract::CheckMailLink(CFX_WideString& str)
{
    str.MakeLower();
    int aPos = str.Find(L'@');
    if (aPos < 1) {
        return FALSE;
    }
    if (str.GetAt(aPos - 1) == L'.' || str.GetAt(aPos - 1) == L'_') {
        return FALSE;
    }
    int i;
    for (i = aPos - 1; i >= 0; i--) {
        FX_WCHAR ch = str.GetAt(i);
        if (ch == L'_' || ch == L'.' || (ch >= L'a' && ch <= L'z') || (ch >= L'0' && ch <= L'9')) {
            continue;
        } else {
            if (i == aPos - 1) {
                return FALSE;
            }
            str = str.Right(str.GetLength() - i - 1);
            break;
        }
    }
    aPos = str.Find(L'@');
    if (aPos < 1) {
        return FALSE;
    }
    CFX_WideString strtemp = L"";
    for (i = 0; i < aPos; i++) {
        FX_WCHAR wch = str.GetAt(i);
        if (wch >= L'a' && wch <= L'z') {
            break;
        } else {
            strtemp = str.Right(str.GetLength() - i + 1);
        }
    }
    if (strtemp != L"") {
        str = strtemp;
    }
    aPos = str.Find(L'@');
    if (aPos < 1) {
        return FALSE;
    }
    str.TrimRight(L'.');
    strtemp = str;
    int ePos = str.Find(L'.');
    if (ePos == -1) {
        return FALSE;
    }
    while (ePos != -1) {
        strtemp = strtemp.Right(strtemp.GetLength() - ePos - 1);
        ePos = strtemp.Find('.');
    }
    ePos = strtemp.GetLength();
    for (i = 0; i < ePos; i++) {
        FX_WCHAR wch = str.GetAt(i);
        if ((wch >= L'a' && wch <= L'z') || (wch >= L'0' && wch <= L'9')) {
            continue;
        } else {
            str = str.Left(str.GetLength() - ePos + i + 1);
            ePos = ePos - i - 1;
            break;
        }
    }
    int nLen = str.GetLength();
    for (i = aPos + 1; i < nLen - ePos; i++) {
        FX_WCHAR wch = str.GetAt(i);
        if (wch == L'-' || wch == L'.' || (wch >= L'a' && wch <= L'z') || (wch >= L'0' && wch <= L'9')) {
            continue;
        } else {
            return FALSE;
        }
    }
    if (str.Find(L"mailto:") == -1) {
        str = L"mailto:" + str;
    }
    return TRUE;
}
FX_BOOL CPDF_LinkExtract::AppendToLinkList(int start, int count, const CFX_WideString& strUrl)
{
    CPDF_LinkExt* linkInfo = new CPDF_LinkExt;
    linkInfo->m_strUrl = strUrl;
    linkInfo->m_Start = start;
    linkInfo->m_Count = count;
    m_LinkList.Add(linkInfo);
    return TRUE;
}
CFX_WideString CPDF_LinkExtract::GetURL(int index) const
{
    if (!m_IsParserd || index < 0 || index >= m_LinkList.GetSize()) {
        return L"";
    }
    CPDF_LinkExt* link = NULL;
    link = m_LinkList.GetAt(index);
    if (!link) {
        return L"";
    }
    return link->m_strUrl;
}
void CPDF_LinkExtract::GetBoundedSegment(int index, int& start, int& count) const
{
    if (!m_IsParserd || index < 0 || index >= m_LinkList.GetSize()) {
        return ;
    }
    CPDF_LinkExt* link = NULL;
    link = m_LinkList.GetAt(index);
    if (!link) {
        return ;
    }
    start = link->m_Start;
    count = link->m_Count;
}
void CPDF_LinkExtract::GetRects(int index, CFX_RectArray& rects) const
{
    if (!m_IsParserd || index < 0 || index >= m_LinkList.GetSize()) {
        return;
    }
    CPDF_LinkExt* link = NULL;
    link = m_LinkList.GetAt(index);
    if (!link) {
        return ;
    }
    m_pTextPage->GetRectArray(link->m_Start, link->m_Count, rects);
}
