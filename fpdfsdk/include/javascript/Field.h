// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_FIELD_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_FIELD_H_

#include <string>  // For std::wstring.

// TODO(tsepez): include PWL_Wnd.h for PWL_Color after fixing its IWYU.
#include "JS_Define.h"

class Document;

enum FIELD_PROP
{
	FP_ALIGNMENT,
	FP_BORDERSTYLE,
	FP_BUTTONALIGNX,
	FP_BUTTONALIGNY,
	FP_BUTTONFITBOUNDS,
	FP_BUTTONPOSITION,
	FP_BUTTONSCALEHOW,
	FP_BUTTONSCALEWHEN,
	FP_CALCORDERINDEX,
	FP_CHARLIMIT,
	FP_COMB,
	FP_COMMITONSELCHANGE,
	FP_CURRENTVALUEINDICES,
	FP_DEFAULTVALUE,
	FP_DONOTSCROLL,
	FP_DISPLAY,
	FP_FILLCOLOR,
	FP_HIDDEN,
	FP_HIGHLIGHT,
	FP_LINEWIDTH,
	FP_MULTILINE,
	FP_MULTIPLESELECTION,
	FP_PASSWORD,
	FP_RECT,
	FP_RICHTEXT,
	FP_RICHVALUE,
	FP_ROTATION,
	FP_STROKECOLOR,
	FP_STYLE,
	FP_TEXTCOLOR,
	FP_TEXTFONT,
	FP_TEXTSIZE,
	FP_USERNAME,
	FP_VALUE
};

class CJS_WideStringArray
{
public:
	CJS_WideStringArray(){}
	virtual ~CJS_WideStringArray()
	{
		for (int i=0,sz=m_Data.GetSize(); i<sz; i++)
			delete m_Data.GetAt(i);
		m_Data.RemoveAll();
	}

	void Add(const CFX_WideString& string)
	{
		m_Data.Add(new CFX_WideString(string));
	}

	int GetSize() const
	{
		return m_Data.GetSize();
	}

	CFX_WideString GetAt(int i) const
	{
		return *m_Data.GetAt(i);
	}

private:
	CFX_ArrayTemplate<CFX_WideString*>	m_Data;
};

struct CJS_DelayData
{
	CFX_WideString			sFieldName;
	int						nControlIndex;
	enum FIELD_PROP			eProp;
	int32_t				num;
	bool					b;
	CFX_ByteString			string;
	CFX_WideString			widestring;
	CPDF_Rect				rect;
	CPWL_Color				color;
	CFX_DWordArray			wordarray;
	CJS_WideStringArray		widestringarray;
};

class Field : public CJS_EmbedObj
{
public:
	Field(CJS_Object* pJSObject);
	virtual ~Field(void);

