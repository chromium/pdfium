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

JSConstSpec CJS_Event::ConstSpecs[] = {{0, JSConstSpec::Number, 0, 0}};

JSPropertySpec CJS_Event::PropertySpecs[] = {
    {"change", get_change_static, set_change_static},
    {"changeEx", get_change_ex_static, set_change_ex_static},
    {"commitKey", get_commit_key_static, set_commit_key_static},
    {"fieldFull", get_field_full_static, set_field_full_static},
    {"keyDown", get_key_down_static, set_key_down_static},
    {"modifier", get_modifier_static, set_modifier_static},
    {"name", get_name_static, set_name_static},
    {"rc", get_rc_static, set_rc_static},
    {"richChange", get_rich_change_static, set_rich_change_static},
    {"richChangeEx", get_rich_change_ex_static, set_rich_change_ex_static},
    {"richValue", get_rich_value_static, set_rich_value_static},
    {"selEnd", get_sel_end_static, set_sel_end_static},
    {"selStart", get_sel_start_static, set_sel_start_static},
    {"shift", get_shift_static, set_shift_static},
    {"source", get_source_static, set_source_static},
    {"target", get_target_static, set_target_static},
    {"targetName", get_target_name_static, set_target_name_static},
    {"type", get_type_static, set_type_static},
    {"value", get_value_static, set_value_static},
    {"willCommit", get_will_commit_static, set_will_commit_static},
    {0, 0, 0}};

JSMethodSpec CJS_Event::MethodSpecs[] = {{0, 0}};

IMPLEMENT_JS_CLASS(CJS_Event, event, event)

event::event(CJS_Object* pJsObject) : CJS_EmbedObj(pJsObject) {}

event::~event() {}

bool event::get_change(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Change());
  return true;
}

bool event::set_change(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (vp.GetType() == CJS_Value::VT_string) {
    WideString& wChange = pEvent->Change();
    wChange = vp.ToWideString(pRuntime);
  }
  return true;
}

bool event::get_change_ex(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  vp->Set(pRuntime, pEvent->ChangeEx());
  return true;
}

bool event::set_change_ex(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  return false;
}

bool event::get_commit_key(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  vp->Set(pRuntime, pEvent->CommitKey());
  return true;
}

bool event::set_commit_key(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return false;
}

bool event::get_field_full(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return false;

  vp->Set(pRuntime, pEvent->FieldFull());
  return true;
}

bool event::set_field_full(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return false;
}

bool event::get_key_down(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->KeyDown());
  return true;
}

bool event::set_key_down(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  return false;
}

bool event::get_modifier(CJS_Runtime* pRuntime,
                         CJS_Value* vp,
                         WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Modifier());
  return true;
}

bool event::set_modifier(CJS_Runtime* pRuntime,
                         const CJS_Value& vp,
                         WideString* sError) {
  return false;
}

bool event::get_name(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Name());
  return true;
}

bool event::set_name(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  return false;
}

bool event::get_rc(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Rc());
  return true;
}

bool event::set_rc(CJS_Runtime* pRuntime,
                   const CJS_Value& vp,
                   WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  pEvent->Rc() = vp.ToBool(pRuntime);
  return true;
}

bool event::get_rich_change(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  return true;
}

bool event::set_rich_change(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return true;
}

bool event::get_rich_change_ex(CJS_Runtime* pRuntime,
                               CJS_Value* vp,
                               WideString* sError) {
  return true;
}

bool event::set_rich_change_ex(CJS_Runtime* pRuntime,
                               const CJS_Value& vp,
                               WideString* sError) {
  return true;
}

bool event::get_rich_value(CJS_Runtime* pRuntime,
                           CJS_Value* vp,
                           WideString* sError) {
  return true;
}

bool event::set_rich_value(CJS_Runtime* pRuntime,
                           const CJS_Value& vp,
                           WideString* sError) {
  return true;
}

bool event::get_sel_end(CJS_Runtime* pRuntime,
                        CJS_Value* vp,
                        WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  vp->Set(pRuntime, pEvent->SelEnd());
  return true;
}

bool event::set_sel_end(CJS_Runtime* pRuntime,
                        const CJS_Value& vp,
                        WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  pEvent->SelEnd() = vp.ToInt(pRuntime);
  return true;
}

bool event::get_sel_start(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  vp->Set(pRuntime, pEvent->SelStart());
  return true;
}

bool event::set_sel_start(CJS_Runtime* pRuntime,
                          const CJS_Value& vp,
                          WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Name(), L"Keystroke") != 0)
    return true;

  pEvent->SelStart() = vp.ToInt(pRuntime);
  return true;
}

bool event::get_shift(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Shift());
  return true;
}

bool event::set_shift(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  return false;
}

bool event::get_source(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Source()->GetJSObject());
  return true;
}

bool event::set_source(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  return false;
}

bool event::get_target(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Target_Field()->GetJSObject());
  return true;
}

bool event::set_target(CJS_Runtime* pRuntime,
                       const CJS_Value& vp,
                       WideString* sError) {
  return false;
}

bool event::get_target_name(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->TargetName());
  return true;
}

bool event::set_target_name(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return false;
}

bool event::get_type(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->Type());
  return true;
}

bool event::set_type(CJS_Runtime* pRuntime,
                     const CJS_Value& vp,
                     WideString* sError) {
  return false;
}

bool event::get_value(CJS_Runtime* pRuntime,
                      CJS_Value* vp,
                      WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Type(), L"Field") != 0)
    return false;

  if (!pEvent->m_pValue)
    return false;

  vp->Set(pRuntime, pEvent->Value());
  return true;
}

bool event::set_value(CJS_Runtime* pRuntime,
                      const CJS_Value& vp,
                      WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();

  if (wcscmp((const wchar_t*)pEvent->Type(), L"Field") != 0)
    return false;

  if (!pEvent->m_pValue)
    return false;

  pEvent->Value() = vp.ToWideString(pRuntime);
  return true;
}

bool event::get_will_commit(CJS_Runtime* pRuntime,
                            CJS_Value* vp,
                            WideString* sError) {
  CJS_EventHandler* pEvent =
      pRuntime->GetCurrentEventContext()->GetEventHandler();
  vp->Set(pRuntime, pEvent->WillCommit());
  return true;
}

bool event::set_will_commit(CJS_Runtime* pRuntime,
                            const CJS_Value& vp,
                            WideString* sError) {
  return false;
}
