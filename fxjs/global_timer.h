// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_GLOBAL_TIMER_H_
#define FXJS_GLOBAL_TIMER_H_

#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cjs_runtime.h"

class CJS_App;

class GlobalTimer {
 public:
  enum class Type : bool {
    kRepeating = false,
    kOneShot = true,
  };

  static void InitializeGlobals();
  static void DestroyGlobals();

  GlobalTimer(CJS_App* pObj,
              CJS_Runtime* pRuntime,
              Type nType,
              const WideString& script,
              uint32_t dwElapse,
              uint32_t dwTimeOut);
  ~GlobalTimer();

  static void Trigger(int32_t nTimerID);
  static void Cancel(int32_t nTimerID);

  bool IsOneShot() const { return type_ == Type::kOneShot; }
  uint32_t GetTimeOut() const { return time_out_; }
  int32_t GetTimerID() const { return timer_id_; }
  CJS_Runtime* GetRuntime() const { return runtime_.Get(); }
  WideString GetJScript() const { return jscript_; }

 private:
  bool HasValidID() const;

  const Type type_;
  bool processing_ = false;
  const int32_t timer_id_;
  const uint32_t time_out_;
  const WideString jscript_;
  ObservedPtr<CJS_Runtime> runtime_;
  UnownedPtr<CJS_App> const embed_app_;
};

#endif  // FXJS_GLOBAL_TIMER_H_
