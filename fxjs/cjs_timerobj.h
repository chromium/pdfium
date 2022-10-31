// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_TIMEROBJ_H_
#define FXJS_CJS_TIMEROBJ_H_

#include "fxjs/cjs_object.h"

class GlobalTimer;

class CJS_TimerObj final : public CJS_Object {
 public:
  static uint32_t GetObjDefnID();
  static void DefineJSObjects(CFXJS_Engine* pEngine);

  CJS_TimerObj(v8::Local<v8::Object> pObject, CJS_Runtime* pRuntime);
  ~CJS_TimerObj() override;

  void SetTimer(GlobalTimer* pTimer);
  int GetTimerID() const { return m_nTimerID; }

 private:
  static uint32_t ObjDefnID;

  int m_nTimerID = 0;  // Weak reference to GlobalTimer through global map.
};

#endif  // FXJS_CJS_TIMEROBJ_H_
