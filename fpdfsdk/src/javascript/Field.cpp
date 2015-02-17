// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_Define.h"
#include "../../include/javascript/JS_Object.h"
#include "../../include/javascript/JS_Value.h"
#include "../../include/javascript/Field.h"
#include "../../include/javascript/JS_EventHandler.h"
//#include "../include/JS_ResMgr.h"
#include "../../include/javascript/JS_Context.h"
#include "../../include/javascript/JS_Runtime.h"
#include "../../include/javascript/Document.h"
#include "../../include/javascript/color.h"
#include "../../include/javascript/PublicMethods.h"
#include "../../include/javascript/Icon.h"


/* ---------------------- Field ---------------------- */

BEGIN_JS_STATIC_CONST(CJS_Field)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Field)
	JS_STATIC_PROP_ENTRY(alignment)
	JS_STATIC_PROP_ENTRY(borderStyle)
	JS_STATIC_PROP_ENTRY(buttonAlignX)
	JS_STATIC_PROP_ENTRY(buttonAlignY)
	JS_STATIC_PROP_ENTRY(buttonFitBounds)
	JS_STATIC_PROP_ENTRY(buttonPosition)
	JS_STATIC_PROP_ENTRY(buttonScaleHow)
	JS_STATIC_PROP_ENTRY(buttonScaleWhen)
	JS_STATIC_PROP_ENTRY(calcOrderIndex)
	JS_STATIC_PROP_ENTRY(charLimit)
	JS_STATIC_PROP_ENTRY(comb)
	JS_STATIC_PROP_ENTRY(commitOnSelChange)
	JS_STATIC_PROP_ENTRY(currentValueIndices)
	JS_STATIC_PROP_ENTRY(defaultStyle)
	JS_STATIC_PROP_ENTRY(defaultValue)
	JS_STATIC_PROP_ENTRY(doNotScroll)
	JS_STATIC_PROP_ENTRY(doNotSpellCheck)
	JS_STATIC_PROP_ENTRY(delay)
	JS_STATIC_PROP_ENTRY(display)
	JS_STATIC_PROP_ENTRY(doc)
	JS_STATIC_PROP_ENTRY(editable)
	JS_STATIC_PROP_ENTRY(exportValues)
	JS_STATIC_PROP_ENTRY(hidden)
	JS_STATIC_PROP_ENTRY(fileSelect)
	JS_STATIC_PROP_ENTRY(fillColor)
	JS_STATIC_PROP_ENTRY(lineWidth)
	JS_STATIC_PROP_ENTRY(highlight)
	JS_STATIC_PROP_ENTRY(multiline)
	JS_STATIC_PROP_ENTRY(multipleSelection)
	JS_STATIC_PROP_ENTRY(name)
	JS_STATIC_PROP_ENTRY(numItems)
	JS_STATIC_PROP_ENTRY(page)
	JS_STATIC_PROP_ENTRY(password)
	JS_STATIC_PROP_ENTRY(print)
	JS_STATIC_PROP_ENTRY(radiosInUnison)
	JS_STATIC_PROP_ENTRY(readonly)
	JS_STATIC_PROP_ENTRY(rect)
	JS_STATIC_PROP_ENTRY(required)
	JS_STATIC_PROP_ENTRY(richText)
	JS_STATIC_PROP_ENTRY(richValue)
	JS_STATIC_PROP_ENTRY(rotation)
	JS_STATIC_PROP_ENTRY(strokeColor)
	JS_STATIC_PROP_ENTRY(style)
	JS_STATIC_PROP_ENTRY(submitName)
	JS_STATIC_PROP_ENTRY(textColor)
	JS_STATIC_PROP_ENTRY(textFont)
	JS_STATIC_PROP_ENTRY(textSize)
	JS_STATIC_PROP_ENTRY(type)
	JS_STATIC_PROP_ENTRY(userName)
	JS_STATIC_PROP_ENTRY(value)
	JS_STATIC_PROP_ENTRY(valueAsString)
	JS_STATIC_PROP_ENTRY(source)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Field)
	JS_STATIC_METHOD_ENTRY(browseForFileToSubmit,      0)
	JS_STATIC_METHOD_ENTRY(buttonGetCaption,           1)
	JS_STATIC_METHOD_ENTRY(buttonGetIcon,              1)
	JS_STATIC_METHOD_ENTRY(buttonImportIcon,           0)
	JS_STATIC_METHOD_ENTRY(buttonSetCaption,           2)
	JS_STATIC_METHOD_ENTRY(buttonSetIcon,              2)
	JS_STATIC_METHOD_ENTRY(checkThisBox,               2)
	JS_STATIC_METHOD_ENTRY(clearItems,                 0)
	JS_STATIC_METHOD_ENTRY(defaultIsChecked,           2)
	JS_STATIC_METHOD_ENTRY(deleteItemAt,               1)
	JS_STATIC_METHOD_ENTRY(getArray ,                  0)
	JS_STATIC_METHOD_ENTRY(getItemAt,                  0)
	JS_STATIC_METHOD_ENTRY(getLock,                    0)
	JS_STATIC_METHOD_ENTRY(insertItemAt,               0)
	JS_STATIC_METHOD_ENTRY(isBoxChecked,               1)
	JS_STATIC_METHOD_ENTRY(isDefaultChecked,           1)
	JS_STATIC_METHOD_ENTRY(setAction,                  2)
	JS_STATIC_METHOD_ENTRY(setFocus,                   0)
	JS_STATIC_METHOD_ENTRY(setItems,                   1)
	JS_STATIC_METHOD_ENTRY(setLock,                    0)
	JS_STATIC_METHOD_ENTRY(signatureGetModifications,  0)
	JS_STATIC_METHOD_ENTRY(signatureGetSeedValue,      0)
	JS_STATIC_METHOD_ENTRY(signatureInfo,              0)
	JS_STATIC_METHOD_ENTRY(signatureSetSeedValue,      0)
	JS_STATIC_METHOD_ENTRY(signatureSign,              0)
	JS_STATIC_METHOD_ENTRY(signatureValidate,          0)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Field, Field)

FX_BOOL	CJS_Field::InitInstance(IFXJS_Context* cc)
{
	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);

	Field* pField = (Field*)GetEmbedObject();
	ASSERT(pField != NULL);

	pField->SetIsolate(pContext->GetJSRuntime()->GetIsolate());

	return TRUE;
};

Field::Field(CJS_Object* pJSObject): CJS_EmbedObj(pJSObject),
	m_pJSDoc(NULL),
	m_pDocument(NULL),
	m_nFormControlIndex(-1),
	m_bCanSet(FALSE),
	m_bDelay(FALSE),
	m_isolate(NULL)
{
}

Field::~Field()
{
}

//note: iControlNo = -1, means not a widget.
void Field::ParseFieldName(const std::wstring &strFieldNameParsed,std::wstring &strFieldName,int & iControlNo)
{
	int iStart = strFieldNameParsed.find_last_of(L'.');
	if (iStart == -1)
	{
		strFieldName = strFieldNameParsed;
		iControlNo = -1;
		return;
	}
	std::wstring suffixal = strFieldNameParsed.substr(iStart+1);
	iControlNo = FXSYS_wtoi(suffixal.c_str());
	if (iControlNo == 0)
	{
		int iStart;
		while((iStart = suffixal.find_last_of(L" ")) != -1)
		{
			suffixal.erase(iStart,1);
		}

		if (suffixal.compare(L"0") != 0)
		{
			strFieldName = strFieldNameParsed;
			iControlNo = -1;
			return;
		}

	}
	strFieldName = strFieldNameParsed.substr(0,iStart);    
}

FX_BOOL Field::AttachField(Document* pDocument, const CFX_WideString& csFieldName)
{
	ASSERT(pDocument != NULL);
	m_pJSDoc = pDocument;

	m_pDocument = pDocument->GetReaderDoc();
	ASSERT(m_pDocument != NULL);

	m_bCanSet = m_pDocument->GetPermissions(FPDFPERM_FILL_FORM) || 
		m_pDocument->GetPermissions(FPDFPERM_ANNOT_FORM) || 
		m_pDocument->GetPermissions(FPDFPERM_MODIFY);

	CPDFSDK_InterForm* pRDInterForm = m_pDocument->GetInterForm();
	ASSERT(pRDInterForm != NULL);

	CPDF_InterForm* pInterForm = pRDInterForm->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_WideString swFieldNameTemp = csFieldName;
	swFieldNameTemp.Replace(L"..", L".");

	if (pInterForm->CountFields(swFieldNameTemp) <= 0)
	{
		std::wstring strFieldName;
		int iControlNo = -1;
		ParseFieldName(swFieldNameTemp.c_str(), strFieldName, iControlNo);
		if (iControlNo == -1) return FALSE;
		
		m_FieldName = strFieldName.c_str();
		m_nFormControlIndex = iControlNo;
		return TRUE;
	}

	m_FieldName = swFieldNameTemp;
	m_nFormControlIndex = -1;

	return TRUE;
}

void Field::GetFormFields(CPDFSDK_Document* pDocument, const CFX_WideString& csFieldName, CFX_PtrArray& FieldArray)
{
	ASSERT(pDocument != NULL);

	CPDFSDK_InterForm* pReaderInterForm = pDocument->GetInterForm();
	ASSERT(pReaderInterForm != NULL);

	CPDF_InterForm* pInterForm = pReaderInterForm->GetInterForm();
	ASSERT(pInterForm != NULL);

	ASSERT(FieldArray.GetSize() == 0);

	for (int i=0,sz=pInterForm->CountFields(csFieldName); i<sz; i++)
	{
		if (CPDF_FormField* pFormField = pInterForm->GetField(i, csFieldName))
			FieldArray.Add((void*)pFormField);
	}
}

