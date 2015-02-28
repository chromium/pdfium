// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxedit/fxet_stub.h"
#include "../../include/fxedit/fx_edit.h"
#include "../../include/fxedit/fxet_edit.h"

#define FX_EDIT_UNDERLINEHALFWIDTH				0.5f
#define FX_EDIT_CROSSOUTHALFWIDTH				0.5f

extern CFX_ByteString GetPDFWordString(IFX_Edit_FontMap * pFontMap, FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord);

CPDF_Rect GetUnderLineRect(const CPVT_Word& word)
{
	return CPDF_Rect(word.ptWord.x, word.ptWord.y + word.fDescent * 0.5f,
						word.ptWord.x + word.fWidth, word.ptWord.y + word.fDescent * 0.25f);
}

CPDF_Rect GetCrossoutRect(const CPVT_Word& word)
{
	return CPDF_Rect(word.ptWord.x, word.ptWord.y + (word.fAscent + word.fDescent) * 0.5f + word.fDescent * 0.25f,
							word.ptWord.x + word.fWidth, word.ptWord.y + (word.fAscent + word.fDescent) * 0.5f);
}

static void DrawTextString(CFX_RenderDevice* pDevice, const CPDF_Point& pt, CPDF_Font* pFont, FX_FLOAT fFontSize, CPDF_Matrix* pUser2Device,
					  const CFX_ByteString& str, FX_ARGB crTextFill, FX_ARGB crTextStroke, FX_INT32 nHorzScale)
{
	FX_FLOAT x = pt.x, y = pt.y;
	pUser2Device->Transform(x, y);

	if (pFont)
	{
		if (nHorzScale != 100)
		{
			CPDF_Matrix mt(nHorzScale/100.0f,0,0,1,0,0);
			mt.Concat(*pUser2Device);

			CPDF_RenderOptions ro;
			ro.m_Flags = RENDER_CLEARTYPE;
			ro.m_ColorMode = RENDER_COLOR_NORMAL;

			if (crTextStroke != 0)
			{
				CPDF_Point pt1(0,0), pt2(1,0);
				pUser2Device->Transform(pt1.x, pt1.y);
				pUser2Device->Transform(pt2.x, pt2.y);
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = (FX_FLOAT)FXSYS_fabs((pt2.x + pt2.y) - (pt1.x + pt1.y));

				CPDF_TextRenderer::DrawTextString(pDevice,x, y, pFont, fFontSize, &mt, str, crTextFill, crTextStroke, &gsd, &ro);
			}
			else
				CPDF_TextRenderer::DrawTextString(pDevice,x, y, pFont, fFontSize, &mt, str, crTextFill, 0, NULL, &ro);
		}
		else
		{
			CPDF_RenderOptions ro;
			ro.m_Flags = RENDER_CLEARTYPE;
			ro.m_ColorMode = RENDER_COLOR_NORMAL;

			if (crTextStroke != 0)
			{
				CPDF_Point pt1(0,0), pt2(1,0);
				pUser2Device->Transform(pt1.x, pt1.y);
				pUser2Device->Transform(pt2.x, pt2.y);
				CFX_GraphStateData gsd;
				gsd.m_LineWidth = (FX_FLOAT)FXSYS_fabs((pt2.x + pt2.y) - (pt1.x + pt1.y));

				CPDF_TextRenderer::DrawTextString(pDevice,x, y, pFont, fFontSize, pUser2Device, str, crTextFill, crTextStroke, &gsd, &ro);
			}
			else
				CPDF_TextRenderer::DrawTextString(pDevice,x, y, pFont, fFontSize, pUser2Device, str, crTextFill, 0, NULL, &ro);
		}
	}
}

void IFX_Edit::DrawUnderline(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, FX_COLORREF color,
								const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange)
{
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

			while (pIterator->NextWord())
			{
				CPVT_WordPlace place = pIterator->GetAt();
				if (pRange && place.WordCmp(pRange->EndPos) > 0) break;

				CPVT_Word word;				
				if (pIterator->GetWord(word))
				{
					CFX_PathData pathUnderline;
					CPDF_Rect rcUnderline = GetUnderLineRect(word);
					rcUnderline.left += ptOffset.x;
					rcUnderline.right += ptOffset.x;
					rcUnderline.top += ptOffset.y;
					rcUnderline.bottom += ptOffset.y;
					pathUnderline.AppendRect(rcUnderline.left, rcUnderline.bottom, rcUnderline.right, rcUnderline.top);

					pDevice->DrawPath(&pathUnderline, pUser2Device, NULL, color, 0, FXFILL_WINDING);
				}
			}			
		}
	}
	
	pDevice->RestoreState();
}

