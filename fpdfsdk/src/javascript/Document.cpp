// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/Document.h"
#include "../../include/javascript/JS_EventHandler.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/app.h"
#include "../../include/javascript/Field.h"
#include "../../include/javascript/Icon.h"
#include "../../include/javascript/Field.h"

#include "../../../third_party/base/numerics/safe_math.h"

static v8::Isolate* GetIsolate(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context *)cc;
	ASSERT(pContext != NULL);

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	return pRuntime->GetIsolate();
}

BEGIN_JS_STATIC_CONST(CJS_PrintParamsObj)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_PrintParamsObj)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_PrintParamsObj)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_PrintParamsObj, PrintParamsObj)

PrintParamsObj::PrintParamsObj(CJS_Object* pJSObject)
: CJS_EmbedObj(pJSObject)
{
	bUI = TRUE;
	nStart = 0;
	nEnd = 0;
	bSilent = FALSE;
	bShrinkToFit = FALSE;
	bPrintAsImage = FALSE;
	bReverse = FALSE;
	bAnnotations = TRUE;
}

/* ---------------------- Document ---------------------- */

#define MINWIDTH  5.0f
#define MINHEIGHT 5.0f

BEGIN_JS_STATIC_CONST(CJS_Document)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Document)
	JS_STATIC_PROP_ENTRY(ADBE)
	JS_STATIC_PROP_ENTRY(author)
	JS_STATIC_PROP_ENTRY(baseURL)
	JS_STATIC_PROP_ENTRY(bookmarkRoot)
	JS_STATIC_PROP_ENTRY(calculate)
	JS_STATIC_PROP_ENTRY(Collab)
	JS_STATIC_PROP_ENTRY(creationDate)
	JS_STATIC_PROP_ENTRY(creator)
	JS_STATIC_PROP_ENTRY(delay)
	JS_STATIC_PROP_ENTRY(dirty)
	JS_STATIC_PROP_ENTRY(documentFileName)
	JS_STATIC_PROP_ENTRY(external)
	JS_STATIC_PROP_ENTRY(filesize)
	JS_STATIC_PROP_ENTRY(icons)
	JS_STATIC_PROP_ENTRY(info)
	JS_STATIC_PROP_ENTRY(keywords)
	JS_STATIC_PROP_ENTRY(layout)
	JS_STATIC_PROP_ENTRY(media)
	JS_STATIC_PROP_ENTRY(modDate)
	JS_STATIC_PROP_ENTRY(mouseX)
	JS_STATIC_PROP_ENTRY(mouseY)
	JS_STATIC_PROP_ENTRY(numFields)
	JS_STATIC_PROP_ENTRY(numPages)
	JS_STATIC_PROP_ENTRY(pageNum)
	JS_STATIC_PROP_ENTRY(pageWindowRect)
	JS_STATIC_PROP_ENTRY(path)
	JS_STATIC_PROP_ENTRY(producer)
	JS_STATIC_PROP_ENTRY(subject)
	JS_STATIC_PROP_ENTRY(title)
	JS_STATIC_PROP_ENTRY(zoom)
	JS_STATIC_PROP_ENTRY(zoomType)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Document)
	JS_STATIC_METHOD_ENTRY(addAnnot,0)
	JS_STATIC_METHOD_ENTRY(addField, 4)
	JS_STATIC_METHOD_ENTRY(addLink, 0)
	JS_STATIC_METHOD_ENTRY(addIcon, 0)
	JS_STATIC_METHOD_ENTRY(calculateNow, 0)
	JS_STATIC_METHOD_ENTRY(closeDoc, 0)
	JS_STATIC_METHOD_ENTRY(createDataObject, 0)
	JS_STATIC_METHOD_ENTRY(deletePages, 2)
	JS_STATIC_METHOD_ENTRY(exportAsText, 3)
	JS_STATIC_METHOD_ENTRY(exportAsFDF, 6)
	JS_STATIC_METHOD_ENTRY(exportAsXFDF, 5)
	JS_STATIC_METHOD_ENTRY(extractPages, 3)
	JS_STATIC_METHOD_ENTRY(getAnnot, 0)
	JS_STATIC_METHOD_ENTRY(getAnnots, 2)
	JS_STATIC_METHOD_ENTRY(getAnnot3D, 2)
	JS_STATIC_METHOD_ENTRY(getAnnots3D, 1)
	JS_STATIC_METHOD_ENTRY(getField, 1)
	JS_STATIC_METHOD_ENTRY(getIcon, 0)
	JS_STATIC_METHOD_ENTRY(getLinks, 0)
	JS_STATIC_METHOD_ENTRY(getNthFieldName, 1)
	JS_STATIC_METHOD_ENTRY(getOCGs, 0)
	JS_STATIC_METHOD_ENTRY(getPageBox, 0)
	JS_STATIC_METHOD_ENTRY(getPageNthWord, 3)
	JS_STATIC_METHOD_ENTRY(getPageNthWordQuads, 2)
	JS_STATIC_METHOD_ENTRY(getPageNumWords, 1)
	JS_STATIC_METHOD_ENTRY(getPrintParams, 0)
	JS_STATIC_METHOD_ENTRY(getURL, 2)
	JS_STATIC_METHOD_ENTRY(importAnFDF, 1)
	JS_STATIC_METHOD_ENTRY(importAnXFDF, 1)
	JS_STATIC_METHOD_ENTRY(importTextData, 2)
	JS_STATIC_METHOD_ENTRY(insertPages, 4)
	JS_STATIC_METHOD_ENTRY(mailForm, 6)
	JS_STATIC_METHOD_ENTRY(print, 9)
	JS_STATIC_METHOD_ENTRY(removeField, 1)
	JS_STATIC_METHOD_ENTRY(replacePages, 4)
	JS_STATIC_METHOD_ENTRY(resetForm, 1)
	JS_STATIC_METHOD_ENTRY(removeIcon, 0)
	JS_STATIC_METHOD_ENTRY(saveAs, 5)
	JS_STATIC_METHOD_ENTRY(submitForm, 23)
	JS_STATIC_METHOD_ENTRY(mailDoc, 0)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Document, Document)

