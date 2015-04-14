// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfapi/fpdf_pageobj.h"
#include "../../include/fpdftext/fpdf_text.h"
#include "../../include/fpdfapi/fpdf_page.h"
class CPDF_TextStream 
{
public:
    CPDF_TextStream(CFX_WideTextBuf& buffer, FX_BOOL bUseLF, CFX_PtrArray* pObjArray);
    ~CPDF_TextStream() {}
    FX_BOOL ProcessObject(const CPDF_TextObject* pObj, FX_BOOL bFirstLine);
    CFX_WideTextBuf&	m_Buffer;
    FX_BOOL				m_bUseLF;
    CFX_PtrArray*		m_pObjArray;
    const CPDF_TextObject*	m_pLastObj;
};
CPDF_TextStream::CPDF_TextStream(CFX_WideTextBuf& buffer, FX_BOOL bUseLF, CFX_PtrArray* pObjArray) : m_Buffer(buffer)
{
    m_pLastObj = NULL;
    m_bUseLF = bUseLF;
    m_pObjArray = pObjArray;
}
FX_BOOL FPDFText_IsSameTextObject(const CPDF_TextObject* pTextObj1, const CPDF_TextObject* pTextObj2)
{
    if (!pTextObj1 || !pTextObj2) {
        return FALSE;
    }
    CFX_FloatRect rcPreObj(pTextObj2->m_Left, pTextObj2->m_Bottom, pTextObj2->m_Right, pTextObj2->m_Top);
    CFX_FloatRect rcCurObj(pTextObj1->m_Left, pTextObj1->m_Bottom, pTextObj1->m_Right, pTextObj1->m_Top);
    if (rcPreObj.IsEmpty() && rcCurObj.IsEmpty()) {
        return TRUE;
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
int GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont)
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
int FPDFText_ProcessInterObj(const CPDF_TextObject* pPrevObj, const CPDF_TextObject* pObj)
{
    if(FPDFText_IsSameTextObject(pPrevObj, pObj)) {
        return -1;
    }
    CPDF_TextObjectItem item;
    int nItem = pPrevObj->CountItems();
    pPrevObj->GetItemInfo(nItem - 1, &item);
    FX_WCHAR preChar = 0, curChar = 0;
    CFX_WideString wstr = pPrevObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
    if(wstr.GetLength()) {
        preChar = wstr.GetAt(0);
    }
    FX_FLOAT last_pos = item.m_OriginX;
    int nLastWidth = GetCharWidth(item.m_CharCode, pPrevObj->GetFont());
    FX_FLOAT last_width = nLastWidth * pPrevObj->GetFontSize() / 1000;
    last_width = FXSYS_fabs(last_width);
    pObj->GetItemInfo(0, &item);
    wstr = pObj->GetFont()->UnicodeFromCharCode(item.m_CharCode);
    if(wstr.GetLength()) {
        curChar = wstr.GetAt(0);
    }
    int nThisWidth = GetCharWidth(item.m_CharCode, pObj->GetFont());
    FX_FLOAT this_width = nThisWidth * pObj->GetFontSize() / 1000;
    this_width = FXSYS_fabs(this_width);
    FX_FLOAT threshold = last_width > this_width ? last_width / 4 : this_width / 4;
    CFX_AffineMatrix prev_matrix, prev_reverse;
    pPrevObj->GetTextMatrix(&prev_matrix);
    prev_reverse.SetReverse(prev_matrix);
    FX_FLOAT x = pObj->GetPosX(), y = pObj->GetPosY();
    prev_reverse.Transform(x, y);
    if (FXSYS_fabs(y) > threshold * 2) {
        return 2;
    }
    threshold = (FX_FLOAT)(nLastWidth > nThisWidth ? nLastWidth : nThisWidth);
    threshold = threshold > 400 ? (threshold < 700 ? threshold / 4 :  threshold / 5) : (threshold / 2);
    threshold *= nLastWidth > nThisWidth ? FXSYS_fabs(pPrevObj->GetFontSize()) : FXSYS_fabs(pObj->GetFontSize());
    threshold /= 1000;
    if (FXSYS_fabs(last_pos + last_width - x) > threshold && curChar != L' ' && preChar != L' ')
        if(curChar != L' ' && preChar != L' ') {
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
    if(last_pos + last_width > x + this_width && curChar == L' ') {
        return 3;
    }
    return 0;
}
FX_BOOL CPDF_TextStream::ProcessObject(const CPDF_TextObject* pObj, FX_BOOL bFirstLine)
{
    CPDF_Font* pFont = pObj->GetFont();
    CFX_AffineMatrix matrix;
    pObj->GetTextMatrix(&matrix);
    int item_index = 0;
    if (m_pLastObj) {
        int result = FPDFText_ProcessInterObj(m_pLastObj, pObj);
        if (result == 2) {
            int len = m_Buffer.GetLength();
            if (len && m_bUseLF && m_Buffer.GetBuffer()[len - 1] == L'-') {
                m_Buffer.Delete(len - 1, 1);
                if (m_pObjArray) {
                    m_pObjArray->RemoveAt((len - 1) * 2, 2);
                }
            } else {
                if (bFirstLine) {
                    return TRUE;
                }
                if (m_bUseLF) {
                    m_Buffer.AppendChar(L'\r');
                    m_Buffer.AppendChar(L'\n');
                    if (m_pObjArray) {
                        for (int i = 0; i < 4; i ++) {
                            m_pObjArray->Add(NULL);
                        }
                    }
                } else {
                    m_Buffer.AppendChar(' ');
                    if (m_pObjArray) {
                        m_pObjArray->Add(NULL);
                        m_pObjArray->Add(NULL);
                    }
                }
            }
        } else if (result == 1) {
            m_Buffer.AppendChar(L' ');
            if (m_pObjArray) {
                m_pObjArray->Add(NULL);
                m_pObjArray->Add(NULL);
            }
        } else if (result == -1) {
            m_pLastObj = pObj;
            return FALSE;
        } else if (result == 3) {
            item_index = 1;
        }
    }
    m_pLastObj = pObj;
    int nItems = pObj->CountItems();
    FX_FLOAT Ignorekerning = 0;
    for(int i = 1; i < nItems - 1; i += 2) {
        CPDF_TextObjectItem item;
        pObj->GetItemInfo(i, &item);
        if (item.m_CharCode == (FX_DWORD) - 1) {
            if(i == 1) {
                Ignorekerning = item.m_OriginX;
            } else if(Ignorekerning > item.m_OriginX) {
                Ignorekerning = item.m_OriginX;
            }
        } else {
            Ignorekerning = 0;
            break;
        }
    }
    FX_FLOAT spacing = 0;
    for (; item_index < nItems; item_index ++) {
        CPDF_TextObjectItem item;
        pObj->GetItemInfo(item_index, &item);
        if (item.m_CharCode == (FX_DWORD) - 1) {
            CFX_WideString wstr = m_Buffer.GetWideString();
            if (wstr.IsEmpty() || wstr.GetAt(wstr.GetLength() - 1) == L' ') {
                continue;
            }
            FX_FLOAT fontsize_h = pObj->m_TextState.GetFontSizeH();
            spacing = -fontsize_h * (item.m_OriginX - Ignorekerning) / 1000;
            continue;
        }
        FX_FLOAT charSpace = pObj->m_TextState.GetObject()->m_CharSpace;
        if(nItems > 3 && !spacing) {
            charSpace = 0;
        }
        if((spacing || charSpace) && item_index > 0) {
            int last_width = 0;
            FX_FLOAT fontsize_h = pObj->m_TextState.GetFontSizeH();
            FX_DWORD space_charcode = pFont->CharCodeFromUnicode(' ');
            FX_FLOAT threshold = 0;
            if (space_charcode != -1) {
                threshold = fontsize_h * pFont->GetCharWidthF(space_charcode) / 1000 ;
            }
            if(threshold > fontsize_h / 3) {
                threshold = 0;
            } else {
                threshold /= 2;
            }
            if (threshold == 0) {
                threshold = fontsize_h;
                int this_width = FXSYS_abs(GetCharWidth(item.m_CharCode, pFont));
                threshold = this_width > last_width ? (FX_FLOAT)this_width : (FX_FLOAT)last_width;
                int nDivide = 6;
                if (threshold < 300) {
                    nDivide = 2;
                } else if (threshold < 500) {
                    nDivide = 4;
                } else if (threshold < 700) {
                    nDivide = 5;
                }
                threshold = threshold / nDivide;
                threshold = fontsize_h * threshold / 1000;
            }
            if(charSpace > 0.001) {
                spacing += matrix.TransformDistance(charSpace);
            } else if(charSpace < -0.001) {
                spacing -= matrix.TransformDistance(FXSYS_fabs(charSpace));
            }
            if (threshold && (spacing && spacing >= threshold) ) {
                m_Buffer.AppendChar(L' ');
                if (m_pObjArray) {
                    m_pObjArray->Add(NULL);
                    m_pObjArray->Add(NULL);
                }
            }
            if (item.m_CharCode == (FX_DWORD) - 1) {
                continue;
            }
            spacing = 0;
        }
        CFX_WideString unicode_str = pFont->UnicodeFromCharCode(item.m_CharCode);
        if (unicode_str.IsEmpty()) {
            m_Buffer.AppendChar((FX_WCHAR)item.m_CharCode);
            if (m_pObjArray) {
                m_pObjArray->Add((void*)pObj);
                m_pObjArray->Add((void*)(FX_INTPTR)item_index);
            }
        } else {
            m_Buffer << unicode_str;
            if (m_pObjArray) {
                for (int i = 0; i < unicode_str.GetLength(); i ++) {
                    m_pObjArray->Add((void*)pObj);
                    m_pObjArray->Add((void*)(FX_INTPTR)item_index);
                }
            }
        }
    }
    return FALSE;
}
void _PDF_GetTextStream_Unicode(CFX_WideTextBuf& buffer, CPDF_PageObjects* pPage, FX_BOOL bUseLF,
                                CFX_PtrArray* pObjArray)
{
    CPDF_TextStream textstream(buffer, bUseLF, pObjArray);
    FX_POSITION pos = pPage->GetFirstObjectPosition();
    while (pos) {
        CPDF_PageObject* pObject = pPage->GetNextObject(pos);
        if (pObject == NULL) {
            continue;
        }
        if (pObject->m_Type != PDFPAGE_TEXT) {
            continue;
        }
        textstream.ProcessObject((CPDF_TextObject*)pObject, FALSE);
    }
}
CFX_WideString PDF_GetFirstTextLine_Unicode(CPDF_Document* pDoc, CPDF_Dictionary* pPage)
{
    CFX_WideTextBuf buffer;
    buffer.EstimateSize(0, 1024);
    CPDF_Page page;
    page.Load(pDoc, pPage);
    CPDF_ParseOptions options;
    options.m_bTextOnly = TRUE;
    options.m_bSeparateForm = FALSE;
    page.ParseContent(&options);
    CPDF_TextStream textstream(buffer, FALSE, NULL);
    FX_POSITION pos = page.GetFirstObjectPosition();
    while (pos) {
        CPDF_PageObject* pObject = page.GetNextObject(pos);
        if (pObject->m_Type != PDFPAGE_TEXT) {
            continue;
        }
        if (textstream.ProcessObject((CPDF_TextObject*)pObject, TRUE)) {
            break;
        }
    }
    return buffer.GetWideString();
}