void IFX_Edit::DrawEdit(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit, FX_COLORREF crTextFill, FX_COLORREF crTextStroke, 
						const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, IFX_SystemHandler* pSystemHandler, void* pFFLData)
{
	
	FX_BOOL bContinuous = pEdit->GetCharArray() == 0;
	if (pEdit->GetCharSpace() > 0.0f)
		bContinuous = FALSE;

	FX_WORD SubWord = pEdit->GetPasswordChar();
	FX_FLOAT fFontSize = pEdit->GetFontSize();
	CPVT_WordRange wrSelect = pEdit->GetSelectWordRange();
	FX_INT32 nHorzScale = pEdit->GetHorzScale();

	FX_COLORREF crCurFill = crTextFill;
	FX_COLORREF crOldFill = crCurFill;

	FX_BOOL bSelect = FALSE;
	const FX_COLORREF crWhite = ArgbEncode(255,255,255,255);
	const FX_COLORREF crSelBK = ArgbEncode(255,0,51,113);

	CFX_ByteTextBuf sTextBuf;
	FX_INT32 nFontIndex = -1;
	CPDF_Point ptBT(0.0f,0.0f);

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
		if (IFX_Edit_FontMap* pFontMap = pEdit->GetFontMap())
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

				if (wrSelect.IsExist())
				{
					bSelect = place.WordCmp(wrSelect.BeginPos) > 0 && place.WordCmp(wrSelect.EndPos) <= 0;
					if (bSelect)
					{						
						crCurFill = crWhite;						
					}
					else
					{
						crCurFill = crTextFill;
					}
				}
				if(pSystemHandler && pSystemHandler->IsSelectionImplemented())	
				{
					crCurFill = crTextFill;
 					crOldFill = crCurFill;
				}
				CPVT_Word word;				
				if (pIterator->GetWord(word))
				{

					if (bSelect)
					{
						
						CPVT_Line line;
						pIterator->GetLine(line);

						if(pSystemHandler && pSystemHandler->IsSelectionImplemented())
						{
							CPDF_Rect rc(word.ptWord.x,line.ptLine.y + line.fLineDescent,
								word.ptWord.x+word.fWidth,line.ptLine.y + line.fLineAscent);
							rc.Intersect(rcClip);
							//CFX_Edit* pEt = (CFX_Edit*)pEdit;
							//CPDF_Rect rcEdit = pEt->VTToEdit(rc);
							pSystemHandler->OutputSelectedRect(pFFLData,rc);
						}
						else
						{	
 							CFX_PathData pathSelBK;
 							pathSelBK.AppendRect(word.ptWord.x,line.ptLine.y + line.fLineDescent,
 								word.ptWord.x+word.fWidth,line.ptLine.y + line.fLineAscent);
 							
 							pDevice->DrawPath(&pathSelBK, pUser2Device, NULL, crSelBK, 0, FXFILL_WINDING);	
						}
					}

					if (bContinuous)
					{
						if (place.LineCmp(oldplace) != 0 || word.nFontIndex != nFontIndex || 
							crOldFill != crCurFill)
						{
							if (sTextBuf.GetLength() > 0)
							{								
								DrawTextString(pDevice, CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), pFontMap->GetPDFFont(nFontIndex),
									fFontSize, pUser2Device, sTextBuf.GetByteString(), crOldFill, crTextStroke, nHorzScale);

								sTextBuf.Clear();
							}
							nFontIndex = word.nFontIndex;
							ptBT = word.ptWord;
							crOldFill = crCurFill;
						}

						sTextBuf << GetPDFWordString(pFontMap, word.nFontIndex, word.Word, SubWord);						
					}
					else
					{
						DrawTextString(pDevice,CPDF_Point(word.ptWord.x+ptOffset.x, word.ptWord.y+ptOffset.y), pFontMap->GetPDFFont(word.nFontIndex),
							fFontSize, pUser2Device, GetPDFWordString(pFontMap, word.nFontIndex, word.Word, SubWord), crCurFill, crTextStroke, nHorzScale);

					}
					oldplace = place;


				}
			}

			if (sTextBuf.GetLength() > 0)
			{				
				DrawTextString(pDevice, CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), pFontMap->GetPDFFont(nFontIndex),
					fFontSize, pUser2Device, sTextBuf.GetByteString(), crOldFill, crTextStroke, nHorzScale);
			}			
		}
	}
	
	pDevice->RestoreState();
}