FX_BOOL	CJS_Document::InitInstance(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);

	Document* pDoc = (Document*)GetEmbedObject();
	ASSERT(pDoc != NULL);

	pDoc->AttachDoc(pContext->GetReaderDocument());
	pDoc->SetIsolate(pContext->GetJSRuntime()->GetIsolate());
	return TRUE;
};

/* --------------------------------- Document --------------------------------- */

Document::Document(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject),
	m_isolate(NULL),
	m_pIconTree(NULL),
	m_pDocument(NULL),
	m_cwBaseURL(L""),
	m_bDelay(FALSE)
{
}

Document::~Document()
{
	if (m_pIconTree)
	{
		m_pIconTree->DeleteIconTree();
		delete m_pIconTree;
		m_pIconTree = NULL;
	}
	for (int i=0; i<m_DelayData.GetSize(); i++)
	{
		if (CJS_DelayData* pData = m_DelayData.GetAt(i))
		{
			delete pData;
			pData = NULL;
			m_DelayData.SetAt(i, NULL);

		}
	}

	m_DelayData.RemoveAll();
	m_DelayAnnotData.RemoveAll();
}

//the total number of fileds in document.
FX_BOOL Document::numFields(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting()) return FALSE;

	ASSERT(m_pDocument != NULL);

	CPDFSDK_InterForm *pInterForm = m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDF_InterForm *pPDFForm = pInterForm->GetInterForm();
	ASSERT(pPDFForm != NULL);

	vp << (int)pPDFForm->CountFields();

	return TRUE;
}

FX_BOOL Document::dirty(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsGetting())
	{
		if (m_pDocument->GetChangeMark())
			vp << true;
		else
			vp << false;
	}
	else
	{
		bool bChanged = false;

		vp >> bChanged;

		if (bChanged)
			m_pDocument->SetChangeMark();
		else
			m_pDocument->ClearChangeMark();
	}

	return TRUE;
}

FX_BOOL Document::ADBE(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsGetting())
	{
		vp.SetNull();
	}
	else
	{
	}

	return TRUE;
}

FX_BOOL Document::pageNum(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsGetting())
	{
		if (CPDFSDK_PageView* pPageView = m_pDocument->GetCurrentView())
		{
			vp << pPageView->GetPageIndex();
		}
	}
	else
	{
		int iPageCount = m_pDocument->GetPageCount();

		int iPageNum = 0;
		vp >> iPageNum;

		CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
		if(!pEnv)
			return FALSE;

		if (iPageNum >= 0 && iPageNum < iPageCount)
		{
			 pEnv->JS_docgotoPage(iPageNum);
		}
		else if (iPageNum >= iPageCount)
		{
			 pEnv->JS_docgotoPage(iPageCount-1);
		}
		else if (iPageNum < 0)
		{
			 pEnv->JS_docgotoPage(0);
		}
	}

	return TRUE;
}

FX_BOOL Document::ParserParams(JSObject* pObj,CJS_AnnotObj& annotobj)
{
	// Not supported.
	return TRUE;
}

FX_BOOL Document::addAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Not supported.
	return TRUE;
}

FX_BOOL Document::addField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Not supported.
	return TRUE;
}

FX_BOOL Document::exportAsText(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

FX_BOOL Document::exportAsFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

FX_BOOL Document::exportAsXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

//Maps a field object in PDF document to a JavaScript variable
//comment:
//note: the paremter cName, this is clue how to treat if the cName is not a valiable filed name in this document

FX_BOOL Document::getField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	v8::Isolate* isolate = GetIsolate(cc);
	ASSERT(m_pDocument != NULL);

	if (params.size() < 1) return FALSE;

	CFX_WideString wideName = params[0].operator CFX_WideString();

	CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
	ASSERT(pPDFForm != NULL);

	if (pPDFForm->CountFields(wideName) <= 0)
	{
		vRet.SetNull();
		return TRUE;
	}

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	JSFXObject  pFieldObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"Field"));

	CJS_Field * pJSField = (CJS_Field*)JS_GetPrivate(isolate,pFieldObj);
	ASSERT(pJSField != NULL);

	Field * pField = (Field *)pJSField->GetEmbedObject();
	ASSERT(pField != NULL);

	pField->AttachField(this, wideName);
	vRet = pJSField;

	return TRUE;
}

//Gets the name of the nth field in the document
FX_BOOL Document::getNthFieldName(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nIndex = params.size() > 0 ? (int)params[0] : -1;
	if (nIndex == -1) return FALSE;

	CPDFSDK_InterForm* pInterForm = m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
	ASSERT(pPDFForm != NULL);

	CPDF_FormField* pField = pPDFForm->GetField(nIndex);
	if (!pField)
		return FALSE;

	vRet = pField->GetFullName();
	return TRUE;
}

FX_BOOL Document::importAnFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

FX_BOOL Document::importAnXFDF(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

FX_BOOL Document::importTextData(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	// Unsafe, not supported.
	return TRUE;
}

//exports the form data and mails the resulting fdf file as an attachment to all recipients.
//comment: need reader supports
//note:
//int CPDFSDK_Document::mailForm(FX_BOOL bUI,String cto,string ccc,string cbcc,string cSubject,string cms);

FX_BOOL Document::mailForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) return FALSE;

	int iLength = params.size();

	FX_BOOL bUI = iLength > 0 ? (FX_BOOL)params[0] : TRUE;
	CFX_WideString cTo = iLength > 1 ? (FX_LPCWSTR)params[1].operator CFX_WideString() : L"";
	CFX_WideString cCc = iLength > 2 ? (FX_LPCWSTR)params[2].operator CFX_WideString() : L"";
	CFX_WideString cBcc = iLength > 3 ? (FX_LPCWSTR)params[3].operator CFX_WideString() : L"";
	CFX_WideString cSubject = iLength > 4 ? (FX_LPCWSTR)params[4].operator CFX_WideString() : L"";
	CFX_WideString cMsg = iLength > 5 ? (FX_LPCWSTR)params[5].operator CFX_WideString() : L"";

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_ByteTextBuf textBuf;
	if (!pInterForm->ExportFormToFDFTextBuf(textBuf))
		return FALSE;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CPDFDoc_Environment* pEnv = pContext->GetReaderApp();
	ASSERT(pEnv != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	pRuntime->BeginBlock();
	pEnv->JS_docmailForm(textBuf.GetBuffer(), textBuf.GetLength(), bUI, cTo.c_str(), cSubject.c_str(), cCc.c_str(), cBcc.c_str(), cMsg.c_str());
	pRuntime->EndBlock();
	return TRUE;
}

