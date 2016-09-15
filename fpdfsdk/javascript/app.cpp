// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/app.h"

#include <memory>
#include <vector>

#include "fpdfsdk/include/cpdfsdk_document.h"
#include "fpdfsdk/include/cpdfsdk_environment.h"
#include "fpdfsdk/include/cpdfsdk_interform.h"
#include "fpdfsdk/javascript/Document.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_context.h"
#include "fpdfsdk/javascript/cjs_runtime.h"
#include "fpdfsdk/javascript/resource.h"
#include "third_party/base/stl_util.h"

class GlobalTimer {
 public:
  GlobalTimer(app* pObj,
              CPDFSDK_Environment* pApp,
              CJS_Runtime* pRuntime,
              int nType,
              const CFX_WideString& script,
              uint32_t dwElapse,
              uint32_t dwTimeOut);
  ~GlobalTimer();

  static void Trigger(int nTimerID);
  static void Cancel(int nTimerID);

  bool IsOneShot() const { return m_nType == 1; }
  uint32_t GetTimeOut() const { return m_dwTimeOut; }
  int GetTimerID() const { return m_nTimerID; }
  CJS_Runtime* GetRuntime() const { return m_pRuntime.Get(); }
  CFX_WideString GetJScript() const { return m_swJScript; }

 private:
  using TimerMap = std::map<uint32_t, GlobalTimer*>;
  static TimerMap* GetGlobalTimerMap();

  uint32_t m_nTimerID;
  app* const m_pEmbedObj;
  bool m_bProcessing;

  // data
  const int m_nType;  // 0:Interval; 1:TimeOut
  const uint32_t m_dwTimeOut;
  const CFX_WideString m_swJScript;
  CJS_Runtime::ObservedPtr m_pRuntime;
  CPDFSDK_Environment* const m_pApp;
};

GlobalTimer::GlobalTimer(app* pObj,
                         CPDFSDK_Environment* pApp,
                         CJS_Runtime* pRuntime,
                         int nType,
                         const CFX_WideString& script,
                         uint32_t dwElapse,
                         uint32_t dwTimeOut)
    : m_nTimerID(0),
      m_pEmbedObj(pObj),
      m_bProcessing(false),
      m_nType(nType),
      m_dwTimeOut(dwTimeOut),
      m_swJScript(script),
      m_pRuntime(pRuntime),
      m_pApp(pApp) {
  CFX_SystemHandler* pHandler = m_pApp->GetSysHandler();
  m_nTimerID = pHandler->SetTimer(dwElapse, Trigger);
  (*GetGlobalTimerMap())[m_nTimerID] = this;
}

GlobalTimer::~GlobalTimer() {
  if (!m_nTimerID)
    return;

  if (GetRuntime())
    m_pApp->GetSysHandler()->KillTimer(m_nTimerID);

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

BEGIN_JS_STATIC_CONST(CJS_TimerObj)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_TimerObj)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_TimerObj)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_TimerObj, TimerObj)

TimerObj::TimerObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_nTimerID(0) {}

TimerObj::~TimerObj() {}

void TimerObj::SetTimer(GlobalTimer* pTimer) {
  m_nTimerID = pTimer->GetTimerID();
}

#define JS_STR_VIEWERTYPE L"pdfium"
#define JS_STR_VIEWERVARIATION L"Full"
#define JS_STR_PLATFORM L"WIN"
#define JS_STR_LANGUANGE L"ENU"
#define JS_NUM_VIEWERVERSION 8
#ifdef PDF_ENABLE_XFA
#define JS_NUM_VIEWERVERSION_XFA 11
#endif  // PDF_ENABLE_XFA
#define JS_NUM_FORMSVERSION 7

