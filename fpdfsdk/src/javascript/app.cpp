// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "app.h"

#include <memory>

#include "Document.h"
#include "JS_Context.h"
#include "JS_Define.h"
#include "JS_EventHandler.h"
#include "JS_Object.h"
#include "JS_Runtime.h"
#include "JS_Value.h"
#include "fpdfsdk/include/fsdk_mgr.h"  // For CPDFDoc_Environment.
#include "fpdfsdk/include/javascript/IJavaScript.h"
#include "resource.h"

BEGIN_JS_STATIC_CONST(CJS_TimerObj)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_TimerObj)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_TimerObj)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_TimerObj, TimerObj)

TimerObj::TimerObj(CJS_Object* pJSObject)
    : CJS_EmbedObj(pJSObject), m_pTimer(NULL) {}

TimerObj::~TimerObj() {}

void TimerObj::SetTimer(CJS_Timer* pTimer) {
  m_pTimer = pTimer;
}

CJS_Timer* TimerObj::GetTimer() const {
  return m_pTimer;
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
  for (int i = 0, sz = m_aTimer.GetSize(); i < sz; i++)
    delete m_aTimer[i];

  m_aTimer.RemoveAll();
}

FX_BOOL app::activeDocs(IJS_Context* cc,
                        CJS_PropValue& vp,
                        CFX_WideString& sError) {
  if (!vp.IsGetting())
    return FALSE;

  CJS_Context* pContext = (CJS_Context*)cc;
  CPDFDoc_Environment* pApp = pContext->GetReaderApp();
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  CPDFSDK_Document* pCurDoc = pContext->GetReaderDocument();
  CJS_Array aDocs(pRuntime);
  if (CPDFSDK_Document* pDoc = pApp->GetSDKDocument()) {
    CJS_Document* pJSDocument = NULL;
    if (pDoc == pCurDoc) {
      v8::Local<v8::Object> pObj = FXJS_GetThisObj(pRuntime->GetIsolate());
      if (FXJS_GetObjDefnID(pObj) == CJS_Document::g_nObjDefnID)
        pJSDocument =
            (CJS_Document*)FXJS_GetPrivate(pRuntime->GetIsolate(), pObj);
    } else {
      v8::Local<v8::Object> pObj = FXJS_NewFxDynamicObj(
          pRuntime->GetIsolate(), pRuntime, CJS_Document::g_nObjDefnID);
      pJSDocument =
          (CJS_Document*)FXJS_GetPrivate(pRuntime->GetIsolate(), pObj);
      ASSERT(pJSDocument);
    }
    aDocs.SetElement(0, CJS_Value(pRuntime, pJSDocument));
  }
  if (aDocs.GetLength() > 0)
    vp << aDocs;
  else
    vp.SetNull();

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
    CPDFDoc_Environment* pApp = pContext->GetReaderApp();
    CJS_Runtime* pRuntime = pContext->GetJSRuntime();
    CJS_Array aDocs(pRuntime);
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
  CPDFDoc_Environment* pEnv =
      static_cast<CJS_Context*>(cc)->GetJSRuntime()->GetReaderApp();
  if (!pEnv)
    return FALSE;
  CFX_WideString platfrom = pEnv->FFI_GetPlatform();
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
  CPDFDoc_Environment* pEnv =
      static_cast<CJS_Context*>(cc)->GetJSRuntime()->GetReaderApp();
  if (!pEnv)
    return FALSE;
  CFX_WideString language = pEnv->FFI_GetLanguage();
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
// CFDF_Document * CPDFDoc_Environment::NewFDF();
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
// CFDF_Document * CPDFDoc_Environment::OpenFDF(string strPath,bool bUserConv);

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
  int iSize = params.size();
  if (iSize < 1)
    return FALSE;

  CFX_WideString swMsg = L"";
  CFX_WideString swTitle = L"";
  int iIcon = 0;
  int iType = 0;

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  v8::Isolate* isolate = pRuntime->GetIsolate();

  if (iSize == 1) {
    if (params[0].GetType() == CJS_Value::VT_object) {
      v8::Local<v8::Object> pObj = params[0].ToV8Object();
      {
        v8::Local<v8::Value> pValue =
            FXJS_GetObjectElement(isolate, pObj, L"cMsg");
        swMsg = CJS_Value(pRuntime, pValue, CJS_Value::VT_unknown)
                    .ToCFXWideString();

        pValue = FXJS_GetObjectElement(isolate, pObj, L"cTitle");
        swTitle = CJS_Value(pRuntime, pValue, CJS_Value::VT_unknown)
                      .ToCFXWideString();

        pValue = FXJS_GetObjectElement(isolate, pObj, L"nIcon");
        iIcon = CJS_Value(pRuntime, pValue, CJS_Value::VT_unknown).ToInt();

        pValue = FXJS_GetObjectElement(isolate, pObj, L"nType");
        iType = CJS_Value(pRuntime, pValue, CJS_Value::VT_unknown).ToInt();
      }

      if (swMsg == L"") {
        CJS_Array carray(pRuntime);
        if (params[0].ConvertToArray(carray)) {
          int iLength = carray.GetLength();
          CJS_Value* pValue = new CJS_Value(pRuntime);
          for (int i = 0; i < iLength; ++i) {
            carray.GetElement(i, *pValue);
            swMsg += (*pValue).ToCFXWideString();
            if (i < iLength - 1)
              swMsg += L",  ";
          }

          delete pValue;
        }
      }

      if (swTitle == L"")
        swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
    } else if (params[0].GetType() == CJS_Value::VT_boolean) {
      FX_BOOL bGet = params[0].ToBool();
      if (bGet)
        swMsg = L"true";
      else
        swMsg = L"false";

      swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
    } else {
      swMsg = params[0].ToCFXWideString();
      swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);
    }
  } else {
    if (params[0].GetType() == CJS_Value::VT_boolean) {
      FX_BOOL bGet = params[0].ToBool();
      if (bGet)
        swMsg = L"true";
      else
        swMsg = L"false";
    } else {
      swMsg = params[0].ToCFXWideString();
    }
    swTitle = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSALERT);

    for (int i = 1; i < iSize; i++) {
      if (i == 1)
        iIcon = params[i].ToInt();
      if (i == 2)
        iType = params[i].ToInt();
      if (i == 3)
        swTitle = params[i].ToCFXWideString();
    }
  }

  pRuntime->BeginBlock();
  vRet = MsgBox(pRuntime->GetReaderApp(), swMsg.c_str(), swTitle.c_str(), iType,
                iIcon);
  pRuntime->EndBlock();
  return TRUE;
}

