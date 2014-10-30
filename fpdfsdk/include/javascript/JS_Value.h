// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JS_VALUE_H_
#define _JS_VALUE_H_

class CJS_Array;
class CJS_Date;
class CJS_Document;
class CJS_Object;

class CJS_Value
{
public:
	CJS_Value(v8::Isolate* isolate);
	CJS_Value(v8::Isolate* isolate, v8::Handle<v8::Value> pValue,FXJSVALUETYPE t);
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
    void Attach(v8::Handle<v8::Value> pValue,FXJSVALUETYPE t);
	void Attach(CJS_Value *pValue);
	void Detach();


	operator int() const;
	operator bool() const;
	operator double() const;
	operator float() const;
	operator CJS_Object*() const;
	operator v8::Handle<v8::Object>() const;
	operator v8::Handle<v8::Array>() const;
	operator CFX_WideString() const;
	operator CFX_ByteString() const;
	v8::Handle<v8::Value> ToJSValue();

	void operator = (int iValue);
	void operator = (bool bValue);
	void operator = (double);
	void operator = (float);
	void operator = (CJS_Object*);
	void operator = (v8::Handle<v8::Object>);
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
	v8::Handle<v8::Value> m_pValue;
	FXJSVALUETYPE m_eType;
	v8::Isolate* m_isolate;
};

template<class TYPE> class CJS_ParametersTmpl : public CFX_ArrayTemplate<TYPE>
{
public:
	void push_back(TYPE newElement){CFX_ArrayTemplate<TYPE>::Add(newElement);}
	int size() const{return CFX_ArrayTemplate<TYPE>::GetSize();}
};
typedef CJS_ParametersTmpl<CJS_Value> CJS_Parameters;

class CJS_PropValue: public CJS_Value
{
public:
	CJS_PropValue(const CJS_Value &);
	CJS_PropValue(v8::Isolate* isolate);
	~CJS_PropValue();
public:
	FX_BOOL IsSetting();
	FX_BOOL IsGetting();
	void operator<<(int );
	void operator>>(int &) const;
	void operator<<(bool);
	void operator>>(bool &) const;
	void operator<<(double );
	void operator>>(double &) const;
	void operator<<(CJS_Object *pObj);
	void operator>>(CJS_Object *&ppObj) const;
	void operator<<(CFX_ByteString);
	void operator>>(CFX_ByteString &) const;
	void operator<<(CFX_WideString);
	void operator>>(CFX_WideString &) const;
	void operator<<(FX_LPCWSTR c_string);

	void operator<<(JSFXObject);
	void operator>>(JSFXObject &) const;

	void operator>>(CJS_Array &array) const;
	void operator<<(CJS_Array &array);

	void operator<<(CJS_Date &date);
	void operator>>(CJS_Date &date) const;

	operator v8::Handle<v8::Value>() const;

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

	void Attach(v8::Handle<v8::Array> pArray);
	void GetElement(unsigned index,CJS_Value &value);
	void SetElement(unsigned index,CJS_Value value);
    int GetLength();
	FX_BOOL IsAttached();
	operator v8::Handle<v8::Array>();

	v8::Isolate* GetIsolate() {return m_isolate;}
private:
	v8::Handle<v8::Array> m_pArray;
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
	void Attach(v8::Handle<v8::Value> pDate);

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

	operator v8::Handle<v8::Value>();
	operator double() const;

	CFX_WideString	ToString() const;

	static double	MakeDate(int year, int mon, int mday,int hour, int min, int sec,int ms);

	FX_BOOL	IsValidDate();

protected:
	v8::Handle<v8::Value> m_pDate;
	v8::Isolate* m_isolate;
};

#endif //_JS_VALUE_H_

