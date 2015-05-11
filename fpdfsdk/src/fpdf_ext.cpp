// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../public/fpdf_ext.h"
#include "../include/fsdk_define.h"

#define  FPDFSDK_UNSUPPORT_CALL 100

class CFSDK_UnsupportInfo_Adapter
{
public:
	CFSDK_UnsupportInfo_Adapter(UNSUPPORT_INFO* unsp_info){ m_unsp_info = unsp_info;}
//	FX_BOOL NeedToPauseNow();
	void ReportError(int nErrorType);

private:
	UNSUPPORT_INFO* m_unsp_info;
};

void CFSDK_UnsupportInfo_Adapter::ReportError(int nErrorType)
{
	if(m_unsp_info && m_unsp_info->FSDK_UnSupport_Handler)
	{
		m_unsp_info->FSDK_UnSupport_Handler(m_unsp_info,nErrorType);
	}
}

void FreeUnsupportInfo(FX_LPVOID pData)
{
	CFSDK_UnsupportInfo_Adapter * pAdapter = (CFSDK_UnsupportInfo_Adapter *)pData;
	delete pAdapter;
}

FX_BOOL FPDF_UnSupportError(int nError)
{
	CFSDK_UnsupportInfo_Adapter * pAdapter = (CFSDK_UnsupportInfo_Adapter *)CPDF_ModuleMgr::Get()->GetPrivateData((void *)FPDFSDK_UNSUPPORT_CALL);

	if(!pAdapter)
		return FALSE;
	pAdapter->ReportError(nError);
	return TRUE;
}	

DLLEXPORT FPDF_BOOL STDCALL FSDK_SetUnSpObjProcessHandler(UNSUPPORT_INFO* unsp_info)
{
	if (!unsp_info || unsp_info->version!=1)
		return FALSE;
	CFSDK_UnsupportInfo_Adapter * pAdapter = new CFSDK_UnsupportInfo_Adapter(unsp_info);

	CPDF_ModuleMgr::Get()->SetPrivateData((void *)FPDFSDK_UNSUPPORT_CALL,pAdapter, &FreeUnsupportInfo);

	return TRUE;
}

void CheckUnSupportAnnot(CPDF_Document * pDoc, CPDF_Annot* pPDFAnnot)
{
	CFX_ByteString cbSubType = pPDFAnnot->GetSubType();
	if(cbSubType.Compare("3D") == 0)
	{
		FPDF_UnSupportError(FPDF_UNSP_ANNOT_3DANNOT);
	}
	else if(cbSubType.Compare("Screen") ==0)
	{
		CPDF_Dictionary* pAnnotDict = pPDFAnnot->m_pAnnotDict;
		CFX_ByteString cbString;
		if(pAnnotDict->KeyExist("IT"))
			cbString = pAnnotDict->GetString("IT");
		if(cbString.Compare("Img") != 0)
			FPDF_UnSupportError(FPDF_UNSP_ANNOT_SCREEN_MEDIA);
	}
	else if(cbSubType.Compare("Movie") ==0)
	{
		FPDF_UnSupportError(FPDF_UNSP_ANNOT_MOVIE);
	}
	else if(cbSubType.Compare("Sound") ==0)
	{
		FPDF_UnSupportError(FPDF_UNSP_ANNOT_SOUND);
	}
	else if(cbSubType.Compare("RichMedia") ==0)
	{
		FPDF_UnSupportError(FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA);
	}
	else if(cbSubType.Compare("FileAttachment") ==0)
	{
		FPDF_UnSupportError(FPDF_UNSP_ANNOT_ATTACHMENT);
	}
	else if(cbSubType.Compare("Widget") ==0)
	{
		CPDF_Dictionary* pAnnotDict = pPDFAnnot->m_pAnnotDict;
		CFX_ByteString cbString;
		if(pAnnotDict->KeyExist("FT"))
		{
			cbString = pAnnotDict->GetString("FT");
		}	
		if(cbString.Compare("Sig") == 0)
		{
			FPDF_UnSupportError(FPDF_UNSP_ANNOT_SIG);
		}
	}
	
}

