// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fpdfedit.h"


#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

DLLEXPORT FPDF_DOCUMENT STDCALL FPDF_CreateNewDocument()
{
	CPDF_Document* pDoc = FX_NEW CPDF_Document;
	if (!pDoc)
		return NULL;
	pDoc->CreateNewDoc();
	time_t currentTime;

	CFX_ByteString DateStr;

	if(FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
	{
		if ( -1 != time( &currentTime ) )
		{
			tm * pTM = localtime( &currentTime );
			if ( pTM )
			{
				DateStr.Format(	"D:%04d%02d%02d%02d%02d%02d", pTM->tm_year+1900, pTM->tm_mon+1,
					pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec );
			}
		}
	}
	
	CPDF_Dictionary* pInfoDict = NULL;
	pInfoDict = pDoc->GetInfo();
	if (pInfoDict)
	{
		if(FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
			pInfoDict->SetAt("CreationDate", new CPDF_String(DateStr));
		pInfoDict->SetAt("Creator",FX_NEW CPDF_String(L"PDFium"));
	}

	return pDoc;
}

DLLEXPORT void STDCALL FPDFPage_Delete(FPDF_DOCUMENT document, int page_index)
{
	CPDF_Document* pDoc = (CPDF_Document*)document;
	if (pDoc == NULL) 
		return;
	if (page_index < 0 || page_index >= pDoc->GetPageCount()) 
		return;

	pDoc->DeletePage(page_index);
}

DLLEXPORT FPDF_PAGE STDCALL FPDFPage_New(FPDF_DOCUMENT document, int page_index, double width, double height)
{
	if (!document)
		return NULL;

//	CPDF_Parser* pParser = (CPDF_Parser*)document;
	CPDF_Document* pDoc = (CPDF_Document*)document;
	if(page_index < 0)
		page_index = 0;
	if(pDoc->GetPageCount()<page_index)
		page_index = pDoc->GetPageCount();
//	if (page_index < 0 || page_index >= pDoc->GetPageCount()) 
//		return NULL;

	CPDF_Dictionary* pPageDict = pDoc->CreateNewPage(page_index);
	if(!pPageDict)
		return NULL;
	CPDF_Array* pMediaBoxArray = FX_NEW CPDF_Array;
	pMediaBoxArray->Add(FX_NEW CPDF_Number(0));
	pMediaBoxArray->Add(FX_NEW CPDF_Number(0));
	pMediaBoxArray->Add(FX_NEW CPDF_Number(FX_FLOAT(width)));
	pMediaBoxArray->Add(FX_NEW CPDF_Number(FX_FLOAT(height)));

	pPageDict->SetAt("MediaBox", pMediaBoxArray);
	pPageDict->SetAt("Rotate", FX_NEW CPDF_Number(0));
	pPageDict->SetAt("Resources", FX_NEW CPDF_Dictionary);

	CPDF_Page* pPage = FX_NEW CPDF_Page;
	pPage->Load(pDoc,pPageDict);
	pPage->ParseContent();

	return pPage;
}

DLLEXPORT int STDCALL FPDFPage_GetRotation(FPDF_PAGE page)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type") || !pPage->m_pFormDict->GetElement("Type")->GetDirect()
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return -1;
	}
	CPDF_Dictionary* pDict = pPage->m_pFormDict;

	int rotate = 0;
	if(pDict != NULL)
	{
		if (pDict->KeyExist("Rotate"))
			rotate = pDict->GetElement("Rotate")->GetDirect()? pDict->GetElement("Rotate")->GetDirect()->GetInteger() / 90 : 0;
		else
		{
			if(pDict->KeyExist("Parent"))
			{
				CPDF_Dictionary* pPages = (CPDF_Dictionary*)pDict->GetElement("Parent")->GetDirect();
				while(pPages)
				{
					if(pPages->KeyExist("Rotate"))
					{
						rotate = pPages->GetElement("Rotate")->GetDirect()? pPages->GetElement("Rotate")->GetDirect()->GetInteger() / 90 : 0;
						break;
					}
					else if(pPages->KeyExist("Parent"))
						pPages = (CPDF_Dictionary*)pPages->GetElement("Parent")->GetDirect();
					else break;
				}
			}
		}
	}
	else
	{
		return -1;
	}
	
	return rotate;
}

DLLEXPORT void STDCALL FPDFPage_InsertObject(FPDF_PAGE page, FPDF_PAGEOBJECT page_obj)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type") || !pPage->m_pFormDict->GetElement("Type")->GetDirect()
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return;
	}
	CPDF_PageObject* pPageObj = (CPDF_PageObject*)page_obj;
	if(pPageObj == NULL)
		return;
	FX_POSITION LastPersition = pPage->GetLastObjectPosition();

	pPage->InsertObject(LastPersition, pPageObj);
	switch(pPageObj->m_Type)
	{
	case FPDF_PAGEOBJ_PATH:
		{
			CPDF_PathObject* pPathObj = (CPDF_PathObject*)pPageObj;
			pPathObj->CalcBoundingBox();
			break;
		}
	case FPDF_PAGEOBJ_TEXT:
		{
			//	CPDF_PathObject* pPathObj = (CPDF_PathObject*)pPageObj;
			//	pPathObj->CalcBoundingBox();
			break;
		}
	case FPDF_PAGEOBJ_IMAGE:
		{
			CPDF_ImageObject* pImageObj = (CPDF_ImageObject*)pPageObj;
			pImageObj->CalcBoundingBox();
			break;
		}
	case FPDF_PAGEOBJ_SHADING:
		{
			CPDF_ShadingObject* pShadingObj = (CPDF_ShadingObject*)pPageObj;
			pShadingObj->CalcBoundingBox();
			break;
		}
	case FPDF_PAGEOBJ_FORM:
		{
			CPDF_FormObject* pFormObj = (CPDF_FormObject*)pPageObj;
			pFormObj->CalcBoundingBox();
			break;
		}
	default:
		break;
	}

	//	pPage->ParseContent();
	//pPage->GenerateContent();

}

