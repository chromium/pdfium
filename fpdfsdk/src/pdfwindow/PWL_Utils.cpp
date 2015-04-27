// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/pdfwindow/PDFWindow.h"
#include "../../include/pdfwindow/PWL_Wnd.h"
#include "../../include/pdfwindow/PWL_Utils.h"
#include "../../include/pdfwindow/PWL_Icon.h"

#define IsFloatZero(f)						((f) < 0.0001 && (f) > -0.0001)
#define IsFloatBigger(fa,fb)				((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa,fb)				((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa,fb)					IsFloatZero((fa)-(fb))

/* ---------------------------- CPWL_Utils ------------------------------ */

CFX_ByteString CPWL_Utils::GetAppStreamFromArray(const CPWL_PathData* pPathData, FX_INT32 nCount)
{
	CFX_ByteTextBuf csAP;

	for (FX_INT32 i=0; i<nCount; i++)
	{
		switch (pPathData[i].type)
		{
		case PWLPT_MOVETO:
			csAP << pPathData[i].point.x << " " << pPathData[i].point.y << " m\n";
			break;
		case PWLPT_LINETO:
			csAP << pPathData[i].point.x << " " << pPathData[i].point.y << " l\n";
			break;
		case PWLPT_BEZIERTO:
			csAP << pPathData[i].point.x << " " << pPathData[i].point.y << " "
				 << pPathData[i+1].point.x << " " << pPathData[i+1].point.y << " "
				 << pPathData[i+2].point.x << " " << pPathData[i+2].point.y << " c\n";

			i += 2;
			break;
		default:
			break;
		}
	}

	return csAP.GetByteString();
}

void CPWL_Utils::GetPathDataFromArray(CFX_PathData& path, const CPWL_PathData* pPathData, FX_INT32 nCount)
{
	path.SetPointCount(nCount);

	for (FX_INT32 i=0; i<nCount; i++)
	{
		switch (pPathData[i].type)
		{
		case PWLPT_MOVETO:
			path.SetPoint(i, pPathData[i].point.x, pPathData[i].point.y, FXPT_MOVETO);
			break;
		case PWLPT_LINETO:
			path.SetPoint(i, pPathData[i].point.x, pPathData[i].point.y, FXPT_LINETO);
			break;
		case PWLPT_BEZIERTO:
			path.SetPoint(i, pPathData[i].point.x, pPathData[i].point.y, FXPT_BEZIERTO);
			break;
		default:
			break;
		}
	}
}


CPDF_Rect CPWL_Utils::MaxRect(const CPDF_Rect & rect1,const CPDF_Rect & rect2)
{
	CPDF_Rect rcRet;

	rcRet.left = PWL_MIN(rect1.left,rect2.left);
	rcRet.bottom = PWL_MIN(rect1.bottom,rect2.bottom);
	rcRet.right = PWL_MAX(rect1.right,rect2.right);
	rcRet.top = PWL_MAX(rect1.top,rect2.top);

	return rcRet;
}

CPDF_Rect CPWL_Utils::OffsetRect(const CPDF_Rect & rect,FX_FLOAT x,FX_FLOAT y)
{
	return CPDF_Rect(rect.left + x,rect.bottom + y,
		rect.right + x,rect.top + y);
}

FX_BOOL CPWL_Utils::ContainsRect(const CPDF_Rect& rcParent, const CPDF_Rect& rcChild)
{
	return rcChild.left >= rcParent.left && rcChild.bottom >= rcParent.bottom &&
			rcChild.right <= rcParent.right && rcChild.top <= rcParent.top;
}

FX_BOOL CPWL_Utils::IntersectRect(const CPDF_Rect& rect1, const CPDF_Rect& rect2)
{
	FX_FLOAT left = rect1.left > rect2.left ? rect1.left : rect2.left;
	FX_FLOAT right = rect1.right < rect2.right ? rect1.right : rect2.right;
	FX_FLOAT bottom = rect1.bottom > rect2.bottom ? rect1.bottom : rect2.bottom;
	FX_FLOAT top = rect1.top < rect2.top ? rect1.top : rect2.top;

	return left < right && bottom < top;
}

CPDF_Point CPWL_Utils::OffsetPoint(const CPDF_Point& point,FX_FLOAT x,FX_FLOAT y)
{
	return CPDF_Point(point.x + x,point.y + y);
}

CPVT_WordRange CPWL_Utils::OverlapWordRange(const CPVT_WordRange & wr1, const CPVT_WordRange & wr2)
{
	CPVT_WordRange wrRet;

	if (wr2.EndPos.WordCmp(wr1.BeginPos) < 0 || wr2.BeginPos.WordCmp(wr1.EndPos) > 0) return wrRet;
	if (wr1.EndPos.WordCmp(wr2.BeginPos) < 0 || wr1.BeginPos.WordCmp(wr2.EndPos) > 0) return wrRet;

	if (wr1.BeginPos.WordCmp(wr2.BeginPos) < 0)
	{
		wrRet.BeginPos = wr2.BeginPos;
	}
	else
	{
		wrRet.BeginPos = wr1.BeginPos;
	}

	if (wr1.EndPos.WordCmp(wr2.EndPos) < 0)
	{
		wrRet.EndPos = wr1.EndPos;
	}
	else
	{
		wrRet.EndPos = wr2.EndPos;
	}

	return wrRet;
}

