// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TEXTOUT
#define _FDE_TEXTOUT
class IFDE_TextOut;
#define FDE_TTOSTYLE_Underline			0x0001
#define FDE_TTOSTYLE_Strikeout			0x0002
#define FDE_TTOSTYLE_VerticalLayout		0x0004
#define FDE_TTOSTYLE_SingleLine			0x0010
#define FDE_TTOSTYLE_ExpandTab			0x0020
#define FDE_TTOSTYLE_HotKey				0x0040
#define FDE_TTOSTYLE_Ellipsis			0x0080
#define FDE_TTOSTYLE_LineWrap			0x0100
#define FDE_TTOSTYLE_ArabicShapes		0x0200
#define FDE_TTOSTYLE_RTL				0x0400
#define FDE_TTOSTYLE_ArabicContext		0x0800
#define FDE_TTOSTYLE_LastLineHeight		0x1000
#define FDE_TTOALIGNMENT_TopLeft		0
#define FDE_TTOALIGNMENT_TopCenter		1
#define FDE_TTOALIGNMENT_TopRight		2
#define FDE_TTOALIGNMENT_TopAuto		3
#define FDE_TTOALIGNMENT_CenterLeft		4
#define FDE_TTOALIGNMENT_Center			5
#define FDE_TTOALIGNMENT_CenterRight	6
#define FDE_TTOALIGNMENT_CenterAuto		7
#define FDE_TTOALIGNMENT_BottomLeft		8
#define FDE_TTOALIGNMENT_BottomCenter	9
#define FDE_TTOALIGNMENT_BottomRight	10
#define FDE_TTOALIGNMENT_BottomAuto		11
class IFDE_TextOut
{
public:
    static IFDE_TextOut*	Create();
    virtual void		Release() = 0;
    virtual void		SetFont(IFX_Font *pFont) = 0;
    virtual void		SetFontSize(FX_FLOAT fFontSize) = 0;
    virtual void		SetTextColor(FX_ARGB color) = 0;
    virtual void		SetStyles(FX_DWORD dwStyles) = 0;
    virtual void		SetTabWidth(FX_FLOAT fTabWidth) = 0;
    virtual void		SetEllipsisString(const CFX_WideString &wsEllipsis) = 0;
    virtual void		SetParagraphBreakChar(FX_WCHAR wch) = 0;
    virtual void		SetAlignment(FX_INT32 iAlignment) = 0;
    virtual void		SetLineSpace(FX_FLOAT fLineSpace) = 0;
    virtual void		SetDIBitmap(CFX_DIBitmap *pDIB) = 0;
    virtual void		SetRenderDevice(CFX_RenderDevice *pDevice) = 0;
    virtual void		SetClipRect(const CFX_Rect &rtClip) = 0;
    virtual void		SetClipRect(const CFX_RectF &rtClip) = 0;
    virtual void		SetMatrix(const CFX_Matrix &matrix) = 0;
    virtual void		SetLineBreakTolerance(FX_FLOAT fTolerance) = 0;
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_Size &size) = 0;
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_SizeF &size) = 0;
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_Rect &rect) = 0;
    virtual void		CalcSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_RectF &rect) = 0;
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_INT32 x, FX_INT32 y) = 0;
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_FLOAT x, FX_FLOAT y) = 0;
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_Rect &rect) = 0;
    virtual void		DrawText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect) = 0;
    virtual void		SetLogicClipRect(const CFX_RectF &rtClip) = 0;
    virtual void		CalcLogicSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_SizeF &size) = 0;
    virtual void		CalcLogicSize(FX_LPCWSTR pwsStr, FX_INT32 iLength, CFX_RectF &rect) = 0;
    virtual void		DrawLogicText(FX_LPCWSTR pwsStr, FX_INT32 iLength, FX_FLOAT x, FX_FLOAT y) = 0;
    virtual void		DrawLogicText(FX_LPCWSTR pwsStr, FX_INT32 iLength, const CFX_RectF &rect) = 0;
    virtual FX_INT32	GetTotalLines() = 0;
};
#endif