DLLEXPORT int STDCALL FPDFPage_CountObject(FPDF_PAGE page)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type") || !pPage->m_pFormDict->GetElement("Type")->GetDirect()
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return -1;
	}
	return pPage->CountObjects();
//	return 0;
}

DLLEXPORT FPDF_PAGEOBJECT STDCALL FPDFPage_GetObject(FPDF_PAGE page, int index)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type")
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return NULL;
	}
	return pPage->GetObjectByIndex(index);
//	return NULL;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_HasTransparency(FPDF_PAGE page)
{
	if(!page) return FALSE;
	CPDF_Page* pPage = (CPDF_Page*)page;

	return pPage->BackgroundAlphaNeeded();
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPageObj_HasTransparency(FPDF_PAGEOBJECT pageObject)
{
	if(!pageObject) return FALSE;
	CPDF_PageObject* pPageObj = (CPDF_PageObject*)pageObject;

	const CPDF_GeneralStateData* pGeneralState = pPageObj->m_GeneralState;
	int blend_type = pGeneralState ? pGeneralState->m_BlendType : FXDIB_BLEND_NORMAL;
	if (blend_type != FXDIB_BLEND_NORMAL) return TRUE;

	CPDF_Dictionary* pSMaskDict = pGeneralState ? (CPDF_Dictionary*)pGeneralState->m_pSoftMask : NULL;
	if(pSMaskDict) return TRUE;

	if(pGeneralState && pGeneralState->m_FillAlpha != 1.0f)
		return TRUE;

	if(pPageObj->m_Type == PDFPAGE_PATH)
	{
		if(pGeneralState && pGeneralState->m_StrokeAlpha != 1.0f)
			return TRUE;
	}

	if(pPageObj->m_Type == PDFPAGE_FORM)
	{
		CPDF_FormObject* pFormObj = (CPDF_FormObject*)pPageObj;
		if(pFormObj->m_pForm && (pFormObj->m_pForm->m_Transparency & PDFTRANS_ISOLATED))
			return TRUE;
		if(pFormObj->m_pForm && (!(pFormObj->m_pForm->m_Transparency & PDFTRANS_ISOLATED) && (pFormObj->m_pForm->m_Transparency & PDFTRANS_GROUP)))
			return TRUE;
	}
	return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDFPage_GenerateContent(FPDF_PAGE page)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type") || !pPage->m_pFormDict->GetElement("Type")->GetDirect()
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return FALSE;
	}
	CPDF_PageContentGenerate CG(pPage);
	CG.GenerateContent();

	return TRUE;
}

DLLEXPORT void STDCALL FPDFPageObj_Transform(FPDF_PAGEOBJECT page_object,
			 double a, double b, double c, double d, double e, double f)  
{
	CPDF_PageObject* pPageObj = (CPDF_PageObject*)page_object;
	if(pPageObj == NULL)
		return;
//PDF_ImageObject* pImageObj = FX_NEW CPDF_ImageObject;
	CFX_AffineMatrix matrix((FX_FLOAT)a,(FX_FLOAT)b,(FX_FLOAT)c,(FX_FLOAT)d,(FX_FLOAT)e,(FX_FLOAT)f);
	pPageObj->Transform(matrix);
}
DLLEXPORT void STDCALL FPDFPage_TransformAnnots(FPDF_PAGE page,
											   double a, double b, double c, double d, double e, double f)
{
	if(page == NULL)
		return;
	CPDF_Page* pPage = (CPDF_Page*)page;
	CPDF_AnnotList AnnotList(pPage);
	for (int i=0; i<AnnotList.Count();i++)
	{
		CPDF_Annot* pAnnot = AnnotList.GetAt(i);
		// transformAnnots Rectangle
		CPDF_Rect rect;
		pAnnot->GetRect(rect);
		CFX_AffineMatrix matrix((FX_FLOAT)a,(FX_FLOAT)b,(FX_FLOAT)c,(FX_FLOAT)d,(FX_FLOAT)e,(FX_FLOAT)f);
		rect.Transform(&matrix);
		CPDF_Array *pRectArray = NULL;
		pRectArray = pAnnot->m_pAnnotDict->GetArray("Rect");
		if (!pRectArray) pRectArray=CPDF_Array::Create();
		pRectArray->SetAt(0,FX_NEW CPDF_Number(rect.left));
		pRectArray->SetAt(1,FX_NEW CPDF_Number(rect.bottom));
		pRectArray->SetAt(2,FX_NEW CPDF_Number(rect.right));
		pRectArray->SetAt(3,FX_NEW CPDF_Number(rect.top));
		pAnnot->m_pAnnotDict->SetAt("Rect",pRectArray);

		//Transform AP's rectangle
		//To Do

	}

}

DLLEXPORT void STDCALL FPDFPage_SetRotation(FPDF_PAGE page, int rotate)
{
	CPDF_Page* pPage = (CPDF_Page*)page;
	if (!pPage || !pPage->m_pFormDict || !pPage->m_pFormDict->KeyExist("Type") || !pPage->m_pFormDict->GetElement("Type")->GetDirect()
		|| pPage->m_pFormDict->GetElement("Type")->GetDirect()->GetString().Compare("Page"))
	{
		return;
	}
	CPDF_Dictionary* pDict = pPage->m_pFormDict;
	rotate %=4;

	pDict->SetAt("Rotate", FX_NEW CPDF_Number(rotate * 90));
}