void Field::GetFormFields(const CFX_WideString& csFieldName, CFX_PtrArray& FieldArray)
{
	ASSERT(m_pDocument != NULL);

	Field::GetFormFields(m_pDocument, csFieldName, FieldArray);
}

void Field::UpdateFormField(CPDFSDK_Document* pDocument, CPDF_FormField* pFormField, 
							FX_BOOL bChangeMark, FX_BOOL bResetAP, FX_BOOL bRefresh)
{
	ASSERT(pDocument != NULL);
	ASSERT(pFormField != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray widgets;
	pInterForm->GetWidgets(pFormField, widgets);

	if (bResetAP)
	{
		int nFieldType = pFormField->GetFieldType();
		if (nFieldType == FIELDTYPE_COMBOBOX || nFieldType == FIELDTYPE_TEXTFIELD)
		{
			for (int i=0,sz=widgets.GetSize(); i<sz; i++)
			{
				CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)widgets.GetAt(i);
				ASSERT(pWidget != NULL);

				FX_BOOL bFormated = FALSE;
				CFX_WideString sValue = pWidget->OnFormat(0, bFormated);
				if (bFormated)
					pWidget->ResetAppearance(sValue, FALSE);
				else
					pWidget->ResetAppearance(NULL, FALSE);
			}
		}
		else
		{
			for (int i=0,sz=widgets.GetSize(); i<sz; i++)
			{
				CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)widgets.GetAt(i);
				ASSERT(pWidget != NULL);

				pWidget->ResetAppearance(NULL, FALSE);
			}
		}
	}

	if (bRefresh)
	{
		for (int i=0,sz=widgets.GetSize(); i<sz; i++)
		{
			CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)widgets.GetAt(i);
			ASSERT(pWidget != NULL);
			
			CPDFSDK_InterForm * pInterForm = pWidget->GetInterForm();
			CPDFSDK_Document* pDoc = pInterForm->GetDocument();
// 			CReader_Page* pPage = pWidget->GetPage();
 			ASSERT(pDoc != NULL);
			pDoc->UpdateAllViews(NULL, pWidget);
		}
	}		
	
	if (bChangeMark)
		pDocument->SetChangeMark();
}

void Field::UpdateFormControl(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl, 
							FX_BOOL bChangeMark, FX_BOOL bResetAP, FX_BOOL bRefresh)
{
	ASSERT(pDocument != NULL);
	ASSERT(pFormControl != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl);
	
	if (pWidget)
	{
		if (bResetAP)
		{
			int nFieldType = pWidget->GetFieldType();
			if (nFieldType == FIELDTYPE_COMBOBOX || nFieldType == FIELDTYPE_TEXTFIELD)
			{
				FX_BOOL bFormated = FALSE;
				CFX_WideString sValue = pWidget->OnFormat(0, bFormated);
				if (bFormated)
					pWidget->ResetAppearance(sValue, FALSE);
				else
					pWidget->ResetAppearance(NULL, FALSE);
			}
			else
			{
				pWidget->ResetAppearance(NULL, FALSE);
			}
		}

		if (bRefresh)
		{
			CPDFSDK_InterForm * pInterForm = pWidget->GetInterForm();
			CPDFSDK_Document* pDoc = pInterForm->GetDocument();
			ASSERT(pDoc != NULL);
			pDoc->UpdateAllViews(NULL, pWidget);
		}

	}

	if (bChangeMark)
		pDocument->SetChangeMark();
}

CPDFSDK_Widget* Field::GetWidget(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl)
{
	ASSERT(pDocument != NULL);
	ASSERT(pFormControl != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	return pInterForm->GetWidget(pFormControl);
}

FX_BOOL Field::ValueIsOccur(CPDF_FormField* pFormField, CFX_WideString csOptLabel)
{
	ASSERT(pFormField != NULL);

	for (int i=0,sz = pFormField->CountOptions(); i < sz; i++)
	{
		if (csOptLabel.Compare(pFormField->GetOptionLabel(i)) == 0)
			return TRUE;
	}

	return FALSE;
}

CPDF_FormControl* Field::GetSmartFieldControl(CPDF_FormField* pFormField)
{
	ASSERT(pFormField != NULL);
	if(!pFormField->CountControls() || m_nFormControlIndex>=pFormField->CountControls()) return NULL;

	if (m_nFormControlIndex<0)
		return pFormField->GetControl(0);
	else
		return pFormField->GetControl(m_nFormControlIndex);
}

/* ---------------------------------------- property ---------------------------------------- */

FX_BOOL Field::alignment(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_ByteString alignStr;
		vp >> alignStr;

		if (m_bDelay)
		{
			AddDelay_String(FP_ALIGNMENT, alignStr);
		}
		else
		{
			Field::SetAlignment(m_pDocument, m_FieldName, m_nFormControlIndex, alignStr);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		switch (pFormControl->GetControlAlignment())
		{
			case 1:
				vp << L"center";
				break;
			case 0:
				vp << L"left";
				break;
			case 2:
				vp << L"right";
				break;
			default:
				vp << L"";
		}
	}

	return TRUE;
}

void Field::SetAlignment(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, 
						 const CFX_ByteString& string)
{
	//Not supported.
}

FX_BOOL Field::borderStyle(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{	
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_ByteString strType = "";
		vp >> strType;

		if (m_bDelay)
		{
			AddDelay_String(FP_BORDERSTYLE, strType);
		}
		else
		{
			Field::SetBorderStyle(m_pDocument, m_FieldName, m_nFormControlIndex, strType);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		if (!pFormField) return FALSE;

		CPDFSDK_Widget* pWidget = GetWidget(m_pDocument, GetSmartFieldControl(pFormField));
		if (!pWidget) return FALSE;

		int nBorderstyle = pWidget->GetBorderStyle();

		switch (nBorderstyle)
		{
			case BBS_SOLID:
				vp << L"solid";
				break;
			case BBS_DASH:
				vp << L"dashed";
				break;
			case BBS_BEVELED:
				vp << L"beveled";
				break;
			case BBS_INSET:
				vp << L"inset";
				break;
			case BBS_UNDERLINE:
				vp << L"underline";
				break;
			default:
				vp << L"";
				break;
		}
	}

	return TRUE;
}

void Field::SetBorderStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, 
						   const CFX_ByteString& string)
{
	ASSERT(pDocument != NULL);

	int nBorderStyle = 0;

	if (string == "solid")
		nBorderStyle = BBS_SOLID;
	else if (string == "beveled")
		nBorderStyle = BBS_BEVELED;
	else if (string == "dashed")
		nBorderStyle = BBS_DASH;
	else if (string == "inset")
		nBorderStyle = BBS_INSET;
	else if (string == "underline")
		nBorderStyle = BBS_UNDERLINE;
	else return;

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (nControlIndex < 0)
		{
			FX_BOOL bSet = FALSE;
			for (int j=0,jsz = pFormField->CountControls(); j<jsz; j++)
			{
				if (CPDFSDK_Widget* pWidget = GetWidget(pDocument, pFormField->GetControl(j)))
				{
					if (pWidget->GetBorderStyle() != nBorderStyle)
					{
						pWidget->SetBorderStyle(nBorderStyle);
						bSet = TRUE;
					}
				}
			}
			if (bSet) UpdateFormField(pDocument, pFormField, TRUE, TRUE, TRUE);
		}
		else
		{
			if(nControlIndex >= pFormField->CountControls()) return;
			if (CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex))
			{
				if (CPDFSDK_Widget* pWidget = GetWidget(pDocument, pFormControl))
				{
					if (pWidget->GetBorderStyle() != nBorderStyle)
					{
						pWidget->SetBorderStyle(nBorderStyle);
						UpdateFormControl(pDocument, pFormControl, TRUE, TRUE, TRUE);
					}
				}
			}
		}
	}
}

FX_BOOL Field::buttonAlignX(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_BUTTONALIGNX, nVP);
		}
		else
		{
			Field::SetButtonAlignX(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{		
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		CPDF_IconFit IconFit = pFormControl->GetIconFit();

		FX_FLOAT fLeft,fBottom;
		IconFit.GetIconPosition(fLeft,fBottom);

		vp << (FX_INT32)fLeft;
	}

	return TRUE;
}

void Field::SetButtonAlignX(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::buttonAlignY(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_BUTTONALIGNY, nVP);
		}
		else
		{
			Field::SetButtonAlignY(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		CPDF_IconFit IconFit = pFormControl->GetIconFit();

		FX_FLOAT fLeft,fBottom;
		IconFit.GetIconPosition(fLeft,fBottom);

		vp <<  (FX_INT32)fBottom;
	}

	return TRUE;
}

void Field::SetButtonAlignY(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::buttonFitBounds(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_BUTTONFITBOUNDS, bVP);
		}
		else
		{
			Field::SetButtonFitBounds(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		CPDF_IconFit IconFit = pFormControl->GetIconFit();
		vp << IconFit.GetFittingBounds();		
	}

	return TRUE;
}

void Field::SetButtonFitBounds(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::buttonPosition(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_BUTTONPOSITION, nVP);
		}
		else
		{
			Field::SetButtonPosition(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		vp <<  pFormControl->GetTextPosition();
	}
	return TRUE;
}

void Field::SetButtonPosition(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::buttonScaleHow(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_BUTTONSCALEHOW, nVP);
		}
		else
		{
			Field::SetButtonScaleHow(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		CPDF_IconFit IconFit = pFormControl->GetIconFit();
		if (IconFit.IsProportionalScale())
			vp << (FX_INT32)0;
		else
			vp << (FX_INT32)1;
	}

	return TRUE;
}

void Field::SetButtonScaleHow(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::buttonScaleWhen(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_BUTTONSCALEWHEN, nVP);
		}
		else
		{
			Field::SetButtonScaleWhen(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*) FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl) return FALSE;

		CPDF_IconFit IconFit = pFormControl->GetIconFit();
		int ScaleM = IconFit.GetScaleMethod();
		switch (ScaleM)
		{
			case CPDF_IconFit::Always :
				vp <<  (FX_INT32) CPDF_IconFit::Always;
				break;
			case CPDF_IconFit::Bigger :
				vp <<  (FX_INT32) CPDF_IconFit::Bigger;
				break;
			case CPDF_IconFit::Never :
				vp <<  (FX_INT32) CPDF_IconFit::Never;
				break;
			case CPDF_IconFit::Smaller :
				vp <<  (FX_INT32) CPDF_IconFit::Smaller;
				break;
		}
	}

	return TRUE;
}

