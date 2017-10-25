// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/app.h"

#include <map>
#include <memory>
#include <vector>

#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/javascript/Document.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"
#include "third_party/base/stl_util.h"

namespace {

bool IsTypeKnown(v8::Local<v8::Value> value) {
  return !value.IsEmpty() &&
         (value->IsString() || value->IsNumber() || value->IsBoolean() ||
          value->IsDate() || value->IsObject() || value->IsNull() ||
          value->IsUndefined());
}

}  // namespace

class GlobalTimer {
 public:
  GlobalTimer(app* pObj,
              CPDFSDK_FormFillEnvironment* pFormFillEnv,
              CJS_Runtime* pRuntime,
              int nType,
              const WideString& script,
              uint32_t dwElapse,
              uint32_t dwTimeOut);
  ~GlobalTimer();

  static void Trigger(int nTimerID);
  static void Cancel(int nTimerID);

  bool IsOneShot() const { return m_nType == 1; }
  uint32_t GetTimeOut() const { return m_dwTimeOut; }
  int GetTimerID() const { return m_nTimerID; }
  CJS_Runtime* GetRuntime() const { return m_pRuntime.Get(); }
  WideString GetJScript() const { return m_swJScript; }

 private:
  using TimerMap = std::map<uint32_t, GlobalTimer*>;
  static TimerMap* GetGlobalTimerMap();

  uint32_t m_nTimerID;
  app* const m_pEmbedObj;
  bool m_bProcessing;

  // data
  const int m_nType;  // 0:Interval; 1:TimeOut
  const uint32_t m_dwTimeOut;
  const WideString m_swJScript;
  CJS_Runtime::ObservedPtr m_pRuntime;
  CPDFSDK_FormFillEnvironment::ObservedPtr m_pFormFillEnv;
};

GlobalTimer::GlobalTimer(app* pObj,
                         CPDFSDK_FormFillEnvironment* pFormFillEnv,
                         CJS_Runtime* pRuntime,
                         int nType,
                         const WideString& script,
                         uint32_t dwElapse,
                         uint32_t dwTimeOut)
    : m_nTimerID(0),
      m_pEmbedObj(pObj),
      m_bProcessing(false),
      m_nType(nType),
      m_dwTimeOut(dwTimeOut),
      m_swJScript(script),
      m_pRuntime(pRuntime),
      m_pFormFillEnv(pFormFillEnv) {
  CFX_SystemHandler* pHandler = m_pFormFillEnv->GetSysHandler();
  m_nTimerID = pHandler->SetTimer(dwElapse, Trigger);
  if (m_nTimerID)
    (*GetGlobalTimerMap())[m_nTimerID] = this;
}

GlobalTimer::~GlobalTimer() {
  if (!m_nTimerID)
    return;

  if (GetRuntime())
    m_pFormFillEnv->GetSysHandler()->KillTimer(m_nTimerID);

  GetGlobalTimerMap()->erase(m_nTimerID);
}

// static
void GlobalTimer::Trigger(int nTimerID) {
  auto it = GetGlobalTimerMap()->find(nTimerID);
  if (it == GetGlobalTimerMap()->end())
    return;

  GlobalTimer* pTimer = it->second;
  if (pTimer->m_bProcessing)
    return;

  pTimer->m_bProcessing = true;
  if (pTimer->m_pEmbedObj)
    pTimer->m_pEmbedObj->TimerProc(pTimer);

  // Timer proc may have destroyed timer, find it again.
  it = GetGlobalTimerMap()->find(nTimerID);
  if (it == GetGlobalTimerMap()->end())
    return;

  pTimer = it->second;
  pTimer->m_bProcessing = false;
  if (pTimer->IsOneShot())
    pTimer->m_pEmbedObj->CancelProc(pTimer);
}

// static
void GlobalTimer::Cancel(int nTimerID) {
  auto it = GetGlobalTimerMap()->find(nTimerID);
  if (it == GetGlobalTimerMap()->end())
    return;

  GlobalTimer* pTimer = it->second;
  pTimer->m_pEmbedObj->CancelProc(pTimer);
}

// static
GlobalTimer::TimerMap* GlobalTimer::GetGlobalTimerMap() {
  // Leak the timer array at shutdown.
  static auto* s_TimerMap = new TimerMap;
  return s_TimerMap;
}

JSConstSpec CJS_TimerObj::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_TimerObj::PropertySpecs[] = {{0, 0, 0}};