CFX_ByteString CPWL_Utils::GetAP_Check(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	const FX_INT32 num = 8;

	CPWL_Point pts[num*3] = 
	{
		//1
		CPWL_Point(0.28f, 0.52f), 
		CPWL_Point(0.27f, 0.48f),
		CPWL_Point(0.29f, 0.40f),

		//2
		CPWL_Point(0.30f, 0.33f), 
		CPWL_Point(0.31f, 0.29f),
		CPWL_Point(0.31f, 0.28f),

		//3
		CPWL_Point(0.39f, 0.28f), 
		CPWL_Point(0.49f, 0.29f),
		CPWL_Point(0.77f, 0.67f),

		//4
		CPWL_Point(0.76f, 0.68f), 
		CPWL_Point(0.78f, 0.69f),
		CPWL_Point(0.76f, 0.75f),

		//5
		CPWL_Point(0.76f, 0.75f), 
		CPWL_Point(0.73f, 0.80f),
		CPWL_Point(0.68f, 0.75f),

		//6
		CPWL_Point(0.68f, 0.74f), 
		CPWL_Point(0.68f, 0.74f),
		CPWL_Point(0.44f, 0.47f),

		//7
		CPWL_Point(0.43f, 0.47f), 
		CPWL_Point(0.40f, 0.47f),
		CPWL_Point(0.41f, 0.58f),

		//8
		CPWL_Point(0.40f, 0.60f), 
		CPWL_Point(0.28f, 0.66f),
		CPWL_Point(0.30f, 0.56f)
	};

	for (FX_INT32 j=0; j<num*3; j++)
	{
		pts[j].x *= fWidth;
		pts[j].x += crBBox.left;

		pts[j].y *= fHeight;
		pts[j].y += crBBox.bottom;
	}

	csAP << pts[0].x << " " << pts[0].y << " m\n";

	for (FX_INT32 i=0; i<num; i++)
	{
		FX_INT32 nCur = i*3;
		FX_INT32 n1 = i*3 + 1;
		FX_INT32 n2 = i*3 + 2;
		FX_INT32 nNext = (i < num-1 ? (i+1)*3 : 0);

		FX_FLOAT px1 = pts[n1].x - pts[nCur].x;
		FX_FLOAT py1 = pts[n1].y - pts[nCur].y;
		FX_FLOAT px2 = pts[n2].x - pts[nNext].x;
		FX_FLOAT py2 = pts[n2].y - pts[nNext].y;

		csAP << pts[nCur].x + px1 * PWL_BEZIER << " " << pts[nCur].y + py1 * PWL_BEZIER << " " 
			<< pts[nNext].x + px2 * PWL_BEZIER << " " << pts[nNext].y + py2 * PWL_BEZIER << " "
			<< pts[nNext].x << " " << pts[nNext].y << " c\n";
	}

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_Circle(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPDF_Point pt1(crBBox.left,crBBox.bottom + fHeight / 2);
	CPDF_Point pt2(crBBox.left + fWidth / 2,crBBox.top);
	CPDF_Point pt3(crBBox.right,crBBox.bottom + fHeight / 2);
	CPDF_Point pt4(crBBox.left + fWidth / 2,crBBox.bottom);

	csAP << pt1.x << " " << pt1.y << " m\n";

	FX_FLOAT px = pt2.x - pt1.x;
	FX_FLOAT py = pt2.y - pt1.y;

	csAP << pt1.x << " " << pt1.y + py * PWL_BEZIER << " " 
		<< pt2.x - px * PWL_BEZIER << " " << pt2.y << " "
		<< pt2.x << " " << pt2.y << " c\n";

	px = pt3.x - pt2.x;
	py = pt2.y - pt3.y;

	csAP << pt2.x + px * PWL_BEZIER << " " << pt2.y << " " 
		<< pt3.x << " " << pt3.y + py * PWL_BEZIER << " "
		<< pt3.x << " " << pt3.y << " c\n";

	px = pt3.x - pt4.x;
	py = pt3.y - pt4.y;

	csAP << pt3.x << " " << pt3.y - py * PWL_BEZIER << " " 
		<< pt4.x + px * PWL_BEZIER << " " << pt4.y << " "
		<< pt4.x << " " << pt4.y << " c\n";

	px = pt4.x - pt1.x;
	py = pt1.y - pt4.y;

	csAP << pt4.x - px * PWL_BEZIER << " " << pt4.y << " " 
		<< pt1.x << " " << pt1.y - py * PWL_BEZIER << " "
		<< pt1.x << " " << pt1.y << " c\n";

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_Cross(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	csAP << crBBox.left << " " << crBBox.top << " m\n";
	csAP << crBBox.right << " " << crBBox.bottom << " l\n";
	csAP << crBBox.left << " " << crBBox.bottom << " m\n";
	csAP << crBBox.right << " " << crBBox.top << " l\n";

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_Diamond(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPDF_Point pt1(crBBox.left,crBBox.bottom + fHeight / 2);
	CPDF_Point pt2(crBBox.left + fWidth / 2,crBBox.top);
	CPDF_Point pt3(crBBox.right,crBBox.bottom + fHeight / 2);
	CPDF_Point pt4(crBBox.left + fWidth / 2,crBBox.bottom);
	
	csAP << pt1.x << " " << pt1.y << " m\n";
	csAP << pt2.x << " " << pt2.y << " l\n";
	csAP << pt3.x << " " << pt3.y << " l\n";
	csAP << pt4.x << " " << pt4.y << " l\n";
	csAP << pt1.x << " " << pt1.y << " l\n";

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_Square(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	csAP << crBBox.left << " " << crBBox.top << " m\n";
	csAP << crBBox.right << " " << crBBox.top << " l\n";
	csAP << crBBox.right << " " << crBBox.bottom << " l\n";
	csAP << crBBox.left << " " << crBBox.bottom << " l\n";
	csAP << crBBox.left << " " << crBBox.top << " l\n";

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_Star(const CPDF_Rect & crBBox)
{
	CFX_ByteTextBuf csAP;

	FX_FLOAT fRadius = (crBBox.top - crBBox.bottom)/(1+(FX_FLOAT)cos(PWL_PI/5.0f));
	CPDF_Point ptCenter = CPDF_Point((crBBox.left + crBBox.right) / 2.0f,(crBBox.top + crBBox.bottom) / 2.0f);
	
	FX_FLOAT px[5],py[5];

	FX_FLOAT fAngel = PWL_PI/10.0f;

	for (FX_INT32 i=0; i<5; i++)
	{
		px[i] = ptCenter.x + fRadius * (FX_FLOAT)cos(fAngel);
		py[i] = ptCenter.y + fRadius * (FX_FLOAT)sin(fAngel);

		fAngel += PWL_PI * 2 / 5.0f;
	}

	csAP << px[0] << " " << py[0] << " m\n";

	FX_INT32 nNext = 0;
	for (FX_INT32 j=0; j<5; j++)
	{
		nNext += 2;
		if (nNext >= 5) nNext -= 5;
		csAP << px[nNext] << " " << py[nNext] << " l\n";
	}

	return csAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAP_HalfCircle(const CPDF_Rect & crBBox,FX_FLOAT fRotate)
{
	CFX_ByteTextBuf csAP;

	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPDF_Point pt1(-fWidth/2,0);
	CPDF_Point pt2(0,fHeight/2);
	CPDF_Point pt3(fWidth/2,0);

	FX_FLOAT px,py;

	csAP << cos(fRotate) << " " << sin(fRotate) << " " << -sin(fRotate) << " " << cos(fRotate) << " " 
		<< crBBox.left + fWidth / 2 << " " << crBBox.bottom + fHeight / 2 << " cm\n";


	csAP << pt1.x << " " << pt1.y << " m\n";

	px = pt2.x - pt1.x;
	py = pt2.y - pt1.y;

	csAP << pt1.x << " " << pt1.y + py * PWL_BEZIER << " " 
		<< pt2.x - px * PWL_BEZIER << " " << pt2.y << " "
		<< pt2.x << " " << pt2.y << " c\n";

	px = pt3.x - pt2.x;
	py = pt2.y - pt3.y;

	csAP << pt2.x + px * PWL_BEZIER << " " << pt2.y << " " 
		<< pt3.x << " " << pt3.y + py * PWL_BEZIER << " "
		<< pt3.x << " " << pt3.y << " c\n";

	return csAP.GetByteString();
}


CPDF_Rect CPWL_Utils::InflateRect(const CPDF_Rect & rcRect, FX_FLOAT fSize)
{
	if (rcRect.IsEmpty()) return rcRect;

	CPDF_Rect rcNew(rcRect.left - fSize,
					rcRect.bottom - fSize,
					rcRect.right + fSize,
					rcRect.top + fSize);
	rcNew.Normalize();
	return rcNew;
}

CPDF_Rect CPWL_Utils::DeflateRect(const CPDF_Rect & rcRect, FX_FLOAT fSize)
{
	if (rcRect.IsEmpty()) return rcRect;

	CPDF_Rect rcNew(rcRect.left + fSize,
					rcRect.bottom + fSize,
					rcRect.right - fSize,
					rcRect.top - fSize);
	rcNew.Normalize();
	return rcNew;
}

CPDF_Rect CPWL_Utils::ScaleRect(const CPDF_Rect & rcRect,FX_FLOAT fScale)
{
	FX_FLOAT fHalfWidth = (rcRect.right - rcRect.left) / 2.0f;
	FX_FLOAT fHalfHeight = (rcRect.top - rcRect.bottom) / 2.0f;

	CPDF_Point ptCenter = CPDF_Point((rcRect.left + rcRect.right) / 2,(rcRect.top + rcRect.bottom) / 2);

	return CPDF_Rect(ptCenter.x - fHalfWidth * fScale,
				ptCenter.y - fHalfHeight * fScale,
				ptCenter.x + fHalfWidth * fScale,
				ptCenter.y + fHalfHeight * fScale);
}

CFX_ByteString CPWL_Utils::GetRectFillAppStream(const CPDF_Rect & rect,const CPWL_Color & color)
{
	CFX_ByteTextBuf sAppStream;

	CFX_ByteString sColor = GetColorAppStream(color,TRUE);
	if (sColor.GetLength() > 0)
	{
		sAppStream << "q\n" << sColor;
		sAppStream << rect.left << " " << rect.bottom << " "
			<< rect.right - rect.left << " " << rect.top - rect.bottom << " re f\nQ\n";		
	}
	
	return sAppStream.GetByteString();
}

CFX_ByteString CPWL_Utils::GetCircleFillAppStream(const CPDF_Rect & rect,const CPWL_Color & color)
{
	CFX_ByteTextBuf sAppStream;

	CFX_ByteString sColor = GetColorAppStream(color,TRUE);
	if (sColor.GetLength() > 0)
	{
		sAppStream << "q\n" << sColor << CPWL_Utils::GetAP_Circle(rect) << "f\nQ\n";		
	}
	
	return sAppStream.GetByteString();
}

CPDF_Rect CPWL_Utils::GetCenterSquare(const CPDF_Rect & rect)
{
	FX_FLOAT fWidth = rect.right -  rect.left;
	FX_FLOAT fHeight = rect.top - rect.bottom;

	FX_FLOAT fCenterX = (rect.left + rect.right)/2.0f;
	FX_FLOAT fCenterY = (rect.top + rect.bottom)/2.0f;

	FX_FLOAT fRadius = (fWidth > fHeight) ? fHeight / 2 : fWidth / 2;

	return CPDF_Rect(fCenterX - fRadius,fCenterY - fRadius,fCenterX + fRadius,fCenterY + fRadius);
}

CFX_ByteString CPWL_Utils::GetEditAppStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset, const CPVT_WordRange * pRange, 
														FX_BOOL bContinuous, FX_WORD SubWord)
{
	return IFX_Edit::GetEditAppearanceStream(pEdit,ptOffset,pRange,bContinuous,SubWord);
}

CFX_ByteString CPWL_Utils::GetEditSelAppStream(IFX_Edit* pEdit, const CPDF_Point & ptOffset, 
								const CPVT_WordRange * pRange)
{
	return IFX_Edit::GetSelectAppearanceStream(pEdit,ptOffset,pRange);
}

static CFX_ByteString GetSquigglyAppearanceStream(FX_FLOAT fStartX, FX_FLOAT fEndX, FX_FLOAT fY, FX_FLOAT fStep)
{
	CFX_ByteTextBuf sRet;

	sRet << "0 w\n" << fStartX << " " << fY << " m\n";

	FX_FLOAT fx;
	FX_INT32 i;

	for (i=1,fx=fStartX+fStep; fx<fEndX; fx+=fStep,i++)
	{
		sRet << fx << " " << fY + (i&1)*fStep << " l\n";
	}

	sRet << "S\n";

	return sRet.GetByteString();
}

static CFX_ByteString GetWordSpellCheckAppearanceStream(IFX_Edit_Iterator* pIterator, const CPDF_Point & ptOffset,
																  const CPVT_WordRange & wrWord)
{
	CFX_ByteTextBuf sRet;

	FX_FLOAT fStartX = 0.0f;
	FX_FLOAT fEndX = 0.0f;
	FX_FLOAT fY = 0.0f;
	FX_FLOAT fStep = 0.0f; 

	FX_BOOL bBreak = FALSE;

	if (pIterator)
	{
		pIterator->SetAt(wrWord.BeginPos);

		do
		{
			CPVT_WordPlace place = pIterator->GetAt();

			CPVT_Line line;				
			if (pIterator->GetLine(line))
			{
				fY = line.ptLine.y;
				fStep = (line.fLineAscent - line.fLineDescent) / 16.0f;
			}

			if (place.LineCmp(wrWord.BeginPos) == 0)
			{
				pIterator->SetAt(wrWord.BeginPos);
				CPVT_Word word;
				if (pIterator->GetWord(word))
				{
					fStartX = word.ptWord.x;
				}
			}
			else
			{
				fStartX = line.ptLine.x;
			}

			if (place.LineCmp(wrWord.EndPos) == 0)
			{
				pIterator->SetAt(wrWord.EndPos);
				CPVT_Word word;
				if (pIterator->GetWord(word))
				{
					fEndX = word.ptWord.x + word.fWidth;
				}

				bBreak = TRUE;
			}
			else
			{
				fEndX = line.ptLine.x + line.fLineWidth;
			}

			sRet << GetSquigglyAppearanceStream(fStartX + ptOffset.x, fEndX + ptOffset.x, fY + ptOffset.y,fStep);

			if (bBreak) break;
		}
		while (pIterator->NextLine());
	}

	return sRet.GetByteString();
}

CFX_ByteString CPWL_Utils::GetSpellCheckAppStream(IFX_Edit* pEdit, IPWL_SpellCheck* pSpellCheck, const CPDF_Point & ptOffset,
								const CPVT_WordRange * pRange)
{
	ASSERT(pEdit != NULL);
	ASSERT(pSpellCheck != NULL);

	CFX_ByteTextBuf sRet;

	if (pRange && pRange->IsExist())
	{
		if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator())
		{
			pIterator->SetAt(pRange->BeginPos);

			FX_BOOL bLatinWord = FALSE;
			CPVT_WordPlace wpWordStart;
			CFX_ByteString sWord;

			CPVT_WordPlace oldplace;
			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();
				if (pRange && place.WordCmp(pRange->EndPos) > 0) break;				

				CPVT_Word word;
				if (pIterator->GetWord(word))
				{
					if (FX_EDIT_ISLATINWORD(word.Word))
					{
						if (!bLatinWord)
						{
							wpWordStart = place;
							bLatinWord = TRUE;								
						}

						sWord += (char)word.Word;
						oldplace = place;
					}
					else
					{
						if (bLatinWord)
						{
							if (!pSpellCheck->CheckWord(sWord))
							{
								sRet << GetWordSpellCheckAppearanceStream(pIterator,ptOffset,CPVT_WordRange(wpWordStart,oldplace));
								pIterator->SetAt(place);
							}
							bLatinWord = FALSE;
						}

						sWord.Empty();
					}
				}
				else
				{
					if (bLatinWord)
					{
						if (!pSpellCheck->CheckWord(sWord))
							sRet << GetWordSpellCheckAppearanceStream(pIterator,ptOffset,CPVT_WordRange(wpWordStart,oldplace));
						bLatinWord = FALSE;
						sWord.Empty();
					}
				}
			}

			if (bLatinWord)
			{
				if (!pSpellCheck->CheckWord(sWord))
					sRet << GetWordSpellCheckAppearanceStream(pIterator,ptOffset,CPVT_WordRange(wpWordStart,oldplace));

				bLatinWord = FALSE;
				sWord.Empty();
			}
		}
	}

	return sRet.GetByteString();
}

CFX_ByteString CPWL_Utils::GetTextAppStream(const CPDF_Rect & rcBBox,IFX_Edit_FontMap * pFontMap,
														const CFX_WideString & sText, FX_INT32 nAlignmentH, FX_INT32 nAlignmentV,
														FX_FLOAT fFontSize, FX_BOOL bMultiLine, FX_BOOL bAutoReturn, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sRet;

	if (IFX_Edit * pEdit = IFX_Edit::NewEdit())
	{			
		pEdit->SetFontMap(pFontMap);

		pEdit->SetPlateRect(rcBBox);
		pEdit->SetAlignmentH(nAlignmentH);
		pEdit->SetAlignmentV(nAlignmentV);
		pEdit->SetMultiLine(bMultiLine);
		pEdit->SetAutoReturn(bAutoReturn);
		if (IsFloatZero(fFontSize))
			pEdit->SetAutoFontSize(TRUE);
		else
			pEdit->SetFontSize(fFontSize);
		pEdit->Initialize();

		pEdit->SetText(sText);
		
		CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(pEdit, CPDF_Point(0.0f,0.0f));
		if (sEdit.GetLength() > 0)
		{
			sRet << "BT\n" << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n";
		}
		
		IFX_Edit::DelEdit(pEdit);
	}

	return sRet.GetByteString();
}

CFX_ByteString CPWL_Utils::GetPushButtonAppStream(const CPDF_Rect & rcBBox,
											IFX_Edit_FontMap * pFontMap,
											CPDF_Stream * pIconStream,
											CPDF_IconFit & IconFit,
											const CFX_WideString & sLabel,											
											const CPWL_Color & crText,
											FX_FLOAT fFontSize,
											FX_INT32 nLayOut)
{
	const FX_FLOAT fAutoFontScale = 1.0f / 3.0f;	

	if (IFX_Edit * pEdit = IFX_Edit::NewEdit())
	{			
		pEdit->SetFontMap(pFontMap);

		pEdit->SetAlignmentH(1);
		pEdit->SetAlignmentV(1);
		pEdit->SetMultiLine(FALSE);
		pEdit->SetAutoReturn(FALSE);
		if (IsFloatZero(fFontSize))
			pEdit->SetAutoFontSize(TRUE);
		else
			pEdit->SetFontSize(fFontSize);
		pEdit->Initialize();
		pEdit->SetText(sLabel);

		CPDF_Rect rcLabelContent = pEdit->GetContentRect();
		
		CPWL_Icon Icon;
		PWL_CREATEPARAM cp;
		cp.dwFlags = PWS_VISIBLE;
		Icon.Create(cp);
		Icon.SetIconFit(&IconFit);
		Icon.SetPDFStream(pIconStream);

		CPDF_Rect rcLabel = CPDF_Rect(0,0,0,0);
		CPDF_Rect rcIcon = CPDF_Rect(0,0,0,0);
		FX_FLOAT fWidth = 0.0f;
		FX_FLOAT fHeight = 0.0f;

		switch (nLayOut)
		{
		case PPBL_LABEL:
			rcLabel = rcBBox;
			rcIcon = CPDF_Rect(0,0,0,0);
			break;
		case PPBL_ICON:
			rcIcon = rcBBox;
			rcLabel = CPDF_Rect(0,0,0,0);
			break;
		case PPBL_ICONTOPLABELBOTTOM:

			if (pIconStream)
			{
				if (IsFloatZero(fFontSize))
				{
					fHeight = rcBBox.top - rcBBox.bottom;
					rcLabel = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.right,rcBBox.bottom + fHeight * fAutoFontScale);
					rcIcon = CPDF_Rect(rcBBox.left,rcLabel.top,rcBBox.right,rcBBox.top);
				}
				else
				{
					fHeight = rcLabelContent.Height();

					if (rcBBox.bottom + fHeight > rcBBox.top)
					{			
						rcIcon = CPDF_Rect(0,0,0,0);				
						rcLabel = rcBBox;
					}
					else
					{
						rcLabel = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.right,rcBBox.bottom + fHeight);
						rcIcon = CPDF_Rect(rcBBox.left,rcLabel.top,rcBBox.right,rcBBox.top);	
					}
				}
			}
			else
			{
				rcLabel = rcBBox;
				rcIcon = CPDF_Rect(0,0,0,0);
			}

			break;
		case PPBL_LABELTOPICONBOTTOM:

			if (pIconStream)
			{
				if (IsFloatZero(fFontSize))
				{
					fHeight = rcBBox.top - rcBBox.bottom;
					rcLabel = CPDF_Rect(rcBBox.left,rcBBox.top - fHeight * fAutoFontScale ,rcBBox.right,rcBBox.top);
					rcIcon = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.right,rcLabel.bottom);
				}
				else
				{
					fHeight = rcLabelContent.Height();

					if (rcBBox.bottom + fHeight > rcBBox.top)
					{			
						rcIcon = CPDF_Rect(0,0,0,0);				
						rcLabel = rcBBox;
					}
					else
					{
						rcLabel = CPDF_Rect(rcBBox.left,rcBBox.top - fHeight,rcBBox.right,rcBBox.top);
						rcIcon = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.right,rcLabel.bottom);		
					}
				}
			}
			else
			{
				rcLabel = rcBBox;
				rcIcon = CPDF_Rect(0,0,0,0);
			}

			break;
		case PPBL_ICONLEFTLABELRIGHT:

			if (pIconStream)
			{
				if (IsFloatZero(fFontSize))
				{
					fWidth = rcBBox.right - rcBBox.left;
					rcLabel = CPDF_Rect(rcBBox.right - fWidth * fAutoFontScale,rcBBox.bottom,rcBBox.right,rcBBox.top);
					rcIcon = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcLabel.left,rcBBox.top);

					if (rcLabelContent.Width() < fWidth * fAutoFontScale)
					{							
					}
					else 
					{
						if (rcLabelContent.Width() < fWidth)
						{
							rcLabel = CPDF_Rect(rcBBox.right - rcLabelContent.Width(),rcBBox.bottom,rcBBox.right,rcBBox.top);
							rcIcon = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcLabel.left,rcBBox.top);
						}
						else
						{
							rcLabel = rcBBox;
							rcIcon = CPDF_Rect(0,0,0,0);
						}
					}
				}
				else
				{
					fWidth = rcLabelContent.Width();

					if (rcBBox.left + fWidth > rcBBox.right)
					{
						rcLabel = rcBBox;
						rcIcon = CPDF_Rect(0,0,0,0);
					}
					else
					{
						rcLabel = CPDF_Rect(rcBBox.right - fWidth,rcBBox.bottom,rcBBox.right,rcBBox.top);
						rcIcon = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcLabel.left,rcBBox.top);			
					}
				}
			}
			else
			{
				rcLabel = rcBBox;
				rcIcon = CPDF_Rect(0,0,0,0);
			}

			break;
		case PPBL_LABELLEFTICONRIGHT:

			if (pIconStream)
			{
				if (IsFloatZero(fFontSize))
				{
					fWidth = rcBBox.right - rcBBox.left;
					rcLabel = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.left + fWidth * fAutoFontScale,rcBBox.top);
					rcIcon = CPDF_Rect(rcLabel.right,rcBBox.bottom,rcBBox.right,rcBBox.top);

					if (rcLabelContent.Width() < fWidth * fAutoFontScale)
					{							
					}
					else 
					{
						if (rcLabelContent.Width() < fWidth)
						{
							rcLabel = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.left + rcLabelContent.Width(),rcBBox.top);
							rcIcon = CPDF_Rect(rcLabel.right,rcBBox.bottom,rcBBox.right,rcBBox.top);
						}
						else
						{
							rcLabel = rcBBox;
							rcIcon = CPDF_Rect(0,0,0,0);
						}
					}
				}
				else
				{
					fWidth = rcLabelContent.Width();

					if (rcBBox.left + fWidth > rcBBox.right)
					{
						rcLabel = rcBBox;
						rcIcon = CPDF_Rect(0,0,0,0);
					}
					else
					{
						rcLabel = CPDF_Rect(rcBBox.left,rcBBox.bottom,rcBBox.left + fWidth,rcBBox.top);
						rcIcon = CPDF_Rect(rcLabel.right,rcBBox.bottom,rcBBox.right,rcBBox.top);		
					}	
				}
			}
			else
			{
				rcLabel = rcBBox;
				rcIcon = CPDF_Rect(0,0,0,0);
			}

			break;
		case PPBL_LABELOVERICON:
			rcLabel = rcBBox;
			rcIcon = rcBBox;
			break;
		}

		CFX_ByteTextBuf sAppStream,sTemp;

		if (!rcIcon.IsEmpty())
		{
			Icon.Move(rcIcon, FALSE, FALSE);
			sTemp << Icon.GetImageAppStream();
		}

		Icon.Destroy();

		if (!rcLabel.IsEmpty())
		{
			pEdit->SetPlateRect(rcLabel);
			CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(pEdit,CPDF_Point(0.0f,0.0f));
			if (sEdit.GetLength() > 0)
			{
				sTemp << "BT\n" << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n";
			}
		}

		IFX_Edit::DelEdit(pEdit);

		if (sTemp.GetSize() > 0)
		{
			sAppStream << "q\n" << rcBBox.left << " " << rcBBox.bottom << " "
				<< rcBBox.right - rcBBox.left << " " << rcBBox.top - rcBBox.bottom << " re W n\n";
			sAppStream << sTemp << "Q\n";
		}

		return sAppStream.GetByteString();
	}

	return "";
}