void Field::SetButtonScaleWhen(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::calcOrderIndex(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{	
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_CALCORDERINDEX, nVP);
		}
		else
		{
			Field::SetCalcOrderIndex(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX && pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		CPDFSDK_InterForm* pRDInterForm = m_pDocument->GetInterForm();
		ASSERT(pRDInterForm != NULL);

		CPDF_InterForm* pInterForm = pRDInterForm->GetInterForm();
		ASSERT(pInterForm != NULL);

		vp << (FX_INT32)pInterForm->FindFieldInCalculationOrder(pFormField);
	}

	return TRUE;
}

void Field::SetCalcOrderIndex(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::charLimit(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_CHARLIMIT, nVP);
		}
		else
		{
			Field::SetCharLimit(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		vp << (FX_INT32)pFormField->GetMaxLen();
	}
	return TRUE;
}

void Field::SetCharLimit(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::comb(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_COMB, bVP);
		}
		else
		{
			Field::SetComb(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{	
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_COMB)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetComb(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::commitOnSelChange(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_COMMITONSELCHANGE, bVP);
		}
		else
		{
			Field::SetCommitOnSelChange(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX && pFormField->GetFieldType() != FIELDTYPE_LISTBOX)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_COMMITONSELCHANGE)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetCommitOnSelChange(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::currentValueIndices(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_DWordArray array;

		if (vp.GetType() == VT_number)
		{
			int iSelecting = 0;
			vp >> iSelecting;
			array.Add(iSelecting);
		}
		else if (vp.IsArrayObject())
		{
			CJS_Array SelArray(m_isolate);
			CJS_Value SelValue(m_isolate);
			int iSelecting;
			vp >> SelArray;
			for (int i=0,sz=SelArray.GetLength(); i<sz; i++)
			{
				SelArray.GetElement(i,SelValue);
				iSelecting = (FX_INT32)SelValue;
				array.Add(iSelecting);
			}
		}
		
		if (m_bDelay)
		{
			AddDelay_WordArray(FP_CURRENTVALUEINDICES, array);
		}
		else
		{
			Field::SetCurrentValueIndices(m_pDocument, m_FieldName, m_nFormControlIndex, array);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX && pFormField->GetFieldType() != FIELDTYPE_LISTBOX)
			return FALSE;

		if (pFormField->CountSelectedItems() == 1)
			vp << pFormField->GetSelectedIndex(0);
		else if (pFormField->CountSelectedItems() > 1)
		{
			CJS_Array SelArray(m_isolate);
			for (int i=0,sz=pFormField->CountSelectedItems(); i<sz; i++)
			{
				SelArray.SetElement(i, CJS_Value(m_isolate,pFormField->GetSelectedIndex(i)));
			}
			vp << SelArray;
		}
		else
			vp << -1;
	}

	return TRUE;
}

void Field::SetCurrentValueIndices(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, 
								   const CFX_DWordArray& array)
{
	ASSERT(pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		int nFieldType = pFormField->GetFieldType();
		if (nFieldType == FIELDTYPE_COMBOBOX || nFieldType == FIELDTYPE_LISTBOX)
		{
			FX_DWORD dwFieldFlags = pFormField->GetFieldFlags();
			pFormField->ClearSelection(TRUE);

			for (int i=0,sz=array.GetSize(); i<sz; i++)
			{
				if (i>0 && !(dwFieldFlags & (1<<21)))
				{
					break;
				}

				int iSelecting = (FX_INT32)array.GetAt(i);
				if (iSelecting < pFormField->CountOptions() && !pFormField->IsItemSelected(iSelecting))
					pFormField->SetItemSelection(iSelecting, TRUE);

			}
			UpdateFormField(pDocument, pFormField, TRUE, TRUE, TRUE);
		}
	}
}

FX_BOOL Field::defaultStyle(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	// MQG sError = JSGetStringFromID(IDS_STRING_NOTSUPPORT);
	return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		;		
	}
	else
	{
		;
	}
	return TRUE;
}

void Field::SetDefaultStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex)
{
	//Not supported.
}

FX_BOOL Field::defaultValue(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_WideString WideStr;
		vp >> WideStr; 

		if (m_bDelay)
		{
			AddDelay_WideString(FP_DEFAULTVALUE, WideStr);
		}
		else
		{
			Field::SetDefaultValue(m_pDocument, m_FieldName, m_nFormControlIndex, WideStr);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON || 
			pFormField->GetFieldType() == FIELDTYPE_SIGNATURE)
			return FALSE;

		vp << pFormField->GetDefaultValue();
	}
	return TRUE;
}

void Field::SetDefaultValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex,
							const CFX_WideString& string)
{
	//Not supported.
}

FX_BOOL Field::doNotScroll(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_DONOTSCROLL, bVP);
		}
		else
		{
			Field::SetDoNotScroll(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_DONOTSCROLL)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetDoNotScroll(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::doNotSpellCheck(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD && 
			pFormField->GetFieldType() != FIELDTYPE_COMBOBOX)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_DONOTSPELLCHECK)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetDelay(FX_BOOL bDelay)
{
	m_bDelay = bDelay;

	if (!m_bDelay)
	{
		if (m_pJSDoc)
			m_pJSDoc->DoFieldDelay(m_FieldName, m_nFormControlIndex);
	}
}

FX_BOOL Field::delay(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;
		
		bool bVP;
		vp >> bVP;

		SetDelay(bVP);
	}
	else
	{
		vp << m_bDelay;
	}
	return TRUE;
}

FX_BOOL Field::display(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;	

		if (m_bDelay)
		{
			AddDelay_Int(FP_DISPLAY, nVP);
		}
		else
		{
			Field::SetDisplay(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		CPDFSDK_Widget* pWidget = pInterForm->GetWidget(GetSmartFieldControl(pFormField));
		if (!pWidget)return FALSE;

		FX_DWORD dwFlag = pWidget->GetFlags();

		if (ANNOTFLAG_INVISIBLE & dwFlag || ANNOTFLAG_HIDDEN & dwFlag) 
		{
			vp << (FX_INT32)1;
		}
		else 
		{
			if (ANNOTFLAG_PRINT & dwFlag)
			{
				if (ANNOTFLAG_NOVIEW & dwFlag)
				{
					vp << (FX_INT32)3;
				}
				else
				{
					vp << (FX_INT32)0;
				}
			}
			else
			{
				vp << (FX_INT32)2;
			}				
		}
	}

	return TRUE;
}

void Field::SetDisplay(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	ASSERT(pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (nControlIndex < 0)
		{
			FX_BOOL bSet = FALSE;
			for (int j=0,jsz = pFormField->CountControls(); j<jsz; j++)
			{
				CPDF_FormControl* pFormControl = pFormField->GetControl(j);
				ASSERT(pFormControl != NULL);

				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					FX_DWORD dwFlag = pWidget->GetFlags();
					switch (number)
					{
					case 0:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						dwFlag |= ANNOTFLAG_PRINT;							
						break;
					case 1:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						dwFlag |= (ANNOTFLAG_HIDDEN | ANNOTFLAG_PRINT);
						break;
					case 2:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_PRINT);
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						break;
					case 3:
						dwFlag |= ANNOTFLAG_NOVIEW;
						dwFlag |= ANNOTFLAG_PRINT;
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						break;
					}	

					if (dwFlag != pWidget->GetFlags())
					{
						pWidget->SetFlags(dwFlag);
						bSet = TRUE;
					}
				}
			}		
			
			if (bSet) UpdateFormField(pDocument, pFormField, TRUE, FALSE, TRUE);
		}
		else
		{
			if(nControlIndex >= pFormField->CountControls()) return;
			if (CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex))
			{
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{

					FX_DWORD dwFlag = pWidget->GetFlags();
					switch (number)
					{
					case 0:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						dwFlag |= ANNOTFLAG_PRINT;							
						break;
					case 1:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						dwFlag |= (ANNOTFLAG_HIDDEN | ANNOTFLAG_PRINT);
						break;
					case 2:
						dwFlag &= (~ANNOTFLAG_INVISIBLE);
						dwFlag &= (~ANNOTFLAG_PRINT);
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						dwFlag &= (~ANNOTFLAG_NOVIEW);
						break;
					case 3:
						dwFlag |= ANNOTFLAG_NOVIEW;
						dwFlag |= ANNOTFLAG_PRINT;
						dwFlag &= (~ANNOTFLAG_HIDDEN);
						break;
					}	
					if (dwFlag != pWidget->GetFlags())
					{
						pWidget->SetFlags(dwFlag);
						UpdateFormControl(pDocument, pFormControl, TRUE, FALSE, TRUE);
					}
				}
			}
		}
	}
}

FX_BOOL Field::doc(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pJSDoc != NULL);

	if (!vp.IsGetting())return FALSE;

	vp << (CJS_Object*)(*m_pJSDoc);

	return TRUE;
}

