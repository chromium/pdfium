// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_transformpage.h"
#include "../include/fsdk_define.h"

DLLEXPORT void STDCALL FPDFPage_SetMediaBox(FPDF_PAGE page, float left, float bottom, float right, float top)
{
	if(!page)
		return;

	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
	CPDF_Array* pMediaBoxArray = new CPDF_Array;
	pMediaBoxArray->Add(new CPDF_Number(left));
	pMediaBoxArray->Add(new CPDF_Number(bottom));
	pMediaBoxArray->Add(new CPDF_Number(FX_FLOAT(right)));
	pMediaBoxArray->Add(new CPDF_Number(FX_FLOAT(top)));
	pPageDict->SetAt("MediaBox", pMediaBoxArray);
}


DLLEXPORT void STDCALL FPDFPage_SetCropBox(FPDF_PAGE page, float left, float bottom, float right, float top)
{
	if(!page)
		return;

	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
	CPDF_Array* pCropBoxArray = new CPDF_Array;
	pCropBoxArray->Add(new CPDF_Number(left));
	pCropBoxArray->Add(new CPDF_Number(bottom));
	pCropBoxArray->Add(new CPDF_Number(FX_FLOAT(right)));
	pCropBoxArray->Add(new CPDF_Number(FX_FLOAT(top)));
	pPageDict->SetAt("CropBox", pCropBoxArray);
}


