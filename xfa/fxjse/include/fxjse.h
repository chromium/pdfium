// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXJSE_INCLUDE_FXJSE_H_
#define XFA_FXJSE_INCLUDE_FXJSE_H_

#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

struct FXJSE_CLASS;
class CFXJSE_Arguments;

typedef struct FXJSE_HRUNTIME_ { void* pData; } * FXJSE_HRUNTIME;
typedef struct FXJSE_HCONTEXT_ { void* pData; } * FXJSE_HCONTEXT;
typedef struct FXJSE_HCLASS_ { void* pData; } * FXJSE_HCLASS;
typedef struct FXJSE_HVALUE_ { void* pData; } * FXJSE_HVALUE;
// NOLINTNEXTLINE
typedef struct FXJSE_HOBJECT_ : public FXJSE_HVALUE_{} * FXJSE_HOBJECT;

typedef void (*FXJSE_FuncCallback)(FXJSE_HOBJECT hThis,
                                   const CFX_ByteStringC& szFuncName,
                                   CFXJSE_Arguments& args);
typedef void (*FXJSE_PropAccessor)(FXJSE_HOBJECT hObject,
                                   const CFX_ByteStringC& szPropName,
                                   FXJSE_HVALUE hValue);
typedef int32_t (*FXJSE_PropTypeGetter)(FXJSE_HOBJECT hObject,
                                        const CFX_ByteStringC& szPropName,
                                        FX_BOOL bQueryIn);
