// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/Consts.h"

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"

JSConstSpec CJS_Border::ConstSpecs[] = {
    {L"s", JSConstSpec::String, 0, L"solid"},
    {L"b", JSConstSpec::String, 0, L"beveled"},
    {L"d", JSConstSpec::String, 0, L"dashed"},
    {L"i", JSConstSpec::String, 0, L"inset"},
    {L"u", JSConstSpec::String, 0, L"underline"},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Border, border)

JSConstSpec CJS_Display::ConstSpecs[] = {
    {L"visible", JSConstSpec::Number, 0, 0},
    {L"hidden", JSConstSpec::Number, 1, 0},
    {L"noPrint", JSConstSpec::Number, 2, 0},
    {L"noView", JSConstSpec::Number, 3, 0},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Display, display)

JSConstSpec CJS_Font::ConstSpecs[] = {
    {L"Times", JSConstSpec::String, 0, L"Times-Roman"},
    {L"TimesB", JSConstSpec::String, 0, L"Times-Bold"},
    {L"TimesI", JSConstSpec::String, 0, L"Times-Italic"},
    {L"TimesBI", JSConstSpec::String, 0, L"Times-BoldItalic"},
    {L"Helv", JSConstSpec::String, 0, L"Helvetica"},
    {L"HelvB", JSConstSpec::String, 0, L"Helvetica-Bold"},
    {L"HelvI", JSConstSpec::String, 0, L"Helvetica-Oblique"},
    {L"HelvBI", JSConstSpec::String, 0, L"Helvetica-BoldOblique"},
    {L"Cour", JSConstSpec::String, 0, L"Courier"},
    {L"CourB", JSConstSpec::String, 0, L"Courier-Bold"},
    {L"CourI", JSConstSpec::String, 0, L"Courier-Oblique"},
    {L"CourBI", JSConstSpec::String, 0, L"Courier-BoldOblique"},
    {L"Symbol", JSConstSpec::String, 0, L"Symbol"},
    {L"ZapfD", JSConstSpec::String, 0, L"ZapfDingbats"},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Font, font)

JSConstSpec CJS_Highlight::ConstSpecs[] = {
    {L"n", JSConstSpec::String, 0, L"none"},
    {L"i", JSConstSpec::String, 0, L"invert"},
    {L"p", JSConstSpec::String, 0, L"push"},
    {L"o", JSConstSpec::String, 0, L"outline"},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Highlight, highlight)

JSConstSpec CJS_Position::ConstSpecs[] = {
    {L"textOnly", JSConstSpec::Number, 0, 0},
    {L"iconOnly", JSConstSpec::Number, 1, 0},
    {L"iconTextV", JSConstSpec::Number, 2, 0},
    {L"textIconV", JSConstSpec::Number, 3, 0},
    {L"iconTextH", JSConstSpec::Number, 4, 0},
    {L"textIconH", JSConstSpec::Number, 5, 0},
    {L"overlay", JSConstSpec::Number, 6, 0},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Position, position)

JSConstSpec CJS_ScaleHow::ConstSpecs[] = {
    {L"proportional", JSConstSpec::Number, 0, 0},
    {L"anamorphic", JSConstSpec::Number, 1, 0},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_ScaleHow, scaleHow)

JSConstSpec CJS_ScaleWhen::ConstSpecs[] = {
    {L"always", JSConstSpec::Number, 0, 0},
    {L"never", JSConstSpec::Number, 1, 0},
    {L"tooBig", JSConstSpec::Number, 2, 0},
    {L"tooSmall", JSConstSpec::Number, 3, 0},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_ScaleWhen, scaleWhen)

JSConstSpec CJS_Style::ConstSpecs[] = {
    {L"ch", JSConstSpec::String, 0, L"check"},
    {L"cr", JSConstSpec::String, 0, L"cross"},
    {L"di", JSConstSpec::String, 0, L"diamond"},
    {L"ci", JSConstSpec::String, 0, L"circle"},
    {L"st", JSConstSpec::String, 0, L"star"},
    {L"sq", JSConstSpec::String, 0, L"square"},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Style, style)

JSConstSpec CJS_Zoomtype::ConstSpecs[] = {
    {L"none", JSConstSpec::String, 0, L"NoVary"},
    {L"fitP", JSConstSpec::String, 0, L"FitPage"},
    {L"fitW", JSConstSpec::String, 0, L"FitWidth"},
    {L"fitH", JSConstSpec::String, 0, L"FitHeight"},
    {L"fitV", JSConstSpec::String, 0, L"FitVisibleWidth"},
    {L"pref", JSConstSpec::String, 0, L"Preferred"},
    {L"refW", JSConstSpec::String, 0, L"ReflowWidth"},
    {0, JSConstSpec::Number, 0, 0}};
IMPLEMENT_JS_CLASS_CONST(CJS_Zoomtype, zoomtype)

#define GLOBAL_STRING(rt, name, value)                                \
  (rt)->DefineGlobalConst(                                            \
      (name), [](const v8::FunctionCallbackInfo<v8::Value>& info) {   \
        info.GetReturnValue().Set(                                    \
            CFXJS_Engine::CurrentEngineFromIsolate(info.GetIsolate()) \
                ->NewString(value));                                  \
      })