FX_BOOL Document::print(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	FX_BOOL bUI = TRUE;
	int nStart = 0;
	int nEnd = 0;
	FX_BOOL bSilent = FALSE;
	FX_BOOL bShrinkToFit = FALSE;
	FX_BOOL bPrintAsImage = FALSE;
	FX_BOOL bReverse = FALSE;
	FX_BOOL bAnnotations = FALSE;

	int nlength = params.size();
	if(nlength ==9)
	{
		if (params[8].GetType() == VT_fxobject)
		{
			JSFXObject pObj = (JSFXObject)params[8];
			{
				if (JS_GetObjDefnID(pObj) == JS_GetObjDefnID(*pRuntime, L"PrintParamsObj"))
				{
					if (CJS_Object* pJSObj = (CJS_Object*)params[8])
					{
							if (PrintParamsObj* pprintparamsObj = (PrintParamsObj*)pJSObj->GetEmbedObject())
							{
								bUI = pprintparamsObj->bUI;
								nStart = pprintparamsObj->nStart;
								nEnd = pprintparamsObj->nEnd;
								bSilent = pprintparamsObj->bSilent;
								bShrinkToFit = pprintparamsObj->bShrinkToFit;
								bPrintAsImage = pprintparamsObj->bPrintAsImage;
								bReverse = pprintparamsObj->bReverse;
								bAnnotations = pprintparamsObj->bAnnotations;
							}
					}
				}
			}
		}
	}
	else
	{
		if(nlength >= 1)
			 bUI = params[0];
		if(nlength >= 2)
			 nStart = (int)params[1];
		if(nlength >= 3)
			nEnd = (int)params[2];
		if(nlength >= 4)
			bSilent = params[3];
		if(nlength >= 5)
			bShrinkToFit = params[4];
		if(nlength >= 6)
			bPrintAsImage = params[5];
		if(nlength >= 7)
			bReverse = params[6];
		if(nlength >= 8)
			bAnnotations = params[7];
	}

	ASSERT(m_pDocument != NULL);

	if (CPDFDoc_Environment* pEnv = m_pDocument->GetEnv())
	{
		pEnv->JS_docprint(bUI, nStart, nEnd, bSilent, bShrinkToFit, bPrintAsImage, bReverse, bAnnotations);
		return TRUE;
	}
	return FALSE;
}

//removes the specified field from the document.
//comment:
//note: if the filed name is not retional, adobe is dumb for it.

FX_BOOL Document::removeField(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
		m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM))) return FALSE;

	if (params.size() < 1)
		return TRUE;

	CFX_WideString sFieldName = params[0].operator CFX_WideString();

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray widgets;
	pInterForm->GetWidgets(sFieldName, widgets);

	int nSize = widgets.GetSize();

	if (nSize > 0)
	{
		for (int i=0; i<nSize; i++)
		{
			CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)widgets[i];
			ASSERT(pWidget != NULL);

			CPDF_Rect rcAnnot = pWidget->GetRect();
			rcAnnot.left -= 1;
			rcAnnot.bottom -= 1;
			rcAnnot.right += 1;
			rcAnnot.top += 1;

			CFX_RectArray aRefresh;
			aRefresh.Add(rcAnnot);

			CPDF_Page* pPage = pWidget->GetPDFPage();
			ASSERT(pPage != NULL);

			CPDFSDK_PageView* pPageView = m_pDocument->GetPageView(pPage);
			pPageView->DeleteAnnot(pWidget);

			pPageView->UpdateRects(aRefresh);
		}
		m_pDocument->SetChangeMark();
	}

	return TRUE;
}

//reset filed values within a document.
//comment:
//note: if the fields names r not rational, aodbe is dumb for it.

FX_BOOL Document::resetForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
		m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM) ||
		m_pDocument->GetPermissions(FPDFPERM_FILL_FORM))) return FALSE;

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
	ASSERT(pPDFForm != NULL);

	v8::Isolate* isolate = GetIsolate(cc);
	CJS_Array aName(isolate);

	if (params.size() > 0)
	{
		switch (params[0].GetType())
		{
		default:
			aName.Attach(params[0]);
			break;
		case VT_string:
			aName.SetElement(0,params[0]);
			break;
		}

		CFX_PtrArray aFields;

		for (int i=0,isz=aName.GetLength(); i<isz; i++)
		{
			CJS_Value valElement(isolate);
			aName.GetElement(i,valElement);
			CFX_WideString swVal = valElement.operator CFX_WideString();

			for (int j=0,jsz=pPDFForm->CountFields(swVal); j<jsz; j++)
			{
				aFields.Add((void*)pPDFForm->GetField(j,swVal));
			}
		}

		if (aFields.GetSize() > 0)
		{
			pPDFForm->ResetForm(aFields, TRUE, TRUE);
			m_pDocument->SetChangeMark();

		}
	}
	else
	{
		pPDFForm->ResetForm(TRUE);
		m_pDocument->SetChangeMark();

	}

	return TRUE;
}


FX_BOOL Document::saveAs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}


