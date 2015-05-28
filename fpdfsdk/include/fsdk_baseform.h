// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_BASEFORM_H_
#define FPDFSDK_INCLUDE_FSDK_BASEFORM_H_

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

#include "../../core/include/fpdfapi/fpdf_parser.h"
#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fxcrt/fx_basic.h"
#include "../../core/include/fxge/fx_dib.h"
#include "fsdk_baseannot.h"

class CFFL_FormFiller;
class CPDFSDK_Annot;
class CPDFSDK_DateTime;
class CPDFSDK_Document;
class CPDFSDK_InterForm;
class CPDFSDK_PageView;
class CPDF_Action;
class CPDF_FormField;
struct CPWL_Color;

typedef struct _PDFSDK_FieldAction
{
	_PDFSDK_FieldAction()
	{
		bModifier = FALSE;
		bShift = FALSE;
		nCommitKey = 0;
		bKeyDown = FALSE;
		nSelEnd = nSelStart = 0;
		bWillCommit = FALSE;
		bFieldFull = FALSE;
		bRC = TRUE;
	}
	
	FX_BOOL					bModifier;		//in
	FX_BOOL					bShift;			//in
	int						nCommitKey;		//in
	CFX_WideString			sChange;		//in[out]
	CFX_WideString			sChangeEx;		//in
	FX_BOOL					bKeyDown;		//in
	int						nSelEnd;		//in[out]
	int						nSelStart;		//in[out]
	CFX_WideString			sValue;			//in[out]
	FX_BOOL					bWillCommit;	//in
	FX_BOOL					bFieldFull;		//in
	FX_BOOL					bRC;			//in[out]
}PDFSDK_FieldAction;
class CPDFSDK_Widget:public CPDFSDK_Annot
{
public:
	CPDFSDK_Widget(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPageView, CPDFSDK_InterForm* pInterForm);
	virtual ~CPDFSDK_Widget();

	virtual CFX_ByteString			GetSubType() const;
	
	virtual CPDF_Action				GetAAction(CPDF_AAction::AActionType eAAT);

	int								GetFieldType() const;
	//define layout order to 2.
	virtual int						GetLayoutOrder() const {return 2;}
	/*
	FIELDFLAG_READONLY
	FIELDFLAG_REQUIRED
	FIELDFLAG_NOEXPORT
	*/
	
	int								GetFieldFlags() const;
	int								GetRotate() const;

	FX_BOOL							GetFillColor(FX_COLORREF& color) const;
	FX_BOOL							GetBorderColor(FX_COLORREF& color) const;
	FX_BOOL							GetTextColor(FX_COLORREF& color) const;
	FX_FLOAT						GetFontSize() const;

	int								GetSelectedIndex(int nIndex) const;
	CFX_WideString					GetValue() const;
	CFX_WideString					GetDefaultValue() const;
	CFX_WideString					GetOptionLabel(int nIndex) const;
	int								CountOptions() const;
	FX_BOOL							IsOptionSelected(int nIndex) const;
	int								GetTopVisibleIndex() const;
	FX_BOOL							IsChecked() const;
	/*
	BF_ALIGN_LEFT
	BF_ALIGN_MIDDL
	BF_ALIGN_RIGHT
	*/
	int								GetAlignment() const;
	int								GetMaxLen() const;
	CFX_WideString					GetAlternateName() const;

//Set Properties.
	void							SetCheck(FX_BOOL bChecked, FX_BOOL bNotify);
	void							SetValue(const CFX_WideString& sValue, FX_BOOL bNotify);
	void							SetDefaultValue(const CFX_WideString& sValue);
	void							SetOptionSelection(int index, FX_BOOL bSelected, FX_BOOL bNotify);
	void							ClearSelection(FX_BOOL bNotify);
	void							SetTopVisibleIndex(int index);

	void							ResetAppearance(FX_LPCWSTR sValue, FX_BOOL bValueChanged);
	void							ResetFieldAppearance(FX_BOOL bValueChanged);
	void							UpdateField();
	CFX_WideString					OnFormat(int nCommitKey, FX_BOOL& bFormated);
	
//Message.
 	FX_BOOL							OnAAction(CPDF_AAction::AActionType type, PDFSDK_FieldAction& data, 
												CPDFSDK_PageView* pPageView);

	CPDFSDK_InterForm*				GetInterForm() const {return m_pInterForm;}
	CPDF_FormField*					GetFormField() const;
	CPDF_FormControl*				GetFormControl() const;
	static CPDF_FormControl*		GetFormControl(CPDF_InterForm* pInterForm, CPDF_Dictionary* pAnnotDict);

