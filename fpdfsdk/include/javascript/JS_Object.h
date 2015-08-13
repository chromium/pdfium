// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_

#include <map>

#include "../fsdk_define.h"       // For FX_UINT
#include "../fsdk_mgr.h"          // For CPDFDoc_Environment
#include "../fx_systemhandler.h"  // For IFX_SystemHandler
#include "../jsapi/fxjs_v8.h"

class CPDFSDK_PageView;
class CJS_Object;
class CJS_Timer;
class CJS_Context;

class CJS_EmbedObj {
 public:
  CJS_EmbedObj(CJS_Object* pJSObject);
  virtual ~CJS_EmbedObj();

  virtual void TimerProc(CJS_Timer* pTimer){};

  CJS_Timer* BeginTimer(CPDFDoc_Environment* pApp, FX_UINT nElapse);
  void EndTimer(CJS_Timer* pTimer);

  CJS_Object* GetJSObject() { return m_pJSObject; };
  operator CJS_Object*() { return m_pJSObject; };

  CPDFSDK_PageView* JSGetPageView(IFXJS_Context* cc);
  int MsgBox(CPDFDoc_Environment* pApp,
             CPDFSDK_PageView* pPageView,
             const FX_WCHAR* swMsg,
             const FX_WCHAR* swTitle = NULL,
             FX_UINT nType = 0,
             FX_UINT nIcon = 0);
  void Alert(CJS_Context* pContext, const FX_WCHAR* swMsg);

 protected:
  CJS_Object* m_pJSObject;
};

class CJS_Object {
 public:
  CJS_Object(JSFXObject pObject);
  virtual ~CJS_Object(void);

  void MakeWeak();
  void Dispose();

  virtual FX_BOOL IsType(const FX_CHAR* sClassName) { return TRUE; };
  virtual CFX_ByteString GetClassName() { return ""; };

  virtual FX_BOOL InitInstance(IFXJS_Context* cc) { return TRUE; };
  virtual FX_BOOL ExitInstance() { return TRUE; };

  operator JSFXObject() {
    return v8::Local<v8::Object>::New(m_pIsolate, m_pObject);
  }
  operator CJS_EmbedObj*() { return m_pEmbedObj; };

  void SetEmbedObject(CJS_EmbedObj* pObj) { m_pEmbedObj = pObj; };
  CJS_EmbedObj* GetEmbedObject() { return m_pEmbedObj; };

  static CPDFSDK_PageView* JSGetPageView(IFXJS_Context* cc);
  static int MsgBox(CPDFDoc_Environment* pApp,
                    CPDFSDK_PageView* pPageView,
                    const FX_WCHAR* swMsg,
                    const FX_WCHAR* swTitle = NULL,
                    FX_UINT nType = 0,
                    FX_UINT nIcon = 0);
  static void Alert(CJS_Context* pContext, const FX_WCHAR* swMsg);

  v8::Isolate* GetIsolate() { return m_pIsolate; }

 protected:
  CJS_EmbedObj* m_pEmbedObj;
  v8::Global<v8::Object> m_pObject;
  v8::Isolate* m_pIsolate;
};

using JSTimerMap = std::map<FX_UINT, CJS_Timer*>;
JSTimerMap* GetGlobalTimerMap();

class CJS_Runtime;

class CJS_Timer {
 public:
  CJS_Timer(CJS_EmbedObj* pObj, CPDFDoc_Environment* pApp)
      : m_nTimerID(0),
        m_pEmbedObj(pObj),
        m_bProcessing(FALSE),
        m_dwStartTime(0),
        m_dwTimeOut(0),
        m_dwElapse(0),
        m_pRuntime(NULL),
        m_nType(0),
        m_pApp(pApp) {}

  virtual ~CJS_Timer() { KillJSTimer(); }

 public:
  FX_UINT SetJSTimer(FX_UINT nElapse) {
    if (m_nTimerID)
      KillJSTimer();
    IFX_SystemHandler* pHandler = m_pApp->GetSysHandler();
    m_nTimerID = pHandler->SetTimer(nElapse, TimerProc);
    (*GetGlobalTimerMap())[m_nTimerID] = this;
    m_dwElapse = nElapse;
    return m_nTimerID;
  };

  void KillJSTimer() {
    if (m_nTimerID) {
      IFX_SystemHandler* pHandler = m_pApp->GetSysHandler();
      pHandler->KillTimer(m_nTimerID);
      GetGlobalTimerMap()->erase(m_nTimerID);
      m_nTimerID = 0;
    }
  };

  void SetType(int nType) { m_nType = nType; }

  int GetType() const { return m_nType; }

  void SetStartTime(FX_DWORD dwStartTime) { m_dwStartTime = dwStartTime; }

  FX_DWORD GetStartTime() const { return m_dwStartTime; }

  void SetTimeOut(FX_DWORD dwTimeOut) { m_dwTimeOut = dwTimeOut; }

  FX_DWORD GetTimeOut() const { return m_dwTimeOut; }

  void SetRuntime(CJS_Runtime* pRuntime) { m_pRuntime = pRuntime; }

  CJS_Runtime* GetRuntime() const { return m_pRuntime; }

  void SetJScript(const CFX_WideString& script) { m_swJScript = script; }

  CFX_WideString GetJScript() const { return m_swJScript; }

  static void TimerProc(int idEvent) {
    const auto it = GetGlobalTimerMap()->find(idEvent);
    if (it != GetGlobalTimerMap()->end()) {
      CJS_Timer* pTimer = it->second;
      if (!pTimer->m_bProcessing) {
        pTimer->m_bProcessing = TRUE;
        if (pTimer->m_pEmbedObj)
          pTimer->m_pEmbedObj->TimerProc(pTimer);
        pTimer->m_bProcessing = FALSE;
      }
    }
  };

 private:
  FX_UINT m_nTimerID;
  CJS_EmbedObj* m_pEmbedObj;
  FX_BOOL m_bProcessing;

  // data
  FX_DWORD m_dwStartTime;
  FX_DWORD m_dwTimeOut;
  FX_DWORD m_dwElapse;
  CJS_Runtime* m_pRuntime;
  CFX_WideString m_swJScript;
  int m_nType;  // 0:Interval; 1:TimeOut

  CPDFDoc_Environment* m_pApp;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_
