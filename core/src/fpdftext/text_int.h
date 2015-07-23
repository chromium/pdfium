// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFTEXT_TEXT_INT_H_
#define CORE_SRC_FPDFTEXT_TEXT_INT_H_

class CPDF_TextPage;
class CPDF_LinkExtract;
class CPDF_TextPageFind;
class CPDF_DocProgressiveSearch;
#define FPDFTEXT_CHAR_ERROR			-1
#define FPDFTEXT_CHAR_NORMAL		0
#define FPDFTEXT_CHAR_GENERATED		1
#define FPDFTEXT_CHAR_UNUNICODE		2
#define FPDFTEXT_CHAR_HYPHEN		3
#define FPDFTEXT_CHAR_PIECE			4
#define FPDFTEXT_MC_PASS			0
#define FPDFTEXT_MC_DONE			1
#define FPDFTEXT_MC_DELAY			2
typedef struct _PAGECHAR_INFO {
    int					m_CharCode;
    FX_WCHAR			m_Unicode;
    FX_FLOAT			m_OriginX;
    FX_FLOAT			m_OriginY;
    int32_t			m_Flag;
    CFX_FloatRect		m_CharBox;
    CPDF_TextObject*	m_pTextObj;
    CFX_AffineMatrix	m_Matrix;
    int					m_Index;
} PAGECHAR_INFO;
typedef	CFX_SegmentedArray<PAGECHAR_INFO> PAGECHAR_InfoArray;
typedef struct {
    int	m_Start;
    int m_nCount;
} FPDF_SEGMENT;
typedef CFX_ArrayTemplate<FPDF_SEGMENT> SEGMENT_Array;
typedef struct {
    CPDF_TextObject*	m_pTextObj;
    CFX_AffineMatrix	m_formMatrix;
} PDFTEXT_Obj;
typedef CFX_ArrayTemplate<PDFTEXT_Obj> LINEOBJ;
class CPDF_TextPage: public IPDF_TextPage
{
public:
    CPDF_TextPage(const CPDF_Page* pPage, int flags = 0);
    CPDF_TextPage(const CPDF_PageObjects* pPage, int flags = 0);
    CPDF_TextPage(const CPDF_Page* pPage, CPDFText_ParseOptions ParserOptions);
    virtual bool					ParseTextPage();
    virtual void					NormalizeObjects(bool bNormalize);
    virtual	bool					IsParsered() const
    {
        return m_IsParsered;
    }
    virtual ~CPDF_TextPage() {};
public:
    virtual int CharIndexFromTextIndex(int TextIndex)const ;
    virtual int TextIndexFromCharIndex(int CharIndex)const;
    virtual int						CountChars() const;
    virtual	void					GetCharInfo(int index, FPDF_CHAR_INFO & info) const;
    virtual void					GetRectArray(int start, int nCount, CFX_RectArray& rectArray) const;
    virtual int						GetIndexAtPos(CPDF_Point point, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const;
    virtual int						GetIndexAtPos(FX_FLOAT x, FX_FLOAT y, FX_FLOAT xTorelance,
            FX_FLOAT yTorelance) const;
    virtual CFX_WideString			GetTextByRect(const CFX_FloatRect& rect) const;
    virtual void					GetRectsArrayByRect(const CFX_FloatRect& rect, CFX_RectArray& resRectArray) const;
    virtual	CFX_WideString			GetPageText(int start = 0, int nCount = -1) const;

    virtual int						CountRects(int start, int nCount);
    virtual	void					GetRect(int rectIndex, FX_FLOAT& left, FX_FLOAT& top
                                            , FX_FLOAT& right, FX_FLOAT &bottom) const;
    virtual bool					GetBaselineRotate(int rectIndex, int& Rotate);
    virtual bool					GetBaselineRotate(const CFX_FloatRect& rect, int& Rotate);
    virtual	int						CountBoundedSegments(FX_FLOAT left, FX_FLOAT top,
            FX_FLOAT right, FX_FLOAT bottom, bool bContains = false);
    virtual	void					GetBoundedSegment(int index, int& start, int& count) const;
    virtual int						GetWordBreak(int index, int direction) const;
public:
    const	PAGECHAR_InfoArray*		GetCharList() const
    {
        return &m_charList;
    }
    static	bool					IsRectIntersect(const CFX_FloatRect& rect1, const CFX_FloatRect& rect2);
    static	bool					IsLetter(FX_WCHAR unicode);
private:
    bool							IsHyphen(FX_WCHAR curChar);
    bool							IsControlChar(const PAGECHAR_INFO& charInfo);
    bool							GetBaselineRotate(int start, int end, int& Rotate);
    void							ProcessObject();
    void							ProcessFormObject(CPDF_FormObject*	pFormObj, const CFX_AffineMatrix& formMatrix);
    void							ProcessTextObject(PDFTEXT_Obj pObj);
    void							ProcessTextObject(CPDF_TextObject*	pTextObj, const CFX_AffineMatrix& formMatrix, FX_POSITION ObjPos);
    int								ProcessInsertObject(const CPDF_TextObject* pObj, const CFX_AffineMatrix& formMatrix);
    bool							GenerateCharInfo(FX_WCHAR unicode, PAGECHAR_INFO& info);
    bool							IsSameAsPreTextObject(CPDF_TextObject* pTextObj, FX_POSITION ObjPos);
    bool							IsSameTextObject(CPDF_TextObject* pTextObj1, CPDF_TextObject* pTextObj2);
    int								GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont) const;
    void							CloseTempLine();
    void							OnPiece(IFX_BidiChar* pBidi, CFX_WideString& str);
    int32_t	PreMarkedContent(PDFTEXT_Obj pObj);
    void		ProcessMarkedContent(PDFTEXT_Obj pObj);
    void		CheckMarkedContentObject(int32_t& start, int32_t& nCount) const;
    void		FindPreviousTextObject(void);
    void		AddCharInfoByLRDirection(CFX_WideString& str, int i);
    void		AddCharInfoByRLDirection(CFX_WideString& str, int i);
    int32_t	GetTextObjectWritingMode(const CPDF_TextObject* pTextObj);
    int32_t	FindTextlineFlowDirection();
    void SwapTempTextBuf(int32_t iCharListStartAppend,
                         int32_t iBufStartAppend);
    bool IsRightToLeft(const CPDF_TextObject* pTextObj,
                          const CPDF_Font* pFont,
                          int nItems) const;
protected:
    CPDFText_ParseOptions			m_ParseOptions;
    CFX_WordArray					m_CharIndex;
    const CPDF_PageObjects*			m_pPage;
    PAGECHAR_InfoArray				m_charList;
    CFX_WideTextBuf					m_TextBuf;
    PAGECHAR_InfoArray				m_TempCharList;
    CFX_WideTextBuf					m_TempTextBuf;
    int								m_parserflag;
    CPDF_TextObject*				m_pPreTextObj;
    CFX_AffineMatrix				m_perMatrix;
    bool							m_IsParsered;
    CFX_AffineMatrix				m_DisplayMatrix;

