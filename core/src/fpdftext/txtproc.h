// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFTEXT_TXTPROC_H_
#define CORE_SRC_FPDFTEXT_TXTPROC_H_

class CTextColumn 
{
public:
    FX_FLOAT	m_AvgPos;
    int		m_Count;
    int		m_TextPos;
};
class CTextBox 
{
public:
    CFX_WideString	m_Text;
    FX_FLOAT	m_Left;
    FX_FLOAT	m_Right;
    FX_FLOAT	m_SpaceWidth;
    FX_FLOAT	m_Top;
    FX_FLOAT	m_Bottom;
    FX_FLOAT	m_FontSizeV;
    CTextColumn* m_pColumn;
};
class CTextBaseLine 
{
public:
    CTextBaseLine();
    ~CTextBaseLine();
    void	InsertTextBox(FX_FLOAT leftx, FX_FLOAT rightx, FX_FLOAT topy, FX_FLOAT bottomy,
                          FX_FLOAT spacew, FX_FLOAT fontsize_v, const CFX_WideString& str);
    FX_BOOL	GetWidth(FX_FLOAT& leftx, FX_FLOAT& rightx);
    FX_BOOL	CanMerge(CTextBaseLine* pOther);
    void	Merge(CTextBaseLine* pOther);
    void	MergeBoxes();
    void	CountChars(int& count, FX_FLOAT& width, int& minchars);
    void	WriteOutput(CFX_WideString& str, FX_FLOAT leftx, FX_FLOAT width, int iWidth);
    FX_FLOAT	m_BaseLine;
    FX_FLOAT	m_Top;
    FX_FLOAT	m_Bottom;
    FX_FLOAT	m_MaxFontSizeV;
    CFX_PtrArray		m_TextList;
};
class CPDF_PageObject;
class CPDF_TextObject;
class CTextPage 
{
public:
    CTextPage();
    ~CTextPage();
    void	ProcessObject(CPDF_PageObject* pObj);
    CTextBaseLine* InsertTextBox(CTextBaseLine* pBaseLine, FX_FLOAT basey, FX_FLOAT leftx,
                                 FX_FLOAT rightx, FX_FLOAT topy, FX_FLOAT bottomy, FX_FLOAT spacew, FX_FLOAT fontsize_v,
                                 CFX_ByteString& str, CPDF_Font* pFont);
    void	WriteOutput(CFX_WideStringArray& lines, int iMinWidth);
    FX_BOOL	m_bAutoWidth;
    FX_BOOL	m_bKeepColumn;
    FX_BOOL	m_bBreakSpace;
    FX_BOOL	m_bOCR;
private:
    CFX_PtrArray	m_BaseLines;
    CFX_PtrArray	m_TextColumns;
    void	FindColumns();
    CTextColumn*	FindColumn(FX_FLOAT xpos);
    void	BreakSpace(CPDF_TextObject* pTextObj);
};

#endif  // CORE_SRC_FPDFTEXT_TXTPROC_H_
