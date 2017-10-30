// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_COLOR_H_
#define FXJS_CJS_COLOR_H_

#include <vector>

#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fxjs/JS_Define.h"

class color : public CJS_EmbedObj {
 public:
  static v8::Local<v8::Array> ConvertPWLColorToArray(CJS_Runtime* pRuntime,
                                                     const CFX_Color& color);
  static CFX_Color ConvertArrayToPWLColor(CJS_Runtime* pRuntime,
                                          v8::Local<v8::Array> array);

  explicit color(CJS_Object* pJSObject);
  ~color() override;

  CJS_Return get_black(CJS_Runtime* pRuntime);
  CJS_Return set_black(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_blue(CJS_Runtime* pRuntime);
  CJS_Return set_blue(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_cyan(CJS_Runtime* pRuntime);
  CJS_Return set_cyan(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_dark_gray(CJS_Runtime* pRuntime);
  CJS_Return set_dark_gray(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_gray(CJS_Runtime* pRuntime);
  CJS_Return set_gray(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_green(CJS_Runtime* pRuntime);
  CJS_Return set_green(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_light_gray(CJS_Runtime* pRuntime);
  CJS_Return set_light_gray(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_magenta(CJS_Runtime* pRuntime);
  CJS_Return set_magenta(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_red(CJS_Runtime* pRuntime);
  CJS_Return set_red(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_transparent(CJS_Runtime* pRuntime);
  CJS_Return set_transparent(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_white(CJS_Runtime* pRuntime);
  CJS_Return set_white(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_yellow(CJS_Runtime* pRuntime);
  CJS_Return set_yellow(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return convert(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return equal(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params);

 private:
  CJS_Return GetPropertyHelper(CJS_Runtime* pRuntime, CFX_Color* val);
  CJS_Return SetPropertyHelper(CJS_Runtime* pRuntime,
                               v8::Local<v8::Value> vp,
                               CFX_Color* val);

  CFX_Color m_crTransparent;
  CFX_Color m_crBlack;
  CFX_Color m_crWhite;
  CFX_Color m_crRed;
  CFX_Color m_crGreen;
  CFX_Color m_crBlue;
  CFX_Color m_crCyan;
  CFX_Color m_crMagenta;
  CFX_Color m_crYellow;
  CFX_Color m_crDKGray;
  CFX_Color m_crGray;
  CFX_Color m_crLTGray;
};

class CJS_Color : public CJS_Object {
 public:
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_Color(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Color() override {}

  JS_STATIC_PROP(black, black, color);
  JS_STATIC_PROP(blue, blue, color);
  JS_STATIC_PROP(cyan, cyan, color);
  JS_STATIC_PROP(dkGray, dark_gray, color);
  JS_STATIC_PROP(gray, gray, color);
  JS_STATIC_PROP(green, green, color);
  JS_STATIC_PROP(ltGray, light_gray, color);
  JS_STATIC_PROP(magenta, magenta, color);
  JS_STATIC_PROP(red, red, color);
  JS_STATIC_PROP(transparent, transparent, color);
  JS_STATIC_PROP(white, white, color);
  JS_STATIC_PROP(yellow, yellow, color);

  JS_STATIC_METHOD(convert, color);
  JS_STATIC_METHOD(equal, color);

 private:
  static int ObjDefnID;
  static const JSPropertySpec PropertySpecs[];
  static const JSMethodSpec MethodSpecs[];
};

#endif  // FXJS_CJS_COLOR_H_