FX_BOOL Field::editable(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_EDIT)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::exportValues(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if (pFormField->GetFieldType() != FIELDTYPE_CHECKBOX && 
		pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON)
		return FALSE;

	if (vp.IsSetting())
	{	
		if (!m_bCanSet) return FALSE;
		if (!vp.IsArrayObject())return FALSE;
	}
	else
	{
		CJS_Array ExportValusArray(m_isolate);

		if (m_nFormControlIndex < 0)
		{
			for (int i=0,sz=pFormField->CountControls(); i<sz; i++)
			{
				CPDF_FormControl* pFormControl = pFormField->GetControl(i);
				ASSERT(pFormControl != NULL);

				ExportValusArray.SetElement(i, CJS_Value(m_isolate,pFormControl->GetExportValue().c_str()));
			}
		}
		else
		{
			if(m_nFormControlIndex >= pFormField->CountControls()) return FALSE;
			CPDF_FormControl* pFormControl = pFormField->GetControl(m_nFormControlIndex);
			if (!pFormControl) return FALSE;

			ExportValusArray.SetElement(0, CJS_Value(m_isolate,pFormControl->GetExportValue().c_str()));
		}

		vp << ExportValusArray;
	}

	return TRUE;
}

FX_BOOL Field::fileSelect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
		return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

	}
	else
	{
		if (pFormField->GetFieldFlags() & FIELDFLAG_FILESELECT)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::fillColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CJS_Array crArray(m_isolate);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;
		if (!vp.IsArrayObject()) return FALSE;

		vp >> crArray;

		CPWL_Color color;
		color::ConvertArrayToPWLColor(crArray, color);

		if (m_bDelay)
		{
			AddDelay_Color(FP_FILLCOLOR, color);
		}
		else
		{
			Field::SetFillColor(m_pDocument, m_FieldName, m_nFormControlIndex, color);
		}
	}
	else
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		int iColorType;
		pFormControl->GetBackgroundColor(iColorType);

		CPWL_Color color;

		if (iColorType == COLORTYPE_TRANSPARENT)
		{
			color = CPWL_Color(COLORTYPE_TRANSPARENT);
		}
		else if (iColorType == COLORTYPE_GRAY)
		{
			color = CPWL_Color(COLORTYPE_GRAY, pFormControl->GetOriginalBackgroundColor(0));
		}
		else if (iColorType == COLORTYPE_RGB)
		{
			color = CPWL_Color(COLORTYPE_RGB, pFormControl->GetOriginalBackgroundColor(0),
				pFormControl->GetOriginalBackgroundColor(1),
				pFormControl->GetOriginalBackgroundColor(2));
		}
		else if (iColorType == COLORTYPE_CMYK)
		{
			color = CPWL_Color(COLORTYPE_CMYK, pFormControl->GetOriginalBackgroundColor(0),
				pFormControl->GetOriginalBackgroundColor(1),
				pFormControl->GetOriginalBackgroundColor(2),
				pFormControl->GetOriginalBackgroundColor(3));
		}
		else
			return FALSE;

		color::ConvertPWLColorToArray(color, crArray);
        vp  <<  crArray;
	}

	return TRUE;
}

void Field::SetFillColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color)
{
	//Not supported.
}

FX_BOOL Field::hidden(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_HIDDEN, bVP);
		}
		else
		{
			Field::SetHidden(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		CPDFSDK_Widget* pWidget = pInterForm->GetWidget(GetSmartFieldControl(pFormField));
		if (!pWidget) return FALSE;

		FX_DWORD dwFlags = pWidget->GetFlags();

		if (ANNOTFLAG_INVISIBLE & dwFlags || ANNOTFLAG_HIDDEN & dwFlags) 
		{
			vp << true;
		}
		else 
			vp << false;
	}

	return TRUE;
}

void Field::SetHidden(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	ASSERT(pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (nControlIndex < 0)
		{
			FX_BOOL bSet = FALSE;
			for (int j=0,jsz = pFormField->CountControls(); j<jsz; j++)
			{
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormField->GetControl(j)))
				{					
					FX_DWORD dwFlags = pWidget->GetFlags();
					
					if (b)
					{
						dwFlags &= (~ANNOTFLAG_INVISIBLE);
						dwFlags &= (~ANNOTFLAG_NOVIEW);
						dwFlags |= (ANNOTFLAG_HIDDEN | ANNOTFLAG_PRINT);
					}
					else
					{
						dwFlags &= (~ANNOTFLAG_INVISIBLE);
						dwFlags &= (~ANNOTFLAG_HIDDEN);
						dwFlags &= (~ANNOTFLAG_NOVIEW);
						dwFlags |= ANNOTFLAG_PRINT;	
					}

					if (dwFlags != pWidget->GetFlags())
					{
						pWidget->SetFlags(dwFlags);	
						bSet = TRUE;
					}
				}
			}

			if (bSet)
				UpdateFormField(pDocument, pFormField, TRUE, FALSE, TRUE);	
		}
		else
		{
			if(nControlIndex >= pFormField->CountControls()) return;
			if (CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex))
			{
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					FX_DWORD dwFlags = pWidget->GetFlags();
					
					if (b)
					{
						dwFlags &= (~ANNOTFLAG_INVISIBLE);
						dwFlags &= (~ANNOTFLAG_NOVIEW);
						dwFlags |= (ANNOTFLAG_HIDDEN | ANNOTFLAG_PRINT);
					}
					else
					{
						dwFlags &= (~ANNOTFLAG_INVISIBLE);
						dwFlags &= (~ANNOTFLAG_HIDDEN);
						dwFlags &= (~ANNOTFLAG_NOVIEW);
						dwFlags |= ANNOTFLAG_PRINT;	
					}

					if (dwFlags != pWidget->GetFlags())
					{
						pWidget->SetFlags(dwFlags);	
						UpdateFormControl(pDocument, pFormControl, TRUE, FALSE, TRUE);	
					}
				}
			}
		}
	}
}

FX_BOOL Field::highlight(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_ByteString strMode;
		vp >> strMode;

		if (m_bDelay)
		{
			AddDelay_String(FP_HIGHLIGHT, strMode);
		}
		else
		{
			Field::SetHighlight(m_pDocument, m_FieldName, m_nFormControlIndex, strMode);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl) return FALSE;

		int eHM = pFormControl->GetHighlightingMode();
		switch (eHM)
		{
		case CPDF_FormControl::None:
			vp  <<  L"none";
			break;
		case CPDF_FormControl::Push:
			vp  <<  L"push";
			break;
		case CPDF_FormControl::Invert:
			vp  <<  L"invert";
			break;
		case CPDF_FormControl::Outline:
			vp  <<  L"outline";
			break;
		case CPDF_FormControl::Toggle:
			 vp  <<  L"toggle";
			 break;
		}
	}

	return TRUE;
}

void Field::SetHighlight(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string)
{
	//Not supported.
}

FX_BOOL Field::lineWidth(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int iWidth;
		vp >> iWidth;

		if (m_bDelay)
		{
			AddDelay_Int(FP_LINEWIDTH, iWidth);
		}
		else
		{
			Field::SetLineWidth(m_pDocument, m_FieldName, m_nFormControlIndex, iWidth);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl) return FALSE;

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		if(!pFormField->CountControls()) return FALSE;

		CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormField->GetControl(0));
		if (!pWidget) return FALSE;

		vp << (FX_INT32)pWidget->GetBorderWidth();
	}

	return TRUE;
}

void Field::SetLineWidth(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	ASSERT(pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (nControlIndex < 0)
		{
			FX_BOOL bSet = FALSE;
			for (int j=0,jsz=pFormField->CountControls(); j<jsz; j++)
			{
				CPDF_FormControl* pFormControl = pFormField->GetControl(j);
				ASSERT(pFormControl != NULL);
				
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					if (number != pWidget->GetBorderWidth())
					{
						pWidget->SetBorderWidth(number);
						bSet = TRUE;
					}
				}
			}
			if (bSet) UpdateFormField(pDocument, pFormField, TRUE, TRUE, TRUE);
		}
		else
		{
			if(nControlIndex >= pFormField->CountControls()) return;
			if (CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex))
			{
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					if (number != pWidget->GetBorderWidth())
					{
						pWidget->SetBorderWidth(number);
						UpdateFormControl(pDocument, pFormControl, TRUE, TRUE, TRUE);
					}
				}
			}
		}
	}
}

