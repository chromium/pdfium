// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fpdfdoc/fpdf_vt.h"
#include "pdf_vt.h"
const FX_BYTE gFontSizeSteps[] = {	4, 6, 8, 9, 10,	12, 14, 18, 20, 25,	30, 35, 40, 45, 50,	55, 60, 70, 80, 90,	100, 110, 120, 130, 144};
#define PVT_RETURN_LENGTH					1
#define PVT_DEFAULT_FONTSIZE				18.0f
#define PVTWORD_SCRIPT_NORMAL				0
#define PVTWORD_SCRIPT_SUPER				1
#define PVTWORD_SCRIPT_SUB					2
#define	PVT_FONTSCALE						0.001f
#define PVT_PERCENT							0.01f
#define PVT_HALF							0.5f
CLine::CLine()
{
}
CLine::~CLine()
{
}
CPVT_WordPlace CLine::GetBeginWordPlace() const
{
    return CPVT_WordPlace(LinePlace.nSecIndex, LinePlace.nLineIndex, -1);
}
CPVT_WordPlace CLine::GetEndWordPlace() const
{
    return CPVT_WordPlace(LinePlace.nSecIndex, LinePlace.nLineIndex, m_LineInfo.nEndWordIndex);
}
CPVT_WordPlace CLine::GetPrevWordPlace(const CPVT_WordPlace & place) const
{
    if (place.nWordIndex > m_LineInfo.nEndWordIndex) {
        return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, m_LineInfo.nEndWordIndex);
    }
    return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, place.nWordIndex - 1);
}
CPVT_WordPlace CLine::GetNextWordPlace(const CPVT_WordPlace & place) const
{
    if (place.nWordIndex < m_LineInfo.nBeginWordIndex) {
        return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, m_LineInfo.nBeginWordIndex);
    }
    return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, place.nWordIndex + 1);
}
CSection::CSection(CPDF_VariableText * pVT) : m_pVT(pVT)
{
}
CSection::~CSection()
{
    ResetAll();
}
void CSection::ResetAll()
{
    ResetWordArray();
    ResetLineArray();
}
void CSection::ResetLineArray()
{
    m_LineArray.RemoveAll();
}
void CSection::ResetWordArray()
{
    for (FX_INT32 i = 0, sz = m_WordArray.GetSize(); i < sz; i++) {
        delete m_WordArray.GetAt(i);
    }
    m_WordArray.RemoveAll();
}
void CSection::ResetLinePlace()
{
    for (FX_INT32 i = 0, sz = m_LineArray.GetSize(); i < sz; i++) {
        if (CLine * pLine = m_LineArray.GetAt(i)) {
            pLine->LinePlace = CPVT_WordPlace(SecPlace.nSecIndex, i, -1);
        }
    }
}
CPVT_WordPlace CSection::AddWord(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo)
{
    CPVT_WordInfo * pWord = new CPVT_WordInfo(wordinfo);
    FX_INT32 nWordIndex = FPDF_MAX(FPDF_MIN(place.nWordIndex, this->m_WordArray.GetSize()), 0);
    if (nWordIndex == m_WordArray.GetSize()) {
        m_WordArray.Add(pWord);
    } else {
        m_WordArray.InsertAt(nWordIndex, pWord);
    }
    return place;
}
CPVT_WordPlace CSection::AddLine(const CPVT_LineInfo & lineinfo)
{
    return CPVT_WordPlace(SecPlace.nSecIndex, m_LineArray.Add(lineinfo), -1);
}
CPVT_FloatRect CSection::Rearrange()
{
    ASSERT(m_pVT != NULL);
    if (m_pVT->m_nCharArray > 0) {
        return CTypeset(this).CharArray();
    } else {
        return CTypeset(this).Typeset();
    }
}
CPVT_Size CSection::GetSectionSize(FX_FLOAT fFontSize)
{
    return CTypeset(this).GetEditSize(fFontSize);
}
CPVT_WordPlace CSection::GetBeginWordPlace() const
{
    if (CLine * pLine = m_LineArray.GetAt(0)) {
        return pLine->GetBeginWordPlace();
    } else {
        return SecPlace;
    }
}
CPVT_WordPlace CSection::GetEndWordPlace() const
{
    if (CLine * pLine = m_LineArray.GetAt(m_LineArray.GetSize() - 1)) {
        return pLine->GetEndWordPlace();
    } else {
        return this->SecPlace;
    }
}
CPVT_WordPlace CSection::GetPrevWordPlace(const CPVT_WordPlace & place) const
{
    if (place.nLineIndex < 0) {
        return GetBeginWordPlace();
    }
    if (place.nLineIndex >= m_LineArray.GetSize()) {
        return GetEndWordPlace();
    }
    if (CLine * pLine = m_LineArray.GetAt(place.nLineIndex)) {
        if (place.nWordIndex == pLine->m_LineInfo.nBeginWordIndex) {
            return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, -1);
        } else if (place.nWordIndex < pLine->m_LineInfo.nBeginWordIndex) {
            if (CLine * pPrevLine = m_LineArray.GetAt(place.nLineIndex - 1)) {
                return pPrevLine->GetEndWordPlace();
            }
        } else {
            return pLine->GetPrevWordPlace(place);
        }
    }
    return place;
}
CPVT_WordPlace CSection::GetNextWordPlace(const CPVT_WordPlace & place) const
{
    if (place.nLineIndex < 0) {
        return GetBeginWordPlace();
    }
    if (place.nLineIndex >= m_LineArray.GetSize()) {
        return GetEndWordPlace();
    }
    if (CLine * pLine = m_LineArray.GetAt(place.nLineIndex)) {
        if (place.nWordIndex >= pLine->m_LineInfo.nEndWordIndex) {
            if (CLine * pNextLine = m_LineArray.GetAt(place.nLineIndex + 1)) {
                return pNextLine->GetBeginWordPlace();
            }
        } else {
            return pLine->GetNextWordPlace(place);
        }
    }
    return place;
}
void CSection::UpdateWordPlace(CPVT_WordPlace & place) const
{
    FX_INT32 nLeft = 0;
    FX_INT32 nRight = m_LineArray.GetSize() - 1;
    FX_INT32 nMid = (nLeft + nRight) / 2;
    while (nLeft <= nRight) {
        if (CLine * pLine = m_LineArray.GetAt(nMid)) {
            if (place.nWordIndex < pLine->m_LineInfo.nBeginWordIndex) {
                nRight = nMid - 1;
                nMid = (nLeft + nRight) / 2;
            } else if (place.nWordIndex > pLine->m_LineInfo.nEndWordIndex) {
                nLeft = nMid + 1;
                nMid = (nLeft + nRight) / 2;
            } else {
                place.nLineIndex = nMid;
                return;
            }
        } else {
            break;
        }
    }
}
CPVT_WordPlace CSection::SearchWordPlace(const CPDF_Point & point) const
{
    ASSERT(m_pVT != NULL);
    CPVT_WordPlace place = GetBeginWordPlace();
    FX_BOOL bUp = TRUE;
    FX_BOOL bDown = TRUE;
    FX_INT32 nLeft = 0;
    FX_INT32 nRight = m_LineArray.GetSize() - 1;
    FX_INT32 nMid = m_LineArray.GetSize() / 2;
    FX_FLOAT fTop = 0;
    FX_FLOAT fBottom = 0;
    while (nLeft <= nRight) {
        if (CLine * pLine = m_LineArray.GetAt(nMid)) {
            fTop = pLine->m_LineInfo.fLineY - pLine->m_LineInfo.fLineAscent - m_pVT->GetLineLeading(m_SecInfo);
            fBottom = pLine->m_LineInfo.fLineY - pLine->m_LineInfo.fLineDescent;
            if (IsFloatBigger(point.y, fTop)) {
                bUp = FALSE;
            }
            if (IsFloatSmaller(point.y, fBottom)) {
                bDown = FALSE;
            }
            if (IsFloatSmaller(point.y, fTop)) {
                nRight = nMid - 1;
                nMid = (nLeft + nRight) / 2;
                continue;
            } else if (IsFloatBigger(point.y, fBottom)) {
                nLeft = nMid + 1;
                nMid = (nLeft + nRight) / 2;
                continue;
            } else {
                place = SearchWordPlace(point.x,
                                        CPVT_WordRange(pLine->GetNextWordPlace(pLine->GetBeginWordPlace()), pLine->GetEndWordPlace())
                                       );
                place.nLineIndex = nMid;
                return place;
            }
        }
    }
    if (bUp) {
        place = GetBeginWordPlace();
    }
    if (bDown) {
        place = GetEndWordPlace();
    }
    return place;
}
CPVT_WordPlace CSection::SearchWordPlace(FX_FLOAT fx, const CPVT_WordPlace & lineplace) const
{
    if (CLine * pLine = m_LineArray.GetAt(lineplace.nLineIndex)) {
        return SearchWordPlace(fx - m_SecInfo.rcSection.left,
                               CPVT_WordRange(pLine->GetNextWordPlace(pLine->GetBeginWordPlace()), pLine->GetEndWordPlace()));
    }
    return GetBeginWordPlace();
}
CPVT_WordPlace CSection::SearchWordPlace(FX_FLOAT fx, const CPVT_WordRange & range) const
{
    CPVT_WordPlace wordplace = range.BeginPos;
    wordplace.nWordIndex = -1;
    if (!m_pVT)	{
        return wordplace;
    }
    FX_INT32 nLeft = range.BeginPos.nWordIndex;
    FX_INT32 nRight = range.EndPos.nWordIndex + 1;
    FX_INT32 nMid = (nLeft + nRight) / 2;
    while (nLeft < nRight) {
        if (nMid == nLeft) {
            break;
        }
        if (nMid == nRight) {
            nMid--;
            break;
        }
        if (CPVT_WordInfo * pWord = m_WordArray.GetAt(nMid)) {
            if (fx > pWord->fWordX + m_pVT->GetWordWidth(*pWord) * PVT_HALF) {
                nLeft = nMid;
                nMid = (nLeft + nRight) / 2;
                continue;
            } else {
                nRight = nMid;
                nMid = (nLeft + nRight) / 2;
                continue;
            }
        } else {
            break;
        }
    }
    if (CPVT_WordInfo * pWord = m_WordArray.GetAt(nMid)) {
        if (fx > pWord->fWordX + m_pVT->GetWordWidth(*pWord) * PVT_HALF) {
            wordplace.nWordIndex = nMid;
        }
    }
    return wordplace;
}
void CSection::ClearLeftWords(FX_INT32 nWordIndex)
{
    for (FX_INT32 i = nWordIndex; i >= 0; i--) {
        delete m_WordArray.GetAt(i);
        m_WordArray.RemoveAt(i);
    }
}
void CSection::ClearRightWords(FX_INT32 nWordIndex)
{
    for (FX_INT32 i = m_WordArray.GetSize() - 1; i > nWordIndex; i--) {
        delete m_WordArray.GetAt(i);
        m_WordArray.RemoveAt(i);
    }
}
void CSection::ClearMidWords(FX_INT32 nBeginIndex, FX_INT32 nEndIndex)
{
    for (FX_INT32 i = nEndIndex; i > nBeginIndex; i--) {
        delete m_WordArray.GetAt(i);
        m_WordArray.RemoveAt(i);
    }
}
void CSection::ClearWords(const CPVT_WordRange & PlaceRange)
{
    CPVT_WordPlace SecBeginPos = GetBeginWordPlace();
    CPVT_WordPlace SecEndPos = GetEndWordPlace();
    if (PlaceRange.BeginPos.WordCmp(SecBeginPos) >= 0) {
        if (PlaceRange.EndPos.WordCmp(SecEndPos) <= 0) {
            ClearMidWords(PlaceRange.BeginPos.nWordIndex, PlaceRange.EndPos.nWordIndex);
        } else {
            ClearRightWords(PlaceRange.BeginPos.nWordIndex);
        }
    } else if (PlaceRange.EndPos.WordCmp(SecEndPos) <= 0) {
        ClearLeftWords(PlaceRange.EndPos.nWordIndex);
    } else {
        ResetWordArray();
    }
}
void CSection::ClearWord(const CPVT_WordPlace & place)
{
    delete m_WordArray.GetAt(place.nWordIndex);
    m_WordArray.RemoveAt(place.nWordIndex);
}
CTypeset::CTypeset(CSection * pSection) : m_rcRet(0.0f, 0.0f, 0.0f, 0.0f), m_pVT(pSection->m_pVT), m_pSection(pSection)
{
}
CTypeset::~CTypeset()
{
}
CPVT_FloatRect CTypeset::CharArray()
{
    ASSERT(m_pSection != NULL);
    ASSERT(m_pVT != NULL);
    FX_FLOAT fLineAscent = m_pVT->GetFontAscent(m_pVT->GetDefaultFontIndex(), m_pVT->GetFontSize());
    FX_FLOAT fLineDescent = m_pVT->GetFontDescent(m_pVT->GetDefaultFontIndex(), m_pVT->GetFontSize());
    m_rcRet.Default();
    FX_FLOAT x = 0.0f, y = 0.0f;
    FX_FLOAT fNextWidth;
    FX_INT32 nStart = 0;
    FX_FLOAT fNodeWidth = m_pVT->GetPlateWidth() / (m_pVT->m_nCharArray <= 0 ? 1 : m_pVT->m_nCharArray);
    if (CLine * pLine = m_pSection->m_LineArray.GetAt(0)) {
        x = 0.0f;
        y +=  m_pVT->GetLineLeading(m_pSection->m_SecInfo);
        y += fLineAscent;
        nStart = 0;
        switch (m_pVT->GetAlignment(m_pSection->m_SecInfo)) {
            case 0:
                pLine->m_LineInfo.fLineX = fNodeWidth * PVT_HALF;
                break;
            case 1:
                nStart = (m_pVT->m_nCharArray - m_pSection->m_WordArray.GetSize()) / 2;
                pLine->m_LineInfo.fLineX = fNodeWidth * nStart - fNodeWidth * PVT_HALF;
                break;
            case 2:
                nStart = m_pVT->m_nCharArray - m_pSection->m_WordArray.GetSize();
                pLine->m_LineInfo.fLineX = fNodeWidth * nStart - fNodeWidth * PVT_HALF;
                break;
        }
        for (FX_INT32 w = 0, sz = m_pSection->m_WordArray.GetSize(); w < sz; w++) {
            if (w >= m_pVT->m_nCharArray) {
                break;
            }
            fNextWidth = 0;
            if (CPVT_WordInfo * pNextWord = (CPVT_WordInfo *)m_pSection->m_WordArray.GetAt(w + 1)) {
                pNextWord->fWordTail = 0;
                fNextWidth = m_pVT->GetWordWidth(*pNextWord);
            }
            if (CPVT_WordInfo * pWord = (CPVT_WordInfo *)m_pSection->m_WordArray.GetAt(w)) {
                pWord->fWordTail = 0;
                FX_FLOAT fWordWidth = m_pVT->GetWordWidth(*pWord);
                FX_FLOAT fWordAscent = m_pVT->GetWordAscent(*pWord);
                FX_FLOAT fWordDescent = m_pVT->GetWordDescent(*pWord);
                x = (FX_FLOAT)(fNodeWidth * (w + nStart + 0.5) - fWordWidth * PVT_HALF);
                pWord->fWordX = x;
                pWord->fWordY = y;
                if (w == 0) {
                    pLine->m_LineInfo.fLineX = x;
                }
                if (w != m_pSection->m_WordArray.GetSize() - 1)
                    pWord->fWordTail = (fNodeWidth - (fWordWidth + fNextWidth) * PVT_HALF > 0 ?
                                        fNodeWidth - (fWordWidth + fNextWidth) * PVT_HALF : 0);
                else {
                    pWord->fWordTail = 0;
                }
                x += fWordWidth;
                fLineAscent = FPDF_MAX(fLineAscent, fWordAscent);
                fLineDescent = FPDF_MIN(fLineDescent, fWordDescent);
            }
        }
        pLine->m_LineInfo.nBeginWordIndex = 0;
        pLine->m_LineInfo.nEndWordIndex = m_pSection->m_WordArray.GetSize() - 1;
        pLine->m_LineInfo.fLineY = y;
        pLine->m_LineInfo.fLineWidth = 	x - pLine->m_LineInfo.fLineX;
        pLine->m_LineInfo.fLineAscent = fLineAscent;
        pLine->m_LineInfo.fLineDescent = fLineDescent;
        y += (-fLineDescent);
    }
    return m_rcRet = CPVT_FloatRect(0, 0, x, y);
}
CPVT_Size CTypeset::GetEditSize(FX_FLOAT fFontSize)
{
    ASSERT(m_pSection != NULL);
    ASSERT(m_pVT != NULL);
    SplitLines(FALSE, fFontSize);
    return CPVT_Size(m_rcRet.Width(), m_rcRet.Height());
}
CPVT_FloatRect CTypeset::Typeset()
{
    ASSERT(m_pSection != NULL);
    ASSERT(m_pVT != NULL);
    m_pSection->m_LineArray.Empty();
    SplitLines(TRUE, 0.0f);
    m_pSection->m_LineArray.Clear();
    OutputLines();
    return m_rcRet;
}
static int special_chars[128] = {
    0x0000, 0x000C, 0x0008, 0x000C, 0x0008, 0x0000, 0x0020, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0008, 0x0008, 0x0000, 0x0010, 0x0000, 0x0000, 0x0028,
    0x000C, 0x0008, 0x0000, 0x0000, 0x0028, 0x0028, 0x0028, 0x0028,
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0002, 0x0002, 0x0008, 0x0008, 0x0000, 0x0000, 0x0000, 0x0008,
    0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x000C, 0x0000, 0x0008, 0x0000, 0x0000,
    0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
    0x0001, 0x0001, 0x0001, 0x000C, 0x0000, 0x0008, 0x0000, 0x0000,
};
static FX_BOOL IsLatin(FX_WORD word)
{
    if (word <= 0x007F) {
        if (special_chars[word] & 0x0001) {
            return TRUE;
        }
    }
    if ((word >= 0x00C0 && word <= 0x00FF) ||
            (word >= 0x0100 && word <= 0x024F) ||
            (word >= 0x1E00 && word <= 0x1EFF) ||
            (word >= 0x2C60 && word <= 0x2C7F) ||
            (word >= 0xA720 && word <= 0xA7FF) ||
            (word >= 0xFF21 && word <= 0xFF3A) ||
            (word >= 0xFF41 && word <= 0xFF5A)) {
        return TRUE;
    }
    return FALSE;
}
static FX_BOOL IsDigit(FX_DWORD word)
{
    return (word >= 0x0030 && word <= 0x0039) ? TRUE : FALSE;
}
static FX_BOOL IsCJK(FX_DWORD word)
{
    if ((word >= 0x1100 && word <= 0x11FF) ||
            (word >= 0x2E80 && word <= 0x2FFF) ||
            (word >= 0x3040 && word <= 0x9FBF) ||
            (word >= 0xAC00 && word <= 0xD7AF) ||
            (word >= 0xF900 && word <= 0xFAFF) ||
            (word >= 0xFE30 && word <= 0xFE4F) ||
            (word >= 0x20000 && word <= 0x2A6DF) ||
            (word >= 0x2F800 && word <= 0x2FA1F)) {
        return TRUE;
    }
    if (word >= 0x3000 && word <= 0x303F) {
        if (word == 0x3005 || word == 0x3006 || word == 0x3021 || word == 0x3022 ||
                word == 0x3023 || word == 0x3024 || word == 0x3025 || word == 0x3026 ||
                word == 0x3027 || word == 0x3028 || word == 0x3029 || word == 0x3031 ||
                word == 0x3032 || word == 0x3033 || word == 0x3034 || word == 0x3035) {
            return TRUE;
        }
        return FALSE;
    }
    if (word >= 0xFF66 && word <= 0xFF9D) {
        return TRUE;
    }
    return FALSE;
}
static FX_BOOL IsPunctuation(FX_DWORD word)
{
    if (word <= 0x007F) {
        if ((special_chars[word] >> 3) & 1) {
            return TRUE;
        }
    } else if (word >= 0x0080 && word <= 0x00FF) {
        if (word == 0x0082 || word == 0x0084 || word == 0x0085 || word == 0x0091 ||
                word == 0x0092 || word == 0x0093 || word <= 0x0094 || word == 0x0096 ||
                word == 0x00B4 || word == 0x00B8) {
            return TRUE;
        }
    } else if (word >= 0x2000 && word <= 0x206F) {
        if (word == 0x2010 || word == 0x2011 || word == 0x2012 || word == 0x2013 ||
                word == 0x2018 || word == 0x2019 || word == 0x201A || word == 0x201B ||
                word == 0x201C || word == 0x201D || word == 0x201E || word == 0x201F ||
                word == 0x2032 || word == 0x2033 || word == 0x2034 || word == 0x2035 ||
                word == 0x2036 || word == 0x2037 || word == 0x203C || word == 0x203D ||
                word == 0x203E || word == 0x2044) {
            return TRUE;
        }
    } else if (word >= 0x3000 && word <= 0x303F) {
        if (word == 0x3001 || word == 0x3002 || word == 0x3003 || word == 0x3005 ||
                word == 0x3009 || word == 0x300A || word == 0x300B || word == 0x300C ||
                word == 0x300D || word == 0x300F || word == 0x300E || word == 0x3010 ||
                word == 0x3011 || word == 0x3014 || word == 0x3015 || word == 0x3016 ||
                word == 0x3017 || word == 0x3018 || word == 0x3019 || word == 0x301A ||
                word == 0x301B || word == 0x301D || word == 0x301E || word == 0x301F) {
            return TRUE;
        }
    } else if (word >= 0xFE50 && word <= 0xFE6F) {
        if ((word >= 0xFE50 && word <= 0xFE5E) || word == 0xFE63) {
            return TRUE;
        }
    } else if (word >= 0xFF00 && word <= 0xFFEF) {
        if (word == 0xFF01 || word == 0xFF02 || word == 0xFF07 || word == 0xFF08 ||
                word == 0xFF09 || word == 0xFF0C || word == 0xFF0E || word == 0xFF0F ||
                word == 0xFF1A || word == 0xFF1B || word == 0xFF1F || word == 0xFF3B ||
                word == 0xFF3D || word == 0xFF40 || word == 0xFF5B || word == 0xFF5C ||
                word == 0xFF5D || word == 0xFF61 || word == 0xFF62 || word == 0xFF63 ||
                word == 0xFF64 || word == 0xFF65 || word == 0xFF9E || word == 0xFF9F) {
            return TRUE;
        }
    }
    return FALSE;
}
static FX_BOOL IsConnectiveSymbol(FX_DWORD word)
{
    if (word <= 0x007F) {
        if ((special_chars[word] >> 5) & 1) {
            return TRUE;
        }
    }
    return FALSE;
}
static FX_BOOL IsOpenStylePunctuation(FX_DWORD word)
{
    if (word <= 0x007F) {
        if ((special_chars[word] >> 2) & 1) {
            return TRUE;
        }
    } else if (word == 0x300A || word == 0x300C || word == 0x300E || word == 0x3010 ||
               word == 0x3014 || word == 0x3016 || word == 0x3018 || word == 0x301A ||
               word == 0xFF08 || word == 0xFF3B || word == 0xFF5B || word == 0xFF62) {
        return TRUE;
    }
    return FALSE;
}
static FX_BOOL IsCurrencySymbol(FX_WORD word)
{
    if (word == 0x0024 || word == 0x0080 || word == 0x00A2 || word == 0x00A3 ||
            word == 0x00A4 || word == 0x00A5 || (word >= 0x20A0 && word <= 0x20CF) ||
            word == 0xFE69 || word == 0xFF04 || word == 0xFFE0 || word == 0xFFE1 ||
            word == 0xFFE5 || word == 0xFFE6) {
        return TRUE;
    }
    return FALSE;
}
static FX_BOOL IsPrefixSymbol(FX_WORD word)
{
    if (IsCurrencySymbol(word)) {
        return TRUE;
    }
    if (word == 0x2116) {
        return TRUE;
    }
    return FALSE;
}
static FX_BOOL IsSpace(FX_WORD word)
{
    return (word == 0x0020 || word == 0x3000) ? TRUE : FALSE;
}
static FX_BOOL NeedDivision(FX_WORD prevWord, FX_WORD curWord)
{
    if ((IsLatin(prevWord) || IsDigit(prevWord)) && (IsLatin(curWord) || IsDigit(curWord))) {
        return FALSE;
    } else if (IsSpace(curWord) || IsPunctuation(curWord)) {
        return FALSE;
    } else if (IsConnectiveSymbol(prevWord) || IsConnectiveSymbol(curWord)) {
        return FALSE;
    } else if (IsSpace(prevWord) || IsPunctuation(prevWord)) {
        return TRUE;
    } else if (IsPrefixSymbol(prevWord)) {
        return FALSE;
    } else if (IsPrefixSymbol(curWord) || IsCJK(curWord)) {
        return TRUE;
    } else if (IsCJK(prevWord)) {
        return TRUE;
    }
    return FALSE;
}
void CTypeset::SplitLines(FX_BOOL bTypeset, FX_FLOAT fFontSize)
{
    ASSERT(m_pVT != NULL);
    ASSERT(m_pSection != NULL);
    FX_INT32 nLineHead = 0;
    FX_INT32 nLineTail = 0;
    FX_FLOAT fMaxX = 0.0f, fMaxY = 0.0f;
    FX_FLOAT fLineWidth = 0.0f, fBackupLineWidth = 0.0f;
    FX_FLOAT fLineAscent = 0.0f, fBackupLineAscent = 0.0f;
    FX_FLOAT fLineDescent = 0.0f, fBackupLineDescent = 0.0f;
    FX_INT32 nWordStartPos = 0;
    FX_BOOL bFullWord = FALSE;
    FX_INT32 nLineFullWordIndex = 0;
    FX_INT32 nCharIndex = 0;
    CPVT_LineInfo line;
    FX_FLOAT fWordWidth = 0;
    FX_FLOAT fTypesetWidth = FPDF_MAX(m_pVT->GetPlateWidth() - m_pVT->GetLineIndent(m_pSection->m_SecInfo), 0.0f);
    FX_INT32 nTotalWords = m_pSection->m_WordArray.GetSize();
    FX_BOOL bOpened = FALSE;
    if (nTotalWords > 0) {
        FX_INT32 i = 0;
        while (i < nTotalWords) {
            CPVT_WordInfo * pWord = m_pSection->m_WordArray.GetAt(i);
            CPVT_WordInfo* pOldWord = pWord;
            if (i > 0) {
                pOldWord = m_pSection->m_WordArray.GetAt(i - 1);
            }
            if (pWord) {
                if (bTypeset) {
                    fLineAscent = FPDF_MAX(fLineAscent, m_pVT->GetWordAscent(*pWord, TRUE));
                    fLineDescent = FPDF_MIN(fLineDescent, m_pVT->GetWordDescent(*pWord, TRUE));
                    fWordWidth = m_pVT->GetWordWidth(*pWord);
                } else {
                    fLineAscent = FPDF_MAX(fLineAscent, m_pVT->GetWordAscent(*pWord, fFontSize));
                    fLineDescent = FPDF_MIN(fLineDescent, m_pVT->GetWordDescent(*pWord, fFontSize));
                    fWordWidth = m_pVT->GetWordWidth(pWord->nFontIndex,
                                                     pWord->Word,
                                                     m_pVT->m_wSubWord,
                                                     m_pVT->m_fCharSpace,
                                                     m_pVT->m_nHorzScale,
                                                     fFontSize,
                                                     pWord->fWordTail,
                                                     0);
                }
                if (!bOpened) {
                    if (IsOpenStylePunctuation(pWord->Word)) {
                        bOpened = TRUE;
                        bFullWord = TRUE;
                    } else if (pOldWord != NULL) {
                        if (NeedDivision(pOldWord->Word, pWord->Word)) {
                            bFullWord = TRUE;
                        }
                    }
                } else {
                    if (!IsSpace(pWord->Word) && !IsOpenStylePunctuation(pWord->Word)) {
                        bOpened = FALSE;
                    }
                }
                if (bFullWord) {
                    bFullWord = FALSE;
                    if (nCharIndex > 0) {
                        nLineFullWordIndex ++;
                    }
                    nWordStartPos = i;
                    fBackupLineWidth = fLineWidth;
                    fBackupLineAscent = fLineAscent;
                    fBackupLineDescent = fLineDescent;
                }
                nCharIndex++;
            }
            if (m_pVT->m_bLimitWidth && fTypesetWidth > 0 &&
                    fLineWidth + fWordWidth > fTypesetWidth) {
                if (nLineFullWordIndex > 0) {
                    i = nWordStartPos;
                    fLineWidth = fBackupLineWidth;
                    fLineAscent = fBackupLineAscent;
                    fLineDescent = fBackupLineDescent;
                }
                if (nCharIndex == 1) {
                    fLineWidth =  fWordWidth;
                    i++;
                }
                nLineTail = i - 1;
                if (bTypeset) {
                    line.nBeginWordIndex = nLineHead;
                    line.nEndWordIndex = nLineTail;
                    line.nTotalWord = nLineTail - nLineHead + 1;
                    line.fLineWidth = fLineWidth;
                    line.fLineAscent = fLineAscent;
                    line.fLineDescent = fLineDescent;
                    m_pSection->AddLine(line);
                }
                fMaxY += (fLineAscent + m_pVT->GetLineLeading(m_pSection->m_SecInfo));
                fMaxY += (-fLineDescent);
                fMaxX = FPDF_MAX(fLineWidth, fMaxX);
                nLineHead = i;
                fLineWidth = 0.0f;
                fLineAscent = 0.0f;
                fLineDescent = 0.0f;
                nCharIndex = 0;
                nLineFullWordIndex = 0;
                bFullWord = FALSE;
            } else {
                fLineWidth += fWordWidth;
                i++;
            }
        }
        if (nLineHead <= nTotalWords - 1) {
            nLineTail = nTotalWords - 1;
            if (bTypeset) {
                line.nBeginWordIndex = nLineHead;
                line.nEndWordIndex = nLineTail;
                line.nTotalWord = nLineTail - nLineHead + 1;
                line.fLineWidth = fLineWidth;
                line.fLineAscent = fLineAscent;
                line.fLineDescent = fLineDescent;
                m_pSection->AddLine(line);
            }
            fMaxY += (fLineAscent + m_pVT->GetLineLeading(m_pSection->m_SecInfo));
            fMaxY += (-fLineDescent);
            fMaxX = FPDF_MAX(fLineWidth, fMaxX);
        }
    } else {
        if (bTypeset) {
            fLineAscent = m_pVT->GetLineAscent(m_pSection->m_SecInfo);
            fLineDescent = m_pVT->GetLineDescent(m_pSection->m_SecInfo);
        } else {
            fLineAscent = m_pVT->GetFontAscent(m_pVT->GetDefaultFontIndex(), fFontSize);
            fLineDescent = m_pVT->GetFontDescent(m_pVT->GetDefaultFontIndex(), fFontSize);
        }
        if (bTypeset) {
            line.nBeginWordIndex = -1;
            line.nEndWordIndex = -1;
            line.nTotalWord = 0;
            line.fLineWidth = 0;
            line.fLineAscent = fLineAscent;
            line.fLineDescent = fLineDescent;
            m_pSection->AddLine(line);
        }
        fMaxY += (m_pVT->GetLineLeading(m_pSection->m_SecInfo) + fLineAscent + (-fLineDescent));
    }
    m_rcRet = CPVT_FloatRect(0, 0, fMaxX, fMaxY);
}
void CTypeset::OutputLines()
{
    ASSERT(m_pVT != NULL);
    ASSERT(m_pSection != NULL);
    FX_FLOAT fMinX = 0.0f, fMinY = 0.0f, fMaxX = 0.0f, fMaxY = 0.0f;
    FX_FLOAT fPosX = 0.0f, fPosY = 0.0f;
    FX_FLOAT fLineIndent = m_pVT->GetLineIndent(m_pSection->m_SecInfo);
    FX_FLOAT fTypesetWidth = FPDF_MAX(m_pVT->GetPlateWidth() - fLineIndent, 0.0f);
    switch (m_pVT->GetAlignment(m_pSection->m_SecInfo)) {
        default:
        case 0:
            fMinX = 0.0f;
            break;
        case 1:
            fMinX = (fTypesetWidth - m_rcRet.Width())  * PVT_HALF;
            break;
        case 2:
            fMinX = fTypesetWidth - m_rcRet.Width();
            break;
    }
    fMaxX = fMinX + m_rcRet.Width();
    fMinY = 0.0f;
    fMaxY = m_rcRet.Height();
    FX_INT32 nTotalLines = m_pSection->m_LineArray.GetSize();
    if (nTotalLines > 0) {
        m_pSection->m_SecInfo.nTotalLine = nTotalLines;
        for (FX_INT32 l = 0; l < nTotalLines; l++) {
            if (CLine * pLine = m_pSection->m_LineArray.GetAt(l)) {
                switch (m_pVT->GetAlignment(m_pSection->m_SecInfo)) {
                    default:
                    case 0:
                        fPosX = 0;
                        break;
                    case 1:
                        fPosX = (fTypesetWidth - pLine->m_LineInfo.fLineWidth) * PVT_HALF;
                        break;
                    case 2:
                        fPosX = fTypesetWidth - pLine->m_LineInfo.fLineWidth;
                        break;
                }
                fPosX += fLineIndent;
                fPosY += m_pVT->GetLineLeading(m_pSection->m_SecInfo);
                fPosY += pLine->m_LineInfo.fLineAscent;
                pLine->m_LineInfo.fLineX = fPosX - fMinX;
                pLine->m_LineInfo.fLineY = fPosY - fMinY;
                for (FX_INT32 w = pLine->m_LineInfo.nBeginWordIndex; w <= pLine->m_LineInfo.nEndWordIndex; w++) {
                    if (CPVT_WordInfo * pWord = m_pSection->m_WordArray.GetAt(w)) {
                        pWord->fWordX = fPosX - fMinX;
                        if (pWord->pWordProps) {
                            switch (pWord->pWordProps->nScriptType) {
                                default:
                                case PVTWORD_SCRIPT_NORMAL:
                                    pWord->fWordY = fPosY - fMinY;
                                    break;
                                case PVTWORD_SCRIPT_SUPER:
                                    pWord->fWordY = fPosY - m_pVT->GetWordAscent(*pWord) - fMinY;
                                    break;
                                case PVTWORD_SCRIPT_SUB:
                                    pWord->fWordY = fPosY - m_pVT->GetWordDescent(*pWord) - fMinY;
                                    break;
                            }
                        } else {
                            pWord->fWordY = fPosY - fMinY;
                        }
                        fPosX += m_pVT->GetWordWidth(*pWord);
                    }
                }
                fPosY += (-pLine->m_LineInfo.fLineDescent);
            }
        }
    }
    m_rcRet = CPVT_FloatRect(fMinX, fMinY, fMaxX, fMaxY);
}
CPDF_VariableText::CPDF_VariableText() :
    m_nLimitChar(0),
    m_nCharArray(0),
    m_bMultiLine(FALSE),
    m_bLimitWidth(FALSE),
    m_bAutoFontSize(FALSE),
    m_nAlignment(0),
    m_fLineLeading(0.0f),
    m_fCharSpace(0.0f),
    m_nHorzScale(100),
    m_wSubWord(0),
    m_fFontSize(0.0f),
    m_bInitial(FALSE),
    m_bRichText(FALSE),
    m_pVTProvider(NULL),
    m_pVTIterator(NULL)
{
}
CPDF_VariableText::~CPDF_VariableText()
{
    if (m_pVTIterator) {
        delete m_pVTIterator;
        m_pVTIterator = NULL;
    }
    ResetAll();
}
void CPDF_VariableText::Initialize()
{
    if (!m_bInitial) {
        CPVT_SectionInfo secinfo;
        if (m_bRichText) {
            secinfo.pSecProps = new CPVT_SecProps(0.0f, 0.0f, 0);
            secinfo.pWordProps = new CPVT_WordProps(GetDefaultFontIndex(), PVT_DEFAULT_FONTSIZE, 0, 0, 0);
        }
        CPVT_WordPlace place;
        place.nSecIndex = 0;
        AddSection(place, secinfo);
        CPVT_LineInfo lineinfo;
        lineinfo.fLineAscent = GetFontAscent(GetDefaultFontIndex(), GetFontSize());
        lineinfo.fLineDescent = GetFontDescent(GetDefaultFontIndex(), GetFontSize());
        AddLine(place, lineinfo);
        if (CSection * pSection = m_SectionArray.GetAt(0)) {
            pSection->ResetLinePlace();
        }
        m_bInitial = TRUE;
    }
}
void CPDF_VariableText::ResetAll()
{
    m_bInitial = FALSE;
    ResetSectionArray();
}
CPVT_WordPlace CPDF_VariableText::InsertWord(const CPVT_WordPlace & place, FX_WORD word, FX_INT32 charset,
        const CPVT_WordProps * pWordProps)
{
    FX_INT32 nTotlaWords = this->GetTotalWords();
    if (m_nLimitChar > 0 && nTotlaWords >= m_nLimitChar) {
        return place;
    }
    if (m_nCharArray > 0 && nTotlaWords >= m_nCharArray) {
        return place;
    }
    CPVT_WordPlace newplace = place;
    newplace.nWordIndex ++;
    if (m_bRichText) {
        CPVT_WordProps * pNewProps = pWordProps ? new CPVT_WordProps(*pWordProps) : new CPVT_WordProps();
        pNewProps->nFontIndex = GetWordFontIndex(word, charset, pWordProps->nFontIndex);
        return AddWord(newplace, CPVT_WordInfo(word, charset, -1, pNewProps));
    } else {
        FX_INT32 nFontIndex = GetSubWord() > 0 ? GetDefaultFontIndex() : GetWordFontIndex(word, charset, GetDefaultFontIndex());
        return AddWord(newplace, CPVT_WordInfo(word, charset, nFontIndex, NULL));
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::InsertSection(const CPVT_WordPlace & place, const CPVT_SecProps * pSecProps,
        const CPVT_WordProps * pWordProps)
{
    FX_INT32 nTotlaWords = this->GetTotalWords();
    if (m_nLimitChar > 0 && nTotlaWords >= m_nLimitChar) {
        return place;
    }
    if (m_nCharArray > 0 && nTotlaWords >= m_nCharArray) {
        return place;
    }
    if (!m_bMultiLine) {
        return place;
    }
    CPVT_WordPlace wordplace = place;
    UpdateWordPlace(wordplace);
    CPVT_WordPlace newplace = place;
    if (CSection * pSection = m_SectionArray.GetAt(wordplace.nSecIndex)) {
        CPVT_WordPlace NewPlace(wordplace.nSecIndex + 1, 0, -1);
        CPVT_SectionInfo secinfo;
        if (m_bRichText) {
            if (pSecProps) {
                secinfo.pSecProps = new CPVT_SecProps(*pSecProps);
            }
            if (pWordProps) {
                secinfo.pWordProps = new CPVT_WordProps(*pWordProps);
            }
        }
        AddSection(NewPlace, secinfo);
        newplace = NewPlace;
        if (CSection * pNewSection = m_SectionArray.GetAt(NewPlace.nSecIndex)) {
            for (FX_INT32 w = wordplace.nWordIndex + 1, sz = pSection->m_WordArray.GetSize(); w < sz; w++) {
                if (CPVT_WordInfo * pWord = pSection->m_WordArray.GetAt(w)) {
                    NewPlace.nWordIndex++;
                    pNewSection->AddWord(NewPlace, *pWord);
                }
            }
        }
        ClearSectionRightWords(wordplace);
    }
    return newplace;
}
CPVT_WordPlace CPDF_VariableText::InsertText(const CPVT_WordPlace & place, FX_LPCWSTR text, FX_INT32 charset,
        const CPVT_SecProps * pSecProps, const CPVT_WordProps * pProps)
{
    CFX_WideString swText = text;
    CPVT_WordPlace wp = place;
    for (FX_INT32 i = 0, sz = swText.GetLength(); i < sz; i++) {
        CPVT_WordPlace oldwp = wp;
        FX_WORD word = swText.GetAt(i);
        switch (word) {
            case 0x0D:
                if (m_bMultiLine) {
                    if (swText.GetAt(i + 1) == 0x0A) {
                        i += 1;
                    }
                    wp = InsertSection(wp, pSecProps, pProps);
                }
                break;
            case 0x0A:
                if (m_bMultiLine) {
                    if (swText.GetAt(i + 1) == 0x0D) {
                        i += 1;
                    }
                    wp = InsertSection(wp, pSecProps, pProps);
                }
                break;
            case 0x09:
                word = 0x20;
            default:
                wp = InsertWord(wp, word, charset, pProps);
                break;
        }
        if (wp == oldwp) {
            break;
        }
    }
    return wp;
}
CPVT_WordPlace CPDF_VariableText::DeleteWords(const CPVT_WordRange & PlaceRange)
{
    FX_BOOL bLastSecPos = FALSE;
    if (CSection * pSection = m_SectionArray.GetAt(PlaceRange.EndPos.nSecIndex)) {
        bLastSecPos = (PlaceRange.EndPos == pSection->GetEndWordPlace());
    }
    ClearWords(PlaceRange);
    if (PlaceRange.BeginPos.nSecIndex != PlaceRange.EndPos.nSecIndex) {
        ClearEmptySections(PlaceRange);
        if (!bLastSecPos) {
            LinkLatterSection(PlaceRange.BeginPos);
        }
    }
    return PlaceRange.BeginPos;
}
CPVT_WordPlace CPDF_VariableText::DeleteWord(const CPVT_WordPlace & place)
{
    return ClearRightWord(AjustLineHeader(place, TRUE));
}
CPVT_WordPlace CPDF_VariableText::BackSpaceWord(const CPVT_WordPlace & place)
{
    return ClearLeftWord(AjustLineHeader(place, TRUE));
}
void CPDF_VariableText::SetText(FX_LPCWSTR text, FX_INT32 charset, const CPVT_SecProps * pSecProps,
                                const CPVT_WordProps * pWordProps)
{
    DeleteWords(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
    CFX_WideString swText = text;
    CPVT_WordPlace	wp(0, 0, -1);
    CPVT_SectionInfo secinfo;
    if (m_bRichText) {
        if (pSecProps) {
            secinfo.pSecProps = new CPVT_SecProps(*pSecProps);
        }
        if (pWordProps) {
            secinfo.pWordProps = new CPVT_WordProps(*pWordProps);
        }
    }
    if (CSection * pSection = m_SectionArray.GetAt(0)) {
        pSection->m_SecInfo = secinfo;
    }
    FX_INT32 nCharCount = 0;
    for (FX_INT32 i = 0, sz = swText.GetLength(); i < sz; i++) {
        if (m_nLimitChar > 0 && nCharCount >= m_nLimitChar) {
            break;
        }
        if (m_nCharArray > 0 && nCharCount >= m_nCharArray) {
            break;
        }
        FX_WORD word = swText.GetAt(i);
        switch (word) {
            case 0x0D:
                if (m_bMultiLine) {
                    if (swText.GetAt(i + 1) == 0x0A) {
                        i += 1;
                    }
                    wp.nSecIndex ++;
                    wp.nLineIndex = 0;
                    wp.nWordIndex = -1;
                    AddSection(wp, secinfo);
                }
                break;
            case 0x0A:
                if (m_bMultiLine) {
                    if (swText.GetAt(i + 1) == 0x0D) {
                        i += 1;
                    }
                    wp.nSecIndex ++;
                    wp.nLineIndex = 0;
                    wp.nWordIndex = -1;
                    AddSection(wp, secinfo);
                }
                break;
            case 0x09:
                word = 0x20;
            default:
                wp = InsertWord(wp, word, charset, pWordProps);
                break;
        }
        nCharCount++;
    }
}
void CPDF_VariableText::UpdateWordPlace(CPVT_WordPlace & place) const
{
    if (place.nSecIndex < 0) {
        place = GetBeginWordPlace();
    }
    if (place.nSecIndex >= m_SectionArray.GetSize()) {
        place = GetEndWordPlace();
    }
    place = AjustLineHeader(place, TRUE);
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        pSection->UpdateWordPlace(place);
    }
}
FX_INT32 CPDF_VariableText::WordPlaceToWordIndex(const CPVT_WordPlace & place) const
{
    CPVT_WordPlace newplace = place;
    UpdateWordPlace(newplace);
    FX_INT32 nIndex = 0;
    FX_INT32 i = 0;
    FX_INT32 sz = 0;
    for (i = 0, sz = m_SectionArray.GetSize(); i < sz && i < newplace.nSecIndex; i++) {
        if (CSection * pSection = m_SectionArray.GetAt(i)) {
            nIndex += pSection->m_WordArray.GetSize();
            if (i != m_SectionArray.GetSize() - 1) {
                nIndex += PVT_RETURN_LENGTH;
            }
        }
    }
    if (i >= 0 && i < m_SectionArray.GetSize()) {
        nIndex += newplace.nWordIndex + PVT_RETURN_LENGTH;
    }
    return nIndex;
}
CPVT_WordPlace CPDF_VariableText::WordIndexToWordPlace(FX_INT32 index) const
{
    CPVT_WordPlace place = GetBeginWordPlace();
    FX_INT32 nOldIndex = 0 , nIndex = 0;
    FX_BOOL bFind = FALSE;
    for (FX_INT32 i = 0, sz = m_SectionArray.GetSize(); i < sz; i++) {
        if (CSection * pSection = m_SectionArray.GetAt(i)) {
            nIndex += pSection->m_WordArray.GetSize();
            if (nIndex == index) {
                place = pSection->GetEndWordPlace();
                bFind = TRUE;
                break;
            } else if (nIndex > index) {
                place.nSecIndex = i;
                place.nWordIndex = index - nOldIndex - 1;
                pSection->UpdateWordPlace(place);
                bFind = TRUE;
                break;
            }
            if (i != m_SectionArray.GetSize() - 1) {
                nIndex += PVT_RETURN_LENGTH;
            }
            nOldIndex = nIndex;
        }
    }
    if (!bFind) {
        place = GetEndWordPlace();
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetBeginWordPlace() const
{
    return m_bInitial ? CPVT_WordPlace(0, 0, -1) : CPVT_WordPlace();
}
CPVT_WordPlace CPDF_VariableText::GetEndWordPlace() const
{
    if (CSection * pSection = m_SectionArray.GetAt(m_SectionArray.GetSize() - 1)) {
        return pSection->GetEndWordPlace();
    }
    return CPVT_WordPlace();
}
CPVT_WordPlace CPDF_VariableText::GetPrevWordPlace(const CPVT_WordPlace & place) const
{
    if( place.nSecIndex < 0) {
        return GetBeginWordPlace();
    }
    if (place.nSecIndex >= m_SectionArray.GetSize()) {
        return GetEndWordPlace();
    }
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (place.WordCmp(pSection->GetBeginWordPlace()) <= 0) {
            if (CSection * pPrevSection = m_SectionArray.GetAt(place.nSecIndex - 1)) {
                return pPrevSection->GetEndWordPlace();
            } else {
                return GetBeginWordPlace();
            }
        } else {
            return pSection->GetPrevWordPlace(place);
        }
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetNextWordPlace(const CPVT_WordPlace & place) const
{
    if (place.nSecIndex < 0) {
        return GetBeginWordPlace();
    }
    if (place.nSecIndex >= m_SectionArray.GetSize()) {
        return GetEndWordPlace();
    }
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (place.WordCmp(pSection->GetEndWordPlace()) >= 0) {
            if (CSection * pNextSection = m_SectionArray.GetAt(place.nSecIndex + 1)) {
                return pNextSection->GetBeginWordPlace();
            } else {
                return GetEndWordPlace();
            }
        } else {
            return pSection->GetNextWordPlace(place);
        }
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::SearchWordPlace(const CPDF_Point & point) const
{
    CPDF_Point pt = OutToIn(point);
    CPVT_WordPlace place = GetBeginWordPlace();
    FX_INT32 nLeft = 0;
    FX_INT32 nRight = m_SectionArray.GetSize() - 1;
    FX_INT32 nMid = m_SectionArray.GetSize() / 2;
    FX_BOOL bUp = TRUE;
    FX_BOOL bDown = TRUE;
    while (nLeft <= nRight) {
        if (CSection * pSection = m_SectionArray.GetAt(nMid)) {
            if (IsFloatBigger(pt.y, pSection->m_SecInfo.rcSection.top)) {
                bUp = FALSE;
            }
            if (IsFloatBigger(pSection->m_SecInfo.rcSection.bottom, pt.y)) {
                bDown = FALSE;
            }
            if (IsFloatSmaller(pt.y, pSection->m_SecInfo.rcSection.top)) {
                nRight = nMid - 1;
                nMid = (nLeft + nRight) / 2;
                continue;
            } else if (IsFloatBigger(pt.y, pSection->m_SecInfo.rcSection.bottom)) {
                nLeft = nMid + 1;
                nMid = (nLeft + nRight) / 2;
                continue;
            } else {
                place = pSection->SearchWordPlace(
                            CPDF_Point(pt.x - pSection->m_SecInfo.rcSection.left, pt.y - pSection->m_SecInfo.rcSection.top)
                        );
                place.nSecIndex = nMid;
                return place;
            }
        } else {
            break;
        }
    }
    if (bUp) {
        place = GetBeginWordPlace();
    }
    if (bDown) {
        place = GetEndWordPlace();
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetUpWordPlace(const CPVT_WordPlace & place, const CPDF_Point & point) const
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        CPVT_WordPlace temp =  place;
        CPDF_Point pt = OutToIn(point);
        if (temp.nLineIndex-- > 0) {
            return pSection->SearchWordPlace(pt.x - pSection->m_SecInfo.rcSection.left, temp);
        } else {
            if (temp.nSecIndex-- > 0) {
                if (CSection * pLastSection = m_SectionArray.GetAt(temp.nSecIndex)) {
                    temp.nLineIndex = pLastSection->m_LineArray.GetSize() - 1;
                    return pLastSection->SearchWordPlace(pt.x - pLastSection->m_SecInfo.rcSection.left, temp);
                }
            }
        }
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetDownWordPlace(const CPVT_WordPlace & place, const CPDF_Point & point) const
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        CPVT_WordPlace temp =  place;
        CPDF_Point pt = OutToIn(point);
        if (temp.nLineIndex++ < pSection->m_LineArray.GetSize() - 1) {
            return pSection->SearchWordPlace(pt.x - pSection->m_SecInfo.rcSection.left, temp);
        } else {
            if (temp.nSecIndex++ < m_SectionArray.GetSize() - 1) {
                if (CSection * pNextSection = m_SectionArray.GetAt(temp.nSecIndex)) {
                    temp.nLineIndex = 0;
                    return pNextSection->SearchWordPlace(pt.x - pSection->m_SecInfo.rcSection.left, temp);
                }
            }
        }
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetLineBeginPlace(const CPVT_WordPlace & place) const
{
    return CPVT_WordPlace(place.nSecIndex, place.nLineIndex, -1);
}
CPVT_WordPlace CPDF_VariableText::GetLineEndPlace(const CPVT_WordPlace & place) const
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex))
        if (CLine * pLine = pSection->m_LineArray.GetAt(place.nLineIndex)) {
            return pLine->GetEndWordPlace();
        }
    return place;
}
CPVT_WordPlace CPDF_VariableText::GetSectionBeginPlace(const CPVT_WordPlace & place) const
{
    return CPVT_WordPlace(place.nSecIndex, 0, -1);
}
CPVT_WordPlace CPDF_VariableText::GetSectionEndPlace(const CPVT_WordPlace & place) const
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        return pSection->GetEndWordPlace();
    }
    return place;
}
FX_INT32 CPDF_VariableText::GetTotalWords() const
{
    FX_INT32 nTotal = 0;
    for (FX_INT32 i = 0, sz = m_SectionArray.GetSize(); i < sz; i++)
        if (CSection * pSection = m_SectionArray.GetAt(i)) {
            nTotal += (pSection->m_WordArray.GetSize() + PVT_RETURN_LENGTH);
        }
    return nTotal - PVT_RETURN_LENGTH;
}
void CPDF_VariableText::ResetSectionArray()
{
    for (FX_INT32 s = 0, sz = m_SectionArray.GetSize(); s < sz; s++) {
        delete m_SectionArray.GetAt(s);
    }
    m_SectionArray.RemoveAll();
}
CPVT_WordPlace CPDF_VariableText::AddSection(const CPVT_WordPlace & place, const CPVT_SectionInfo & secinfo)
{
    if (IsValid() && !m_bMultiLine) {
        return place;
    }
    FX_INT32 nSecIndex = FPDF_MAX(FPDF_MIN(place.nSecIndex, m_SectionArray.GetSize()), 0);
    CSection * pSection = new CSection(this);
    pSection->m_SecInfo = secinfo;
    pSection->SecPlace.nSecIndex = nSecIndex;
    if (nSecIndex == m_SectionArray.GetSize()) {
        m_SectionArray.Add(pSection);
    } else {
        m_SectionArray.InsertAt(nSecIndex, pSection);
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::AddLine(const CPVT_WordPlace & place, const CPVT_LineInfo & lineinfo)
{
    if (m_SectionArray.IsEmpty()) {
        return place;
    }
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        return pSection->AddLine(lineinfo);
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::AddWord(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo)
{
    if (m_SectionArray.GetSize() <= 0) {
        return place;
    }
    CPVT_WordPlace newplace = place;
    newplace.nSecIndex = FPDF_MAX(FPDF_MIN(newplace.nSecIndex, m_SectionArray.GetSize() - 1), 0);
    if (CSection * pSection = m_SectionArray.GetAt(newplace.nSecIndex)) {
        return pSection->AddWord(newplace, wordinfo);
    }
    return place;
}
FX_BOOL CPDF_VariableText::GetWordInfo(const CPVT_WordPlace & place, CPVT_WordInfo & wordinfo)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (CPVT_WordInfo * pWord = pSection->m_WordArray.GetAt(place.nWordIndex)) {
            wordinfo = *pWord;
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_VariableText::SetWordInfo(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (CPVT_WordInfo * pWord = pSection->m_WordArray.GetAt(place.nWordIndex)) {
            *pWord = wordinfo;
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_VariableText::GetLineInfo(const CPVT_WordPlace & place, CPVT_LineInfo & lineinfo)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (CLine * pLine = pSection->m_LineArray.GetAt(place.nLineIndex)) {
            lineinfo = pLine->m_LineInfo;
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_VariableText::GetSectionInfo(const CPVT_WordPlace & place, CPVT_SectionInfo & secinfo)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        secinfo = pSection->m_SecInfo;
        return TRUE;
    }
    return FALSE;
}
CPDF_Rect CPDF_VariableText::GetContentRect() const
{
    return InToOut(CPDF_EditContainer::GetContentRect());
}
FX_FLOAT CPDF_VariableText::GetWordFontSize(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize)
{
    return m_bRichText && WordInfo.pWordProps ? (WordInfo.pWordProps->nScriptType == PVTWORD_SCRIPT_NORMAL || bFactFontSize ? WordInfo.pWordProps->fFontSize : WordInfo.pWordProps->fFontSize * PVT_HALF) : GetFontSize();
}
FX_INT32 CPDF_VariableText::GetWordFontIndex(const CPVT_WordInfo & WordInfo)
{
    return m_bRichText && WordInfo.pWordProps ? WordInfo.pWordProps->nFontIndex : WordInfo.nFontIndex;
}
FX_FLOAT CPDF_VariableText::GetWordWidth(FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord,
        FX_FLOAT fCharSpace, FX_INT32 nHorzScale,
        FX_FLOAT fFontSize, FX_FLOAT fWordTail, FX_INT32 nWordStyle)
{
    return (GetCharWidth(nFontIndex, Word, SubWord, nWordStyle) * fFontSize * PVT_FONTSCALE + fCharSpace) * nHorzScale * PVT_PERCENT + fWordTail;
}
FX_FLOAT CPDF_VariableText::GetWordWidth(const CPVT_WordInfo & WordInfo)
{
    return GetWordWidth(GetWordFontIndex(WordInfo), WordInfo.Word, GetSubWord(), GetCharSpace(WordInfo), GetHorzScale(WordInfo),
                        GetWordFontSize(WordInfo), WordInfo.fWordTail,
                        WordInfo.pWordProps ? WordInfo.pWordProps->nWordStyle : 0);
}
FX_FLOAT CPDF_VariableText::GetLineAscent(const CPVT_SectionInfo & SecInfo)
{
    return m_bRichText && SecInfo.pWordProps ? GetFontAscent(SecInfo.pWordProps->nFontIndex, SecInfo.pWordProps->fFontSize) :
           GetFontAscent(GetDefaultFontIndex(), GetFontSize());
}
FX_FLOAT CPDF_VariableText::GetLineDescent(const CPVT_SectionInfo & SecInfo)
{
    return m_bRichText && SecInfo.pWordProps ? GetFontDescent(SecInfo.pWordProps->nFontIndex, SecInfo.pWordProps->fFontSize) :
           GetFontDescent(GetDefaultFontIndex(), GetFontSize());
}
FX_FLOAT CPDF_VariableText::GetFontAscent(FX_INT32 nFontIndex, FX_FLOAT fFontSize)
{
    return (FX_FLOAT)GetTypeAscent(nFontIndex) * fFontSize * PVT_FONTSCALE;
}
FX_FLOAT CPDF_VariableText::GetFontDescent(FX_INT32 nFontIndex, FX_FLOAT fFontSize)
{
    return (FX_FLOAT)GetTypeDescent(nFontIndex) * fFontSize * PVT_FONTSCALE;
}
FX_FLOAT CPDF_VariableText::GetWordAscent(const CPVT_WordInfo & WordInfo, FX_FLOAT fFontSize)
{
    return GetFontAscent(GetWordFontIndex(WordInfo), fFontSize);
}
FX_FLOAT CPDF_VariableText::GetWordDescent(const CPVT_WordInfo & WordInfo, FX_FLOAT fFontSize)
{
    return GetFontDescent(GetWordFontIndex(WordInfo), fFontSize);
}
FX_FLOAT CPDF_VariableText::GetWordAscent(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize)
{
    return GetFontAscent(GetWordFontIndex(WordInfo), GetWordFontSize(WordInfo, bFactFontSize));
}
FX_FLOAT CPDF_VariableText::GetWordDescent(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize)
{
    return GetFontDescent(GetWordFontIndex(WordInfo), GetWordFontSize(WordInfo, bFactFontSize));
}
FX_FLOAT CPDF_VariableText::GetLineLeading(const CPVT_SectionInfo & SecInfo)
{
    return m_bRichText && SecInfo.pSecProps ? SecInfo.pSecProps->fLineLeading : m_fLineLeading;
}
FX_FLOAT CPDF_VariableText::GetLineIndent(const CPVT_SectionInfo & SecInfo)
{
    return m_bRichText && SecInfo.pSecProps ? SecInfo.pSecProps->fLineIndent : 0.0f;
}
FX_INT32 CPDF_VariableText::GetAlignment(const CPVT_SectionInfo& SecInfo)
{
    return m_bRichText && SecInfo.pSecProps ? SecInfo.pSecProps->nAlignment : this->m_nAlignment;
}
FX_FLOAT CPDF_VariableText::GetCharSpace(const CPVT_WordInfo & WordInfo)
{
    return m_bRichText && WordInfo.pWordProps ? WordInfo.pWordProps->fCharSpace : m_fCharSpace;
}
FX_INT32 CPDF_VariableText::GetHorzScale(const CPVT_WordInfo & WordInfo)
{
    return m_bRichText && WordInfo.pWordProps ? WordInfo.pWordProps->nHorzScale : m_nHorzScale;
}
void CPDF_VariableText::ClearSectionRightWords(const CPVT_WordPlace & place)
{
    CPVT_WordPlace wordplace = AjustLineHeader(place, TRUE);
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        for (FX_INT32 w = pSection->m_WordArray.GetSize() - 1; w > wordplace.nWordIndex; w--) {
            delete pSection->m_WordArray.GetAt(w);
            pSection->m_WordArray.RemoveAt(w);
        }
    }
}
CPVT_WordPlace CPDF_VariableText::AjustLineHeader(const CPVT_WordPlace & place, FX_BOOL bPrevOrNext) const
{
    if (place.nWordIndex < 0 && place.nLineIndex > 0) {
        if (bPrevOrNext) {
            return GetPrevWordPlace(place);
        } else {
            return GetNextWordPlace(place);
        }
    }
    return place;
}
FX_BOOL CPDF_VariableText::ClearEmptySection(const CPVT_WordPlace & place)
{
    if (place.nSecIndex == 0 && m_SectionArray.GetSize() == 1) {
        return FALSE;
    }
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        if (pSection->m_WordArray.GetSize() == 0) {
            delete pSection;
            m_SectionArray.RemoveAt(place.nSecIndex);
            return TRUE;
        }
    }
    return FALSE;
}
void CPDF_VariableText::ClearEmptySections(const CPVT_WordRange & PlaceRange)
{
    CPVT_WordPlace wordplace;
    for (FX_INT32 s = PlaceRange.EndPos.nSecIndex; s > PlaceRange.BeginPos.nSecIndex; s--) {
        wordplace.nSecIndex = s;
        ClearEmptySection(wordplace);
    }
}
void CPDF_VariableText::LinkLatterSection(const CPVT_WordPlace & place)
{
    CPVT_WordPlace oldplace = AjustLineHeader(place, TRUE);
    if (CSection * pNextSection = m_SectionArray.GetAt(place.nSecIndex + 1)) {
        if (CSection * pSection = m_SectionArray.GetAt(oldplace.nSecIndex)) {
            for (FX_INT32 w = 0, sz = pNextSection->m_WordArray.GetSize(); w < sz; w++) {
                if (CPVT_WordInfo * pWord = pNextSection->m_WordArray.GetAt(w)) {
                    oldplace.nWordIndex ++;
                    pSection->AddWord(oldplace, *pWord);
                }
            }
        }
        delete pNextSection;
        m_SectionArray.RemoveAt(place.nSecIndex + 1);
    }
}
void CPDF_VariableText::ClearWords(const CPVT_WordRange & PlaceRange)
{
    CPVT_WordRange NewRange;
    NewRange.BeginPos = AjustLineHeader(PlaceRange.BeginPos, TRUE);
    NewRange.EndPos = AjustLineHeader(PlaceRange.EndPos, TRUE);
    for (FX_INT32 s = NewRange.EndPos.nSecIndex; s >= NewRange.BeginPos.nSecIndex; s--) {
        if (CSection * pSection = m_SectionArray.GetAt(s)) {
            pSection->ClearWords(NewRange);
        }
    }
}
CPVT_WordPlace CPDF_VariableText::ClearLeftWord(const CPVT_WordPlace & place)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        CPVT_WordPlace leftplace = this->GetPrevWordPlace(place);
        if (leftplace != place) {
            if (leftplace.nSecIndex != place.nSecIndex) {
                if (pSection->m_WordArray.GetSize() == 0) {
                    this->ClearEmptySection(place);
                } else {
                    this->LinkLatterSection(leftplace);
                }
            } else {
                pSection->ClearWord(place);
            }
        }
        return leftplace;
    }
    return place;
}
CPVT_WordPlace CPDF_VariableText::ClearRightWord(const CPVT_WordPlace & place)
{
    if (CSection * pSection = m_SectionArray.GetAt(place.nSecIndex)) {
        CPVT_WordPlace rightplace = AjustLineHeader(this->GetNextWordPlace(place), FALSE);
        if (rightplace != place) {
            if(rightplace.nSecIndex != place.nSecIndex) {
                LinkLatterSection(place);
            } else {
                pSection->ClearWord(rightplace);
            }
        }
    }
    return place;
}
void CPDF_VariableText::RearrangeAll()
{
    Rearrange(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
}
void CPDF_VariableText::RearrangePart(const CPVT_WordRange & PlaceRange)
{
    Rearrange(PlaceRange);
}
CPVT_FloatRect CPDF_VariableText::Rearrange(const CPVT_WordRange & PlaceRange)
{
    CPVT_FloatRect rcRet;
    if (IsValid()) {
        if (m_bAutoFontSize) {
            SetFontSize(GetAutoFontSize());
            rcRet = RearrangeSections(CPVT_WordRange(GetBeginWordPlace(), GetEndWordPlace()));
        } else {
            rcRet = RearrangeSections(PlaceRange);
        }
    }
    SetContentRect(rcRet);
    return rcRet;
}
FX_FLOAT CPDF_VariableText::GetAutoFontSize()
{
    FX_INT32 nTotal = sizeof(gFontSizeSteps) / sizeof(FX_BYTE);
    if (IsMultiLine()) {
        nTotal /= 4;
    }
    if (nTotal <= 0) {
        return 0;
    }
    if (GetPlateWidth() <= 0) {
        return 0;
    }
    FX_INT32 nLeft = 0;
    FX_INT32 nRight = nTotal - 1;
    FX_INT32 nMid = nTotal / 2;
    while (nLeft <= nRight) {
        if (IsBigger(gFontSizeSteps[nMid])) {
            nRight = nMid - 1;
            nMid = (nLeft + nRight) / 2;
            continue;
        } else {
            nLeft = nMid + 1;
            nMid = (nLeft + nRight) / 2;
            continue;
        }
    }
    return (FX_FLOAT)gFontSizeSteps[nMid];
}
FX_BOOL	CPDF_VariableText::IsBigger(FX_FLOAT fFontSize)
{
    FX_BOOL bBigger =  FALSE;
    CPVT_Size szTotal;
    for (FX_INT32 s = 0, sz = m_SectionArray.GetSize(); s < sz; s++) {
        if (CSection * pSection = m_SectionArray.GetAt(s)) {
            CPVT_Size size = pSection->GetSectionSize(fFontSize);
            szTotal.x = FPDF_MAX(size.x, szTotal.x);
            szTotal.y += size.y;
            if (IsFloatBigger(szTotal.x, GetPlateWidth())
                    || IsFloatBigger(szTotal.y, GetPlateHeight())
               ) {
                bBigger = TRUE;
                break;
            }
        }
    }
    return bBigger;
}
CPVT_FloatRect CPDF_VariableText::RearrangeSections(const CPVT_WordRange & PlaceRange)
{
    CPVT_WordPlace place;
    FX_FLOAT fPosY = 0;
    FX_FLOAT fOldHeight;
    FX_INT32 nSSecIndex = PlaceRange.BeginPos.nSecIndex;
    FX_INT32 nESecIndex = PlaceRange.EndPos.nSecIndex;
    CPVT_FloatRect rcRet;
    for (FX_INT32 s = 0, sz = m_SectionArray.GetSize(); s < sz; s++) {
        place.nSecIndex = s;
        if (CSection * pSection = m_SectionArray.GetAt(s)) {
            pSection->SecPlace = place;
            CPVT_FloatRect rcSec = pSection->m_SecInfo.rcSection;
            if (s >= nSSecIndex) {
                if (s <= nESecIndex) {
                    rcSec = pSection->Rearrange();
                    rcSec.top += fPosY;
                    rcSec.bottom += fPosY;
                } else {
                    fOldHeight = pSection->m_SecInfo.rcSection.bottom - pSection->m_SecInfo.rcSection.top;
                    rcSec.top = fPosY;
                    rcSec.bottom = fPosY + fOldHeight;
                }
                pSection->m_SecInfo.rcSection = rcSec;
                pSection->ResetLinePlace();
            }
            if (s == 0) {
                rcRet = rcSec;
            } else {
                rcRet.left = FPDF_MIN(rcSec.left, rcRet.left);
                rcRet.top = FPDF_MIN(rcSec.top, rcRet.top);
                rcRet.right = FPDF_MAX(rcSec.right, rcRet.right);
                rcRet.bottom = FPDF_MAX(rcSec.bottom, rcRet.bottom);
            }
            fPosY += rcSec.Height();
        }
    }
    return rcRet;
}
FX_INT32 CPDF_VariableText::GetCharWidth(FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord, FX_INT32 nWordStyle)
{
    if (m_pVTProvider) {
        if (SubWord > 0) {
            return m_pVTProvider->GetCharWidth(nFontIndex, SubWord, nWordStyle);
        } else {
            return m_pVTProvider->GetCharWidth(nFontIndex, Word, nWordStyle);
        }
    }
    return 0;
}
FX_INT32 CPDF_VariableText::GetTypeAscent(FX_INT32 nFontIndex)
{
    return m_pVTProvider ? m_pVTProvider->GetTypeAscent(nFontIndex) : 0;
}
FX_INT32 CPDF_VariableText::GetTypeDescent(FX_INT32 nFontIndex)
{
    return m_pVTProvider ? m_pVTProvider->GetTypeDescent(nFontIndex) : 0;
}
FX_INT32 CPDF_VariableText::GetWordFontIndex(FX_WORD word, FX_INT32 charset, FX_INT32 nFontIndex)
{
    return m_pVTProvider ? m_pVTProvider->GetWordFontIndex(word, charset, nFontIndex) : -1;
}
FX_INT32 CPDF_VariableText::GetDefaultFontIndex()
{
    return m_pVTProvider ? m_pVTProvider->GetDefaultFontIndex() : -1;
}
FX_BOOL	CPDF_VariableText::IsLatinWord(FX_WORD word)
{
    return m_pVTProvider ? m_pVTProvider->IsLatinWord(word) : FALSE;
}
IPDF_VariableText_Iterator * CPDF_VariableText::GetIterator()
{
    if (!m_pVTIterator) {
        m_pVTIterator = new CPDF_VariableText_Iterator(this);
    }
    return m_pVTIterator;
}
IPDF_VariableText_Provider*	CPDF_VariableText::SetProvider(IPDF_VariableText_Provider * pProvider)
{
    IPDF_VariableText_Provider* pOld = m_pVTProvider;
    m_pVTProvider = pProvider;
    return pOld;
}
CPDF_VariableText_Iterator::CPDF_VariableText_Iterator(CPDF_VariableText * pVT):
    m_CurPos(-1, -1, -1),
    m_pVT(pVT)
{
}
CPDF_VariableText_Iterator::~CPDF_VariableText_Iterator()
{
}
void CPDF_VariableText_Iterator::SetAt(FX_INT32 nWordIndex)
{
    ASSERT(m_pVT != NULL);
    m_CurPos = m_pVT->WordIndexToWordPlace(nWordIndex);
}
void CPDF_VariableText_Iterator::SetAt(const CPVT_WordPlace & place)
{
    ASSERT(m_pVT != NULL);
    m_CurPos = place;
}
FX_BOOL	CPDF_VariableText_Iterator::NextWord()
{
    ASSERT(m_pVT != NULL);
    if (m_CurPos == m_pVT->GetEndWordPlace()) {
        return FALSE;
    }
    m_CurPos = m_pVT->GetNextWordPlace(m_CurPos);
    return TRUE;
}
FX_BOOL	CPDF_VariableText_Iterator::PrevWord()
{
    ASSERT(m_pVT != NULL);
    if (m_CurPos == m_pVT->GetBeginWordPlace()) {
        return FALSE;
    }
    m_CurPos = m_pVT->GetPrevWordPlace(m_CurPos);
    return TRUE;
}
FX_BOOL	CPDF_VariableText_Iterator::NextLine()
{
    ASSERT(m_pVT != NULL);
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (m_CurPos.nLineIndex < pSection->m_LineArray.GetSize() - 1) {
            m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex + 1, -1);
            return TRUE;
        } else {
            if (m_CurPos.nSecIndex < m_pVT->m_SectionArray.GetSize() - 1) {
                m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex + 1, 0, -1);
                return TRUE;
            }
        }
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::PrevLine()
{
    ASSERT(m_pVT != NULL);
    if (m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (m_CurPos.nLineIndex > 0) {
            m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex - 1, -1);
            return TRUE;
        } else {
            if (m_CurPos.nSecIndex > 0) {
                if (CSection * pLastSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex - 1)) {
                    m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex - 1, pLastSection->m_LineArray.GetSize() - 1, -1);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::NextSection()
{
    ASSERT(m_pVT != NULL);
    if (m_CurPos.nSecIndex < m_pVT->m_SectionArray.GetSize() - 1) {
        m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex + 1, 0, -1);
        return TRUE;
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::PrevSection()
{
    ASSERT(m_pVT != NULL);
    if (m_CurPos.nSecIndex > 0) {
        m_CurPos = CPVT_WordPlace(m_CurPos.nSecIndex - 1, 0, -1);
        return TRUE;
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::GetWord(CPVT_Word & word) const
{
    ASSERT(m_pVT != NULL);
    word.WordPlace = m_CurPos;
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (pSection->m_LineArray.GetAt(m_CurPos.nLineIndex)) {
            if (CPVT_WordInfo * pWord = pSection->m_WordArray.GetAt(m_CurPos.nWordIndex)) {
                word.Word = pWord->Word;
                word.nCharset = pWord->nCharset;
                word.fWidth = m_pVT->GetWordWidth(*pWord);
                word.ptWord = m_pVT->InToOut(
                                  CPDF_Point(pWord->fWordX + pSection->m_SecInfo.rcSection.left,
                                             pWord->fWordY + pSection->m_SecInfo.rcSection.top) );
                word.fAscent = m_pVT->GetWordAscent(*pWord);
                word.fDescent = m_pVT->GetWordDescent(*pWord);
                if (pWord->pWordProps) {
                    word.WordProps = *pWord->pWordProps;
                }
                word.nFontIndex = m_pVT->GetWordFontIndex(*pWord);
                word.fFontSize = m_pVT->GetWordFontSize(*pWord);
                return TRUE;
            }
        }
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::SetWord(const CPVT_Word & word)
{
    ASSERT(m_pVT != NULL);
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (CPVT_WordInfo * pWord = pSection->m_WordArray.GetAt(m_CurPos.nWordIndex)) {
            if (pWord->pWordProps) {
                *pWord->pWordProps = word.WordProps;
            }
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::GetLine(CPVT_Line & line) const
{
    ASSERT(m_pVT != NULL);
    line.lineplace = CPVT_WordPlace(m_CurPos.nSecIndex, m_CurPos.nLineIndex, -1);
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (CLine * pLine = pSection->m_LineArray.GetAt(m_CurPos.nLineIndex)) {
            line.ptLine = m_pVT->InToOut(
                              CPDF_Point(pLine->m_LineInfo.fLineX + pSection->m_SecInfo.rcSection.left,
                                         pLine->m_LineInfo.fLineY + pSection->m_SecInfo.rcSection.top) );
            line.fLineWidth = pLine->m_LineInfo.fLineWidth;
            line.fLineAscent = pLine->m_LineInfo.fLineAscent;
            line.fLineDescent = pLine->m_LineInfo.fLineDescent;
            line.lineEnd = pLine->GetEndWordPlace();
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::GetSection(CPVT_Section & section) const
{
    ASSERT(m_pVT != NULL);
    section.secplace = CPVT_WordPlace(m_CurPos.nSecIndex, 0, -1);
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        section.rcSection = m_pVT->InToOut(pSection->m_SecInfo.rcSection);
        if (pSection->m_SecInfo.pSecProps) {
            section.SecProps = *pSection->m_SecInfo.pSecProps;
        }
        if (pSection->m_SecInfo.pWordProps) {
            section.WordProps = *pSection->m_SecInfo.pWordProps;
        }
        return TRUE;
    }
    return FALSE;
}
FX_BOOL	CPDF_VariableText_Iterator::SetSection(const CPVT_Section & section)
{
    ASSERT(m_pVT != NULL);
    if (CSection * pSection = m_pVT->m_SectionArray.GetAt(m_CurPos.nSecIndex)) {
        if (pSection->m_SecInfo.pSecProps) {
            *pSection->m_SecInfo.pSecProps = section.SecProps;
        }
        if (pSection->m_SecInfo.pWordProps) {
            *pSection->m_SecInfo.pWordProps = section.WordProps;
        }
        return TRUE;
    }
    return FALSE;
}
