// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fsdk_define.h"
#include "../include/fsdk_mgr.h"
#include "../include/fsdk_baseannot.h"
#include "../include/fsdk_baseform.h"
#include "../include/formfiller/FFL_FormFiller.h"
#include "../include/fsdk_actionhandler.h"

#include "../include/javascript/IJavaScript.h"

//------------------------------------------------------------------------------------
//*										CPDFSDK_Widget 
//------------------------------------------------------------------------------------

#define IsFloatZero(f)						((f) < 0.01 && (f) > -0.01)
#define IsFloatBigger(fa,fb)				((fa) > (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatSmaller(fa,fb)				((fa) < (fb) && !IsFloatZero((fa) - (fb)))
#define IsFloatEqual(fa,fb)					IsFloatZero((fa)-(fb))

CPDFSDK_Widget::CPDFSDK_Widget(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPageView, CPDFSDK_InterForm* pInterForm) :
					CPDFSDK_Annot(pAnnot, pPageView),
					m_pInterForm(pInterForm),
					m_nAppAge(0),
					m_nValueAge(0)
{
	ASSERT(m_pInterForm != NULL);
}

CPDFSDK_Widget::~CPDFSDK_Widget()
{

}

FX_BOOL		CPDFSDK_Widget::IsWidgetAppearanceValid(CPDF_Annot::AppearanceMode mode)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);
	
	CPDF_Dictionary* pAP = m_pAnnot->m_pAnnotDict->GetDict("AP");
	if (pAP == NULL) return FALSE;
	
	// Choose the right sub-ap
	const FX_CHAR* ap_entry = "N";
	if (mode == CPDF_Annot::Down)
		ap_entry = "D";
	else if (mode == CPDF_Annot::Rollover)
		ap_entry = "R";
	if (!pAP->KeyExist(ap_entry))
		ap_entry = "N";
	
	// Get the AP stream or subdirectory
	CPDF_Object* psub = pAP->GetElementValue(ap_entry);
	if (psub == NULL) return FALSE;
	
	int nFieldType = GetFieldType();
	switch (nFieldType)
	{
	case FIELDTYPE_PUSHBUTTON:
	case FIELDTYPE_COMBOBOX:
	case FIELDTYPE_LISTBOX:
	case FIELDTYPE_TEXTFIELD:
	case FIELDTYPE_SIGNATURE:
		return psub->GetType() == PDFOBJ_STREAM;
	case FIELDTYPE_CHECKBOX:
	case FIELDTYPE_RADIOBUTTON:
		if (psub->GetType() == PDFOBJ_DICTIONARY) 
		{
			CPDF_Dictionary* pSubDict = (CPDF_Dictionary*)psub;
			
			return pSubDict->GetStream(this->GetAppState()) != NULL;
		}
		else
			return FALSE;
		break;
	}
	
	return TRUE;
}

int	CPDFSDK_Widget::GetFieldType() const
{
	CPDF_FormField* pField = GetFormField();
	ASSERT(pField != NULL);
	
	return pField->GetFieldType();
}

int CPDFSDK_Widget::GetFieldFlags() const
{
	CPDF_InterForm* pPDFInterForm = m_pInterForm->GetInterForm();
	ASSERT(pPDFInterForm != NULL);

	CPDF_FormControl* pFormControl = pPDFInterForm->GetControlByDict(m_pAnnot->m_pAnnotDict);
	CPDF_FormField* pFormField = pFormControl->GetField();
	return pFormField->GetFieldFlags();
}

CFX_ByteString CPDFSDK_Widget::GetSubType() const
{
	int nType = GetFieldType();
	
	if (nType == FIELDTYPE_SIGNATURE)
		return BFFT_SIGNATURE;
	return CPDFSDK_Annot::GetSubType();
}

CPDF_FormField*	CPDFSDK_Widget::GetFormField() const
{
	ASSERT(m_pInterForm != NULL);
	
	CPDF_FormControl* pCtrl = GetFormControl();	
	ASSERT(pCtrl != NULL);
	
	return pCtrl->GetField();
}

CPDF_FormControl* CPDFSDK_Widget::GetFormControl() const
{
	ASSERT(m_pInterForm != NULL);
	
	CPDF_InterForm* pPDFInterForm = m_pInterForm->GetInterForm();
	ASSERT(pPDFInterForm != NULL);
	
	return pPDFInterForm->GetControlByDict(GetAnnotDict());
}
static CPDF_Dictionary* BF_GetField(CPDF_Dictionary* pFieldDict, const FX_CHAR* name)
{
	if (pFieldDict == NULL) return NULL;
	// First check the dictionary itself
	CPDF_Object* pAttr = pFieldDict->GetElementValue(name);
	if (pAttr) return pFieldDict;
	
	// Now we need to search from parents
	CPDF_Dictionary* pParent = pFieldDict->GetDict("Parent");
	if (pParent == NULL) return NULL;
	
	return BF_GetField(pParent, name);
}

CPDF_FormControl* CPDFSDK_Widget::GetFormControl(CPDF_InterForm* pInterForm, CPDF_Dictionary* pAnnotDict)
{
	ASSERT(pInterForm != NULL);
	ASSERT(pAnnotDict != NULL);
	
	CPDF_FormControl* pControl = pInterForm->GetControlByDict(pAnnotDict);
	
	return pControl;
}

int CPDFSDK_Widget::GetRotate() const
{
	CPDF_FormControl* pCtrl = this->GetFormControl();
	ASSERT(pCtrl != NULL);
	
	return pCtrl->GetRotation() % 360;
}

FX_BOOL	CPDFSDK_Widget::GetFillColor(FX_COLORREF& color) const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	int iColorType = 0;	
	color = FX_ARGBTOCOLORREF(pFormCtrl->GetBackgroundColor(iColorType));
	
	return iColorType != COLORTYPE_TRANSPARENT;
}

FX_BOOL	CPDFSDK_Widget::GetBorderColor(FX_COLORREF& color) const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	int iColorType = 0;	
	color = FX_ARGBTOCOLORREF(pFormCtrl->GetBorderColor(iColorType));
	
	return iColorType != COLORTYPE_TRANSPARENT;
}

FX_BOOL	CPDFSDK_Widget::GetTextColor(FX_COLORREF& color) const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	CPDF_DefaultAppearance da = pFormCtrl->GetDefaultAppearance();
	if (da.HasColor())
	{
		FX_ARGB argb;
		int iColorType = COLORTYPE_TRANSPARENT;	
		da.GetColor(argb, iColorType);
		color = FX_ARGBTOCOLORREF(argb);
		
		return iColorType != COLORTYPE_TRANSPARENT;
	}
	
	return FALSE;
}

FX_FLOAT CPDFSDK_Widget::GetFontSize() const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	CPDF_DefaultAppearance pDa = pFormCtrl->GetDefaultAppearance();
	CFX_ByteString csFont = "";
	FX_FLOAT fFontSize = 0.0f;
	pDa.GetFont(csFont, fFontSize);
	
	return fFontSize;
}

int	CPDFSDK_Widget::GetSelectedIndex(int nIndex) const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetSelectedIndex(nIndex);
}

CFX_WideString CPDFSDK_Widget::GetValue() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetValue();
}

CFX_WideString CPDFSDK_Widget::GetDefaultValue() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetDefaultValue();
}

CFX_WideString CPDFSDK_Widget::GetOptionLabel(int nIndex) const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetOptionLabel(nIndex);
}

int	CPDFSDK_Widget::CountOptions() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->CountOptions();
}

FX_BOOL	CPDFSDK_Widget::IsOptionSelected(int nIndex) const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->IsItemSelected(nIndex);
}

int	CPDFSDK_Widget::GetTopVisibleIndex() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetTopVisibleIndex();
}

FX_BOOL	CPDFSDK_Widget::IsChecked() const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	return pFormCtrl->IsChecked();
}

int	CPDFSDK_Widget::GetAlignment() const
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	return pFormCtrl->GetControlAlignment();
}

int	CPDFSDK_Widget::GetMaxLen() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	return pFormField->GetMaxLen();
}

void CPDFSDK_Widget::SetCheck(FX_BOOL bChecked, FX_BOOL bNotify)
{
	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);
	
	CPDF_FormField*	pFormField = pFormCtrl->GetField();
	ASSERT(pFormField != NULL);
	
	pFormField->CheckControl(pFormField->GetControlIndex(pFormCtrl), bChecked, bNotify);
}

void CPDFSDK_Widget::SetValue(const CFX_WideString& sValue, FX_BOOL bNotify)
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	pFormField->SetValue(sValue, bNotify);
}

void CPDFSDK_Widget::SetDefaultValue(const CFX_WideString& sValue)
{
}
void CPDFSDK_Widget::SetOptionSelection(int index, FX_BOOL bSelected, FX_BOOL bNotify)
{
	CPDF_FormField* pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	pFormField->SetItemSelection(index, bSelected, bNotify);
}

void CPDFSDK_Widget::ClearSelection(FX_BOOL bNotify)
{
	CPDF_FormField* pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	pFormField->ClearSelection(bNotify);
}

void CPDFSDK_Widget::SetTopVisibleIndex(int index)
{
}

void CPDFSDK_Widget::SetAppModified()
{
	m_bAppModified = TRUE;
}

void CPDFSDK_Widget::ClearAppModified()
{
	m_bAppModified = FALSE;
}

FX_BOOL CPDFSDK_Widget::IsAppModified() const
{
	return m_bAppModified;
}

void CPDFSDK_Widget::ResetAppearance(FX_LPCWSTR sValue, FX_BOOL bValueChanged)
{
	SetAppModified();

	m_nAppAge++;
	if (m_nAppAge > 999999)
		m_nAppAge = 0;
	if (bValueChanged)
		m_nValueAge++;

	int nFieldType = GetFieldType();
	
	switch (nFieldType)
	{
	case FIELDTYPE_PUSHBUTTON:
		ResetAppearance_PushButton();
		break;
	case FIELDTYPE_CHECKBOX:
		ResetAppearance_CheckBox();
		break;
	case FIELDTYPE_RADIOBUTTON:
		ResetAppearance_RadioButton();
		break;
	case FIELDTYPE_COMBOBOX:
		ResetAppearance_ComboBox(sValue);
		break;
	case FIELDTYPE_LISTBOX:
		ResetAppearance_ListBox();
		break;
	case FIELDTYPE_TEXTFIELD:
		ResetAppearance_TextField(sValue);
		break;
	}
	
	ASSERT(m_pAnnot != NULL);
	m_pAnnot->ClearCachedAP();
}

CFX_WideString CPDFSDK_Widget::OnFormat(int nCommitKey, FX_BOOL& bFormated)
{
 	CPDF_FormField* pFormField = GetFormField();
 	ASSERT(pFormField != NULL);
 	
 	ASSERT(m_pInterForm != NULL);
	
	return m_pInterForm->OnFormat(pFormField, nCommitKey, bFormated);

}

void CPDFSDK_Widget::ResetFieldAppearance(FX_BOOL bValueChanged)
{
	CPDF_FormField* pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	ASSERT(m_pInterForm != NULL);

	m_pInterForm->ResetFieldAppearance(pFormField, NULL, bValueChanged);
}

void	CPDFSDK_Widget::DrawAppearance(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
		CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions)
{
	int nFieldType = GetFieldType();
	
	if ((nFieldType == FIELDTYPE_CHECKBOX || nFieldType == FIELDTYPE_RADIOBUTTON) &&
		mode == CPDF_Annot::Normal && 
		!this->IsWidgetAppearanceValid(CPDF_Annot::Normal))
	{
		CFX_PathData pathData;
		
		CPDF_Rect rcAnnot = this->GetRect();
		
		pathData.AppendRect(rcAnnot.left, rcAnnot.bottom,
			rcAnnot.right, rcAnnot.top);
		
		CFX_GraphStateData gsd;
		gsd.m_LineWidth = 0.0f;
		
		pDevice->DrawPath(&pathData, pUser2Device, &gsd, 0, 0xFFAAAAAA, FXFILL_ALTERNATE);
	}
	else
	{
		CPDFSDK_Annot::DrawAppearance(pDevice, pUser2Device, mode, pOptions);
	}
}

