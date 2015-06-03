// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFTEXT_FPDF_TEXT_H_
#define CORE_INCLUDE_FPDFTEXT_FPDF_TEXT_H_

#include "../fpdfapi/fpdf_page.h"
#include "../fpdfapi/fpdf_pageobj.h"
#include "../fpdfapi/fpdf_parser.h"

class CPDF_PageObjects;
class IPDF_LinkExtract;
class IPDF_ReflowedPage;
class IPDF_TextPage;
class IPDF_TextPageFind;

#define PDF2TXT_AUTO_ROTATE		1
#define PDF2TXT_AUTO_WIDTH		2
#define PDF2TXT_KEEP_COLUMN		4
#define PDF2TXT_USE_OCR			8
#define PDF2TXT_INCLUDE_INVISIBLE	16
void PDF_GetPageText(CFX_ByteStringArray& lines, CPDF_Document* pDoc, CPDF_Dictionary* pPage,
                     int iMinWidth, FX_DWORD flags);
void PDF_GetPageText_Unicode(CFX_WideStringArray& lines, CPDF_Document* pDoc, CPDF_Dictionary* pPage,
                             int iMinWidth, FX_DWORD flags);
void PDF_GetTextStream_Unicode(CFX_WideTextBuf& buffer, CPDF_Document* pDoc, CPDF_Dictionary* pPage,
                               FX_DWORD flags);
CFX_WideString PDF_GetFirstTextLine_Unicode(CPDF_Document* pDoc, CPDF_Dictionary* pPage);
#define CHAR_ERROR			-1
#define CHAR_NORMAL			0
#define CHAR_GENERATED		1
#define CHAR_UNUNICODE		2
typedef struct {
    FX_WCHAR			m_Unicode;
    FX_WCHAR			m_Charcode;
    FX_INT32			m_Flag;
    FX_FLOAT			m_FontSize;
    FX_FLOAT			m_OriginX;
    FX_FLOAT			m_OriginY;
    CFX_FloatRect		m_CharBox;
    CPDF_TextObject*	m_pTextObj;
    CFX_AffineMatrix	m_Matrix;
} FPDF_CHAR_INFO;
typedef	CFX_ArrayTemplate<CFX_FloatRect> CFX_RectArray;
#define FPDFTEXT_LRTB	0
#define FPDFTEXT_RLTB	1
#define FPDFTEXT_TBRL	2
#define FPDFTEXT_LEFT			-1
#define FPDFTEXT_RIGHT			1
#define FPDFTEXT_UP				-2
#define FPDFTEXT_DOWN			2
#define FPDFTEXT_WRITINGMODE_UNKNOW	0
#define FPDFTEXT_WRITINGMODE_LRTB	1
#define FPDFTEXT_WRITINGMODE_RLTB	2
#define FPDFTEXT_WRITINGMODE_TBRL	3
class CPDFText_ParseOptions 
{
public:

    CPDFText_ParseOptions();
    FX_BOOL			m_bGetCharCodeOnly;
    FX_BOOL			m_bNormalizeObjs;
    FX_BOOL			m_bOutputHyphen;
};
class IPDF_TextPage 
{
public:

    virtual ~IPDF_TextPage() {}
    static IPDF_TextPage*	CreateTextPage(const CPDF_Page* pPage, CPDFText_ParseOptions ParserOptions);
    static IPDF_TextPage*	CreateTextPage(const CPDF_Page* pPage, int flags = 0);
    static IPDF_TextPage*	CreateTextPage(const CPDF_PageObjects* pObjs, int flags = 0);
    static IPDF_TextPage*	CreateReflowTextPage(IPDF_ReflowedPage* pRefPage);

    virtual void			NormalizeObjects(FX_BOOL bNormalize) = 0;

    virtual FX_BOOL			ParseTextPage() = 0;


    virtual FX_BOOL			IsParsered() const = 0;
public:

    virtual int CharIndexFromTextIndex(int TextIndex) const = 0;

    virtual int TextIndexFromCharIndex(int CharIndex) const = 0;


    virtual int				CountChars() const = 0;

    virtual	void			GetCharInfo(int index, FPDF_CHAR_INFO & info) const = 0;

    virtual void			GetRectArray(int start, int nCount, CFX_RectArray& rectArray) const = 0;



    virtual int				GetIndexAtPos(CPDF_Point point, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const = 0;

    virtual int				GetIndexAtPos(FX_FLOAT x, FX_FLOAT y, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const = 0;

    virtual	int				GetOrderByDirection(int index, int direction) const = 0;

    virtual CFX_WideString	GetTextByRect(const CFX_FloatRect& rect) const = 0;

    virtual void			GetRectsArrayByRect(const CFX_FloatRect& rect, CFX_RectArray& resRectArray) const = 0;


    virtual int				CountRects(int start, int nCount) = 0;

    virtual	void			GetRect(int rectIndex, FX_FLOAT& left, FX_FLOAT& top, FX_FLOAT& right, FX_FLOAT &bottom) const = 0;

    virtual FX_BOOL			GetBaselineRotate(int rectIndex, int& Rotate) = 0;

    virtual FX_BOOL			GetBaselineRotate(const CFX_FloatRect& rect, int& Rotate) = 0;

    virtual	int				CountBoundedSegments(FX_FLOAT left, FX_FLOAT top, FX_FLOAT right, FX_FLOAT bottom, FX_BOOL bContains = FALSE) = 0;

    virtual	void			GetBoundedSegment(int index, int& start, int& count) const = 0;


    virtual int				GetWordBreak(int index, int direction) const = 0;

    virtual CFX_WideString	GetPageText(int start = 0, int nCount = -1 ) const = 0;
};
#define FPDFTEXT_MATCHCASE      0x00000001
#define FPDFTEXT_MATCHWHOLEWORD 0x00000002
#define FPDFTEXT_CONSECUTIVE	0x00000004
class IPDF_TextPageFind 
{
public:

    virtual	~IPDF_TextPageFind() {}

    static	IPDF_TextPageFind*	CreatePageFind(const IPDF_TextPage* pTextPage);
public:

    virtual	FX_BOOL				FindFirst(const CFX_WideString& findwhat, int flags, int startPos = 0) = 0;

    virtual	FX_BOOL				FindNext() = 0;

    virtual	FX_BOOL				FindPrev() = 0;

    virtual void				GetRectArray(CFX_RectArray& rects) const = 0;

    virtual int					GetCurOrder() const = 0;

    virtual int					GetMatchedCount() const = 0;
};
class IPDF_LinkExtract 
{
public:

    virtual	~IPDF_LinkExtract() {}

    static	IPDF_LinkExtract*	CreateLinkExtract();

    virtual FX_BOOL				ExtractLinks(const IPDF_TextPage* pTextPage) = 0;
public:

    virtual int					CountLinks() const = 0;

    virtual CFX_WideString		GetURL(int index) const = 0;

    virtual	void				GetBoundedSegment(int index, int& start, int& count) const = 0;

    virtual void				GetRects(int index, CFX_RectArray& rects) const = 0;
};

#endif  // CORE_INCLUDE_FPDFTEXT_FPDF_TEXT_H_
