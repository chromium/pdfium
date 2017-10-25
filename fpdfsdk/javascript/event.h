// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_EVENT_H_
#define FPDFSDK_JAVASCRIPT_EVENT_H_

#include "fpdfsdk/javascript/JS_Define.h"

class event : public CJS_EmbedObj {
 public:
  explicit event(CJS_Object* pJSObject);
  ~event() override;

 public:
  bool get_change(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_change(CJS_Runtime* pRuntime,
                  v8::Local<v8::Value> vp,
                  WideString* sError);

  bool get_change_ex(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_change_ex(CJS_Runtime* pRuntime,
                     v8::Local<v8::Value> vp,
                     WideString* sError);

  bool get_commit_key(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_commit_key(CJS_Runtime* pRuntime,
                      v8::Local<v8::Value> vp,
                      WideString* sError);

  bool get_field_full(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_field_full(CJS_Runtime* pRuntime,
                      v8::Local<v8::Value> vp,
                      WideString* sError);

  bool get_key_down(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_key_down(CJS_Runtime* pRuntime,
                    v8::Local<v8::Value> vp,
                    WideString* sError);

  bool get_modifier(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_modifier(CJS_Runtime* pRuntime,
                    v8::Local<v8::Value> vp,
                    WideString* sError);

  bool get_name(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_name(CJS_Runtime* pRuntime,
                v8::Local<v8::Value> vp,
                WideString* sError);

  bool get_rc(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_rc(CJS_Runtime* pRuntime,
              v8::Local<v8::Value> vp,
              WideString* sError);

  bool get_rich_change(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError);
  bool set_rich_change(CJS_Runtime* pRuntime,
                       v8::Local<v8::Value> vp,
                       WideString* sError);

  bool get_rich_change_ex(CJS_Runtime* pRuntime,
                          CJS_Value* vp,
                          WideString* sError);
  bool set_rich_change_ex(CJS_Runtime* pRuntime,
                          v8::Local<v8::Value> vp,
                          WideString* sError);

  bool get_rich_value(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_rich_value(CJS_Runtime* pRuntime,
                      v8::Local<v8::Value> vp,
                      WideString* sError);

  bool get_sel_end(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_sel_end(CJS_Runtime* pRuntime,
                   v8::Local<v8::Value> vp,
                   WideString* sError);

  bool get_sel_start(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_sel_start(CJS_Runtime* pRuntime,
                     v8::Local<v8::Value> vp,
                     WideString* sError);

  bool get_shift(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_shift(CJS_Runtime* pRuntime,
                 v8::Local<v8::Value> vp,
                 WideString* sError);

  bool get_source(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_source(CJS_Runtime* pRuntime,
                  v8::Local<v8::Value> vp,
                  WideString* sError);

  bool get_target(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_target(CJS_Runtime* pRuntime,
                  v8::Local<v8::Value> vp,
                  WideString* sError);

  bool get_target_name(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError);
  bool set_target_name(CJS_Runtime* pRuntime,
                       v8::Local<v8::Value> vp,
                       WideString* sError);

  bool get_type(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_type(CJS_Runtime* pRuntime,
                v8::Local<v8::Value> vp,
                WideString* sError);

  bool get_value(CJS_Runtime* pRuntime, CJS_Value* vp, WideString* sError);
  bool set_value(CJS_Runtime* pRuntime,
                 v8::Local<v8::Value> vp,
                 WideString* sError);

  bool get_will_commit(CJS_Runtime* pRuntime,
                       CJS_Value* vp,
                       WideString* sError);
  bool set_will_commit(CJS_Runtime* pRuntime,
                       v8::Local<v8::Value> vp,
                       WideString* sError);
};

class CJS_Event : public CJS_Object {
 public:
  explicit CJS_Event(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Event() override {}

  DECLARE_JS_CLASS();
  JS_STATIC_PROP(change, change, event);
  JS_STATIC_PROP(changeEx, change_ex, event);
  JS_STATIC_PROP(commitKey, commit_key, event);
  JS_STATIC_PROP(fieldFull, field_full, event);
  JS_STATIC_PROP(keyDown, key_down, event);
  JS_STATIC_PROP(modifier, modifier, event);
  JS_STATIC_PROP(name, name, event);
  JS_STATIC_PROP(rc, rc, event);
  JS_STATIC_PROP(richChange, rich_change, event);
  JS_STATIC_PROP(richChangeEx, rich_change_ex, event);
  JS_STATIC_PROP(richValue, rich_value, event);
  JS_STATIC_PROP(selEnd, sel_end, event);
  JS_STATIC_PROP(selStart, sel_start, event);
  JS_STATIC_PROP(shift, shift, event);
  JS_STATIC_PROP(source, source, event);
  JS_STATIC_PROP(target, target, event);
  JS_STATIC_PROP(targetName, target_name, event);
  JS_STATIC_PROP(type, type, event);
  JS_STATIC_PROP(value, value, event);
  JS_STATIC_PROP(willCommit, will_commit, event);
};

#endif  // FPDFSDK_JAVASCRIPT_EVENT_H_
