// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_APP_H_
#define FPDFSDK_JAVASCRIPT_APP_H_

#include <memory>
#include <set>
#include <vector>

#include "fpdfsdk/javascript/JS_Define.h"

class CJS_Runtime;
class GlobalTimer;

class TimerObj : public CJS_EmbedObj {
 public:
  explicit TimerObj(CJS_Object* pJSObject);
  ~TimerObj() override;

  void SetTimer(GlobalTimer* pTimer);
  int GetTimerID() const { return m_nTimerID; }

 private:
  int m_nTimerID;  // Weak reference to GlobalTimer through global map.
};

class CJS_TimerObj : public CJS_Object {
 public:
  explicit CJS_TimerObj(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_TimerObj() override {}

  DECLARE_JS_CLASS();
};

class app : public CJS_EmbedObj {
 public:
  explicit app(CJS_Object* pJSObject);
  ~app() override;

  bool get_active_docs(CJS_Runtime* pRuntime,
                       CJS_PropValue* vp,
                       WideString* sError);
  bool set_active_docs(CJS_Runtime* pRuntime,
                       const CJS_PropValue& vp,
                       WideString* sError);

  bool get_calculate(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_calculate(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_forms_version(CJS_Runtime* pRuntime,
                         CJS_PropValue* vp,
                         WideString* sError);
  bool set_forms_version(CJS_Runtime* pRuntime,
                         const CJS_PropValue& vp,
                         WideString* sError);

  bool get_fs(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_fs(CJS_Runtime* pRuntime,
              const CJS_PropValue& vp,
              WideString* sError);

  bool get_fullscreen(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_fullscreen(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_language(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_language(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_media(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_media(CJS_Runtime* pRuntime,
                 const CJS_PropValue& vp,
                 WideString* sError);

  bool get_platform(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_platform(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_runtime_highlight(CJS_Runtime* pRuntime,
                             CJS_PropValue* vp,
                             WideString* sError);
  bool set_runtime_highlight(CJS_Runtime* pRuntime,
                             const CJS_PropValue& vp,
                             WideString* sError);

  bool get_viewer_type(CJS_Runtime* pRuntime,
                       CJS_PropValue* vp,
                       WideString* sError);
  bool set_viewer_type(CJS_Runtime* pRuntime,
                       const CJS_PropValue& vp,
                       WideString* sError);

  bool get_viewer_variation(CJS_Runtime* pRuntime,
                            CJS_PropValue* vp,
                            WideString* sError);
  bool set_viewer_variation(CJS_Runtime* pRuntime,
                            const CJS_PropValue& vp,
                            WideString* sError);

  bool get_viewer_version(CJS_Runtime* pRuntime,
                          CJS_PropValue* vp,
                          WideString* sError);
  bool set_viewer_version(CJS_Runtime* pRuntime,
                          const CJS_PropValue& vp,
                          WideString* sError);

  bool alert(CJS_Runtime* pRuntime,
             const std::vector<CJS_Value>& params,
             CJS_Value& vRet,
             WideString& sError);
  bool beep(CJS_Runtime* pRuntime,
            const std::vector<CJS_Value>& params,
            CJS_Value& vRet,
            WideString& sError);
  bool browseForDoc(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool clearInterval(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool clearTimeOut(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool execDialog(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  WideString& sError);
  bool execMenuItem(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool findComponent(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool goBack(CJS_Runtime* pRuntime,
              const std::vector<CJS_Value>& params,
              CJS_Value& vRet,
              WideString& sError);
  bool goForward(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 WideString& sError);
  bool launchURL(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 WideString& sError);
  bool mailMsg(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool newFDF(CJS_Runtime* pRuntime,
              const std::vector<CJS_Value>& params,
              CJS_Value& vRet,
              WideString& sError);
  bool newDoc(CJS_Runtime* pRuntime,
              const std::vector<CJS_Value>& params,
              CJS_Value& vRet,
              WideString& sError);
  bool openDoc(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool openFDF(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool popUpMenuEx(CJS_Runtime* pRuntime,
                   const std::vector<CJS_Value>& params,
                   CJS_Value& vRet,
                   WideString& sError);
  bool popUpMenu(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 WideString& sError);
  bool response(CJS_Runtime* pRuntime,
                const std::vector<CJS_Value>& params,
                CJS_Value& vRet,
                WideString& sError);
  bool setInterval(CJS_Runtime* pRuntime,
                   const std::vector<CJS_Value>& params,
                   CJS_Value& vRet,
                   WideString& sError);
  bool setTimeOut(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  WideString& sError);

  void TimerProc(GlobalTimer* pTimer);
  void CancelProc(GlobalTimer* pTimer);

  static WideString SysPathToPDFPath(const WideString& sOldPath);

 private:
  // CJS_EmbedObj
  void RunJsScript(CJS_Runtime* pRuntime, const WideString& wsScript);

  void ClearTimerCommon(CJS_Runtime* pRuntime, const CJS_Value& param);

  bool m_bCalculate;
  bool m_bRuntimeHighLight;
  std::set<std::unique_ptr<GlobalTimer>> m_Timers;
};

class CJS_App : public CJS_Object {
 public:
  explicit CJS_App(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_App() override {}

  DECLARE_JS_CLASS();

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
};

#endif  // FPDFSDK_JAVASCRIPT_APP_H_
