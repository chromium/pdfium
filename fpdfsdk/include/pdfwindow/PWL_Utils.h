// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_UTILS_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_UTILS_H_

#include "../../../core/include/fpdfapi/fpdf_parser.h"  // For CPDF_Point.
#include "../../../core/include/fpdfdoc/fpdf_vt.h"  // For CPVT_WordRange.
#include "../fxedit/fx_edit.h"
#include "PWL_Wnd.h"

class CFX_PathData;
struct CPWL_Color;

template<class T> T PWL_MIN (const T & i, const T & j) { return ((i < j) ? i : j); }
template<class T> T PWL_MAX (const T & i, const T & j) { return ((i > j) ? i : j); }

#define PWL_PDF2WIN(color)					(FX_BYTE(color*255))
#define PWL_WIN2PDF(color)					((FX_FLOAT)((FX_FLOAT)color/255.0f))

#define PWL_MAKEDWORD(low,high)				((FX_DWORD)((FX_WORD)(low) | (FX_DWORD)(((FX_WORD)(high))<<16))) 
#define PWL_GETLOWWORD(dword)				((FX_WORD)(dword))
#define PWL_GETHIGHWORD(dword)				((FX_WORD)(dword>>16))

#define PWL_ICONTYPE_CHECKMARK				0
#define PWL_ICONTYPE_CIRCLE					1
#define PWL_ICONTYPE_COMMENT				2
#define PWL_ICONTYPE_CROSS					3
#define PWL_ICONTYPE_HELP					4
#define PWL_ICONTYPE_INSERTTEXT				5
#define PWL_ICONTYPE_KEY					6
#define PWL_ICONTYPE_NEWPARAGRAPH			7
#define PWL_ICONTYPE_TEXTNOTE				8
#define PWL_ICONTYPE_PARAGRAPH				9
#define PWL_ICONTYPE_RIGHTARROW				10
#define PWL_ICONTYPE_RIGHTPOINTER			11
#define PWL_ICONTYPE_STAR					12
#define PWL_ICONTYPE_UPARROW				13
#define PWL_ICONTYPE_UPLEFTARROW			14

#define PWL_ICONTYPE_GRAPH					15
#define PWL_ICONTYPE_PAPERCLIP				16
#define PWL_ICONTYPE_ATTACHMENT				17
#define PWL_ICONTYPE_TAG					18

#define PWL_ICONTYPE_FOXIT					19

#define PWL_ICONTYPE_UNKNOWN				-1

//checkbox & radiobutton style
#define PCS_CHECK							0
#define PCS_CIRCLE							1
#define PCS_CROSS							2
#define PCS_DIAMOND							3
#define PCS_SQUARE							4
#define PCS_STAR							5

#define	PWL_PI								3.14159265358979f
#define PWL_BEZIER							0.5522847498308f

//pushbutton layout style
#define PPBL_LABEL							0
#define PPBL_ICON							1
#define PPBL_ICONTOPLABELBOTTOM				2
#define	PPBL_LABELTOPICONBOTTOM				3
#define	PPBL_ICONLEFTLABELRIGHT				4
#define PPBL_LABELLEFTICONRIGHT				5
#define PPBL_LABELOVERICON					6

class CPWL_Point : public CPDF_Point
{
public:
	CPWL_Point() : CPDF_Point(0.0f,0.0f){}
	CPWL_Point(FX_FLOAT fx, FX_FLOAT fy) : CPDF_Point(fx,fy) {}
	CPWL_Point(const CPWL_Point& point) : CPDF_Point(point.x, point.y) {}
};

enum PWL_PATHDATA_TYPE
{
	PWLPT_MOVETO,
	PWLPT_LINETO,
	PWLPT_BEZIERTO,
	PWLPT_UNKNOWN
};

enum PWL_PATH_TYPE
{
	PWLPT_PATHDATA,
	PWLPT_STREAM
};

class CPWL_PathData
{
public:
	CPWL_PathData() : point(), type(PWLPT_UNKNOWN){}
	CPWL_PathData(const CPWL_Point& pt, PWL_PATHDATA_TYPE tp) : point(pt), type(tp) {}

	CPWL_Point								point;
	PWL_PATHDATA_TYPE						type;
};

class IPWL_SpellCheck;