void CPDFSDK_Widget::UpdateField()
{
	CPDF_FormField* pFormField = GetFormField();
	ASSERT(pFormField != NULL);
	
	ASSERT(m_pInterForm != NULL);
	m_pInterForm->UpdateField(pFormField);
}

void CPDFSDK_Widget::DrawShadow(CFX_RenderDevice* pDevice, CPDFSDK_PageView* pPageView)
{
 	ASSERT(m_pInterForm != NULL);
 
	int nFieldType = GetFieldType();
 	if (m_pInterForm->IsNeedHighLight(nFieldType))
 	{
 
//  		if (nFieldType != FIELDTYPE_PUSHBUTTON)
//  		{
			CPDF_Rect rc  = GetRect();
			FX_COLORREF color = m_pInterForm->GetHighlightColor(nFieldType);
			FX_BYTE alpha = m_pInterForm->GetHighlightAlpha();

			CFX_FloatRect rcDevice;
			ASSERT(m_pInterForm->GetDocument());
			CPDFDoc_Environment* pEnv = m_pInterForm->GetDocument()->GetEnv();
			if(!pEnv)
				return;
			CFX_AffineMatrix page2device;
			pPageView->GetCurrentMatrix(page2device);
			page2device.Transform(((FX_FLOAT)rc.left), ((FX_FLOAT)rc.bottom), rcDevice.left, rcDevice.bottom);
// 			pEnv->FFI_PageToDevice(m_pPageView->GetPDFPage(), rc.left, rc.bottom, &rcDevice.left, &rcDevice.bottom);
// 			pEnv->FFI_PageToDevice(m_pPageView->GetPDFPage(), rc.right, rc.top, &rcDevice.right, &rcDevice.top);
			page2device.Transform(((FX_FLOAT)rc.right), ((FX_FLOAT)rc.top), rcDevice.right, rcDevice.top);

			rcDevice.Normalize();

			FX_ARGB argb = ArgbEncode((int)alpha, color);
			FX_RECT rcDev((int)rcDevice.left,(int)rcDevice.top,(int)rcDevice.right,(int)rcDevice.bottom);
			pDevice->FillRect(&rcDev, argb);	
			/* 		}*/
	}
}

void CPDFSDK_Widget::ResetAppearance_PushButton()
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);


	
	CPDF_Rect rcWindow = GetRotatedRect();	

	FX_INT32 nLayout = 0;

	switch (pControl->GetTextPosition())
	{
	case TEXTPOS_ICON:
		nLayout = PPBL_ICON;
		break;
	case TEXTPOS_BELOW:
		nLayout = PPBL_ICONTOPLABELBOTTOM;
		break;
	case TEXTPOS_ABOVE:
		nLayout = PPBL_LABELTOPICONBOTTOM;
		break;
	case TEXTPOS_RIGHT:
		nLayout = PPBL_ICONLEFTLABELRIGHT;
		break;
	case TEXTPOS_LEFT:
		nLayout = PPBL_LABELLEFTICONRIGHT;
		break;
	case TEXTPOS_OVERLAID:
		nLayout = PPBL_LABELOVERICON;
		break;
	default:
		nLayout = PPBL_LABEL;
		break;
	}

	CPWL_Color crBackground, crBorder;

	int iColorType;
	FX_FLOAT fc[4];

	pControl->GetOriginalBackgroundColor(iColorType, fc);
	if (iColorType > 0)
		crBackground = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	pControl->GetOriginalBorderColor(iColorType, fc);
	if (iColorType > 0)
		crBorder = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
	FX_INT32 nBorderStyle = 0;
	CPWL_Dash dsBorder(3,0,0);
	CPWL_Color crLeftTop,crRightBottom;

	switch (GetBorderStyle())
	{
	case BBS_DASH:
		nBorderStyle = PBS_DASH;
		dsBorder = CPWL_Dash(3, 3, 0);
		break;
	case BBS_BEVELED:
		nBorderStyle = PBS_BEVELED;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,1);
		crRightBottom = CPWL_Utils::DevideColor(crBackground,2);
		break;
	case BBS_INSET:
		nBorderStyle = PBS_INSET;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,0.5);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY,0.75);
		break;
	case BBS_UNDERLINE:
		nBorderStyle = PBS_UNDERLINED;
		break;
	default: 
		nBorderStyle = PBS_SOLID;
		break;
	}

	CPDF_Rect rcClient = CPWL_Utils::DeflateRect(rcWindow,fBorderWidth);	

	CPWL_Color crText(COLORTYPE_GRAY,0);

	FX_FLOAT fFontSize = 12.0f;
	CFX_ByteString csNameTag;

	CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
	if (da.HasColor())
	{
		da.GetColor(iColorType, fc);
		crText = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
	}

	if (da.HasFont()) 
		da.GetFont(csNameTag, fFontSize);

	CFX_WideString csWCaption;
	CFX_WideString csNormalCaption, csRolloverCaption, csDownCaption;

	if (pControl->HasMKEntry("CA"))
	{
		csNormalCaption = pControl->GetNormalCaption();
	}
	if (pControl->HasMKEntry("RC"))
	{
		csRolloverCaption = pControl->GetRolloverCaption();
	}
	if (pControl->HasMKEntry("AC"))
	{
		csDownCaption = pControl->GetDownCaption();
	}

	CPDF_Stream* pNormalIcon = NULL;
	CPDF_Stream* pRolloverIcon = NULL;
	CPDF_Stream* pDownIcon = NULL;

	if (pControl->HasMKEntry("I"))
	{
		pNormalIcon = pControl->GetNormalIcon();
	}
	if (pControl->HasMKEntry("RI"))
	{
		pRolloverIcon = pControl->GetRolloverIcon();
	}
	if (pControl->HasMKEntry("IX"))
	{
		pDownIcon = pControl->GetDownIcon();
	}

	if (pNormalIcon)
	{
		if (CPDF_Dictionary* pImageDict = pNormalIcon->GetDict())
		{
			if (pImageDict->GetString("Name").IsEmpty())
				pImageDict->SetAtString("Name", "ImgA");
		}
	}

	if (pRolloverIcon)
	{
		if (CPDF_Dictionary* pImageDict = pRolloverIcon->GetDict())
		{
			if (pImageDict->GetString("Name").IsEmpty())
				pImageDict->SetAtString("Name", "ImgB");
		}
	}

	if (pDownIcon)
	{
		if (CPDF_Dictionary* pImageDict = pDownIcon->GetDict())
		{
			if (pImageDict->GetString("Name").IsEmpty())
				pImageDict->SetAtString("Name", "ImgC");
		}
	}

	CPDF_IconFit iconFit = pControl->GetIconFit();

// 	ASSERT(this->m_pBaseForm != NULL);
	ASSERT(this->m_pInterForm != NULL);
	CPDFSDK_Document* pDoc = m_pInterForm->GetDocument();
	ASSERT(pDoc != NULL);
	CPDFDoc_Environment* pEnv = pDoc->GetEnv();

 	CBA_FontMap FontMap(this,pEnv->GetSysHandler());//, ISystemHandle::GetSystemHandler(m_pBaseForm->GetEnv()));
	FontMap.Initial();

	FontMap.SetAPType("N");

	CFX_ByteString csAP = CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) + 
		CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder, crLeftTop, crRightBottom, nBorderStyle, dsBorder) +
		CPWL_Utils::GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient, &FontMap, pNormalIcon, iconFit, csNormalCaption, crText, fFontSize, nLayout);

	WriteAppearance("N", GetRotatedRect(), GetMatrix(), csAP);
	if (pNormalIcon)
		AddImageToAppearance("N", pNormalIcon);

	CPDF_FormControl::HighlightingMode eHLM = pControl->GetHighlightingMode();
	if (eHLM == CPDF_FormControl::Push || eHLM == CPDF_FormControl::Toggle)
	{
		if (csRolloverCaption.IsEmpty() && !pRolloverIcon)			
		{
			csRolloverCaption = csNormalCaption;
			pRolloverIcon = pNormalIcon;
		}

		FontMap.SetAPType("R");

		csAP = CPWL_Utils::GetRectFillAppStream(rcWindow, crBackground) + 
				CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder, crLeftTop, crRightBottom, nBorderStyle, dsBorder) +
				CPWL_Utils::GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient, &FontMap, pRolloverIcon, iconFit, csRolloverCaption, crText, fFontSize, nLayout);

		WriteAppearance("R", GetRotatedRect(), GetMatrix(), csAP);
		if (pRolloverIcon)
			AddImageToAppearance("R", pRolloverIcon);

		if (csDownCaption.IsEmpty() && !pDownIcon)
		{
			csDownCaption = csNormalCaption;
			pDownIcon = pNormalIcon;
		}

		switch (nBorderStyle)
		{
		case PBS_BEVELED:
			{
				CPWL_Color crTemp = crLeftTop;
				crLeftTop = crRightBottom;
				crRightBottom = crTemp;
			}
			break;
		case PBS_INSET:
			crLeftTop = CPWL_Color(COLORTYPE_GRAY,0);
			crRightBottom = CPWL_Color(COLORTYPE_GRAY,1);
			break;
		}
		
		FontMap.SetAPType("D");

		csAP = CPWL_Utils::GetRectFillAppStream(rcWindow, CPWL_Utils::SubstractColor(crBackground,0.25f)) + 
			CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder, crLeftTop, crRightBottom, nBorderStyle, dsBorder) + 
			CPWL_Utils::GetPushButtonAppStream(iconFit.GetFittingBounds() ? rcWindow : rcClient, &FontMap, pDownIcon, iconFit, csDownCaption, crText, fFontSize, nLayout);

		WriteAppearance("D", GetRotatedRect(), GetMatrix(), csAP);
		if (pDownIcon)
			AddImageToAppearance("D", pDownIcon);
	}
	else
	{
		RemoveAppearance("D");
		RemoveAppearance("R");
	}
}

void CPDFSDK_Widget::ResetAppearance_CheckBox()
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);



	CPWL_Color crBackground, crBorder, crText;
	
	int iColorType;
	FX_FLOAT fc[4];

	pControl->GetOriginalBackgroundColor(iColorType, fc);
	if (iColorType > 0)
		crBackground = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	pControl->GetOriginalBorderColor(iColorType, fc);
	if (iColorType > 0)
		crBorder = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
	FX_INT32 nBorderStyle = 0;
	CPWL_Dash dsBorder(3,0,0);
	CPWL_Color crLeftTop,crRightBottom;

	switch (GetBorderStyle())
	{
	case BBS_DASH:
		nBorderStyle = PBS_DASH;
		dsBorder = CPWL_Dash(3, 3, 0);
		break;
	case BBS_BEVELED:
		nBorderStyle = PBS_BEVELED;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,1);
		crRightBottom = CPWL_Utils::DevideColor(crBackground,2);
		break;
	case BBS_INSET:
		nBorderStyle = PBS_INSET;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,0.5);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY,0.75);
		break;
	case BBS_UNDERLINE:
		nBorderStyle = PBS_UNDERLINED;
		break;
	default: 
		nBorderStyle = PBS_SOLID;
		break;
	}

	CPDF_Rect rcWindow = GetRotatedRect();
	CPDF_Rect rcClient = CPWL_Utils::DeflateRect(rcWindow,fBorderWidth);

	CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
	if (da.HasColor())
	{
		da.GetColor(iColorType, fc);
		crText = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
	}

	FX_INT32 nStyle = 0;

	CFX_WideString csWCaption = pControl->GetNormalCaption();
	if (csWCaption.GetLength() > 0)
	{
		switch (csWCaption[0])
		{
		case L'l':
			nStyle = PCS_CIRCLE;			
			break;
		case L'8':
			nStyle = PCS_CROSS;
			break;
		case L'u':
			nStyle = PCS_DIAMOND;
			break;
		case L'n':
			nStyle = PCS_SQUARE;
			break;
		case L'H':
			nStyle = PCS_STAR;
			break;
		default: //L'4'
			nStyle = PCS_CHECK;
			break;
		}
	}
	else
	{
		nStyle = PCS_CHECK;
	}

	CFX_ByteString csAP_N_ON = CPWL_Utils::GetRectFillAppStream(rcWindow,crBackground) +
		CPWL_Utils::GetBorderAppStream(rcWindow,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);

	CFX_ByteString csAP_N_OFF = csAP_N_ON;

	switch (nBorderStyle)
	{
	case PBS_BEVELED:
		{
			CPWL_Color crTemp = crLeftTop;
			crLeftTop = crRightBottom;
			crRightBottom = crTemp;
		}
		break;
	case PBS_INSET:
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,0);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY,1);
		break;
	}

	CFX_ByteString csAP_D_ON = CPWL_Utils::GetRectFillAppStream(rcWindow,CPWL_Utils::SubstractColor(crBackground,0.25f)) + 
		CPWL_Utils::GetBorderAppStream(rcWindow,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);

	CFX_ByteString csAP_D_OFF = csAP_D_ON;

	csAP_N_ON += CPWL_Utils::GetCheckBoxAppStream(rcClient,nStyle,crText);
	csAP_D_ON += CPWL_Utils::GetCheckBoxAppStream(rcClient,nStyle,crText);

	WriteAppearance("N", GetRotatedRect(), GetMatrix(), csAP_N_ON, pControl->GetCheckedAPState());
	WriteAppearance("N", GetRotatedRect(), GetMatrix(), csAP_N_OFF, "Off");

	WriteAppearance("D", GetRotatedRect(), GetMatrix(), csAP_D_ON, pControl->GetCheckedAPState());
	WriteAppearance("D", GetRotatedRect(), GetMatrix(), csAP_D_OFF, "Off");

	CFX_ByteString csAS = GetAppState();
	if (csAS.IsEmpty())
		SetAppState("Off");
}

