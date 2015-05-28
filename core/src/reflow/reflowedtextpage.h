// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_REFLOW_REFLOWEDTEXTPAGE_H_
#define CORE_SRC_REFLOW_REFLOWEDTEXTPAGE_H_

#include "../../include/reflow/reflowengine.h"
#include "../../src/reflow/reflowedpage.h"

typedef CFX_SegmentedArray<CRF_CharData*> CRF_CharDataPtrArray;
typedef CFX_SegmentedArray<FX_INT32> CFX_CountBSINT32Array;
class CRF_TextPage : public IPDF_TextPage
{
public:
    CRF_TextPage(IPDF_ReflowedPage* pRefPage);

    virtual ~CRF_TextPage() ;
    FX_BOOL			ParseTextPage();
    void			NormalizeObjects(FX_BOOL bNormalize)
    {
        return;
    };

    FX_BOOL			IsParsered() const;
public:

    int CharIndexFromTextIndex(int TextIndex) const;

    int TextIndexFromCharIndex(int CharIndex) const;


    int				CountChars() const;

    virtual	void	GetCharInfo(int index, FPDF_CHAR_INFO & info) const;

    void			GetRectArray(int start, int nCount, CFX_RectArray& rectArray) const;


    int				GetIndexAtPos(CPDF_Point point, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const;

    int				GetIndexAtPos(FX_FLOAT x, FX_FLOAT y, FX_FLOAT xTorelance, FX_FLOAT yTorelance) const;

    virtual	int				GetOrderByDirection(int index, int direction) const;

    CFX_WideString	GetTextByRect(CFX_FloatRect rect) const;

    void			GetRectsArrayByRect(CFX_FloatRect rect, CFX_RectArray& resRectArray) const;


    int				CountRects(int start, int nCount);

    virtual	void			GetRect(int rectIndex, FX_FLOAT& left, FX_FLOAT& top, FX_FLOAT& right, FX_FLOAT &bottom) const;
    virtual	FX_BOOL			GetBaselineRotate(int rectIndex, int& Rotate);
    virtual FX_BOOL			GetBaselineRotate(CFX_FloatRect rect, int& Rotate);

    virtual	int				CountBoundedSegments(FX_FLOAT left, FX_FLOAT top, FX_FLOAT right, FX_FLOAT bottom, FX_BOOL bContains = FALSE);

    virtual	void			GetBoundedSegment(int index, int& start, int& count) const;


    int				GetWordBreak(int index, int direction) const;

    CFX_WideString	GetPageText(int start, int nCount = -1 ) const;
private:
    CPDF_ReflowedPage*		m_pRefPage;
    CRF_CharDataPtrArray*	m_pDataList;
    CFX_RectArray			m_rectArray;
    CFX_CountBSINT32Array*	m_CountBSArray;
};

#endif  // CORE_SRC_REFLOW_REFLOWEDTEXTPAGE_H_