FX_BOOL Document::submitForm(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

//	if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) return FALSE;

	int nSize = params.size();
	if (nSize < 1) return FALSE;

	CFX_WideString strURL;
	FX_BOOL bFDF = TRUE;
	FX_BOOL bEmpty = FALSE;
	v8::Isolate* isolate = GetIsolate(cc);
	CJS_Array aFields(isolate);

	CJS_Value v = params[0];
	if (v.GetType() == VT_string)
	{
		strURL = params[0].operator CFX_WideString();
		if (nSize > 1)
			bFDF = params[1];
		if (nSize > 2)
			bEmpty = params[2];
		if (nSize > 3)
			aFields.Attach(params[3]);
	}
	else if (v.GetType() == VT_object)
	{
		JSObject pObj = (JSObject)params[0];
		v8::Handle<v8::Value> pValue = JS_GetObjectElement(isolate,pObj, L"cURL");
		if (!pValue.IsEmpty())
			strURL = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue));
		pValue = JS_GetObjectElement(isolate,pObj, L"bFDF");
			bFDF = CJS_Value(isolate,pValue, GET_VALUE_TYPE(pValue));
		pValue = JS_GetObjectElement(isolate,pObj, L"bEmpty");
			bEmpty = CJS_Value(isolate,pValue, GET_VALUE_TYPE(pValue));
		pValue = JS_GetObjectElement(isolate,pObj,L"aFields");
			aFields.Attach(CJS_Value(isolate,pValue, GET_VALUE_TYPE(pValue)));
	}

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);
	CPDF_InterForm* pPDFInterForm = pInterForm->GetInterForm();
	ASSERT(pPDFInterForm != NULL);

	FX_BOOL bAll = (aFields.GetLength() == 0);

	if (bAll && bEmpty)
	{
		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);
		CJS_Runtime* pRuntime = pContext->GetJSRuntime();
		ASSERT(pRuntime != NULL);


		if (pPDFInterForm->CheckRequiredFields())
		{
			pRuntime->BeginBlock();
			pInterForm->SubmitForm(strURL, FALSE);
			pRuntime->EndBlock();
		}

		return TRUE;
	}
	else
	{
		CFX_PtrArray fieldObjects;

		for (int i=0,sz=aFields.GetLength(); i<sz; i++)
		{
			CJS_Value valName(isolate);
			aFields.GetElement(i, valName);
			CFX_WideString sName = valName.operator CFX_WideString();

			CPDF_InterForm* pPDFForm = pInterForm->GetInterForm();
			ASSERT(pPDFForm != NULL);

			for (int j=0, jsz=pPDFForm->CountFields(sName); j<jsz; j++)
			{
				CPDF_FormField* pField = pPDFForm->GetField(j, sName);
				if (!bEmpty && pField->GetValue().IsEmpty())
					continue;

				fieldObjects.Add(pField);
			}
		}

		CJS_Context* pContext = (CJS_Context*)cc;
		ASSERT(pContext != NULL);
		CJS_Runtime* pRuntime = pContext->GetJSRuntime();
		ASSERT(pRuntime != NULL);


		if (pPDFInterForm->CheckRequiredFields(&fieldObjects, TRUE))
		{
			pRuntime->BeginBlock();
			pInterForm->SubmitFields(strURL, fieldObjects, TRUE, !bFDF);
			pRuntime->EndBlock();
		}

		return TRUE;
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////

void Document::AttachDoc(CPDFSDK_Document *pDoc)
{
	m_pDocument = pDoc;
}

CPDFSDK_Document * Document::GetReaderDoc()
{
	return m_pDocument;
}

FX_BOOL Document::ExtractFileName(CPDFSDK_Document *pDoc,CFX_ByteString &strFileName)
{
	return FALSE;
}

FX_BOOL Document::ExtractFolderName(CPDFSDK_Document *pDoc,CFX_ByteString &strFolderName)
{
	return FALSE;
}

FX_BOOL Document::bookmarkRoot(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::mailDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	FX_BOOL bUI = TRUE;
	CFX_WideString cTo = L"";
	CFX_WideString cCc = L"";
	CFX_WideString cBcc = L"";
	CFX_WideString cSubject = L"";
	CFX_WideString cMsg = L"";


	bUI = params.size()>=1?static_cast<FX_BOOL>(params[0]):TRUE;
	cTo = params.size()>=2?(const wchar_t*)params[1].operator CFX_WideString():L"";
	cCc = params.size()>=3?(const wchar_t*)params[2].operator CFX_WideString():L"";
	cBcc = params.size()>=4?(const wchar_t*)params[3].operator CFX_WideString():L"";
	cSubject = params.size()>=5?(const wchar_t*)params[4].operator CFX_WideString():L"";
	cMsg = params.size()>=6?(const wchar_t*)params[5].operator CFX_WideString():L"";

	v8::Isolate* isolate = GetIsolate(cc);

	if(params.size()>=1 && params[0].GetType() == VT_object)
	{
		JSObject  pObj = (JSObject )params[0];

		v8::Handle<v8::Value> pValue = JS_GetObjectElement(isolate,pObj, L"bUI");
			bUI = (int)CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue));

		pValue = JS_GetObjectElement(isolate,pObj, L"cTo");
			cTo = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).operator CFX_WideString();

		pValue = JS_GetObjectElement(isolate,pObj, L"cCc");
			cCc = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).operator CFX_WideString();

		pValue = JS_GetObjectElement(isolate,pObj, L"cBcc");
			cBcc = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).operator CFX_WideString();

		pValue = JS_GetObjectElement(isolate,pObj, L"cSubject");
			cSubject = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).operator CFX_WideString();

		pValue = JS_GetObjectElement(isolate,pObj, L"cMsg");
			cMsg = CJS_Value(isolate,pValue,GET_VALUE_TYPE(pValue)).operator CFX_WideString();

	}

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	pRuntime->BeginBlock();
	CPDFDoc_Environment* pEnv = pRuntime->GetReaderApp();
	pEnv->JS_docmailForm(NULL, 0, bUI, cTo.c_str(), cSubject.c_str(), cCc.c_str(), cBcc.c_str(), cMsg.c_str());
	pRuntime->EndBlock();

	return TRUE;
}