void CPDFSDK_Widget::ResetAppearance_RadioButton()
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);
	


	CPWL_Color crBackground, crBorder, crText;
	
	int iColorType;
	FX_FLOAT fc[4];

	pControl->GetOriginalBackgroundColor(iColorType, fc);
	if (iColorType > 0)
		crBackground = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	pControl->GetOriginalBorderColor(iColorType, fc);
	if (iColorType > 0)
		crBorder = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
	FX_INT32 nBorderStyle = 0;
	CPWL_Dash dsBorder(3,0,0);
	CPWL_Color crLeftTop,crRightBottom;

	switch (GetBorderStyle())
	{
	case BBS_DASH:
		nBorderStyle = PBS_DASH;
		dsBorder = CPWL_Dash(3, 3, 0);
		break;
	case BBS_BEVELED:
		nBorderStyle = PBS_BEVELED;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,1);
		crRightBottom = CPWL_Utils::DevideColor(crBackground,2);
		break;
	case BBS_INSET:
		nBorderStyle = PBS_INSET;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,0.5);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY,0.75);
		break;
	case BBS_UNDERLINE:
		nBorderStyle = PBS_UNDERLINED;
		break;
	default: 
		nBorderStyle = PBS_SOLID;
		break;
	}

	CPDF_Rect rcWindow = GetRotatedRect();
	CPDF_Rect rcClient = CPWL_Utils::DeflateRect(rcWindow, fBorderWidth);

	CPDF_DefaultAppearance da = pControl->GetDefaultAppearance();
	if (da.HasColor())
	{
		da.GetColor(iColorType, fc);
		crText = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
	}

	FX_INT32 nStyle = 0;

	CFX_WideString csWCaption = pControl->GetNormalCaption();
	if (csWCaption.GetLength() > 0)
	{
		switch (csWCaption[0])
		{
		default: //L'l':
			nStyle = PCS_CIRCLE;			
			break;
		case L'8':
			nStyle = PCS_CROSS;
			break;
		case L'u':
			nStyle = PCS_DIAMOND;
			break;
		case L'n':
			nStyle = PCS_SQUARE;
			break;
		case L'H':
			nStyle = PCS_STAR;
			break;
		case L'4':
			nStyle = PCS_CHECK;
			break;
		}
	}
	else
	{
		nStyle = PCS_CIRCLE;
	}

	CFX_ByteString csAP_N_ON;

	CPDF_Rect rcCenter = CPWL_Utils::DeflateRect(CPWL_Utils::GetCenterSquare(rcWindow), 1.0f);
	
	if (nStyle == PCS_CIRCLE)
	{
		if (nBorderStyle == PBS_BEVELED)
		{
			crLeftTop = CPWL_Color(COLORTYPE_GRAY, 1);
			crRightBottom = CPWL_Utils::SubstractColor(crBackground,0.25f);
		}
		else if (nBorderStyle == PBS_INSET)
		{
			crLeftTop = CPWL_Color(COLORTYPE_GRAY,0.5f);
			crRightBottom = CPWL_Color(COLORTYPE_GRAY,0.75f);
		}

		csAP_N_ON = CPWL_Utils::GetCircleFillAppStream(rcCenter,crBackground) + 
			CPWL_Utils::GetCircleBorderAppStream(rcCenter,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);
	}
	else
	{
		csAP_N_ON = CPWL_Utils::GetRectFillAppStream(rcWindow,crBackground) + 
			CPWL_Utils::GetBorderAppStream(rcWindow,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);
	}

	CFX_ByteString csAP_N_OFF = csAP_N_ON;

	switch (nBorderStyle)
	{
	case PBS_BEVELED:
		{
			CPWL_Color crTemp = crLeftTop;
			crLeftTop = crRightBottom;
			crRightBottom = crTemp;
		}
		break;
	case PBS_INSET:
		crLeftTop = CPWL_Color(COLORTYPE_GRAY,0);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY,1);
		break;
	}

	CFX_ByteString csAP_D_ON;

	if (nStyle == PCS_CIRCLE)
	{
		CPWL_Color crBK = CPWL_Utils::SubstractColor(crBackground,0.25f);
		if (nBorderStyle == PBS_BEVELED)
		{
			crLeftTop = CPWL_Utils::SubstractColor(crBackground,0.25f);
			crRightBottom = CPWL_Color(COLORTYPE_GRAY, 1);
			crBK = crBackground;
		}
		else if (nBorderStyle == PBS_INSET)
		{
			crLeftTop = CPWL_Color(COLORTYPE_GRAY,0);
			crRightBottom = CPWL_Color(COLORTYPE_GRAY,1);
		}

		csAP_D_ON = CPWL_Utils::GetCircleFillAppStream(rcCenter,crBK)
			+ CPWL_Utils::GetCircleBorderAppStream(rcCenter,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);
	}
	else
	{
		csAP_D_ON = CPWL_Utils::GetRectFillAppStream(rcWindow,CPWL_Utils::SubstractColor(crBackground,0.25f)) + 
			CPWL_Utils::GetBorderAppStream(rcWindow,fBorderWidth,crBorder,crLeftTop,crRightBottom,nBorderStyle,dsBorder);		
	}

	CFX_ByteString csAP_D_OFF = csAP_D_ON;

	csAP_N_ON += CPWL_Utils::GetRadioButtonAppStream(rcClient,nStyle,crText);
	csAP_D_ON += CPWL_Utils::GetRadioButtonAppStream(rcClient,nStyle,crText);

	WriteAppearance("N", GetRotatedRect(), GetMatrix(), csAP_N_ON, pControl->GetCheckedAPState());
	WriteAppearance("N", GetRotatedRect(), GetMatrix(), csAP_N_OFF, "Off");

	WriteAppearance("D", GetRotatedRect(), GetMatrix(), csAP_D_ON, pControl->GetCheckedAPState());
	WriteAppearance("D", GetRotatedRect(), GetMatrix(), csAP_D_OFF, "Off");

	CFX_ByteString csAS = GetAppState();
	if (csAS.IsEmpty())
		SetAppState("Off");
}

void CPDFSDK_Widget::ResetAppearance_ComboBox(FX_LPCWSTR sValue)
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);
	CPDF_FormField* pField = pControl->GetField();
	ASSERT(pField != NULL);

	CFX_ByteTextBuf sBody, sLines;

	CPDF_Rect rcClient = GetClientRect();
	CPDF_Rect rcButton = rcClient;
	rcButton.left = rcButton.right - 13;
	rcButton.Normalize();

	if (IFX_Edit * pEdit = IFX_Edit::NewEdit())
	{
		pEdit->EnableRefresh(FALSE);

		ASSERT(this->m_pInterForm != NULL);
		CPDFSDK_Document* pDoc = m_pInterForm->GetDocument();
		ASSERT(pDoc != NULL);
		CPDFDoc_Environment* pEnv = pDoc->GetEnv();
		CBA_FontMap FontMap(this,pEnv->GetSysHandler());
		FontMap.Initial();
		pEdit->SetFontMap(&FontMap);

		CPDF_Rect rcEdit = rcClient;
		rcEdit.right = rcButton.left;
		rcEdit.Normalize();
		
		pEdit->SetPlateRect(rcEdit);
		pEdit->SetAlignmentV(1);

		FX_FLOAT fFontSize = this->GetFontSize();
		if (IsFloatZero(fFontSize))
			pEdit->SetAutoFontSize(TRUE);
		else
			pEdit->SetFontSize(fFontSize);
		
		pEdit->Initialize();
		
		if (sValue)
			pEdit->SetText(sValue);
		else
		{
			FX_INT32 nCurSel = pField->GetSelectedIndex(0);

			if (nCurSel < 0)
				pEdit->SetText(pField->GetValue().c_str());
			else
				pEdit->SetText(pField->GetOptionLabel(nCurSel).c_str());
		}

		CPDF_Rect rcContent = pEdit->GetContentRect();

		CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(pEdit,CPDF_Point(0.0f,0.0f));
		if (sEdit.GetLength() > 0)
		{
			sBody << "/Tx BMC\n" << "q\n";
			if (rcContent.Width() > rcEdit.Width() ||
				rcContent.Height() > rcEdit.Height())
			{
				sBody << rcEdit.left << " " << rcEdit.bottom << " " 
					<< rcEdit.Width() << " " << rcEdit.Height() << " re\nW\nn\n";
			}

			CPWL_Color crText = GetTextPWLColor();	
			sBody << "BT\n" << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n" << "Q\nEMC\n";
		}

		IFX_Edit::DelEdit(pEdit);
	}

	sBody << CPWL_Utils::GetDropButtonAppStream(rcButton);

	CFX_ByteString sAP = GetBackgroundAppStream() + GetBorderAppStream() + sLines.GetByteString() + sBody.GetByteString();

	WriteAppearance("N", GetRotatedRect(), GetMatrix(), sAP);
}