CFX_ByteString CPWL_Utils::GetColorAppStream(const CPWL_Color & color,const FX_BOOL & bFillOrStroke)
{
	CFX_ByteTextBuf sColorStream;

	switch (color.nColorType)
	{		
	case COLORTYPE_RGB:
		sColorStream << color.fColor1 << " " << color.fColor2 << " " << color.fColor3 << " " 
			<< (bFillOrStroke ? "rg" : "RG") << "\n";
		break;
	case COLORTYPE_GRAY:
		sColorStream << color.fColor1 << " " << (bFillOrStroke ? "g" : "G") << "\n";
		break;
	case COLORTYPE_CMYK:
		sColorStream << color.fColor1 << " " << color.fColor2 << " " << color.fColor3 << " " << color.fColor4 << " " 
			<< (bFillOrStroke ? "k" : "K") << "\n";
		break;
	}

	return sColorStream.GetByteString();
}

CFX_ByteString CPWL_Utils::GetBorderAppStream(const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash)
{
	CFX_ByteTextBuf sAppStream;
	CFX_ByteString sColor;

	FX_FLOAT fLeft = rect.left;
	FX_FLOAT fRight = rect.right;
	FX_FLOAT fTop = rect.top;
	FX_FLOAT fBottom = rect.bottom;

	if (fWidth > 0.0f)
	{
		FX_FLOAT fHalfWidth = fWidth / 2.0f;

		sAppStream << "q\n";

		switch (nStyle)
		{
		default:
		case PBS_SOLID:
			sColor = CPWL_Utils::GetColorAppStream(color,TRUE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " " << fTop - fBottom << " re\n";
				sAppStream << fLeft + fWidth << " " << fBottom + fWidth << " " 
					<< fRight - fLeft - fWidth * 2 << " " << fTop - fBottom - fWidth * 2 << " re\n";
				sAppStream << "f*\n";
			}
			break;
		case PBS_DASH:
			sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fWidth << " w" << " [" << dash.nDash << " " << dash.nGap << "] " << dash.nPhase << " d\n";
				sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2 << " m\n";
				sAppStream << fLeft + fWidth / 2 << " " << fTop - fWidth / 2 << " l\n";
				sAppStream << fRight - fWidth / 2 << " " << fTop - fWidth / 2 << " l\n";
				sAppStream << fRight - fWidth / 2 << " " << fBottom + fWidth / 2 << " l\n";
				sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2 << " l S\n";
			}
			break;
		case PBS_BEVELED:
		case PBS_INSET:
			sColor = CPWL_Utils::GetColorAppStream(crLeftTop,TRUE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " m\n";
				sAppStream << fLeft + fHalfWidth << " " << fTop - fHalfWidth << " l\n";
				sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth << " l\n";
				sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l\n";
				sAppStream << fLeft + fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l\n";
				sAppStream << fLeft + fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l f\n";
			}

			sColor = CPWL_Utils::GetColorAppStream(crRightBottom,TRUE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fRight - fHalfWidth << " " <<	fTop - fHalfWidth << " m\n";
				sAppStream << fRight - fHalfWidth << " " <<	fBottom + fHalfWidth << " l\n";
				sAppStream << fLeft + fHalfWidth << " " << 	fBottom + fHalfWidth << " l\n";
				sAppStream << fLeft + fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l\n";
				sAppStream << fRight - fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l\n";
				sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l f\n";
			}

			sColor = CPWL_Utils::GetColorAppStream(color,TRUE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fLeft << " " << fBottom << " " <<	fRight - fLeft << " " << fTop - fBottom << " re\n";
				sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " " 
					<< fRight - fLeft - fHalfWidth * 2 << " " << fTop - fBottom - fHalfWidth * 2 << " re f*\n";						
			}
			break;
		case PBS_UNDERLINED:
			sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
			if (sColor.GetLength() > 0)
			{
				sAppStream << sColor;
				sAppStream << fWidth << " w\n";
				sAppStream << fLeft << " " << fBottom + fWidth / 2 << " m\n";
				sAppStream << fRight << " " << fBottom + fWidth / 2 << " l S\n";
			}
			break;
		}
		
		sAppStream << "Q\n";
	}

	return sAppStream.GetByteString();
}

CFX_ByteString CPWL_Utils::GetCircleBorderAppStream(const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash)
{
	CFX_ByteTextBuf sAppStream;
	CFX_ByteString sColor;

	




	if (fWidth > 0.0f)
	{
		sAppStream << "q\n";

		switch (nStyle)
		{
		default:
		case PBS_SOLID:
		case PBS_UNDERLINED:
			{
				sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fWidth << " w\n" << sColor 
						<< CPWL_Utils::GetAP_Circle(CPWL_Utils::DeflateRect(rect,fWidth / 2.0f))
						<< " S\nQ\n";
				}
			}
			break;
		case PBS_DASH:
			{
				sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fWidth << " w\n" 
						<< "[" << dash.nDash << " " << dash.nGap << "] " << dash.nPhase << " d\n" 
						<< sColor << CPWL_Utils::GetAP_Circle(CPWL_Utils::DeflateRect(rect,fWidth / 2.0f))
						<< " S\nQ\n";
				}						
			}
			break;
		case PBS_BEVELED:
			{
				FX_FLOAT fHalfWidth = fWidth / 2.0f;

				sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_Circle(rect)
						<< " S\nQ\n";
				}					

				sColor = CPWL_Utils::GetColorAppStream(crLeftTop,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_HalfCircle(CPWL_Utils::DeflateRect(rect,fHalfWidth * 0.75f),PWL_PI/4.0f)
						<< " S\nQ\n";
				}	

				sColor = CPWL_Utils::GetColorAppStream(crRightBottom,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_HalfCircle(CPWL_Utils::DeflateRect(rect,fHalfWidth * 0.75f),PWL_PI*5/4.0f)
						<< " S\nQ\n";
				}	
			}
			break;
		case PBS_INSET:
			{
				FX_FLOAT fHalfWidth = fWidth / 2.0f;

				sColor = CPWL_Utils::GetColorAppStream(color,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_Circle(rect)
						<< " S\nQ\n";
				}

				sColor = CPWL_Utils::GetColorAppStream(crLeftTop,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_HalfCircle(CPWL_Utils::DeflateRect(rect,fHalfWidth * 0.75f),PWL_PI/4.0f)
						<< " S\nQ\n";
				}	

				sColor = CPWL_Utils::GetColorAppStream(crRightBottom,FALSE);
				if (sColor.GetLength() > 0)
				{
					sAppStream << "q\n" << fHalfWidth << " w\n"
						<< sColor << CPWL_Utils::GetAP_HalfCircle(CPWL_Utils::DeflateRect(rect,fHalfWidth * 0.75f),PWL_PI*5/4.0f)
						<< " S\nQ\n";
				}						
			}
			break;
		}
		
		sAppStream << "Q\n";
	}

	return sAppStream.GetByteString();
}

CPWL_Color CPWL_Utils::SubstractColor(const CPWL_Color & sColor,FX_FLOAT fColorSub)
{
	CPWL_Color sRet;
	sRet.nColorType = sColor.nColorType;

	switch (sColor.nColorType)
	{
	case COLORTYPE_TRANSPARENT:
		sRet.nColorType = COLORTYPE_RGB;
		sRet.fColor1 = PWL_MAX(1 - fColorSub,0.0f);
		sRet.fColor2 = PWL_MAX(1 - fColorSub,0.0f);
		sRet.fColor3 = PWL_MAX(1 - fColorSub,0.0f);
		break;
	case COLORTYPE_RGB:
	case COLORTYPE_GRAY:
	case COLORTYPE_CMYK:
		sRet.fColor1 = PWL_MAX(sColor.fColor1 - fColorSub,0.0f);
		sRet.fColor2 = PWL_MAX(sColor.fColor2 - fColorSub,0.0f);
		sRet.fColor3 = PWL_MAX(sColor.fColor3 - fColorSub,0.0f);
		sRet.fColor4 = PWL_MAX(sColor.fColor4 - fColorSub,0.0f);
		break;
	}	

	return sRet;
}