FX_BOOL Document::author(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Author");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csAuthor;
		vp >> csAuthor;
		pDictionary->SetAtString("Author", PDF_EncodeText(csAuthor));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::info(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	CFX_WideString cwAuthor			= pDictionary->GetUnicodeText("Author");
	CFX_WideString cwTitle			= pDictionary->GetUnicodeText("Title");
	CFX_WideString cwSubject		= pDictionary->GetUnicodeText("Subject");
	CFX_WideString cwKeywords		= pDictionary->GetUnicodeText("Keywords");
	CFX_WideString cwCreator		= pDictionary->GetUnicodeText("Creator");
	CFX_WideString cwProducer		= pDictionary->GetUnicodeText("Producer");
	CFX_WideString cwCreationDate	= pDictionary->GetUnicodeText("CreationDate");
	CFX_WideString cwModDate		= pDictionary->GetUnicodeText("ModDate");
	CFX_WideString cwTrapped		= pDictionary->GetUnicodeText("Trapped");

	v8::Isolate* isolate = GetIsolate(cc);
	if (!vp.IsSetting())
	{
		CJS_Context* pContext = (CJS_Context *)cc;
		CJS_Runtime* pRuntime = pContext->GetJSRuntime();

		JSFXObject  pObj = JS_NewFxDynamicObj(*pRuntime, pContext, -1);

		JS_PutObjectString(isolate,pObj, L"Author", cwAuthor);
		JS_PutObjectString(isolate,pObj, L"Title", cwTitle);
		JS_PutObjectString(isolate,pObj, L"Subject", cwSubject);
		JS_PutObjectString(isolate,pObj, L"Keywords", cwKeywords);
		JS_PutObjectString(isolate,pObj, L"Creator", cwCreator);
		JS_PutObjectString(isolate,pObj, L"Producer", cwProducer);
		JS_PutObjectString(isolate,pObj, L"CreationDate", cwCreationDate);
		JS_PutObjectString(isolate,pObj, L"ModDate", cwModDate);
		JS_PutObjectString(isolate,pObj, L"Trapped", cwTrapped);

// It's to be compatible to non-standard info dictionary.
		FX_POSITION pos = pDictionary->GetStartPos();
		while(pos)
		{
			CFX_ByteString bsKey;
			CPDF_Object* pValueObj = pDictionary->GetNextElement(pos, bsKey);
			CFX_WideString wsKey  = CFX_WideString::FromUTF8(bsKey, bsKey.GetLength());
			if((pValueObj->GetType()==PDFOBJ_STRING) || (pValueObj->GetType()==PDFOBJ_NAME) )
					JS_PutObjectString(isolate,pObj, wsKey, pValueObj->GetUnicodeText());
			if(pValueObj->GetType()==PDFOBJ_NUMBER)
				JS_PutObjectNumber(isolate,pObj, wsKey, (float)pValueObj->GetNumber());
			if(pValueObj->GetType()==PDFOBJ_BOOLEAN)
				JS_PutObjectBoolean(isolate,pObj, wsKey, (bool)pValueObj->GetInteger());
		}

		vp << pObj;
		return TRUE;
	}
	else
	{
		return TRUE;
	}
}

FX_BOOL Document::creationDate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("CreationDate");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csCreationDate;
		vp >> csCreationDate;
		pDictionary->SetAtString("CreationDate", PDF_EncodeText(csCreationDate));
		m_pDocument->SetChangeMark();

		return TRUE;
	}
}

FX_BOOL Document::creator(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Creator");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csCreator;
		vp >> csCreator;
		pDictionary->SetAtString("Creator", PDF_EncodeText(csCreator));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::delay(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsGetting())
	{
		vp << m_bDelay;
		return TRUE;
	}
	else
	{
		ASSERT(m_pDocument != NULL);

		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		bool b;
		vp >> b;

		m_bDelay = b;

		if (m_bDelay)
		{
			for (int i=0,sz=m_DelayData.GetSize(); i<sz; i++)
				delete m_DelayData.GetAt(i);

			m_DelayData.RemoveAll();
		}
		else
		{
			for (int i=0,sz=m_DelayData.GetSize(); i<sz; i++)
			{
				if (CJS_DelayData* pData = m_DelayData.GetAt(i))
				{
					Field::DoDelay(m_pDocument, pData);
					delete m_DelayData.GetAt(i);
				}
			}
			m_DelayData.RemoveAll();
		}

		return TRUE;
	}
}

