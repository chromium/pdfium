// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_EVENTPSEUDOMODEL_H_
#define FXJS_XFA_CJX_EVENTPSEUDOMODEL_H_

#include "fxjs/xfa/cjx_object.h"
#include "fxjs/xfa/jse_define.h"

class CScript_EventPseudoModel;

enum class XFA_Event {
  Change = 0,
  CommitKey,
  FullText,
  Keydown,
  Modifier,
  NewContentType,
  NewText,
  PreviousContentType,
  PreviousText,
  Reenter,
  SelectionEnd,
  SelectionStart,
  Shift,
  SoapFaultCode,
  SoapFaultString,
  Target,
  CancelAction
};

class CJX_EventPseudoModel final : public CJX_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_EventPseudoModel() override;

  // CJX_Object:
  bool DynamicTypeIs(TypeTag eType) const override;

  JSE_METHOD(emit);
  JSE_METHOD(reset);

  JSE_PROP(cancelAction);
  JSE_PROP(change);
  JSE_PROP(commitKey);
  JSE_PROP(fullText);
  JSE_PROP(keyDown);
  JSE_PROP(modifier);
  JSE_PROP(newContentType);
  JSE_PROP(newText);
  JSE_PROP(prevContentType);
  JSE_PROP(prevText);
  JSE_PROP(reenter);
  JSE_PROP(selEnd);
  JSE_PROP(selStart);
  JSE_PROP(shift);
  JSE_PROP(soapFaultCode);
  JSE_PROP(soapFaultString);
  JSE_PROP(target);

 private:
  explicit CJX_EventPseudoModel(CScript_EventPseudoModel* model);

  using Type__ = CJX_EventPseudoModel;
  using ParentType__ = CJX_Object;

  static const TypeTag static_type__ = TypeTag::EventPseudoModel;
  static const CJX_MethodSpec MethodSpecs[];

  void Property(v8::Isolate* pIsolate,
                v8::Local<v8::Value>* pValue,
                XFA_Event dwFlag,
                bool bSetting);
};

#endif  // FXJS_XFA_CJX_EVENTPSEUDOMODEL_H_