JSMethodSpec CJS_TimerObj::MethodSpecs[] = {{0, 0}};

IMPLEMENT_JS_CLASS(CJS_TimerObj, TimerObj, TimerObj)

TimerObj::TimerObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_nTimerID(0) {}

TimerObj::~TimerObj() {}

void TimerObj::SetTimer(GlobalTimer* pTimer) {
  m_nTimerID = pTimer->GetTimerID();
}

#define JS_STR_VIEWERTYPE L"pdfium"
#define JS_STR_VIEWERVARIATION L"Full"
#define JS_STR_PLATFORM L"WIN"
#define JS_STR_LANGUAGE L"ENU"
#define JS_NUM_VIEWERVERSION 8
#ifdef PDF_ENABLE_XFA
#define JS_NUM_VIEWERVERSION_XFA 11
#endif  // PDF_ENABLE_XFA
#define JS_NUM_FORMSVERSION 7

JSConstSpec CJS_App::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_App::PropertySpecs[] = {
    {"activeDocs", get_active_docs_static, set_active_docs_static},
    {"calculate", get_calculate_static, set_calculate_static},
    {"formsVersion", get_forms_version_static, set_forms_version_static},
    {"fs", get_fs_static, set_fs_static},
    {"fullscreen", get_fullscreen_static, set_fullscreen_static},
    {"language", get_language_static, set_language_static},
    {"media", get_media_static, set_media_static},
    {"platform", get_platform_static, set_platform_static},
    {"runtimeHighlight", get_runtime_highlight_static,
     set_runtime_highlight_static},
    {"viewerType", get_viewer_type_static, set_viewer_type_static},
    {"viewerVariation", get_viewer_variation_static,
     set_viewer_variation_static},
    {"viewerVersion", get_viewer_version_static, set_viewer_version_static},
    {0, 0, 0}};

JSMethodSpec CJS_App::MethodSpecs[] = {{"alert", alert_static},
                                       {"beep", beep_static},
                                       {"browseForDoc", browseForDoc_static},
                                       {"clearInterval", clearInterval_static},
                                       {"clearTimeOut", clearTimeOut_static},
                                       {"execDialog", execDialog_static},
                                       {"execMenuItem", execMenuItem_static},
                                       {"findComponent", findComponent_static},
                                       {"goBack", goBack_static},
                                       {"goForward", goForward_static},
                                       {"launchURL", launchURL_static},
                                       {"mailMsg", mailMsg_static},
                                       {"newFDF", newFDF_static},
                                       {"newDoc", newDoc_static},
                                       {"openDoc", openDoc_static},
                                       {"openFDF", openFDF_static},
                                       {"popUpMenuEx", popUpMenuEx_static},
                                       {"popUpMenu", popUpMenu_static},
                                       {"response", response_static},
                                       {"setInterval", setInterval_static},
                                       {"setTimeOut", setTimeOut_static},
                                       {0, 0}};

IMPLEMENT_JS_CLASS(CJS_App, app, app)