FX_BOOL Document::keywords(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Keywords");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csKeywords;
		vp >> csKeywords;
		pDictionary->SetAtString("Keywords", PDF_EncodeText(csKeywords));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::modDate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("ModDate");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csmodDate;
		vp >> csmodDate;
		pDictionary->SetAtString("ModDate", PDF_EncodeText(csmodDate));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::producer(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Producer");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString csproducer;
		vp >> csproducer;
		pDictionary->SetAtString("Producer", PDF_EncodeText(csproducer));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::subject(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Subject");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString cssubject;
		vp >> cssubject;
		pDictionary->SetAtString("Subject", PDF_EncodeText(cssubject));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::title(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (m_pDocument == NULL || m_pDocument->GetDocument() == NULL)
		return FALSE;

	CPDF_Dictionary* pDictionary = m_pDocument->GetDocument()->GetInfo();
	if (!pDictionary)return FALSE;

	if (vp.IsGetting())
	{
		vp << pDictionary->GetUnicodeText("Title");
		return TRUE;
	}
	else
	{
		if (!m_pDocument->GetPermissions(FPDFPERM_MODIFY)) return FALSE;

		CFX_WideString cstitle;
		vp >> cstitle;
		pDictionary->SetAtString("Title", PDF_EncodeText(cstitle));
		m_pDocument->SetChangeMark();
		return TRUE;
	}
}

FX_BOOL Document::numPages(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsGetting())
	{
		ASSERT(m_pDocument != NULL);
		vp << m_pDocument->GetPageCount();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

FX_BOOL Document::external(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	//In Chrome case,should always return true.
	vp << TRUE;
	return TRUE;
}

FX_BOOL Document::filesize(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting())return FALSE;

	vp << 0;
	return TRUE;
}

FX_BOOL Document::mouseX(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::mouseY(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::baseURL(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsGetting())
	{
		vp << m_cwBaseURL;
		return TRUE;
	}
	else
	{
		vp >> m_cwBaseURL;
		return TRUE;
	}
}

FX_BOOL Document::calculate(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	if (vp.IsGetting())
	{
		if (pInterForm->IsCalculateEnabled())
			vp << true;
		else
			vp << false;
	}
	else
	{
		bool bCalculate;
		vp >> bCalculate;

		pInterForm->EnableCalculate(bCalculate);
	}

	return TRUE;
}

FX_BOOL Document::documentFileName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting())
		return FALSE;

	CFX_WideString wsFilePath = m_pDocument->GetPath();

	FX_INT32 i = wsFilePath.GetLength() - 1;
	for ( ; i >= 0; i-- )
	{
		if ( wsFilePath.GetAt( i ) == L'\\' || wsFilePath.GetAt( i ) == L'/' )
			break;
	}
	if ( i >= 0 && i < wsFilePath.GetLength() - 1 )
	{
		vp << ( wsFilePath.GetBuffer( wsFilePath.GetLength() ) + i + 1 );
	}else{
		vp << L"";
	}
	return TRUE;
}

CFX_WideString Document::ReversalStr(CFX_WideString cbFrom)
{
	size_t iLength = cbFrom.GetLength();
        pdfium::base::CheckedNumeric<size_t> iSize = sizeof(wchar_t);
	iSize *= (iLength + 1);
	wchar_t* pResult = (wchar_t*)malloc(iSize.ValueOrDie());
	wchar_t* pFrom = (wchar_t*)cbFrom.GetBuffer(iLength);

	for (size_t i = 0; i < iLength; i++)
	{
		pResult[i] = *(pFrom + iLength - i - 1);
	}
	pResult[iLength] = L'\0';

	cbFrom.ReleaseBuffer();
	CFX_WideString cbRet = CFX_WideString(pResult);
	free(pResult);
	pResult = NULL;
	return cbRet;
}

CFX_WideString Document::CutString(CFX_WideString cbFrom)
{
	size_t iLength = cbFrom.GetLength();
	pdfium::base::CheckedNumeric<size_t> iSize = sizeof(wchar_t);
	iSize *= (iLength + 1);
	wchar_t* pResult = (wchar_t*)malloc(iSize.ValueOrDie());
	wchar_t* pFrom = (wchar_t*)cbFrom.GetBuffer(iLength);

	for (int i = 0; i < iLength; i++)
	{
		if (pFrom[i] == L'\\' || pFrom[i] == L'/')
		{
			pResult[i] = L'\0';
			break;
		}
		pResult[i] = pFrom[i];
	}
	pResult[iLength] = L'\0';

	cbFrom.ReleaseBuffer();
	CFX_WideString cbRet = CFX_WideString(pResult);
	free(pResult);
	pResult = NULL;
	return cbRet;
}

FX_BOOL Document::path(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting()) return FALSE;

	vp << app::SysPathToPDFPath(m_pDocument->GetPath());

	return TRUE;
}

FX_BOOL Document::pageWindowRect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::layout(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::addLink(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::closeDoc(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);





	return TRUE;
}

FX_BOOL Document::getPageBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}


FX_BOOL Document::getAnnot(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::getAnnots(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	vRet.SetNull();
	return TRUE;
}

FX_BOOL Document::getAnnot3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	vRet.SetNull();
	return TRUE;
}

FX_BOOL Document::getAnnots3D(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	vRet = VT_undefined;
	return TRUE;
}

FX_BOOL Document::getOCGs(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::getLinks(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

bool Document::IsEnclosedInRect(CFX_FloatRect rect, CFX_FloatRect LinkRect)
{
	if (rect.left <= LinkRect.left
	  && rect.top <= LinkRect.top
	  && rect.right >= LinkRect.right
	  && rect.bottom >= LinkRect.bottom)
		return true;
	else
		return false;
}

void IconTree::InsertIconElement(IconElement* pNewIcon)
{
	if (!pNewIcon)return;

	if (m_pHead == NULL && m_pEnd == NULL)
	{
		m_pHead = m_pEnd = pNewIcon;
		m_iLength++;
	}
	else
	{
		m_pEnd->NextIcon = pNewIcon;
		m_pEnd = pNewIcon;
		m_iLength++;
	}
}

void IconTree::DeleteIconTree()
{
	if (!m_pHead || !m_pEnd)return;

	IconElement* pTemp = NULL;
	while(m_pEnd != m_pHead)
	{
		pTemp = m_pHead;
		m_pHead = m_pHead->NextIcon;
		delete pTemp;
	}

	delete m_pEnd;
	m_pHead = NULL;
	m_pEnd = NULL;
}

int IconTree::GetLength()
{
	return m_iLength;
}

IconElement* IconTree::operator [](int iIndex)
{
	if (iIndex >= 0 && iIndex <= m_iLength)
	{
		IconElement* pTemp = m_pHead;
		for (int i = 0; i < iIndex; i++)
		{
			pTemp = pTemp->NextIcon;
		}
		return pTemp;
	}
	else
		return NULL;
}

void IconTree::DeleteIconElement(CFX_WideString swIconName)
{
	IconElement* pTemp = m_pHead;
	int iLoopCount = m_iLength;
	for (int i = 0; i < iLoopCount - 1; i++)
	{
		if (pTemp == m_pEnd)
			break;

		if (m_pHead->IconName == swIconName)
		{
			m_pHead = m_pHead->NextIcon;
			delete pTemp;
			m_iLength--;
			pTemp = m_pHead;
		}
		if (pTemp->NextIcon->IconName == swIconName)
		{
			if (pTemp->NextIcon == m_pEnd)
			{
				m_pEnd = pTemp;
				delete pTemp->NextIcon;
				m_iLength--;
				pTemp->NextIcon = NULL;
			}
			else
			{
				IconElement* pElement = pTemp->NextIcon;
				pTemp->NextIcon = pTemp->NextIcon->NextIcon;
				delete pElement;
				m_iLength--;
				pElement = NULL;
			}

			continue;
		}

		pTemp = pTemp->NextIcon;
	}
}

FX_BOOL Document::addIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	if (params.size() != 2)return FALSE;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	CFX_WideString swIconName = params[0].operator CFX_WideString();

	JSFXObject pJSIcon = (JSFXObject)params[1];
	if (JS_GetObjDefnID(pJSIcon) != JS_GetObjDefnID(*pRuntime, L"Icon")) return FALSE;

	CJS_EmbedObj* pEmbedObj = ((CJS_Object*)params[1])->GetEmbedObject();
	if (!pEmbedObj)return FALSE;
	Icon* pIcon = (Icon*)pEmbedObj;

	if (!m_pIconTree)
		m_pIconTree = new IconTree();

	IconElement* pNewIcon = new IconElement();
	pNewIcon->IconName = swIconName;
	pNewIcon->NextIcon = NULL;
	pNewIcon->IconStream = pIcon;
	m_pIconTree->InsertIconElement(pNewIcon);
	return TRUE;
}

FX_BOOL Document::icons(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsSetting())
		return FALSE;

	if (!m_pIconTree)
	{
		vp.SetNull();
		return TRUE;
	}

	CJS_Array Icons(m_isolate);
	IconElement* pIconElement = NULL;
	int iIconTreeLength = m_pIconTree->GetLength();

	CJS_Context* pContext = (CJS_Context *)cc;
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();

	for (int i = 0; i < iIconTreeLength; i++)
	{
		pIconElement = (*m_pIconTree)[i];

		JSFXObject  pObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"Icon"));
		if (pObj.IsEmpty()) return FALSE;

		CJS_Icon * pJS_Icon = (CJS_Icon *)JS_GetPrivate(pObj);
		if (!pJS_Icon) return FALSE;

		Icon* pIcon = (Icon*)pJS_Icon->GetEmbedObject();
		if (!pIcon)return FALSE;

		pIcon->SetStream(pIconElement->IconStream->GetStream());
		pIcon->SetIconName(pIconElement->IconName);
		Icons.SetElement(i, CJS_Value(m_isolate,pJS_Icon));
	}

	vp << Icons;
	return TRUE;
}