FX_BOOL Field::multiline(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_MULTILINE, bVP);
		}
		else
		{
			Field::SetMultiline(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName, FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_MULTILINE)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetMultiline(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::multipleSelection(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_MULTIPLESELECTION, bVP);
		}
		else
		{
			Field::SetMultipleSelection(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_LISTBOX)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_MULTISELECT)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetMultipleSelection(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::name(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting()) return FALSE;

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

   	vp << m_FieldName;

	return TRUE;
}

FX_BOOL Field::numItems(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{	
	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if (pFormField->GetFieldType() != FIELDTYPE_COMBOBOX &&
		pFormField->GetFieldType() != FIELDTYPE_LISTBOX)
		return FALSE;

	if (!vp.IsGetting()) return FALSE;

	vp << (FX_INT32)pFormField->CountOptions();

	return TRUE;
}

FX_BOOL Field::page(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (!vp.IsGetting()) return FALSE;

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	if (!pFormField) return FALSE;

	ASSERT(m_pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray widgetArray;
	pInterForm->GetWidgets(pFormField, widgetArray);

	if (widgetArray.GetSize() > 0)
	{
		CJS_Array PageArray(m_isolate);

		for (int i=0,sz=widgetArray.GetSize(); i<sz; i++)
		{
			CPDFSDK_Widget* pWidget = (CPDFSDK_Widget*)widgetArray.GetAt(i);
			ASSERT(pWidget != NULL);

			CPDFSDK_PageView* pPageView = pWidget->GetPageView();
			if(!pPageView)
				return FALSE;

			PageArray.SetElement(i, CJS_Value(m_isolate,(FX_INT32)pPageView->GetPageIndex()));
		}

		vp << PageArray;
	}
	else
	{
		vp << (FX_INT32) -1;
	}

	return TRUE;
}

FX_BOOL Field::password(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_PASSWORD, bVP);
		}
		else
		{
			Field::SetPassword(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_PASSWORD)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetPassword(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::print(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
		{
			CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
			ASSERT(pFormField != NULL);

			if (m_nFormControlIndex < 0)
			{
				FX_BOOL bSet = FALSE;
				for (int j=0,jsz = pFormField->CountControls(); j<jsz; j++)
				{
					if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormField->GetControl(j)))
					{
						FX_DWORD dwFlags = pWidget->GetFlags();
						if (bVP)
							dwFlags |= ANNOTFLAG_PRINT;
						else
							dwFlags &= ~ANNOTFLAG_PRINT;

						if (dwFlags != pWidget->GetFlags())
						{
							pWidget->SetFlags(dwFlags);
							bSet = TRUE;
						}
					}
				}

				if (bSet)
					UpdateFormField(m_pDocument, pFormField, TRUE, FALSE, TRUE);
			}
			else
			{
				if(m_nFormControlIndex >= pFormField->CountControls()) return FALSE;
				if (CPDF_FormControl* pFormControl = pFormField->GetControl(m_nFormControlIndex))
				{
					if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
					{
						FX_DWORD dwFlags = pWidget->GetFlags();
						if (bVP)
							dwFlags |= ANNOTFLAG_PRINT;
						else
							dwFlags &= ~ANNOTFLAG_PRINT;

						if (dwFlags != pWidget->GetFlags())
						{
							pWidget->SetFlags(dwFlags);
							UpdateFormControl(m_pDocument, pFormField->GetControl(m_nFormControlIndex), TRUE, FALSE, TRUE);
						}
					}
				}
			}
		}
	}
	else
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);
		
		CPDFSDK_Widget* pWidget = pInterForm->GetWidget(GetSmartFieldControl(pFormField));
		if (!pWidget) return FALSE;

		if (pWidget->GetFlags() & ANNOTFLAG_PRINT)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::radiosInUnison(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

	}
	else
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_RADIOSINUNISON)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::readonly(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

	}
	else
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldFlags() & FIELDFLAG_READONLY)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::rect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;
		if (!vp.IsArrayObject())return FALSE;

		CJS_Array rcArray(m_isolate);
		vp >> rcArray;
		CJS_Value Upper_Leftx(m_isolate), Upper_Lefty(m_isolate), Lower_Rightx(m_isolate), Lower_Righty(m_isolate);
		rcArray.GetElement(0, Upper_Leftx);
		rcArray.GetElement(1, Upper_Lefty);
		rcArray.GetElement(2, Lower_Rightx);
		rcArray.GetElement(3, Lower_Righty);

		FX_FLOAT pArray[4] = {0.0f,0.0f,0.0f,0.0f};
		pArray[0] = (FX_FLOAT)(FX_INT32)Upper_Leftx;
		pArray[1] = (FX_FLOAT)(FX_INT32)Lower_Righty;
		pArray[2] = (FX_FLOAT)(FX_INT32)Lower_Rightx;
		pArray[3] = (FX_FLOAT)(FX_INT32)Upper_Lefty;

		CPDF_Rect crRect(pArray);

		if (m_bDelay)
		{
			AddDelay_Rect(FP_RECT, crRect);
		}
		else
		{
			Field::SetRect(m_pDocument, m_FieldName, m_nFormControlIndex, crRect);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
		ASSERT(pInterForm != NULL);

		CPDFSDK_Widget* pWidget = pInterForm->GetWidget(GetSmartFieldControl(pFormField));
		if (!pWidget) return FALSE;

		CFX_FloatRect crRect = pWidget->GetRect();
		CJS_Value Upper_Leftx(m_isolate),Upper_Lefty(m_isolate),Lower_Rightx(m_isolate),Lower_Righty(m_isolate);
		Upper_Leftx = (FX_INT32)crRect.left;
		Upper_Lefty = (FX_INT32)crRect.top;
		Lower_Rightx = (FX_INT32)crRect.right;
		Lower_Righty = (FX_INT32)crRect.bottom;

		CJS_Array rcArray(m_isolate);
		rcArray.SetElement(0,Upper_Leftx);
		rcArray.SetElement(1,Upper_Lefty);
		rcArray.SetElement(2,Lower_Rightx);
		rcArray.SetElement(3,Lower_Righty);

		vp  <<  rcArray;			
	}

	return TRUE;
}

void Field::SetRect(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPDF_Rect& rect)
{
	ASSERT(pDocument != NULL);

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (nControlIndex < 0)
		{
			FX_BOOL bSet = FALSE;
			for (int i=0, sz=pFormField->CountControls(); i<sz; i++)
			{
				CPDF_FormControl* pFormControl = pFormField->GetControl(i);
				ASSERT(pFormControl != NULL);

				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					CPDF_Rect crRect = rect;

					CPDF_Page* pPDFPage = pWidget->GetPDFPage();
					ASSERT(pPDFPage != NULL);

// 					CPDF_Page* pPDFPage = pPage->GetPage();
// 					ASSERT(pPDFPage != NULL);

					crRect.Intersect(pPDFPage->GetPageBBox());

					if (!crRect.IsEmpty())
					{
						CPDF_Rect rcOld = pWidget->GetRect();
						if (crRect.left != rcOld.left ||
							crRect.right != rcOld.right ||
							crRect.top != rcOld.top ||
							crRect.bottom != rcOld.bottom)
						{
							pWidget->SetRect(crRect);
							bSet = TRUE;
						}
					}
				}
			}

			if (bSet) UpdateFormField(pDocument, pFormField, TRUE, TRUE, TRUE);
		}
		else
		{
			if(nControlIndex >= pFormField->CountControls()) return;
			if (CPDF_FormControl* pFormControl = pFormField->GetControl(nControlIndex))
			{
				if (CPDFSDK_Widget* pWidget = pInterForm->GetWidget(pFormControl))
				{
					CPDF_Rect crRect = rect;
					
					CPDF_Page* pPDFPage = pWidget->GetPDFPage();
					ASSERT(pPDFPage != NULL);

// 					CPDF_Page* pPDFPage = pPage->GetPage();
// 					ASSERT(pPDFPage != NULL);

					crRect.Intersect(pPDFPage->GetPageBBox());

					if (!crRect.IsEmpty())
					{
						CPDF_Rect rcOld = pWidget->GetRect();
						if (crRect.left != rcOld.left ||
							crRect.right != rcOld.right ||
							crRect.top != rcOld.top ||
							crRect.bottom != rcOld.bottom)
						{
							pWidget->SetRect(crRect);
							UpdateFormControl(pDocument, pFormControl, TRUE, TRUE, TRUE);
						}
					}
				}
			}
		}
	}
}

FX_BOOL Field::required(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;


		bool bVP;
		vp >> bVP;

	}
	else
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_REQUIRED)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

FX_BOOL Field::richText(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		bool bVP;
		vp >> bVP;

		if (m_bDelay)
		{
			AddDelay_Bool(FP_RICHTEXT, bVP);
		}
		else
		{
			Field::SetRichText(m_pDocument, m_FieldName, m_nFormControlIndex, bVP);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_TEXTFIELD)
			return FALSE;

		if (pFormField->GetFieldFlags() & FIELDFLAG_RICHTEXT)
			vp << true;
		else
			vp << false;
	}

	return TRUE;
}

void Field::SetRichText(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b)
{
	//Not supported.
}

FX_BOOL Field::richValue(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;
		;
	}
	else
	{
		;
	}
	return TRUE;
}

void Field::SetRichValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex)
{
	//Not supported.
}

FX_BOOL Field::rotation(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_ROTATION, nVP);
		}
		else
		{
			Field::SetRotation(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		vp << (FX_INT32)pFormControl->GetRotation();
	}

	return TRUE;
}

void Field::SetRotation(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::strokeColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		if (!vp.IsArrayObject())return FALSE;

		CJS_Array crArray(m_isolate);
		vp >> crArray;

		CPWL_Color color;
		color::ConvertArrayToPWLColor(crArray, color);

		if (m_bDelay)
		{
			AddDelay_Color(FP_STROKECOLOR, color);
		}
		else
		{
			Field::SetStrokeColor(m_pDocument, m_FieldName, m_nFormControlIndex, color);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		int iColorType;
		pFormControl->GetBorderColor(iColorType);

		CPWL_Color color;

		if (iColorType == COLORTYPE_TRANSPARENT)
		{
			color = CPWL_Color(COLORTYPE_TRANSPARENT);
		}
		else if (iColorType == COLORTYPE_GRAY)
		{
			color = CPWL_Color(COLORTYPE_GRAY, pFormControl->GetOriginalBorderColor(0));
		}
		else if (iColorType == COLORTYPE_RGB)
		{
			color = CPWL_Color(COLORTYPE_RGB, pFormControl->GetOriginalBorderColor(0),
				pFormControl->GetOriginalBorderColor(1),
				pFormControl->GetOriginalBorderColor(2));
		}
		else if (iColorType == COLORTYPE_CMYK)
		{
			color = CPWL_Color(COLORTYPE_CMYK, pFormControl->GetOriginalBorderColor(0),
				pFormControl->GetOriginalBorderColor(1),
				pFormControl->GetOriginalBorderColor(2),
				pFormControl->GetOriginalBorderColor(3));
		}
		else
			return FALSE;

		CJS_Array crArray(m_isolate);
		color::ConvertPWLColorToArray(color, crArray);
        vp  <<  crArray;
	}

	return TRUE;
}

void Field::SetStrokeColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color)
{
	//Not supported.
}