void CPDFSDK_Widget::ResetAppearance_ListBox()
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);
	CPDF_FormField* pField = pControl->GetField();
	ASSERT(pField != NULL);

	CPDF_Rect rcClient = GetClientRect();

	CFX_ByteTextBuf sBody, sLines;

	if (IFX_Edit * pEdit = IFX_Edit::NewEdit())
	{
		pEdit->EnableRefresh(FALSE);

//		ASSERT(this->m_pBaseForm != NULL);
		ASSERT(this->m_pInterForm != NULL);
		CPDFSDK_Document* pDoc = m_pInterForm->GetDocument();
		ASSERT(pDoc != NULL);
		CPDFDoc_Environment* pEnv = pDoc->GetEnv();

		CBA_FontMap FontMap(this,pEnv->GetSysHandler());
		FontMap.Initial();
		pEdit->SetFontMap(&FontMap);

		pEdit->SetPlateRect(CPDF_Rect(rcClient.left,0.0f,rcClient.right,0.0f));	
		
		FX_FLOAT fFontSize = GetFontSize();

		if (IsFloatZero(fFontSize))
			pEdit->SetFontSize(12.0f);
		else
			pEdit->SetFontSize(fFontSize);
		
		pEdit->Initialize();

		CFX_ByteTextBuf sList;
		FX_FLOAT fy = rcClient.top;

		FX_INT32 nTop = pField->GetTopVisibleIndex();
		FX_INT32 nCount = pField->CountOptions();
		FX_INT32 nSelCount = pField->CountSelectedItems();

		for (FX_INT32 i=nTop; i<nCount; i++)
		{
			FX_BOOL bSelected = FALSE;				
			for (FX_INT32 j=0; j<nSelCount; j++)
			{
				if (pField->GetSelectedIndex(j) == i)
				{
					bSelected = TRUE;
					break;
				}
			}

			pEdit->SetText(pField->GetOptionLabel(i).c_str());

			CPDF_Rect rcContent = pEdit->GetContentRect();
			FX_FLOAT fItemHeight = rcContent.Height();

			if (bSelected)
			{
				CPDF_Rect rcItem = CPDF_Rect(rcClient.left,fy-fItemHeight,rcClient.right,fy);
				sList << "q\n" << CPWL_Utils::GetColorAppStream(CPWL_Color(COLORTYPE_RGB,0,51.0f/255.0f,113.0f/255.0f),TRUE)
					<< rcItem.left << " " << rcItem.bottom << " " << rcItem.Width() << " " << rcItem.Height() << " re f\n" << "Q\n";

				sList << "BT\n" << CPWL_Utils::GetColorAppStream(CPWL_Color(COLORTYPE_GRAY,1),TRUE) << 
					CPWL_Utils::GetEditAppStream(pEdit,CPDF_Point(0.0f,fy)) << "ET\n";
			}
			else
			{
				CPWL_Color crText = GetTextPWLColor();
				sList << "BT\n" << CPWL_Utils::GetColorAppStream(crText,TRUE) << 
				CPWL_Utils::GetEditAppStream(pEdit,CPDF_Point(0.0f,fy)) << "ET\n";
			}

			fy -= fItemHeight;
		}
					
		if (sList.GetSize() > 0)
		{
			sBody << "/Tx BMC\n" << "q\n" << rcClient.left << " " << rcClient.bottom << " " 
					<< rcClient.Width() << " " << rcClient.Height() << " re\nW\nn\n";
			sBody << sList << "Q\nEMC\n";
		}

		IFX_Edit::DelEdit(pEdit);
	}

	CFX_ByteString sAP = GetBackgroundAppStream() + GetBorderAppStream() + sLines.GetByteString() + sBody.GetByteString();

	WriteAppearance("N", GetRotatedRect(), GetMatrix(), sAP);
}

void CPDFSDK_Widget::ResetAppearance_TextField(FX_LPCWSTR sValue)
{
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);
	CPDF_FormField* pField = pControl->GetField();
	ASSERT(pField != NULL);

	CFX_ByteTextBuf sBody, sLines;
	
	if (IFX_Edit * pEdit = IFX_Edit::NewEdit())
	{
		pEdit->EnableRefresh(FALSE);

//		ASSERT(this->m_pBaseForm != NULL);
		ASSERT(this->m_pInterForm != NULL);
		CPDFSDK_Document* pDoc = m_pInterForm->GetDocument();
		ASSERT(pDoc != NULL);
		CPDFDoc_Environment* pEnv = pDoc->GetEnv();

		CBA_FontMap FontMap(this,pEnv->GetSysHandler());//, ISystemHandle::GetSystemHandler(m_pBaseForm->GetEnv()));
		FontMap.Initial();
		pEdit->SetFontMap(&FontMap);

		CPDF_Rect rcClient = GetClientRect();
		pEdit->SetPlateRect(rcClient);
		pEdit->SetAlignmentH(pControl->GetControlAlignment());
		
		FX_DWORD dwFieldFlags = pField->GetFieldFlags();
		FX_BOOL bMultiLine = (dwFieldFlags >> 12) & 1;

		if (bMultiLine)
		{
			pEdit->SetMultiLine(TRUE);
			pEdit->SetAutoReturn(TRUE);
		}
		else
		{
			pEdit->SetAlignmentV(1);
		}

		FX_WORD subWord = 0;
		if ((dwFieldFlags >> 13) & 1)
		{
			subWord = '*';
			pEdit->SetPasswordChar(subWord);
		}

		int nMaxLen = pField->GetMaxLen();
		FX_BOOL bCharArray = (dwFieldFlags >> 24) & 1;
		FX_FLOAT fFontSize = GetFontSize();	

		if (nMaxLen > 0)
		{
			if (bCharArray)
			{
				pEdit->SetCharArray(nMaxLen);

				if (IsFloatZero(fFontSize))
				{
					fFontSize = CPWL_Edit::GetCharArrayAutoFontSize(FontMap.GetPDFFont(0),rcClient,nMaxLen);
				}
			}
			else
			{
				if (sValue)
					nMaxLen = wcslen((const wchar_t*)sValue); 
				pEdit->SetLimitChar(nMaxLen);
			}
		}

		if (IsFloatZero(fFontSize))
			pEdit->SetAutoFontSize(TRUE);
		else
			pEdit->SetFontSize(fFontSize);

		pEdit->Initialize();
		
		if (sValue)
			pEdit->SetText(sValue);
		else
			pEdit->SetText(pField->GetValue().c_str());

		CPDF_Rect rcContent = pEdit->GetContentRect();

		CFX_ByteString sEdit = CPWL_Utils::GetEditAppStream(pEdit,CPDF_Point(0.0f,0.0f),
																	NULL,!bCharArray,subWord);

		if (sEdit.GetLength() > 0)
		{
			sBody << "/Tx BMC\n" << "q\n";
			if (rcContent.Width() > rcClient.Width() ||
				rcContent.Height() > rcClient.Height())
			{
				sBody << rcClient.left << " " << rcClient.bottom << " " 
					<< rcClient.Width() << " " << rcClient.Height() << " re\nW\nn\n";
			}
			CPWL_Color crText = GetTextPWLColor();	
			sBody << "BT\n" << CPWL_Utils::GetColorAppStream(crText) << sEdit << "ET\n" << "Q\nEMC\n";
		}

		if (bCharArray)
		{
			switch (GetBorderStyle())
			{
			case BBS_SOLID:
				{
					CFX_ByteString sColor = CPWL_Utils::GetColorAppStream(GetBorderPWLColor(),FALSE);
					if (sColor.GetLength() > 0)
					{
						sLines << "q\n" << GetBorderWidth() << " w\n" 
							<< CPWL_Utils::GetColorAppStream(GetBorderPWLColor(),FALSE) << " 2 J 0 j\n";					

						for (FX_INT32 i=1;i<nMaxLen;i++)
						{
							sLines << rcClient.left + ((rcClient.right - rcClient.left)/nMaxLen)*i << " "
								<< rcClient.bottom << " m\n"
								<< rcClient.left + ((rcClient.right - rcClient.left)/nMaxLen)*i << " "
								<< rcClient.top << " l S\n";						
						}

						sLines << "Q\n";		
					}
				}
				break;
			case BBS_DASH:
				{
					CFX_ByteString sColor = CPWL_Utils::GetColorAppStream(GetBorderPWLColor(),FALSE);
					if (sColor.GetLength() > 0)
					{
						CPWL_Dash dsBorder = CPWL_Dash(3, 3, 0);

						sLines << "q\n" << GetBorderWidth() << " w\n" 
							<< CPWL_Utils::GetColorAppStream(GetBorderPWLColor(),FALSE)
							<< "[" << dsBorder.nDash << " " 
							<< dsBorder.nGap << "] " 
							<< dsBorder.nPhase << " d\n";

						for (FX_INT32 i=1;i<nMaxLen;i++)					
						{
							sLines << rcClient.left + ((rcClient.right - rcClient.left)/nMaxLen)*i << " "
								<< rcClient.bottom << " m\n"
								<< rcClient.left + ((rcClient.right - rcClient.left)/nMaxLen)*i << " "
								<< rcClient.top << " l S\n";	
						}

						sLines << "Q\n";
					}
				}
				break;
			}
		}

		IFX_Edit::DelEdit(pEdit);
	}

	CFX_ByteString sAP = GetBackgroundAppStream() + GetBorderAppStream() + sLines.GetByteString() + sBody.GetByteString();
	WriteAppearance("N", GetRotatedRect(), GetMatrix(), sAP);
}

CPDF_Rect CPDFSDK_Widget::GetClientRect() const
{
	CPDF_Rect rcWindow = GetRotatedRect();
	FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
	switch (GetBorderStyle())
	{
	case BBS_BEVELED:
	case BBS_INSET:
		fBorderWidth *= 2.0f;
		break;
	}

	return CPWL_Utils::DeflateRect(rcWindow, fBorderWidth);
}

CPDF_Rect CPDFSDK_Widget::GetRotatedRect() const
{
	CPDF_Rect rectAnnot = GetRect();
	FX_FLOAT fWidth = rectAnnot.right - rectAnnot.left;
	FX_FLOAT fHeight = rectAnnot.top - rectAnnot.bottom;

	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);

	CPDF_Rect rcPDFWindow;
	switch(abs(pControl->GetRotation() % 360))
	{
		case 0:
		case 180:
		default:
			rcPDFWindow = CPDF_Rect(0, 0, fWidth, fHeight);	
			break;
		case 90:
		case 270:
			rcPDFWindow = CPDF_Rect(0, 0, fHeight, fWidth);
			break;
	}

	return rcPDFWindow;
}

CFX_ByteString CPDFSDK_Widget::GetBackgroundAppStream() const
{
	CPWL_Color crBackground = GetFillPWLColor();
	if (crBackground.nColorType != COLORTYPE_TRANSPARENT)
		return CPWL_Utils::GetRectFillAppStream(GetRotatedRect(), crBackground);
	else
		return "";
}

CFX_ByteString CPDFSDK_Widget::GetBorderAppStream() const
{
	CPDF_Rect rcWindow = GetRotatedRect();
	CPWL_Color crBorder = GetBorderPWLColor();
	CPWL_Color crBackground = GetFillPWLColor();
	CPWL_Color crLeftTop, crRightBottom;

	FX_FLOAT fBorderWidth = (FX_FLOAT)GetBorderWidth();
	FX_INT32 nBorderStyle = 0;
	CPWL_Dash dsBorder(3,0,0);

	switch (GetBorderStyle())
	{
	case BBS_DASH:
		nBorderStyle = PBS_DASH;
		dsBorder = CPWL_Dash(3, 3, 0);
		break;
	case BBS_BEVELED:
		nBorderStyle = PBS_BEVELED;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY, 1);
		crRightBottom = CPWL_Utils::DevideColor(crBackground, 2);
		break;
	case BBS_INSET:
		nBorderStyle = PBS_INSET;
		fBorderWidth *= 2;
		crLeftTop = CPWL_Color(COLORTYPE_GRAY, 0.5);
		crRightBottom = CPWL_Color(COLORTYPE_GRAY, 0.75);
		break;
	case BBS_UNDERLINE:
		nBorderStyle = PBS_UNDERLINED;
		break;
	default: 
		nBorderStyle = PBS_SOLID;
		break;
	}

	return CPWL_Utils::GetBorderAppStream(rcWindow, fBorderWidth, crBorder, crLeftTop, 
		crRightBottom, nBorderStyle, dsBorder);
}

CPDF_Matrix CPDFSDK_Widget::GetMatrix() const
{
	CPDF_Matrix mt;
	CPDF_FormControl* pControl = GetFormControl();
	ASSERT(pControl != NULL);

	CPDF_Rect rcAnnot = GetRect();
	FX_FLOAT fWidth = rcAnnot.right - rcAnnot.left;
	FX_FLOAT fHeight = rcAnnot.top - rcAnnot.bottom;
	


	switch (abs(pControl->GetRotation() % 360))
	{
		case 0:
		default:
			mt = CPDF_Matrix(1, 0, 0, 1, 0, 0);
			break;
		case 90:
			mt = CPDF_Matrix(0, 1, -1, 0, fWidth, 0);
			break;
		case 180:
			mt = CPDF_Matrix(-1, 0, 0, -1, fWidth, fHeight);
			break;
		case 270:
			mt = CPDF_Matrix(0, -1, 1, 0, 0, fHeight);
			break;
	}

	return mt;
}

CPWL_Color CPDFSDK_Widget::GetTextPWLColor() const
{
	CPWL_Color crText = CPWL_Color(COLORTYPE_GRAY, 0);

	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);

	CPDF_DefaultAppearance da = pFormCtrl->GetDefaultAppearance();
	if (da.HasColor())
	{
		FX_INT32 iColorType;
		FX_FLOAT fc[4];
		da.GetColor(iColorType, fc);
		crText = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);
	}

	return crText;
}