CPWL_Color CPWL_Utils::DevideColor(const CPWL_Color & sColor,FX_FLOAT fColorDevide)
{
	CPWL_Color sRet;
	sRet.nColorType = sColor.nColorType;

	switch (sColor.nColorType)
	{
	case COLORTYPE_TRANSPARENT:
		sRet.nColorType = COLORTYPE_RGB;
		sRet.fColor1 = 1 / fColorDevide;
		sRet.fColor2 = 1 / fColorDevide;
		sRet.fColor3 = 1 / fColorDevide;
		break;
	case COLORTYPE_RGB:
	case COLORTYPE_GRAY:
	case COLORTYPE_CMYK:
		sRet = sColor;
		sRet.fColor1 /= fColorDevide;
		sRet.fColor2 /= fColorDevide;
		sRet.fColor3 /= fColorDevide;
		sRet.fColor4 /= fColorDevide;
		break;
	}	

	return sRet;
}

CFX_ByteString CPWL_Utils::GetAppStream_Check(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << CPWL_Utils::GetAP_Check(rcBBox) << "f\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAppStream_Circle(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << CPWL_Utils::GetAP_Circle(rcBBox) << "f\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAppStream_Cross(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n" << CPWL_Utils::GetColorAppStream(crText,FALSE) << CPWL_Utils::GetAP_Cross(rcBBox) << "S\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAppStream_Diamond(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n1 w\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << CPWL_Utils::GetAP_Diamond(rcBBox) << "f\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAppStream_Square(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << CPWL_Utils::GetAP_Square(rcBBox) << "f\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetAppStream_Star(const CPDF_Rect & rcBBox, const CPWL_Color & crText)
{
	CFX_ByteTextBuf sAP;
	sAP << "q\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << CPWL_Utils::GetAP_Star(rcBBox) << "f\nQ\n";
	return sAP.GetByteString();
}

CFX_ByteString CPWL_Utils::GetCheckBoxAppStream(const CPDF_Rect & rcBBox,
													FX_INT32 nStyle,
													const CPWL_Color & crText)
{
	CPDF_Rect rcCenter = GetCenterSquare(rcBBox);
	switch (nStyle)
	{
	default:
	case PCS_CHECK:
		return GetAppStream_Check(rcCenter,crText);
	case PCS_CIRCLE:
		return GetAppStream_Circle(ScaleRect(rcCenter,2.0f/3.0f),crText);
	case PCS_CROSS:
		return GetAppStream_Cross(rcCenter,crText);
	case PCS_DIAMOND:
		return GetAppStream_Diamond(ScaleRect(rcCenter,2.0f/3.0f),crText);		
	case PCS_SQUARE:
		return GetAppStream_Square(ScaleRect(rcCenter,2.0f/3.0f),crText);
	case PCS_STAR:
		return GetAppStream_Star(ScaleRect(rcCenter,2.0f/3.0f),crText);
	}
}

CFX_ByteString CPWL_Utils::GetRadioButtonAppStream(const CPDF_Rect & rcBBox,
													FX_INT32 nStyle,
													const CPWL_Color & crText)
{
	CPDF_Rect rcCenter = GetCenterSquare(rcBBox);
	switch (nStyle)
	{
	default:
	case PCS_CHECK:
		return GetAppStream_Check(rcCenter,crText);
	case PCS_CIRCLE:
		return GetAppStream_Circle(ScaleRect(rcCenter,1.0f/2.0f),crText);
	case PCS_CROSS:
		return GetAppStream_Cross(rcCenter,crText);
	case PCS_DIAMOND:
		return GetAppStream_Diamond(ScaleRect(rcCenter,2.0f/3.0f),crText);		
	case PCS_SQUARE:
		return GetAppStream_Square(ScaleRect(rcCenter,2.0f/3.0f),crText);
	case PCS_STAR:
		return GetAppStream_Star(ScaleRect(rcCenter,2.0f/3.0f),crText);
	}
}

CFX_ByteString CPWL_Utils::GetDropButtonAppStream(const CPDF_Rect & rcBBox)
{
	CFX_ByteTextBuf sAppStream;

	if (!rcBBox.IsEmpty())
	{
		sAppStream << "q\n" << CPWL_Utils::GetColorAppStream(CPWL_Color(COLORTYPE_RGB,220.0f/255.0f,220.0f/255.0f,220.0f/255.0f),TRUE);
		sAppStream << rcBBox.left << " " << rcBBox.bottom << " " 
				<< rcBBox.right - rcBBox.left << " " << rcBBox.top - rcBBox.bottom << " re f\n";
		sAppStream << "Q\n";

		sAppStream << "q\n" << 
			CPWL_Utils::GetBorderAppStream(rcBBox,2,CPWL_Color(COLORTYPE_GRAY,0),CPWL_Color(COLORTYPE_GRAY,1),CPWL_Color(COLORTYPE_GRAY,0.5),PBS_BEVELED,CPWL_Dash(3,0,0)) 
			<< "Q\n";

		CPDF_Point ptCenter = CPDF_Point((rcBBox.left + rcBBox.right)/2,(rcBBox.top + rcBBox.bottom)/2);
		if (IsFloatBigger(rcBBox.right - rcBBox.left,6) && IsFloatBigger(rcBBox.top - rcBBox.bottom,6))
		{
			sAppStream << "q\n" << " 0 g\n";
			sAppStream << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " m\n";
			sAppStream << ptCenter.x + 3 << " " << ptCenter.y + 1.5f << " l\n";
			sAppStream << ptCenter.x << " " << ptCenter.y - 1.5f << " l\n";
			sAppStream << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " l f\n";
			sAppStream << "Q\n";
		}
	}

	return sAppStream.GetByteString();
}

void CPWL_Utils::ConvertCMYK2GRAY(FX_FLOAT dC,FX_FLOAT dM,FX_FLOAT dY,FX_FLOAT dK,FX_FLOAT &dGray)
{
	if (dC<0 || dC>1 || dM<0 || dM>1 || dY < 0 || dY >1 || dK < 0 || dK >1)
		return;
	dGray = 1.0f - FX_MIN(1.0f,0.3f*dC+0.59f * dM + 0.11f*dY+dK);
}

void CPWL_Utils::ConvertGRAY2CMYK(FX_FLOAT dGray,FX_FLOAT  &dC,FX_FLOAT &dM,FX_FLOAT &dY,FX_FLOAT &dK)
{
	if (dGray <0 || dGray >1)
		return;
	dC = 0.0f;
	dM = 0.0f;
	dY = 0.0f;
	dK = 1.0f-dGray;
}

void CPWL_Utils::ConvertGRAY2RGB(FX_FLOAT dGray,FX_FLOAT &dR,FX_FLOAT &dG,FX_FLOAT &dB)
{
	if (dGray <0 || dGray >1)
		return;
	dR = dGray;
	dG = dGray;
	dB = dGray;
}

void CPWL_Utils::ConvertRGB2GRAY(FX_FLOAT dR,FX_FLOAT dG,FX_FLOAT dB,FX_FLOAT &dGray)
{
	if (dR<0 || dR>1 || dG<0 || dG > 0 || dB < 0 || dB >1)
		return;
	dGray = 0.3f*dR+0.59f*dG+0.11f*dB;
}

void CPWL_Utils::ConvertCMYK2RGB(FX_FLOAT dC,FX_FLOAT dM,FX_FLOAT dY,FX_FLOAT dK,FX_FLOAT &dR,FX_FLOAT &dG,FX_FLOAT &dB)
{
	if (dC <0 || dC>1 || dM < 0 || dM > 1 || dY < 0 || dY > 1 || dK < 0 || dK > 1 )
		return;
	dR = 1.0f - FX_MIN(1.0f, dC + dK);
	dG = 1.0f - FX_MIN(1.0f, dM + dK);
	dB = 1.0f - FX_MIN(1.0f, dY + dK);
}

void CPWL_Utils::ConvertRGB2CMYK(FX_FLOAT dR,FX_FLOAT dG,FX_FLOAT dB,FX_FLOAT &dC,FX_FLOAT &dM,FX_FLOAT &dY,FX_FLOAT &dK)
{
	if (dR<0 || dR>1 || dG<0 || dG>1 || dB<0 || dB>1)
		return;

	dC = 1.0f - dR;
	dM = 1.0f - dG;
	dY = 1.0f - dB;
	dK = FX_MIN(dC, FX_MIN(dM, dY));
}

void CPWL_Utils::PWLColorToARGB(const CPWL_Color& color, FX_INT32& alpha, FX_FLOAT& red, FX_FLOAT& green, FX_FLOAT& blue)
{
	switch (color.nColorType)
	{
	case COLORTYPE_TRANSPARENT:
		{
			alpha = 0;
		}
		break;
	case COLORTYPE_GRAY:
		{
			ConvertGRAY2RGB(color.fColor1, red, green, blue);
		}
		break;
	case COLORTYPE_RGB:
		{
			red = color.fColor1;
			green = color.fColor2;
			blue = color.fColor3;
		}
		break;
	case COLORTYPE_CMYK:
		{
			ConvertCMYK2RGB(color.fColor1, color.fColor2, color.fColor3, color.fColor4,
				red, green, blue);
		}
		break;
	}
}

FX_COLORREF CPWL_Utils::PWLColorToFXColor(const CPWL_Color& color, FX_INT32 nTransparancy)
{
	FX_INT32 nAlpha = nTransparancy;
	FX_FLOAT dRed = 0;
	FX_FLOAT dGreen = 0;
	FX_FLOAT dBlue = 0;

	PWLColorToARGB(color, nAlpha, dRed, dGreen, dBlue);

	return ArgbEncode(nAlpha, (FX_INT32)(dRed*255), (FX_INT32)(dGreen*255), (FX_INT32)(dBlue*255));
}

void CPWL_Utils::DrawFillRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,const CPDF_Rect & rect,
							  const FX_COLORREF & color)
{
	CFX_PathData path;
	CPDF_Rect rcTemp(rect);	
	path.AppendRect(rcTemp.left,rcTemp.bottom,rcTemp.right,rcTemp.top);
	pDevice->DrawPath(&path, pUser2Device, NULL, color, 0, FXFILL_WINDING);
}

void CPWL_Utils::DrawFillArea(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
							const CPDF_Point* pPts, FX_INT32 nCount, const FX_COLORREF& color)
{
	CFX_PathData path;
	path.SetPointCount(nCount);

	path.SetPoint(0, pPts[0].x, pPts[0].y, FXPT_MOVETO);
	for (FX_INT32 i=1; i<nCount; i++)
		path.SetPoint(i, pPts[i].x, pPts[i].y, FXPT_LINETO);

	pDevice->DrawPath(&path, pUser2Device, NULL, color, 0, FXFILL_ALTERNATE);
}

void CPWL_Utils::DrawStrokeRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,const CPDF_Rect & rect,
							  const FX_COLORREF & color, FX_FLOAT fWidth)
{
	CFX_PathData path;
	CPDF_Rect rcTemp(rect);	
	path.AppendRect(rcTemp.left,rcTemp.bottom,rcTemp.right,rcTemp.top);
	
	CFX_GraphStateData gsd;
	gsd.m_LineWidth = fWidth;

	pDevice->DrawPath(&path, pUser2Device, &gsd, 0, color, FXFILL_ALTERNATE);
}

void CPWL_Utils::DrawStrokeLine(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
							const CPDF_Point & ptMoveTo, const CPDF_Point & ptLineTo, const FX_COLORREF & color, FX_FLOAT fWidth)
{
	CFX_PathData path;
	path.SetPointCount(2);
	path.SetPoint(0, ptMoveTo.x, ptMoveTo.y, FXPT_MOVETO);
	path.SetPoint(1, ptLineTo.x, ptLineTo.y, FXPT_LINETO);

	CFX_GraphStateData gsd;
	gsd.m_LineWidth = fWidth;

	pDevice->DrawPath(&path, pUser2Device, &gsd, 0, color, FXFILL_ALTERNATE);
}

void CPWL_Utils::DrawFillRect(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,const CPDF_Rect & rect,
							  const CPWL_Color & color, FX_INT32 nTransparancy)
{
	CPWL_Utils::DrawFillRect(pDevice,pUser2Device,rect,PWLColorToFXColor(color,nTransparancy));
}

void CPWL_Utils::DrawShadow(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
														FX_BOOL bVertical, FX_BOOL bHorizontal, CPDF_Rect rect,
														FX_INT32 nTransparancy, FX_INT32 nStartGray, FX_INT32 nEndGray)
{
	FX_FLOAT fStepGray = 1.0f;

	if (bVertical)
	{
		fStepGray = (nEndGray - nStartGray) / rect.Height();

		for (FX_FLOAT fy=rect.bottom+0.5f; fy<=rect.top-0.5f; fy+=1.0f)
		{			
			FX_INT32 nGray = nStartGray + (FX_INT32)(fStepGray * (fy-rect.bottom));
			CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, CPDF_Point(rect.left, fy),
				CPDF_Point(rect.right, fy), ArgbEncode(nTransparancy, nGray, nGray, nGray), 1.5f);
		}
	}

	if (bHorizontal)
	{
		fStepGray = (nEndGray - nStartGray) / rect.Width();

		for (FX_FLOAT fx=rect.left+0.5f; fx<=rect.right-0.5f; fx+=1.0f)
		{			
			FX_INT32 nGray = nStartGray + (FX_INT32)(fStepGray * (fx-rect.left));
			CPWL_Utils::DrawStrokeLine(pDevice, pUser2Device, CPDF_Point(fx, rect.bottom),
				CPDF_Point(fx, rect.top), ArgbEncode(nTransparancy, nGray, nGray, nGray), 1.5f);
		}
	}
}