	void							DrawShadow(CFX_RenderDevice* pDevice, CPDFSDK_PageView* pPageView);
	
	void							SetAppModified();
	void							ClearAppModified();
	FX_BOOL							IsAppModified() const;
	
	FX_INT32						GetAppearanceAge() const;
	FX_INT32						GetValueAge() const;
	
private:
	void							ResetAppearance_PushButton();
	void							ResetAppearance_CheckBox();
	void							ResetAppearance_RadioButton();
	void							ResetAppearance_ComboBox(FX_LPCWSTR sValue);
	void							ResetAppearance_ListBox();
	void							ResetAppearance_TextField(FX_LPCWSTR sValue);
	
	CPDF_Rect						GetClientRect() const;
	CPDF_Rect						GetRotatedRect() const;
	
	CFX_ByteString					GetBackgroundAppStream() const;
	CFX_ByteString					GetBorderAppStream() const;
	CPDF_Matrix						GetMatrix() const;
	
	CPWL_Color						GetTextPWLColor() const;
	CPWL_Color						GetBorderPWLColor() const;
	CPWL_Color						GetFillPWLColor() const;
	
	void							AddImageToAppearance(const CFX_ByteString& sAPType, CPDF_Stream* pImage);
	void							RemoveAppearance(const CFX_ByteString& sAPType);
public:
	FX_BOOL							IsWidgetAppearanceValid(CPDF_Annot::AppearanceMode mode);
	void							DrawAppearance(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
		CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions);
public:
	FX_BOOL							HitTest(FX_FLOAT pageX, FX_FLOAT pageY);
private:
	CPDFSDK_InterForm*				m_pInterForm;
	FX_BOOL							m_bAppModified;
	FX_INT32						m_nAppAge;
	FX_INT32						m_nValueAge;
};

#define CPDFSDK_WidgetMap				CFX_MapPtrTemplate<CPDF_FormControl*, CPDFSDK_Widget*>

class CPDFSDK_InterForm : public CPDF_FormNotify
{
public:
	CPDFSDK_InterForm(CPDFSDK_Document* pDocument);
	virtual ~CPDFSDK_InterForm();
	
public:
	virtual void					Destroy();
	virtual CPDF_InterForm*			GetInterForm();
	
	CPDFSDK_Document*				GetDocument();
	FX_BOOL							HighlightWidgets();
	
	CPDFSDK_Widget*					GetSibling(CPDFSDK_Widget* pWidget, FX_BOOL bNext) const;
	CPDFSDK_Widget*					GetWidget(CPDF_FormControl* pControl) const;
	void							GetWidgets(const CFX_WideString& sFieldName, CFX_PtrArray& widgets);
	void							GetWidgets(CPDF_FormField* pField, CFX_PtrArray& widgets);
	
	void							AddMap(CPDF_FormControl* pControl, CPDFSDK_Widget* pWidget);
	void							RemoveMap(CPDF_FormControl* pControl);
	
	void							EnableCalculate(FX_BOOL bEnabled);
	FX_BOOL							IsCalculateEnabled() const;

#ifdef _WIN32
	CPDF_Stream*					LoadImageFromFile(const CFX_WideString& sFile);
#endif

	void							OnKeyStrokeCommit(CPDF_FormField* pFormField, CFX_WideString& csValue, FX_BOOL& bRC);
	void							OnValidate(CPDF_FormField* pFormField, CFX_WideString& csValue, FX_BOOL& bRC);
	void							OnCalculate(CPDF_FormField* pFormField = NULL);
	CFX_WideString					OnFormat(CPDF_FormField* pFormField, int nCommitKey, FX_BOOL& bFormated);
	
	void							ResetFieldAppearance(CPDF_FormField* pFormField, FX_LPCWSTR sValue, FX_BOOL bValueChanged);
	void							UpdateField(CPDF_FormField* pFormField);
	
public:
	FX_BOOL							DoAction_Hide(const CPDF_Action& action);
	FX_BOOL							DoAction_SubmitForm(const CPDF_Action& action);
	FX_BOOL							DoAction_ResetForm(const CPDF_Action& action);
	FX_BOOL							DoAction_ImportData(const CPDF_Action& action);
	