DLLEXPORT FX_BOOL STDCALL FPDFPage_GetMediaBox(FPDF_PAGE page, float* left, float* bottom, float* right, float* top)
{
	if(!page)
		return FALSE;
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
	CPDF_Array* pArray = pPageDict->GetArray("MediaBox");
	if(pArray)
	{
		*left = pArray->GetFloat(0);
		*bottom = pArray->GetFloat(1);
		*right = pArray->GetFloat(2);
		*top = pArray->GetFloat(3);
		return TRUE;
	}
	return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GetCropBox(FPDF_PAGE page, float* left, float* bottom, float* right, float* top)
{
	if(!page)
		return FALSE;
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDict = pPage->m_pFormDict;
	CPDF_Array* pArray = pPageDict->GetArray("CropBox");
	if(pArray)
	{
		*left = pArray->GetFloat(0);
		*bottom = pArray->GetFloat(1);
		*right = pArray->GetFloat(2);
		*top = pArray->GetFloat(3);
		return TRUE;
	}
	return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_TransFormWithClip(FPDF_PAGE page, FS_MATRIX* matrix, FS_RECTF* clipRect)
{
	if(!page)
		return FALSE;

	CFX_ByteTextBuf textBuf;
	textBuf<<"q ";
	CFX_FloatRect rect(clipRect->left, clipRect->bottom, clipRect->right, clipRect->top);
	rect.Normalize();
	CFX_ByteString bsClipping;
	bsClipping.Format("%f %f %f %f re W* n ", rect.left, rect.bottom, rect.Width(), rect.Height());
	textBuf<<bsClipping;

	CFX_ByteString bsMatix;
	bsMatix.Format("%f %f %f %f %f %f cm ", matrix->a, matrix->b,matrix->c,matrix->d,matrix->e,matrix->f);
	textBuf<<bsMatix;
	

	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDic = pPage->m_pFormDict;
	CPDF_Object* pContentObj = pPageDic ? pPageDic->GetElement("Contents") : NULL;
	if(!pContentObj)
		pContentObj = pPageDic ? pPageDic->GetArray("Contents") : NULL;
	if(!pContentObj)
		return FALSE;
	
	CPDF_Dictionary* pDic = new CPDF_Dictionary;
	CPDF_Stream* pStream = new CPDF_Stream(NULL,0, pDic);
	pStream->SetData(textBuf.GetBuffer(), textBuf.GetSize(), FALSE, FALSE);
	CPDF_Document* pDoc = pPage->m_pDocument;
	if(!pDoc)
		return FALSE;
	pDoc->AddIndirectObject(pStream);

	pDic = new CPDF_Dictionary;
	CPDF_Stream* pEndStream = new CPDF_Stream(NULL,0, pDic);
	pEndStream->SetData((FX_LPCBYTE)" Q", 2, FALSE, FALSE);
	pDoc->AddIndirectObject(pEndStream);
	
	CPDF_Array* pContentArray = NULL;
	if (pContentObj && pContentObj->GetType() == PDFOBJ_ARRAY)
	{
		pContentArray = (CPDF_Array*)pContentObj;
		CPDF_Reference* pRef = new CPDF_Reference(pDoc, pStream->GetObjNum());
		pContentArray->InsertAt(0, pRef);
		pContentArray->AddReference(pDoc,pEndStream);
		
	}
	else if(pContentObj && pContentObj->GetType() == PDFOBJ_REFERENCE)
	{
		CPDF_Reference* pReference = (CPDF_Reference*)pContentObj;
		CPDF_Object* pDirectObj = pReference->GetDirect();
		if(pDirectObj != NULL)
		{
			if(pDirectObj->GetType() == PDFOBJ_ARRAY)
			{
				pContentArray = (CPDF_Array*)pDirectObj;
				CPDF_Reference* pRef = new CPDF_Reference(pDoc, pStream->GetObjNum());
				pContentArray->InsertAt(0, pRef);
				pContentArray->AddReference(pDoc,pEndStream);
			}
			else if(pDirectObj->GetType() == PDFOBJ_STREAM)
			{
				pContentArray = new CPDF_Array();
				pContentArray->AddReference(pDoc,pStream->GetObjNum());
				pContentArray->AddReference(pDoc,pDirectObj->GetObjNum());
				pContentArray->AddReference(pDoc, pEndStream);
				pPageDic->SetAtReference("Contents", pDoc, pDoc->AddIndirectObject(pContentArray));
			}
		}
	}

	//Need to transform the patterns as well.
	CPDF_Dictionary* pRes = pPageDic->GetDict(FX_BSTRC("Resources"));
	if(pRes)
	{
		CPDF_Dictionary* pPattenDict = pRes->GetDict(FX_BSTRC("Pattern"));
		if(pPattenDict)
		{
			FX_POSITION pos = pPattenDict->GetStartPos();
			while(pos)
			{
				CPDF_Dictionary* pDict = NULL;
				CFX_ByteString key;
				CPDF_Object* pObj = pPattenDict->GetNextElement(pos, key);
				if(pObj->GetType() == PDFOBJ_REFERENCE)
					pObj = pObj->GetDirect();
				if(pObj->GetType() == PDFOBJ_DICTIONARY)
				{
					pDict = (CPDF_Dictionary*)pObj;
				}
				else if(pObj->GetType() == PDFOBJ_STREAM)
				{
					pDict = ((CPDF_Stream*)pObj)->GetDict();
				}
				else
					continue;
				
				CFX_AffineMatrix m = pDict->GetMatrix(FX_BSTRC("Matrix"));
				CFX_AffineMatrix t = *(CFX_AffineMatrix*)matrix;
				m.Concat(t);
				pDict->SetAtMatrix(FX_BSTRC("Matrix"), m);
			}
		}
	}

	return TRUE;
}

DLLEXPORT void STDCALL FPDFPageObj_TransformClipPath(FPDF_PAGEOBJECT page_object,double a, double b, double c, double d, double e, double f)
{
	CPDF_PageObject* pPageObj = (CPDF_PageObject*)page_object;
	if(pPageObj == NULL)
		return;
	CFX_AffineMatrix matrix((FX_FLOAT)a,(FX_FLOAT)b,(FX_FLOAT)c,(FX_FLOAT)d,(FX_FLOAT)e,(FX_FLOAT)f);
	
	//Special treatment to shading object, because the ClipPath for shading object is already transformed.
	if(pPageObj->m_Type != PDFPAGE_SHADING)
		pPageObj->TransformClipPath(matrix);
	pPageObj->TransformGeneralState(matrix);
}


DLLEXPORT FPDF_CLIPPATH STDCALL FPDF_CreateClipPath(float left, float bottom, float right, float top)
{
	CPDF_ClipPath* pNewClipPath = new CPDF_ClipPath();
	pNewClipPath->GetModify();
	CPDF_Path Path;
	Path.GetModify();
	Path.AppendRect(left, bottom, right, top);
	pNewClipPath->AppendPath(Path, FXFILL_ALTERNATE, FALSE);
	return pNewClipPath;
}

DLLEXPORT void STDCALL FPDF_DestroyClipPath(FPDF_CLIPPATH clipPath)
{
	if(clipPath)
		delete (CPDF_ClipPath*)clipPath;
}

void OutputPath(CFX_ByteTextBuf& buf, CPDF_Path path)
{
	const CFX_PathData* pPathData = path;
	if (pPathData == NULL) return;
	
	FX_PATHPOINT* pPoints = pPathData->GetPoints();
	
	if (path.IsRect()) {
		buf << (pPoints[0].m_PointX) << " " << (pPoints[0].m_PointY) << " " 
			<< (pPoints[2].m_PointX - pPoints[0].m_PointX) << " " 
			<< (pPoints[2].m_PointY - pPoints[0].m_PointY) << " re\n";
		return;
	}
	
	CFX_ByteString temp;
	for (int i = 0; i < pPathData->GetPointCount(); i ++) {
		buf << (pPoints[i].m_PointX) << " " << (pPoints[i].m_PointY);
		int point_type = pPoints[i].m_Flag & FXPT_TYPE;
		if (point_type == FXPT_MOVETO)
			buf << " m\n";
		else if (point_type == FXPT_BEZIERTO) {
			buf << " " << (pPoints[i+1].m_PointX) << " " << (pPoints[i+1].m_PointY) << " " << 
				(pPoints[i+2].m_PointX) << " " << (pPoints[i+2].m_PointY);
			if (pPoints[i+2].m_Flag & FXPT_CLOSEFIGURE)
				buf << " c h\n";
			else
				buf << " c\n";
			i += 2;
		} else if (point_type == FXPT_LINETO) {
			if (pPoints[i].m_Flag & FXPT_CLOSEFIGURE)
				buf << " l h\n";
			else
				buf << " l\n";
		}
	}
}

DLLEXPORT void STDCALL FPDFPage_InsertClipPath(FPDF_PAGE page,FPDF_CLIPPATH clipPath)
{
	if(!page)
		return;
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_Dictionary* pPageDic = pPage->m_pFormDict;
	CPDF_Object* pContentObj = pPageDic ? pPageDic->GetElement("Contents") : NULL;
	if(!pContentObj)
		pContentObj = pPageDic ? pPageDic->GetArray("Contents") : NULL;
	if(!pContentObj)
		return;

	CFX_ByteTextBuf strClip;
	CPDF_ClipPath* pClipPath = (CPDF_ClipPath*)clipPath;
	FX_DWORD i;
	for (i = 0; i < pClipPath->GetPathCount(); i ++) {
		CPDF_Path path = pClipPath->GetPath(i);
		int iClipType = pClipPath->GetClipType(i);
		if (path.GetPointCount() == 0) {
			// Empty clipping (totally clipped out)
			strClip << "0 0 m W n ";
		} else {
			OutputPath(strClip, path);
			if (iClipType == FXFILL_WINDING)
				strClip << "W n\n";
			else
				strClip << "W* n\n";
		}
	}
	CPDF_Dictionary* pDic = new CPDF_Dictionary;
	CPDF_Stream* pStream = new CPDF_Stream(NULL,0, pDic);
	pStream->SetData(strClip.GetBuffer(), strClip.GetSize(), FALSE, FALSE);
	CPDF_Document* pDoc = pPage->m_pDocument;
	if(!pDoc)
		return;
	pDoc->AddIndirectObject(pStream);
	
	CPDF_Array* pContentArray = NULL;
	if (pContentObj && pContentObj->GetType() == PDFOBJ_ARRAY)
	{
		pContentArray = (CPDF_Array*)pContentObj;
		CPDF_Reference* pRef = new CPDF_Reference(pDoc, pStream->GetObjNum());
		pContentArray->InsertAt(0, pRef);
	}
	else if(pContentObj && pContentObj->GetType() == PDFOBJ_REFERENCE)
	{
		CPDF_Reference* pReference = (CPDF_Reference*)pContentObj;
		CPDF_Object* pDirectObj = pReference->GetDirect();
		if(pDirectObj != NULL)
		{
			if(pDirectObj->GetType() == PDFOBJ_ARRAY)
			{
				pContentArray = (CPDF_Array*)pDirectObj;
				CPDF_Reference* pRef = new CPDF_Reference(pDoc, pStream->GetObjNum());
				pContentArray->InsertAt(0, pRef);
			}
			else if(pDirectObj->GetType() == PDFOBJ_STREAM)
			{
				pContentArray = new CPDF_Array();
				pContentArray->AddReference(pDoc,pStream->GetObjNum());
				pContentArray->AddReference(pDoc,pDirectObj->GetObjNum());
				pPageDic->SetAtReference("Contents", pDoc, pDoc->AddIndirectObject(pContentArray));
			}
		}
	}
}