void CPWL_Utils::DrawBorder(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
												const CPDF_Rect & rect, FX_FLOAT fWidth,
												const CPWL_Color & color, const CPWL_Color & crLeftTop, const CPWL_Color & crRightBottom,
												FX_INT32 nStyle, const CPWL_Dash & dash, FX_INT32 nTransparancy)
{
	FX_FLOAT fLeft = rect.left;
	FX_FLOAT fRight = rect.right;
	FX_FLOAT fTop = rect.top;
	FX_FLOAT fBottom = rect.bottom;

	if (fWidth > 0.0f)
	{
		FX_FLOAT fHalfWidth = fWidth / 2.0f;

		switch (nStyle)
		{
		default:
		case PBS_SOLID:
			{
				CFX_PathData path;
				path.AppendRect(fLeft, fBottom, fRight, fTop);
				path.AppendRect(fLeft + fWidth, fBottom + fWidth, fRight - fWidth, fTop - fWidth);
				pDevice->DrawPath(&path, pUser2Device, NULL, PWLColorToFXColor(color,nTransparancy), 0, FXFILL_ALTERNATE);
			}
			break;
		case PBS_DASH:
			{
				CFX_PathData path;

				path.SetPointCount(5);
				path.SetPoint(0, fLeft + fWidth / 2.0f, fBottom + fWidth / 2.0f, FXPT_MOVETO);
				path.SetPoint(1, fLeft + fWidth / 2.0f, fTop - fWidth / 2.0f, FXPT_LINETO);
				path.SetPoint(2, fRight - fWidth / 2.0f, fTop - fWidth / 2.0f, FXPT_LINETO);
				path.SetPoint(3, fRight - fWidth / 2.0f, fBottom + fWidth / 2.0f, FXPT_LINETO);
				path.SetPoint(4, fLeft + fWidth / 2.0f, fBottom + fWidth / 2.0f, FXPT_LINETO);

				CFX_GraphStateData gsd;
				gsd.SetDashCount(2);				
				gsd.m_DashArray[0] = 3.0f;
				gsd.m_DashArray[1] = 3.0f;
				gsd.m_DashPhase = 0;	
				
				gsd.m_LineWidth = fWidth;
				pDevice->DrawPath(&path, pUser2Device, &gsd, 0, PWLColorToFXColor(color,nTransparancy), FXFILL_WINDING);				
			}			
			break;
		case PBS_BEVELED:
		case PBS_INSET:	
			{
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = fHalfWidth;

				CFX_PathData pathLT;

				pathLT.SetPointCount(7);
				pathLT.SetPoint(0, fLeft + fHalfWidth, fBottom + fHalfWidth, FXPT_MOVETO);
				pathLT.SetPoint(1, fLeft + fHalfWidth, fTop - fHalfWidth, FXPT_LINETO);
				pathLT.SetPoint(2, fRight - fHalfWidth, fTop - fHalfWidth, FXPT_LINETO);
				pathLT.SetPoint(3, fRight - fHalfWidth * 2, fTop - fHalfWidth * 2, FXPT_LINETO);
				pathLT.SetPoint(4, fLeft + fHalfWidth * 2, fTop - fHalfWidth * 2, FXPT_LINETO);
				pathLT.SetPoint(5, fLeft + fHalfWidth * 2, fBottom + fHalfWidth * 2, FXPT_LINETO);
				pathLT.SetPoint(6, fLeft + fHalfWidth, fBottom + fHalfWidth, FXPT_LINETO);

				pDevice->DrawPath(&pathLT, pUser2Device, &gsd, PWLColorToFXColor(crLeftTop,nTransparancy), 0, FXFILL_ALTERNATE);

				CFX_PathData pathRB;

				pathRB.SetPointCount(7);
				pathRB.SetPoint(0, fRight - fHalfWidth, fTop - fHalfWidth, FXPT_MOVETO);
				pathRB.SetPoint(1, fRight - fHalfWidth, fBottom + fHalfWidth, FXPT_LINETO);
				pathRB.SetPoint(2, fLeft + fHalfWidth, fBottom + fHalfWidth, FXPT_LINETO);
				pathRB.SetPoint(3, fLeft + fHalfWidth * 2, fBottom + fHalfWidth * 2, FXPT_LINETO);
				pathRB.SetPoint(4, fRight - fHalfWidth * 2, fBottom + fHalfWidth * 2, FXPT_LINETO);
				pathRB.SetPoint(5, fRight - fHalfWidth * 2, fTop - fHalfWidth * 2, FXPT_LINETO);
				pathRB.SetPoint(6, fRight - fHalfWidth, fTop - fHalfWidth, FXPT_LINETO);

				pDevice->DrawPath(&pathRB, pUser2Device, &gsd, PWLColorToFXColor(crRightBottom,nTransparancy), 0, FXFILL_ALTERNATE);

				CFX_PathData path;

				path.AppendRect(fLeft, fBottom, fRight, fTop);
				path.AppendRect(fLeft + fHalfWidth, fBottom + fHalfWidth, fRight - fHalfWidth, fTop - fHalfWidth);

				pDevice->DrawPath(&path, pUser2Device, &gsd, PWLColorToFXColor(color,nTransparancy), 0, FXFILL_ALTERNATE);
			}
			break;
		case PBS_UNDERLINED:
			{
				CFX_PathData path;

				path.SetPointCount(2);
				path.SetPoint(0, fLeft, fBottom + fWidth / 2, FXPT_MOVETO);
				path.SetPoint(1, fRight, fBottom + fWidth / 2, FXPT_LINETO);
				
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = fWidth;

				pDevice->DrawPath(&path, pUser2Device, &gsd,0,  PWLColorToFXColor(color,nTransparancy), FXFILL_ALTERNATE);
			}
			break;
		case PBS_SHADOW:
			{
				CFX_PathData path;
				path.AppendRect(fLeft, fBottom, fRight, fTop);
				path.AppendRect(fLeft + fWidth, fBottom + fWidth, fRight - fWidth, fTop - fWidth);
				pDevice->DrawPath(&path, pUser2Device, NULL, PWLColorToFXColor(color,nTransparancy/2), 0, FXFILL_ALTERNATE);
			}
			break;
		}
	}
}

static void AddSquigglyPath(CFX_PathData & PathData, FX_FLOAT fStartX, FX_FLOAT fEndX, FX_FLOAT fY, FX_FLOAT fStep)
{
	PathData.AddPointCount(1);
	PathData.SetPoint(PathData.GetPointCount() - 1, fStartX, fY, FXPT_MOVETO);

	FX_FLOAT fx;
	FX_INT32 i;

	for (i=1,fx=fStartX+fStep; fx<fEndX; fx+=fStep,i++)
	{
		PathData.AddPointCount(1);
		PathData.SetPoint(PathData.GetPointCount() - 1, fx, fY + (i&1)*fStep, FXPT_LINETO);
	}
}

static void AddSpellCheckObj(CFX_PathData & PathData, IFX_Edit* pEdit, const CPVT_WordRange& wrWord)
{
	FX_FLOAT fStartX = 0.0f;
	FX_FLOAT fEndX = 0.0f;
	FX_FLOAT fY = 0.0f;
	FX_FLOAT fStep = 0.0f; 

	FX_BOOL bBreak = FALSE;

	if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator())
	{
		pIterator->SetAt(wrWord.BeginPos);

		do
		{
			CPVT_WordPlace place = pIterator->GetAt();

			CPVT_Line line;				
			if (pIterator->GetLine(line))
			{
				fY = line.ptLine.y;
				fStep = (line.fLineAscent - line.fLineDescent) / 16.0f;
			}

			if (place.LineCmp(wrWord.BeginPos) == 0)
			{
				pIterator->SetAt(wrWord.BeginPos);
				CPVT_Word word;
				if (pIterator->GetWord(word))
				{
					fStartX = word.ptWord.x;
				}
			}
			else
			{
				fStartX = line.ptLine.x;
			}

			if (place.LineCmp(wrWord.EndPos) == 0)
			{
				pIterator->SetAt(wrWord.EndPos);
				CPVT_Word word;
				if (pIterator->GetWord(word))
				{
					fEndX = word.ptWord.x + word.fWidth;
				}

				bBreak = TRUE;
			}
			else
			{
				fEndX = line.ptLine.x + line.fLineWidth;
			}

			AddSquigglyPath(PathData, fStartX, fEndX, fY, fStep);

			if (bBreak) break;
		}
		while (pIterator->NextLine());
	}
}

void CPWL_Utils::DrawEditSpellCheck(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, 
						const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, 
						IPWL_SpellCheck * pSpellCheck)
{
	const FX_COLORREF crSpell = ArgbEncode(255,255,0,0);

	//for spellcheck
	FX_BOOL bLatinWord = FALSE;
	CPVT_WordPlace wpWordStart;
	CFX_ByteString sLatinWord;

	CFX_PathData pathSpell;

	pDevice->SaveState();

	if (!rcClip.IsEmpty())
	{
		CPDF_Rect rcTemp = rcClip;
		pUser2Device->TransformRect(rcTemp);
		FX_RECT rcDevClip;
		rcDevClip.left = (FX_INT32)rcTemp.left;
		rcDevClip.right = (FX_INT32)rcTemp.right;
		rcDevClip.top = (FX_INT32)rcTemp.top;
		rcDevClip.bottom = (FX_INT32)rcTemp.bottom;
		pDevice->SetClip_Rect(&rcDevClip);
	}

	if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator())
	{
		if (pEdit->GetFontMap())
		{
			if (pRange)
				pIterator->SetAt(pRange->BeginPos);
			else
				pIterator->SetAt(0);

			CPVT_WordPlace oldplace;			

			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();
				if (pRange && place.WordCmp(pRange->EndPos) > 0) break;				

				CPVT_Word word;				
				if (pIterator->GetWord(word))
				{
					if (FX_EDIT_ISLATINWORD(word.Word))
					{
						if (!bLatinWord)
						{
							wpWordStart = place;
							bLatinWord = TRUE;								
						}

						sLatinWord += (char)word.Word;
					}
					else
					{
						if (bLatinWord)
						{
							if (!sLatinWord.IsEmpty())
							{
								if (pSpellCheck && !pSpellCheck->CheckWord(sLatinWord))
								{
									AddSpellCheckObj(pathSpell,pEdit,CPVT_WordRange(wpWordStart,oldplace));									
									pIterator->SetAt(place);
								}
							}
							bLatinWord = FALSE;
						}

						sLatinWord.Empty();
					}
					
					oldplace = place;
				}
				else
				{
					if (bLatinWord)
					{
						if (!sLatinWord.IsEmpty())
						{
							if (pSpellCheck && !pSpellCheck->CheckWord(sLatinWord))
							{
								AddSpellCheckObj(pathSpell,pEdit,CPVT_WordRange(wpWordStart,oldplace));									
								pIterator->SetAt(place);
							}
						}
						bLatinWord = FALSE;
					}

					sLatinWord.Empty();
				}
			}

			if (!sLatinWord.IsEmpty()) 
			{
				if (pSpellCheck && !pSpellCheck->CheckWord(sLatinWord))
				{
					AddSpellCheckObj(pathSpell,pEdit,CPVT_WordRange(wpWordStart,oldplace));	
				}
			}
		}
	}
	
	CFX_GraphStateData gsd;
	gsd.m_LineWidth = 0;
	if (pathSpell.GetPointCount() > 0)
		pDevice->DrawPath(&pathSpell, pUser2Device, &gsd, 0, crSpell, FXFILL_ALTERNATE);

	pDevice->RestoreState();
}