    SEGMENT_Array					m_Segment;
    CFX_RectArray					m_SelRects;
    LINEOBJ							m_LineObj;
    bool							m_TextlineDir;
    CFX_FloatRect					m_CurlineRect;
};
class CPDF_TextPageFind: public IPDF_TextPageFind
{
public:
    CPDF_TextPageFind(const IPDF_TextPage* pTextPage);
    virtual							~CPDF_TextPageFind() {};
public:
    virtual	bool					FindFirst(const CFX_WideString& findwhat, int flags, int startPos = 0);
    virtual	bool					FindNext();
    virtual	bool					FindPrev();

    virtual void					GetRectArray(CFX_RectArray& rects) const;
    virtual int						GetCurOrder() const;
    virtual int						GetMatchedCount()const;
protected:
    void							ExtractFindWhat(const CFX_WideString& findwhat);
    bool							IsMatchWholeWord(const CFX_WideString& csPageText, int startPos, int endPos);
    bool							ExtractSubString(CFX_WideString& rString, const FX_WCHAR* lpszFullString,
            int iSubString, FX_WCHAR chSep);
    CFX_WideString					MakeReverse(const CFX_WideString& str);
    int								ReverseFind(const CFX_WideString& csPageText, const CFX_WideString& csWord, int nStartPos, int& WordLength);
    int								GetCharIndex(int index) const;
private:
    CFX_WordArray					m_CharIndex;
    const IPDF_TextPage*			m_pTextPage;
    CFX_WideString					m_strText;
    CFX_WideString					m_findWhat;
    int								m_flags;
    CFX_WideStringArray				m_csFindWhatArray;
    int								m_findNextStart;
    int								m_findPreStart;
    bool							m_bMatchCase;
    bool							m_bMatchWholeWord;
    int								m_resStart;
    int								m_resEnd;
    CFX_RectArray					m_resArray;
    bool							m_IsFind;
};
class CPDF_LinkExt
{
public:
    CPDF_LinkExt() {};
    int								m_Start;
    int								m_Count;
    CFX_WideString					m_strUrl;
    virtual							~CPDF_LinkExt() {};
};
typedef CFX_ArrayTemplate<CPDF_LinkExt*> LINK_InfoArray;
class CPDF_LinkExtract: public IPDF_LinkExtract
{
public:
    CPDF_LinkExtract();
    virtual							~CPDF_LinkExtract();
    virtual bool					ExtractLinks(const IPDF_TextPage* pTextPage);
    virtual	bool					IsExtract() const
    {
        return m_IsParserd;
    }
public:
    virtual int						CountLinks() const;
    virtual	CFX_WideString			GetURL(int index) const;
    virtual	void					GetBoundedSegment(int index, int& start, int& count) const;
    virtual	void					GetRects(int index, CFX_RectArray& rects)const;
protected:
    void							parserLink();
    void							DeleteLinkList();
    bool							CheckWebLink(CFX_WideString& strBeCheck);
    bool							CheckMailLink(CFX_WideString& str);
    bool							AppendToLinkList(int start, int count, const CFX_WideString& strUrl);
private:
    LINK_InfoArray					m_LinkList;
    const CPDF_TextPage*			m_pTextPage;
    CFX_WideString					m_strPageText;
    bool							m_IsParserd;
};
FX_STRSIZE FX_Unicode_GetNormalization(FX_WCHAR wch, FX_WCHAR* pDst);
void NormalizeString(CFX_WideString& str);
void NormalizeCompositeChar(FX_WCHAR wChar, CFX_WideString& sDest);

#endif  // CORE_SRC_FPDFTEXT_TEXT_INT_H_