app::app(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_bCalculate(true), m_bRuntimeHighLight(false) {}

app::~app() {
}

bool app::get_active_docs(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  CJS_Document* pJSDocument = nullptr;
  v8::Local<v8::Object> pObj = pRuntime->GetThisObj();
  if (CFXJS_Engine::GetObjDefnID(pObj) == CJS_Document::g_nObjDefnID)
    pJSDocument = static_cast<CJS_Document*>(pRuntime->GetObjectPrivate(pObj));

  CJS_Array aDocs;
  aDocs.SetElement(pRuntime, 0,
                   pJSDocument ? v8::Local<v8::Value>(pJSDocument->ToV8Object())
                               : v8::Local<v8::Value>());
  if (aDocs.GetLength(pRuntime) > 0) {
    if (aDocs.ToV8Value().IsEmpty())
      vp->Set(pRuntime->NewArray());
    else
      vp->Set(aDocs.ToV8Value());
  } else {
    vp->Set(pRuntime->NewNull());
  }

  return true;
}

bool app::set_active_docs(CJS_Runtime* pRuntime,
                          v8::Local<v8::Value> vp,
                          WideString* sError) {
  return false;
}

bool app::get_calculate(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  vp->Set(pRuntime->NewBoolean(m_bCalculate));
  return true;
}

bool app::set_calculate(CJS_Runtime* pRuntime,
                        v8::Local<v8::Value> vp,
                        WideString* sError) {
  m_bCalculate = pRuntime->ToBoolean(vp);
  pRuntime->GetFormFillEnv()->GetInterForm()->EnableCalculate(m_bCalculate);
  return true;
}

bool app::get_forms_version(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  vp->Set(pRuntime->NewNumber(JS_NUM_FORMSVERSION));
  return true;
}

bool app::set_forms_version(CJS_Runtime* pRuntime,
                            v8::Local<v8::Value> vp,
                            WideString* sError) {
  return false;
}

bool app::get_viewer_type(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  vp->Set(pRuntime->NewString(JS_STR_VIEWERTYPE));
  return true;
}

bool app::set_viewer_type(CJS_Runtime* pRuntime,
                          v8::Local<v8::Value> vp,
                          WideString* sError) {
  return false;
}

bool app::get_viewer_variation(CJS_Runtime* pRuntime,
                               CJS_Value* vp,
                               WideString* sError) {
  vp->Set(pRuntime->NewString(JS_STR_VIEWERVARIATION));
  return true;
}

bool app::set_viewer_variation(CJS_Runtime* pRuntime,
                               v8::Local<v8::Value> vp,
                               WideString* sError) {
  return false;
}

bool app::get_viewer_version(CJS_Runtime* pRuntime,
                             CJS_Value* vp,
                             WideString* sError) {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Context* pXFAContext = pRuntime->GetFormFillEnv()->GetXFAContext();
  if (pXFAContext->ContainsXFAForm()) {
    vp->Set(pRuntime->NewNumber(JS_NUM_VIEWERVERSION_XFA));
    return true;
  }
#endif  // PDF_ENABLE_XFA
  vp->Set(pRuntime->NewNumber(JS_NUM_VIEWERVERSION));
  return true;
}

bool app::set_viewer_version(CJS_Runtime* pRuntime,
                             v8::Local<v8::Value> vp,
                             WideString* sError) {
  return false;
}

bool app::get_platform(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
#ifdef PDF_ENABLE_XFA
  CPDFSDK_FormFillEnvironment* pFormFillEnv = pRuntime->GetFormFillEnv();
  if (!pFormFillEnv)
    return false;

  WideString platfrom = pFormFillEnv->GetPlatform();
  if (!platfrom.IsEmpty()) {
    vp->Set(pRuntime->NewString(platfrom.c_str()));
    return true;
  }
#endif
  vp->Set(pRuntime->NewString(JS_STR_PLATFORM));
  return true;
}

bool app::set_platform(CJS_Runtime* pRuntime,
                       v8::Local<v8::Value> vp,
                       WideString* sError) {
  return false;
}

bool app::get_language(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
#ifdef PDF_ENABLE_XFA
  CPDFSDK_FormFillEnvironment* pFormFillEnv = pRuntime->GetFormFillEnv();
  if (!pFormFillEnv)
    return false;

  WideString language = pFormFillEnv->GetLanguage();
  if (!language.IsEmpty()) {
    vp->Set(pRuntime->NewString(language.c_str()));
    return true;
  }
#endif
  vp->Set(pRuntime->NewString(JS_STR_LANGUAGE));
  return true;
}

bool app::set_language(CJS_Runtime* pRuntime,
                       v8::Local<v8::Value> vp,
                       WideString* sError) {
  return false;
}

// creates a new fdf object that contains no data
// comment: need reader support
// note:
// CFDF_Document * CPDFSDK_FormFillEnvironment::NewFDF();
bool app::newFDF(CJS_Runtime* pRuntime,
                 const std::vector<v8::Local<v8::Value>>& params,
                 CJS_Value& vRet,
                 WideString& sError) {
  return true;
}
// opens a specified pdf document and returns its document object
// comment:need reader support
// note: as defined in js reference, the proto of this function's fourth
// parmeters, how old an fdf document while do not show it.
// CFDF_Document * CPDFSDK_FormFillEnvironment::OpenFDF(string strPath,bool
// bUserConv);

bool app::openFDF(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params,
                  CJS_Value& vRet,
                  WideString& sError) {
  return true;
}

bool app::alert(CJS_Runtime* pRuntime,
                const std::vector<v8::Local<v8::Value>>& params,
                CJS_Value& vRet,
                WideString& sError) {
  std::vector<v8::Local<v8::Value>> newParams = ExpandKeywordParams(
      pRuntime, params, 4, L"cMsg", L"nIcon", L"nType", L"cTitle");

  if (!IsTypeKnown(newParams[0])) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  CPDFSDK_FormFillEnvironment* pFormFillEnv = pRuntime->GetFormFillEnv();
  if (!pFormFillEnv) {
    vRet = CJS_Value(pRuntime->NewNumber(0));
    return true;
  }

  WideString swMsg;
  if (newParams[0]->IsArray()) {
    CJS_Array carray(pRuntime->ToArray(newParams[0]));
    swMsg = L"[";
    for (int i = 0; i < carray.GetLength(pRuntime); ++i) {
      if (i)
        swMsg += L", ";

      swMsg += pRuntime->ToWideString(carray.GetElement(pRuntime, i));
    }
    swMsg += L"]";
  } else {
    swMsg = pRuntime->ToWideString(newParams[0]);
  }

  int iIcon = 0;
  if (IsTypeKnown(newParams[1]))
    iIcon = pRuntime->ToInt32(newParams[1]);

  int iType = 0;
  if (IsTypeKnown(newParams[2]))
    iType = pRuntime->ToInt32(newParams[2]);

  WideString swTitle;
  if (IsTypeKnown(newParams[3]))
    swTitle = pRuntime->ToWideString(newParams[3]);
  else
    swTitle = JSGetStringFromID(IDS_STRING_JSALERT);

  pRuntime->BeginBlock();
  pFormFillEnv->KillFocusAnnot(0);

  vRet = CJS_Value(pRuntime->NewNumber(
      pFormFillEnv->JS_appAlert(swMsg.c_str(), swTitle.c_str(), iType, iIcon)));
  pRuntime->EndBlock();
  return true;
}

bool app::beep(CJS_Runtime* pRuntime,
               const std::vector<v8::Local<v8::Value>>& params,
               CJS_Value& vRet,
               WideString& sError) {
  if (params.size() == 1) {
    pRuntime->GetFormFillEnv()->JS_appBeep(pRuntime->ToInt32(params[0]));
    return true;
  }

  sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
  return false;
}

bool app::findComponent(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params,
                        CJS_Value& vRet,
                        WideString& sError) {
  return true;
}

bool app::popUpMenuEx(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params,
                      CJS_Value& vRet,
                      WideString& sError) {
  return false;
}

bool app::get_fs(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  return false;
}

bool app::set_fs(CJS_Runtime* pRuntime,
                 v8::Local<v8::Value> vp,
                 WideString* sError) {
  return false;
}

bool app::setInterval(CJS_Runtime* pRuntime,
                      const std::vector<v8::Local<v8::Value>>& params,
                      CJS_Value& vRet,
                      WideString& sError) {
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  WideString script =
      params.size() > 0 ? pRuntime->ToWideString(params[0]) : L"";
  if (script.IsEmpty()) {
    sError = JSGetStringFromID(IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return true;
  }

  uint32_t dwInterval = params.size() > 1 ? pRuntime->ToInt32(params[1]) : 1000;

  GlobalTimer* timerRef = new GlobalTimer(this, pRuntime->GetFormFillEnv(),
                                          pRuntime, 0, script, dwInterval, 0);
  m_Timers.insert(std::unique_ptr<GlobalTimer>(timerRef));

  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_TimerObj::g_nObjDefnID);
  if (pRetObj.IsEmpty())
    return false;

  CJS_TimerObj* pJS_TimerObj =
      static_cast<CJS_TimerObj*>(pRuntime->GetObjectPrivate(pRetObj));
  TimerObj* pTimerObj = static_cast<TimerObj*>(pJS_TimerObj->GetEmbedObject());
  pTimerObj->SetTimer(timerRef);

  vRet = CJS_Value(pRetObj);
  return true;
}

bool app::setTimeOut(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params,
                     CJS_Value& vRet,
                     WideString& sError) {
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  WideString script = pRuntime->ToWideString(params[0]);
  if (script.IsEmpty()) {
    sError = JSGetStringFromID(IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return true;
  }

  uint32_t dwTimeOut = params.size() > 1 ? pRuntime->ToInt32(params[1]) : 1000;
  GlobalTimer* timerRef =
      new GlobalTimer(this, pRuntime->GetFormFillEnv(), pRuntime, 1, script,
                      dwTimeOut, dwTimeOut);
  m_Timers.insert(std::unique_ptr<GlobalTimer>(timerRef));

  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_TimerObj::g_nObjDefnID);
  if (pRetObj.IsEmpty())
    return false;

  CJS_TimerObj* pJS_TimerObj =
      static_cast<CJS_TimerObj*>(pRuntime->GetObjectPrivate(pRetObj));
  TimerObj* pTimerObj = static_cast<TimerObj*>(pJS_TimerObj->GetEmbedObject());
  pTimerObj->SetTimer(timerRef);
  vRet = CJS_Value(pRetObj);
  return true;
}

bool app::clearTimeOut(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params,
                       CJS_Value& vRet,
                       WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  app::ClearTimerCommon(pRuntime, params[0]);
  return true;
}

bool app::clearInterval(CJS_Runtime* pRuntime,
                        const std::vector<v8::Local<v8::Value>>& params,
                        CJS_Value& vRet,
                        WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }

  app::ClearTimerCommon(pRuntime, params[0]);
  return true;
}