    bool alignment(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool borderStyle(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool buttonAlignX(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool buttonAlignY(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool buttonFitBounds(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool buttonPosition(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool buttonScaleHow(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool buttonScaleWhen(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool calcOrderIndex(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool charLimit(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool comb(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool commitOnSelChange(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool currentValueIndices(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool defaultStyle(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool defaultValue(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool doNotScroll(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool doNotSpellCheck(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool delay(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool display(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool doc(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool editable(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool exportValues(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool fileSelect(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool fillColor(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool hidden(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool highlight(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool lineWidth(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool multiline(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool multipleSelection(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool name(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool numItems(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool page(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool password(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool print(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool radiosInUnison(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool readonly(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool rect(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
    bool required(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool richText(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool richValue(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool rotation(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool strokeColor(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool style(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool submitName(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool textColor(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool textFont(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool textSize(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool type(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool userName(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool value(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool valueAsString(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);
	bool source(IFXJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError);

	bool browseForFileToSubmit(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool buttonGetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool buttonGetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool buttonImportIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool buttonSetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool buttonSetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool checkThisBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool clearItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool defaultIsChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool deleteItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool getArray(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool getItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool getLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool insertItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool isBoxChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool isDefaultChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool setAction(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool setFocus(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool setItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool setLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureGetModifications(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureGetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureInfo(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureSetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureSign(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);
	bool signatureValidate(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, CFX_WideString& sError);

public:
	static void SetAlignment(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string);
    static void SetBorderStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string);
	static void SetButtonAlignX(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetButtonAlignY(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetButtonFitBounds(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetButtonPosition(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetButtonScaleHow(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
    static void SetButtonScaleWhen(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetCalcOrderIndex(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetCharLimit(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetComb(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetCommitOnSelChange(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetCurrentValueIndices(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_DWordArray& array);
    static void SetDefaultStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex);
	static void SetDefaultValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_WideString& string);
	static void SetDoNotScroll(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetDisplay(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetFillColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color);
	static void SetHidden(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
    static void SetHighlight(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string);
	static void SetLineWidth(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetMultiline(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetMultipleSelection(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetPassword(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetRect(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPDF_Rect& rect);
	static void SetRichText(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, bool b);
	static void SetRichValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex);
	static void SetRotation(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetStrokeColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color);
	static void SetStyle(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string);
	static void SetTextColor(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CPWL_Color& color);
	static void SetTextFont(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_ByteString& string);
	static void SetTextSize(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, int number);
	static void SetUserName(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CFX_WideString& string);
	static void SetValue(CPDFSDK_Document* pDocument, const CFX_WideString& swFieldName, int nControlIndex, const CJS_WideStringArray& strArray);

public:
	static void							AddField(CPDFSDK_Document* pDocument, int nPageIndex, int nFieldType,
											const CFX_WideString& sName, const CPDF_Rect& rcCoords);
public:
	static void							UpdateFormField(CPDFSDK_Document* pDocument, CPDF_FormField* pFormField,
											bool bChangeMark, bool bResetAP, bool bRefresh);
	static void							UpdateFormControl(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl,
											bool bChangeMark, bool bResetAP, bool bRefresh);

	static CPDFSDK_Widget*					GetWidget(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl);
	static void							GetFormFields(CPDFSDK_Document* pDocument, const CFX_WideString& csFieldName, CFX_PtrArray& FieldsArray);

	static void							DoDelay(CPDFSDK_Document* pDocument, CJS_DelayData* pData);

public:
	bool								AttachField(Document* pDocument, const CFX_WideString& csFieldName);
	void								SetDelay(bool bDelay);
	void								SetIsolate(v8::Isolate* isolate) {m_isolate = isolate;}
protected:
	void								ParseFieldName(const std::wstring &strFieldNameParsed,std::wstring &strFieldName,int & iControlNo);
	void								GetFormFields(const CFX_WideString& csFieldName, CFX_PtrArray& FieldsArray);
	CPDF_FormControl* 					GetSmartFieldControl(CPDF_FormField* pFormField);
	bool								ValueIsOccur(CPDF_FormField* pFormField, CFX_WideString csOptLabel);

	void								AddDelay_Int(enum FIELD_PROP prop, int32_t n);
	void								AddDelay_Bool(enum FIELD_PROP prop,bool b);
	void								AddDelay_String(enum FIELD_PROP prop, const CFX_ByteString& string);
	void								AddDelay_WideString(enum FIELD_PROP prop, const CFX_WideString& string);
	void								AddDelay_Rect(enum FIELD_PROP prop, const CPDF_Rect& rect);
	void								AddDelay_Color(enum FIELD_PROP prop, const CPWL_Color& color);
	void								AddDelay_WordArray(enum FIELD_PROP prop, const CFX_DWordArray& array);
	void								AddDelay_WideStringArray(enum FIELD_PROP prop, const CJS_WideStringArray& array);

	void								DoDelay();
public:
	Document*							m_pJSDoc;
	CPDFSDK_Document*					m_pDocument;
	CFX_WideString						m_FieldName;
	int									m_nFormControlIndex;
	bool								m_bCanSet;

	bool								m_bDelay;
	v8::Isolate*							m_isolate;
};

class CJS_Field : public CJS_Object
{
public:
	CJS_Field(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Field(void){};

	virtual bool	InitInstance(IFXJS_Context* cc);

	DECLARE_JS_CLASS(CJS_Field);

    JS_STATIC_PROP(alignment, Field);
    JS_STATIC_PROP(borderStyle, Field);
	JS_STATIC_PROP(buttonAlignX, Field);
	JS_STATIC_PROP(buttonAlignY, Field);
	JS_STATIC_PROP(buttonFitBounds, Field);
	JS_STATIC_PROP(buttonPosition, Field);
	JS_STATIC_PROP(buttonScaleHow, Field);
    JS_STATIC_PROP(buttonScaleWhen, Field);
	JS_STATIC_PROP(calcOrderIndex, Field);
	JS_STATIC_PROP(charLimit, Field);
	JS_STATIC_PROP(comb, Field);
	JS_STATIC_PROP(commitOnSelChange, Field);
	JS_STATIC_PROP(currentValueIndices, Field);
    JS_STATIC_PROP(defaultStyle, Field);
	JS_STATIC_PROP(defaultValue, Field);
	JS_STATIC_PROP(doNotScroll, Field);
	JS_STATIC_PROP(doNotSpellCheck, Field);
	JS_STATIC_PROP(delay, Field);
	JS_STATIC_PROP(display, Field);
    JS_STATIC_PROP(doc, Field);
	JS_STATIC_PROP(editable, Field);
	JS_STATIC_PROP(exportValues, Field);
	JS_STATIC_PROP(fileSelect, Field);
	JS_STATIC_PROP(fillColor, Field);
	JS_STATIC_PROP(hidden, Field);
    JS_STATIC_PROP(highlight, Field);
	JS_STATIC_PROP(lineWidth, Field);
	JS_STATIC_PROP(multiline, Field);
	JS_STATIC_PROP(multipleSelection, Field);
	JS_STATIC_PROP(name, Field);
	JS_STATIC_PROP(numItems, Field);
    JS_STATIC_PROP(page, Field);
	JS_STATIC_PROP(password, Field);
	JS_STATIC_PROP(print, Field);
	JS_STATIC_PROP(radiosInUnison, Field);
	JS_STATIC_PROP(readonly, Field);
	JS_STATIC_PROP(rect, Field);
    JS_STATIC_PROP(required, Field);
	JS_STATIC_PROP(richText, Field);
	JS_STATIC_PROP(richValue, Field);
	JS_STATIC_PROP(rotation, Field);
	JS_STATIC_PROP(strokeColor, Field);
	JS_STATIC_PROP(style, Field);
	JS_STATIC_PROP(submitName, Field);
	JS_STATIC_PROP(textColor, Field);
	JS_STATIC_PROP(textFont, Field);
	JS_STATIC_PROP(textSize, Field);
	JS_STATIC_PROP(type, Field);
	JS_STATIC_PROP(userName, Field);
	JS_STATIC_PROP(value, Field);
	JS_STATIC_PROP(valueAsString, Field);
	JS_STATIC_PROP(source, Field);

	JS_STATIC_METHOD(browseForFileToSubmit, Field);
	JS_STATIC_METHOD(buttonGetCaption, Field);
	JS_STATIC_METHOD(buttonGetIcon, Field);
	JS_STATIC_METHOD(buttonImportIcon, Field);
	JS_STATIC_METHOD(buttonSetCaption, Field);
	JS_STATIC_METHOD(buttonSetIcon, Field);
	JS_STATIC_METHOD(checkThisBox, Field);
	JS_STATIC_METHOD(clearItems, Field);
 	JS_STATIC_METHOD(defaultIsChecked, Field);
	JS_STATIC_METHOD(deleteItemAt, Field);
	JS_STATIC_METHOD(getArray, Field);
	JS_STATIC_METHOD(getItemAt, Field);
	JS_STATIC_METHOD(getLock, Field);
	JS_STATIC_METHOD(insertItemAt, Field);
	JS_STATIC_METHOD(isBoxChecked, Field);
	JS_STATIC_METHOD(isDefaultChecked, Field);
	JS_STATIC_METHOD(setAction, Field);
	JS_STATIC_METHOD(setFocus, Field);
	JS_STATIC_METHOD(setItems, Field);
	JS_STATIC_METHOD(setLock, Field);
	JS_STATIC_METHOD(signatureGetModifications, Field);
	JS_STATIC_METHOD(signatureGetSeedValue, Field);
	JS_STATIC_METHOD(signatureInfo, Field);
	JS_STATIC_METHOD(signatureSetSeedValue, Field);
	JS_STATIC_METHOD(signatureSign, Field);
	JS_STATIC_METHOD(signatureValidate, Field);
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_FIELD_H_
