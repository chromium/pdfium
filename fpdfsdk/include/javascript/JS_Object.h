// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_

#include <map>

#include "../../../third_party/base/nonstd_unique_ptr.h"

#include "../fsdk_define.h"       // For FX_UINT
#include "../fsdk_mgr.h"          // For CPDFDoc_Environment
#include "../fx_systemhandler.h"  // For IFX_SystemHandler
#include "../jsapi/fxjs_v8.h"
#include "JS_Runtime.h"

class CPDFSDK_PageView;
class CJS_Context;
class CJS_Object;
class CJS_Timer;

class CJS_EmbedObj {
 public:
  explicit CJS_EmbedObj(CJS_Object* pJSObject);
  virtual ~CJS_EmbedObj();

  virtual void TimerProc(CJS_Timer* pTimer) {}

  CJS_Object* GetJSObject() const { return m_pJSObject; }

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
  explicit CJS_Object(v8::Local<v8::Object> pObject);
  virtual ~CJS_Object();

  void MakeWeak();
  void Dispose();

  virtual FX_BOOL IsType(const FX_CHAR* sClassName) { return TRUE; }
  virtual CFX_ByteString GetClassName() { return ""; }

  virtual FX_BOOL InitInstance(IFXJS_Context* cc) { return TRUE; }
  virtual FX_BOOL ExitInstance() { return TRUE; }

  v8::Local<v8::Object> ToV8Object() { return m_pV8Object.Get(m_pIsolate); }

  // Takes ownership of |pObj|.
  void SetEmbedObject(CJS_EmbedObj* pObj) { m_pEmbedObj.reset(pObj); }
  CJS_EmbedObj* GetEmbedObject() const { return m_pEmbedObj.get(); }

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
  nonstd::unique_ptr<CJS_EmbedObj> m_pEmbedObj;
  v8::Global<v8::Object> m_pV8Object;
  v8::Isolate* m_pIsolate;
};

class CJS_Timer : public CJS_Runtime::Observer {
 public:
  CJS_Timer(CJS_EmbedObj* pObj,
            CPDFDoc_Environment* pApp,
            CJS_Runtime* pRuntime,
            int nType,
            const CFX_WideString& script,
            FX_DWORD dwElapse,
            FX_DWORD dwTimeOut);
  ~CJS_Timer() override;

  void KillJSTimer();

  int GetType() const { return m_nType; }
  FX_DWORD GetTimeOut() const { return m_dwTimeOut; }
  CJS_Runtime* GetRuntime() const { return m_bValid ? m_pRuntime : nullptr; }
  CFX_WideString GetJScript() const { return m_swJScript; }

  static void TimerProc(int idEvent);

 private:
  using TimerMap = std::map<FX_UINT, CJS_Timer*>;
  static TimerMap* GetGlobalTimerMap();

  // CJS_Runtime::Observer
  void OnDestroyed() override;

  FX_DWORD m_nTimerID;
  CJS_EmbedObj* const m_pEmbedObj;
  bool m_bProcessing;
  bool m_bValid;

  // data
  const int m_nType;  // 0:Interval; 1:TimeOut
  const FX_DWORD m_dwTimeOut;
  const CFX_WideString m_swJScript;
  CJS_Runtime* const m_pRuntime;
  CPDFDoc_Environment* const m_pApp;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_OBJECT_H_