FX_BOOL CheckSharedForm(CXML_Element * pElement, CFX_ByteString cbName)
{
	int count = pElement->CountAttrs();
	int i=0;
	for (i = 0; i < count; i++) 
	{
		CFX_ByteString space, name; 
		CFX_WideString value;
		pElement->GetAttrByIndex(i, space, name, value);
		if (space == FX_BSTRC("xmlns") && name == FX_BSTRC("adhocwf") && value ==  L"http://ns.adobe.com/AcrobatAdhocWorkflow/1.0/")
		{
			CXML_Element *pVersion = pElement->GetElement("adhocwf",cbName);
			if (!pVersion)
				continue;
			CFX_WideString wsContent = pVersion->GetContent(0); // == 1.1
			int nType = wsContent.GetInteger();
			switch(nType)
			{
			case 1:
				FPDF_UnSupportError(FPDF_UNSP_DOC_SHAREDFORM_ACROBAT);
				break;
			case 2:
				FPDF_UnSupportError(FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM);
				break;
			case 0:
				FPDF_UnSupportError(FPDF_UNSP_DOC_SHAREDFORM_EMAIL);
				break;
			}
		}
	}

	FX_DWORD nCount = pElement->CountChildren();
	for(i=0; i<(int)nCount; i++)
	{
		CXML_Element::ChildType childType = pElement->GetChildType(i);
		if(childType == CXML_Element::Element)
		{
			CXML_Element * pChild = pElement->GetElement(i);
			if(CheckSharedForm(pChild, cbName))
				return TRUE;
		}
	}
	return FALSE;
}

void CheckUnSupportError(CPDF_Document * pDoc, FX_DWORD err_code)
{
	// Security
	if(err_code == FPDF_ERR_SECURITY)
	{
		FPDF_UnSupportError(FPDF_UNSP_DOC_SECURITY);
		return ;
	}
	if(!pDoc)
		return ;

	// Portfolios and Packages 
	CPDF_Dictionary * pRootDict = pDoc->GetRoot();
	if(pRootDict)
	{
		CFX_ByteString cbString;
		if(pRootDict->KeyExist("Collection"))
		{
			FPDF_UnSupportError(FPDF_UNSP_DOC_PORTABLECOLLECTION);
			return ;
		}
		if(pRootDict->KeyExist("Names"))
		{
			CPDF_Dictionary* pNameDict = pRootDict->GetDict("Names");
			if (pNameDict && pNameDict->KeyExist("EmbeddedFiles"))
			{
				FPDF_UnSupportError(FPDF_UNSP_DOC_ATTACHMENT);
				return;
			}
			else if (pNameDict && pNameDict->KeyExist("JavaScript"))
			{
				CPDF_Dictionary* pJSDict = pNameDict->GetDict("JavaScript");
				CPDF_Array * pArray = pJSDict ? pJSDict->GetArray("Names") : NULL;
				if (pArray) {
					int nCount = pArray->GetCount();
					for(int i=0; i<nCount; i++)
					{
						CFX_ByteString cbStr = pArray->GetString(i);
						if(cbStr.Compare("com.adobe.acrobat.SharedReview.Register") == 0)
						{
							FPDF_UnSupportError(FPDF_UNSP_DOC_SHAREDREVIEW);
							return;
						}
					}
				}
			}
		}
	}

	// SharedForm
	CPDF_Metadata metaData;
	metaData.LoadDoc(pDoc);
	CXML_Element * pElement = metaData.GetRoot();
	if(pElement)
		CheckSharedForm(pElement, "workflowType");

	// XFA Forms
	CPDF_InterForm * pInterForm = new CPDF_InterForm(pDoc,FALSE);
	if (pInterForm->HasXFAForm())
	{
		FPDF_UnSupportError(FPDF_UNSP_DOC_XFAFORM);
	}
	delete pInterForm;
}

DLLEXPORT int FPDFDoc_GetPageMode(FPDF_DOCUMENT document)
{
	if (!document) return PAGEMODE_UNKNOWN;
	CPDF_Dictionary *pRoot = ((CPDF_Document*)document)->GetRoot();
	if (!pRoot)
		return PAGEMODE_UNKNOWN;
	CPDF_Object* pName = pRoot->GetElement("PageMode");
	if (!pName)
		return PAGEMODE_USENONE;
	CFX_ByteString strPageMode = pName->GetString();
	
	if (strPageMode.IsEmpty()||strPageMode.EqualNoCase(FX_BSTR("UseNone")))
		return PAGEMODE_USENONE;
	else if (strPageMode.EqualNoCase(FX_BSTR("UseOutlines")))
		return PAGEMODE_USEOUTLINES;
	else if (strPageMode.EqualNoCase(FX_BSTR("UseThumbs")))
		return PAGEMODE_USETHUMBS;
	else if (strPageMode.EqualNoCase(FX_BSTR("FullScreen")))
		return PAGEMODE_FULLSCREEN;
	else if (strPageMode.EqualNoCase(FX_BSTR("UseOC")))
		return PAGEMODE_USEOC;
	else if (strPageMode.EqualNoCase(FX_BSTR("UseAttachments")))
		return PAGEMODE_USEATTACHMENTS;

	return PAGEMODE_UNKNOWN;
}