CPWL_Color CPDFSDK_Widget::GetBorderPWLColor() const
{
	CPWL_Color crBorder;

	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);

	FX_INT32 iColorType;
	FX_FLOAT fc[4];
	pFormCtrl->GetOriginalBorderColor(iColorType, fc);
	if (iColorType > 0)
		crBorder = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	return crBorder;
}

CPWL_Color CPDFSDK_Widget::GetFillPWLColor() const
{
	CPWL_Color crFill;

	CPDF_FormControl* pFormCtrl = GetFormControl();
	ASSERT(pFormCtrl != NULL);

	FX_INT32 iColorType;
	FX_FLOAT fc[4];
	pFormCtrl->GetOriginalBackgroundColor(iColorType, fc);
	if (iColorType > 0)
		crFill = CPWL_Color(iColorType, fc[0], fc[1], fc[2], fc[3]);

	return crFill;
}

void CPDFSDK_Widget::AddImageToAppearance(const CFX_ByteString& sAPType, CPDF_Stream* pImage)
{
	ASSERT(pImage != NULL);

	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	CPDF_Document* pDoc = m_pPageView->GetPDFDocument();//pDocument->GetDocument();
	ASSERT(pDoc != NULL);

	CPDF_Dictionary* pAPDict = m_pAnnot->m_pAnnotDict->GetDict("AP");
	ASSERT(pAPDict != NULL);

	CPDF_Stream* pStream = pAPDict->GetStream(sAPType);
	ASSERT(pStream != NULL);

	CPDF_Dictionary* pStreamDict = pStream->GetDict();
	ASSERT(pStreamDict != NULL);

	CFX_ByteString sImageAlias = "IMG";

	if (CPDF_Dictionary* pImageDict = pImage->GetDict())
	{
		sImageAlias = pImageDict->GetString("Name");
		if (sImageAlias.IsEmpty())
			sImageAlias = "IMG";
	}	

	CPDF_Dictionary* pStreamResList = pStreamDict->GetDict("Resources");
	if (!pStreamResList)
	{
		pStreamResList = FX_NEW CPDF_Dictionary();
		pStreamDict->SetAt("Resources", pStreamResList);
	}

	if (pStreamResList) 
	{
		CPDF_Dictionary* pXObject = FX_NEW CPDF_Dictionary;			
		pXObject->SetAtReference(sImageAlias, pDoc, pImage);
		pStreamResList->SetAt("XObject", pXObject);
	}
}

void CPDFSDK_Widget::RemoveAppearance(const CFX_ByteString& sAPType)
{
	ASSERT(m_pAnnot != NULL);
	ASSERT(m_pAnnot->m_pAnnotDict != NULL);

	if (CPDF_Dictionary* pAPDict = m_pAnnot->m_pAnnotDict->GetDict("AP"))
	{
		pAPDict->RemoveAt(sAPType);
	}
}

FX_BOOL CPDFSDK_Widget::OnAAction(CPDF_AAction::AActionType type, PDFSDK_FieldAction& data, CPDFSDK_PageView* pPageView)
{
	CPDF_Action action = GetAAction(type);

	if (action && action.GetType() != CPDF_Action::Unknown)
	{
 		CPDFSDK_Document* pDocument = pPageView->GetSDKDocument();
 		ASSERT(pDocument != NULL);
 
 		CPDFDoc_Environment* pEnv = pDocument->GetEnv();
 		ASSERT(pEnv != NULL);

		CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();/*(CPDFSDK_ActionHandler*)pApp->GetActionHandler();*/
 		ASSERT(pActionHandler != NULL);
 
 		return pActionHandler->DoAction_Field(action, type, pDocument, GetFormField(), data);
	}

	return FALSE;
}

CPDF_Action	CPDFSDK_Widget::GetAAction(CPDF_AAction::AActionType eAAT)
{
	switch (eAAT)
	{
	case CPDF_AAction::CursorEnter:
	case CPDF_AAction::CursorExit:
	case CPDF_AAction::ButtonDown:
	case CPDF_AAction::ButtonUp:
	case CPDF_AAction::GetFocus:
	case CPDF_AAction::LoseFocus:
	case CPDF_AAction::PageOpen:
	case CPDF_AAction::PageClose:
	case CPDF_AAction::PageVisible:
	case CPDF_AAction::PageInvisible:
		return CPDFSDK_Annot::GetAAction(eAAT);
	case CPDF_AAction::KeyStroke:
	case CPDF_AAction::Format:
	case CPDF_AAction::Validate:
	case CPDF_AAction::Calculate:
		{
			CPDF_FormField* pField = this->GetFormField();
			ASSERT(pField != NULL);

			if (CPDF_AAction aa = pField->GetAdditionalAction())
				return aa.GetAction(eAAT);
			else 
				return CPDFSDK_Annot::GetAAction(eAAT);
		}
	default:
		return NULL;
	}

	return NULL;
}


CFX_WideString CPDFSDK_Widget::GetAlternateName() const
{
	CPDF_FormField*	pFormField = GetFormField();
	ASSERT(pFormField != NULL);

	return pFormField->GetAlternateName();
}

FX_INT32	CPDFSDK_Widget::GetAppearanceAge() const
{
	return m_nAppAge;
}

FX_INT32 CPDFSDK_Widget::GetValueAge() const
{
	return m_nValueAge;
}


FX_BOOL	CPDFSDK_Widget::HitTest(FX_FLOAT pageX, FX_FLOAT pageY)
{
	CPDF_Annot* pAnnot = GetPDFAnnot();
	CFX_FloatRect annotRect;
	pAnnot->GetRect(annotRect);
	if(annotRect.Contains(pageX, pageY))
	{
		if (!IsVisible()) return FALSE;
		
		int nFieldFlags = GetFieldFlags();
		if ((nFieldFlags & FIELDFLAG_READONLY) == FIELDFLAG_READONLY) 
			return FALSE;
		
		return TRUE;
	}
	return FALSE;
}

CPDFSDK_InterForm::CPDFSDK_InterForm(CPDFSDK_Document* pDocument)
	:m_pDocument(pDocument),
	m_pInterForm(NULL),
	m_bCalculate(TRUE),
	m_bBusy(FALSE)
{
	ASSERT(m_pDocument != NULL);
	m_pInterForm = new CPDF_InterForm(m_pDocument->GetDocument(), FALSE);
	ASSERT(m_pInterForm != NULL);
	m_pInterForm->SetFormNotify(this);

	for(int i=0; i<6; i++)
		m_bNeedHightlight[i] = FALSE;
	m_iHighlightAlpha = 0;
}

CPDFSDK_InterForm::~CPDFSDK_InterForm()
{
	ASSERT(m_pInterForm != NULL);
	delete m_pInterForm;
	m_pInterForm = NULL;

	m_Map.RemoveAll();
}

void CPDFSDK_InterForm::Destroy()
{
	delete this;
}

CPDF_InterForm* CPDFSDK_InterForm::GetInterForm()
{
	return m_pInterForm;
}

CPDFSDK_Document* CPDFSDK_InterForm::GetDocument()
{
	return m_pDocument;
}

FX_BOOL CPDFSDK_InterForm::HighlightWidgets()
{
	return FALSE;
}

CPDFSDK_Widget* CPDFSDK_InterForm::GetSibling(CPDFSDK_Widget* pWidget, FX_BOOL bNext) const
{
	ASSERT(pWidget != NULL);

	CBA_AnnotIterator* pIterator = new CBA_AnnotIterator(pWidget->GetPageView(), "Widget", "");
	ASSERT(pIterator != NULL);

	CPDFSDK_Widget* pRet = NULL;

	if (bNext)
		pRet = (CPDFSDK_Widget*)pIterator->GetNextAnnot(pWidget);
	else
		pRet = (CPDFSDK_Widget*)pIterator->GetPrevAnnot(pWidget);

	pIterator->Release();
	
	return pRet;

}

CPDFSDK_Widget*	CPDFSDK_InterForm::GetWidget(CPDF_FormControl* pControl) const
{
	if(!pControl || !m_pInterForm) return NULL;
	
	CPDFSDK_Widget* pWidget = NULL;
	m_Map.Lookup(pControl, pWidget);

	if (pWidget) return pWidget;

	CPDF_Dictionary* pControlDict = pControl->GetWidget();
	ASSERT(pControlDict != NULL);

	ASSERT(m_pDocument != NULL);
	CPDF_Document* pDocument = m_pDocument->GetDocument();

	CPDFSDK_PageView* pPage = NULL;

	if (CPDF_Dictionary* pPageDict = pControlDict->GetDict("P"))
	{
		int nPageIndex = pDocument->GetPageIndex(pPageDict->GetObjNum());
		if (nPageIndex >= 0)
		{
			pPage = m_pDocument->GetPageView(nPageIndex);
		}
	}

	if (!pPage) 
	{
		int nPageIndex = GetPageIndexByAnnotDict(pDocument, pControlDict);
		if (nPageIndex >= 0)
		{
			pPage = m_pDocument->GetPageView(nPageIndex);
		}
	}

	if (pPage)
		return (CPDFSDK_Widget*)pPage->GetAnnotByDict(pControlDict);

	return NULL;
}

void CPDFSDK_InterForm::GetWidgets(const CFX_WideString& sFieldName, CFX_PtrArray& widgets)
{
	ASSERT(m_pInterForm != NULL);

	for (int i=0,sz=m_pInterForm->CountFields(sFieldName); i<sz; i++)
	{
		CPDF_FormField* pFormField = m_pInterForm->GetField(i, sFieldName);
		ASSERT(pFormField != NULL);

		GetWidgets(pFormField, widgets);	
	}
}

void CPDFSDK_InterForm::GetWidgets(CPDF_FormField* pField, CFX_PtrArray& widgets)
{
	ASSERT(pField != NULL);

	for (int i=0,isz=pField->CountControls(); i<isz; i++)
	{
		CPDF_FormControl* pFormCtrl = pField->GetControl(i);
		ASSERT(pFormCtrl != NULL);

		CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl);

		if (pWidget)
			widgets.Add(pWidget);
	}
}

int CPDFSDK_InterForm::GetPageIndexByAnnotDict(CPDF_Document* pDocument, CPDF_Dictionary* pAnnotDict) const
{
	ASSERT(pDocument != NULL);
	ASSERT(pAnnotDict != NULL);

	for (int i=0,sz=pDocument->GetPageCount(); i<sz; i++)
	{
		if (CPDF_Dictionary* pPageDict = pDocument->GetPage(i))
		{			
			if (CPDF_Array* pAnnots = pPageDict->GetArray("Annots"))
			{
				for (int j=0,jsz=pAnnots->GetCount(); j<jsz; j++)
				{
					CPDF_Object* pDict = pAnnots->GetElementValue(j);
					if (pAnnotDict == pDict)
					{
						return i;
					}
				}
			}
		}
	}

	return -1;
}

void CPDFSDK_InterForm::AddMap(CPDF_FormControl* pControl, CPDFSDK_Widget* pWidget)
{
	m_Map.SetAt(pControl, pWidget);
}

void CPDFSDK_InterForm::RemoveMap(CPDF_FormControl* pControl)
{
	m_Map.RemoveKey(pControl);
}

void CPDFSDK_InterForm::EnableCalculate(FX_BOOL bEnabled)
{
	m_bCalculate = bEnabled;
}

FX_BOOL CPDFSDK_InterForm::IsCalculateEnabled() const
{
	return m_bCalculate;
}