BEGIN_JS_STATIC_CONST(CJS_App)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_App)
JS_STATIC_PROP_ENTRY(activeDocs)
JS_STATIC_PROP_ENTRY(calculate)
JS_STATIC_PROP_ENTRY(formsVersion)
JS_STATIC_PROP_ENTRY(fs)
JS_STATIC_PROP_ENTRY(fullscreen)
JS_STATIC_PROP_ENTRY(language)
JS_STATIC_PROP_ENTRY(media)
JS_STATIC_PROP_ENTRY(platform)
JS_STATIC_PROP_ENTRY(runtimeHighlight)
JS_STATIC_PROP_ENTRY(viewerType)
JS_STATIC_PROP_ENTRY(viewerVariation)
JS_STATIC_PROP_ENTRY(viewerVersion)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_App)
JS_STATIC_METHOD_ENTRY(alert)
JS_STATIC_METHOD_ENTRY(beep)
JS_STATIC_METHOD_ENTRY(browseForDoc)
JS_STATIC_METHOD_ENTRY(clearInterval)
JS_STATIC_METHOD_ENTRY(clearTimeOut)
JS_STATIC_METHOD_ENTRY(execDialog)
JS_STATIC_METHOD_ENTRY(execMenuItem)
JS_STATIC_METHOD_ENTRY(findComponent)
JS_STATIC_METHOD_ENTRY(goBack)
JS_STATIC_METHOD_ENTRY(goForward)
JS_STATIC_METHOD_ENTRY(launchURL)
JS_STATIC_METHOD_ENTRY(mailMsg)
JS_STATIC_METHOD_ENTRY(newFDF)
JS_STATIC_METHOD_ENTRY(newDoc)
JS_STATIC_METHOD_ENTRY(openDoc)
JS_STATIC_METHOD_ENTRY(openFDF)
JS_STATIC_METHOD_ENTRY(popUpMenuEx)
JS_STATIC_METHOD_ENTRY(popUpMenu)
JS_STATIC_METHOD_ENTRY(response)
JS_STATIC_METHOD_ENTRY(setInterval)
JS_STATIC_METHOD_ENTRY(setTimeOut)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_App, app)