FX_BOOL CPWL_Utils::IsBlackOrWhite(const CPWL_Color& color)
{
	switch (color.nColorType)
	{
	case COLORTYPE_TRANSPARENT:
		return FALSE;
	case COLORTYPE_GRAY:
		return color.fColor1 < 0.5f;
	case COLORTYPE_RGB:
		return color.fColor1 + color.fColor2 + color.fColor3 < 1.5f;
	case COLORTYPE_CMYK:
		return color.fColor1 + color.fColor2 + color.fColor3 + color.fColor4 > 2.0f;
	}

	return TRUE;
}

CPWL_Color CPWL_Utils::GetReverseColor(const CPWL_Color& color)
{
	CPWL_Color crRet = color;

	switch (color.nColorType)
	{
	case COLORTYPE_GRAY:
		crRet.fColor1 = 1.0f - crRet.fColor1;
		break;
	case COLORTYPE_RGB:
		crRet.fColor1 = 1.0f - crRet.fColor1;
		crRet.fColor2 = 1.0f - crRet.fColor2;
		crRet.fColor3 = 1.0f - crRet.fColor3;
		break;
	case COLORTYPE_CMYK:
		crRet.fColor1 = 1.0f - crRet.fColor1;
		crRet.fColor2 = 1.0f - crRet.fColor2;
		crRet.fColor3 = 1.0f - crRet.fColor3;
		crRet.fColor4 = 1.0f - crRet.fColor4;
		break;
	}

	return crRet;
}

