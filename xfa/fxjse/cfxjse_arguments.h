// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXJSE_CFXJSE_ARGUMENTS_H_
#define XFA_FXJSE_CFXJSE_ARGUMENTS_H_

#include "xfa/fxjse/include/fxjse.h"

class CFXJSE_Arguments {
 public:
  v8::Isolate* GetRuntime() const;
  int32_t GetLength() const;
  CFXJSE_Value* GetValue(int32_t index) const;
  FX_BOOL GetBoolean(int32_t index) const;
  int32_t GetInt32(int32_t index) const;
  FX_FLOAT GetFloat(int32_t index) const;
  CFX_ByteString GetUTF8String(int32_t index) const;
  void* GetObject(int32_t index, CFXJSE_Class* pClass = nullptr) const;
  CFXJSE_Value* GetReturnValue();
};

#endif  // XFA_FXJSE_CFXJSE_ARGUMENTS_H_