FX_BOOL Field::style(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_ByteString csBCaption;
		vp >> csBCaption;

		if (m_bDelay)
		{
			AddDelay_String(FP_STYLE, csBCaption);
		}
		else
		{
			Field::SetStyle(m_pDocument, m_FieldName, m_nFormControlIndex, csBCaption);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON && 
			pFormField->GetFieldType() != FIELDTYPE_CHECKBOX)
			return FALSE;

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl) return FALSE;

		CFX_WideString csWCaption = pFormControl->GetNormalCaption();
		CFX_ByteString csBCaption;

		switch (csWCaption[0])
		{
			case L'l':
				csBCaption = "circle";
				break;
			case L'8':
				csBCaption = "cross";
				break;
			case L'u':
				csBCaption = "diamond";
				break;
			case L'n':
				csBCaption = "square";
				break;
			case L'H':
				csBCaption = "star";
				break;
			default: //L'4'
				csBCaption = "check";
				break;
		}
		vp << csBCaption;
	}

	return TRUE;
}

void Field::SetStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, 
					 const CFX_ByteString& string)
{
	//Not supported.
}

FX_BOOL Field::submitName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Field::textColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CJS_Array crArray(m_isolate);
		if (!vp.IsArrayObject())return FALSE;
		vp >> crArray;

		CPWL_Color color;
		color::ConvertArrayToPWLColor(crArray, color);

		if (m_bDelay)
		{
			AddDelay_Color(FP_TEXTCOLOR, color);
		}
		else
		{
			Field::SetTextColor(m_pDocument, m_FieldName, m_nFormControlIndex, color);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;
		
		int iColorType;
		FX_ARGB color;
		CPDF_DefaultAppearance FieldAppearance = pFormControl->GetDefaultAppearance();
		FieldAppearance.GetColor(color, iColorType);
		FX_INT32 a,r,g,b;
		ArgbDecode(color, a, r, g, b);

		CPWL_Color crRet = CPWL_Color(COLORTYPE_RGB, r / 255.0f,
				g / 255.0f,
				b / 255.0f);

		if (iColorType == COLORTYPE_TRANSPARENT)
			crRet = CPWL_Color(COLORTYPE_TRANSPARENT);

		CJS_Array crArray(m_isolate);
		color::ConvertPWLColorToArray(crRet, crArray);
        vp  <<  crArray;		
	}

	return TRUE;
}

void Field::SetTextColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color)
{
	//Not supported.
}

FX_BOOL Field::textFont(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_ByteString csFontName;
		vp >> csFontName;
		if (csFontName.IsEmpty()) return FALSE;

		if (m_bDelay)
		{
			AddDelay_String(FP_TEXTFONT, csFontName);
		}
		else
		{
			Field::SetTextFont(m_pDocument, m_FieldName, m_nFormControlIndex, csFontName);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		int nFieldType = pFormField->GetFieldType();

		if (nFieldType == FIELDTYPE_PUSHBUTTON || 
			nFieldType == FIELDTYPE_COMBOBOX || 
			nFieldType == FIELDTYPE_LISTBOX ||
			nFieldType == FIELDTYPE_TEXTFIELD)
		{
			CPDF_Font * pFont = pFormControl->GetDefaultControlFont();
			if (!pFont) return FALSE;

			vp << pFont->GetBaseFont();
		}
		else
			return FALSE;
	}

	return TRUE;
}

void Field::SetTextFont(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string)
{
	//Not supported.
}

FX_BOOL Field::textSize(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		int nVP;
		vp >> nVP;

		if (m_bDelay)
		{
			AddDelay_Int(FP_TEXTSIZE, nVP);
		}
		else
		{
			Field::SetTextSize(m_pDocument, m_FieldName, m_nFormControlIndex, nVP);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
		if (!pFormControl)return FALSE;

		CPDF_DefaultAppearance FieldAppearance = pFormControl->GetDefaultAppearance();

		CFX_ByteString csFontNameTag;
		FX_FLOAT fFontSize;
		FieldAppearance.GetFont(csFontNameTag,fFontSize);

		vp << (int)fFontSize;
	}

	return TRUE;
}

void Field::SetTextSize(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number)
{
	//Not supported.
}

FX_BOOL Field::type(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!vp.IsGetting()) return FALSE;

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

 	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	switch (pFormField->GetFieldType())
	{
		case FIELDTYPE_UNKNOWN:
			vp << L"unknown";
			break;
		case FIELDTYPE_PUSHBUTTON:
			vp << L"button";
			break;
		case FIELDTYPE_CHECKBOX:
			vp << L"checkbox";
			break;
		case FIELDTYPE_RADIOBUTTON:
			vp << L"radiobutton";
			break;
		case FIELDTYPE_COMBOBOX:
			vp << L"combobox";
			break;
		case FIELDTYPE_LISTBOX:
			vp << L"listbox";
			break;
		case FIELDTYPE_TEXTFIELD:
			vp << L"text";
			break;
		case FIELDTYPE_SIGNATURE:
			vp << L"signature";
			break;
		default :
			vp << L"unknown";
			break;
	}

	return TRUE;
}

FX_BOOL Field::userName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

  	if (vp.IsSetting())
	{
		if (!m_bCanSet) return FALSE;

		CFX_WideString swName;
		vp >> swName;

		if (m_bDelay)
		{
			AddDelay_WideString(FP_USERNAME, swName);
		}
		else
		{
			Field::SetUserName(m_pDocument, m_FieldName, m_nFormControlIndex, swName);
		}	
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

 		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);

		vp << (CFX_WideString)pFormField->GetAlternateName();
	}

	return TRUE;
}

void Field::SetUserName(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_WideString& string)
{
	//Not supported.
}

FX_BOOL Field::value(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (vp.IsSetting())
	{		
		if (!m_bCanSet) return FALSE;

		CJS_WideStringArray strArray;

		if (vp.IsArrayObject())
		{
			CJS_Array ValueArray(m_isolate);
			vp.ConvertToArray(ValueArray);
			for (int i = 0,sz = ValueArray.GetLength(); i < sz; i++)
			{
				CJS_Value ElementValue(m_isolate);
				ValueArray.GetElement(i, ElementValue);
				strArray.Add(ElementValue.operator CFX_WideString());
			}
		}
		else
		{
			CFX_WideString swValue;
			vp >> swValue;

			strArray.Add(swValue);
		}

		if (m_bDelay)
		{
			AddDelay_WideStringArray(FP_VALUE, strArray);
		}
		else
		{
			Field::SetValue(m_pDocument, m_FieldName, m_nFormControlIndex, strArray);
		}
	}
	else
	{
		CFX_PtrArray FieldArray;
		GetFormFields(m_FieldName,FieldArray);
		if (FieldArray.GetSize() <= 0) return FALSE;

 		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
		ASSERT(pFormField != NULL);



		switch (pFormField->GetFieldType())
		{
		case FIELDTYPE_PUSHBUTTON:
			return FALSE;
		case FIELDTYPE_COMBOBOX:
		case FIELDTYPE_TEXTFIELD:
			{
				CFX_WideString swValue = pFormField->GetValue();

				double dRet;
				FX_BOOL bDot;
				if (CJS_PublicMethods::ConvertStringToNumber(swValue,dRet,bDot))
				{
					if (bDot)
						vp << dRet;
					else
						vp << dRet;
				}
				else
					vp << swValue;
			}
			break;
		case FIELDTYPE_LISTBOX:
			{
				if (pFormField->CountSelectedItems() > 1)
				{
					CJS_Array ValueArray(m_isolate);
					CJS_Value ElementValue(m_isolate);
					int iIndex;
					for (int i = 0, sz = pFormField->CountSelectedItems(); i < sz; i++)
					{
						iIndex = pFormField->GetSelectedIndex(i);
						ElementValue = pFormField->GetOptionValue(iIndex);
						if (FXSYS_wcslen((FX_LPCWSTR)ElementValue.operator CFX_WideString()) == 0)
							ElementValue = pFormField->GetOptionLabel(iIndex);
						ValueArray.SetElement(i, ElementValue);
					}
					vp << ValueArray;
				}
				else
				{
					CFX_WideString swValue = pFormField->GetValue();
				
					double dRet;
					FX_BOOL bDot;
					if (CJS_PublicMethods::ConvertStringToNumber(swValue,dRet,bDot))
					{
						if (bDot)
							vp << dRet;
						else
							vp << dRet;
					}
					else
						vp << swValue;	
				}
			}
			break;
		case FIELDTYPE_CHECKBOX:
		case FIELDTYPE_RADIOBUTTON:
			{
				FX_BOOL bFind = FALSE;
				for (int i = 0 , sz = pFormField->CountControls(); i < sz; i++)
				{
					if (pFormField->GetControl(i)->IsChecked())
					{
						CFX_WideString swValue = pFormField->GetControl(i)->GetExportValue();
						
						double dRet;
						FX_BOOL bDot;
						if (CJS_PublicMethods::ConvertStringToNumber(swValue,dRet,bDot))
						{
							if (bDot)
								vp << dRet;
							else
								vp << dRet;
						}
						else
							vp << swValue;

						bFind = TRUE;
						break;
					}
					else
						continue;
				}
				if (!bFind)
					vp << L"Off";					
			}
			break;
		default:
			vp << pFormField->GetValue();
			break;
		}
	}

	return TRUE;
}

