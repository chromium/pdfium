// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFDOC_PDF_VT_H_
#define CORE_SRC_FPDFDOC_PDF_VT_H_

class CPVT_Size;
class CPVT_FloatRect;
struct CPVT_SectionInfo;
struct CPVT_LineInfo;
struct CPVT_WordInfo;
class CLine;
class CLines;
class CSection;
class CTypeset;
class CPDF_EditContainer;
class CPDF_VariableText;
class CPDF_VariableText_Iterator;
#define IsFloatZero(f)						((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa,fb)				((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa,fb)				((fa) < (fb) && !IsFloatZero((fa) - (fb)))
template<class T> T FPDF_MIN (const T & i, const T & j)
{
    return ((i < j) ? i : j);
}
template<class T> T FPDF_MAX (const T & i, const T & j)
{
    return ((i > j) ? i : j);
}
class CPVT_Size
{
public:
    CPVT_Size() : x(0.0f), y(0.0f)
    {
    }
    CPVT_Size(FX_FLOAT x, FX_FLOAT y)
    {
        this->x = x;
        this->y = y;
    }
    FX_FLOAT x, y;
};
class CPVT_FloatRect : public CFX_FloatRect
{
public:
    CPVT_FloatRect()
    {
        left = top = right = bottom = 0.0f;
    }
    CPVT_FloatRect(FX_FLOAT left, FX_FLOAT top,
                   FX_FLOAT right, FX_FLOAT bottom)
    {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
    }
    CPVT_FloatRect(const CPDF_Rect & rect)
    {
        this->left = rect.left;
        this->top = rect.top;
        this->right = rect.right;
        this->bottom = rect.bottom;
    }
    void Default()
    {
        left = top = right = bottom = 0.0f;
    }
    FX_FLOAT Height() const
    {
        if(this->top > this->bottom) {
            return this->top - this->bottom;
        } else {
            return this->bottom - this->top;
        }
    }
};
struct CPVT_SectionInfo {
    CPVT_SectionInfo() : rcSection(), nTotalLine(0), pSecProps(NULL), pWordProps(NULL)
    {
    }
    virtual ~CPVT_SectionInfo()
    {
        if (pSecProps) {
            delete pSecProps;
        }
        if (pWordProps) {
            delete pWordProps;
        }
    }
    CPVT_SectionInfo(const CPVT_SectionInfo & other): rcSection(), nTotalLine(0), pSecProps(NULL), pWordProps(NULL)
    {
        operator = (other);
    }
    void operator = (const CPVT_SectionInfo & other)
    {
        if (this == &other) {
            return;
        }
        this->rcSection = other.rcSection;
        this->nTotalLine = other.nTotalLine;
        if (other.pSecProps) {
            if (pSecProps) {
                *pSecProps = *other.pSecProps;
            } else {
                pSecProps = new CPVT_SecProps(*other.pSecProps);
            }
        }
        if (other.pWordProps) {
            if (pWordProps) {
                *pWordProps = *other.pWordProps;
            } else {
                pWordProps = new CPVT_WordProps(*other.pWordProps);
            }
        }
    }
    CPVT_FloatRect				rcSection;
    FX_INT32					nTotalLine;
    CPVT_SecProps*				pSecProps;
    CPVT_WordProps*				pWordProps;
};
struct CPVT_LineInfo {
    CPVT_LineInfo() : nTotalWord(0), nBeginWordIndex(-1), nEndWordIndex(-1),
        fLineX(0.0f), fLineY(0.0f), fLineWidth(0.0f), fLineAscent(0.0f), fLineDescent(0.0f)
    {
    }
    FX_INT32					nTotalWord;
    FX_INT32					nBeginWordIndex;
    FX_INT32					nEndWordIndex;
    FX_FLOAT					fLineX;
    FX_FLOAT					fLineY;
    FX_FLOAT					fLineWidth;
    FX_FLOAT					fLineAscent;
    FX_FLOAT					fLineDescent;
};
struct CPVT_WordInfo  {
    CPVT_WordInfo() : Word(0), nCharset(0),
        fWordX(0.0f), fWordY(0.0f), fWordTail(0.0f), nFontIndex(-1), pWordProps(NULL)
    {
    }
    CPVT_WordInfo(FX_WORD word, FX_INT32 charset, FX_INT32 fontIndex, CPVT_WordProps * pProps):
        Word(word), nCharset(charset), fWordX(0.0f), fWordY(0.0f), fWordTail(0.0f),
        nFontIndex(fontIndex), pWordProps(pProps)
    {
    }
    virtual ~CPVT_WordInfo()
    {
        if (pWordProps) {
            delete pWordProps;
        }
    }
    CPVT_WordInfo(const CPVT_WordInfo & word): Word(0), nCharset(0),
        fWordX(0.0f), fWordY(0.0f), fWordTail(0.0f), nFontIndex(-1), pWordProps(NULL)
    {
        operator = (word);
    }
    void operator = (const CPVT_WordInfo & word)
    {
        if (this == &word) {
            return;
        }
        this->Word = word.Word;
        this->nCharset = word.nCharset;
        this->nFontIndex = word.nFontIndex;
        if (word.pWordProps) {
            if (pWordProps) {
                *pWordProps = *word.pWordProps;
            } else {
                pWordProps = new CPVT_WordProps(*word.pWordProps);
            }
        }
    }
    FX_WORD						Word;
    FX_INT32					nCharset;
    FX_FLOAT					fWordX;
    FX_FLOAT					fWordY;
    FX_FLOAT					fWordTail;
    FX_INT32					nFontIndex;
    CPVT_WordProps*				pWordProps;
};
struct CPVT_FloatRange {
    CPVT_FloatRange() : fMin(0.0f), fMax(0.0f)
    {
    }
    CPVT_FloatRange(FX_FLOAT min, FX_FLOAT max) : fMin(min), fMax(max)
    {
    }
    FX_FLOAT Range() const
    {
        return fMax - fMin;
    }
    FX_FLOAT fMin, fMax;
};
template<class TYPE> class CPVT_ArrayTemplate : public CFX_ArrayTemplate<TYPE>
{
public:
    FX_BOOL IsEmpty()
    {
        return CFX_ArrayTemplate<TYPE>::GetSize() <= 0;
    }
    TYPE GetAt(int nIndex) const
    {
        if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) {
            return CFX_ArrayTemplate<TYPE>::GetAt(nIndex);
        }
        return NULL;
    }
    void RemoveAt(int nIndex)
    {
        if (nIndex >= 0 && nIndex < CFX_ArrayTemplate<TYPE>::GetSize()) {
            CFX_ArrayTemplate<TYPE>::RemoveAt(nIndex);
        }
    }
};
class CLine 
{
public:
    CLine();
    virtual ~CLine();
    CPVT_WordPlace							GetBeginWordPlace() const;
    CPVT_WordPlace							GetEndWordPlace() const;
    CPVT_WordPlace							GetPrevWordPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetNextWordPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							LinePlace;
    CPVT_LineInfo							m_LineInfo;
};
class CLines
{
public:
    CLines() : m_nTotal(0) {}
    virtual ~CLines()
    {
        RemoveAll();
    }
    FX_INT32								GetSize() const
    {
        return m_Lines.GetSize();
    }
    CLine *									GetAt(FX_INT32 nIndex) const
    {
        return m_Lines.GetAt(nIndex);
    }
    void									Empty()
    {
        m_nTotal = 0;
    }
    void									RemoveAll()
    {
        for (FX_INT32 i = 0, sz = GetSize(); i < sz; i++) {
            delete GetAt(i);
        }
        m_Lines.RemoveAll();
        m_nTotal = 0;
    }
    FX_INT32								Add(const CPVT_LineInfo & lineinfo)
    {
        if (m_nTotal >= GetSize()) {
            CLine* pLine = new CLine;
            pLine->m_LineInfo = lineinfo;
            m_Lines.Add(pLine);
        } else if (CLine* pLine = GetAt(m_nTotal)) {
            pLine->m_LineInfo = lineinfo;
        }
        return m_nTotal++;
    }
    void									Clear()
    {
        for (FX_INT32 i = GetSize() - 1; i >= m_nTotal; i--) {
            delete GetAt(i);
            m_Lines.RemoveAt(i);
        }
    }
private:
    CPVT_ArrayTemplate<CLine*>				m_Lines;
    FX_INT32								m_nTotal;
};
class CSection 
{
    friend class CTypeset;
public:
    CSection(CPDF_VariableText * pVT);
    virtual ~CSection();
    void									ResetAll();
    void									ResetLineArray();
    void									ResetWordArray();
    void									ResetLinePlace();
    CPVT_WordPlace							AddWord(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo);
    CPVT_WordPlace							AddLine(const CPVT_LineInfo & lineinfo);
    void									ClearWords(const CPVT_WordRange & PlaceRange);
    void									ClearWord(const CPVT_WordPlace & place);
    CPVT_FloatRect							Rearrange();
    CPVT_Size								GetSectionSize(FX_FLOAT fFontSize);
    CPVT_WordPlace							GetBeginWordPlace() const;
    CPVT_WordPlace							GetEndWordPlace() const;
    CPVT_WordPlace							GetPrevWordPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetNextWordPlace(const CPVT_WordPlace & place) const;
    void									UpdateWordPlace(CPVT_WordPlace & place) const;
    CPVT_WordPlace							SearchWordPlace(const CPDF_Point & point) const;
    CPVT_WordPlace							SearchWordPlace(FX_FLOAT fx, const CPVT_WordPlace & lineplace) const;
    CPVT_WordPlace							SearchWordPlace(FX_FLOAT fx, const CPVT_WordRange & range) const;
public:
    CPVT_WordPlace							SecPlace;
    CPVT_SectionInfo						m_SecInfo;
    CLines									m_LineArray;
    CPVT_ArrayTemplate<CPVT_WordInfo*>		m_WordArray;
private:
    void									ClearLeftWords(FX_INT32 nWordIndex);
    void									ClearRightWords(FX_INT32 nWordIndex);
    void									ClearMidWords(FX_INT32 nBeginIndex, FX_INT32 nEndIndex);