void app::ClearTimerCommon(CJS_Runtime* pRuntime, v8::Local<v8::Value> param) {
  if (!param->IsObject())
    return;

  v8::Local<v8::Object> pObj = pRuntime->ToObject(param);
  if (CFXJS_Engine::GetObjDefnID(pObj) != CJS_TimerObj::g_nObjDefnID)
    return;

  CJS_Object* pJSObj =
      static_cast<CJS_Object*>(pRuntime->GetObjectPrivate(pObj));
  if (!pJSObj)
    return;

  TimerObj* pTimerObj = static_cast<TimerObj*>(pJSObj->GetEmbedObject());
  if (!pTimerObj)
    return;

  GlobalTimer::Cancel(pTimerObj->GetTimerID());
}

bool app::execMenuItem(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params,
                       CJS_Value& vRet,
                       WideString& sError) {
  return false;
}

void app::TimerProc(GlobalTimer* pTimer) {
  CJS_Runtime* pRuntime = pTimer->GetRuntime();
  if (pRuntime && (!pTimer->IsOneShot() || pTimer->GetTimeOut() > 0))
    RunJsScript(pRuntime, pTimer->GetJScript());
}

void app::CancelProc(GlobalTimer* pTimer) {
  m_Timers.erase(pdfium::FakeUniquePtr<GlobalTimer>(pTimer));
}

