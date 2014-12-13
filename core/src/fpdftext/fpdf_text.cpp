// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfapi/fpdf_page.h"
#include "../../include/fpdfapi/fpdf_pageobj.h"
#include "../../include/fpdftext/fpdf_text.h"
#include "txtproc.h"
#include "text_int.h"
extern FX_LPCSTR FCS_GetAltStr(FX_WCHAR);
CFX_ByteString CharFromUnicodeAlt(FX_WCHAR unicode, int destcp, FX_LPCSTR defchar)
{
    if (destcp == 0) {
        if (unicode < 0x80) {
            return CFX_ByteString((char)unicode);
        }
        FX_LPCSTR altstr = FCS_GetAltStr(unicode);
        if (altstr) {
            return CFX_ByteString(altstr, -1);
        }
        return CFX_ByteString(defchar, -1);
    }
    FX_BOOL bDef = FALSE;
    char buf[10];
    int ret = FXSYS_WideCharToMultiByte(destcp, 0, (wchar_t*)&unicode, 1, buf, 10, NULL, &bDef);
    if (ret && !bDef) {
        return CFX_ByteString(buf, ret);
    }
    FX_LPCSTR altstr = FCS_GetAltStr(unicode);
    if (altstr) {
        return CFX_ByteString(altstr, -1);
    }
    return CFX_ByteString(defchar, -1);
}
CTextPage::CTextPage()
{
}
CTextPage::~CTextPage()
{
    int i;
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        delete pBaseLine;
    }
    for (i = 0; i < m_TextColumns.GetSize(); i ++) {
        CTextColumn* pTextColumn = (CTextColumn*)m_TextColumns.GetAt(i);
        delete pTextColumn;
    }
}
void CTextPage::ProcessObject(CPDF_PageObject* pObject)
{
    if (pObject->m_Type != PDFPAGE_TEXT) {
        return;
    }
    CPDF_TextObject* pText = (CPDF_TextObject*)pObject;
    CPDF_Font* pFont = pText->m_TextState.GetFont();
    int count = pText->CountItems();
    FX_FLOAT* pPosArray = FX_Alloc(FX_FLOAT, count * 2);
    if (pPosArray) {
        pText->CalcCharPos(pPosArray);
    }
    FX_FLOAT fontsize_h = pText->m_TextState.GetFontSizeH();
    FX_FLOAT fontsize_v = pText->m_TextState.GetFontSizeV();
    FX_DWORD space_charcode = pFont->CharCodeFromUnicode(' ');
    FX_FLOAT spacew = 0;
    if (space_charcode != -1) {
        spacew = fontsize_h * pFont->GetCharWidthF(space_charcode) / 1000;
    }
    if (spacew == 0) {
        spacew = fontsize_h / 4;
    }
    if (pText->m_TextState.GetBaselineAngle() != 0) {
        int cc = 0;
        CFX_AffineMatrix matrix;
        pText->GetTextMatrix(&matrix);
        for (int i = 0; i < pText->m_nChars; i ++) {
            FX_DWORD charcode = pText->m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)pText->m_pCharCodes : pText->m_pCharCodes[i];
            if (charcode == (FX_DWORD) - 1) {
                continue;
            }
            FX_RECT char_box;
            pFont->GetCharBBox(charcode, char_box);
            FX_FLOAT char_left = pPosArray ? pPosArray[cc * 2] : char_box.left * pText->m_TextState.GetFontSize() / 1000;
            FX_FLOAT char_right = pPosArray ? pPosArray[cc * 2 + 1] : char_box.right * pText->m_TextState.GetFontSize() / 1000;
            FX_FLOAT char_top = char_box.top * pText->m_TextState.GetFontSize() / 1000;
            FX_FLOAT char_bottom = char_box.bottom * pText->m_TextState.GetFontSize() / 1000;
            cc ++;
            FX_FLOAT char_origx, char_origy;
            matrix.Transform(char_left, 0, char_origx, char_origy);
            matrix.TransformRect(char_left, char_right, char_top, char_bottom);
            CFX_ByteString str;
            pFont->AppendChar(str, charcode);
            InsertTextBox(NULL, char_origy, char_left, char_right, char_top,
                          char_bottom, spacew, fontsize_v, str, pFont);
        }
        if (pPosArray) {
            FX_Free(pPosArray);
        }
        return;
    }
    FX_FLOAT ratio_h = fontsize_h / pText->m_TextState.GetFontSize();
    for (int ii = 0; ii < count * 2; ii ++) {
        pPosArray[ii] *= ratio_h;
    }
    FX_FLOAT baseline = pText->m_PosY;
    CTextBaseLine* pBaseLine = NULL;
    FX_FLOAT topy = pText->m_Top;
    FX_FLOAT bottomy = pText->m_Bottom;
    FX_FLOAT leftx = pText->m_Left;
    int cc = 0;
    CFX_ByteString segment;
    int space_count = 0;
    FX_FLOAT last_left = 0, last_right = 0, segment_left = 0, segment_right = 0;
    for (int i = 0; i < pText->m_nChars; i ++) {
        FX_DWORD charcode = pText->m_nChars == 1 ? (FX_DWORD)(FX_UINTPTR)pText->m_pCharCodes : pText->m_pCharCodes[i];
        if (charcode == (FX_DWORD) - 1) {
            continue;
        }
        FX_FLOAT char_left = pPosArray[cc * 2];
        FX_FLOAT char_right = pPosArray[cc * 2 + 1];
        cc ++;
        if (char_left < last_left || (char_left - last_right) > spacew / 2) {
            pBaseLine = InsertTextBox(pBaseLine, baseline, leftx + segment_left, leftx + segment_right,
                                      topy, bottomy, spacew, fontsize_v, segment, pFont);
            segment_left = char_left;
            segment = "";
        }
        if (space_count > 1) {
            pBaseLine = InsertTextBox(pBaseLine, baseline, leftx + segment_left, leftx + segment_right,
                                      topy, bottomy, spacew, fontsize_v, segment, pFont);
            segment = "";
        } else if (space_count == 1) {
            pFont->AppendChar(segment, ' ');
        }
        if (segment.GetLength() == 0) {
            segment_left = char_left;
        }
        segment_right = char_right;
        pFont->AppendChar(segment, charcode);
        space_count = 0;
        last_left = char_left;
        last_right = char_right;
    }
    if (segment.GetLength())
        pBaseLine = InsertTextBox(pBaseLine, baseline, leftx + segment_left, leftx + segment_right,
                                  topy, bottomy, spacew, fontsize_v, segment, pFont);
    FX_Free(pPosArray);
}
static void ConvertPDFString(CFX_ByteString& result, CFX_ByteString& src, CPDF_Font* pFont);
CTextBaseLine* CTextPage::InsertTextBox(CTextBaseLine* pBaseLine, FX_FLOAT basey, FX_FLOAT leftx,
                                        FX_FLOAT rightx, FX_FLOAT topy, FX_FLOAT bottomy, FX_FLOAT spacew, FX_FLOAT fontsize_v,
                                        CFX_ByteString& str, CPDF_Font* pFont)
{
    if (str.GetLength() == 0) {
        return NULL;
    }
    if (pBaseLine == NULL) {
        int i;
        for (i = 0; i < m_BaseLines.GetSize(); i ++) {
            CTextBaseLine* pExistLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
            if (pExistLine->m_BaseLine == basey) {
                pBaseLine = pExistLine;
                break;
            }
            if (pExistLine->m_BaseLine < basey) {
                break;
            }
        }
        if (pBaseLine == NULL) {
            pBaseLine = FX_NEW CTextBaseLine;
            if (NULL == pBaseLine) {
                return NULL;
            }
            pBaseLine->m_BaseLine = basey;
            m_BaseLines.InsertAt(i, pBaseLine);
        }
    }
    CFX_WideString text;
    FX_LPCSTR pStr = str;
    int len = str.GetLength(), offset = 0;
    while (offset < len) {
        FX_DWORD ch = pFont->GetNextChar(pStr, offset);
        CFX_WideString unicode_str = pFont->UnicodeFromCharCode(ch);
        if (unicode_str.IsEmpty()) {
            text += (FX_WCHAR)ch;
        }
        else {
            text += unicode_str;
        }
    }
    pBaseLine->InsertTextBox(leftx, rightx, topy, bottomy, spacew, fontsize_v, text);
    return pBaseLine;
}
void CTextPage::WriteOutput(CFX_WideStringArray& lines, int iMinWidth)
{
    FX_FLOAT lastheight = -1;
    FX_FLOAT lastbaseline = -1;
    FX_FLOAT MinLeftX = 1000000;
    FX_FLOAT MaxRightX = 0;
    int i;
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        FX_FLOAT leftx, rightx;
        if (pBaseLine->GetWidth(leftx, rightx)) {
            if (leftx < MinLeftX) {
                MinLeftX = leftx;
            }
            if (rightx > MaxRightX) {
                MaxRightX = rightx;
            }
        }
    }
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        pBaseLine->MergeBoxes();
    }
    for (i = 1; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        CTextBaseLine* pPrevLine = (CTextBaseLine*)m_BaseLines.GetAt(i - 1);
        if (pBaseLine->CanMerge(pPrevLine)) {
            pPrevLine->Merge(pBaseLine);
            delete pBaseLine;
            m_BaseLines.RemoveAt(i);
            i --;
        }
    }
    if (m_bAutoWidth) {
        int* widths = FX_Alloc(int, m_BaseLines.GetSize());
        if (widths) {
            for (i = 0; i < m_BaseLines.GetSize(); i ++) {
                widths[i] = 0;
                CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
                int TotalChars = 0;
                FX_FLOAT TotalWidth = 0;
                int minchars;
                pBaseLine->CountChars(TotalChars, TotalWidth, minchars);
                if (TotalChars) {
                    FX_FLOAT charwidth = TotalWidth / TotalChars;
                    widths[i] = (int)((MaxRightX - MinLeftX) / charwidth);
                }
                if (widths[i] > 1000) {
                    widths[i] = 1000;
                }
                if (widths[i] < minchars) {
                    widths[i] = minchars;
                }
            }
            int AvgWidth = 0, widthcount = 0;
            for (i = 0; i < m_BaseLines.GetSize(); i ++)
                if (widths[i]) {
                    AvgWidth += widths[i];
                    widthcount ++;
                }
            AvgWidth = int((FX_FLOAT)AvgWidth / widthcount + 0.5);
            int MaxWidth = 0;
            for (i = 0; i < m_BaseLines.GetSize(); i ++)
                if (MaxWidth < widths[i]) {
                    MaxWidth = widths[i];
                }
            if (MaxWidth > AvgWidth * 6 / 5) {
                MaxWidth = AvgWidth * 6 / 5;
            }
            FX_Free(widths);
            if (iMinWidth < MaxWidth) {
                iMinWidth = MaxWidth;
            }
        }
    }
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        pBaseLine->MergeBoxes();
    }
    if (m_bKeepColumn) {
        FindColumns();
    }
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        if (lastheight >= 0) {
            FX_FLOAT dy = lastbaseline - pBaseLine->m_BaseLine;
            if (dy >= (pBaseLine->m_MaxFontSizeV) * 1.5 || dy >= lastheight * 1.5) {
                lines.Add(L"");
            }
        }
        lastheight = pBaseLine->m_MaxFontSizeV;
        lastbaseline = pBaseLine->m_BaseLine;
        CFX_WideString str;
        pBaseLine->WriteOutput(str, MinLeftX, MaxRightX - MinLeftX, iMinWidth);
        lines.Add(str);
    }
}
void NormalizeCompositeChar(FX_WCHAR wChar, CFX_WideString& sDest)
{
    wChar = FX_GetMirrorChar(wChar, TRUE, FALSE);
    FX_LPWSTR pDst = NULL;
    FX_STRSIZE nCount = FX_Unicode_GetNormalization(wChar, pDst);
    if (nCount < 1 ) {
        sDest += wChar;
        return;
    }
    pDst = new FX_WCHAR[nCount];
    FX_Unicode_GetNormalization(wChar, pDst);
    for (int nIndex = 0; nIndex < nCount; nIndex++) {
        sDest += pDst[nIndex];
    }
    delete[] pDst;
}
void NormalizeString(CFX_WideString& str)
{
    if (str.GetLength() <= 0) {
        return;
    }
    CFX_WideString sBuffer;
    IFX_BidiChar* BidiChar = IFX_BidiChar::Create();
    if (NULL == BidiChar)	{
        return;
    }
    CFX_WordArray order;
    FX_BOOL bR2L = FALSE;
    FX_INT32 start = 0, count = 0, i = 0;
    int nR2L = 0, nL2R = 0;
    for (i = 0; i < str.GetLength(); i++) {
        if(BidiChar->AppendChar(str.GetAt(i))) {
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
    if(BidiChar->EndChar()) {
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
    if(bR2L) {
        int count = order.GetSize();
        for(int j = count - 1; j > 0; j -= 3) {
            int ret = order.GetAt(j);
            int start = order.GetAt(j - 2);
            int count1 = order.GetAt(j - 1);
            if(ret == 2 || ret == 0) {
                for(int i = start + count1 - 1; i >= start; i--) {
                    NormalizeCompositeChar(str[i], sBuffer);
                }
            } else {
                i = j;
                FX_BOOL bSymbol = FALSE;
                while(i > 0 && order.GetAt(i) != 2) {
                    bSymbol = !order.GetAt(i);
                    i -= 3;
                }
                int end = start + count1 ;
                int n = 0;
                if(bSymbol) {
                    n = i + 6;
                } else {
                    n = i + 3;
                }
                if(n >= j) {
                    for(int m = start; m < end; m++) {
                        sBuffer += str[m];
                    }
                } else {
                    i = j;
                    j = n;
                    for(; n <= i; n += 3) {
                        int start = order.GetAt(n - 2);
                        int count1 = order.GetAt(n - 1);
                        int end = start + count1 ;
                        for(int m = start; m < end; m++) {
                            sBuffer += str[m];
                        }
                    }
                }
            }
        }
    } else {
        int count = order.GetSize();
        FX_BOOL bL2R = FALSE;
        for(int j = 0; j < count; j += 3) {
            int ret = order.GetAt(j + 2);
            int start = order.GetAt(j);
            int count1 = order.GetAt(j + 1);
            if(ret == 2 || (j == 0 && ret == 0 && !bL2R)) {
                int i = j + 3;
                while(bR2L && i < count) {
                    if(order.GetAt(i + 2) == 1) {
                        break;
                    } else {
                        i += 3;
                    }
                }
                if(i == 3) {
                    j = -3;
                    bL2R = TRUE;
                    continue;
                }
                int end = str.GetLength() - 1;
                if(i < count) {
                    end = order.GetAt(i) - 1;
                }
                j = i - 3;
                for(int n = end; n >= start; n--) {
                    NormalizeCompositeChar(str[i], sBuffer);
                }
            } else {
                int end = start + count1 ;
                for(int i = start; i < end; i++) {
                    sBuffer += str[i];
                }
            }
        }
    }
    str.Empty();
    str += sBuffer;
    BidiChar->Release();
}
static FX_BOOL IsNumber(CFX_WideString& str)
{
    for (int i = 0; i < str.GetLength(); i ++) {
        FX_WCHAR ch = str[i];
        if ((ch < '0' || ch > '9') && ch != '-' && ch != '+' && ch != '.' && ch != ' ') {
            return FALSE;
        }
    }
    return TRUE;
}
void CTextPage::FindColumns()
{
    int i;
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        for (int j = 0; j < pBaseLine->m_TextList.GetSize(); j ++) {
            CTextBox* pTextBox = (CTextBox*)pBaseLine->m_TextList.GetAt(j);
            CTextColumn* pColumn = FindColumn(pTextBox->m_Right);
            if (pColumn == NULL) {
                pColumn = FX_NEW CTextColumn;
                if (pColumn) {
                    pColumn->m_Count = 1;
                    pColumn->m_AvgPos = pTextBox->m_Right;
                    pColumn->m_TextPos = -1;
                    m_TextColumns.Add(pColumn);
                }
            } else {
                pColumn->m_AvgPos = (pColumn->m_Count * pColumn->m_AvgPos + pTextBox->m_Right) /
                                    (pColumn->m_Count + 1);
                pColumn->m_Count ++;
            }
        }
    }
    int mincount = m_BaseLines.GetSize() / 4;
    for (i = 0; i < m_TextColumns.GetSize(); i ++) {
        CTextColumn* pTextColumn = (CTextColumn*)m_TextColumns.GetAt(i);
        if (pTextColumn->m_Count >= mincount) {
            continue;
        }
        delete pTextColumn;
        m_TextColumns.RemoveAt(i);
        i --;
    }
    for (i = 0; i < m_BaseLines.GetSize(); i ++) {
        CTextBaseLine* pBaseLine = (CTextBaseLine*)m_BaseLines.GetAt(i);
        for (int j = 0; j < pBaseLine->m_TextList.GetSize(); j ++) {
            CTextBox* pTextBox = (CTextBox*)pBaseLine->m_TextList.GetAt(j);
            if (IsNumber(pTextBox->m_Text)) {
                pTextBox->m_pColumn = FindColumn(pTextBox->m_Right);
            }
        }
    }
}
CTextColumn* CTextPage::FindColumn(FX_FLOAT xpos)
{
    for (int i = 0; i < m_TextColumns.GetSize(); i ++) {
        CTextColumn* pColumn = (CTextColumn*)m_TextColumns.GetAt(i);
        if (pColumn->m_AvgPos < xpos + 1 && pColumn->m_AvgPos > xpos - 1) {
            return pColumn;
        }
    }
    return NULL;
}
void CTextPage::BreakSpace(CPDF_TextObject* pTextObj)
{
}
CTextBaseLine::CTextBaseLine()
{
    m_Top = -100000;
    m_Bottom = 100000;
    m_MaxFontSizeV = 0;
}
CTextBaseLine::~CTextBaseLine()
{
    for (int i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        delete pText;
    }
}
void CTextBaseLine::InsertTextBox(FX_FLOAT leftx, FX_FLOAT rightx, FX_FLOAT topy, FX_FLOAT bottomy,
                                  FX_FLOAT spacew, FX_FLOAT fontsize_v, const CFX_WideString& text)
{
    if (m_Top < topy) {
        m_Top = topy;
    }
    if (m_Bottom > bottomy) {
        m_Bottom = bottomy;
    }
    if (m_MaxFontSizeV < fontsize_v) {
        m_MaxFontSizeV = fontsize_v;
    }
    int i;
    for (i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        if (pText->m_Left > leftx) {
            break;
        }
    }
    CTextBox* pText = FX_NEW CTextBox;
    if (NULL == pText) {
        return;
    }
    pText->m_Text = text;
    pText->m_Left = leftx;
    pText->m_Right = rightx;
    pText->m_Top = topy;
    pText->m_Bottom = bottomy;
    pText->m_SpaceWidth = spacew;
    pText->m_FontSizeV = fontsize_v;
    pText->m_pColumn = NULL;
    m_TextList.InsertAt(i, pText);
}
FX_BOOL GetIntersection(FX_FLOAT low1, FX_FLOAT high1, FX_FLOAT low2, FX_FLOAT high2,
                        FX_FLOAT& interlow, FX_FLOAT& interhigh);
FX_BOOL CTextBaseLine::CanMerge(CTextBaseLine* pOther)
{
    FX_FLOAT inter_top, inter_bottom;
    if (!GetIntersection(m_Bottom, m_Top, pOther->m_Bottom, pOther->m_Top,
                         inter_bottom, inter_top)) {
        return FALSE;
    }
    FX_FLOAT inter_h = inter_top - inter_bottom;
    if (inter_h < (m_Top - m_Bottom) / 2 && inter_h < (pOther->m_Top - pOther->m_Bottom) / 2) {
        return FALSE;
    }
    FX_FLOAT dy = (FX_FLOAT)FXSYS_fabs(m_BaseLine - pOther->m_BaseLine);
    for (int i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        for (int j = 0; j < pOther->m_TextList.GetSize(); j ++) {
            CTextBox* pOtherText = (CTextBox*)pOther->m_TextList.GetAt(j);
            FX_FLOAT inter_left, inter_right;
            if (!GetIntersection(pText->m_Left, pText->m_Right,
                                 pOtherText->m_Left, pOtherText->m_Right, inter_left, inter_right)) {
                continue;
            }
            FX_FLOAT inter_w = inter_right - inter_left;
            if (inter_w < pText->m_SpaceWidth / 2 && inter_w < pOtherText->m_SpaceWidth / 2) {
                continue;
            }
            if (dy >= (pText->m_Bottom - pText->m_Top) / 2 ||
                    dy >= (pOtherText->m_Bottom - pOtherText->m_Top) / 2) {
                return FALSE;
            }
        }
    }
    return TRUE;
}
void CTextBaseLine::Merge(CTextBaseLine* pOther)
{
    for (int i = 0; i < pOther->m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)pOther->m_TextList.GetAt(i);
        InsertTextBox(pText->m_Left, pText->m_Right, pText->m_Top, pText->m_Bottom,
                      pText->m_SpaceWidth, pText->m_FontSizeV, pText->m_Text);
    }
}
FX_BOOL CTextBaseLine::GetWidth(FX_FLOAT& leftx, FX_FLOAT& rightx)
{
    int i;
    for (i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        if (pText->m_Text != L" ") {
            break;
        }
    }
    if (i == m_TextList.GetSize()) {
        return FALSE;
    }
    CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
    leftx = pText->m_Left;
    for (i = m_TextList.GetSize() - 1; i >= 0; i --) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        if (pText->m_Text != L" ") {
            break;
        }
    }
    pText = (CTextBox*)m_TextList.GetAt(i);
    rightx = pText->m_Right;
    return TRUE;
}
void CTextBaseLine::MergeBoxes()
{
    int i = 0;
    while (1) {
        if (i >= m_TextList.GetSize() - 1) {
            break;
        }
        CTextBox* pThisText = (CTextBox*)m_TextList.GetAt(i);
        CTextBox* pNextText = (CTextBox*)m_TextList.GetAt(i + 1);
        FX_FLOAT dx = pNextText->m_Left - pThisText->m_Right;
        FX_FLOAT spacew = (pThisText->m_SpaceWidth == 0.0) ?
                          pNextText->m_SpaceWidth : pThisText->m_SpaceWidth;
        if (spacew > 0.0 && dx < spacew * 2) {
            pThisText->m_Right = pNextText->m_Right;
            if (dx > spacew * 1.5) {
                pThisText->m_Text += L"  ";
            } else if (dx > spacew / 3) {
                pThisText->m_Text += L' ';
            }
            pThisText->m_Text += pNextText->m_Text;
            pThisText->m_SpaceWidth = pNextText->m_SpaceWidth == 0.0 ?
                                      spacew : pNextText->m_SpaceWidth;
            m_TextList.RemoveAt(i + 1);
            delete pNextText;
        } else {
            i ++;
        }
    }
}
void CTextBaseLine::WriteOutput(CFX_WideString& str, FX_FLOAT leftx, FX_FLOAT pagewidth,
                                int iTextWidth)
{
    int lastpos = -1;
    for (int i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        int xpos;
        if (pText->m_pColumn) {
            xpos = (int)((pText->m_pColumn->m_AvgPos - leftx) * iTextWidth / pagewidth + 0.5);
            xpos -= pText->m_Text.GetLength();
        } else {
            xpos = (int)((pText->m_Left - leftx) * iTextWidth / pagewidth + 0.5);
        }
        if (xpos <= lastpos) {
            xpos = lastpos + 1;
        }
        for (int j = lastpos + 1; j < xpos; j ++) {
            str += ' ';
        }
        CFX_WideString sSrc(pText->m_Text);
        NormalizeString(sSrc);
        str += sSrc;
        str += ' ';
        lastpos = xpos + pText->m_Text.GetLength();
    }
}
void CTextBaseLine::CountChars(int& count, FX_FLOAT& width, int& minchars)
{
    minchars = 0;
    for (int i = 0; i < m_TextList.GetSize(); i ++) {
        CTextBox* pText = (CTextBox*)m_TextList.GetAt(i);
        if (pText->m_Right - pText->m_Left < 0.002) {
            continue;
        }
        count += pText->m_Text.GetLength();
        width += pText->m_Right - pText->m_Left;
        minchars += pText->m_Text.GetLength() + 1;
    }
}
#define PI 3.1415926535897932384626433832795
static void CheckRotate(CPDF_Page& page, CFX_FloatRect& page_bbox)
{
    int total_count = 0, rotated_count[3] = {0, 0, 0};
    FX_POSITION pos = page.GetFirstObjectPosition();
    while (pos) {
        CPDF_PageObject* pObj = page.GetNextObject(pos);
        if (pObj->m_Type != PDFPAGE_TEXT) {
            continue;
        }
        total_count ++;
        CPDF_TextObject* pText = (CPDF_TextObject*)pObj;
        FX_FLOAT angle = pText->m_TextState.GetBaselineAngle();
        if (angle == 0.0) {
            continue;
        }
        int degree = (int)(angle * 180 / PI + 0.5);
        if (degree % 90) {
            continue;
        }
        if (degree < 0) {
            degree += 360;
        }
        int index = degree / 90 % 3 - 1;
        if (index < 0) {
            continue;
        }
        rotated_count[index] ++;
    }
    if (total_count == 0) {
        return;
    }
    CFX_AffineMatrix matrix;
    if (rotated_count[0] > total_count * 2 / 3) {
        matrix.Set(0, -1, 1, 0, 0, page.GetPageHeight());
    } else if (rotated_count[1] > total_count * 2 / 3) {
        matrix.Set(-1, 0, 0, -1, page.GetPageWidth(), page.GetPageHeight());
    } else if (rotated_count[2] > total_count * 2 / 3) {
        matrix.Set(0, 1, -1, 0, page.GetPageWidth(), 0);
    } else {
        return;
    }
    page.Transform(matrix);
    page_bbox.Transform(&matrix);
}
void PDF_GetPageText_Unicode(CFX_WideStringArray& lines, CPDF_Document* pDoc, CPDF_Dictionary* pPage,
                             int iMinWidth, FX_DWORD flags)
{
    lines.RemoveAll();
    if (pPage == NULL) {
        return;
    }
    CPDF_Page page;
    page.Load(pDoc, pPage);
    CPDF_ParseOptions options;
    options.m_bTextOnly = TRUE;
    options.m_bSeparateForm = FALSE;
    page.ParseContent(&options);
    CFX_FloatRect page_bbox = page.GetPageBBox();
    if (flags & PDF2TXT_AUTO_ROTATE) {
        CheckRotate(page, page_bbox);
    }
    CTextPage texts;
    texts.m_bAutoWidth = flags & PDF2TXT_AUTO_WIDTH;
    texts.m_bKeepColumn = flags & PDF2TXT_KEEP_COLUMN;
    texts.m_bBreakSpace = TRUE;
    FX_POSITION pos = page.GetFirstObjectPosition();
    while (pos) {
        CPDF_PageObject* pObject = page.GetNextObject(pos);
        if (!(flags & PDF2TXT_INCLUDE_INVISIBLE)) {
            CFX_FloatRect rect(pObject->m_Left, pObject->m_Bottom, pObject->m_Right, pObject->m_Top);
            if (!page_bbox.Contains(rect)) {
                continue;
            }
        }
        texts.ProcessObject(pObject);
    }
    texts.WriteOutput(lines, iMinWidth);
}
void PDF_GetPageText(CFX_ByteStringArray& lines, CPDF_Document* pDoc, CPDF_Dictionary* pPage,
                     int iMinWidth, FX_DWORD flags)
{
    lines.RemoveAll();
    CFX_WideStringArray wlines;
    PDF_GetPageText_Unicode(wlines, pDoc, pPage, iMinWidth, flags);
    for (int i = 0; i < wlines.GetSize(); i ++) {
        CFX_WideString wstr = wlines[i];
        CFX_ByteString str;
        for (int c = 0; c < wstr.GetLength(); c ++) {
            str += CharFromUnicodeAlt(wstr[c], FXSYS_GetACP(), "?");
        }
        lines.Add(str);
    }
}
extern void _PDF_GetTextStream_Unicode(CFX_WideTextBuf& buffer, CPDF_PageObjects* pPage, FX_BOOL bUseLF,
                                       CFX_PtrArray* pObjArray);
void PDF_GetTextStream_Unicode(CFX_WideTextBuf& buffer, CPDF_Document* pDoc, CPDF_Dictionary* pPage, FX_DWORD flags)
{
    buffer.EstimateSize(0, 10240);
    CPDF_Page page;
    page.Load(pDoc, pPage);
    CPDF_ParseOptions options;
    options.m_bTextOnly = TRUE;
    options.m_bSeparateForm = FALSE;
    page.ParseContent(&options);
    _PDF_GetTextStream_Unicode(buffer, &page, TRUE, NULL);
}