#ifdef _WIN32
CPDF_Stream* CPDFSDK_InterForm::LoadImageFromFile(const CFX_WideString& sFile)
{
	ASSERT(m_pDocument != NULL);
	CPDF_Document* pDocument = m_pDocument->GetDocument();
	ASSERT(pDocument != NULL);

	CPDF_Stream* pRetStream = NULL;

	if (CFX_DIBitmap* pBmp = CFX_WindowsDIB::LoadFromFile(sFile))
	{
		int nWidth = pBmp->GetWidth();
		int nHeight = pBmp->GetHeight();

		CPDF_Image Image(pDocument);
		Image.SetImage(pBmp, FALSE);
		CPDF_Stream* pImageStream = Image.GetStream();
		if (pImageStream)
		{
			if (pImageStream->GetObjNum() == 0)
				pDocument->AddIndirectObject(pImageStream);

			CPDF_Dictionary* pStreamDict = new CPDF_Dictionary();
			pStreamDict->SetAtName("Subtype", "Form");
			pStreamDict->SetAtName("Name", "IMG");
			CPDF_Array* pMatrix = new CPDF_Array();
			pStreamDict->SetAt("Matrix", pMatrix);
			pMatrix->AddInteger(1);
			pMatrix->AddInteger(0);
			pMatrix->AddInteger(0);
			pMatrix->AddInteger(1);
			pMatrix->AddInteger(-nWidth / 2);
			pMatrix->AddInteger(-nHeight / 2);
			CPDF_Dictionary* pResource = new CPDF_Dictionary();
			pStreamDict->SetAt("Resources", pResource);
			CPDF_Dictionary* pXObject = new CPDF_Dictionary();
			pResource->SetAt("XObject", pXObject);
			pXObject->SetAtReference("Img", pDocument, pImageStream);
			CPDF_Array* pProcSet = new CPDF_Array();
			pResource->SetAt("ProcSet", pProcSet);
			pProcSet->AddName("PDF");
			pProcSet->AddName("ImageC");
			pStreamDict->SetAtName("Type", "XObject");
			CPDF_Array* pBBox = new CPDF_Array();
			pStreamDict->SetAt("BBox", pBBox);
			pBBox->AddInteger(0);
			pBBox->AddInteger(0);
			pBBox->AddInteger(nWidth);
			pBBox->AddInteger(nHeight);
			pStreamDict->SetAtInteger("FormType", 1);

			pRetStream = new CPDF_Stream(NULL, 0, NULL);
			CFX_ByteString csStream;
			csStream.Format("q\n%d 0 0 %d 0 0 cm\n/Img Do\nQ", nWidth, nHeight);
			pRetStream->InitStream((FX_BYTE*)csStream.c_str(), csStream.GetLength(), pStreamDict);
			pDocument->AddIndirectObject(pRetStream);
		}

		delete pBmp;
	}

	return pRetStream;
}
#endif

void CPDFSDK_InterForm::OnCalculate(CPDF_FormField* pFormField)
{
	ASSERT(m_pDocument != NULL);
	CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
	ASSERT(pEnv);
	if(!pEnv->IsJSInitiated())
		return;

	if (m_bBusy) return;

	m_bBusy = TRUE;

	if (this->IsCalculateEnabled())
	{
		IFXJS_Runtime* pRuntime = m_pDocument->GetJsRuntime();
		ASSERT(pRuntime != NULL);

		pRuntime->SetReaderDocument(m_pDocument);

		int nSize = m_pInterForm->CountFieldsInCalculationOrder();
		for (int i=0; i<nSize; i++)
		{
			if(CPDF_FormField* pField = m_pInterForm->GetFieldInCalculationOrder(i))
			{
//			ASSERT(pField != NULL);
				int nType = pField->GetFieldType();
				if (nType == FIELDTYPE_COMBOBOX || nType == FIELDTYPE_TEXTFIELD)
				{
					CPDF_AAction aAction = pField->GetAdditionalAction();
					if (aAction && aAction.ActionExist(CPDF_AAction::Calculate))
					{
						CPDF_Action action = aAction.GetAction(CPDF_AAction::Calculate);
						if (action)
						{
							CFX_WideString csJS = action.GetJavaScript();
							if (!csJS.IsEmpty())
							{
								IFXJS_Context* pContext = pRuntime->NewContext();
								ASSERT(pContext != NULL);
								
								CFX_WideString sOldValue = pField->GetValue();
								CFX_WideString sValue = sOldValue;
								FX_BOOL bRC = TRUE;
								pContext->OnField_Calculate(pFormField, pField, sValue, bRC);
								
								CFX_WideString sInfo;
								FX_BOOL bRet = pContext->RunScript(csJS, sInfo);
								pRuntime->ReleaseContext(pContext);
								
								if (bRet)
								{
									if (bRC)
									{
										if (sValue.Compare(sOldValue) != 0)
											pField->SetValue(sValue, TRUE);
									}
								}
							}
						}
					}
				}
			}
		}

		
	}

	m_bBusy = FALSE;
}

CFX_WideString CPDFSDK_InterForm::OnFormat(CPDF_FormField* pFormField, int nCommitKey, FX_BOOL& bFormated)
{
	ASSERT(m_pDocument != NULL);
	ASSERT(pFormField != NULL);

	CFX_WideString sValue = pFormField->GetValue();
	CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
	ASSERT(pEnv);
	if(!pEnv->IsJSInitiated())
	{
		bFormated = FALSE;
		return sValue;
	} 

	IFXJS_Runtime* pRuntime = m_pDocument->GetJsRuntime();
	ASSERT(pRuntime != NULL);
	
	pRuntime->SetReaderDocument(m_pDocument);

	if (pFormField->GetFieldType() == FIELDTYPE_COMBOBOX)
	{
		if (pFormField->CountSelectedItems() > 0)
		{
			int index = pFormField->GetSelectedIndex(0);
			if (index >= 0)
				sValue = pFormField->GetOptionLabel(index);
		}
	}

	bFormated = FALSE;

	CPDF_AAction aAction = pFormField->GetAdditionalAction();
	if (aAction != NULL && aAction.ActionExist(CPDF_AAction::Format)) 
	{
		CPDF_Action action = aAction.GetAction(CPDF_AAction::Format);
		if (action)
		{			
			CFX_WideString script = action.GetJavaScript();
			if (!script.IsEmpty())
			{
				CFX_WideString Value = sValue;

				IFXJS_Context* pContext = pRuntime->NewContext();
				ASSERT(pContext != NULL);

				pContext->OnField_Format(nCommitKey, pFormField, Value, TRUE);
			
				CFX_WideString sInfo;
 				FX_BOOL bRet = pContext->RunScript(script, sInfo);
				pRuntime->ReleaseContext(pContext);

				if (bRet)
				{
					sValue = Value;
					bFormated = TRUE;
				}
			}
		}
	}

	return sValue;
}

void CPDFSDK_InterForm::ResetFieldAppearance(CPDF_FormField* pFormField, FX_LPCWSTR sValue, FX_BOOL bValueChanged)
{
	ASSERT(pFormField != NULL);

	for (int i=0,sz=pFormField->CountControls(); i<sz; i++)
	{
		CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
		ASSERT(pFormCtrl != NULL);

		ASSERT(m_pInterForm != NULL);
		if(CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl))
			pWidget->ResetAppearance(sValue, bValueChanged);
	}
}

void CPDFSDK_InterForm::UpdateField(CPDF_FormField* pFormField)
{
	ASSERT(pFormField != NULL);

	for (int i=0,sz=pFormField->CountControls(); i<sz; i++)
	{
		CPDF_FormControl* pFormCtrl = pFormField->GetControl(i);
		ASSERT(pFormCtrl != NULL);

		if(CPDFSDK_Widget* pWidget = GetWidget(pFormCtrl))
		{
			CPDFDoc_Environment * pEnv = m_pDocument->GetEnv();
			CFFL_IFormFiller* pIFormFiller = pEnv->GetIFormFiller();
			
			CPDF_Page * pPage = pWidget->GetPDFPage();
			CPDFSDK_PageView * pPageView = m_pDocument->GetPageView(pPage,FALSE);

			FX_RECT rcBBox = pIFormFiller->GetViewBBox(pPageView, pWidget);

			pEnv->FFI_Invalidate(pPage,rcBBox.left, rcBBox.top, rcBBox.right, rcBBox.bottom);
		}
	}
}

void CPDFSDK_InterForm::OnKeyStrokeCommit(CPDF_FormField* pFormField, CFX_WideString& csValue, FX_BOOL& bRC)
{
	ASSERT(pFormField != NULL);

 	CPDF_AAction aAction = pFormField->GetAdditionalAction();
 	if (aAction != NULL && aAction.ActionExist(CPDF_AAction::KeyStroke)) 
 	{
 		CPDF_Action action = aAction.GetAction(CPDF_AAction::KeyStroke);
 		if (action)
 		{			 
 			ASSERT(m_pDocument != NULL);
 			CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
 			ASSERT(pEnv != NULL);

			CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
			ASSERT(pActionHandler != NULL);
	
			PDFSDK_FieldAction fa;
			fa.bModifier = pEnv->FFI_IsCTRLKeyDown(0);
 			fa.bShift = pEnv->FFI_IsSHIFTKeyDown(0);
			fa.sValue = csValue;

   			pActionHandler->DoAction_FieldJavaScript(action, CPDF_AAction::KeyStroke, 
   				m_pDocument, pFormField, fa);
   			bRC = fa.bRC;
 		}
 	}
}

void CPDFSDK_InterForm::OnValidate(CPDF_FormField* pFormField, CFX_WideString& csValue, FX_BOOL& bRC)
{
	ASSERT(pFormField != NULL);

 	CPDF_AAction aAction = pFormField->GetAdditionalAction();
 	if (aAction != NULL && aAction.ActionExist(CPDF_AAction::Validate)) 
 	{
 		CPDF_Action action = aAction.GetAction(CPDF_AAction::Validate);
		if (action)
 		{		
			ASSERT(m_pDocument != NULL);
			CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
			ASSERT(pEnv != NULL);
			
			CPDFSDK_ActionHandler* pActionHandler = pEnv->GetActionHander();
			ASSERT(pActionHandler != NULL);

			PDFSDK_FieldAction fa;
			fa.bModifier = pEnv->FFI_IsCTRLKeyDown(0);
			fa.bShift = pEnv->FFI_IsSHIFTKeyDown(0);
			fa.sValue = csValue;

			pActionHandler->DoAction_FieldJavaScript(action, CPDF_AAction::Validate, m_pDocument, pFormField, fa);
			bRC = fa.bRC;
 	 
		}
 	}
}

/* ----------------------------- action ----------------------------- */

FX_BOOL CPDFSDK_InterForm::DoAction_Hide(const CPDF_Action& action)
{
	ASSERT(action != NULL);

	CPDF_ActionFields af = action.GetWidgets();
	CFX_PtrArray fieldObjects;
	af.GetAllFields(fieldObjects);
	CFX_PtrArray widgetArray;
	CFX_PtrArray fields;
	GetFieldFromObjects(fieldObjects, fields);

	FX_BOOL bHide = action.GetHideStatus();

	FX_BOOL bChanged = FALSE;
	
	for (int i=0, sz=fields.GetSize(); i<sz; i++)
	{
		CPDF_FormField* pField = (CPDF_FormField*)fields[i];
		ASSERT(pField != NULL);

	
		for (int j=0,jsz=pField->CountControls(); j<jsz; j++)
		{
			CPDF_FormControl* pControl = pField->GetControl(j);
			ASSERT(pControl != NULL);

			if (CPDFSDK_Widget* pWidget = GetWidget(pControl))
			{
				int nFlags = pWidget->GetFlags();
				if (bHide)
				{
					nFlags &= (~ANNOTFLAG_INVISIBLE);
					nFlags &= (~ANNOTFLAG_NOVIEW);
					nFlags |= (ANNOTFLAG_HIDDEN);
				}
				else
				{
					nFlags &= (~ANNOTFLAG_INVISIBLE);
					nFlags &= (~ANNOTFLAG_HIDDEN);
					nFlags &= (~ANNOTFLAG_NOVIEW);
				}
				pWidget->SetFlags(nFlags);

 				CPDFSDK_PageView* pPageView = pWidget->GetPageView();
 				ASSERT(pPageView != NULL);
 
 				pPageView->UpdateView(pWidget);

				bChanged = TRUE;
			}
		}
	}

	return bChanged;
}

