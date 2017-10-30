// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_APP_H_
#define FXJS_CJS_APP_H_

#include <memory>
#include <set>
#include <vector>

#include "fxjs/JS_Define.h"

class CJS_Runtime;
class GlobalTimer;

class app : public CJS_EmbedObj {
 public:
  explicit app(CJS_Object* pJSObject);
  ~app() override;

  CJS_Return get_active_docs(CJS_Runtime* pRuntime);
  CJS_Return set_active_docs(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_calculate(CJS_Runtime* pRuntime);
  CJS_Return set_calculate(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_forms_version(CJS_Runtime* pRuntime);
  CJS_Return set_forms_version(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_fs(CJS_Runtime* pRuntime);
  CJS_Return set_fs(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_fullscreen(CJS_Runtime* pRuntime);
  CJS_Return set_fullscreen(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_language(CJS_Runtime* pRuntime);
  CJS_Return set_language(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_media(CJS_Runtime* pRuntime);
  CJS_Return set_media(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_platform(CJS_Runtime* pRuntime);
  CJS_Return set_platform(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_runtime_highlight(CJS_Runtime* pRuntime);
  CJS_Return set_runtime_highlight(CJS_Runtime* pRuntime,
                                   v8::Local<v8::Value> vp);

  CJS_Return get_viewer_type(CJS_Runtime* pRuntime);
  CJS_Return set_viewer_type(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return get_viewer_variation(CJS_Runtime* pRuntime);
  CJS_Return set_viewer_variation(CJS_Runtime* pRuntime,
                                  v8::Local<v8::Value> vp);

  CJS_Return get_viewer_version(CJS_Runtime* pRuntime);
  CJS_Return set_viewer_version(CJS_Runtime* pRuntime, v8::Local<v8::Value> vp);

  CJS_Return alert(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return beep(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return browseForDoc(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return clearInterval(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return clearTimeOut(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return execDialog(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return execMenuItem(CJS_Runtime* pRuntime,
                          const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return findComponent(CJS_Runtime* pRuntime,
                           const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return goBack(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return goForward(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return launchURL(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return mailMsg(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return newFDF(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return newDoc(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return openDoc(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return openFDF(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return popUpMenuEx(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return popUpMenu(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return response(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return setInterval(CJS_Runtime* pRuntime,
                         const std::vector<v8::Local<v8::Value>>& params);
  CJS_Return setTimeOut(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params);

  void TimerProc(GlobalTimer* pTimer);
  void CancelProc(GlobalTimer* pTimer);

  static WideString SysPathToPDFPath(const WideString& sOldPath);

 private:
  // CJS_EmbedObj
  void RunJsScript(CJS_Runtime* pRuntime, const WideString& wsScript);

  void ClearTimerCommon(CJS_Runtime* pRuntime, v8::Local<v8::Value> param);

  bool m_bCalculate;
  bool m_bRuntimeHighLight;
  std::set<std::unique_ptr<GlobalTimer>> m_Timers;
};

class CJS_App : public CJS_Object {
 public:
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  explicit CJS_App(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_App() override {}

  JS_STATIC_PROP(activeDocs, active_docs, app);
  JS_STATIC_PROP(calculate, calculate, app);
  JS_STATIC_PROP(formsVersion, forms_version, app);
  JS_STATIC_PROP(fs, fs, app);
  JS_STATIC_PROP(fullscreen, fullscreen, app);
  JS_STATIC_PROP(language, language, app);
  JS_STATIC_PROP(media, media, app);
  JS_STATIC_PROP(platform, platform, app);
  JS_STATIC_PROP(runtimeHighlight, runtime_highlight, app);
  JS_STATIC_PROP(viewerType, viewer_type, app);
  JS_STATIC_PROP(viewerVariation, viewer_variation, app);
  JS_STATIC_PROP(viewerVersion, viewer_version, app);

  JS_STATIC_METHOD(alert, app);
  JS_STATIC_METHOD(beep, app);
  JS_STATIC_METHOD(browseForDoc, app);
  JS_STATIC_METHOD(clearInterval, app);
  JS_STATIC_METHOD(clearTimeOut, app);
  JS_STATIC_METHOD(execDialog, app);
  JS_STATIC_METHOD(execMenuItem, app);
  JS_STATIC_METHOD(findComponent, app);
  JS_STATIC_METHOD(goBack, app);
  JS_STATIC_METHOD(goForward, app);
  JS_STATIC_METHOD(launchURL, app);
  JS_STATIC_METHOD(mailMsg, app);
  JS_STATIC_METHOD(newFDF, app);
  JS_STATIC_METHOD(newDoc, app);
  JS_STATIC_METHOD(openDoc, app);
  JS_STATIC_METHOD(openFDF, app);
  JS_STATIC_METHOD(popUpMenuEx, app);
  JS_STATIC_METHOD(popUpMenu, app);
  JS_STATIC_METHOD(response, app);
  JS_STATIC_METHOD(setInterval, app);
  JS_STATIC_METHOD(setTimeOut, app);

 private:
  static int ObjDefnID;
  static const JSPropertySpec PropertySpecs[];
  static const JSMethodSpec MethodSpecs[];
};

#endif  // FXJS_CJS_APP_H_
