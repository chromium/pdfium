// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_VALUE_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_VALUE_H_

#include "../../../core/include/fxcrt/fx_basic.h"
#include "../jsapi/fxjs_v8.h"

class CJS_Array;
class CJS_Date;
class CJS_Document;
class CJS_Object;

class CJS_Value
{
public:
	CJS_Value(v8::Isolate* isolate);
	CJS_Value(v8::Isolate* isolate, v8::Local<v8::Value> pValue,FXJSVALUETYPE t);
	CJS_Value(v8::Isolate* isolate, const int &iValue);
	CJS_Value(v8::Isolate* isolate, const double &dValue);
	CJS_Value(v8::Isolate* isolate, const float &fValue);
	CJS_Value(v8::Isolate* isolate, const bool &bValue);
	CJS_Value(v8::Isolate* isolate, JSFXObject);
	CJS_Value(v8::Isolate* isolate, CJS_Object*);
   	CJS_Value(v8::Isolate* isolate, CJS_Document*);
	CJS_Value(v8::Isolate* isolate, FX_LPCSTR pStr);
	CJS_Value(v8::Isolate* isolate, FX_LPCWSTR pWstr);
	CJS_Value(v8::Isolate* isolate, CJS_Array& array);

	~CJS_Value();

	void SetNull();
    void Attach(v8::Local<v8::Value> pValue,FXJSVALUETYPE t);
	void Attach(CJS_Value *pValue);
	void Detach();


	int ToInt() const;
	bool ToBool() const;
	double ToDouble() const;
	float  ToFloat() const;
	CJS_Object* ToCJSObject() const;
	CFX_WideString ToCFXWideString() const;
	CFX_ByteString ToCFXByteString() const;
	v8::Local<v8::Object> ToV8Object() const;
	v8::Local<v8::Array> ToV8Array() const;
	v8::Local<v8::Value> ToV8Value() const;

	void operator = (int iValue);
	void operator = (bool bValue);
	void operator = (double);
	void operator = (float);
	void operator = (CJS_Object*);
	void operator = (CJS_Document*);
	void operator = (v8::Local<v8::Object>);
	void operator = (CJS_Array &);
	void operator = (CJS_Date &);
	void operator = (FX_LPCWSTR pWstr);
	void operator = (FX_LPCSTR pStr);
	void operator = (CJS_Value value);

	FX_BOOL IsArrayObject() const;
	FX_BOOL	IsDateObject() const;
	FXJSVALUETYPE GetType() const;

	FX_BOOL ConvertToArray(CJS_Array &) const;
	FX_BOOL ConvertToDate(CJS_Date &) const;

	v8::Isolate* GetIsolate() {return m_isolate;}
protected:
	v8::Local<v8::Value> m_pValue;
	FXJSVALUETYPE m_eType;
	v8::Isolate* m_isolate;
};

class CJS_Parameters : public CFX_ArrayTemplate<CJS_Value>
{
public:
	void push_back(const CJS_Value& newElement) {
		CFX_ArrayTemplate<CJS_Value>::Add(newElement);
	}
	int size() const {
		return CFX_ArrayTemplate<CJS_Value>::GetSize();
	}
};

class CJS_PropValue: public CJS_Value
{
public:
	CJS_PropValue(const CJS_Value&);
	CJS_PropValue(v8::Isolate* isolate);
	~CJS_PropValue();
public:
	FX_BOOL IsSetting();
	FX_BOOL IsGetting();
	void operator<<(int);
	void operator>>(int&) const;
	void operator<<(bool);
	void operator>>(bool&) const;
	void operator<<(double);
	void operator>>(double&) const;
	void operator<<(CJS_Object* pObj);
	void operator>>(CJS_Object*& ppObj) const;
	void operator<<(CJS_Document* pJsDoc);
	void operator>>(CJS_Document*& ppJsDoc) const;
	void operator<<(CFX_ByteString);
	void operator>>(CFX_ByteString&) const;
	void operator<<(CFX_WideString);
	void operator>>(CFX_WideString&) const;
	void operator<<(FX_LPCWSTR c_string);
	void operator<<(JSFXObject);
	void operator>>(JSFXObject&) const;
	void operator>>(CJS_Array& array) const;
	void operator<<(CJS_Array& array);
	void operator<<(CJS_Date& date);
	void operator>>(CJS_Date& date) const;
	operator v8::Local<v8::Value>() const;
	void StartSetting();
	void StartGetting();
private:
	FX_BOOL m_bIsSetting;
};

class CJS_Array
{
public:
	CJS_Array(v8::Isolate* isolate);
	virtual ~CJS_Array();

	void Attach(v8::Local<v8::Array> pArray);
	void GetElement(unsigned index,CJS_Value &value);
	void SetElement(unsigned index,CJS_Value value);
    int GetLength();
	FX_BOOL IsAttached();
	operator v8::Local<v8::Array>();

	v8::Isolate* GetIsolate() {return m_isolate;}
private:
	v8::Local<v8::Array> m_pArray;
	v8::Isolate* m_isolate;
};

class CJS_Date
{
friend class CJS_Value;
public:
	CJS_Date(v8::Isolate* isolate);
	CJS_Date(v8::Isolate* isolate,double dMsec_time);
	CJS_Date(v8::Isolate* isolate,int year, int mon, int day,int hour, int min, int sec);
	virtual ~CJS_Date();
	void Attach(v8::Local<v8::Value> pDate);

	int     GetYear();
	void    SetYear(int iYear);

	int     GetMonth();
	void    SetMonth(int iMonth);

	int     GetDay();
	void    SetDay(int iDay);

	int     GetHours();
	void    SetHours(int iHours);

	int     GetMinutes();
	void    SetMinutes(int minutes);

	int     GetSeconds();
	void    SetSeconds(int seconds);

	operator v8::Local<v8::Value>();
	operator double() const;

	CFX_WideString	ToString() const;

	static double	MakeDate(int year, int mon, int mday,int hour, int min, int sec,int ms);

	FX_BOOL	IsValidDate();

protected:
	v8::Local<v8::Value> m_pDate;
	v8::Isolate* m_isolate;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_VALUE_H_