typedef FX_BOOL (*FXJSE_PropDeleter)(FXJSE_HOBJECT hObject,
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

FXJSE_HRUNTIME FXJSE_Runtime_Create();
void FXJSE_Runtime_Release(FXJSE_HRUNTIME hRuntime, bool bOwnedRuntime);

FXJSE_HCONTEXT FXJSE_Context_Create(FXJSE_HRUNTIME hRuntime,
                                    const FXJSE_CLASS* lpGlobalClass = nullptr,
                                    void* lpGlobalObject = nullptr);
void FXJSE_Context_Release(FXJSE_HCONTEXT hContext);
FXJSE_HVALUE FXJSE_Context_GetGlobalObject(FXJSE_HCONTEXT hContext);
FXJSE_HRUNTIME FXJSE_Context_GetRuntime(FXJSE_HCONTEXT hContext);

void FXJSE_Context_EnableCompatibleMode(FXJSE_HCONTEXT hContext,
                                        uint32_t dwCompatibleFlags);

void FXJSE_DefineFunctions(FXJSE_HCONTEXT hContext,
                           const FXJSE_FUNCTION* lpFunctions,
                           int nNum);
FXJSE_HCLASS FXJSE_DefineClass(FXJSE_HCONTEXT hContext,
                               const FXJSE_CLASS* lpClass);
FXJSE_HCLASS FXJSE_GetClass(FXJSE_HCONTEXT hContext,
                            const CFX_ByteStringC& szName);

FXJSE_HVALUE FXJSE_Value_Create(FXJSE_HRUNTIME hRuntime);
void FXJSE_Value_Release(FXJSE_HVALUE hValue);
FXJSE_HRUNTIME FXJSE_Value_GetRuntime(FXJSE_HVALUE hValue);

FX_BOOL FXJSE_Value_IsUndefined(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsNull(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsBoolean(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsUTF8String(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsNumber(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsInteger(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsObject(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsArray(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsFunction(FXJSE_HVALUE hValue);
FX_BOOL FXJSE_Value_IsDate(FXJSE_HVALUE hValue);

FX_BOOL FXJSE_Value_ToBoolean(FXJSE_HVALUE hValue);
FX_FLOAT FXJSE_Value_ToFloat(FXJSE_HVALUE hValue);
double FXJSE_Value_ToDouble(FXJSE_HVALUE hValue);
int32_t FXJSE_Value_ToInteger(FXJSE_HVALUE hValue);
void FXJSE_Value_ToUTF8String(FXJSE_HVALUE hValue, CFX_ByteString& szStrOutput);
void* FXJSE_Value_ToObject(FXJSE_HVALUE hValue, FXJSE_HCLASS hClass);

void FXJSE_Value_SetUndefined(FXJSE_HVALUE hValue);
void FXJSE_Value_SetNull(FXJSE_HVALUE hValue);
void FXJSE_Value_SetBoolean(FXJSE_HVALUE hValue, FX_BOOL bBoolean);
void FXJSE_Value_SetUTF8String(FXJSE_HVALUE hValue,
                               const CFX_ByteStringC& szString);
void FXJSE_Value_SetInteger(FXJSE_HVALUE hValue, int32_t nInteger);
void FXJSE_Value_SetFloat(FXJSE_HVALUE hValue, FX_FLOAT fFloat);
void FXJSE_Value_SetDouble(FXJSE_HVALUE hValue, double dDouble);
void FXJSE_Value_SetObject(FXJSE_HVALUE hValue,
                           void* lpObject,
                           FXJSE_HCLASS hClass);
void FXJSE_Value_SetArray(FXJSE_HVALUE hValue,
                          uint32_t uValueCount,
                          FXJSE_HVALUE* rgValues);
void FXJSE_Value_SetDate(FXJSE_HVALUE hValue, double dDouble);
void FXJSE_Value_Set(FXJSE_HVALUE hValue, FXJSE_HVALUE hOriginalValue);

FX_BOOL FXJSE_Value_GetObjectProp(FXJSE_HVALUE hValue,
                                  const CFX_ByteStringC& szPropName,
                                  FXJSE_HVALUE hPropValue);
FX_BOOL FXJSE_Value_SetObjectProp(FXJSE_HVALUE hValue,
                                  const CFX_ByteStringC& szPropName,
                                  FXJSE_HVALUE hPropValue);
FX_BOOL FXJSE_Value_GetObjectPropByIdx(FXJSE_HVALUE hValue,
                                       uint32_t uPropIdx,
                                       FXJSE_HVALUE hPropValue);
FX_BOOL FXJSE_Value_SetObjectPropByIdx(FXJSE_HVALUE hValue,
                                       uint32_t uPropIdx,
                                       FXJSE_HVALUE hPropValue);
FX_BOOL FXJSE_Value_DeleteObjectProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName);
FX_BOOL FXJSE_Value_ObjectHasOwnProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName,
                                     FX_BOOL bUseTypeGetter);
FX_BOOL FXJSE_Value_SetObjectOwnProp(FXJSE_HVALUE hValue,
                                     const CFX_ByteStringC& szPropName,
                                     FXJSE_HVALUE hPropValue);

FX_BOOL FXJSE_Value_CallFunction(FXJSE_HVALUE hFunction,
                                 FXJSE_HVALUE hThis,
                                 FXJSE_HVALUE hRetValue,
                                 uint32_t nArgCount,
                                 FXJSE_HVALUE* lpArgs);
FX_BOOL FXJSE_Value_SetFunctionBind(FXJSE_HVALUE hValue,
                                    FXJSE_HVALUE hOldFunction,
                                    FXJSE_HVALUE hNewThis);

FX_BOOL FXJSE_ExecuteScript(FXJSE_HCONTEXT hContext,
                            const FX_CHAR* szScript,
                            FXJSE_HVALUE hRetValue,
                            FXJSE_HVALUE hNewThisObject = nullptr);

void FXJSE_ThrowMessage(const CFX_ByteStringC& utf8Name,
                        const CFX_ByteStringC& utf8Message);

FX_BOOL FXJSE_ReturnValue_GetMessage(FXJSE_HVALUE hRetValue,
                                     CFX_ByteString& utf8Name,
                                     CFX_ByteString& utf8Message);
FX_BOOL FXJSE_ReturnValue_GetLineInfo(FXJSE_HVALUE hRetValue,
                                      int32_t& nLine,
                                      int32_t& nCol);

#endif  // XFA_FXJSE_INCLUDE_FXJSE_H_