FX_BOOL Document::getIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	if (params.size() != 1)return FALSE;
	if(!m_pIconTree)
		return FALSE;
	CFX_WideString swIconName = params[0].operator CFX_WideString();
	int iIconCounts = m_pIconTree->GetLength();

	CJS_Context* pContext = (CJS_Context *)cc;
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();

	for (int i = 0; i < iIconCounts; i++)
	{
		if ((*m_pIconTree)[i]->IconName == swIconName)
		{
			Icon* pRetIcon = (*m_pIconTree)[i]->IconStream;

			JSFXObject  pObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"Icon"));
			if (pObj.IsEmpty()) return FALSE;

			CJS_Icon * pJS_Icon = (CJS_Icon *)JS_GetPrivate(pObj);
			if (!pJS_Icon) return FALSE;

			Icon* pIcon = (Icon*)pJS_Icon->GetEmbedObject();
			if (!pIcon)return FALSE;

			pIcon->SetIconName(swIconName);
			pIcon->SetStream(pRetIcon->GetStream());
			vRet = pJS_Icon;
			return TRUE;
		}
	}

	return FALSE;
}

FX_BOOL Document::removeIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	if (params.size() != 1)return FALSE;
	if(!m_pIconTree)
		return FALSE;
	CFX_WideString swIconName = params[0].operator CFX_WideString();
	return TRUE;
}

FX_BOOL Document::createDataObject(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not implemented.
  return TRUE;
}

FX_BOOL Document::media(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::calculateNow(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
		m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM) ||
		m_pDocument->GetPermissions(FPDFPERM_FILL_FORM))) return FALSE;

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);
	pInterForm->OnCalculate();
	return TRUE;
}

FX_BOOL Document::Collab(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::getPageNthWord(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) return FALSE;

	int nPageNo = params.GetSize() > 0 ? (int)params[0] : 0;
	int nWordNo = params.GetSize() > 1 ? (int)params[1] : 0;
	bool bStrip = params.GetSize() > 2 ? (bool)params[2] : true;

	CPDF_Document* pDocument = m_pDocument->GetDocument();
	if (!pDocument) return FALSE;

	if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount())
	{
		//sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
	if (!pPageDict) return FALSE;

	CPDF_Page page;
	page.Load(pDocument, pPageDict);
	page.StartParse();
	page.ParseContent();

	FX_POSITION pos = page.GetFirstObjectPosition();

	int nWords = 0;

	CFX_WideString swRet;

	while (pos)
	{
		if (CPDF_PageObject* pPageObj = page.GetNextObject(pos))
		{
			if (pPageObj->m_Type == PDFPAGE_TEXT)
			{
				int nObjWords = CountWords((CPDF_TextObject*)pPageObj);

				if (nWords + nObjWords >= nWordNo)
				{
					swRet = GetObjWordStr((CPDF_TextObject*)pPageObj, nWordNo - nWords);
					break;
				}

				nWords += nObjWords;
			}
		}
	}

	if (bStrip)
	{
		swRet.TrimLeft();
		swRet.TrimRight();
	}

	vRet = swRet;
	return TRUE;
}

FX_BOOL Document::getPageNthWordQuads(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) return FALSE;

	return FALSE;
}

FX_BOOL Document::getPageNumWords(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_pDocument->GetPermissions(FPDFPERM_EXTRACT_ACCESS)) return FALSE;

	int nPageNo = params.GetSize() > 0 ? (int)params[0] : 0;

	CPDF_Document* pDocument = m_pDocument->GetDocument();
	ASSERT(pDocument != NULL);

	if (nPageNo < 0 || nPageNo >= pDocument->GetPageCount())
	{
		//sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
		return FALSE;
	}

	CPDF_Dictionary* pPageDict = pDocument->GetPage(nPageNo);
	if (!pPageDict) return FALSE;

	CPDF_Page page;
	page.Load(pDocument, pPageDict);
	page.StartParse();
	page.ParseContent();

	FX_POSITION pos = page.GetFirstObjectPosition();

	int nWords = 0;

	while (pos)
	{
		if (CPDF_PageObject* pPageObj = page.GetNextObject(pos))
		{
			if (pPageObj->m_Type == PDFPAGE_TEXT)
			{
				CPDF_TextObject* pTextObj = (CPDF_TextObject*)pPageObj;
				nWords += CountWords(pTextObj);
			}
		}
	}

	vRet = nWords;

	return TRUE;
}