void IFX_Edit::DrawRichEdit(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device, IFX_Edit* pEdit,  
						const CPDF_Rect& rcClip, const CPDF_Point& ptOffset, const CPVT_WordRange* pRange)
{
	//FX_FLOAT fFontSize = pEdit->GetFontSize();
	CPVT_WordRange wrSelect = pEdit->GetSelectWordRange();

	FX_COLORREF crCurText = ArgbEncode(255, 0,0,0);
	FX_COLORREF crOld = crCurText;
	FX_BOOL bSelect = FALSE;
	const FX_COLORREF crWhite = ArgbEncode(255,255,255,255);
	const FX_COLORREF crSelBK = ArgbEncode(255,0,51,113);

	CFX_ByteTextBuf sTextBuf;
	CPVT_WordProps wp;
	CPDF_Point ptBT(0.0f,0.0f);

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
		if (IFX_Edit_FontMap* pFontMap = pEdit->GetFontMap())
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
					word.WordProps.fFontSize = word.fFontSize;

					crCurText = ArgbEncode(255,word.WordProps.dwWordColor);

					if (wrSelect.IsExist())
					{
						bSelect = place.WordCmp(wrSelect.BeginPos) > 0 && place.WordCmp(wrSelect.EndPos) <= 0;
						if (bSelect)
						{						
							crCurText = crWhite;
						}
					}

					if (bSelect)
					{
						CPVT_Line line;
						pIterator->GetLine(line);

						CFX_PathData pathSelBK;
						pathSelBK.AppendRect(word.ptWord.x		+ ptOffset.x,
							line.ptLine.y + line.fLineDescent	+ ptOffset.y,
							word.ptWord.x+word.fWidth			+ ptOffset.x,
							line.ptLine.y + line.fLineAscent	+ ptOffset.y);

						pDevice->DrawPath(&pathSelBK, pUser2Device, NULL, crSelBK, 0, FXFILL_WINDING);												
					}

					if (place.LineCmp(oldplace) != 0 || word.WordProps.fCharSpace > 0.0f || word.WordProps.nHorzScale != 100 || 
						FXSYS_memcmp(&word.WordProps, &wp, sizeof(CPVT_WordProps)) != 0 || 
						crOld != crCurText)
					{
						if (sTextBuf.GetLength() > 0)
						{								
							DrawTextString(pDevice, CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), pFontMap->GetPDFFont(wp.nFontIndex),
								wp.fFontSize, pUser2Device, sTextBuf.GetByteString(), crOld, 0, wp.nHorzScale);

							sTextBuf.Clear();
						}
						wp = word.WordProps;
						ptBT = word.ptWord;
						crOld = crCurText;
					}

					sTextBuf << GetPDFWordString(pFontMap, word.WordProps.nFontIndex, word.Word, 0);	
					
					if (word.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE)
					{
						CFX_PathData pathUnderline;
						CPDF_Rect rcUnderline = GetUnderLineRect(word);
						pathUnderline.AppendRect(rcUnderline.left, rcUnderline.bottom, rcUnderline.right, rcUnderline.top);

						pDevice->DrawPath(&pathUnderline, pUser2Device, NULL, crCurText, 0, FXFILL_WINDING);	
					}

					if (word.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT)
					{
						CFX_PathData pathCrossout;
						CPDF_Rect rcCrossout = GetCrossoutRect(word);
						pathCrossout.AppendRect(rcCrossout.left, rcCrossout.bottom, rcCrossout.right, rcCrossout.top);

						pDevice->DrawPath(&pathCrossout, pUser2Device, NULL, crCurText, 0, FXFILL_WINDING);
					}

					oldplace = place;					
				}
			}

			if (sTextBuf.GetLength() > 0)
			{				
				DrawTextString(pDevice, CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), pFontMap->GetPDFFont(wp.nFontIndex),
					wp.fFontSize, pUser2Device, sTextBuf.GetByteString(), crOld, 0, wp.nHorzScale);
			}
		}
	}
	
	pDevice->RestoreState();
}