    CPDF_VariableText						*m_pVT;
};
class CTypeset
{
public:
    CTypeset(CSection * pSection);
    virtual ~CTypeset();
    CPVT_Size								GetEditSize(FX_FLOAT fFontSize);
    CPVT_FloatRect							Typeset();
    CPVT_FloatRect							CharArray();
private:
    void									SplitLines(FX_BOOL bTypeset, FX_FLOAT fFontSize);
    void									OutputLines();

    CPVT_FloatRect							m_rcRet;
    CPDF_VariableText						* m_pVT;
    CSection								* m_pSection;
};
class CPDF_EditContainer
{
public:
    CPDF_EditContainer(): m_rcPlate(0, 0, 0, 0), m_rcContent(0, 0, 0, 0) {};
    virtual ~CPDF_EditContainer() {};
    virtual void							SetPlateRect(const CPDF_Rect & rect)
    {
        m_rcPlate = rect;
    };
    virtual const CPDF_Rect &				GetPlateRect() const
    {
        return m_rcPlate;
    };
    virtual void							SetContentRect(const CPVT_FloatRect & rect)
    {
        m_rcContent = rect;
    };
    virtual CPDF_Rect 						GetContentRect() const
    {
        return m_rcContent;
    };
    FX_FLOAT								GetPlateWidth() const
    {
        return m_rcPlate.right - m_rcPlate.left;
    };
    FX_FLOAT								GetPlateHeight() const
    {
        return m_rcPlate.top - m_rcPlate.bottom;
    };
    CPVT_Size								GetPlateSize() const
    {
        return CPVT_Size(GetPlateWidth(), GetPlateHeight());
    };
    CPDF_Point								GetBTPoint() const
    {
        return CPDF_Point(m_rcPlate.left, m_rcPlate.top);
    };
    CPDF_Point								GetETPoint() const
    {
        return CPDF_Point(m_rcPlate.right, m_rcPlate.bottom);
    };
    inline CPDF_Point						InToOut(const CPDF_Point & point) const
    {
        return CPDF_Point(point.x + GetBTPoint().x, GetBTPoint().y - point.y);
    };
    inline CPDF_Point						OutToIn(const CPDF_Point & point) const
    {
        return CPDF_Point(point.x - GetBTPoint().x, GetBTPoint().y - point.y);
    };
    inline CPDF_Rect						InToOut(const CPVT_FloatRect & rect) const
    {
        CPDF_Point ptLeftTop = InToOut(CPDF_Point(rect.left, rect.top));
        CPDF_Point ptRightBottom = InToOut(CPDF_Point(rect.right, rect.bottom));
        return CPDF_Rect(ptLeftTop.x, ptRightBottom.y, ptRightBottom.x, ptLeftTop.y);
    };
    inline CPVT_FloatRect					OutToIn(const CPDF_Rect & rect) const
    {
        CPDF_Point ptLeftTop = OutToIn(CPDF_Point(rect.left, rect.top));
        CPDF_Point ptRightBottom = OutToIn(CPDF_Point(rect.right, rect.bottom));
        return CPVT_FloatRect(ptLeftTop.x, ptLeftTop.y, ptRightBottom.x, ptRightBottom.y);
    };

private:
    CPDF_Rect								m_rcPlate;
    CPVT_FloatRect							m_rcContent;
};
class CPDF_VariableText : public IPDF_VariableText, private CPDF_EditContainer
{
    friend class CTypeset;
    friend class CSection;
    friend class CPDF_VariableText_Iterator;
public:
    CPDF_VariableText();
    virtual ~CPDF_VariableText();
    IPDF_VariableText_Provider*				SetProvider(IPDF_VariableText_Provider * pProvider);
    IPDF_VariableText_Iterator*				GetIterator();
    void									SetPlateRect(const CPDF_Rect & rect)
    {
        CPDF_EditContainer::SetPlateRect(rect);
    }
    void									SetAlignment(FX_INT32 nFormat = 0)
    {
        m_nAlignment = nFormat;
    }
    void									SetPasswordChar(FX_WORD wSubWord = '*')
    {
        m_wSubWord = wSubWord;
    }
    void									SetLimitChar(FX_INT32 nLimitChar = 0)
    {
        m_nLimitChar = nLimitChar;
    }
    void									SetCharSpace(FX_FLOAT fCharSpace = 0.0f)
    {
        m_fCharSpace = fCharSpace;
    }
    void									SetHorzScale(FX_INT32 nHorzScale = 100)
    {
        m_nHorzScale = nHorzScale;
    }
    void									SetMultiLine(FX_BOOL bMultiLine = TRUE)
    {
        m_bMultiLine = bMultiLine;
    }
    void									SetAutoReturn(FX_BOOL bAuto = TRUE)
    {
        m_bLimitWidth = bAuto;
    }
    void									SetFontSize(FX_FLOAT fFontSize)
    {
        m_fFontSize = fFontSize;
    }
    void									SetCharArray(FX_INT32 nCharArray = 0)
    {
        m_nCharArray = nCharArray;
    }
    void									SetAutoFontSize(FX_BOOL bAuto = TRUE)
    {
        m_bAutoFontSize = bAuto;
    }
    void									SetRichText(FX_BOOL bRichText)
    {
        m_bRichText = bRichText;
    }
    void									SetLineLeading(FX_FLOAT fLineLeading)
    {
        m_fLineLeading = fLineLeading;
    }
    void									Initialize();
    FX_BOOL									IsValid() const
    {
        return m_bInitial;
    }
    FX_BOOL									IsRichText() const
    {
        return m_bRichText;
    }
    void									RearrangeAll();
    void									RearrangePart(const CPVT_WordRange & PlaceRange);
    void									ResetAll();
    void									SetText(FX_LPCWSTR text, FX_INT32 charset = 1, const CPVT_SecProps * pSecProps = NULL,
            const CPVT_WordProps * pWordProps = NULL);
    CPVT_WordPlace							InsertWord(const CPVT_WordPlace & place, FX_WORD word, FX_INT32 charset = 1,
            const CPVT_WordProps * pWordProps = NULL);
    CPVT_WordPlace							InsertSection(const CPVT_WordPlace & place, const CPVT_SecProps * pSecProps = NULL,
            const CPVT_WordProps * pWordProps = NULL);
    CPVT_WordPlace							InsertText(const CPVT_WordPlace & place, FX_LPCWSTR text, FX_INT32 charset = 1,
            const CPVT_SecProps * pSecProps = NULL,	const CPVT_WordProps * pWordProps = NULL);
    CPVT_WordPlace							DeleteWords(const CPVT_WordRange & PlaceRange);
    CPVT_WordPlace							DeleteWord(const CPVT_WordPlace & place);
    CPVT_WordPlace							BackSpaceWord(const CPVT_WordPlace & place);
    const CPDF_Rect &						GetPlateRect() const
    {
        return CPDF_EditContainer::GetPlateRect();
    }
    CPDF_Rect								GetContentRect() const;
    FX_INT32								GetTotalWords() const;
    FX_FLOAT								GetFontSize() const
    {
        return m_fFontSize;
    }
    FX_INT32								GetAlignment() const
    {
        return m_nAlignment;
    }
    FX_INT32								GetCharArray() const
    {
        return m_nCharArray;
    }
    FX_INT32								GetLimitChar() const
    {
        return m_nLimitChar;
    }
    FX_BOOL									IsMultiLine() const
    {
        return m_bMultiLine;
    }
    FX_INT32								GetHorzScale() const
    {
        return m_nHorzScale;
    }
    FX_FLOAT								GetCharSpace() const
    {
        return m_fCharSpace;
    }

