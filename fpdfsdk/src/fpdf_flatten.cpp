// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fpdf_flatten.h"

typedef CFX_ArrayTemplate<CPDF_Dictionary*> CPDF_ObjectArray;
typedef CFX_ArrayTemplate<CPDF_Rect> CPDF_RectArray;

enum FPDF_TYPE { MAX, MIN };
enum FPDF_VALUE { TOP, LEFT, RIGHT, BOTTOM };

FX_BOOL IsValiableRect(CPDF_Rect rect, CPDF_Rect rcPage)
{
	if ( rect.left - rect.right > 0.000001f || 
		 rect.bottom - rect.top > 0.000001f)
		return FALSE;
	
	if (rect.left == 0.0f &&
		rect.top == 0.0f &&
		rect.right == 0.0f &&
		rect.bottom == 0.0f)
		return FALSE;
	
	if (!rcPage.IsEmpty())
	{
		if (rect.left - rcPage.left < -10.000001f ||
			rect.right - rcPage.right > 10.000001f ||
			rect.top - rcPage.top > 10.000001f ||
			rect.bottom - rcPage.bottom < -10.000001f)
			return FALSE;
	}
	
	return TRUE;
}


FX_BOOL GetContentsRect( CPDF_Document * pDoc, CPDF_Dictionary* pDict, CPDF_RectArray * pRectArray )
{
	CPDF_Page* pPDFPage = FX_NEW CPDF_Page;
	pPDFPage->Load( pDoc, pDict, FALSE );
	pPDFPage->ParseContent();

	FX_POSITION pos = pPDFPage->GetFirstObjectPosition();
	
	while (pos)
	{
		CPDF_PageObject* pPageObject = pPDFPage->GetNextObject(pos);
		if (!pPageObject)continue;
		
		CPDF_Rect rc;
		rc.left = pPageObject->m_Left;
		rc.right = pPageObject->m_Right;
		rc.bottom = pPageObject->m_Bottom;
		rc.top = pPageObject->m_Top;
		
		if (IsValiableRect(rc, pDict->GetRect("MediaBox")))
		{
			pRectArray->Add(rc);
		}
	}
	
	delete pPDFPage;
	return TRUE;
}


void ParserStream( CPDF_Dictionary * pPageDic, CPDF_Dictionary* pStream, CPDF_RectArray * pRectArray, CPDF_ObjectArray * pObjectArray )
{
	if (!pStream)return;
	CPDF_Rect rect;
	if (pStream->KeyExist("Rect"))
		rect = pStream->GetRect("Rect");
	else if (pStream->KeyExist("BBox"))
		rect = pStream->GetRect("BBox");
	
	if (IsValiableRect(rect, pPageDic->GetRect("MediaBox")))
		pRectArray->Add(rect);
	
	pObjectArray->Add(pStream);
}


int ParserAnnots( CPDF_Document* pSourceDoc, CPDF_Dictionary * pPageDic, CPDF_RectArray * pRectArray, CPDF_ObjectArray * pObjectArray, int nUsage)
{
	if (!pSourceDoc || !pPageDic) return FLATTEN_FAIL;
	
	GetContentsRect( pSourceDoc, pPageDic, pRectArray );
	CPDF_Array* pAnnots = pPageDic->GetArray("Annots");
	if (pAnnots)
	{
		FX_DWORD dwSize = pAnnots->GetCount();
		
		for (int i = 0; i < (int)dwSize; i++)
		{
			CPDF_Object* pObj = pAnnots->GetElementValue(i);
			
			if (!pObj)continue;
			
			if (pObj->GetType() == PDFOBJ_DICTIONARY)
			{
				CPDF_Dictionary* pAnnotDic = (CPDF_Dictionary*)pObj;
				CFX_ByteString sSubtype = pAnnotDic->GetString("Subtype");
				if (sSubtype == "Popup")continue;

				int nAnnotFlag = pAnnotDic->GetInteger("F");

				if(nAnnotFlag & ANNOTFLAG_HIDDEN) 
					continue;
				if(nUsage == FLAT_NORMALDISPLAY)
				{
					if(nAnnotFlag & ANNOTFLAG_INVISIBLE)
						continue;
					ParserStream( pPageDic, pAnnotDic, pRectArray, pObjectArray );		
				}
				else
				{
					if(nAnnotFlag & ANNOTFLAG_PRINT)
						ParserStream( pPageDic, pAnnotDic, pRectArray, pObjectArray );
				}			
			}
		}
		return FLATTEN_SUCCESS;
	}else{
		return FLATTEN_NOTINGTODO;
	}
}