static void AddRectToPageObjects(CPDF_PageObjects* pPageObjs, FX_COLORREF crFill, const CPDF_Rect& rcFill)
{
	CPDF_PathObject* pPathObj = new CPDF_PathObject;
	CPDF_PathData* pPathData = pPathObj->m_Path.GetModify();
	pPathData->AppendRect(rcFill.left,rcFill.bottom,rcFill.right,rcFill.top);	
	
	FX_FLOAT rgb[3];
	rgb[0] = FXARGB_R(crFill) / 255.0f ;
	rgb[1] = FXARGB_G(crFill) / 255.0f;
	rgb[2] = FXARGB_B(crFill) / 255.0f;
	pPathObj->m_ColorState.SetFillColor(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB), rgb, 3);

	pPathObj->m_FillType = FXFILL_ALTERNATE;
	pPathObj->m_bStroke = FALSE;

	pPageObjs->InsertObject(pPageObjs->GetLastObjectPosition(),pPathObj);
}

static CPDF_TextObject* AddTextObjToPageObjects(CPDF_PageObjects* pPageObjs, FX_COLORREF crText, 
							 CPDF_Font* pFont, FX_FLOAT fFontSize, FX_FLOAT fCharSpace, FX_INT32 nHorzScale, 
							 const CPDF_Point& point, const CFX_ByteString& text)
{
	CPDF_TextObject* pTxtObj = new CPDF_TextObject;
			
	CPDF_TextStateData* pTextStateData = pTxtObj->m_TextState.GetModify();
	pTextStateData->m_pFont = pFont;
	pTextStateData->m_FontSize = fFontSize;
	pTextStateData->m_CharSpace = fCharSpace;
	pTextStateData->m_WordSpace = 0;
	pTextStateData->m_TextMode  = 0;
	pTextStateData->m_Matrix[0] = nHorzScale / 100.0f;
	pTextStateData->m_Matrix[1] = 0;
	pTextStateData->m_Matrix[2] = 0;
	pTextStateData->m_Matrix[3] = 1;

	FX_FLOAT rgb[3];
	rgb[0] = FXARGB_R(crText) / 255.0f ;
	rgb[1] = FXARGB_G(crText) / 255.0f;
	rgb[2] = FXARGB_B(crText) / 255.0f;
	pTxtObj->m_ColorState.SetFillColor(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB),rgb, 3);
	pTxtObj->m_ColorState.SetStrokeColor(CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB),rgb, 3);

	pTxtObj->SetPosition(point.x,point.y);
	pTxtObj->SetText(text);	

	pPageObjs->InsertObject(pPageObjs->GetLastObjectPosition(),pTxtObj);

	return pTxtObj;
}

void IFX_Edit::GeneratePageObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
								   const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, FX_COLORREF crText, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray)
{
	FX_FLOAT fFontSize = pEdit->GetFontSize();

	FX_INT32 nOldFontIndex = -1;

	CFX_ByteTextBuf sTextBuf;
	CPDF_Point ptBT(0.0f,0.0f);

	ObjArray.RemoveAll();

	if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator())
	{
		if (IFX_Edit_FontMap* pFontMap = pEdit->GetFontMap())
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
					if (place.LineCmp(oldplace) != 0 || nOldFontIndex != word.nFontIndex)
					{
						if (sTextBuf.GetLength() > 0)
						{
							ObjArray.Add(AddTextObjToPageObjects(pPageObjects, crText, pFontMap->GetPDFFont(nOldFontIndex), fFontSize, 0.0f, 100,
								CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), sTextBuf.GetByteString()));

							sTextBuf.Clear();
						}

						ptBT = word.ptWord;
						nOldFontIndex = word.nFontIndex;
					}

					sTextBuf << GetPDFWordString(pFontMap, word.nFontIndex, word.Word, 0);
					oldplace = place;					
				}
			}

			if (sTextBuf.GetLength() > 0)
			{				
				ObjArray.Add(AddTextObjToPageObjects(pPageObjects, crText, pFontMap->GetPDFFont(nOldFontIndex), fFontSize, 0.0f, 100,
					CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), sTextBuf.GetByteString()));
			}
		}
	}
}

