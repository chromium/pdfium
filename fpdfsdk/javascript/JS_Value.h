// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_JS_VALUE_H_
#define FPDFSDK_JAVASCRIPT_JS_VALUE_H_

#include <vector>

#include "fxjs/fxjs_v8.h"

class CJS_Array;
class CJS_Date;
class CJS_Document;
class CJS_Object;
class CJS_Runtime;

class CJS_Value {
 public:
  enum Type {
    VT_unknown,
    VT_string,
    VT_number,
    VT_boolean,
    VT_date,
    VT_object,
    VT_null,
    VT_undefined
  };

  static Type GetValueType(v8::Local<v8::Value> value);

  CJS_Value();
  explicit CJS_Value(v8::Local<v8::Value> pValue);
  CJS_Value(CJS_Runtime* pRuntime, int iValue);
  CJS_Value(CJS_Runtime* pRuntime, double dValue);
  CJS_Value(CJS_Runtime* pRuntime, bool bValue);
  explicit CJS_Value(CJS_Object* pObj);
  CJS_Value(CJS_Runtime* pRuntime, const char* pStr);
  CJS_Value(CJS_Runtime* pRuntime, const wchar_t* pWstr);
  CJS_Value(CJS_Runtime* pRuntime, const CJS_Array& array);
  explicit CJS_Value(const CJS_Date& date);
  CJS_Value(const CJS_Value& other);

  ~CJS_Value();

  // These calls may re-enter JS (and hence invalidate objects).
  void Set(v8::Local<v8::Value> pValue);

  Type GetType() const { return GetValueType(m_pValue); }

  int ToInt(CJS_Runtime* pRuntime) const;
  bool ToBool(CJS_Runtime* pRuntime) const;
  double ToDouble(CJS_Runtime* pRuntime) const;
  float ToFloat(CJS_Runtime* pRuntime) const;
  CJS_Object* ToObject(CJS_Runtime* pRuntime) const;
  CJS_Document* ToDocument(CJS_Runtime* pRuntime) const;
  CJS_Array ToArray(CJS_Runtime* pRuntime) const;
  CJS_Date ToDate() const;
  WideString ToWideString(CJS_Runtime* pRuntime) const;
  ByteString ToByteString(CJS_Runtime* pRuntime) const;
  v8::Local<v8::Object> ToV8Object(CJS_Runtime* pRuntime) const;
  v8::Local<v8::Array> ToV8Array(CJS_Runtime* pRuntime) const;
  v8::Local<v8::Value> ToV8Value() const;

  // Replace the current |m_pValue| with a v8::Number if possible
  // to make one from the current |m_pValue|.
  void MaybeCoerceToNumber(CJS_Runtime* pRuntime);

  bool IsArrayObject() const;
  bool IsDateObject() const;

 private:
  v8::Local<v8::Value> m_pValue;
};

class CJS_Array {
 public:
  CJS_Array();
  explicit CJS_Array(v8::Local<v8::Array> pArray);
  CJS_Array(const CJS_Array& other);
  virtual ~CJS_Array();

  int GetLength(CJS_Runtime* pRuntime) const;

  // These two calls may re-enter JS (and hence invalidate objects).
  CJS_Value GetElement(CJS_Runtime* pRuntime, unsigned index) const;
  void SetElement(CJS_Runtime* pRuntime,
                  unsigned index,
                  const CJS_Value& value);

  v8::Local<v8::Array> ToV8Array(CJS_Runtime* pRuntime) const;

 private:
  mutable v8::Local<v8::Array> m_pArray;
};

class CJS_Date {
 public:
  CJS_Date();
  explicit CJS_Date(v8::Local<v8::Date> pDate);
  CJS_Date(CJS_Runtime* pRuntime, double dMsec_time);
  CJS_Date(CJS_Runtime* pRuntime,
           int year,
           int mon,
           int day,
           int hour,
           int min,
           int sec);
  CJS_Date(const CJS_Date&);
  virtual ~CJS_Date();

  bool IsValidDate(CJS_Runtime* pRuntime) const;

  int GetYear(CJS_Runtime* pRuntime) const;
  int GetMonth(CJS_Runtime* pRuntime) const;
  int GetDay(CJS_Runtime* pRuntime) const;
  int GetHours(CJS_Runtime* pRuntime) const;
  int GetMinutes(CJS_Runtime* pRuntime) const;
  int GetSeconds(CJS_Runtime* pRuntime) const;

  v8::Local<v8::Date> ToV8Date() const;
  WideString ToWideString(int style) const;

 protected:
  v8::Local<v8::Date> m_pDate;
};

double JS_GetDateTime();
int JS_GetYearFromTime(double dt);
int JS_GetMonthFromTime(double dt);
int JS_GetDayFromTime(double dt);
int JS_GetHourFromTime(double dt);
int JS_GetMinFromTime(double dt);
int JS_GetSecFromTime(double dt);
double JS_DateParse(const WideString& str);
double JS_MakeDay(int nYear, int nMonth, int nDay);
double JS_MakeTime(int nHour, int nMin, int nSec, int nMs);
double JS_MakeDate(double day, double time);

// Some JS methods have the bizarre convention that they may also be called
// with a single argument which is an object containing the actual arguments
// as its properties. The varying arguments to this method are the property
// names as wchar_t string literals corresponding to each positional argument.
// The result will always contain |nKeywords| value, with unspecified ones
// being set to type VT_unknown.
std::vector<CJS_Value> ExpandKeywordParams(
    CJS_Runtime* pRuntime,
    const std::vector<CJS_Value>& originals,
    size_t nKeywords,
    ...);

#endif  // FPDFSDK_JAVASCRIPT_JS_VALUE_H_