void Field::SetValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, 
					 int nControlIndex, const CJS_WideStringArray& strArray)
{
	ASSERT(pDocument != NULL);

	if (strArray.GetSize() < 1) return;

	CFX_PtrArray FieldArray;
	GetFormFields(pDocument, swFieldName, FieldArray);

	for (int i=0,isz=FieldArray.GetSize(); i<isz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		if (pFormField->GetFullName().Compare(swFieldName) != 0)
			continue;

		switch (pFormField->GetFieldType())
		{
		case FIELDTYPE_TEXTFIELD:
		case FIELDTYPE_COMBOBOX:
			if (pFormField->GetValue() != strArray.GetAt(0))
			{
				CFX_WideString WideString = strArray.GetAt(0);
				pFormField->SetValue(strArray.GetAt(0), TRUE);	
				UpdateFormField(pDocument, pFormField, TRUE, FALSE, TRUE);
			}
			break;
		case FIELDTYPE_CHECKBOX: //mantis: 0004493
		case FIELDTYPE_RADIOBUTTON:
			{
				if (pFormField->GetValue() != strArray.GetAt(0))
				{
					pFormField->SetValue(strArray.GetAt(0), TRUE);	
					UpdateFormField(pDocument, pFormField, TRUE, FALSE, TRUE);
				}
			}
			break;
		case FIELDTYPE_LISTBOX:
			{
				FX_BOOL bModified = FALSE;

				for (int i=0,sz=strArray.GetSize(); i<sz; i++)
				{
					int iIndex = pFormField->FindOption(strArray.GetAt(i));

					if (!pFormField->IsItemSelected(iIndex))
					{
						bModified = TRUE;
						break;
					}
				}

				if (bModified)
				{
					pFormField->ClearSelection(TRUE);
					for (int i=0,sz=strArray.GetSize(); i<sz; i++)
					{
						int iIndex = pFormField->FindOption(strArray.GetAt(i));
						pFormField->SetItemSelection(iIndex, TRUE, TRUE);
					}

					UpdateFormField(pDocument, pFormField, TRUE, FALSE, TRUE);
				}
			}
			break;
		default:				
			break;
		}
	}
}

FX_BOOL Field::valueAsString(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!vp.IsGetting()) return FALSE;

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

   	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if (pFormField->GetFieldType() == FIELDTYPE_PUSHBUTTON)
		return FALSE;

	if (pFormField->GetFieldType() == FIELDTYPE_CHECKBOX)
	{
		if(!pFormField->CountControls()) return FALSE;

		if (pFormField->GetControl(0)->IsChecked())
			vp << L"Yes";
		else
			vp << L"Off";
	}
	else if (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON && !(pFormField->GetFieldFlags() & FIELDFLAG_RADIOSINUNISON))
	{
		for (int i=0, sz=pFormField->CountControls(); i<sz; i++)
		{
			if (pFormField->GetControl(i)->IsChecked())
			{
				vp << pFormField->GetControl(i)->GetExportValue().c_str();
				break;
			}
			else
				vp << L"Off";
		}
	}
	else if (pFormField->GetFieldType() == FIELDTYPE_LISTBOX && (pFormField->CountSelectedItems() > 1))
	{
		vp << L"";
	}
	else
		vp << pFormField->GetValue().c_str();

	return TRUE;
}

/* --------------------------------- methods --------------------------------- */

FX_BOOL Field::browseForFileToSubmit(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName, FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

 	CPDFDoc_Environment* pApp = m_pDocument->GetEnv();
 	ASSERT(pApp != NULL);

	if ((pFormField->GetFieldFlags() & FIELDFLAG_FILESELECT) && 
		(pFormField->GetFieldType() == FIELDTYPE_TEXTFIELD))
	{		
		CFX_WideString wsFileName = pApp->JS_fieldBrowse();
		if(!wsFileName.IsEmpty())
		{
 			pFormField->SetValue(wsFileName);
 			UpdateFormField(m_pDocument, pFormField, TRUE, TRUE, TRUE);
         }
	}
	else 
		return FALSE;

	return TRUE;
}


FX_BOOL Field::buttonGetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nface = 0;
	int iSize = params.size();
	if ( iSize >= 1)
		nface = (FX_INT32) params[0];

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);
	
	if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
		return FALSE;

	CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
	if (!pFormControl)return FALSE;
	
	if (nface == 0)
		vRet = pFormControl->GetNormalCaption();
	else if (nface == 1)
		vRet = pFormControl->GetDownCaption();
	else if (nface == 2)
		vRet = pFormControl->GetRolloverCaption();
	else
		return FALSE;

	return TRUE;
}

//#pragma warning(disable: 4800)

FX_BOOL Field::buttonGetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nface = 0;
	int iSize = params.size();
	if ( iSize >= 1)
		nface = (FX_INT32) params[0];
	
	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);
	
	if (pFormField->GetFieldType() != FIELDTYPE_PUSHBUTTON)
		return FALSE;

	CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
	if (!pFormControl)return FALSE;

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);

	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);
	
	JSFXObject pObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"Icon"));
	ASSERT(pObj.IsEmpty() == FALSE);
	
	CJS_Icon* pJS_Icon = (CJS_Icon*)JS_GetPrivate(pObj);
	ASSERT(pJS_Icon != NULL);

	Icon* pIcon = (Icon*)pJS_Icon->GetEmbedObject();
	ASSERT(pIcon != NULL);

	CPDF_Stream* pIconStream = NULL;
	if (nface == 0)
		pIconStream = pFormControl->GetNormalIcon();
	else if (nface == 1)
		pIconStream = pFormControl->GetDownIcon();
	else if (nface == 2)
		pIconStream = pFormControl->GetRolloverIcon();
	else
		return FALSE;

	pIcon->SetStream(pIconStream);
	vRet = pJS_Icon;

	return TRUE;
}

//#pragma warning(default: 4800)

FX_BOOL Field::buttonImportIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
#if 0  
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	if (!pFormField)return FALSE;

	CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
	ASSERT(pEnv);

	CFX_WideString sIconFileName = pEnv->JS_fieldBrowse();
	if (sIconFileName.IsEmpty()) 
	{
		vRet = 1;
		return TRUE;
	}

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDF_Stream* pStream = pInterForm->LoadImageFromFile(sIconFileName);
	if (!pStream) 
	{
		vRet = -1;
		return TRUE;
	}

	CPDF_FormControl* pFormControl = GetSmartFieldControl(pFormField);
	if (!pFormControl)return FALSE;

	pFormControl->SetNormalIcon(pStream);
	UpdateFormControl(m_pDocument, pFormControl, TRUE, TRUE, TRUE);

	vRet = 0;
#endif // 0
	return TRUE;
}

FX_BOOL Field::buttonSetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::buttonSetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::checkThisBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_bCanSet) return FALSE;

	int iSize = params.size();
	int nWidget = -1;
	if ( iSize >= 1)
		nWidget= (FX_INT32) params[0];
	else
		return FALSE;
	FX_BOOL bCheckit = TRUE;
	if ( iSize >= 2)
		bCheckit = params[1];


	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);
	
	if (pFormField->GetFieldType() != FIELDTYPE_CHECKBOX && pFormField->GetFieldType() != FIELDTYPE_RADIOBUTTON)
		return FALSE;	
	if(nWidget <0 || nWidget >= pFormField->CountControls())
		return FALSE;
	if (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON)
		pFormField->CheckControl(nWidget, bCheckit, TRUE);
	else
		pFormField->CheckControl(nWidget, bCheckit, TRUE);

	UpdateFormField(m_pDocument, pFormField, TRUE, TRUE, TRUE);

	return TRUE;
}

FX_BOOL Field::clearItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Field::defaultIsChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	if (!m_bCanSet) return FALSE;

	int iSize = params.size();
	int nWidget = -1;
	if ( iSize >= 1)
		nWidget= (FX_INT32) params[0];
	else
		return FALSE;
	//FX_BOOL bIsDefaultChecked = TRUE;
	//if ( iSize >= 2)
	//	bIsDefaultChecked =  params[1];

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if(nWidget <0 || nWidget >= pFormField->CountControls())
	{
		vRet = FALSE;
		return FALSE;
	}
	if ((pFormField->GetFieldType() == FIELDTYPE_CHECKBOX)
		|| (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON))
	{

		vRet = TRUE;
	}
	else
		vRet = FALSE;

	return TRUE;
}

FX_BOOL Field::deleteItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

int JS_COMPARESTRING(CFX_WideString* ps1, CFX_WideString* ps2)
{
	ASSERT(ps1 != NULL);
	ASSERT(ps2 != NULL);

	return ps1->Compare(*ps2);
}


FX_BOOL Field::getArray(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CGW_ArrayTemplate<CFX_WideString*> swSort;

	for (int i=0,sz=FieldArray.GetSize(); i<sz; i++)
	{
		CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(i);
		ASSERT(pFormField != NULL);

		swSort.Add(new CFX_WideString(pFormField->GetFullName()));
		
	}
	swSort.Sort(JS_COMPARESTRING);

	CJS_Context* pContext = (CJS_Context*)cc;
	ASSERT(pContext != NULL);
	CJS_Runtime* pRuntime = pContext->GetJSRuntime();
	ASSERT(pRuntime != NULL);

	CJS_Array FormFieldArray(m_isolate);
	for (int j=0,jsz = swSort.GetSize(); j<jsz; j++)
	{
		CFX_WideString* pStr = swSort.GetAt(j);

		JSFXObject pObj = JS_NewFxDynamicObj(*pRuntime, pContext, JS_GetObjDefnID(*pRuntime, L"Field"));
		ASSERT(pObj.IsEmpty() == FALSE);

		CJS_Field* pJSField = (CJS_Field*)JS_GetPrivate(pObj);
		ASSERT(pJSField != NULL);

		Field* pField = (Field*)pJSField->GetEmbedObject(); 
		ASSERT(pField != NULL);

		pField->AttachField(this->m_pJSDoc, *pStr);
	
		CJS_Value FormFieldValue(m_isolate);
		FormFieldValue = pJSField;
		FormFieldArray.SetElement(j, FormFieldValue);

		delete pStr;
	}

	vRet = FormFieldArray;
	swSort.RemoveAll();
	return TRUE;
}
	