FX_FLOAT GetMinMaxValue( CPDF_RectArray& array, FPDF_TYPE type, FPDF_VALUE value)
{
	int nRects = array.GetSize();
	FX_FLOAT fRet = 0.0f;
	
	if (nRects <= 0)return 0.0f;
	
	FX_FLOAT* pArray = new FX_FLOAT[nRects];
	switch(value)
	{
	case LEFT:
		{
			for (int i = 0; i < nRects; i++)
				pArray[i] = CPDF_Rect(array.GetAt(i)).left;
			
			break;
		}
	case TOP:
		{
			for (int i = 0; i < nRects; i++)
				pArray[i] = CPDF_Rect(array.GetAt(i)).top;
			
			break;
		}
	case RIGHT:
		{
			for (int i = 0; i < nRects; i++)
				pArray[i] = CPDF_Rect(array.GetAt(i)).right;
			
			break;
		}
	case BOTTOM:
		{
			for (int i = 0; i < nRects; i++)
				pArray[i] = CPDF_Rect(array.GetAt(i)).bottom;
			
			break;
		}
	default:
		break;
	}
	fRet = pArray[0];
	if (type == MAX)
	{
		for (int i = 1; i < nRects; i++)
			if (fRet <= pArray[i])
				fRet = pArray[i];
	}
	else
	{
		for (int i = 1; i < nRects; i++)
			if (fRet >= pArray[i])
				fRet = pArray[i];
	}
	delete[] pArray;
	return fRet;
}

CPDF_Rect CalculateRect( CPDF_RectArray * pRectArray )
{

	CPDF_Rect rcRet;
	
	rcRet.left = GetMinMaxValue(*pRectArray, MIN, LEFT);
	rcRet.top = GetMinMaxValue(*pRectArray, MAX, TOP);
	rcRet.right = GetMinMaxValue(*pRectArray, MAX, RIGHT);
	rcRet.bottom = GetMinMaxValue(*pRectArray, MIN, BOTTOM);
	
	return rcRet;
}


void SetPageContents(CFX_ByteString key, CPDF_Dictionary* pPage, CPDF_Document* pDocument)
{
	CPDF_Object* pContentsObj = pPage->GetStream("Contents");
	if (!pContentsObj)
	{
		pContentsObj = pPage->GetArray("Contents");
	}
	
	if (!pContentsObj)
	{
		//Create a new contents dictionary
		if (!key.IsEmpty())
		{
			CPDF_Stream* pNewContents = FX_NEW CPDF_Stream(NULL, 0, FX_NEW CPDF_Dictionary);
			if (!pNewContents)return;
			pPage->SetAtReference("Contents", pDocument, pDocument->AddIndirectObject(pNewContents));
			
			CFX_ByteString sStream;
			sStream.Format("q 1 0 0 1 0 0 cm /%s Do Q", key.c_str());
			pNewContents->SetData((FX_LPCBYTE)sStream, sStream.GetLength(), FALSE, FALSE);
		}
		return;
	}

	int iType = pContentsObj->GetType();
	CPDF_Array* pContentsArray = NULL;
	
	switch(iType)
	{
	case PDFOBJ_STREAM:
		{
			pContentsArray = FX_NEW CPDF_Array;
			CPDF_Stream* pContents = (CPDF_Stream*)pContentsObj;
			FX_DWORD dwObjNum = pDocument->AddIndirectObject(pContents);
			CPDF_StreamAcc acc;
			acc.LoadAllData(pContents);
			CFX_ByteString sStream = "q\n";
			CFX_ByteString sBody = CFX_ByteString((FX_LPCSTR)acc.GetData(), acc.GetSize());
			sStream = sStream + sBody + "\nQ";
			pContents->SetData((FX_LPCBYTE)sStream, sStream.GetLength(), FALSE, FALSE);
			pContentsArray->AddReference(pDocument, dwObjNum);
			break;
		}
		
	case PDFOBJ_ARRAY:
		{
			pContentsArray = (CPDF_Array*)pContentsObj;
			break;
		}
	default:
		break;
	}	
	
	if (!pContentsArray)return;
	
	FX_DWORD dwObjNum = pDocument->AddIndirectObject(pContentsArray);
	pPage->SetAtReference("Contents", pDocument, dwObjNum);
	
	if (!key.IsEmpty())
	{
		CPDF_Stream* pNewContents = FX_NEW CPDF_Stream(NULL, 0, FX_NEW CPDF_Dictionary);
		dwObjNum = pDocument->AddIndirectObject(pNewContents);
		pContentsArray->AddReference(pDocument, dwObjNum);
		
		CFX_ByteString sStream;
		sStream.Format("q 1 0 0 1 0 0 cm /%s Do Q", key.c_str());
		pNewContents->SetData((FX_LPCBYTE)sStream, sStream.GetLength(), FALSE, FALSE);
	}
}
 