FX_BOOL CPDFSDK_InterForm::DoAction_SubmitForm(const CPDF_Action& action)
{
	ASSERT(action != NULL);
	ASSERT(m_pInterForm != NULL);

	CFX_WideString sDestination = action.GetFilePath();
	if (sDestination.IsEmpty()) return FALSE;

	CPDF_Dictionary* pActionDict = action;
	if (pActionDict->KeyExist("Fields"))
	{
		CPDF_ActionFields af = action.GetWidgets();
		FX_DWORD dwFlags = action.GetFlags();
		
		CFX_PtrArray fieldObjects;
		af.GetAllFields(fieldObjects);
		CFX_PtrArray fields;
		GetFieldFromObjects(fieldObjects, fields);
		
		if (fields.GetSize() != 0)
		{
			FX_BOOL bIncludeOrExclude = !(dwFlags & 0x01);
			if (m_pInterForm->CheckRequiredFields(&fields, bIncludeOrExclude))
			{
				return FALSE;
			}
			return SubmitFields(sDestination, fields, bIncludeOrExclude, FALSE);
		}
		else
		{
			if ( m_pInterForm->CheckRequiredFields())
			{
				return FALSE;
			}

			return SubmitForm(sDestination, FALSE);
		}
	}
	else
	{
		if ( m_pInterForm->CheckRequiredFields())
		{
			return FALSE;
		}

		return SubmitForm(sDestination, FALSE);
	}
}

FX_BOOL CPDFSDK_InterForm::SubmitFields(const CFX_WideString& csDestination, const CFX_PtrArray& fields,
									FX_BOOL bIncludeOrExclude, FX_BOOL bUrlEncoded)
{
	CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
	ASSERT(pEnv != NULL);

	CFX_ByteTextBuf textBuf;
	ExportFieldsToFDFTextBuf(fields, bIncludeOrExclude, textBuf);

	FX_LPBYTE pBuffer = textBuf.GetBuffer();
	FX_STRSIZE nBufSize = textBuf.GetLength();
	
	if (bUrlEncoded)
	{
		if(!FDFToURLEncodedData(pBuffer, nBufSize))
			return FALSE;
	}

	pEnv->JS_docSubmitForm(pBuffer, nBufSize, csDestination.c_str());
	
	return TRUE;
}

void CPDFSDK_InterForm::DoFDFBuffer(CFX_ByteString sBuffer)
{
	ASSERT(m_pDocument != NULL);

	if (CFDF_Document *pFDFDocument = CFDF_Document::ParseMemory((const unsigned char *)sBuffer.GetBuffer(sBuffer.GetLength()), sBuffer.GetLength()))
	{						
		CPDF_Dictionary* pRootDic = pFDFDocument->GetRoot();
		if(pRootDic)
		{
			CPDF_Dictionary * pFDFDict=pRootDic->GetDict("FDF");
			if(pFDFDict)
			{		
				CPDF_Dictionary * pJSDict = pFDFDict->GetDict("JavaScript");
				if(pJSDict)
				{
					CFX_WideString csJS;
				
					CPDF_Object* pJS = pJSDict->GetElementValue("Before");
					if (pJS != NULL)
					{
						int iType = pJS->GetType();
						if (iType == PDFOBJ_STRING)
							csJS = pJSDict->GetUnicodeText("Before");
						else if (iType == PDFOBJ_STREAM)
							csJS = pJS->GetUnicodeText();
					}
					
				}
			}
		}
		delete pFDFDocument;
	}

	sBuffer.ReleaseBuffer();
}

FX_BOOL CPDFSDK_InterForm::FDFToURLEncodedData(CFX_WideString csFDFFile, CFX_WideString csTxtFile)
{
	return TRUE;
}

FX_BOOL CPDFSDK_InterForm::FDFToURLEncodedData(FX_LPBYTE& pBuf, FX_STRSIZE& nBufSize)
{
 	CFDF_Document* pFDF = CFDF_Document::ParseMemory(pBuf, nBufSize);
 	if (pFDF)
 	{
 		CPDF_Dictionary* pMainDict = pFDF->GetRoot()->GetDict("FDF");
 		if (pMainDict == NULL) return FALSE;
 		
 		// Get fields
 		CPDF_Array* pFields = pMainDict->GetArray("Fields");
 		if (pFields == NULL) return FALSE;
		
		CFX_ByteTextBuf fdfEncodedData;

 		for (FX_DWORD i = 0; i < pFields->GetCount(); i ++) 
 		{
 			CPDF_Dictionary* pField = pFields->GetDict(i);
 			if (pField == NULL) continue;
 			CFX_WideString name;
 			name = pField->GetUnicodeText("T");
 			CFX_ByteString name_b = CFX_ByteString::FromUnicode(name);
 			CFX_ByteString csBValue = pField->GetString("V");
 			CFX_WideString csWValue = PDF_DecodeText(csBValue);
 			CFX_ByteString csValue_b = CFX_ByteString::FromUnicode(csWValue);

			fdfEncodedData = fdfEncodedData<<name_b.GetBuffer(name_b.GetLength());
  			name_b.ReleaseBuffer();
			fdfEncodedData = fdfEncodedData<<"=";
			fdfEncodedData = fdfEncodedData<<csValue_b.GetBuffer(csValue_b.GetLength());
  			csValue_b.ReleaseBuffer();
  			if(i != pFields->GetCount()-1)
  				fdfEncodedData = fdfEncodedData<<"&";
 		}
		
		nBufSize = fdfEncodedData.GetLength();
		pBuf = FX_Alloc(FX_BYTE, nBufSize);
		if(!pBuf)
			return FALSE;
		FXSYS_memcpy(pBuf, fdfEncodedData.GetBuffer(), nBufSize);
 		
 	}
	return TRUE;
}

FX_BOOL CPDFSDK_InterForm::ExportFieldsToFDFTextBuf(const CFX_PtrArray& fields,FX_BOOL bIncludeOrExclude, CFX_ByteTextBuf& textBuf)
{
	ASSERT(m_pDocument != NULL);
	ASSERT(m_pInterForm != NULL);
	
	CFDF_Document* pFDF = m_pInterForm->ExportToFDF(m_pDocument->GetPath(),(CFX_PtrArray&)fields, bIncludeOrExclude);
	if (!pFDF) return FALSE;
	FX_BOOL bRet = pFDF->WriteBuf(textBuf); // = FALSE;//
	delete pFDF;
	
	return bRet;
}

CFX_WideString CPDFSDK_InterForm::GetTemporaryFileName(const CFX_WideString& sFileExt)
{
	CFX_WideString sFileName;
	return L"";
}

FX_BOOL CPDFSDK_InterForm::SubmitForm(const CFX_WideString& sDestination, FX_BOOL bUrlEncoded)
{
 	if (sDestination.IsEmpty()) return FALSE;

	CPDFDoc_Environment* pEnv = m_pDocument->GetEnv();
	ASSERT(pEnv != NULL);

	if(NULL == m_pDocument) return FALSE;
	CFX_WideString wsPDFFilePath = m_pDocument->GetPath();
	
	if(NULL == m_pInterForm) return FALSE;
	CFDF_Document* pFDFDoc = m_pInterForm->ExportToFDF(wsPDFFilePath);
	if (NULL == pFDFDoc) return FALSE;

	CFX_ByteTextBuf FdfBuffer;
	FX_BOOL bRet = pFDFDoc->WriteBuf(FdfBuffer);
	delete pFDFDoc;
	if (!bRet) return FALSE;

	FX_LPBYTE pBuffer = FdfBuffer.GetBuffer();
	FX_STRSIZE nBufSize = FdfBuffer.GetLength();
	
	if (bUrlEncoded)
	{
		if(!FDFToURLEncodedData(pBuffer, nBufSize))
			return FALSE;
	}

	pEnv->JS_docSubmitForm(pBuffer, nBufSize, sDestination.c_str());
	
	if (bUrlEncoded && pBuffer)
	{
		FX_Free(pBuffer);
		pBuffer = NULL;	
	}

	return TRUE;
}

FX_BOOL CPDFSDK_InterForm::ExportFormToFDFTextBuf(CFX_ByteTextBuf& textBuf)
{

	ASSERT(m_pInterForm != NULL);
	ASSERT(m_pDocument != NULL);
	
	CFDF_Document* pFDF = m_pInterForm->ExportToFDF(m_pDocument->GetPath());
	if (!pFDF) return FALSE;
	
	FX_BOOL bRet = pFDF->WriteBuf(textBuf);
	delete pFDF;
	
	return bRet;
}


FX_BOOL CPDFSDK_InterForm::DoAction_ResetForm(const CPDF_Action& action)
{
	ASSERT(action != NULL);

	CPDF_Dictionary* pActionDict = action;

	if (pActionDict->KeyExist("Fields"))
	{
		CPDF_ActionFields af = action.GetWidgets();
		FX_DWORD dwFlags = action.GetFlags();
		
		CFX_PtrArray fieldObjects;
		af.GetAllFields(fieldObjects);
		CFX_PtrArray fields;
		GetFieldFromObjects(fieldObjects, fields);
		
		ASSERT(m_pInterForm != NULL);

		return m_pInterForm->ResetForm(fields, !(dwFlags & 0x01), TRUE);
	}
	else
	{
		ASSERT(m_pInterForm != NULL);
		return m_pInterForm->ResetForm(TRUE);
	}
}

FX_BOOL CPDFSDK_InterForm::DoAction_ImportData(const CPDF_Action& action)
{
	return FALSE;
}

void CPDFSDK_InterForm::GetFieldFromObjects(const CFX_PtrArray& objects, CFX_PtrArray& fields)
{
	ASSERT(m_pInterForm != NULL);

	int iCount = objects.GetSize();
	for (int i = 0; i < iCount; i ++)
	{
		CPDF_Object* pObject = (CPDF_Object*)objects[i];
		if (pObject == NULL) continue;
		
		int iType = pObject->GetType();
		if (iType == PDFOBJ_STRING)
		{
			CFX_WideString csName = pObject->GetUnicodeText();
			CPDF_FormField* pField = m_pInterForm->GetField(0, csName);
			if (pField != NULL)
				fields.Add(pField);
		}
		else if (iType == PDFOBJ_DICTIONARY)
		{
			if (m_pInterForm->IsValidFormField(pObject))
				fields.Add(pObject);
		}
	}
}

/* ----------------------------- CPDF_FormNotify ----------------------------- */

int	CPDFSDK_InterForm::BeforeValueChange(const CPDF_FormField* pField, CFX_WideString& csValue)
{
	ASSERT(pField != NULL);

	CPDF_FormField* pFormField = (CPDF_FormField*)pField;

	int nType = pFormField->GetFieldType();
	if (nType == FIELDTYPE_COMBOBOX || nType == FIELDTYPE_TEXTFIELD)
	{
		FX_BOOL bRC = TRUE;
		OnKeyStrokeCommit(pFormField, csValue, bRC);
		if (bRC) 
		{
			OnValidate(pFormField, csValue, bRC);
			if (bRC)
				return 1;
			else
				return -1;
		}
		else
			return -1;
	}
	else
		return 0;
}

int	CPDFSDK_InterForm::AfterValueChange(const CPDF_FormField* pField)
{
	ASSERT(pField != NULL);

	CPDF_FormField* pFormField = (CPDF_FormField*)pField;
	int nType = pFormField->GetFieldType();

	if (nType == FIELDTYPE_COMBOBOX || nType == FIELDTYPE_TEXTFIELD)
	{
		this->OnCalculate(pFormField);
		FX_BOOL bFormated = FALSE;
		CFX_WideString sValue = this->OnFormat(pFormField, 0, bFormated);
		if (bFormated)
			this->ResetFieldAppearance(pFormField, sValue, TRUE);
		else
			this->ResetFieldAppearance(pFormField, NULL, TRUE);
		this->UpdateField(pFormField);
	}

	return 0;
}

int	CPDFSDK_InterForm::BeforeSelectionChange(const CPDF_FormField* pField, CFX_WideString& csValue)
{
	ASSERT(pField != NULL);

	CPDF_FormField* pFormField = (CPDF_FormField*)pField;

	int nType = pFormField->GetFieldType();
	if (nType == FIELDTYPE_LISTBOX)
	{
		FX_BOOL bRC = TRUE;
		OnKeyStrokeCommit(pFormField, csValue, bRC);
		if (bRC) 
		{
			OnValidate(pFormField, csValue, bRC);
			if (bRC)
				return 1;
			else
				return -1;
		}
		else
			return -1;
	}
	else
		return 0;
}