app::app(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_bCalculate(true), m_bRuntimeHighLight(false) {}

app::~app() {
}

FX_BOOL app::activeDocs(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (!vp.IsGetting())
    return FALSE;

  CJS_Context* pContext = (CJS_Context*)cc;
  CPDFSDK_Environment* pApp = pContext->GetReaderApp();
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CPDFSDK_Document* pCurDoc = pContext->GetReaderDocument();
  CJS_Array aDocs;
  if (CPDFSDK_Document* pDoc = pApp->GetSDKDocument()) {
    CJS_Document* pJSDocument = nullptr;
    if (pDoc == pCurDoc) {
      v8::Local<v8::Object> pObj = pRuntime->GetThisObj();
      if (CFXJS_Engine::GetObjDefnID(pObj) == CJS_Document::g_nObjDefnID) {
        pJSDocument =
            static_cast<CJS_Document*>(pRuntime->GetObjectPrivate(pObj));
      }
    } else {
      v8::Local<v8::Object> pObj =
          pRuntime->NewFxDynamicObj(CJS_Document::g_nObjDefnID);
      pJSDocument =
          static_cast<CJS_Document*>(pRuntime->GetObjectPrivate(pObj));
      ASSERT(pJSDocument);
    }
    aDocs.SetElement(pRuntime, 0, CJS_Value(pRuntime, pJSDocument));
  }
  if (aDocs.GetLength(pRuntime) > 0)
    vp << aDocs;
  else
    vp.GetJSValue()->SetNull(pRuntime);

  return TRUE;
}

FX_BOOL app::calculate(IJS_Context* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (vp.IsSetting()) {
    bool bVP;
    vp >> bVP;
    m_bCalculate = (FX_BOOL)bVP;

    CJS_Context* pContext = (CJS_Context*)cc;
    CPDFSDK_Environment* pApp = pContext->GetReaderApp();
    if (CPDFSDK_Document* pDoc = pApp->GetSDKDocument())
      pDoc->GetInterForm()->EnableCalculate((FX_BOOL)m_bCalculate);
  } else {
    vp << (bool)m_bCalculate;
  }
  return TRUE;
}

FX_BOOL app::formsVersion(IJS_Context* cc,
                          CJS_PropValue& vp,
                          CFX_WideString& sError) {
  if (vp.IsGetting()) {
    vp << JS_NUM_FORMSVERSION;
    return TRUE;
  }

  return FALSE;
}

FX_BOOL app::viewerType(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (vp.IsGetting()) {
    vp << JS_STR_VIEWERTYPE;
    return TRUE;
  }

  return FALSE;
}

FX_BOOL app::viewerVariation(IJS_Context* cc,
                             CJS_PropValue& vp,
                             CFX_WideString& sError) {
  if (vp.IsGetting()) {
    vp << JS_STR_VIEWERVARIATION;
    return TRUE;
  }

  return FALSE;
}

FX_BOOL app::viewerVersion(IJS_Context* cc,
                           CJS_PropValue& vp,
                           CFX_WideString& sError) {
  if (!vp.IsGetting())
    return FALSE;
#ifdef PDF_ENABLE_XFA
  CJS_Context* pContext = (CJS_Context*)cc;
  CPDFSDK_Document* pCurDoc = pContext->GetReaderDocument();
  CPDFXFA_Document* pDoc = pCurDoc->GetXFADocument();
  if (pDoc->GetDocType() == 1 || pDoc->GetDocType() == 2) {
    vp << JS_NUM_VIEWERVERSION_XFA;
    return TRUE;
  }
#endif  // PDF_ENABLE_XFA
  vp << JS_NUM_VIEWERVERSION;
  return TRUE;
}

FX_BOOL app::platform(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (!vp.IsGetting())
    return FALSE;
#ifdef PDF_ENABLE_XFA
  CPDFSDK_Environment* pEnv =
      static_cast<CJS_Context*>(cc)->GetJSRuntime()->GetReaderApp();
  if (!pEnv)
    return FALSE;
  CFX_WideString platfrom = pEnv->GetPlatform();
  if (!platfrom.IsEmpty()) {
    vp << platfrom;
    return TRUE;
  }
#endif
  vp << JS_STR_PLATFORM;
  return TRUE;
}

FX_BOOL app::language(IJS_Context* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (!vp.IsGetting())
    return FALSE;
#ifdef PDF_ENABLE_XFA
  CPDFSDK_Environment* pEnv =
      static_cast<CJS_Context*>(cc)->GetJSRuntime()->GetReaderApp();
  if (!pEnv)
    return FALSE;
  CFX_WideString language = pEnv->GetLanguage();
  if (!language.IsEmpty()) {
    vp << language;
    return TRUE;
  }
#endif
  vp << JS_STR_LANGUANGE;
  return TRUE;
}

// creates a new fdf object that contains no data
// comment: need reader support
// note:
// CFDF_Document * CPDFSDK_Environment::NewFDF();
FX_BOOL app::newFDF(IJS_Context* cc,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    CFX_WideString& sError) {
  return TRUE;
}
// opens a specified pdf document and returns its document object
// comment:need reader support
// note: as defined in js reference, the proto of this function's fourth
// parmeters, how old an fdf document while do not show it.
// CFDF_Document * CPDFSDK_Environment::OpenFDF(string strPath,bool bUserConv);

FX_BOOL app::openFDF(IJS_Context* cc,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL app::alert(IJS_Context* cc,
                   const std::vector<CJS_Value>& params,
                   CJS_Value& vRet,
                   CFX_WideString& sError) {
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  std::vector<CJS_Value> newParams = JS_ExpandKeywordParams(
      pRuntime, params, 4, L"cMsg", L"nIcon", L"nType", L"cTitle");

  if (newParams[0].GetType() == CJS_Value::VT_unknown) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CPDFSDK_Environment* pApp = pRuntime->GetReaderApp();
  if (!pApp) {
    vRet = CJS_Value(pRuntime, 0);
    return TRUE;
  }

  CFX_WideString swMsg;
  if (newParams[0].GetType() == CJS_Value::VT_object) {
    CJS_Array carray;
    if (newParams[0].ConvertToArray(pRuntime, carray)) {
      swMsg = L"[";
      CJS_Value element(pRuntime);
      for (int i = 0; i < carray.GetLength(pRuntime); ++i) {
        if (i)
          swMsg += L", ";
        carray.GetElement(pRuntime, i, element);
        swMsg += element.ToCFXWideString(pRuntime);
      }
      swMsg += L"]";
    } else {
      swMsg = newParams[0].ToCFXWideString(pRuntime);
    }
  } else {
    swMsg = newParams[0].ToCFXWideString(pRuntime);
  }

  int iIcon = 0;
  if (newParams[1].GetType() != CJS_Value::VT_unknown)
    iIcon = newParams[1].ToInt(pRuntime);

  int iType = 0;
  if (newParams[2].GetType() != CJS_Value::VT_unknown)
    iType = newParams[2].ToInt(pRuntime);

  CFX_WideString swTitle;
  if (newParams[3].GetType() != CJS_Value::VT_unknown)
    swTitle = newParams[3].ToCFXWideString(pRuntime);
  else
    swTitle = JSGetStringFromID(IDS_STRING_JSALERT);

  pRuntime->BeginBlock();
  if (CPDFSDK_Document* pDoc = pApp->GetSDKDocument())
    pDoc->KillFocusAnnot();

  vRet = CJS_Value(pRuntime, pApp->JS_appAlert(swMsg.c_str(), swTitle.c_str(),
                                               iType, iIcon));
  pRuntime->EndBlock();
  return TRUE;
}

FX_BOOL app::beep(IJS_Context* cc,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  CFX_WideString& sError) {
  if (params.size() == 1) {
    CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
    CPDFSDK_Environment* pEnv = pRuntime->GetReaderApp();
    pEnv->JS_appBeep(params[0].ToInt(pRuntime));
    return TRUE;
  }

  sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
  return FALSE;
}

FX_BOOL app::findComponent(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  return TRUE;
}

FX_BOOL app::popUpMenuEx(IJS_Context* cc,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::fs(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::setInterval(IJS_Context* cc,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         CFX_WideString& sError) {
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  CFX_WideString script =
      params.size() > 0 ? params[0].ToCFXWideString(pRuntime) : L"";
  if (script.IsEmpty()) {
    sError = JSGetStringFromID(IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return TRUE;
  }

  uint32_t dwInterval = params.size() > 1 ? params[1].ToInt(pRuntime) : 1000;
  CPDFSDK_Environment* pApp = pRuntime->GetReaderApp();

  GlobalTimer* timerRef =
      new GlobalTimer(this, pApp, pRuntime, 0, script, dwInterval, 0);
  m_Timers.insert(std::unique_ptr<GlobalTimer>(timerRef));

  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_TimerObj::g_nObjDefnID);
  CJS_TimerObj* pJS_TimerObj =
      static_cast<CJS_TimerObj*>(pRuntime->GetObjectPrivate(pRetObj));
  TimerObj* pTimerObj = static_cast<TimerObj*>(pJS_TimerObj->GetEmbedObject());
  pTimerObj->SetTimer(timerRef);

  vRet = CJS_Value(pRuntime, pRetObj);
  return TRUE;
}

FX_BOOL app::setTimeOut(IJS_Context* cc,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        CFX_WideString& sError) {
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  CFX_WideString script = params[0].ToCFXWideString(pRuntime);
  if (script.IsEmpty()) {
    sError = JSGetStringFromID(IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return TRUE;
  }

  uint32_t dwTimeOut = params.size() > 1 ? params[1].ToInt(pRuntime) : 1000;
  CPDFSDK_Environment* pApp = pRuntime->GetReaderApp();

  GlobalTimer* timerRef =
      new GlobalTimer(this, pApp, pRuntime, 1, script, dwTimeOut, dwTimeOut);
  m_Timers.insert(std::unique_ptr<GlobalTimer>(timerRef));

  v8::Local<v8::Object> pRetObj =
      pRuntime->NewFxDynamicObj(CJS_TimerObj::g_nObjDefnID);

  CJS_TimerObj* pJS_TimerObj =
      static_cast<CJS_TimerObj*>(pRuntime->GetObjectPrivate(pRetObj));

  TimerObj* pTimerObj = static_cast<TimerObj*>(pJS_TimerObj->GetEmbedObject());
  pTimerObj->SetTimer(timerRef);

  vRet = CJS_Value(pRuntime, pRetObj);
  return TRUE;
}

FX_BOOL app::clearTimeOut(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  app::ClearTimerCommon(CJS_Runtime::FromContext(cc), params[0]);
  return TRUE;
}

FX_BOOL app::clearInterval(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  if (params.size() != 1) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  app::ClearTimerCommon(CJS_Runtime::FromContext(cc), params[0]);
  return TRUE;
}

void app::ClearTimerCommon(CJS_Runtime* pRuntime, const CJS_Value& param) {
  if (param.GetType() != CJS_Value::VT_object)
    return;

  v8::Local<v8::Object> pObj = param.ToV8Object(pRuntime);
  if (CFXJS_Engine::GetObjDefnID(pObj) != CJS_TimerObj::g_nObjDefnID)
    return;

  CJS_Object* pJSObj = param.ToCJSObject(pRuntime);
  if (!pJSObj)
    return;

  TimerObj* pTimerObj = static_cast<TimerObj*>(pJSObj->GetEmbedObject());
  if (!pTimerObj)
    return;

  GlobalTimer::Cancel(pTimerObj->GetTimerID());
}

FX_BOOL app::execMenuItem(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  return FALSE;
}

void app::TimerProc(GlobalTimer* pTimer) {
  CJS_Runtime* pRuntime = pTimer->GetRuntime();
  if (pRuntime && (!pTimer->IsOneShot() || pTimer->GetTimeOut() > 0))
    RunJsScript(pRuntime, pTimer->GetJScript());
}

void app::CancelProc(GlobalTimer* pTimer) {
  m_Timers.erase(pdfium::FakeUniquePtr<GlobalTimer>(pTimer));
}

void app::RunJsScript(CJS_Runtime* pRuntime, const CFX_WideString& wsScript) {
  if (!pRuntime->IsBlocking()) {
    IJS_Context* pContext = pRuntime->NewContext();
    pContext->OnExternal_Exec();
    CFX_WideString wtInfo;
    pContext->RunScript(wsScript, &wtInfo);
    pRuntime->ReleaseContext(pContext);
  }
}

FX_BOOL app::goBack(IJS_Context* cc,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    CFX_WideString& sError) {
  // Not supported.
  return TRUE;
}

FX_BOOL app::goForward(IJS_Context* cc,
                       const std::vector<CJS_Value>& params,
                       CJS_Value& vRet,
                       CFX_WideString& sError) {
  // Not supported.
  return TRUE;
}

FX_BOOL app::mailMsg(IJS_Context* cc,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     CFX_WideString& sError) {
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  std::vector<CJS_Value> newParams =
      JS_ExpandKeywordParams(pRuntime, params, 6, L"bUI", L"cTo", L"cCc",
                             L"cBcc", L"cSubject", L"cMsg");

  if (newParams[0].GetType() == CJS_Value::VT_unknown) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  bool bUI = newParams[0].ToBool(pRuntime);

  CFX_WideString cTo;
  if (newParams[1].GetType() != CJS_Value::VT_unknown) {
    cTo = newParams[1].ToCFXWideString(pRuntime);
  } else {
    if (!bUI) {
      // cTo parameter required when UI not invoked.
      sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
      return FALSE;
    }
  }

  CFX_WideString cCc;
  if (newParams[2].GetType() != CJS_Value::VT_unknown)
    cCc = newParams[2].ToCFXWideString(pRuntime);

  CFX_WideString cBcc;
  if (newParams[3].GetType() != CJS_Value::VT_unknown)
    cBcc = newParams[3].ToCFXWideString(pRuntime);

  CFX_WideString cSubject;
  if (newParams[4].GetType() != CJS_Value::VT_unknown)
    cSubject = newParams[4].ToCFXWideString(pRuntime);

  CFX_WideString cMsg;
  if (newParams[5].GetType() != CJS_Value::VT_unknown)
    cMsg = newParams[5].ToCFXWideString(pRuntime);

  pRuntime->BeginBlock();
  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  pContext->GetReaderApp()->JS_docmailForm(nullptr, 0, bUI, cTo.c_str(),
                                           cSubject.c_str(), cCc.c_str(),
                                           cBcc.c_str(), cMsg.c_str());
  pRuntime->EndBlock();
  return TRUE;
}

FX_BOOL app::launchURL(IJS_Context* cc,
                       const std::vector<CJS_Value>& params,
                       CJS_Value& vRet,
                       CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

FX_BOOL app::runtimeHighlight(IJS_Context* cc,
                              CJS_PropValue& vp,
                              CFX_WideString& sError) {
  if (vp.IsSetting()) {
    vp >> m_bRuntimeHighLight;
  } else {
    vp << m_bRuntimeHighLight;
  }
  return TRUE;
}

FX_BOOL app::fullscreen(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::popUpMenu(IJS_Context* cc,
                       const std::vector<CJS_Value>& params,
                       CJS_Value& vRet,
                       CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::browseForDoc(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  // Unsafe, not supported.
  return TRUE;
}

CFX_WideString app::SysPathToPDFPath(const CFX_WideString& sOldPath) {
  CFX_WideString sRet = L"/";

  for (int i = 0, sz = sOldPath.GetLength(); i < sz; i++) {
    wchar_t c = sOldPath.GetAt(i);
    if (c == L':') {
    } else {
      if (c == L'\\') {
        sRet += L"/";
      } else {
        sRet += c;
      }
    }
  }

  return sRet;
}

FX_BOOL app::newDoc(IJS_Context* cc,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::openDoc(IJS_Context* cc,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::response(IJS_Context* cc,
                      const std::vector<CJS_Value>& params,
                      CJS_Value& vRet,
                      CFX_WideString& sError) {
  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  std::vector<CJS_Value> newParams =
      JS_ExpandKeywordParams(pRuntime, params, 5, L"cQuestion", L"cTitle",
                             L"cDefault", L"bPassword", L"cLabel");

  if (newParams[0].GetType() == CJS_Value::VT_unknown) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAMERROR);
    return FALSE;
  }
  CFX_WideString swQuestion = newParams[0].ToCFXWideString(pRuntime);

  CFX_WideString swTitle = L"PDF";
  if (newParams[1].GetType() != CJS_Value::VT_unknown)
    swTitle = newParams[1].ToCFXWideString(pRuntime);

  CFX_WideString swDefault;
  if (newParams[2].GetType() != CJS_Value::VT_unknown)
    swDefault = newParams[2].ToCFXWideString(pRuntime);

  bool bPassword = false;
  if (newParams[3].GetType() != CJS_Value::VT_unknown)
    bPassword = newParams[3].ToBool(pRuntime);

  CFX_WideString swLabel;
  if (newParams[4].GetType() != CJS_Value::VT_unknown)
    swLabel = newParams[4].ToCFXWideString(pRuntime);

  const int MAX_INPUT_BYTES = 2048;
  std::unique_ptr<char[]> pBuff(new char[MAX_INPUT_BYTES + 2]);
  memset(pBuff.get(), 0, MAX_INPUT_BYTES + 2);

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  int nLengthBytes = pContext->GetReaderApp()->JS_appResponse(
      swQuestion.c_str(), swTitle.c_str(), swDefault.c_str(), swLabel.c_str(),
      bPassword, pBuff.get(), MAX_INPUT_BYTES);

  if (nLengthBytes < 0 || nLengthBytes > MAX_INPUT_BYTES) {
    sError = JSGetStringFromID(IDS_STRING_JSPARAM_TOOLONG);
    return FALSE;
  }

  vRet = CJS_Value(pRuntime, CFX_WideString::FromUTF16LE(
                                 reinterpret_cast<uint16_t*>(pBuff.get()),
                                 nLengthBytes / sizeof(uint16_t))
                                 .c_str());

  return TRUE;
}

FX_BOOL app::media(IJS_Context* cc, CJS_PropValue& vp, CFX_WideString& sError) {
  return FALSE;
}

FX_BOOL app::execDialog(IJS_Context* cc,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        CFX_WideString& sError) {
  return TRUE;
}