    CPVT_WordPlace							GetBeginWordPlace() const;
    CPVT_WordPlace							GetEndWordPlace() const;
    CPVT_WordPlace							GetPrevWordPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetNextWordPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							SearchWordPlace(const CPDF_Point & point) const;
    CPVT_WordPlace							GetUpWordPlace(const CPVT_WordPlace & place, const CPDF_Point & point) const;
    CPVT_WordPlace							GetDownWordPlace(const CPVT_WordPlace & place, const CPDF_Point & point) const;
    CPVT_WordPlace							GetLineBeginPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetLineEndPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetSectionBeginPlace(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							GetSectionEndPlace(const CPVT_WordPlace & place) const;
    void									UpdateWordPlace(CPVT_WordPlace & place) const;
    FX_INT32								WordPlaceToWordIndex(const CPVT_WordPlace & place) const;
    CPVT_WordPlace							WordIndexToWordPlace(FX_INT32 index) const;
    FX_WORD									GetPasswordChar() const
    {
        return GetSubWord();
    }
    FX_WORD									GetSubWord() const
    {
        return m_wSubWord;
    }
private:
    FX_INT32								GetCharWidth(FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord, FX_INT32 nWordStyle);
    FX_INT32								GetTypeAscent(FX_INT32 nFontIndex);
    FX_INT32								GetTypeDescent(FX_INT32 nFontIndex);
    FX_INT32								GetWordFontIndex(FX_WORD word, FX_INT32 charset, FX_INT32 nFontIndex);
    FX_INT32								GetDefaultFontIndex();
    FX_BOOL									IsLatinWord(FX_WORD word);
private:

    CPVT_WordPlace							AddSection(const CPVT_WordPlace & place, const CPVT_SectionInfo & secinfo);
    CPVT_WordPlace							AddLine(const CPVT_WordPlace & place, const CPVT_LineInfo & lineinfo);
    CPVT_WordPlace							AddWord(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo);
    FX_BOOL									GetWordInfo(const CPVT_WordPlace & place, CPVT_WordInfo & wordinfo);
    FX_BOOL									SetWordInfo(const CPVT_WordPlace & place, const CPVT_WordInfo & wordinfo);
    FX_BOOL									GetLineInfo(const CPVT_WordPlace & place, CPVT_LineInfo & lineinfo);
    FX_BOOL									GetSectionInfo(const CPVT_WordPlace & place, CPVT_SectionInfo & secinfo);
    FX_FLOAT								GetWordFontSize(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize = FALSE);
    FX_FLOAT								GetWordWidth(FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord,
            FX_FLOAT fCharSpace, FX_INT32 nHorzScale,
            FX_FLOAT fFontSize, FX_FLOAT fWordTail, FX_INT32 nWordStyle);
    FX_FLOAT								GetWordWidth(const CPVT_WordInfo & WordInfo);
    FX_FLOAT								GetWordAscent(const CPVT_WordInfo & WordInfo, FX_FLOAT fFontSize);
    FX_FLOAT								GetWordDescent(const CPVT_WordInfo & WordInfo, FX_FLOAT fFontSize);
    FX_FLOAT								GetWordAscent(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize = FALSE);
    FX_FLOAT								GetWordDescent(const CPVT_WordInfo & WordInfo, FX_BOOL bFactFontSize = FALSE);
    FX_FLOAT								GetLineAscent(const CPVT_SectionInfo & SecInfo);
    FX_FLOAT								GetLineDescent(const CPVT_SectionInfo & SecInfo);
    FX_FLOAT								GetFontAscent(FX_INT32 nFontIndex, FX_FLOAT fFontSize);
    FX_FLOAT								GetFontDescent(FX_INT32 nFontIndex, FX_FLOAT fFontSize);
    FX_INT32								GetWordFontIndex(const CPVT_WordInfo & WordInfo);
    FX_FLOAT								GetCharSpace(const CPVT_WordInfo & WordInfo);
    FX_INT32								GetHorzScale(const CPVT_WordInfo & WordInfo);
    FX_FLOAT								GetLineLeading(const CPVT_SectionInfo & SecInfo);
    FX_FLOAT								GetLineIndent(const CPVT_SectionInfo & SecInfo);
    FX_INT32								GetAlignment(const CPVT_SectionInfo& SecInfo);

    void									ClearSectionRightWords(const CPVT_WordPlace & place);
    CPVT_WordPlace							AjustLineHeader(const CPVT_WordPlace & place, FX_BOOL bPrevOrNext) const;
    FX_BOOL									ClearEmptySection(const CPVT_WordPlace & place);
    void									ClearEmptySections(const CPVT_WordRange & PlaceRange);
    void									LinkLatterSection(const CPVT_WordPlace & place);
    void									ClearWords(const CPVT_WordRange & PlaceRange);
    CPVT_WordPlace							ClearLeftWord(const CPVT_WordPlace & place);
    CPVT_WordPlace							ClearRightWord(const CPVT_WordPlace & place);
private:
    CPVT_FloatRect							Rearrange(const CPVT_WordRange & PlaceRange);
    FX_FLOAT								GetAutoFontSize();
    FX_BOOL									IsBigger(FX_FLOAT fFontSize);
    CPVT_FloatRect							RearrangeSections(const CPVT_WordRange & PlaceRange);
private:
    void									ResetSectionArray();
private:
    CPVT_ArrayTemplate<CSection*>			m_SectionArray;
    FX_INT32								m_nLimitChar;
    FX_INT32								m_nCharArray;
    FX_BOOL									m_bMultiLine;
    FX_BOOL									m_bLimitWidth;
    FX_BOOL									m_bAutoFontSize;
    FX_INT32								m_nAlignment;
    FX_FLOAT								m_fLineLeading;
    FX_FLOAT								m_fCharSpace;
    FX_INT32								m_nHorzScale;
    FX_WORD									m_wSubWord;
    FX_FLOAT								m_fFontSize;

private:
    FX_BOOL									m_bInitial;
    FX_BOOL									m_bRichText;
    IPDF_VariableText_Provider *			m_pVTProvider;
    CPDF_VariableText_Iterator *			m_pVTIterator;
};
class CPDF_VariableText_Iterator : public IPDF_VariableText_Iterator
{
public:
    CPDF_VariableText_Iterator(CPDF_VariableText * pVT);
    virtual ~CPDF_VariableText_Iterator();
    FX_BOOL									NextWord();
    FX_BOOL									PrevWord();
    FX_BOOL									NextLine();
    FX_BOOL									PrevLine();
    FX_BOOL									NextSection();
    FX_BOOL									PrevSection();
    FX_BOOL									SetWord(const CPVT_Word & word);
    FX_BOOL									GetWord(CPVT_Word & word) const;
    FX_BOOL									GetLine(CPVT_Line & line) const;
    FX_BOOL									GetSection(CPVT_Section & section) const;
    FX_BOOL									SetSection(const CPVT_Section & section);
    void									SetAt(FX_INT32 nWordIndex);
    void									SetAt(const CPVT_WordPlace & place);
    const CPVT_WordPlace &					GetAt() const
    {
        return m_CurPos;
    };
private:
    CPVT_WordPlace							m_CurPos;
    CPDF_VariableText *						m_pVT;
};

#endif  // CORE_SRC_FPDFDOC_PDF_VT_H_
