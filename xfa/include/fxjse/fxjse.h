// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXJSE_H_
#define _FXJSE_H_
#ifdef __cplusplus
#define FXJSE_DEFINEINHERITHANDLE(name, subtypename) typedef struct _##subtypename : public _##name {} * subtypename;
#else
#define FXJSE_DEFINEINHERITHANDLE(name, subtypename) typedef name subtypename;
#endif
FX_DEFINEHANDLE(FXJSE_HRUNTIME);
FX_DEFINEHANDLE(FXJSE_HCONTEXT);
FX_DEFINEHANDLE(FXJSE_HCLASS);
FX_DEFINEHANDLE(FXJSE_HVALUE);
FXJSE_DEFINEINHERITHANDLE(FXJSE_HVALUE, FXJSE_HOBJECT);
typedef double FXJSE_DOUBLE;
void FXJSE_Initialize();
void FXJSE_Finalize();
FXJSE_HRUNTIME	FXJSE_Runtime_Create();
void			FXJSE_Runtime_Release(FXJSE_HRUNTIME hRuntime);
typedef struct _FXJSE_CLASS FXJSE_CLASS;
FXJSE_HCONTEXT	FXJSE_Context_Create	(FXJSE_HRUNTIME hRuntime, const FXJSE_CLASS* lpGlobalClass = NULL, FX_LPVOID lpGlobalObject = NULL);
void			FXJSE_Context_Release	(FXJSE_HCONTEXT hContext);
FXJSE_HVALUE	FXJSE_Context_GetGlobalObject(FXJSE_HCONTEXT hContext);
FXJSE_HRUNTIME  FXJSE_Context_GetRuntime(FXJSE_HCONTEXT hContext);
enum FXJSE_CompatibleModeFlags {
    FXJSE_COMPATIBLEMODEFLAG_CONSTRUCTOREXTRAMETHODS = (1 << 0),
    FXJSE_COMPATIBLEMODEFLAGCOUNT = 1,
};
void			FXJSE_Context_EnableCompatibleMode(FXJSE_HCONTEXT hContext, FX_DWORD dwCompatibleFlags);
class CFXJSE_Arguments
{
public:
    FXJSE_HRUNTIME	GetRuntime() const;
    FX_INT32		GetLength() const;
    FXJSE_HVALUE	GetValue(FX_INT32 index) const;
    FX_BOOL			GetBoolean(FX_INT32 index) const;
    FX_INT32		GetInt32(FX_INT32 index) const;
    FX_FLOAT		GetFloat(FX_INT32 index) const;
    CFX_ByteString	GetUTF8String(FX_INT32 index) const;
    FX_LPVOID		GetObject(FX_INT32 index, FXJSE_HCLASS hClass = NULL) const;
    FXJSE_HVALUE	GetReturnValue();
};
typedef void	(*FXJSE_FuncCallback)	(FXJSE_HOBJECT hThis,	FX_BSTR szFuncName, CFXJSE_Arguments &args);
typedef void	(*FXJSE_PropAccessor)	(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FXJSE_HVALUE hValue);
typedef FX_INT32(*FXJSE_PropTypeGetter)	(FXJSE_HOBJECT hObject, FX_BSTR szPropName, FX_BOOL bQueryIn);
typedef FX_BOOL (*FXJSE_PropDeleter)	(FXJSE_HOBJECT hObject, FX_BSTR szPropName);
typedef struct _FXJSE_FUNCTION {
    FX_LPCSTR				name;
    FXJSE_FuncCallback		callbackProc;
} FXJSE_FUNCTION;
#define FXJSE_DEF_FUNCTION(functionName, functionCallback) {functionName, functionCallback}
void	FXJSE_DefineFunctions(FXJSE_HCONTEXT hContext, const FXJSE_FUNCTION* lpFunctions, int nNum);
typedef struct _FXJSE_PROPERTY {
    FX_LPCSTR				name;
    FXJSE_PropAccessor		getProc;
    FXJSE_PropAccessor		setProc;
} FXJSE_PROPERTY;
enum FXJSE_ClassPropTypes {
    FXJSE_ClassPropType_None,
    FXJSE_ClassPropType_Property,
    FXJSE_ClassPropType_Method
};
typedef struct _FXJSE_CLASS {
    FX_LPCSTR				name;
    FXJSE_FuncCallback		constructor;
    FXJSE_PROPERTY*			properties;
    FXJSE_FUNCTION*			methods;
    FX_INT32				propNum;
    FX_INT32				methNum;
    FXJSE_PropTypeGetter	dynPropTypeGetter;
    FXJSE_PropAccessor		dynPropGetter;
    FXJSE_PropAccessor		dynPropSetter;
    FXJSE_PropDeleter		dynPropDeleter;
    FXJSE_FuncCallback		dynMethodCall;
} FXJSE_CLASS;
FXJSE_HCLASS	FXJSE_DefineClass(FXJSE_HCONTEXT hContext, const FXJSE_CLASS* lpClass);
FXJSE_HCLASS	FXJSE_GetClass(FXJSE_HCONTEXT hContext, FX_BSTR szName);
FXJSE_HVALUE	FXJSE_Value_Create(FXJSE_HRUNTIME hRuntime);
void			FXJSE_Value_Release(FXJSE_HVALUE hValue);
FXJSE_HRUNTIME	FXJSE_Value_GetRuntime(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsUndefined		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsNull			(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsBoolean		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsUTF8String	(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsNumber		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsInteger		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsObject		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsArray			(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsFunction		(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_IsDate			(FXJSE_HVALUE hValue);
FX_BOOL			FXJSE_Value_ToBoolean		(FXJSE_HVALUE hValue);
FX_FLOAT		FXJSE_Value_ToFloat			(FXJSE_HVALUE hValue);
FXJSE_DOUBLE	FXJSE_Value_ToDouble		(FXJSE_HVALUE hValue);
FX_INT32		FXJSE_Value_ToInteger		(FXJSE_HVALUE hValue);
void			FXJSE_Value_ToUTF8String	(FXJSE_HVALUE hValue, CFX_ByteString& szStrOutput);
FX_LPVOID		FXJSE_Value_ToObject		(FXJSE_HVALUE hValue, FXJSE_HCLASS hClass);
void			FXJSE_Value_SetUndefined	(FXJSE_HVALUE hValue);
void			FXJSE_Value_SetNull			(FXJSE_HVALUE hValue);
void			FXJSE_Value_SetBoolean		(FXJSE_HVALUE hValue, FX_BOOL	   bBoolean);
void			FXJSE_Value_SetUTF8String	(FXJSE_HVALUE hValue, FX_BSTR	   szString);
void			FXJSE_Value_SetInteger		(FXJSE_HVALUE hValue, FX_INT32     nInteger);
void			FXJSE_Value_SetFloat		(FXJSE_HVALUE hValue, FX_FLOAT     fFloat);
void			FXJSE_Value_SetDouble		(FXJSE_HVALUE hValue, FXJSE_DOUBLE dDouble);
void			FXJSE_Value_SetObject		(FXJSE_HVALUE hValue, FX_LPVOID lpObject, FXJSE_HCLASS hClass);
void			FXJSE_Value_SetArray		(FXJSE_HVALUE hValue, FX_UINT32 uValueCount, FXJSE_HVALUE* rgValues);
void			FXJSE_Value_SetDate			(FXJSE_HVALUE hValue, FXJSE_DOUBLE dDouble);
void			FXJSE_Value_Set				(FXJSE_HVALUE hValue, FXJSE_HVALUE hOriginalValue);
FX_BOOL			FXJSE_Value_GetObjectProp		(FXJSE_HVALUE hValue, FX_BSTR	szPropName,	FXJSE_HVALUE hPropValue);
FX_BOOL			FXJSE_Value_SetObjectProp		(FXJSE_HVALUE hValue, FX_BSTR	szPropName,	FXJSE_HVALUE hPropValue);
FX_BOOL			FXJSE_Value_GetObjectPropByIdx	(FXJSE_HVALUE hValue, FX_UINT32	uPropIdx,	FXJSE_HVALUE hPropValue);
FX_BOOL			FXJSE_Value_SetObjectPropByIdx	(FXJSE_HVALUE hValue, FX_UINT32	uPropIdx,	FXJSE_HVALUE hPropValue);
FX_BOOL			FXJSE_Value_DeleteObjectProp	(FXJSE_HVALUE hValue, FX_BSTR	szPropName);
FX_BOOL			FXJSE_Value_ObjectHasOwnProp	(FXJSE_HVALUE hValue, FX_BSTR	szPropName, FX_BOOL	bUseTypeGetter);
FX_BOOL			FXJSE_Value_SetObjectOwnProp	(FXJSE_HVALUE hValue, FX_BSTR	szPropName, FXJSE_HVALUE hPropValue);
FX_BOOL			FXJSE_Value_CallFunction		(FXJSE_HVALUE hFunction, FXJSE_HVALUE hThis, FXJSE_HVALUE hRetValue, FX_UINT32 nArgCount, FXJSE_HVALUE* lpArgs);
FX_BOOL			FXJSE_Value_SetFunctionBind	(FXJSE_HVALUE hValue, FXJSE_HVALUE hOldFunction, FXJSE_HVALUE hNewThis);
FX_BOOL			FXJSE_ExecuteScript(FXJSE_HCONTEXT hContext, FX_LPCSTR szScript, FXJSE_HVALUE hRetValue, FXJSE_HVALUE hNewThisObject = NULL);
void			FXJSE_ThrowMessage(FX_BSTR utf8Name, FX_BSTR utf8Message);
FX_BOOL			FXJSE_ReturnValue_GetMessage(FXJSE_HVALUE hRetValue, CFX_ByteString& utf8Name, CFX_ByteString& utf8Message);
FX_BOOL			FXJSE_ReturnValue_GetLineInfo(FXJSE_HVALUE hRetValue, FX_INT32& nLine, FX_INT32& nCol);
#endif
