// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FIELD_H_
#define _FIELD_H_

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
	FX_INT32				num;
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

    FX_BOOL alignment(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL borderStyle(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL buttonAlignX(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL buttonAlignY(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL buttonFitBounds(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL buttonPosition(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL buttonScaleHow(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL buttonScaleWhen(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL calcOrderIndex(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL charLimit(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL comb(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL commitOnSelChange(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL currentValueIndices(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL defaultStyle(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL defaultValue(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL doNotScroll(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL doNotSpellCheck(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL delay(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL display(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL doc(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL editable(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL exportValues(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL fileSelect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL fillColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL hidden(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL highlight(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL lineWidth(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL multiline(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL multipleSelection(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL name(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL numItems(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL page(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL password(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL print(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL radiosInUnison(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL readonly(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL rect(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
    FX_BOOL required(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL richText(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL richValue(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL rotation(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL strokeColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL style(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL submitName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL textColor(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL textFont(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL textSize(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL type(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL userName(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL value(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL valueAsString(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);
	FX_BOOL source(IFXJS_Context* cc, CJS_PropValue& vp, JS_ErrorString& sError);

	FX_BOOL browseForFileToSubmit(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL buttonGetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL buttonGetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL buttonImportIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL buttonSetCaption(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL buttonSetIcon(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL checkThisBox(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL clearItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL defaultIsChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL deleteItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL getArray(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL getItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL getLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL insertItemAt(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL isBoxChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL isDefaultChecked(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL setAction(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL setFocus(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL setItems(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL setLock(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureGetModifications(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureGetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureInfo(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureSetSeedValue(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureSign(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);
	FX_BOOL signatureValidate(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);

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
											FX_BOOL bChangeMark, FX_BOOL bResetAP, FX_BOOL bRefresh);
	static void							UpdateFormControl(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl, 
											FX_BOOL bChangeMark, FX_BOOL bResetAP, FX_BOOL bRefresh);

	static CPDFSDK_Widget*					GetWidget(CPDFSDK_Document* pDocument, CPDF_FormControl* pFormControl);
	static void							GetFormFields(CPDFSDK_Document* pDocument, const CFX_WideString& csFieldName, CFX_PtrArray& FieldsArray);

	static void							DoDelay(CPDFSDK_Document* pDocument, CJS_DelayData* pData);

public:
	FX_BOOL								AttachField(Document* pDocument, const CFX_WideString& csFieldName);
	void								SetDelay(FX_BOOL bDelay);
	void								SetIsolate(v8::Isolate* isolate) {m_isolate = isolate;}
protected:
	void								ParseFieldName(const std::wstring &strFieldNameParsed,std::wstring &strFieldName,int & iControlNo);
	void								GetFormFields(const CFX_WideString& csFieldName, CFX_PtrArray& FieldsArray);
	CPDF_FormControl* 					GetSmartFieldControl(CPDF_FormField* pFormField);
	FX_BOOL								ValueIsOccur(CPDF_FormField* pFormField, CFX_WideString csOptLabel);

	void								AddDelay_Int(enum FIELD_PROP prop, FX_INT32 n);
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
	FX_BOOL								m_bCanSet;

	FX_BOOL								m_bDelay;
	v8::Isolate*							m_isolate;
};

class CJS_Field : public CJS_Object
{
public:
	CJS_Field(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Field(void){};

	virtual FX_BOOL	InitInstance(IFXJS_Context* cc);

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

#endif //_FIELD_H_

