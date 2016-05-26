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

enum FXJSE_CompatibleModeFlags {
  FXJSE_COMPATIBLEMODEFLAG_CONSTRUCTOREXTRAMETHODS = (1 << 0),
  FXJSE_COMPATIBLEMODEFLAGCOUNT = 1,
};

struct FXJSE_FUNCTION {
  const FX_CHAR* name;
  FXJSE_FuncCallback callbackProc;
};

struct FXJSE_PROPERTY {
  const FX_CHAR* name;
  FXJSE_PropAccessor getProc;
  FXJSE_PropAccessor setProc;
};

struct FXJSE_CLASS {
  const FX_CHAR* name;
  FXJSE_FuncCallback constructor;
  FXJSE_PROPERTY* properties;
  FXJSE_FUNCTION* methods;
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

v8::Isolate* FXJSE_Runtime_Create();
void FXJSE_Runtime_Release(v8::Isolate* pIsolate, bool bOwnedRuntime);

CFXJSE_Context* FXJSE_Context_Create(v8::Isolate* pIsolate,
                                     const FXJSE_CLASS* lpGlobalClass = nullptr,
                                     void* lpGlobalObject = nullptr);
void FXJSE_Context_Release(CFXJSE_Context* pContext);
CFXJSE_Value* FXJSE_Context_GetGlobalObject(CFXJSE_Context* pContext);

void FXJSE_Context_EnableCompatibleMode(CFXJSE_Context* pContext,
                                        uint32_t dwCompatibleFlags);

CFXJSE_Class* FXJSE_DefineClass(CFXJSE_Context* pContext,
                                const FXJSE_CLASS* lpClass);

CFXJSE_Value* FXJSE_Value_Create(v8::Isolate* pIsolate);
void FXJSE_Value_Release(CFXJSE_Value* pValue);

FX_BOOL FXJSE_Value_IsUndefined(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsNull(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsBoolean(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsUTF8String(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsNumber(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsObject(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsArray(CFXJSE_Value* pValue);
FX_BOOL FXJSE_Value_IsFunction(CFXJSE_Value* pValue);

FX_BOOL FXJSE_Value_ToBoolean(CFXJSE_Value* pValue);
FX_FLOAT FXJSE_Value_ToFloat(CFXJSE_Value* pValue);
double FXJSE_Value_ToDouble(CFXJSE_Value* pValue);
int32_t FXJSE_Value_ToInteger(CFXJSE_Value* pValue);
void FXJSE_Value_ToUTF8String(CFXJSE_Value* pValue,
                              CFX_ByteString& szStrOutput);
void* FXJSE_Value_ToObject(CFXJSE_Value* pValue, CFXJSE_Class* pClass);

void FXJSE_Value_SetUndefined(CFXJSE_Value* pValue);
void FXJSE_Value_SetNull(CFXJSE_Value* pValue);
void FXJSE_Value_SetBoolean(CFXJSE_Value* pValue, FX_BOOL bBoolean);
void FXJSE_Value_SetUTF8String(CFXJSE_Value* pValue,
                               const CFX_ByteStringC& szString);
void FXJSE_Value_SetInteger(CFXJSE_Value* pValue, int32_t nInteger);
void FXJSE_Value_SetFloat(CFXJSE_Value* pValue, FX_FLOAT fFloat);
void FXJSE_Value_SetDouble(CFXJSE_Value* pValue, double dDouble);
void FXJSE_Value_SetObject(CFXJSE_Value* pValue,
                           void* lpObject,
                           CFXJSE_Class* pClass);
void FXJSE_Value_SetArray(CFXJSE_Value* pValue,
                          uint32_t uValueCount,
                          CFXJSE_Value** rgValues);
void FXJSE_Value_Set(CFXJSE_Value* pValue, CFXJSE_Value* pOriginalValue);

FX_BOOL FXJSE_Value_GetObjectProp(CFXJSE_Value* pValue,
                                  const CFX_ByteStringC& szPropName,
                                  CFXJSE_Value* pPropValue);
FX_BOOL FXJSE_Value_SetObjectProp(CFXJSE_Value* pValue,
                                  const CFX_ByteStringC& szPropName,
                                  CFXJSE_Value* pPropValue);
FX_BOOL FXJSE_Value_GetObjectPropByIdx(CFXJSE_Value* pValue,
                                       uint32_t uPropIdx,
                                       CFXJSE_Value* pPropValue);
FX_BOOL FXJSE_Value_DeleteObjectProp(CFXJSE_Value* pValue,
                                     const CFX_ByteStringC& szPropName);
FX_BOOL FXJSE_Value_ObjectHasOwnProp(CFXJSE_Value* pValue,
                                     const CFX_ByteStringC& szPropName,
                                     FX_BOOL bUseTypeGetter);
FX_BOOL FXJSE_Value_SetObjectOwnProp(CFXJSE_Value* pValue,
                                     const CFX_ByteStringC& szPropName,
                                     CFXJSE_Value* pPropValue);

FX_BOOL FXJSE_Value_SetFunctionBind(CFXJSE_Value* pValue,
                                    CFXJSE_Value* pOldFunction,
                                    CFXJSE_Value* pNewThis);

FX_BOOL FXJSE_ExecuteScript(CFXJSE_Context* pContext,
                            const FX_CHAR* szScript,
                            CFXJSE_Value* pRetValue,
                            CFXJSE_Value* pNewThisObject = nullptr);

void FXJSE_ThrowMessage(const CFX_ByteStringC& utf8Name,
                        const CFX_ByteStringC& utf8Message);

#endif  // XFA_FXJSE_INCLUDE_FXJSE_H_