CFX_ByteString CPWL_Utils::GetIconAppStream(FX_INT32 nType, const CPDF_Rect& rect, const CPWL_Color& crFill, 
												const CPWL_Color& crStroke)
{
	CFX_ByteString sAppStream = CPWL_Utils::GetColorAppStream(crStroke, FALSE);
	sAppStream += CPWL_Utils::GetColorAppStream(crFill, TRUE);

	CFX_ByteString sPath;
	CFX_PathData path;

	switch (nType)
	{
	case PWL_ICONTYPE_CHECKMARK:
		GetGraphics_Checkmark(sPath, path, rect, PWLPT_STREAM);	
		break;
	case PWL_ICONTYPE_CIRCLE:
		GetGraphics_Circle(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_COMMENT:
		GetGraphics_Comment(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_CROSS:
		GetGraphics_Cross(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_HELP:
		GetGraphics_Help(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_INSERTTEXT:
		GetGraphics_InsertText(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_KEY:
		GetGraphics_Key(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_NEWPARAGRAPH:
		GetGraphics_NewParagraph(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_TEXTNOTE:
		GetGraphics_TextNote(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_PARAGRAPH:
		GetGraphics_Paragraph(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_RIGHTARROW:
		GetGraphics_RightArrow(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_RIGHTPOINTER:
		GetGraphics_RightPointer(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_STAR:
		GetGraphics_Star(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_UPARROW:
		GetGraphics_UpArrow(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_UPLEFTARROW:
		GetGraphics_UpLeftArrow(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_GRAPH:
		GetGraphics_Graph(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_PAPERCLIP:
		GetGraphics_Paperclip(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_ATTACHMENT:
		GetGraphics_Attachment(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_TAG:
		GetGraphics_Tag(sPath, path, rect, PWLPT_STREAM);
		break;
	case PWL_ICONTYPE_FOXIT:
		GetGraphics_Foxit(sPath, path, rect, PWLPT_STREAM);
		break;
	}

	sAppStream += sPath;
	if (crStroke.nColorType != COLORTYPE_TRANSPARENT)
		sAppStream += "B*\n";
	else
		sAppStream += "f*\n";

	return sAppStream;
}

void CPWL_Utils::DrawIconAppStream(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
						FX_INT32 nType, const CPDF_Rect & rect, const CPWL_Color& crFill, const CPWL_Color& crStroke, const FX_INT32 nTransparancy)
{
	CFX_GraphStateData gsd;
	gsd.m_LineWidth = 1.0f;

	CFX_ByteString sPath;
	CFX_PathData path;

	switch (nType)
	{
	case PWL_ICONTYPE_CHECKMARK:
		GetGraphics_Checkmark(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_CIRCLE:
		GetGraphics_Circle(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_COMMENT:
		GetGraphics_Comment(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_CROSS:
		GetGraphics_Cross(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_HELP:
		GetGraphics_Help(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_INSERTTEXT:
		GetGraphics_InsertText(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_KEY:
		GetGraphics_Key(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_NEWPARAGRAPH:
		GetGraphics_NewParagraph(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_TEXTNOTE:
		GetGraphics_TextNote(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_PARAGRAPH:
		GetGraphics_Paragraph(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_RIGHTARROW:
		GetGraphics_RightArrow(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_RIGHTPOINTER:
		GetGraphics_RightPointer(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_STAR:
		GetGraphics_Star(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_UPARROW:
		GetGraphics_UpArrow(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_UPLEFTARROW:
		GetGraphics_UpLeftArrow(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_GRAPH:
		GetGraphics_Graph(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_PAPERCLIP:
		GetGraphics_Paperclip(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_ATTACHMENT:
		GetGraphics_Attachment(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_TAG:
		GetGraphics_Tag(sPath, path, rect, PWLPT_PATHDATA);
		break;
	case PWL_ICONTYPE_FOXIT:
		GetGraphics_Foxit(sPath, path, rect, PWLPT_PATHDATA);
		break;
	default:
		return;
	}

	pDevice->DrawPath(&path, pUser2Device, &gsd, 
		PWLColorToFXColor(crFill,nTransparancy), PWLColorToFXColor(crStroke,nTransparancy), FXFILL_ALTERNATE);
}

void CPWL_Utils::GetGraphics_Checkmark(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth / 15.0f, crBBox.bottom + fHeight * 2 / 5.0f),PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth / 15.0f + PWL_BEZIER*(fWidth / 7.0f - fWidth / 15.0f), 
					  crBBox.bottom + fHeight * 2 / 5.0f + PWL_BEZIER*(fHeight * 2 / 7.0f - fHeight * 2 / 5.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 4.5f + PWL_BEZIER*(fWidth / 5.0f - fWidth / 4.5f),
					  crBBox.bottom + fHeight / 16.0f + PWL_BEZIER*(fHeight / 5.0f - fHeight / 16.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 4.5f, crBBox.bottom + fHeight / 16.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 4.5f + PWL_BEZIER*(fWidth / 4.4f - fWidth / 4.5f),
		              crBBox.bottom + fHeight / 16.0f - PWL_BEZIER*fHeight / 16.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.0f + PWL_BEZIER*(fWidth / 4.0f - fWidth / 3.0f),
		              crBBox.bottom), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.0f, crBBox.bottom), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.0f + PWL_BEZIER*fWidth*(1/7.0f + 2/15.0f),
		              crBBox.bottom + PWL_BEZIER*fHeight * 4 / 5.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth * 14 / 15.0f + PWL_BEZIER*fWidth*(1/7.0f - 7/15.0f),
		              crBBox.bottom + fHeight * 15/16.0f + PWL_BEZIER*(fHeight * 4/5.0f - fHeight * 15/16.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth * 14 / 15.0f,crBBox.bottom + fHeight * 15 / 16.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth * 14 / 15.0f + PWL_BEZIER*(fWidth * 7 / 15.0f - fWidth * 14 / 15.0f),
		              crBBox.bottom + fHeight * 15 / 16.0f + PWL_BEZIER*(fHeight * 8 / 7.0f - fHeight * 15 / 16.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.6f + PWL_BEZIER*(fWidth / 3.4f - fWidth / 3.6f),
		              crBBox.bottom + fHeight / 3.5f + PWL_BEZIER*(fHeight / 3.5f - fHeight / 3.5f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.6f,crBBox.bottom + fHeight / 3.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth / 3.6f,
		              crBBox.bottom + fHeight / 3.5f + PWL_BEZIER*(fHeight / 4.0f - fHeight / 3.5f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth / 15.0f + PWL_BEZIER*(fWidth / 3.5f - fWidth / 15.0f),
		              crBBox.bottom + fHeight * 2 / 5.0f + PWL_BEZIER*(fHeight * 3.5f / 5.0f - fHeight * 2 / 5.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left  + fWidth / 15.0f,crBBox.bottom + fHeight * 2 / 5.0f), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 16);
	else
		GetPathDataFromArray(path, PathArray, 16);
}

void CPWL_Utils::GetGraphics_Circle(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] =
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f,crBBox.bottom + fHeight/2.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f,
		              crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight*14/15.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*(fWidth/2.0f - fWidth/15.0f), crBBox.top - fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f,crBBox.top - fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*(fWidth*14/15.0f - fWidth/2.0f), crBBox.top - fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight*14/15.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.bottom + fHeight/2.0f - PWL_BEZIER*(fHeight/2.0f - fHeight/15.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*(fWidth*14/15.0f - fWidth/2.0f), crBBox.bottom + fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.bottom + fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*(fWidth/2.0f - fWidth/15.0f), crBBox.bottom + fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.bottom + fHeight/2.0f - PWL_BEZIER*(fHeight/2.0f - fHeight/15.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f,crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight*4/5.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*(fWidth/2.0f - fWidth*3/15.0f), crBBox.top - fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*(fWidth*4/5.0f - fWidth/2.0f), crBBox.top - fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight*4/5.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f - PWL_BEZIER*(fHeight*4/5.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*(fWidth*4/5.0f - fWidth/2.0f), crBBox.bottom + fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.bottom + fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*(fWidth*4/5.0f - fWidth/2.0f), crBBox.bottom + fHeight*3/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f - PWL_BEZIER*(fHeight*4/5.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*3/15.0f, crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 26);
	else
		GetPathDataFromArray(path, PathArray, 26);
}

void CPWL_Utils::GetGraphics_Comment(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.top - fHeight/6.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.top - fHeight/6.0f + PWL_BEZIER*(fHeight/6.0f - fHeight/10.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f - PWL_BEZIER*fWidth/15.0f, crBBox.top - fHeight/10.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f, crBBox.top - fHeight/10.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f, crBBox.top - fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f + PWL_BEZIER*fWidth/15.0f, crBBox.top - fHeight/10.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.top - fHeight/6 + PWL_BEZIER*(fHeight/6.0f - fHeight/10.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.top - fHeight/6.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.bottom + fHeight/3.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.bottom + fHeight*4/15.0f + PWL_BEZIER*fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f + PWL_BEZIER*fWidth/15.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*5/15.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*5/15.0f, crBBox.bottom + fHeight*2/15 + PWL_BEZIER*fHeight*2/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*5/15.0f - PWL_BEZIER*fWidth*2/15.0f, crBBox.bottom + fHeight*2/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*6/30.0f, crBBox.bottom + fHeight*2/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*7/30.0f + PWL_BEZIER*fWidth/30.0f, crBBox.bottom + fHeight*2/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*7/30.0f, crBBox.bottom + fHeight*2/15.0f + PWL_BEZIER*fHeight*2/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*7/30.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f - PWL_BEZIER*fWidth/15.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.bottom + fHeight/3.0f - PWL_BEZIER*fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.bottom + fHeight/3.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/15.0f, crBBox.top - fHeight/6.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f, crBBox.top - fHeight*8/30.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f, crBBox.top - fHeight*8/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15, crBBox.top - fHeight*25/60.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*2/15.0f, crBBox.top - fHeight*25/60.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*2/15.0f, crBBox.top - fHeight*17/30.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*4/15.0f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 30);
	else
		GetPathDataFromArray(path, PathArray, 30);
}

void CPWL_Utils::GetGraphics_Cross(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;
	//FX_FLOAT fcatercorner = (FX_FLOAT)sqrt(fWidth*fWidth + fHeight*fHeight);
	CPWL_Point center_point(crBBox.left + fWidth/2, crBBox.bottom + fHeight/2);

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(center_point.x, center_point.y + fHeight/10.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(center_point.x + fWidth*0.3f, center_point.y + fHeight/10.0f + fWidth*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x + fWidth/10.0f + fWidth*0.3f, center_point.y + fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x + fWidth/10.0f, center_point.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x + fWidth/10.0f + fWidth*0.3f, center_point.y - fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x + fWidth*0.3f, center_point.y - fHeight/10.0f - fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x, center_point.y - fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x - fWidth*0.3f, center_point.y - fHeight/10 - fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x - fWidth/10.0f - fWidth*0.3f, center_point.y - fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x - fWidth/10, center_point.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x - fWidth/10 - fWidth*0.3f, center_point.y + fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x - fWidth*0.3f, center_point.y + fHeight/10.0f + fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(center_point.x, center_point.y + fHeight/10.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 13);
	else
		GetPathDataFromArray(path, PathArray, 13);
}

void CPWL_Utils::GetGraphics_Help(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60.0f, crBBox.bottom + fHeight/2.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight/60.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*(fWidth/2.0f - fWidth/60.0f), crBBox.bottom + fHeight/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.bottom + fHeight/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*fWidth*29/60.0f, crBBox.bottom + fHeight/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/60.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*(fHeight/60.0f - fHeight/2.0f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/60.0f, crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/60.0f, crBBox.bottom + fHeight/2.0f + PWL_BEZIER*fHeight*29/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + PWL_BEZIER*fWidth*29/60.0f, crBBox.top - fHeight/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/60.0f), PWLPT_BEZIERTO),		
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f - PWL_BEZIER*fWidth*29/60.0f, crBBox.top - fHeight/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60.0f, crBBox.bottom + fHeight/2.0f  + PWL_BEZIER*fHeight*29/60.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60.0f, crBBox.bottom + fHeight/2.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.27f, crBBox.top - fHeight*0.36f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.27f, crBBox.top - fHeight*0.36f + PWL_BEZIER*fHeight*0.23f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f - PWL_BEZIER*fWidth*0.23f, crBBox.bottom + fHeight*0.87f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.bottom + fHeight*0.87f), PWLPT_BEZIERTO),		
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f + PWL_BEZIER*fWidth*0.23f, crBBox.bottom + fHeight*0.87f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.27f, crBBox.top - fHeight*0.36f + PWL_BEZIER*fHeight*0.23f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.27f, crBBox.top - fHeight*0.36f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.27f - fWidth*0.08f*0.2f, crBBox.top - fHeight*0.36f - fHeight*0.15f*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.35f + fWidth*0.08f*0.2f, crBBox.top - fHeight*0.51f + fHeight*0.15f*0.2f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.35f, crBBox.top - fHeight*0.51f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.35f - fWidth*0.1f*0.5f, crBBox.top - fHeight*0.51f - fHeight*0.15f*0.3f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f - fWidth*0.1f*0.5f, crBBox.top - fHeight*0.68f + fHeight*0.15f*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f, crBBox.top - fHeight*0.68f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f, crBBox.bottom + fHeight*0.30f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f, crBBox.bottom + fHeight*0.30f - fWidth*0.1f*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.55f, crBBox.bottom + fHeight*0.30f - fWidth*0.1f*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.55f, crBBox.bottom + fHeight*0.30f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.55f, crBBox.top - fHeight*0.66f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.55f - fWidth*0.1f*0.05f, crBBox.top - fHeight*0.66f + fHeight*0.18f*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f - fWidth*0.1f*0.05f, crBBox.top - fHeight*0.48f - fHeight*0.18f*0.3f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f, crBBox.top - fHeight*0.48f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.45f + fWidth*0.08f*0.2f, crBBox.top - fHeight*0.48f + fHeight*0.18f*0.2f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.37f - fWidth*0.08f*0.2f, crBBox.top - fHeight*0.36f - fHeight*0.18f*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.37f, crBBox.top - fHeight*0.36f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.37f, crBBox.top - fHeight*0.36f + PWL_BEZIER*fHeight*0.13f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f + PWL_BEZIER*fWidth*0.13f, crBBox.bottom + fHeight*0.77f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.bottom + fHeight*0.77f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f - PWL_BEZIER*fWidth*0.13f, crBBox.bottom + fHeight*0.77f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.37f, crBBox.top - fHeight*0.36f + PWL_BEZIER*fHeight*0.13f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.37f, crBBox.top - fHeight*0.36f), PWLPT_BEZIERTO),		
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.37f, crBBox.top - fHeight*0.36f - fWidth*0.1f*0.6f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.27f, crBBox.top - fHeight*0.36f - fWidth*0.1f*0.6f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.27f, crBBox.top - fHeight*0.36f), PWLPT_BEZIERTO),		
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.56f, crBBox.bottom + fHeight*0.13f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.56f, crBBox.bottom + fHeight*0.13f + PWL_BEZIER*fHeight*0.055f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f - PWL_BEZIER*fWidth*0.095f, crBBox.bottom + fHeight*0.185f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f, crBBox.bottom + fHeight*0.185f), PWLPT_BEZIERTO),		
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f + PWL_BEZIER*fWidth*0.065f, crBBox.bottom + fHeight*0.185f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.44f, crBBox.bottom + fHeight*0.13f + PWL_BEZIER*fHeight*0.055f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.44f, crBBox.bottom + fHeight*0.13f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.44f, crBBox.bottom + fHeight*0.13f - PWL_BEZIER*fHeight*0.055f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f + PWL_BEZIER*fWidth*0.065f, crBBox.bottom + fHeight*0.075f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f, crBBox.bottom + fHeight*0.075f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.505f - PWL_BEZIER*fWidth*0.065f, crBBox.bottom + fHeight*0.075f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.56f, crBBox.bottom + fHeight*0.13f - PWL_BEZIER*fHeight*0.055f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.56f, crBBox.bottom + fHeight*0.13f), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 59);
	else
		GetPathDataFromArray(path, PathArray, 59);
}

void CPWL_Utils::GetGraphics_InsertText(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10, crBBox.bottom + fHeight/10), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2, crBBox.top - fHeight*2/15), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10, crBBox.bottom + fHeight/10), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10, crBBox.bottom + fHeight/10), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 4);
	else
		GetPathDataFromArray(path, PathArray, 4);
}

void CPWL_Utils::GetGraphics_Key(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;
	FX_FLOAT k = -fHeight/fWidth;
	CPWL_Point tail;
	CPWL_Point CicleCenter;
	tail.x = crBBox.left + fWidth*0.9f;
	tail.y = k*(tail.x - crBBox.right) + crBBox.bottom;
	CicleCenter.x = crBBox.left + fWidth*0.15f;
	CicleCenter.y = k*(CicleCenter.x - crBBox.right) + crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30.0f, -fWidth/30.0f/k + tail.y), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30.0f - fWidth*0.18f, -k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f + fWidth*0.07f,
		              -fWidth*0.07f/k - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20 + fWidth*0.07f,
		              -fWidth*0.07f/k - k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20,
		              -k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20 - fWidth/15,
		              -k*fWidth/15 - k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20 - fWidth/15 + fWidth*0.07f,
		              -fWidth*0.07f/k - k*fWidth/15 - k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20 - fWidth/15 - fWidth/20 + fWidth*0.07f,
		              -fWidth*0.07f/k + -k*fWidth/20 + -k*fWidth/15 - k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.18f - fWidth/20 - fWidth/15 - fWidth/20,
		              -k*fWidth/20 + -k*fWidth/15 - k*fWidth/20 - k*fWidth*0.18f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.45f, -k*fWidth*0.45f - fWidth/30/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30 - fWidth*0.45f + fWidth*0.2f,
		              -fWidth*0.4f/k - k*fWidth*0.45f - fWidth/30/k + tail.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.2f, - fWidth*0.1f/k + CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x, CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x - fWidth/60.0f, -k*fWidth/60 + CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x - fWidth/60, -k*fWidth/60 + CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x, CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x - fWidth*0.22f, fWidth*0.35f/k + CicleCenter.y - fHeight*0.05f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(tail.x - fWidth/30 - fWidth*0.45f - fWidth*0.18f, fWidth*0.05f/k - k*fWidth*0.45f + fWidth/30/k + tail.y - fHeight*0.05f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(tail.x - fWidth/30.0f - fWidth*0.45f, -k*fWidth*0.45f + fWidth/30.0f/k + tail.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(tail.x - fWidth/30.0f, fWidth/30.0f/k + tail.y), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/30, -fWidth/30/k + tail.y), PWLPT_LINETO),		
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.08f, k*fWidth*0.08f + CicleCenter.y), PWLPT_MOVETO),
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.08f + fWidth*0.1f, -fWidth*0.1f/k + k*fWidth*0.08f + CicleCenter.y), PWLPT_BEZIERTO),
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.22f + fWidth*0.1f, k*fWidth*0.22f + CicleCenter.y - fWidth*0.1f/k), PWLPT_BEZIERTO),
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.22f, k*fWidth*0.22f + CicleCenter.y), PWLPT_BEZIERTO),
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.22f - fWidth*0.1f, fWidth*0.1f/k + k*fWidth*0.22f + CicleCenter.y), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.08f - fWidth*0.1f, fWidth*0.1f/k + k*fWidth*0.08f + CicleCenter.y), PWLPT_BEZIERTO),
 		CPWL_PathData(CPWL_Point(CicleCenter.x + fWidth*0.08f, k*fWidth*0.08f + CicleCenter.y), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 28);
	else
		GetPathDataFromArray(path, PathArray, 28);
}

void CPWL_Utils::GetGraphics_NewParagraph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/20.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10.0f, crBBox.top - fHeight/2.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.top - fHeight/2.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/20.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.12f, crBBox.top - fHeight*17/30.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.12f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.22f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.22f, crBBox.top - fHeight*17/30.0f - fWidth*0.14f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.38f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.48f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.48f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.38f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.38f, crBBox.bottom + fWidth*0.24f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.22f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.12f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.bottom + fHeight/10.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight/10.0f + fHeight/7.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.97f, crBBox.bottom + fHeight/10.0f + fHeight/7.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.97f, crBBox.top - fHeight*17/30.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.top - fHeight*17/30.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*17/30.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.bottom + fHeight/10.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight/7 + fHeight*0.18f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.85f, crBBox.bottom + fHeight/7 + fHeight*0.18f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.85f, crBBox.top - fHeight*17/30.0f - fHeight*0.08f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.top - fHeight*17/30.0f - fHeight*0.08f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight/7 + fHeight*0.18f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 28);
	else
		GetPathDataFromArray(path, PathArray, 28);
}

void CPWL_Utils::GetGraphics_TextNote(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/10.0f, crBBox.bottom + fHeight/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*7/10.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.top - fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10.0f, crBBox.top - fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10.0f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/10.0f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/10.0f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/10.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.bottom + fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/5.0f, crBBox.top - fHeight*4/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/5.0f, crBBox.top - fHeight*4/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/5.0f, crBBox.top - fHeight*7/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/5.0f, crBBox.top - fHeight*7/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/5.0f, crBBox.top - fHeight*10/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*3/10.0f, crBBox.top - fHeight*10/15.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 17);
	else
		GetPathDataFromArray(path, PathArray, 17);
}

void CPWL_Utils::GetGraphics_Paragraph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] =
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.top - fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.634f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.634f, crBBox.top - fHeight*2/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.566f, crBBox.top - fHeight*2/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.566f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/15.0f - fHeight*0.4f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.2f, crBBox.top - fHeight/15.0f - fHeight*0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.2f, crBBox.top - fHeight/15.0f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/15.0f), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 12);
	else
		GetPathDataFromArray(path, PathArray, 12);
}

void CPWL_Utils::GetGraphics_RightArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.top - fHeight/2.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + fWidth/8.0f, crBBox.bottom + fHeight/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.bottom + fHeight/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f - fWidth*0.15f, crBBox.top - fHeight/2.0f - fWidth/25.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.1f, crBBox.top - fHeight/2.0f - fWidth/25.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.1f, crBBox.top - fHeight/2.0f + fWidth/25.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f - fWidth*0.15f, crBBox.top - fHeight/2.0f + fWidth/25.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f + fWidth/8.0f, crBBox.top - fHeight/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15.0f, crBBox.top - fHeight/2.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 10);
	else
		GetPathDataFromArray(path, PathArray, 10);
}

void CPWL_Utils::GetGraphics_RightPointer(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30.0f, crBBox.top - fHeight/2.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/30.0f, crBBox.bottom + fHeight/6.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*4/15.0f, crBBox.top - fHeight/2.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/30.0f, crBBox.top - fHeight/6.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30.0f, crBBox.top - fHeight/2.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 5);
	else
		GetPathDataFromArray(path, PathArray, 5);
}

void CPWL_Utils::GetGraphics_Star(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fLongRadius = (crBBox.top - crBBox.bottom)/(1+(FX_FLOAT)cos(PWL_PI/5.0f));
	fLongRadius = fLongRadius * 0.7f;
	FX_FLOAT fShortRadius = fLongRadius * 0.55f;
	CPDF_Point ptCenter = CPDF_Point((crBBox.left + crBBox.right) / 2.0f,(crBBox.top + crBBox.bottom) / 2.0f);
	
	FX_FLOAT px1[5], py1[5];
	FX_FLOAT px2[5], py2[5];

	FX_FLOAT fAngel = PWL_PI/10.0f;

	for (FX_INT32 i=0; i<5; i++)
	{
		px1[i] = ptCenter.x + fLongRadius * (FX_FLOAT)cos(fAngel);
		py1[i] = ptCenter.y + fLongRadius * (FX_FLOAT)sin(fAngel);

		fAngel += PWL_PI * 2 / 5.0f;
	}

	fAngel = PWL_PI/5.0f + PWL_PI/10.0f;

	for (FX_INT32 j=0; j<5; j++)
	{
		px2[j] = ptCenter.x + fShortRadius * (FX_FLOAT)cos(fAngel);
		py2[j] = ptCenter.y + fShortRadius * (FX_FLOAT)sin(fAngel);

		fAngel += PWL_PI * 2 / 5.0f;
	}

	CPWL_PathData PathArray[11];
	PathArray[0] = CPWL_PathData(CPWL_Point(px1[0], py1[0]), PWLPT_MOVETO);
	PathArray[1] = CPWL_PathData(CPWL_Point(px2[0], py2[0]), PWLPT_LINETO);

	for(FX_INT32 k = 0; k < 4; k++)
	{
		PathArray[(k+1)*2] = CPWL_PathData(CPWL_Point(px1[k+1], py1[k+1]), PWLPT_LINETO);
		PathArray[(k+1)*2 + 1] = CPWL_PathData(CPWL_Point(px2[k+1], py2[k+1]), PWLPT_LINETO);
	}

	PathArray[10] = CPWL_PathData(CPWL_Point(px1[0], py1[0]), PWLPT_LINETO);

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 11);
	else
		GetPathDataFromArray(path, PathArray, 11);
}

void CPWL_Utils::GetGraphics_UpArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/15.0f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/10.0f, crBBox.top - fWidth*3/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fWidth*3/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.bottom + fHeight/15.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fWidth*3/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/10, crBBox.top - fWidth*3/5.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/2.0f, crBBox.top - fHeight/15.0f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 8);
	else
		GetPathDataFromArray(path, PathArray, 8);
}

void CPWL_Utils::GetGraphics_UpLeftArrow(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;
	CPWL_Point leftup(crBBox.left, crBBox.top);
	CPWL_Point rightdown(crBBox.right, crBBox.bottom);
	FX_FLOAT k = -fHeight/fWidth;
	CPWL_Point tail;
	tail.x = crBBox.left + fWidth*4/5.0f;
	tail.y = k*(tail.x - crBBox.right) + rightdown.y;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/20.0f, k*(crBBox.left + fWidth/20.0f - rightdown.x) + rightdown.y), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(fHeight*17/60.0f/k + tail.x + fWidth/10.0f + fWidth/5.0f,
		              -fWidth/5.0f/k + tail.y - fWidth/10.0f/k + fHeight*17/60.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(fHeight*17/60.0f/k + tail.x + fWidth/10.0f,
		              tail.y - fWidth/10.0f/k + fHeight*17/60.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x + fWidth/10.0f, tail.y - fWidth/10.0f/k), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(tail.x - fWidth/10.0f, tail.y + fWidth/10.0f/k), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(fHeight*17/60.0f/k + tail.x - fWidth/10.0f, tail.y + fWidth/10.0f/k + fHeight*17/60.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(fHeight*17/60.0f/k + tail.x - fWidth/10.0f - fWidth/5.0f,
		              fWidth/5.0f/k + tail.y + fWidth/10.0f/k + fHeight*17/60.0f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/20.0f, k*(crBBox.left + fWidth/20.0f - rightdown.x) + rightdown.y), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 8);
	else
		GetPathDataFromArray(path, PathArray, 8);
}

void CPWL_Utils::GetGraphics_Graph(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.05f, crBBox.top - fWidth*0.15f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.25f, crBBox.top - fHeight*0.15f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.275f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.05f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.05f, crBBox.top - fWidth*0.15f), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.275f, crBBox.top - fWidth*0.45f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.475f, crBBox.top - fWidth*0.45f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.475f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.275f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.275f, crBBox.top - fWidth*0.45f), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.top - fHeight*0.05f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.top - fHeight*0.05f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.7f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.top - fHeight*0.05f), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.725f, crBBox.top - fWidth*0.35f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.925f, crBBox.top - fWidth*0.35f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.925f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.725f, crBBox.bottom + fHeight*0.08f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.725f, crBBox.top - fWidth*0.35f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 20);
	else
		GetPathDataFromArray(path, PathArray, 20);
}

