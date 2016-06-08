// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXJSE_INCLUDE_FXJSE_H_
#define XFA_FXJSE_INCLUDE_FXJSE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"
#include "v8/include/v8.h"

class CFXJSE_Arguments;
class CFXJSE_Class;
class CFXJSE_Context;
class CFXJSE_Value;

class CFXJSE_HostObject {};  // C++ object which can be wrapped by CFXJSE_value.

typedef void (*FXJSE_FuncCallback)(CFXJSE_Value* pThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args);
typedef void (*FXJSE_PropAccessor)(CFXJSE_Value* pObject,
                                   const CFX_ByteStringC& szPropName,
                                   CFXJSE_Value* pValue);
typedef int32_t (*FXJSE_PropTypeGetter)(CFXJSE_Value* pObject,
                                        const CFX_ByteStringC& szPropName,
                                        FX_BOOL bQueryIn);
typedef FX_BOOL (*FXJSE_PropDeleter)(CFXJSE_Value* pObject,
                                     const CFX_ByteStringC& szPropName);

enum FXJSE_ClassPropTypes {
  FXJSE_ClassPropType_None,
  FXJSE_ClassPropType_Property,
  FXJSE_ClassPropType_Method
};

struct FXJSE_FUNCTION_DESCRIPTOR {
  const FX_CHAR* name;
  FXJSE_FuncCallback callbackProc;
};

struct FXJSE_PROPERTY_DESCRIPTOR {
  const FX_CHAR* name;
  FXJSE_PropAccessor getProc;
  FXJSE_PropAccessor setProc;
};

struct FXJSE_CLASS_DESCRIPTOR {
  const FX_CHAR* name;
  FXJSE_FuncCallback constructor;
  const FXJSE_PROPERTY_DESCRIPTOR* properties;
  const FXJSE_FUNCTION_DESCRIPTOR* methods;
  int32_t propNum;
  int32_t methNum;
  FXJSE_PropTypeGetter dynPropTypeGetter;
  FXJSE_PropAccessor dynPropGetter;
  FXJSE_PropAccessor dynPropSetter;
  FXJSE_PropDeleter dynPropDeleter;
  FXJSE_FuncCallback dynMethodCall;
};

void FXJSE_Initialize();
void FXJSE_Finalize();

v8::Isolate* FXJSE_Runtime_Create_Own();
void FXJSE_Runtime_Release(v8::Isolate* pIsolate);

CFXJSE_Context* FXJSE_Context_Create(
    v8::Isolate* pIsolate,
    const FXJSE_CLASS_DESCRIPTOR* lpGlobalClass,
    CFXJSE_HostObject* lpGlobalObject);
void FXJSE_Context_Release(CFXJSE_Context* pContext);
CFXJSE_Value* FXJSE_Context_GetGlobalObject(CFXJSE_Context* pContext);
void FXJSE_Context_EnableCompatibleMode(CFXJSE_Context* pContext);

CFXJSE_Class* FXJSE_DefineClass(CFXJSE_Context* pContext,
                                const FXJSE_CLASS_DESCRIPTOR* lpClass);

FX_BOOL FXJSE_Value_IsUndefined(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsNull(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsBoolean(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsUTF8String(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsNumber(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsObject(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsArray(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsFunction(CFXJSE_Value* pValue);

FX_BOOL FXJSE_ExecuteScript(CFXJSE_Context* pContext,
                            const FX_CHAR* szScript,
                            CFXJSE_Value* pRetValue,
                            CFXJSE_Value* pNewThisObject = nullptr);

void FXJSE_ThrowMessage(const CFX_ByteStringC& utf8Name,
                        const CFX_ByteStringC& utf8Message);

#endif  // XFA_FXJSE_INCLUDE_FXJSE_H_