	void							GetFieldFromObjects(const CFX_PtrArray& objects, CFX_PtrArray& fields);
	FX_BOOL							IsValidField(CPDF_Dictionary* pFieldDict);
	FX_BOOL							SubmitFields(const CFX_WideString& csDestination, const CFX_PtrArray& fields, 
		FX_BOOL bIncludeOrExclude, FX_BOOL bUrlEncoded);
	FX_BOOL							SubmitForm(const CFX_WideString& sDestination, FX_BOOL bUrlEncoded);
	FX_BOOL							ExportFormToFDFTextBuf(CFX_ByteTextBuf& textBuf);
	FX_BOOL							ExportFieldsToFDFTextBuf(const CFX_PtrArray& fields,FX_BOOL bIncludeOrExclude, CFX_ByteTextBuf& textBuf);
	CFX_WideString					GetTemporaryFileName(const CFX_WideString& sFileExt);
	
private:
	virtual int						BeforeValueChange(const CPDF_FormField* pField, CFX_WideString& csValue);
	virtual int						AfterValueChange(const CPDF_FormField* pField);
	virtual int						BeforeSelectionChange(const CPDF_FormField* pField, CFX_WideString& csValue);
	virtual int						AfterSelectionChange(const CPDF_FormField* pField);
	virtual int						AfterCheckedStatusChange(const CPDF_FormField* pField, const CFX_ByteArray& statusArray);
	virtual int						BeforeFormReset(const CPDF_InterForm* pForm);
	virtual int						AfterFormReset(const CPDF_InterForm* pForm);
	virtual int						BeforeFormImportData(const CPDF_InterForm* pForm);
	virtual int						AfterFormImportData(const CPDF_InterForm* pForm);
	
private:
	FX_BOOL							FDFToURLEncodedData(CFX_WideString csFDFFile, CFX_WideString csTxtFile);
	FX_BOOL							FDFToURLEncodedData(FX_LPBYTE& pBuf, FX_STRSIZE& nBufSize);
	int								GetPageIndexByAnnotDict(CPDF_Document* pDocument, CPDF_Dictionary* pAnnotDict) const;
	void							DoFDFBuffer(CFX_ByteString sBuffer);
	
private:
	CPDFSDK_Document*				m_pDocument;
	CPDF_InterForm*					m_pInterForm;
	CPDFSDK_WidgetMap				m_Map;
	FX_BOOL							m_bCalculate;
	FX_BOOL							m_bBusy;

public:
	FX_BOOL IsNeedHighLight(int nFieldType);
	void    RemoveAllHighLight();
	void    SetHighlightAlpha(FX_BYTE alpha) {m_iHighlightAlpha = alpha;}
	FX_BYTE GetHighlightAlpha() {return m_iHighlightAlpha;}
	void    SetHighlightColor(FX_COLORREF clr, int nFieldType);
	FX_COLORREF GetHighlightColor(int nFieldType);
private:
	FX_COLORREF m_aHighlightColor[6];
	FX_BYTE m_iHighlightAlpha;
	FX_BOOL	m_bNeedHightlight[6];
};

#define BAI_STRUCTURE		0
#define BAI_ROW				1
#define BAI_COLUMN			2

#define CPDFSDK_Annots				CFX_ArrayTemplate<CPDFSDK_Annot*>
#define CPDFSDK_SortAnnots			CGW_ArrayTemplate<CPDFSDK_Annot*>
class CBA_AnnotIterator
{
public:
	CBA_AnnotIterator(CPDFSDK_PageView* pPageView, const CFX_ByteString& sType, const CFX_ByteString& sSubType);
	~CBA_AnnotIterator();

	CPDFSDK_Annot*				GetFirstAnnot();
	CPDFSDK_Annot*				GetLastAnnot();
	CPDFSDK_Annot*				GetNextAnnot(CPDFSDK_Annot* pAnnot);
	CPDFSDK_Annot*				GetPrevAnnot(CPDFSDK_Annot* pAnnot);

private:
	void								GenerateResults();
	static int							CompareByLeft(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2);
	static int							CompareByTop(CPDFSDK_Annot* p1, CPDFSDK_Annot* p2);
	static CPDF_Rect					GetAnnotRect(CPDFSDK_Annot* pAnnot);

	CPDFSDK_PageView*					m_pPageView;
	CFX_ByteString						m_sType;
	CFX_ByteString						m_sSubType;
	int									m_nTabs;
	CPDFSDK_Annots						m_Annots;
};

#endif  // FPDFSDK_INCLUDE_FSDK_BASEFORM_H_