CFX_AffineMatrix GetMatrix(CPDF_Rect rcAnnot, CPDF_Rect rcStream, CFX_AffineMatrix matrix)
{
	if(rcStream.IsEmpty())
		return CFX_AffineMatrix();
	
	matrix.TransformRect(rcStream);
	rcStream.Normalize();
	
	FX_FLOAT a = rcAnnot.Width()/rcStream.Width();
	FX_FLOAT d = rcAnnot.Height()/rcStream.Height();
	
	FX_FLOAT e = rcAnnot.left - rcStream.left * a;
	FX_FLOAT f = rcAnnot.bottom - rcStream.bottom * d;
	return CFX_AffineMatrix(a, 0, 0, d, e, f);
}

void GetOffset(FX_FLOAT& fa, FX_FLOAT& fd, FX_FLOAT& fe, FX_FLOAT& ff, CPDF_Rect rcAnnot, CPDF_Rect rcStream, CFX_AffineMatrix matrix)
{
	FX_FLOAT fStreamWidth = 0.0f;
	FX_FLOAT fStreamHeight = 0.0f;


	
	if (matrix.a != 0 && matrix.d != 0)
	{
		fStreamWidth = rcStream.right - rcStream.left;
		fStreamHeight = rcStream.top - rcStream.bottom;
	}
	else
	{
		fStreamWidth = rcStream.top - rcStream.bottom;
		fStreamHeight = rcStream.right - rcStream.left;
	}
	
	FX_FLOAT x1 = matrix.a * rcStream.left + matrix.c * rcStream.bottom + matrix.e;
	FX_FLOAT y1 = matrix.b * rcStream.left + matrix.d * rcStream.bottom + matrix.f;
	FX_FLOAT x2 = matrix.a * rcStream.left + matrix.c * rcStream.top + matrix.e;
	FX_FLOAT y2 = matrix.b * rcStream.left + matrix.d * rcStream.top + matrix.f;
	FX_FLOAT x3 = matrix.a * rcStream.right + matrix.c * rcStream.bottom + matrix.e;
	FX_FLOAT y3 = matrix.b * rcStream.right + matrix.d * rcStream.bottom + matrix.f;
	FX_FLOAT x4 = matrix.a * rcStream.right + matrix.c * rcStream.top + matrix.e;
	FX_FLOAT y4 = matrix.b * rcStream.right + matrix.d * rcStream.top + matrix.f;
	
	FX_FLOAT left = FX_MIN(FX_MIN(x1, x2), FX_MIN(x3, x4));
	FX_FLOAT bottom = FX_MIN(FX_MIN(y1, y2), FX_MIN(y3, y4));
	
	fa = (rcAnnot.right - rcAnnot.left)/fStreamWidth;
	fd = (rcAnnot.top - rcAnnot.bottom)/fStreamHeight;
	fe = rcAnnot.left - left * fa;
	ff = rcAnnot.bottom - bottom * fd;
}


