// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/color.h"

#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"

JSConstSpec CJS_Color::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Color::PropertySpecs[] = {
    {"black", get_black_static, set_black_static},
    {"blue", get_blue_static, set_blue_static},
    {"cyan", get_cyan_static, set_cyan_static},
    {"dkGray", get_dark_gray_static, set_dark_gray_static},
    {"gray", get_gray_static, set_gray_static},
    {"green", get_green_static, set_green_static},
    {"ltGray", get_light_gray_static, set_light_gray_static},
    {"magenta", get_magenta_static, set_magenta_static},
    {"red", get_red_static, set_red_static},
    {"transparent", get_transparent_static, set_transparent_static},
    {"white", get_white_static, set_white_static},
    {"yellow", get_yellow_static, set_yellow_static},
    {0, 0, 0}};

JSMethodSpec CJS_Color::MethodSpecs[] = {{"convert", convert_static},
                                         {"equal", equal_static},
                                         {0, 0}};

IMPLEMENT_JS_CLASS(CJS_Color, color)

color::color(CJS_Object* pJSObject) : CJS_EmbedObj(pJSObject) {
  m_crTransparent = CFX_Color(CFX_Color::kTransparent);
  m_crBlack = CFX_Color(CFX_Color::kGray, 0);
  m_crWhite = CFX_Color(CFX_Color::kGray, 1);
  m_crRed = CFX_Color(CFX_Color::kRGB, 1, 0, 0);
  m_crGreen = CFX_Color(CFX_Color::kRGB, 0, 1, 0);
  m_crBlue = CFX_Color(CFX_Color::kRGB, 0, 0, 1);
  m_crCyan = CFX_Color(CFX_Color::kCMYK, 1, 0, 0, 0);
  m_crMagenta = CFX_Color(CFX_Color::kCMYK, 0, 1, 0, 0);
  m_crYellow = CFX_Color(CFX_Color::kCMYK, 0, 0, 1, 0);
  m_crDKGray = CFX_Color(CFX_Color::kGray, 0.25);
  m_crGray = CFX_Color(CFX_Color::kGray, 0.5);
  m_crLTGray = CFX_Color(CFX_Color::kGray, 0.75);
}

color::~color() {}

void color::ConvertPWLColorToArray(CJS_Runtime* pRuntime,
                                   const CFX_Color& color,
                                   CJS_Array* array) {
  switch (color.nColorType) {
    case CFX_Color::kTransparent:
      array->SetElement(pRuntime, 0, CJS_Value(pRuntime, "T"));
      break;
    case CFX_Color::kGray:
      array->SetElement(pRuntime, 0, CJS_Value(pRuntime, "G"));
      array->SetElement(pRuntime, 1, CJS_Value(pRuntime, color.fColor1));
      break;
    case CFX_Color::kRGB:
      array->SetElement(pRuntime, 0, CJS_Value(pRuntime, "RGB"));
      array->SetElement(pRuntime, 1, CJS_Value(pRuntime, color.fColor1));
      array->SetElement(pRuntime, 2, CJS_Value(pRuntime, color.fColor2));
      array->SetElement(pRuntime, 3, CJS_Value(pRuntime, color.fColor3));
      break;
    case CFX_Color::kCMYK:
      array->SetElement(pRuntime, 0, CJS_Value(pRuntime, "CMYK"));
      array->SetElement(pRuntime, 1, CJS_Value(pRuntime, color.fColor1));
      array->SetElement(pRuntime, 2, CJS_Value(pRuntime, color.fColor2));
      array->SetElement(pRuntime, 3, CJS_Value(pRuntime, color.fColor3));
      array->SetElement(pRuntime, 4, CJS_Value(pRuntime, color.fColor4));
      break;
  }
}

void color::ConvertArrayToPWLColor(CJS_Runtime* pRuntime,
                                   const CJS_Array& array,
                                   CFX_Color* color) {
  int nArrayLen = array.GetLength(pRuntime);
  if (nArrayLen < 1)
    return;

  float d1 = 0;
  float d2 = 0;
  float d3 = 0;
  float d4 = 0;

  if (nArrayLen > 1)
    d1 = array.GetElement(pRuntime, 1).ToFloat(pRuntime);
  if (nArrayLen > 2)
    d2 = array.GetElement(pRuntime, 2).ToFloat(pRuntime);
  if (nArrayLen > 3)
    d3 = array.GetElement(pRuntime, 3).ToFloat(pRuntime);
  if (nArrayLen > 4)
    d4 = array.GetElement(pRuntime, 4).ToFloat(pRuntime);

  ByteString sSpace = array.GetElement(pRuntime, 0).ToByteString(pRuntime);
  if (sSpace == "T")
    *color = CFX_Color(CFX_Color::kTransparent);
  else if (sSpace == "G")
    *color = CFX_Color(CFX_Color::kGray, d1);
  else if (sSpace == "RGB")
    *color = CFX_Color(CFX_Color::kRGB, d1, d2, d3);
  else if (sSpace == "CMYK")
    *color = CFX_Color(CFX_Color::kCMYK, d1, d2, d3, d4);
}