void app::RunJsScript(CJS_Runtime* pRuntime, const WideString& wsScript) {
  if (!pRuntime->IsBlocking()) {
    IJS_EventContext* pContext = pRuntime->NewEventContext();
    pContext->OnExternal_Exec();
    WideString wtInfo;
    pContext->RunScript(wsScript, &wtInfo);
    pRuntime->ReleaseEventContext(pContext);
  }
}

bool app::goBack(CJS_Runtime* pRuntime,
                 const std::vector<v8::Local<v8::Value>>& params,
                 CJS_Value& vRet,
                 WideString& sError) {
  // Not supported.
  return true;
}

bool app::goForward(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  // Not supported.
  return true;
}

bool app::mailMsg(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params,
                  CJS_Value& vRet,
                  WideString& sError) {
  std::vector<v8::Local<v8::Value>> newParams =
      ExpandKeywordParams(pRuntime, params, 6, L"bUI", L"cTo", L"cCc", L"cBcc",
                          L"cSubject", L"cMsg");

  if (!IsTypeKnown(newParams[0])) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }
  bool bUI = pRuntime->ToBoolean(newParams[0]);

  WideString cTo;
  if (IsTypeKnown(newParams[1])) {
    cTo = pRuntime->ToWideString(newParams[1]);
  } else {
    if (!bUI) {
      // cTo parameter required when UI not invoked.
      sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
      return false;
    }
  }

  WideString cCc;
  if (IsTypeKnown(newParams[2]))
    cCc = pRuntime->ToWideString(newParams[2]);

  WideString cBcc;
  if (IsTypeKnown(newParams[3]))
    cBcc = pRuntime->ToWideString(newParams[3]);

  WideString cSubject;
  if (IsTypeKnown(newParams[4]))
    cSubject = pRuntime->ToWideString(newParams[4]);

  WideString cMsg;
  if (IsTypeKnown(newParams[5]))
    cMsg = pRuntime->ToWideString(newParams[5]);

  pRuntime->BeginBlock();
  pRuntime->GetFormFillEnv()->JS_docmailForm(nullptr, 0, bUI, cTo.c_str(),
                                             cSubject.c_str(), cCc.c_str(),
                                             cBcc.c_str(), cMsg.c_str());
  pRuntime->EndBlock();
  return true;
}