FX_BOOL Field::getItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nIdx = -1;
	if (params.size() >=1)
		nIdx = (FX_INT32) params[0];
	FX_BOOL bExport = TRUE;
	int iSize = params.size();
	if ( iSize >= 2)
	{
		bExport =(FX_BOOL) params[1];
	}

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if ((pFormField->GetFieldType() == FIELDTYPE_LISTBOX)
		|| (pFormField->GetFieldType() == FIELDTYPE_COMBOBOX))
	{
		if (nIdx == -1 || nIdx > pFormField->CountOptions())
			nIdx = pFormField->CountOptions() -1;
		if (bExport)
		{
			CFX_WideString strval = pFormField->GetOptionValue(nIdx);
			if (strval.IsEmpty())
				vRet = pFormField->GetOptionLabel(nIdx);
			else
				vRet = strval;
		}
		else
			vRet = pFormField->GetOptionLabel(nIdx);
	}
	else
		return FALSE;

	return TRUE;
}

FX_BOOL Field::getLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::insertItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Field::isBoxChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nIndex = -1;
	if (params.size() >=1)
		nIndex = (FX_INT32) params[0];

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if(nIndex <0 || nIndex >= pFormField->CountControls())
	{
		vRet = FALSE;
		return FALSE;
	}

	if ((pFormField->GetFieldType() == FIELDTYPE_CHECKBOX)
		|| (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON))
	{
		if (pFormField->GetControl(nIndex)->IsChecked() !=0 )
			vRet = TRUE;
		else
			vRet = FALSE;
	}
	else
		vRet = FALSE;

	return TRUE;
}

FX_BOOL Field::isDefaultChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	int nIndex = -1;
	if (params.size() >=1)
		nIndex = (FX_INT32) params[0];

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	if(nIndex <0 || nIndex >= pFormField->CountControls())
	{
		vRet = FALSE;
		return FALSE;
	}
	if ((pFormField->GetFieldType() == FIELDTYPE_CHECKBOX)
		|| (pFormField->GetFieldType() == FIELDTYPE_RADIOBUTTON))
	{
		if (pFormField->GetControl(nIndex)->IsDefaultChecked() != 0)
			vRet = TRUE;
		else
			vRet = FALSE;
	}
	else
		vRet = FALSE;

	return TRUE;
}

FX_BOOL Field::setAction(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Field::setFocus(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	ASSERT(m_pDocument != NULL);

	CFX_PtrArray FieldArray;
	GetFormFields(m_FieldName,FieldArray);
	if (FieldArray.GetSize() <= 0) return FALSE;

	CPDF_FormField* pFormField = (CPDF_FormField*)FieldArray.ElementAt(0);
	ASSERT(pFormField != NULL);

	FX_INT32 nCount = pFormField->CountControls();

	if (nCount < 1) return FALSE;

	CPDFSDK_InterForm* pInterForm = (CPDFSDK_InterForm*)m_pDocument->GetInterForm();
	ASSERT(pInterForm != NULL);

	CPDFSDK_Widget* pWidget = NULL;
	if (nCount == 1)
	{	
		pWidget = pInterForm->GetWidget(pFormField->GetControl(0));
	}
	else
	{
		CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
		ASSERT(pEnv);
		CPDF_Page* pPage = (CPDF_Page*)pEnv->FFI_GetCurrentPage(m_pDocument->GetDocument());
		if(!pPage)
			return FALSE;
		if (CPDFSDK_PageView* pCurPageView = m_pDocument->GetPageView(pPage))
		{
			for (FX_INT32 i=0; i<nCount; i++)
			{
				if (CPDFSDK_Widget* pTempWidget =  pInterForm->GetWidget(pFormField->GetControl(i)))
				{				
					if (pTempWidget->GetPDFPage() == pCurPageView->GetPDFPage())
					{
						pWidget = pTempWidget;
						break;
					}
				}
			}
		}
	}

	if (pWidget)
	{
		m_pDocument->SetFocusAnnot(pWidget);
	}

	return TRUE;
}

FX_BOOL Field::setItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return TRUE;
}

FX_BOOL Field::setLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureGetModifications(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureGetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureInfo(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureSetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureSign(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::signatureValidate(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError)
{
	return FALSE;
}

FX_BOOL Field::source(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError)
{
	if (vp.IsGetting())
	{
		vp << (CJS_Object*)NULL;
	}

	return TRUE;
}

/////////////////////////////////////////// delay /////////////////////////////////////////////

void Field::AddDelay_Int(enum FIELD_PROP prop, FX_INT32 n)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->num = n;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_Bool(enum FIELD_PROP prop,bool b)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->b = b;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_String(enum FIELD_PROP prop, const CFX_ByteString& string)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->string = string;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_WideString(enum FIELD_PROP prop, const CFX_WideString& string)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->widestring = string;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_Rect(enum FIELD_PROP prop, const CPDF_Rect& rect)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->rect = rect;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_Color(enum FIELD_PROP prop, const CPWL_Color& color)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	pNewData->color = color;

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_WordArray(enum FIELD_PROP prop, const CFX_DWordArray& array)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;

	for (int i=0,sz=array.GetSize(); i<sz; i++)
		pNewData->wordarray.Add(array.GetAt(i));

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::AddDelay_WideStringArray(enum FIELD_PROP prop, const CJS_WideStringArray& array)
{
	ASSERT(m_pJSDoc != NULL);

	CJS_DelayData* pNewData = new CJS_DelayData;
	pNewData->sFieldName = m_FieldName;
	pNewData->nControlIndex = m_nFormControlIndex;
	pNewData->eProp = prop;
	for (int i=0,sz=array.GetSize(); i<sz; i++)
		pNewData->widestringarray.Add(array.GetAt(i));

	m_pJSDoc->AddDelayData(pNewData);
}

void Field::DoDelay(CPDFSDK_Document* pDocument, CJS_DelayData* pData)
{
	ASSERT(pDocument != NULL);
	ASSERT(pData != NULL);

	switch (pData->eProp)
	{
	case FP_ALIGNMENT:
		Field::SetAlignment(pDocument, pData->sFieldName, pData->nControlIndex, pData->string);
		break;
	case FP_BORDERSTYLE:
		Field::SetBorderStyle(pDocument, pData->sFieldName, pData->nControlIndex, pData->string);
		break;
	case FP_BUTTONALIGNX:
		Field::SetButtonAlignX(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_BUTTONALIGNY:
		Field::SetButtonAlignY(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_BUTTONFITBOUNDS:
		Field::SetButtonFitBounds(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_BUTTONPOSITION:
		Field::SetButtonPosition(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_BUTTONSCALEHOW:
		Field::SetButtonScaleHow(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_BUTTONSCALEWHEN:
		Field::SetButtonScaleWhen(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_CALCORDERINDEX:
		Field::SetCalcOrderIndex(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_CHARLIMIT:
		Field::SetCharLimit(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_COMB:
		Field::SetComb(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_COMMITONSELCHANGE:
		Field::SetCommitOnSelChange(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_CURRENTVALUEINDICES:
		Field::SetCurrentValueIndices(pDocument, pData->sFieldName, pData->nControlIndex, pData->wordarray);
		break;
	case FP_DEFAULTVALUE:
		Field::SetDefaultValue(pDocument, pData->sFieldName, pData->nControlIndex, pData->widestring);
		break;
	case FP_DONOTSCROLL:
		Field::SetDoNotScroll(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_DISPLAY:
		Field::SetDisplay(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_FILLCOLOR:
		Field::SetFillColor(pDocument, pData->sFieldName, pData->nControlIndex, pData->color);
		break;
	case FP_HIDDEN:
		Field::SetHidden(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_HIGHLIGHT:
		Field::SetHighlight(pDocument, pData->sFieldName, pData->nControlIndex, pData->string);
		break;
	case FP_LINEWIDTH:
		Field::SetLineWidth(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_MULTILINE:
		Field::SetMultiline(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_MULTIPLESELECTION:
		Field::SetMultipleSelection(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_PASSWORD:
		Field::SetPassword(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_RECT:
		Field::SetRect(pDocument, pData->sFieldName, pData->nControlIndex, pData->rect);
		break;
	case FP_RICHTEXT:
		Field::SetRichText(pDocument, pData->sFieldName, pData->nControlIndex, pData->b);
		break;
	case FP_RICHVALUE:
		break;
	case FP_ROTATION:
		Field::SetRotation(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_STROKECOLOR:
		Field::SetStrokeColor(pDocument, pData->sFieldName, pData->nControlIndex, pData->color);
		break;
	case FP_STYLE:
		Field::SetStyle(pDocument, pData->sFieldName, pData->nControlIndex, pData->string);
		break;
	case FP_TEXTCOLOR:
		Field::SetTextColor(pDocument, pData->sFieldName, pData->nControlIndex, pData->color);
		break;
	case FP_TEXTFONT:
		Field::SetTextFont(pDocument, pData->sFieldName, pData->nControlIndex, pData->string);
		break;
	case FP_TEXTSIZE:
		Field::SetTextSize(pDocument, pData->sFieldName, pData->nControlIndex, pData->num);
		break;
	case FP_USERNAME:
		Field::SetUserName(pDocument, pData->sFieldName, pData->nControlIndex, pData->widestring);
		break;
	case FP_VALUE:
		Field::SetValue(pDocument, pData->sFieldName, pData->nControlIndex, pData->widestringarray);
		break;
	}
}

#define JS_FIELD_MINWIDTH	1
#define JS_FIELD_MINHEIGHT	1

void Field::AddField(CPDFSDK_Document* pDocument, int nPageIndex, int nFieldType,
							const CFX_WideString& sName, const CPDF_Rect& rcCoords)
{
	//Not supported.
}