void CJS_GlobalConsts::DefineJSObjects(CJS_Runtime* pRuntime) {
  GLOBAL_STRING(pRuntime, L"IDS_GREATER_THAN",
                L"Invalid value: must be greater than or equal to % s.");

  GLOBAL_STRING(pRuntime, L"IDS_GT_AND_LT",
                L"Invalid value: must be greater than or equal to % s "
                L"and less than or equal to % s.");

  GLOBAL_STRING(pRuntime, L"IDS_LESS_THAN",
                L"Invalid value: must be less than or equal to % s.");

  GLOBAL_STRING(pRuntime, L"IDS_INVALID_MONTH", L"**Invalid**");
  GLOBAL_STRING(
      pRuntime, L"IDS_INVALID_DATE",
      L"Invalid date / time: please ensure that the date / time exists.Field");

  GLOBAL_STRING(pRuntime, L"IDS_INVALID_VALUE",
                L"The value entered does not match the format of the field");

  GLOBAL_STRING(pRuntime, L"IDS_AM", L"am");
  GLOBAL_STRING(pRuntime, L"IDS_PM", L"pm");
  GLOBAL_STRING(pRuntime, L"IDS_MONTH_INFO",
                L"January[1] February[2] March[3] April[4] May[5] "
                L"June[6] July[7] August[8] September[9] October[10] "
                L"November[11] December[12] Sept[9] Jan[1] Feb[2] Mar[3] "
                L"Apr[4] Jun[6] Jul[7] Aug[8] Sep[9] Oct[10] Nov[11] "
                L"Dec[12]");

  GLOBAL_STRING(pRuntime, L"IDS_STARTUP_CONSOLE_MSG", L"** ^ _ ^ **");
}

#define GLOBAL_ARRAY(rt, name, ...)                                          \
  {                                                                          \
    const FX_WCHAR* values[] = {__VA_ARGS__};                                \
    v8::Local<v8::Array> array = (rt)->NewArray();                           \
    for (size_t i = 0; i < FX_ArraySize(values); ++i)                        \
      array->Set(i, (rt)->NewString(values[i]));                             \
    (rt)->SetConstArray((name), array);                                      \
    (rt)->DefineGlobalConst(                                                 \
        (name), [](const v8::FunctionCallbackInfo<v8::Value>& info) {        \
          CJS_Runtime* pCurrentRuntime =                                     \
              CJS_Runtime::CurrentRuntimeFromIsolate(info.GetIsolate());     \
          if (pCurrentRuntime)                                               \
            info.GetReturnValue().Set(pCurrentRuntime->GetConstArray(name)); \
        });                                                                  \
  }

void CJS_GlobalArrays::DefineJSObjects(CJS_Runtime* pRuntime) {
  GLOBAL_ARRAY(pRuntime, L"RE_NUMBER_ENTRY_DOT_SEP", L"[+-]?\\d*\\.?\\d*");
  GLOBAL_ARRAY(pRuntime, L"RE_NUMBER_COMMIT_DOT_SEP",
               L"[+-]?\\d+(\\.\\d+)?",  // -1.0 or -1
               L"[+-]?\\.\\d+",         // -.1
               L"[+-]?\\d+\\.");        // -1.

  GLOBAL_ARRAY(pRuntime, L"RE_NUMBER_ENTRY_COMMA_SEP", L"[+-]?\\d*,?\\d*");
  GLOBAL_ARRAY(pRuntime, L"RE_NUMBER_COMMIT_COMMA_SEP",
               L"[+-]?\\d+([.,]\\d+)?",  // -1,0 or -1
               L"[+-]?[.,]\\d+",         // -,1
               L"[+-]?\\d+[.,]");        // -1,

  GLOBAL_ARRAY(pRuntime, L"RE_ZIP_ENTRY", L"\\d{0,5}");
  GLOBAL_ARRAY(pRuntime, L"RE_ZIP_COMMIT", L"\\d{5}");
  GLOBAL_ARRAY(pRuntime, L"RE_ZIP4_ENTRY", L"\\d{0,5}(\\.|[- ])?\\d{0,4}");
  GLOBAL_ARRAY(pRuntime, L"RE_ZIP4_COMMIT", L"\\d{5}(\\.|[- ])?\\d{4}");
  GLOBAL_ARRAY(pRuntime, L"RE_PHONE_ENTRY",
               // 555-1234 or 408 555-1234
               L"\\d{0,3}(\\.|[- ])?\\d{0,3}(\\.|[- ])?\\d{0,4}",

               // (408
               L"\\(\\d{0,3}",

               // (408) 555-1234
               // (allow the addition of parens as an afterthought)
               L"\\(\\d{0,3}\\)(\\.|[- ])?\\d{0,3}(\\.|[- ])?\\d{0,4}",

               // (408 555-1234
               L"\\(\\d{0,3}(\\.|[- ])?\\d{0,3}(\\.|[- ])?\\d{0,4}",

               // 408) 555-1234
               L"\\d{0,3}\\)(\\.|[- ])?\\d{0,3}(\\.|[- ])?\\d{0,4}",

               // international
               L"011(\\.|[- \\d])*");

  GLOBAL_ARRAY(
      pRuntime, L"RE_PHONE_COMMIT", L"\\d{3}(\\.|[- ])?\\d{4}",  // 555-1234
      L"\\d{3}(\\.|[- ])?\\d{3}(\\.|[- ])?\\d{4}",               // 408 555-1234
      L"\\(\\d{3}\\)(\\.|[- ])?\\d{3}(\\.|[- ])?\\d{4}",  // (408) 555-1234
      L"011(\\.|[- \\d])*");                              // international

  GLOBAL_ARRAY(pRuntime, L"RE_SSN_ENTRY",
               L"\\d{0,3}(\\.|[- ])?\\d{0,2}(\\.|[- ])?\\d{0,4}");

  GLOBAL_ARRAY(pRuntime, L"RE_SSN_COMMIT",
               L"\\d{3}(\\.|[- ])?\\d{2}(\\.|[- ])?\\d{4}");
}