class PWL_CLASS CPWL_Utils
{
public:
	static CPDF_Rect						InflateRect(const CPDF_Rect& rcRect, FX_FLOAT fSize);
	static CPDF_Rect						DeflateRect(const CPDF_Rect& rcRect, FX_FLOAT fSize);
	static FX_BOOL							IntersectRect(const CPDF_Rect& rect1, const CPDF_Rect& rect2);
	static FX_BOOL							ContainsRect(const CPDF_Rect& rcParent, const CPDF_Rect& rcChild);
	static CPDF_Rect						ScaleRect(const CPDF_Rect& rcRect,FX_FLOAT fScale);
	static CPVT_WordRange					OverlapWordRange(const CPVT_WordRange& wr1, const CPVT_WordRange& wr2);
	static CPDF_Rect						GetCenterSquare(const CPDF_Rect & rect);
	static CPWL_Color						SubstractColor(const CPWL_Color & sColor,FX_FLOAT fColorSub);
	static CPWL_Color						DevideColor(const CPWL_Color & sColor,FX_FLOAT fColorDevide);
	static CPDF_Rect						MaxRect(const CPDF_Rect & rect1,const CPDF_Rect & rect2);
	static CPDF_Rect						OffsetRect(const CPDF_Rect & rect,FX_FLOAT x,FX_FLOAT y);
	static CPDF_Point						OffsetPoint(const  CPDF_Point & point,FX_FLOAT x,FX_FLOAT y);
	static FX_COLORREF						PWLColorToFXColor(const CPWL_Color& color, FX_INT32 nTransparancy = 255);
	static FX_BOOL							IsBlackOrWhite(const CPWL_Color& color);
	static CPWL_Color						GetReverseColor(const CPWL_Color& color);

	static CFX_ByteString					GetColorAppStream(const CPWL_Color & color,const FX_BOOL & bFillOrStroke = TRUE);
	static CFX_ByteString					GetBorderAppStream(const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash);
	static CFX_ByteString					GetCircleBorderAppStream(const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash);
	static CFX_ByteString					GetRectFillAppStream(const CPDF_Rect & rect,const CPWL_Color & color);
	static CFX_ByteString					GetCircleFillAppStream(const CPDF_Rect & rect,const CPWL_Color & color);

	static CFX_ByteString					GetPushButtonAppStream(const CPDF_Rect & rcBBox,
														IFX_Edit_FontMap * pFontMap,
														CPDF_Stream * pIconStream,
														CPDF_IconFit & IconFit,
														const CFX_WideString & sLabel,														
														const CPWL_Color & crText,
														FX_FLOAT fFontSize,
														FX_INT32 nLayOut);
	static CFX_ByteString					GetCheckBoxAppStream(const CPDF_Rect & rcBBox,
														FX_INT32 nStyle,
														const CPWL_Color & crText);
	static CFX_ByteString					GetRadioButtonAppStream(const CPDF_Rect & rcBBox,
														FX_INT32 nStyle,
														const CPWL_Color & crText);

	static CFX_ByteString					GetEditAppStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset, const CPVT_WordRange * pRange = NULL, 
														FX_BOOL bContinuous = TRUE, FX_WORD SubWord = 0);
	static CFX_ByteString					GetEditSelAppStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset,
														const CPVT_WordRange * pRange = NULL);
	static CFX_ByteString					GetSpellCheckAppStream(IFX_Edit* pEdit, IPWL_SpellCheck* pSpellCheck,
														const CPDF_Point & ptOffset,
														const CPVT_WordRange * pRange = NULL);
	static CFX_ByteString					GetTextAppStream(const CPDF_Rect & rcBBox,IFX_Edit_FontMap * pFontMap,
														const CFX_WideString & sText, FX_INT32 nAlignmentH, FX_INT32 nAlignmentV,
														FX_FLOAT fFontSize, FX_BOOL bMultiLine, FX_BOOL bAutoReturn, const CPWL_Color & crText);
	static CFX_ByteString					GetDropButtonAppStream(const CPDF_Rect & rcBBox);

	static void								DrawFillRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,const CPDF_Rect & rect,
														const CPWL_Color & color, FX_INT32 nTransparancy);
	static void								DrawFillRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
														const CPDF_Rect & rect,const FX_COLORREF & color);
	static void								DrawStrokeRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,const CPDF_Rect & rect,
														const FX_COLORREF & color, FX_FLOAT fWidth);
	static void								DrawStrokeLine(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
														const CPDF_Point & ptMoveTo, const CPDF_Point & ptLineTo, const FX_COLORREF & color, FX_FLOAT fWidth);
	static void								DrawBorder(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
												const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash, FX_INT32 nTransparancy);
	static void								DrawFillArea(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
														const CPDF_Point* pPts, FX_INT32 nCount, const FX_COLORREF& color);
	static void								DrawShadow(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
														FX_BOOL bVertical, FX_BOOL bHorizontal, CPDF_Rect rect,
														FX_INT32 nTransparancy, FX_INT32 nStartGray, FX_INT32 nEndGray);
	static void								DrawEditSpellCheck(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, 
														const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, 
														IPWL_SpellCheck* pSpellCheck);
