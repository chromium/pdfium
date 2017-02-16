// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/javascript/event.h"

#include "fpdfsdk/javascript/Field.h"
#include "fpdfsdk/javascript/JS_Define.h"
#include "fpdfsdk/javascript/JS_EventHandler.h"
#include "fpdfsdk/javascript/JS_Object.h"
#include "fpdfsdk/javascript/JS_Value.h"
#include "fpdfsdk/javascript/cjs_event_context.h"

BEGIN_JS_STATIC_CONST(CJS_Event)
END_JS_STATIC_CONST()

BEGIN_JS_STATIC_PROP(CJS_Event)
JS_STATIC_PROP_ENTRY(change)
JS_STATIC_PROP_ENTRY(changeEx)
JS_STATIC_PROP_ENTRY(commitKey)
JS_STATIC_PROP_ENTRY(fieldFull)
JS_STATIC_PROP_ENTRY(keyDown)
JS_STATIC_PROP_ENTRY(modifier)
JS_STATIC_PROP_ENTRY(name)
JS_STATIC_PROP_ENTRY(rc)
JS_STATIC_PROP_ENTRY(richChange)
JS_STATIC_PROP_ENTRY(richChangeEx)
JS_STATIC_PROP_ENTRY(richValue)
JS_STATIC_PROP_ENTRY(selEnd)
JS_STATIC_PROP_ENTRY(selStart)
JS_STATIC_PROP_ENTRY(shift)
JS_STATIC_PROP_ENTRY(source)
JS_STATIC_PROP_ENTRY(target)
JS_STATIC_PROP_ENTRY(targetName)
JS_STATIC_PROP_ENTRY(type)
JS_STATIC_PROP_ENTRY(value)
JS_STATIC_PROP_ENTRY(willCommit)
END_JS_STATIC_PROP()

BEGIN_JS_STATIC_METHOD(CJS_Event)
END_JS_STATIC_METHOD()

IMPLEMENT_JS_CLASS(CJS_Event, event)

event::event(CJS_Object* pJsObject) : CJS_EmbedObj(pJsObject) {}

event::~event() {}

bool event::change(IJS_EventContext* cc,
                   CJS_PropValue& vp,
                   CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();
  CFX_WideString& wChange = pEvent->Change();
  if (vp.IsSetting()) {
    if (vp.GetJSValue()->GetType() == CJS_Value::VT_string)
      vp >> wChange;
    return true;
  }
  vp << wChange;
  return true;
}

bool event::changeEx(IJS_EventContext* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->ChangeEx();
  return true;
}

bool event::commitKey(IJS_EventContext* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->CommitKey();
  return true;
}

bool event::fieldFull(IJS_EventContext* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (!vp.IsGetting() &&
      wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return false;

  if (pEvent->FieldFull())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::keyDown(IJS_EventContext* cc,
                    CJS_PropValue& vp,
                    CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (pEvent->KeyDown())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::modifier(IJS_EventContext* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (pEvent->Modifier())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::name(IJS_EventContext* cc,
                 CJS_PropValue& vp,
                 CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->Name();
  return true;
}

bool event::rc(IJS_EventContext* cc,
               CJS_PropValue& vp,
               CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  bool& bRc = pEvent->Rc();
  if (vp.IsSetting())
    vp >> bRc;
  else
    vp << bRc;

  return true;
}

bool event::richChange(IJS_EventContext* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  return true;
}

bool event::richChangeEx(IJS_EventContext* cc,
                         CJS_PropValue& vp,
                         CFX_WideString& sError) {
  return true;
}

bool event::richValue(IJS_EventContext* cc,
                      CJS_PropValue& vp,
                      CFX_WideString& sError) {
  return true;
}

bool event::selEnd(IJS_EventContext* cc,
                   CJS_PropValue& vp,
                   CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  int& iSelEnd = pEvent->SelEnd();
  if (vp.IsSetting())
    vp >> iSelEnd;
  else
    vp << iSelEnd;

  return true;
}

bool event::selStart(IJS_EventContext* cc,
                     CJS_PropValue& vp,
                     CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  int& iSelStart = pEvent->SelStart();
  if (vp.IsSetting())
    vp >> iSelStart;
  else
    vp << iSelStart;

  return true;
}

bool event::shift(IJS_EventContext* cc,
                  CJS_PropValue& vp,
                  CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (pEvent->Shift())
    vp << true;
  else
    vp << false;
  return true;
}

bool event::source(IJS_EventContext* cc,
                   CJS_PropValue& vp,
                   CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->Source()->GetJSObject();
  return true;
}

bool event::target(IJS_EventContext* cc,
                   CJS_PropValue& vp,
                   CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->Target_Field()->GetJSObject();
  return true;
}

bool event::targetName(IJS_EventContext* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventContext* pContext = (CJS_EventContext*)cc;
  CJS_EventHandler* pEvent = pContext->GetEventHandler();

  vp << pEvent->TargetName();
  return true;
}

bool event::type(IJS_EventContext* cc,
                 CJS_PropValue& vp,
                 CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  vp << pEvent->Type();
  return true;
}

bool event::value(IJS_EventContext* cc,
                  CJS_PropValue& vp,
                  CFX_WideString& sError) {
  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Type(), L"Field") != 0)
    return false;

  if (!pEvent->m_pValue)
    return false;

  CFX_WideString& val = pEvent->Value();
  if (vp.IsSetting())
    vp >> val;
  else
    vp << val;

  return true;
}

bool event::willCommit(IJS_EventContext* cc,
                       CJS_PropValue& vp,
                       CFX_WideString& sError) {
  if (!vp.IsGetting())
    return false;

  CJS_EventHandler* pEvent =
      static_cast<CJS_EventContext*>(cc)->GetEventHandler();

  if (pEvent->WillCommit())
    vp << true;
  else
    vp << false;

  return true;
}