bool app::launchURL(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  // Unsafe, not supported.
  return true;
}

bool app::get_runtime_highlight(CJS_Runtime* pRuntime,
                                CJS_Value* vp,
                                WideString* sError) {
  vp->Set(pRuntime->NewBoolean(m_bRuntimeHighLight));
  return true;
}

bool app::set_runtime_highlight(CJS_Runtime* pRuntime,
                                v8::Local<v8::Value> vp,
                                WideString* sError) {
  m_bRuntimeHighLight = pRuntime->ToBoolean(vp);
  return true;
}

bool app::get_fullscreen(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  return false;
}

bool app::set_fullscreen(CJS_Runtime* pRuntime,
                         v8::Local<v8::Value> vp,
                         WideString* sError) {
  return false;
}

bool app::popUpMenu(CJS_Runtime* pRuntime,
                    const std::vector<v8::Local<v8::Value>>& params,
                    CJS_Value& vRet,
                    WideString& sError) {
  return false;
}

bool app::browseForDoc(CJS_Runtime* pRuntime,
                       const std::vector<v8::Local<v8::Value>>& params,
                       CJS_Value& vRet,
                       WideString& sError) {
  // Unsafe, not supported.
  return true;
}

WideString app::SysPathToPDFPath(const WideString& sOldPath) {
  WideString sRet = L"/";
  for (const wchar_t& c : sOldPath) {
    if (c != L':')
      sRet += (c == L'\\') ? L'/' : c;
  }
  return sRet;
}

bool app::newDoc(CJS_Runtime* pRuntime,
                 const std::vector<v8::Local<v8::Value>>& params,
                 CJS_Value& vRet,
                 WideString& sError) {
  return false;
}

bool app::openDoc(CJS_Runtime* pRuntime,
                  const std::vector<v8::Local<v8::Value>>& params,
                  CJS_Value& vRet,
                  WideString& sError) {
  return false;
}

bool app::response(CJS_Runtime* pRuntime,
                   const std::vector<v8::Local<v8::Value>>& params,
                   CJS_Value& vRet,
                   WideString& sError) {
  std::vector<v8::Local<v8::Value>> newParams =
      ExpandKeywordParams(pRuntime, params, 5, L"cQuestion", L"cTitle",
                          L"cDefault", L"bPassword", L"cLabel");

  if (!IsTypeKnown(newParams[0])) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return false;
  }
  WideString swQuestion = pRuntime->ToWideString(newParams[0]);

  WideString swTitle = L"PDF";
  if (IsTypeKnown(newParams[1]))
    swTitle = pRuntime->ToWideString(newParams[1]);

  WideString swDefault;
  if (IsTypeKnown(newParams[2]))
    swDefault = pRuntime->ToWideString(newParams[2]);

  bool bPassword = false;
  if (IsTypeKnown(newParams[3]))
    bPassword = pRuntime->ToBoolean(newParams[3]);

  WideString swLabel;
  if (IsTypeKnown(newParams[4]))
    swLabel = pRuntime->ToWideString(newParams[4]);

  const int MAX_INPUT_BYTES = 2048;
  std::vector<uint8_t> pBuff(MAX_INPUT_BYTES + 2);
  int nLengthBytes = pRuntime->GetFormFillEnv()->JS_appResponse(
      swQuestion.c_str(), swTitle.c_str(), swDefault.c_str(), swLabel.c_str(),
      bPassword, pBuff.data(), MAX_INPUT_BYTES);

  if (nLengthBytes < 0 || nLengthBytes > MAX_INPUT_BYTES) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAM_TOOLONG);
    return false;
  }

  vRet = CJS_Value(pRuntime->NewString(
      WideString::FromUTF16LE(reinterpret_cast<uint16_t*>(pBuff.data()),
                              nLengthBytes / sizeof(uint16_t))
          .c_str()));

  return true;
}

bool app::get_media(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  return false;
}

bool app::set_media(CJS_Runtime* pRuntime,
                    v8::Local<v8::Value> vp,
                    WideString* sError) {
  return false;
}

bool app::execDialog(CJS_Runtime* pRuntime,
                     const std::vector<v8::Local<v8::Value>>& params,
                     CJS_Value& vRet,
                     WideString& sError) {
  return true;
}