public:
	static void								ConvertCMYK2RGB(FX_FLOAT dC,FX_FLOAT dM,FX_FLOAT dY,FX_FLOAT dK,FX_FLOAT &dR,FX_FLOAT &dG,FX_FLOAT &dB);
	static void								ConvertRGB2CMYK(FX_FLOAT dR,FX_FLOAT dG,FX_FLOAT dB,FX_FLOAT &dC,FX_FLOAT &dM,FX_FLOAT &dY,FX_FLOAT &dK);
	
	static void								ConvertRGB2GRAY(FX_FLOAT dR,FX_FLOAT dG,FX_FLOAT dB,FX_FLOAT &dGray);
	static void								ConvertGRAY2RGB(FX_FLOAT dGray,FX_FLOAT &dR,FX_FLOAT &dG,FX_FLOAT &dB);

	static void								ConvertCMYK2GRAY(FX_FLOAT dC,FX_FLOAT dM,FX_FLOAT dY,FX_FLOAT dK,FX_FLOAT &dGray);
	static void								ConvertGRAY2CMYK(FX_FLOAT dGray,FX_FLOAT &dC,FX_FLOAT &dM,FX_FLOAT &dY,FX_FLOAT &dK);

	static void								PWLColorToARGB(const CPWL_Color& color, FX_INT32& alpha, FX_FLOAT& red, FX_FLOAT& green, FX_FLOAT& blue);

public:
	static CFX_ByteString					GetIconAppStream(FX_INT32 nType, const CPDF_Rect& rect, const CPWL_Color& crFill, 
												const CPWL_Color& crStroke = PWL_DEFAULT_BLACKCOLOR);
	static void								DrawIconAppStream(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
												FX_INT32 nType, const CPDF_Rect & rect, const CPWL_Color& crFill, 
												const CPWL_Color& crStroke, const FX_INT32 nTransparancy);

private:
	static CFX_ByteString					GetAppStreamFromArray(const CPWL_PathData* pPathData, FX_INT32 nCount);
	static void								GetPathDataFromArray(CFX_PathData& path, const CPWL_PathData* pPathData, FX_INT32 nCount);

	static CFX_ByteString					GetAppStream_Check(const CPDF_Rect & rcBBox, const CPWL_Color & crText);
	static CFX_ByteString					GetAppStream_Circle(const CPDF_Rect & rcBBox, const CPWL_Color & crText);
	static CFX_ByteString					GetAppStream_Cross(const CPDF_Rect & rcBBox, const CPWL_Color & crText);
	static CFX_ByteString					GetAppStream_Diamond(const CPDF_Rect & rcBBox, const CPWL_Color & crText);
	static CFX_ByteString					GetAppStream_Square(const CPDF_Rect & rcBBox, const CPWL_Color & crText);
	static CFX_ByteString					GetAppStream_Star(const CPDF_Rect & rcBBox, const CPWL_Color & crText);

	static CFX_ByteString					GetAP_Check(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_Circle(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_Cross(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_Diamond(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_Square(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_Star(const CPDF_Rect & crBBox);
	static CFX_ByteString					GetAP_HalfCircle(const CPDF_Rect & crBBox,FX_FLOAT fRotate);

	static void								GetGraphics_Checkmark(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Circle(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Comment(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Cross(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Help(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_InsertText(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Key(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_NewParagraph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_TextNote(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Paragraph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_RightArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_RightPointer(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Star(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_UpArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_UpLeftArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Graph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Paperclip(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Attachment(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Tag(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
	static void								GetGraphics_Foxit(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type);
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_UTILS_H_