FX_BOOL app::beep(IJS_Context* cc,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  CFX_WideString& sError) {
  if (params.size() == 1) {
    CJS_Context* pContext = (CJS_Context*)cc;
    CJS_Runtime* pRuntime = pContext->GetJSRuntime();
    CPDFDoc_Environment* pEnv = pRuntime->GetReaderApp();
    pEnv->JS_appBeep(params[0].ToInt());
    return TRUE;
  }

  sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
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
  CJS_Context* pContext = (CJS_Context*)cc;
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CFX_WideString script = params.size() > 0 ? params[0].ToCFXWideString() : L"";
  if (script.IsEmpty()) {
    sError = JSGetStringFromID(pContext, IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return TRUE;
  }

  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  FX_DWORD dwInterval = params.size() > 1 ? params[1].ToInt() : 1000;

  CPDFDoc_Environment* pApp = pRuntime->GetReaderApp();
  ASSERT(pApp);
  CJS_Timer* pTimer =
      new CJS_Timer(this, pApp, pRuntime, 0, script, dwInterval, 0);
  m_aTimer.Add(pTimer);

  v8::Local<v8::Object> pRetObj = FXJS_NewFxDynamicObj(
      pRuntime->GetIsolate(), pRuntime, CJS_TimerObj::g_nObjDefnID);
  CJS_TimerObj* pJS_TimerObj =
      (CJS_TimerObj*)FXJS_GetPrivate(pRuntime->GetIsolate(), pRetObj);
  TimerObj* pTimerObj = (TimerObj*)pJS_TimerObj->GetEmbedObject();
  pTimerObj->SetTimer(pTimer);

  vRet = pRetObj;
  return TRUE;
}

FX_BOOL app::setTimeOut(IJS_Context* cc,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        CFX_WideString& sError) {
  if (params.size() > 2 || params.size() == 0) {
    sError = JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  CJS_Context* pContext = (CJS_Context*)cc;
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();

  CFX_WideString script = params.size() > 0 ? params[0].ToCFXWideString() : L"";
  if (script.IsEmpty()) {
    sError =
        JSGetStringFromID((CJS_Context*)cc, IDS_STRING_JSAFNUMBER_KEYSTROKE);
    return TRUE;
  }

  FX_DWORD dwTimeOut = params.size() > 1 ? params[1].ToInt() : 1000;

  CPDFDoc_Environment* pApp = pRuntime->GetReaderApp();
  ASSERT(pApp);

  CJS_Timer* pTimer =
      new CJS_Timer(this, pApp, pRuntime, 1, script, dwTimeOut, dwTimeOut);
  m_aTimer.Add(pTimer);

  v8::Local<v8::Object> pRetObj = FXJS_NewFxDynamicObj(
      pRuntime->GetIsolate(), pRuntime, CJS_TimerObj::g_nObjDefnID);
  CJS_TimerObj* pJS_TimerObj =
      (CJS_TimerObj*)FXJS_GetPrivate(pRuntime->GetIsolate(), pRetObj);
  TimerObj* pTimerObj = (TimerObj*)pJS_TimerObj->GetEmbedObject();
  pTimerObj->SetTimer(pTimer);

  vRet = pRetObj;
  return TRUE;
}

FX_BOOL app::clearTimeOut(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  if (params.size() != 1) {
    sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  if (params[0].GetType() == CJS_Value::VT_fxobject) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object();
    if (FXJS_GetObjDefnID(pObj) == CJS_TimerObj::g_nObjDefnID) {
      if (CJS_Object* pJSObj = params[0].ToCJSObject()) {
        if (TimerObj* pTimerObj = (TimerObj*)pJSObj->GetEmbedObject()) {
          if (CJS_Timer* pTimer = pTimerObj->GetTimer()) {
            pTimer->KillJSTimer();

            for (int i = 0, sz = m_aTimer.GetSize(); i < sz; i++) {
              if (m_aTimer[i] == pTimer) {
                m_aTimer.RemoveAt(i);
                break;
              }
            }

            delete pTimer;
            pTimerObj->SetTimer(NULL);
          }
        }
      }
    }
  }

  return TRUE;
}

FX_BOOL app::clearInterval(IJS_Context* cc,
                           const std::vector<CJS_Value>& params,
                           CJS_Value& vRet,
                           CFX_WideString& sError) {
  CJS_Context* pContext = (CJS_Context*)cc;
  if (params.size() != 1) {
    sError = JSGetStringFromID(pContext, IDS_STRING_JSPARAMERROR);
    return FALSE;
  }

  if (params[0].GetType() == CJS_Value::VT_fxobject) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object();
    if (FXJS_GetObjDefnID(pObj) == CJS_TimerObj::g_nObjDefnID) {
      if (CJS_Object* pJSObj = params[0].ToCJSObject()) {
        if (TimerObj* pTimerObj = (TimerObj*)pJSObj->GetEmbedObject()) {
          if (CJS_Timer* pTimer = pTimerObj->GetTimer()) {
            pTimer->KillJSTimer();

            for (int i = 0, sz = m_aTimer.GetSize(); i < sz; i++) {
              if (m_aTimer[i] == pTimer) {
                m_aTimer.RemoveAt(i);
                break;
              }
            }

            delete pTimer;
            pTimerObj->SetTimer(NULL);
          }
        }
      }
    }
  }

  return TRUE;
}

FX_BOOL app::execMenuItem(IJS_Context* cc,
                          const std::vector<CJS_Value>& params,
                          CJS_Value& vRet,
                          CFX_WideString& sError) {
  return FALSE;
}

void app::TimerProc(CJS_Timer* pTimer) {
  CJS_Runtime* pRuntime = pTimer->GetRuntime();

  switch (pTimer->GetType()) {
    case 0:  // interval
      if (pRuntime)
        RunJsScript(pRuntime, pTimer->GetJScript());
      break;
    case 1:
      if (pTimer->GetTimeOut() > 0) {
        if (pRuntime)
          RunJsScript(pRuntime, pTimer->GetJScript());
        pTimer->KillJSTimer();
      }
      break;
  }
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
  if (params.size() < 1)
    return FALSE;

  FX_BOOL bUI = TRUE;
  CFX_WideString cTo = L"";
  CFX_WideString cCc = L"";
  CFX_WideString cBcc = L"";
  CFX_WideString cSubject = L"";
  CFX_WideString cMsg = L"";

  CJS_Context* pContext = static_cast<CJS_Context*>(cc);
  CJS_Runtime* pRuntime = pContext->GetJSRuntime();
  v8::Isolate* isolate = pRuntime->GetIsolate();

  if (params[0].GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object();

    v8::Local<v8::Value> pValue = FXJS_GetObjectElement(isolate, pObj, L"bUI");
    bUI = CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToBool();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cTo");
    cTo = CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cCc");
    cCc = CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cBcc");
    cBcc =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cSubject");
    cSubject =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cMsg");
    cMsg =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();
  } else {
    if (params.size() < 2)
      return FALSE;

    bUI = params[0].ToBool();
    cTo = params[1].ToCFXWideString();

    if (params.size() >= 3)
      cCc = params[2].ToCFXWideString();
    if (params.size() >= 4)
      cBcc = params[3].ToCFXWideString();
    if (params.size() >= 5)
      cSubject = params[4].ToCFXWideString();
    if (params.size() >= 6)
      cMsg = params[5].ToCFXWideString();
  }

  pRuntime->BeginBlock();
  pContext->GetReaderApp()->JS_docmailForm(NULL, 0, bUI, cTo.c_str(),
                                           cSubject.c_str(), cCc.c_str(),
                                           cBcc.c_str(), cMsg.c_str());
  pRuntime->EndBlock();

  return FALSE;
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
  CFX_WideString swQuestion = L"";
  CFX_WideString swLabel = L"";
  CFX_WideString swTitle = L"PDF";
  CFX_WideString swDefault = L"";
  bool bPassWord = false;

  CJS_Runtime* pRuntime = CJS_Runtime::FromContext(cc);
  v8::Isolate* isolate = pRuntime->GetIsolate();

  int iLength = params.size();
  if (iLength > 0 && params[0].GetType() == CJS_Value::VT_object) {
    v8::Local<v8::Object> pObj = params[0].ToV8Object();
    v8::Local<v8::Value> pValue =
        FXJS_GetObjectElement(isolate, pObj, L"cQuestion");
    swQuestion =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cTitle");
    swTitle =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cDefault");
    swDefault =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"cLabel");
    swLabel =
        CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToCFXWideString();

    pValue = FXJS_GetObjectElement(isolate, pObj, L"bPassword");
    bPassWord = CJS_Value(pRuntime, pValue, GET_VALUE_TYPE(pValue)).ToBool();
  } else {
    switch (iLength) {
      case 5:
        swLabel = params[4].ToCFXWideString();
      // FALLTHROUGH
      case 4:
        bPassWord = params[3].ToBool();
      // FALLTHROUGH
      case 3:
        swDefault = params[2].ToCFXWideString();
      // FALLTHROUGH
      case 2:
        swTitle = params[1].ToCFXWideString();
      // FALLTHROUGH
      case 1:
        swQuestion = params[0].ToCFXWideString();
      // FALLTHROUGH
      default:
        break;
    }
  }

  CJS_Context* pContext = (CJS_Context*)cc;
  CPDFDoc_Environment* pApp = pContext->GetReaderApp();

  const int MAX_INPUT_BYTES = 2048;
  std::unique_ptr<char[]> pBuff(new char[MAX_INPUT_BYTES + 2]);
  memset(pBuff.get(), 0, MAX_INPUT_BYTES + 2);
  int nLengthBytes = pApp->JS_appResponse(
      swQuestion.c_str(), swTitle.c_str(), swDefault.c_str(), swLabel.c_str(),
      bPassWord, pBuff.get(), MAX_INPUT_BYTES);
  if (nLengthBytes <= 0) {
    vRet.SetNull();
    return FALSE;
  }
  nLengthBytes = std::min(nLengthBytes, MAX_INPUT_BYTES);

  CFX_WideString ret_string = CFX_WideString::FromUTF16LE(
      (unsigned short*)pBuff.get(), nLengthBytes / sizeof(unsigned short));
  vRet = ret_string.c_str();
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