FX_BOOL Document::getPrintParams(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);
	JSFXObject pRetObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"PrintParamsObj"));
	//not implemented yet.
	vRet = pRetObj;

	return TRUE;
}

#define ISLATINWORD(u)	(u != 0x20 && u <= 0x28FF)

int	Document::CountWords(CPDF_TextObject* pTextObj)
{
	if (!pTextObj) return 0;

	int nWords = 0;

	CPDF_Font* pFont = pTextObj->GetFont();
	if (!pFont) return 0;

	FX_BOOL bIsLatin = FALSE;

	for (int i=0, sz=pTextObj->CountChars(); i<sz; i++)
	{
		FX_DWORD charcode = -1;
		FX_FLOAT kerning;

		pTextObj->GetCharInfo(i, charcode, kerning);
		CFX_WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

		FX_WORD unicode = 0;
		if (swUnicode.GetLength() > 0)
			unicode = swUnicode[0];

		if (ISLATINWORD(unicode) && bIsLatin)
			continue;

		bIsLatin = ISLATINWORD(unicode);
		if (unicode != 0x20)
			nWords++;
	}

	return nWords;
}

CFX_WideString Document::GetObjWordStr(CPDF_TextObject* pTextObj, int nWordIndex)
{
	ASSERT(pTextObj != NULL);

	CFX_WideString swRet;

	CPDF_Font* pFont = pTextObj->GetFont();
	if (!pFont) return L"";

	int nWords = 0;
	FX_BOOL bIsLatin = FALSE;

	for (int i=0, sz=pTextObj->CountChars(); i<sz; i++)
	{
		FX_DWORD charcode = -1;
		FX_FLOAT kerning;

		pTextObj->GetCharInfo(i, charcode, kerning);
		CFX_WideString swUnicode = pFont->UnicodeFromCharCode(charcode);

		FX_WORD unicode = 0;
		if (swUnicode.GetLength() > 0)
			unicode = swUnicode[0];

		if (ISLATINWORD(unicode) && bIsLatin)
		{
		}
		else
		{
			bIsLatin = ISLATINWORD(unicode);
			if (unicode != 0x20)
				nWords++;
		}

		if (nWords-1 == nWordIndex)
			swRet += unicode;
	}

	return swRet;
}

FX_BOOL Document::zoom(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{

	return TRUE;
}

/**
(none,	NoVary)
(fitP,	FitPage)
(fitW,	FitWidth)
(fitH,	FitHeight)
(fitV,	FitVisibleWidth)
(pref,	Preferred)
(refW,	ReflowWidth)
*/

FX_BOOL Document::zoomType(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Document::deletePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	v8::Isolate* isolate = GetIsolate(cc);
	ASSERT(m_pDocument != NULL);

	if (!(m_pDocument->GetPermissions(FPDFPERM_MODIFY) ||
		m_pDocument->GetPermissions(FPDFPERM_ASSEMBLE))) return FALSE;

	int iSize = params.size();

	int nStart = 0;
	int nEnd = 0;

	if (iSize < 1)
	{
	}
	else if (iSize == 1)
	{
		if (params[0].GetType() == VT_object)
		{
			JSObject  pObj = (JSObject )params[0];
			v8::Handle<v8::Value> pValue = JS_GetObjectElement(isolate,pObj, L"nStart");
				nStart = (int)CJS_Value(m_isolate,pValue,GET_VALUE_TYPE(pValue));

			pValue = JS_GetObjectElement(isolate,pObj, L"nEnd");
				nEnd = (int)CJS_Value(m_isolate,pValue,GET_VALUE_TYPE(pValue));
		}
		else
		{
			nStart = (int)params[0];
		}
	}
	else
	{
		nStart = (int)params[0];
		nEnd = (int)params[1];
	}

	int nTotal = m_pDocument->GetPageCount();

	if (nStart < 0)	nStart = 0;
	if (nStart >= nTotal) nStart = nTotal - 1;

	if (nEnd < 0) nEnd = 0;
	if (nEnd >= nTotal) nEnd = nTotal - 1;

	if (nEnd < nStart) nEnd = nStart;



	return TRUE;
}

FX_BOOL Document::extractPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::insertPages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::replacePages(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL Document::getURL(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
  // Unsafe, not supported.
  return TRUE;
}

void Document::AddDelayData(CJS_DelayData* pData)
{
	m_DelayData.Add(pData);
}

void Document::DoFieldDelay(const CFX_WideString& sFieldName, int nControlIndex)
{
	CFX_DWordArray DelArray;

	for (int i=0,sz=m_DelayData.GetSize(); i<sz; i++)
	{
		if (CJS_DelayData* pData = m_DelayData.GetAt(i))
		{
			if (pData->sFieldName == sFieldName && pData->nControlIndex == nControlIndex)
			{
				Field::DoDelay(m_pDocument, pData);
				delete pData;
				m_DelayData.SetAt(i, NULL);
				DelArray.Add(i);
			}
		}
	}

	for (int j=DelArray.GetSize()-1; j>=0; j--)
	{
		m_DelayData.RemoveAt(DelArray[j]);
	}
}

void Document::AddDelayAnnotData(CJS_AnnotObj *pData)
{
	m_DelayAnnotData.Add(pData);
}

void Document::DoAnnotDelay()
{
	CFX_DWordArray DelArray;

	for (int j=DelArray.GetSize()-1; j>=0; j--)
	{
		m_DelayData.RemoveAt(DelArray[j]);
	}
}
