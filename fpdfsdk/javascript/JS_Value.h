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

class CJS_Return {
 public:
  explicit CJS_Return(bool);
  explicit CJS_Return(const WideString&);
  explicit CJS_Return(v8::Local<v8::Value>);
  CJS_Return(const CJS_Return&);
  ~CJS_Return();

  bool HasError() const { return is_error_; }
  WideString Error() const { return error_; }

  bool HasReturn() const { return !return_.IsEmpty(); }
  v8::Local<v8::Value> Return() const { return return_; }

 private:
  CJS_Return() = delete;

  bool is_error_ = false;
  WideString error_;
  v8::Local<v8::Value> return_;
};

class CJS_Value {
 public:
  CJS_Value();
  explicit CJS_Value(v8::Local<v8::Value> pValue);
  CJS_Value(const CJS_Value& other);
  ~CJS_Value();

  // These calls may re-enter JS (and hence invalidate objects).
  void Set(v8::Local<v8::Value> pValue);

  v8::Local<v8::Value> ToV8Value() const;

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
  v8::Local<v8::Value> GetElement(CJS_Runtime* pRuntime, unsigned index) const;
  void SetElement(CJS_Runtime* pRuntime,
                  unsigned index,
                  v8::Local<v8::Value> value);

  v8::Local<v8::Value> ToV8Value() const { return m_pArray; }

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

  v8::Local<v8::Value> ToV8Value() const { return m_pDate; }

 private:
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
std::vector<v8::Local<v8::Value>> ExpandKeywordParams(
    CJS_Runtime* pRuntime,
    const std::vector<v8::Local<v8::Value>>& originals,
    size_t nKeywords,
    ...);

#endif  // FPDFSDK_JAVASCRIPT_JS_VALUE_H_