void CPWL_Utils::GetGraphics_Paperclip(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60, crBBox.top - fHeight*0.25f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60, crBBox.bottom + fHeight*0.25f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60, crBBox.bottom + fHeight*0.25f - fWidth*57/60.0f*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30, crBBox.bottom + fHeight*0.25f - fWidth*57/60.0f*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30, crBBox.bottom + fHeight*0.25f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30, crBBox.top - fHeight*0.33f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30, crBBox.top - fHeight*0.33f + fHeight/15*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30 - fWidth*0.12f, crBBox.top - fHeight*0.33f + fHeight/15*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30 - fWidth*0.12f, crBBox.top - fHeight*0.33f), PWLPT_BEZIERTO),
	
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30 - fWidth*0.12f, crBBox.bottom + fHeight*0.2f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/30 - fWidth*0.12f, crBBox.bottom + fHeight*0.2f - (fWidth*57/60.0f - fWidth*0.24f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60 + fWidth*0.12f, crBBox.bottom + fHeight*0.2f - (fWidth*57/60.0f - fWidth*0.24f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60 + fWidth*0.12f, crBBox.bottom + fHeight*0.2f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60 + fWidth*0.12f, crBBox.top - fHeight*0.2f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60 + fWidth*0.12f, crBBox.top - fHeight*0.2f + (fWidth*11/12.0f - fWidth*0.36f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.24f, crBBox.top - fHeight*0.2f + (fWidth*11/12.0f - fWidth*0.36f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.24f, crBBox.top - fHeight*0.2f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.24f, crBBox.bottom + fHeight*0.25f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.24f, crBBox.bottom + fHeight*0.25f - (fWidth*14/15.0f - fWidth*0.53f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.29f, crBBox.bottom + fHeight*0.25f - (fWidth*14/15.0f - fWidth*0.53f)*0.25f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.29f, crBBox.bottom + fHeight*0.25f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.29f, crBBox.top - fHeight*0.33f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.29f, crBBox.top - fHeight*0.33f + fWidth*0.12f*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.17f, crBBox.top - fHeight*0.33f + fWidth*0.12f*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.17f, crBBox.top - fHeight*0.33f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.17f, crBBox.bottom + fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.17f, crBBox.bottom + fHeight*0.3f - fWidth*(14/15.0f - 0.29f)*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.12f, crBBox.bottom + fHeight*0.3f - fWidth*(14/15.0f - 0.29f)*0.35f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.12f, crBBox.bottom + fHeight*0.3f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.12f, crBBox.top - fHeight*0.25f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth/15 - fWidth*0.12f, crBBox.top - fHeight*0.25f + fWidth*0.35f*(11/12.0f - 0.12f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60, crBBox.top - fHeight*0.25f + fWidth*0.35f*(11/12.0f - 0.12f)), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth/60, crBBox.top - fHeight*0.25f), PWLPT_BEZIERTO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 33);
	else
		GetPathDataFromArray(path, PathArray, 33);	
}

void CPWL_Utils::GetGraphics_Attachment(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.25f, crBBox.top - fHeight*0.1f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.23f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f + fWidth*0.04f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.5f + fWidth*0.04f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.5f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.23f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.25f, crBBox.top - fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.25f, crBBox.top - fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.23f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.23f), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f - fWidth*0.25f*0.4f, crBBox.top - fHeight*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.15f, crBBox.top - fHeight*0.65f + fHeight*0.15f*0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.15f, crBBox.top - fHeight*0.65f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.15f, crBBox.top - fHeight*0.65f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.15f, crBBox.top - fHeight*0.65f + fHeight*0.15f*0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f + fWidth*0.25f*0.4f, crBBox.top - fHeight*0.5f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.5f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.6f, crBBox.top - fHeight*0.5f + fWidth*0.04f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f + fWidth*0.04f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.top - fHeight*0.65f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.5f, crBBox.bottom + fHeight*0.1f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 24);
	else
		GetPathDataFromArray(path, PathArray, 24);
}

void CPWL_Utils::GetGraphics_Tag(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fWidth = crBBox.right - crBBox.left;
	FX_FLOAT fHeight = crBBox.top - crBBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.1f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.1f, crBBox.top - fHeight*0.5f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.3f, crBBox.bottom + fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.1f, crBBox.bottom + fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.1f, crBBox.top - fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.1f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.3f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.2f, crBBox.top - fHeight*0.3f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.5f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.2f, crBBox.top - fHeight*0.5f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left + fWidth*0.4f, crBBox.top - fHeight*0.7f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right - fWidth*0.2f, crBBox.top - fHeight*0.7f), PWLPT_LINETO)
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 12);
	else
		GetPathDataFromArray(path, PathArray, 12);
}

void CPWL_Utils::GetGraphics_Foxit(CFX_ByteString& sPathData, CFX_PathData& path, const CPDF_Rect& crBBox, const PWL_PATH_TYPE type)
{
	FX_FLOAT fOutWidth = crBBox.right - crBBox.left;
	FX_FLOAT fOutHeight = crBBox.top - crBBox.bottom;

	CPDF_Rect crInBox = crBBox;
	crInBox.left = crBBox.left + fOutWidth*0.08f;
	crInBox.right = crBBox.right - fOutWidth*0.08f;
	crInBox.top = crBBox.top - fOutHeight*0.08f;
	crInBox.bottom = crBBox.bottom + fOutHeight*0.08f;

	FX_FLOAT fWidth = crInBox.right - crInBox.left;
	FX_FLOAT fHeight = crInBox.top - crInBox.bottom;

	CPWL_PathData PathArray[] = 
	{
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.45f, crInBox.top), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.45f, crInBox.top - PWL_BEZIER * fHeight * 0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.45f  - PWL_BEZIER * fWidth * 0.45f, crInBox.top - fHeight*0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top - fHeight*0.4f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.60f, crInBox.top), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.75f, crInBox.top), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.75f, crInBox.top - PWL_BEZIER * fHeight * 0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.75f  - PWL_BEZIER * fWidth * 0.75f, crInBox.top - fHeight*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top - fHeight*0.7f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top - fHeight*0.55f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.left + PWL_BEZIER * fWidth*0.60f, crInBox.top - fHeight*0.55f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth * 0.60f, crInBox.top - PWL_BEZIER * fHeight * 0.55f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth * 0.60f, crInBox.top), PWLPT_BEZIERTO),

		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.90f, crInBox.top), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.90f, crInBox.top - PWL_BEZIER * fHeight * 0.85f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.90f  - PWL_BEZIER * fWidth * 0.90f, crInBox.top - fHeight*0.85f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.top - fHeight*0.85f), PWLPT_BEZIERTO),
		CPWL_PathData(CPWL_Point(crInBox.left, crInBox.bottom), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.right, crInBox.bottom), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.right, crInBox.top), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crInBox.left + fWidth*0.90f, crInBox.top), PWLPT_LINETO),

		/*
		CPWL_PathData(CPWL_Point(crBBox.left, crBBox.top), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right, crBBox.top), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right, crBBox.bottom), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left, crBBox.bottom), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left, crBBox.top), PWLPT_LINETO),

		CPWL_PathData(CPWL_Point(crBBox.left+fOutWidth*0.04f, crBBox.top-fOutHeight*0.04f), PWLPT_MOVETO),
		CPWL_PathData(CPWL_Point(crBBox.right-fOutWidth*0.04f, crBBox.top-fOutHeight*0.04f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.right-fOutWidth*0.04f, crBBox.bottom+fOutHeight*0.04f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left+fOutWidth*0.04f, crBBox.bottom+fOutHeight*0.04f), PWLPT_LINETO),
		CPWL_PathData(CPWL_Point(crBBox.left+fOutWidth*0.04f, crBBox.top-fOutHeight*0.04f), PWLPT_LINETO),
		*/
	};

	if(type == PWLPT_STREAM)
		sPathData = GetAppStreamFromArray(PathArray, 23);
	else
		GetPathDataFromArray(path, PathArray, 23);
}

void CPWL_Color::ConvertColorType(FX_INT32 nColorType)
{
	switch (this->nColorType)
	{
	case COLORTYPE_TRANSPARENT:
		break;
	case COLORTYPE_GRAY:
		switch (nColorType)
		{
		case COLORTYPE_RGB:
			CPWL_Utils::ConvertGRAY2RGB(this->fColor1, this->fColor1, this->fColor2, this->fColor3);
			break;
		case COLORTYPE_CMYK:
			CPWL_Utils::ConvertGRAY2CMYK(this->fColor1, this->fColor1, this->fColor2, this->fColor3, this->fColor4);
			break;
		}
		break;
	case COLORTYPE_RGB:
		switch (nColorType)
		{
		case COLORTYPE_GRAY:
			CPWL_Utils::ConvertRGB2GRAY(this->fColor1, this->fColor2, this->fColor3, this->fColor1);
			break;
		case COLORTYPE_CMYK:
			CPWL_Utils::ConvertRGB2CMYK(this->fColor1, this->fColor2, this->fColor3, this->fColor1, this->fColor2, this->fColor3, this->fColor4);
			break;
		}
		break;
	case COLORTYPE_CMYK:
		switch (nColorType)
		{
		case COLORTYPE_GRAY:
			CPWL_Utils::ConvertCMYK2GRAY(this->fColor1, this->fColor2, this->fColor3, this->fColor4, this->fColor1);
			break;
		case COLORTYPE_RGB:
			CPWL_Utils::ConvertCMYK2RGB(this->fColor1, this->fColor2, this->fColor3, this->fColor4, this->fColor1, this->fColor2, this->fColor3);
			break;
		}
		break;
	}
	this->nColorType = nColorType;
}