bool color::get_transparent(CJS_Runtime* pRuntime,
                            CJS_PropValue* vp,
                            WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crTransparent);
}

bool color::set_transparent(CJS_Runtime* pRuntime,
                            const CJS_PropValue& vp,
                            WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crTransparent);
}

bool color::get_black(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crBlack);
}

bool color::set_black(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crBlack);
}

bool color::get_white(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crWhite);
}

bool color::set_white(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crWhite);
}

bool color::get_red(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crRed);
}

bool color::set_red(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crRed);
}

bool color::get_green(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crGreen);
}

bool color::set_green(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crGreen);
}

bool color::get_blue(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crBlue);
}

bool color::set_blue(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crBlue);
}

bool color::get_cyan(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crCyan);
}

bool color::set_cyan(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crCyan);
}

bool color::get_magenta(CJS_Runtime* pRuntime,
                        CJS_PropValue* vp,
                        WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crMagenta);
}

bool color::set_magenta(CJS_Runtime* pRuntime,
                        const CJS_PropValue& vp,
                        WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crMagenta);
}

bool color::get_yellow(CJS_Runtime* pRuntime,
                       CJS_PropValue* vp,
                       WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crYellow);
}

bool color::set_yellow(CJS_Runtime* pRuntime,
                       const CJS_PropValue& vp,
                       WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crYellow);
}

bool color::get_dark_gray(CJS_Runtime* pRuntime,
                          CJS_PropValue* vp,
                          WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crDKGray);
}

bool color::set_dark_gray(CJS_Runtime* pRuntime,
                          const CJS_PropValue& vp,
                          WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crDKGray);
}

bool color::get_gray(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crGray);
}

bool color::set_gray(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crGray);
}

bool color::get_light_gray(CJS_Runtime* pRuntime,
                           CJS_PropValue* vp,
                           WideString* sError) {
  return GetPropertyHelper(pRuntime, vp, &m_crLTGray);
}

bool color::set_light_gray(CJS_Runtime* pRuntime,
                           const CJS_PropValue& vp,
                           WideString* sError) {
  return SetPropertyHelper(pRuntime, vp, &m_crLTGray);
}

bool color::GetPropertyHelper(CJS_Runtime* pRuntime,
                              CJS_PropValue* vp,
                              CFX_Color* var) {
  CJS_Array array;
    ConvertPWLColorToArray(pRuntime, *var, &array);
    vp->Set(array);
    return true;
}

bool color::SetPropertyHelper(CJS_Runtime* pRuntime,
                              const CJS_PropValue& vp,
                              CFX_Color* var) {
  if (!vp.GetJSValue()->IsArrayObject())
    return false;

  ConvertArrayToPWLColor(pRuntime, vp.GetJSValue()->ToArray(pRuntime), var);
  return true;
}

bool color::convert(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  int iSize = params.size();
  if (iSize < 2)
    return false;

  if (!params[0].IsArrayObject())
    return false;

  CFX_Color crSource;
  ConvertArrayToPWLColor(pRuntime, params[0].ToArray(pRuntime), &crSource);

  ByteString sDestSpace = params[1].ToByteString(pRuntime);
  int nColorType = CFX_Color::kTransparent;

  if (sDestSpace == "T") {
    nColorType = CFX_Color::kTransparent;
  } else if (sDestSpace == "G") {
    nColorType = CFX_Color::kGray;
  } else if (sDestSpace == "RGB") {
    nColorType = CFX_Color::kRGB;
  } else if (sDestSpace == "CMYK") {
    nColorType = CFX_Color::kCMYK;
  }

  CJS_Array aDest;
  CFX_Color crDest = crSource.ConvertColorType(nColorType);
  ConvertPWLColorToArray(pRuntime, crDest, &aDest);
  vRet = CJS_Value(pRuntime, aDest);

  return true;
}

bool color::equal(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  WideString& sError) {
  if (params.size() < 2)
    return false;
  if (!params[0].IsArrayObject() || !params[1].IsArrayObject())
    return false;

  CFX_Color color1;
  CFX_Color color2;
  ConvertArrayToPWLColor(pRuntime, params[0].ToArray(pRuntime), &color1);
  ConvertArrayToPWLColor(pRuntime, params[1].ToArray(pRuntime), &color2);

  color1 = color1.ConvertColorType(color2.nColorType);
  vRet = CJS_Value(pRuntime, color1 == color2);
  return true;
}