int	CPDFSDK_InterForm::AfterSelectionChange(const CPDF_FormField* pField)
{
	ASSERT(pField != NULL);

	CPDF_FormField* pFormField = (CPDF_FormField*)pField;
	int nType = pFormField->GetFieldType();

	if (nType == FIELDTYPE_LISTBOX)
	{
		this->OnCalculate(pFormField);
		this->ResetFieldAppearance(pFormField, NULL, TRUE);
		this->UpdateField(pFormField);
	}

	return 0;
}

int	CPDFSDK_InterForm::AfterCheckedStatusChange(const CPDF_FormField* pField, const CFX_ByteArray& statusArray)
{
	ASSERT(pField != NULL);

	CPDF_FormField* pFormField = (CPDF_FormField*)pField;
	int nType = pFormField->GetFieldType();

	if (nType == FIELDTYPE_CHECKBOX || nType == FIELDTYPE_RADIOBUTTON)
	{
		this->OnCalculate(pFormField);
		//this->ResetFieldAppearance(pFormField, NULL);
		this->UpdateField(pFormField);
	}

	return 0;
}

int	CPDFSDK_InterForm::BeforeFormReset(const CPDF_InterForm* pForm)
{
	return 0;
}

int	CPDFSDK_InterForm::AfterFormReset(const CPDF_InterForm* pForm)
{
	this->OnCalculate(NULL);

	return 0;
}

int	CPDFSDK_InterForm::BeforeFormImportData(const CPDF_InterForm* pForm)
{
	return 0;
}

int	CPDFSDK_InterForm::AfterFormImportData(const CPDF_InterForm* pForm)
{
	this->OnCalculate(NULL);

	return 0;
}

FX_BOOL CPDFSDK_InterForm::IsNeedHighLight(int nFieldType)
{
	if(nFieldType <1 || nFieldType > 6)
		return FALSE;
	return m_bNeedHightlight[nFieldType-1];
}

void CPDFSDK_InterForm::RemoveAllHighLight()
{
	memset((void*)m_bNeedHightlight, 0, 6*sizeof(FX_BOOL));
}
void   CPDFSDK_InterForm::SetHighlightColor(FX_COLORREF clr, int nFieldType)
{
	if(nFieldType <0 || nFieldType > 6) return;
	switch(nFieldType)
	{
	case 0:
		{
			for(int i=0; i<6; i++)
			{
				m_aHighlightColor[i] = clr;
				m_bNeedHightlight[i] = TRUE;
			}
			break;
		}
	default:
		{
			m_aHighlightColor[nFieldType-1] = clr;
			m_bNeedHightlight[nFieldType-1] = TRUE;
			break;
		}
	}
	
}

FX_COLORREF CPDFSDK_InterForm::GetHighlightColor(int nFieldType)
{
	if(nFieldType <0 || nFieldType >6) return FXSYS_RGB(255,255,255);
	if(nFieldType == 0)
		return m_aHighlightColor[0];
	else
		return m_aHighlightColor[nFieldType-1];
}

/* ------------------------- CBA_AnnotIterator ------------------------- */

CBA_AnnotIterator::CBA_AnnotIterator(CPDFSDK_PageView* pPageView, const CFX_ByteString& sType, const CFX_ByteString& sSubType)
	:m_pPageView(pPageView),
	m_sType(sType),
	m_sSubType(sSubType),
	m_nTabs(BAI_STRUCTURE)
{
	ASSERT(m_pPageView != NULL);

	CPDF_Page* pPDFPage = m_pPageView->GetPDFPage();
	ASSERT(pPDFPage != NULL);
	ASSERT(pPDFPage->m_pFormDict != NULL);

	CFX_ByteString sTabs = pPDFPage->m_pFormDict->GetString("Tabs");

	if (sTabs == "R")
	{
		m_nTabs = BAI_ROW;
	}
	else if (sTabs == "C")
	{
		m_nTabs = BAI_COLUMN;
	}
	else
	{
		m_nTabs = BAI_STRUCTURE;
	}

	GenerateResults();
}

CBA_AnnotIterator::~CBA_AnnotIterator()
{
	m_Annots.RemoveAll();
}

CPDFSDK_Annot* CBA_AnnotIterator::GetFirstAnnot()
{
	if (m_Annots.GetSize() > 0)
		return m_Annots[0];
	
	return NULL;
}

CPDFSDK_Annot* CBA_AnnotIterator::GetLastAnnot()
{
	if (m_Annots.GetSize() > 0)
		return m_Annots[m_Annots.GetSize() - 1];

	return NULL;
}

CPDFSDK_Annot* CBA_AnnotIterator::GetNextAnnot(CPDFSDK_Annot* pAnnot)
{
	for (int i=0,sz=m_Annots.GetSize(); i<sz; i++)
	{
		if (m_Annots[i] == pAnnot)
		{
			if (i+1 < sz)
				return m_Annots[i+1];
			else
				return m_Annots[0];
		}
	}

	return NULL;
}

CPDFSDK_Annot* CBA_AnnotIterator::GetPrevAnnot(CPDFSDK_Annot* pAnnot)
{
	for (int i=0,sz=m_Annots.GetSize(); i<sz; i++)
	{
		if (m_Annots[i] == pAnnot)
		{
			if (i-1 >= 0)
				return m_Annots[i-1];
			else
				return m_Annots[sz-1];
		}
	}

	return NULL;
}

int CBA_AnnotIterator::CompareByLeft(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2)
{
	ASSERT(p1 != NULL);
	ASSERT(p2 != NULL);

	CPDF_Rect rcAnnot1 = GetAnnotRect(p1);
	CPDF_Rect rcAnnot2 = GetAnnotRect(p2);

	if (rcAnnot1.left < rcAnnot2.left)
		return -1;
	if (rcAnnot1.left > rcAnnot2.left)
		return 1;
	return 0;
}


int CBA_AnnotIterator::CompareByTop(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2)
{
	ASSERT(p1 != NULL);
	ASSERT(p2 != NULL);

	CPDF_Rect rcAnnot1 = GetAnnotRect(p1);
	CPDF_Rect rcAnnot2 = GetAnnotRect(p2);

	if (rcAnnot1.top < rcAnnot2.top)
		return -1;
	if (rcAnnot1.top > rcAnnot2.top)
		return 1;
	return 0;
}

void CBA_AnnotIterator::GenerateResults()
{
	ASSERT(m_pPageView != NULL);

	switch (m_nTabs)
	{
	case BAI_STRUCTURE:
		{
			for (int i=0,sz=m_pPageView->CountAnnots(); i<sz; i++)
			{
				CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
				ASSERT(pAnnot != NULL);

				if (pAnnot->GetType() == m_sType 
					&& pAnnot->GetSubType() == m_sSubType)
					m_Annots.Add(pAnnot);
			}
		}
		break;
	case BAI_ROW:
		{
			CPDFSDK_SortAnnots sa;

			{
				
				for (int i=0,sz=m_pPageView->CountAnnots(); i<sz; i++)
				{
					CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
					ASSERT(pAnnot != NULL);

					if (pAnnot->GetType() == m_sType 
						&& pAnnot->GetSubType() == m_sSubType)
						sa.Add(pAnnot);
				}
			}

			if (sa.GetSize() > 0)
			{
				sa.Sort(CBA_AnnotIterator::CompareByLeft);
			}

			while (sa.GetSize() > 0)
			{
				int nLeftTopIndex = -1;

				{
					FX_FLOAT fTop = 0.0f;

					for (int i=sa.GetSize()-1; i>=0; i--)
					{
						CPDFSDK_Annot* pAnnot = sa.GetAt(i);
						ASSERT(pAnnot != NULL);

						CPDF_Rect rcAnnot = GetAnnotRect(pAnnot);

						if (rcAnnot.top > fTop)
						{
							nLeftTopIndex = i;
							fTop = rcAnnot.top;
						}
					}
				}

				if (nLeftTopIndex >= 0)
				{
					CPDFSDK_Annot* pLeftTopAnnot = sa.GetAt(nLeftTopIndex);
					ASSERT(pLeftTopAnnot != NULL);

					CPDF_Rect rcLeftTop = GetAnnotRect(pLeftTopAnnot);
					
					m_Annots.Add(pLeftTopAnnot);
					sa.RemoveAt(nLeftTopIndex);

					CFX_ArrayTemplate<int> aSelect;

					{
						for (int i=0,sz=sa.GetSize(); i<sz; i++)
						{
							CPDFSDK_Annot* pAnnot = sa.GetAt(i);
							ASSERT(pAnnot != NULL);

							CPDF_Rect rcAnnot = GetAnnotRect(pAnnot);

							FX_FLOAT fCenterY = (rcAnnot.top + rcAnnot.bottom) / 2.0f;

							if (fCenterY > rcLeftTop.bottom && fCenterY < rcLeftTop.top)
								aSelect.Add(i);
						}
					}

					{
						for (int i=0,sz=aSelect.GetSize(); i<sz; i++)
						{
							m_Annots.Add(sa[aSelect[i]]);
						}
					}

					{
						for (int i=aSelect.GetSize()-1; i>=0; i--)
						{
							sa.RemoveAt(aSelect[i]);
						}
					}

					aSelect.RemoveAll();
				}
			}
			sa.RemoveAll();
		}
		break;
	case BAI_COLUMN:
		{
			CPDFSDK_SortAnnots sa;

			{
				for (int i=0,sz=m_pPageView->CountAnnots(); i<sz; i++)
				{
					CPDFSDK_Annot* pAnnot = m_pPageView->GetAnnot(i);
					ASSERT(pAnnot != NULL);

					if (pAnnot->GetType() == m_sType 
						&& pAnnot->GetSubType() == m_sSubType)
						sa.Add(pAnnot);
				}
			}

			if (sa.GetSize() > 0)
			{
				sa.Sort(CBA_AnnotIterator::CompareByTop, FALSE);
			}

			while (sa.GetSize() > 0)
			{
				int nLeftTopIndex = -1;

				{
					FX_FLOAT fLeft = -1.0f;

					for (int i=sa.GetSize()-1; i>=0; i--)
					{
						CPDFSDK_Annot* pAnnot = sa.GetAt(i);
						ASSERT(pAnnot != NULL);

						CPDF_Rect rcAnnot = GetAnnotRect(pAnnot);

						if (fLeft < 0)
						{
							nLeftTopIndex = 0;
							fLeft = rcAnnot.left;
						}
						else if (rcAnnot.left < fLeft)
						{
							nLeftTopIndex = i;
							fLeft = rcAnnot.left;
						}
					}
				}

				if (nLeftTopIndex >= 0)
				{
					CPDFSDK_Annot* pLeftTopAnnot = sa.GetAt(nLeftTopIndex);
					ASSERT(pLeftTopAnnot != NULL);

					CPDF_Rect rcLeftTop = GetAnnotRect(pLeftTopAnnot);
					
					m_Annots.Add(pLeftTopAnnot);
					sa.RemoveAt(nLeftTopIndex);

					CFX_ArrayTemplate<int> aSelect;

					{
						for (int i=0,sz=sa.GetSize(); i<sz; i++)
						{
							CPDFSDK_Annot* pAnnot = sa.GetAt(i);
							ASSERT(pAnnot != NULL);

							CPDF_Rect rcAnnot = GetAnnotRect(pAnnot);

							FX_FLOAT fCenterX = (rcAnnot.left + rcAnnot.right) / 2.0f;

							if (fCenterX > rcLeftTop.left && fCenterX < rcLeftTop.right)
								aSelect.Add(i);
						}
					}

					{
						for (int i=0,sz=aSelect.GetSize(); i<sz; i++)
						{
							m_Annots.Add(sa[aSelect[i]]);
						}
					}

					{
						for (int i=aSelect.GetSize()-1; i>=0; i--)
						{
							sa.RemoveAt(aSelect[i]);
						}
					}

					aSelect.RemoveAll();
				}
			}
			sa.RemoveAll();
		}
		break;
	}
}

CPDF_Rect CBA_AnnotIterator::GetAnnotRect(CPDFSDK_Annot* pAnnot)
{
	ASSERT(pAnnot != NULL);

	CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
	ASSERT(pPDFAnnot != NULL);

	CPDF_Rect rcAnnot;
	pPDFAnnot->GetRect(rcAnnot);

	return rcAnnot;
}