DLLEXPORT int STDCALL FPDFPage_Flatten( FPDF_PAGE page, int nFlag)
{
	if (!page)
	{
		return FLATTEN_FAIL;
	}

	CPDF_Page * pPage = (CPDF_Page*)( page );
	CPDF_Document * pDocument = pPage->m_pDocument;
	CPDF_Dictionary * pPageDict = pPage->m_pFormDict;
	
	if ( !pDocument || !pPageDict )
	{
		return FLATTEN_FAIL;
	}

	CPDF_ObjectArray ObjectArray;
	CPDF_RectArray  RectArray;

	int iRet = FLATTEN_FAIL;
	iRet = ParserAnnots( pDocument, pPageDict, &RectArray, &ObjectArray, nFlag);
	if (iRet == FLATTEN_NOTINGTODO)
	{
		return FLATTEN_NOTINGTODO;
	}else if (iRet == FLATTEN_FAIL)
	{
		return FLATTEN_FAIL;
	}
	
	CPDF_Rect rcOriginalCB;
	CPDF_Rect rcMerger = CalculateRect( &RectArray );
	CPDF_Rect rcOriginalMB = pPageDict->GetRect("MediaBox");

	if (pPageDict->KeyExist("CropBox"))
		rcOriginalMB = pPageDict->GetRect("CropBox");
	
	if (rcOriginalMB.IsEmpty()) 	
	{
		rcOriginalMB = CPDF_Rect(0.0f, 0.0f, 612.0f, 792.0f);
	}
	
	rcMerger.left = rcMerger.left < rcOriginalMB.left? rcOriginalMB.left : rcMerger.left;
	rcMerger.right = rcMerger.right > rcOriginalMB.right? rcOriginalMB.right : rcMerger.right;
	rcMerger.top = rcMerger.top > rcOriginalMB.top? rcOriginalMB.top : rcMerger.top;
	rcMerger.bottom = rcMerger.bottom < rcOriginalMB.bottom? rcOriginalMB.bottom : rcMerger.bottom;
	
	if (pPageDict->KeyExist("ArtBox"))
		rcOriginalCB = pPageDict->GetRect("ArtBox");
	else
		rcOriginalCB = rcOriginalMB;

	if (!rcOriginalMB.IsEmpty())
	{
		CPDF_Array* pMediaBox = FX_NEW CPDF_Array();	

		pMediaBox->Add(FX_NEW CPDF_Number(rcOriginalMB.left));
		pMediaBox->Add(FX_NEW CPDF_Number(rcOriginalMB.bottom));
		pMediaBox->Add(FX_NEW CPDF_Number(rcOriginalMB.right));
		pMediaBox->Add(FX_NEW CPDF_Number(rcOriginalMB.top));

		pPageDict->SetAt("MediaBox",pMediaBox);
	}
	
	if (!rcOriginalCB.IsEmpty())
	{
		CPDF_Array* pCropBox = FX_NEW CPDF_Array();
		pCropBox->Add(FX_NEW CPDF_Number(rcOriginalCB.left));
		pCropBox->Add(FX_NEW CPDF_Number(rcOriginalCB.bottom));
		pCropBox->Add(FX_NEW CPDF_Number(rcOriginalCB.right));
		pCropBox->Add(FX_NEW CPDF_Number(rcOriginalCB.top));
		pPageDict->SetAt("ArtBox", pCropBox);
	}

	CPDF_Dictionary* pRes = NULL;
	pRes = pPageDict->GetDict("Resources");
	if (!pRes)
	{
		pRes = FX_NEW CPDF_Dictionary;
		pPageDict->SetAt( "Resources", pRes );
	}

	CPDF_Stream* pNewXObject = FX_NEW CPDF_Stream(NULL, 0, FX_NEW CPDF_Dictionary);
	FX_DWORD dwObjNum = pDocument->AddIndirectObject(pNewXObject);
	CPDF_Dictionary* pPageXObject = pRes->GetDict("XObject");
	if (!pPageXObject)
	{
		pPageXObject = FX_NEW CPDF_Dictionary;
		pRes->SetAt("XObject", pPageXObject);
	}

	CFX_ByteString key = "";
	int nStreams = ObjectArray.GetSize();

	if (nStreams > 0)
	{
		for (int iKey = 0; /*iKey < 100*/; iKey++)
		{
			char sExtend[5] = {0};
			FXSYS_itoa(iKey, sExtend, 10);
			key = CFX_ByteString("FFT") + CFX_ByteString(sExtend);

			if (!pPageXObject->KeyExist(key))
				break;
		}
	}

	SetPageContents(key, pPageDict, pDocument);

	CPDF_Dictionary* pNewXORes = NULL;

	if (!key.IsEmpty())
	{
		pPageXObject->SetAtReference(key, pDocument, dwObjNum);
		CPDF_Dictionary* pNewOXbjectDic = pNewXObject->GetDict();
		pNewXORes = FX_NEW CPDF_Dictionary;
		pNewOXbjectDic->SetAt("Resources", pNewXORes);
		pNewOXbjectDic->SetAtName("Type", "XObject");
		pNewOXbjectDic->SetAtName("Subtype", "Form");
		pNewOXbjectDic->SetAtInteger("FormType", 1);
		pNewOXbjectDic->SetAtName("Name", "FRM");
		CPDF_Rect rcBBox = pPageDict->GetRect("ArtBox"); 
		pNewOXbjectDic->SetAtRect("BBox", rcBBox);
	}
	
	for (int i = 0; i < nStreams; i++)
	{
		CPDF_Dictionary* pAnnotDic = ObjectArray.GetAt(i);
		if (!pAnnotDic)continue;

		CPDF_Rect rcAnnot = pAnnotDic->GetRect("Rect");
		rcAnnot.Normalize();

		CFX_ByteString sAnnotState = pAnnotDic->GetString("AS");
		CPDF_Dictionary* pAnnotAP = pAnnotDic->GetDict("AP");
		if (!pAnnotAP)continue;

		CPDF_Stream* pAPStream = pAnnotAP->GetStream("N");
		if (!pAPStream)
		{
			CPDF_Dictionary* pAPDic = pAnnotAP->GetDict("N");
			if (!pAPDic)continue;

			if (!sAnnotState.IsEmpty())
			{
				pAPStream = pAPDic->GetStream(sAnnotState);
			}
			else
			{
				FX_POSITION pos = pAPDic->GetStartPos();
				if (pos)
				{
					CFX_ByteString sKey;
					CPDF_Object* pFirstObj = pAPDic->GetNextElement(pos, sKey);
					if (pFirstObj)
					{
						if (pFirstObj->GetType() == PDFOBJ_REFERENCE)
							pFirstObj = pFirstObj->GetDirect();
						
						if (pFirstObj->GetType() != PDFOBJ_STREAM)
							continue;

						pAPStream = (CPDF_Stream*)pFirstObj;
					}
				}
			}
		}

		if (!pAPStream)continue;

		CPDF_Dictionary* pAPDic = pAPStream->GetDict();
		CFX_AffineMatrix matrix = pAPDic->GetMatrix("Matrix");

		CPDF_Rect rcStream;
		if (pAPDic->KeyExist("Rect"))
			rcStream = pAPDic->GetRect("Rect");
		else if (pAPDic->KeyExist("BBox"))
			rcStream = pAPDic->GetRect("BBox");

		if (rcStream.IsEmpty())continue;

		CPDF_Object* pObj = pAPStream;

		if (pObj)
		{		
			CPDF_Dictionary* pObjDic = pObj->GetDict();
			if (pObjDic)
			{
				pObjDic->SetAtName("Type", "XObject");
				pObjDic->SetAtName("Subtype", "Form");
			}
		}

		CPDF_Dictionary* pXObject = pNewXORes->GetDict("XObject");
		if (!pXObject)
		{
			pXObject = FX_NEW CPDF_Dictionary;
			pNewXORes->SetAt("XObject", pXObject);
		}

		CFX_ByteString sFormName;
		sFormName.Format("F%d", i);
		FX_DWORD dwObjNum = pDocument->AddIndirectObject(pObj);
		pXObject->SetAtReference(sFormName, pDocument, dwObjNum);

		CPDF_StreamAcc acc;
		acc.LoadAllData(pNewXObject);

		FX_LPCBYTE pData = acc.GetData();
		CFX_ByteString sStream(pData, acc.GetSize());
		CFX_ByteString sTemp;

		if (matrix.IsIdentity())
		{
			matrix.a = 1.0f;
			matrix.b = 0.0f;
			matrix.c = 0.0f;
			matrix.d = 1.0f;
			matrix.e = 0.0f;
			matrix.f = 0.0f;
		}

		CFX_AffineMatrix m = GetMatrix(rcAnnot, rcStream, matrix);
		sTemp.Format("q %f 0 0 %f %f %f cm /%s Do Q\n", m.a, m.d, m.e, m.f, sFormName.c_str());
		sStream += sTemp;

		pNewXObject->SetData((FX_LPCBYTE)sStream, sStream.GetLength(), FALSE, FALSE);
	}
	pPageDict->RemoveAt( "Annots" );

	ObjectArray.RemoveAll();
	RectArray.RemoveAll();

	return FLATTEN_SUCCESS;
}