void IFX_Edit::GenerateRichPageObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
								   const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, CFX_ArrayTemplate<CPDF_TextObject*>& ObjArray)
{


	FX_COLORREF crCurText = ArgbEncode(255, 0, 0, 0);
	FX_COLORREF crOld = crCurText;


	CFX_ByteTextBuf sTextBuf;
	CPVT_WordProps wp;
	CPDF_Point ptBT(0.0f,0.0f);

	ObjArray.RemoveAll();

	if (IFX_Edit_Iterator* pIterator = pEdit->GetIterator())
	{
		if (IFX_Edit_FontMap* pFontMap = pEdit->GetFontMap())
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
					word.WordProps.fFontSize = word.fFontSize;

					crCurText = ArgbEncode(255,word.WordProps.dwWordColor);

					if (place.LineCmp(oldplace) != 0 || word.WordProps.fCharSpace > 0.0f || word.WordProps.nHorzScale != 100 || 
						FXSYS_memcmp(&word.WordProps, &wp, sizeof(CPVT_WordProps)) != 0 || 
						crOld != crCurText)
					{
						if (sTextBuf.GetLength() > 0)
						{
							ObjArray.Add(AddTextObjToPageObjects(pPageObjects, crOld, pFontMap->GetPDFFont(wp.nFontIndex), wp.fFontSize, wp.fCharSpace, wp.nHorzScale,
								CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), sTextBuf.GetByteString()));

							sTextBuf.Clear();
						}

						wp = word.WordProps;
						ptBT = word.ptWord;
						crOld = crCurText;
	
					}

					sTextBuf << GetPDFWordString(pFontMap, word.WordProps.nFontIndex, word.Word, 0);	
					
					if (word.WordProps.nWordStyle & PVTWORD_STYLE_UNDERLINE)
					{/*
						AddLineToPageObjects(pPageObjects, crCurText, 
							CPDF_Point(word.ptWord.x, word.ptWord.y + word.fDescent * 0.4f),
							CPDF_Point(word.ptWord.x + word.fWidth, word.ptWord.y + word.fDescent * 0.4f));							
*/
						CPDF_Rect rcUnderline = GetUnderLineRect(word);
						rcUnderline.left += ptOffset.x;
						rcUnderline.right += ptOffset.x;
						rcUnderline.top += ptOffset.y;
						rcUnderline.bottom += ptOffset.y;
						
						AddRectToPageObjects(pPageObjects, crCurText, rcUnderline);							
					}

					if (word.WordProps.nWordStyle & PVTWORD_STYLE_CROSSOUT)
					{
						CPDF_Rect rcCrossout = GetCrossoutRect(word);
						rcCrossout.left += ptOffset.x;
						rcCrossout.right += ptOffset.x;
						rcCrossout.top += ptOffset.y;
						rcCrossout.bottom += ptOffset.y;

						AddRectToPageObjects(pPageObjects, crCurText, rcCrossout);						
					}

					oldplace = place;					
				}
			}

			if (sTextBuf.GetLength() > 0)
			{				
				ObjArray.Add(AddTextObjToPageObjects(pPageObjects, crOld, pFontMap->GetPDFFont(wp.nFontIndex), wp.fFontSize, wp.fCharSpace, wp.nHorzScale,
					CPDF_Point(ptBT.x+ptOffset.x, ptBT.y+ptOffset.y), sTextBuf.GetByteString()));
			}
		}
	}
}

void IFX_Edit::GenerateUnderlineObjects(CPDF_PageObjects* pPageObjects, IFX_Edit* pEdit,
								   const CPDF_Point& ptOffset, const CPVT_WordRange* pRange, FX_COLORREF color)
{


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
					CPDF_Rect rcUnderline = GetUnderLineRect(word);
					rcUnderline.left += ptOffset.x;
					rcUnderline.right += ptOffset.x;
					rcUnderline.top += ptOffset.y;
					rcUnderline.bottom += ptOffset.y;
					AddRectToPageObjects(pPageObjects, color, rcUnderline);
				}
			}
		}
	}
}

