// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../core/include/fxcrt/fx_basic.h"
#include "../../../core/include/fxcrt/fx_ext.h" 
#include "../../include/jsapi/fxjs_v8.h"
#include "../../include/fsdk_define.h"
#include "time.h"
#include <cmath>
#include <limits>

#define VALUE_NAME_STRING		L"string"
#define VALUE_NAME_NUMBER		L"number"
#define VALUE_NAME_BOOLEAN		L"boolean"
#define VALUE_NAME_DATE			L"date"
#define VALUE_NAME_OBJECT		L"object"
#define VALUE_NAME_FXOBJ		L"fxobj"
#define VALUE_NAME_NULL			L"null"
#define VALUE_NAME_UNDEFINED	L"undefined"

const static FX_DWORD g_nan[2] = {0,0x7FF80000 };
static double GetNan()
{
  return *(double*)g_nan;
}


class CJS_PrivateData: public CFX_Object
{
public:
	CJS_PrivateData():ObjDefID(-1), pPrivate(NULL) {}
	int ObjDefID;
	FX_LPVOID	pPrivate;
};


class CJS_ObjDefintion: public CFX_Object
{
public:
	CJS_ObjDefintion(v8::Isolate* isolate, const wchar_t* sObjName, FXJSOBJTYPE eObjType, LP_CONSTRUCTOR pConstructor, LP_DESTRUCTOR pDestructor, unsigned bApplyNew):
	  objName(sObjName), objType(eObjType), m_pConstructor(pConstructor), m_pDestructor(pDestructor),m_bApplyNew(bApplyNew),m_bSetAsGlobalObject(FALSE)
	  {
		  v8::Isolate::Scope isolate_scope(isolate);
		  v8::HandleScope handle_scope(isolate);

		  v8::Handle<v8::ObjectTemplate> objTemplate = v8::ObjectTemplate::New(isolate);
		  objTemplate->SetInternalFieldCount(1);
		  m_objTemplate.Reset(isolate, objTemplate);

		 //Document as the global object.
		  if(FXSYS_wcscmp(sObjName, L"Document") == 0)
		  {
			 m_bSetAsGlobalObject = TRUE;
		  }

	  }
	  ~CJS_ObjDefintion()
	  {
		  m_objTemplate.Reset();
		  m_StaticObj.Reset();
	  }
public:
	const wchar_t* objName;
	FXJSOBJTYPE objType;
	LP_CONSTRUCTOR m_pConstructor;
	LP_DESTRUCTOR m_pDestructor;
	unsigned m_bApplyNew;
	FX_BOOL	m_bSetAsGlobalObject;

	v8::Persistent<v8::ObjectTemplate> m_objTemplate;
	v8::Persistent<v8::Object> m_StaticObj;
};

int JS_DefineObj(IJS_Runtime* pJSRuntime, const wchar_t* sObjName, FXJSOBJTYPE eObjType, LP_CONSTRUCTOR pConstructor, LP_DESTRUCTOR pDestructor, unsigned bApplyNew)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) 
	{
		pArray = FX_NEW CFX_PtrArray();
		isolate->SetData(0, pArray);
	}
	CJS_ObjDefintion* pObjDef = FX_NEW CJS_ObjDefintion(isolate, sObjName, eObjType, pConstructor, pDestructor, bApplyNew);
	pArray->Add(pObjDef);
	return pArray->GetSize()-1;
}

int JS_DefineObjMethod(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sMethodName, v8::FunctionCallback pMethodCall, unsigned nParamNum)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_WideString ws = CFX_WideString(sMethodName);
	CFX_ByteString bsMethodName = ws.UTF8Encode();

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return 0;

	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return 0;
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);
	v8::Local<v8::ObjectTemplate> objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, pObjDef->m_objTemplate);
	objTemp->Set(v8::String::NewFromUtf8(isolate, FX_LPCSTR(bsMethodName)), v8::FunctionTemplate::New(isolate, pMethodCall), v8::ReadOnly);
	pObjDef->m_objTemplate.Reset(isolate,objTemp);
	return 0;
}

int JS_DefineObjProperty(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sPropName, v8::AccessorGetterCallback pPropGet, v8::AccessorSetterCallback pPropPut)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_WideString ws = CFX_WideString(sPropName);
	CFX_ByteString bsPropertyName = ws.UTF8Encode();

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return 0;

	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return 0;
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);
	v8::Local<v8::ObjectTemplate> objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, pObjDef->m_objTemplate);
	objTemp->SetAccessor(v8::String::NewFromUtf8(isolate, FX_LPCSTR(bsPropertyName)), pPropGet, pPropPut);
	pObjDef->m_objTemplate.Reset(isolate,objTemp);
	return 0;
}

int	JS_DefineObjAllProperties(IJS_Runtime* pJSRuntime, int nObjDefnID, v8::NamedPropertyQueryCallback pPropQurey, v8::NamedPropertyGetterCallback pPropGet, v8::NamedPropertySetterCallback pPropPut, v8::NamedPropertyDeleterCallback pPropDel)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return 0;

	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return 0;
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);
	v8::Local<v8::ObjectTemplate> objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, pObjDef->m_objTemplate);
	objTemp->SetNamedPropertyHandler(pPropGet, pPropPut, pPropQurey, pPropDel);
	pObjDef->m_objTemplate.Reset(isolate,objTemp);
	return 0;
}

int JS_DefineObjConst(IJS_Runtime* pJSRuntime, int nObjDefnID, const wchar_t* sConstName, v8::Handle<v8::Value> pDefault)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return 0;

	CFX_WideString ws = CFX_WideString(sConstName);
	CFX_ByteString bsConstName = ws.UTF8Encode();

	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return 0;
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);
	v8::Local<v8::ObjectTemplate> objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, pObjDef->m_objTemplate);
	objTemp->Set(isolate, FX_LPCSTR(bsConstName), pDefault);
	pObjDef->m_objTemplate.Reset(isolate,objTemp);
	return 0;
}

static v8::Persistent<v8::ObjectTemplate>& _getGlobalObjectTemplate(IJS_Runtime* pJSRuntime)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	ASSERT(pArray != NULL);
	for(int i=0; i<pArray->GetSize(); i++)
	{
		CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(i);
		if(pObjDef->m_bSetAsGlobalObject)
			return pObjDef->m_objTemplate;
	}
	static v8::Persistent<v8::ObjectTemplate> gloabalObjectTemplate;
	return gloabalObjectTemplate;
}

int JS_DefineGlobalMethod(IJS_Runtime* pJSRuntime, const wchar_t* sMethodName, v8::FunctionCallback pMethodCall, unsigned nParamNum)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_WideString ws = CFX_WideString(sMethodName);
	CFX_ByteString bsMethodName = ws.UTF8Encode();

	v8::Local<v8::FunctionTemplate> funTempl = v8::FunctionTemplate::New(isolate, pMethodCall);
	v8::Local<v8::ObjectTemplate> objTemp;

	v8::Persistent<v8::ObjectTemplate>& globalObjTemp = _getGlobalObjectTemplate(pJSRuntime);
	if(globalObjTemp.IsEmpty())
		objTemp = v8::ObjectTemplate::New(isolate);
	else
		objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, globalObjTemp);
	objTemp->Set(v8::String::NewFromUtf8(isolate, FX_LPCSTR(bsMethodName)), funTempl, v8::ReadOnly);

	globalObjTemp.Reset(isolate,objTemp);

	return 0;
}

int JS_DefineGlobalConst(IJS_Runtime* pJSRuntime, const wchar_t* sConstName, v8::Handle<v8::Value> pDefault)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	CFX_WideString ws = CFX_WideString(sConstName);
	CFX_ByteString bsConst= ws.UTF8Encode();

	v8::Local<v8::ObjectTemplate> objTemp;

	v8::Persistent<v8::ObjectTemplate>& globalObjTemp = _getGlobalObjectTemplate(pJSRuntime);
	if(globalObjTemp.IsEmpty())
		objTemp = v8::ObjectTemplate::New(isolate);
	else
		objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, globalObjTemp);
	objTemp->Set(v8::String::NewFromUtf8(isolate, FX_LPCSTR(bsConst)), pDefault, v8::ReadOnly);

	globalObjTemp.Reset(isolate,objTemp);

	return 0;
}


void JS_InitialRuntime(IJS_Runtime* pJSRuntime,IFXJS_Runtime* pFXRuntime, IFXJS_Context* context, v8::Persistent<v8::Context>& v8PersistentContext)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);

	v8::Persistent<v8::ObjectTemplate>& globalObjTemp = _getGlobalObjectTemplate(pJSRuntime);
	v8::Handle<v8::Context> v8Context = v8::Context::New(isolate, NULL, v8::Local<v8::ObjectTemplate>::New(isolate, globalObjTemp));
	v8::Context::Scope context_scope(v8Context);

	v8::Handle<v8::External> ptr = v8::External::New(isolate, pFXRuntime);
	v8Context->SetEmbedderData(1, ptr);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return;

	for(int i=0; i<pArray->GetSize(); i++)
	{
		CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(i);
		CFX_WideString ws = CFX_WideString(pObjDef->objName);
		CFX_ByteString bs = ws.UTF8Encode();
		v8::Handle<v8::String> objName = v8::String::NewFromUtf8(isolate, bs.c_str(), v8::String::kNormalString, bs.GetLength());


		if(pObjDef->objType == JS_DYNAMIC)
		{
			//Document is set as global object, need to construct it first.
			if(ws.Equal(L"Document"))
			{

				CJS_PrivateData* pPrivateData = FX_NEW CJS_PrivateData;
				pPrivateData->ObjDefID = i;
				v8::Handle<v8::External> ptr = v8::External::New(isolate, pPrivateData);

				v8Context->Global()->GetPrototype()->ToObject()->SetInternalField(0, ptr); 

				if(pObjDef->m_pConstructor)
					pObjDef->m_pConstructor(context, v8Context->Global()->GetPrototype()->ToObject(), v8Context->Global()->GetPrototype()->ToObject());
			}
		}
		else
		{
			v8::Handle<v8::Object> obj = JS_NewFxDynamicObj(pJSRuntime, context, i);
			v8Context->Global()->Set(objName, obj);
			pObjDef->m_StaticObj.Reset(isolate, obj);
		}
	}
	v8PersistentContext.Reset(isolate, v8Context);
}

void JS_ReleaseRuntime(IJS_Runtime* pJSRuntime, v8::Persistent<v8::Context>& v8PersistentContext)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::HandleScope handle_scope(isolate);
	v8::Local<v8::Context> context = v8::Local<v8::Context>::New(isolate, v8PersistentContext);
	v8::Context::Scope context_scope(context);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return ;

	for(int i=0; i<pArray->GetSize(); i++)
	{
		CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(i);
		if(!pObjDef->m_StaticObj.IsEmpty())
		{
			v8::Local<v8::Object> pObj = v8::Local<v8::Object>::New(isolate, pObjDef->m_StaticObj);
			if(pObjDef->m_pDestructor)
				pObjDef->m_pDestructor(pObj);
			JS_FreePrivate(pObj);
		}
		delete pObjDef;
	}
	delete pArray;
	isolate->SetData(0,NULL);
}

void JS_Initial() 
{
}
void JS_Release()
{

}
int JS_Parse(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, const wchar_t* script, long length, FXJSErr* perror)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::TryCatch try_catch;

	CFX_WideString wsScript(script);
	CFX_ByteString bsScript = wsScript.UTF8Encode();


	v8::Handle<v8::Script> compiled_script = v8::Script::Compile(v8::String::NewFromUtf8(isolate, bsScript.c_str(), v8::String::kNormalString, bsScript.GetLength()));
	if (compiled_script.IsEmpty()) {
		v8::String::Utf8Value error(try_catch.Exception());
		return -1;
	}
	return 0;
}

int JS_Execute(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, const wchar_t* script, long length, FXJSErr* perror)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	v8::TryCatch try_catch;

	CFX_WideString wsScript(script);
	CFX_ByteString bsScript = wsScript.UTF8Encode();

    v8::Handle<v8::Script> compiled_script = v8::Script::Compile(v8::String::NewFromUtf8(isolate, bsScript.c_str(), v8::String::kNormalString, bsScript.GetLength()));
	if (compiled_script.IsEmpty()) {
		v8::String::Utf8Value error(try_catch.Exception());
		return -1;
	}

	v8::Handle<v8::Value> result = compiled_script->Run();
	if (result.IsEmpty()) {
		v8::String::Utf8Value error(try_catch.Exception());
		return -1;
	}
	return 0;
}

v8::Handle<v8::Object> JS_NewFxDynamicObj(IJS_Runtime* pJSRuntime, IFXJS_Context* pJSContext, int nObjDefnID)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);
	if(-1 == nObjDefnID)
	{
		v8::Local<v8::ObjectTemplate> objTempl = v8::ObjectTemplate::New(isolate);
		return objTempl->NewInstance();
	}

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return v8::Handle<v8::Object>();


	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return v8::Handle<v8::Object>();
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	v8::Local<v8::ObjectTemplate> objTemp = v8::Local<v8::ObjectTemplate>::New(isolate, pObjDef->m_objTemplate);

	v8::Local<v8::Object> obj = objTemp->NewInstance();
	
	CJS_PrivateData* pPrivateData = FX_NEW CJS_PrivateData;
	pPrivateData->ObjDefID = nObjDefnID;
	v8::Handle<v8::External> ptr = v8::External::New(isolate, pPrivateData);
	obj->SetInternalField(0, ptr); 

	if(pObjDef->m_pConstructor)
		pObjDef->m_pConstructor(pJSContext, obj, context->Global()->GetPrototype()->ToObject());

	return obj;
}

v8::Handle<v8::Object> JS_GetStaticObj(IJS_Runtime* pJSRuntime, int nObjDefnID)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return v8::Handle<v8::Object>();

	if(nObjDefnID<0 || nObjDefnID>= pArray->GetSize()) return v8::Handle<v8::Object>();
	CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(nObjDefnID);
	v8::Local<v8::Object> obj = v8::Local<v8::Object>::New(isolate,pObjDef->m_StaticObj);
	return obj;
}

void JS_SetThisObj(IJS_Runtime* pJSRuntime, int nThisObjID)
{
	//Do nothing.
}
v8::Handle<v8::Object>	JS_GetThisObj(IJS_Runtime * pJSRuntime)
{
	//Return the global object.
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return v8::Handle<v8::Object>();

	v8::Local<v8::Context> context = isolate->GetCurrentContext();
	return context->Global()->GetPrototype()->ToObject();
}

int	JS_GetObjDefnID(v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty() || !pObj->InternalFieldCount()) return -1;
	v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(pObj->GetInternalField(0));
	CJS_PrivateData* pPrivateData = (CJS_PrivateData*)field->Value();
	if(pPrivateData)
		return pPrivateData->ObjDefID;
	return -1;
}

IJS_Runtime* JS_GetRuntime(v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty()) return NULL;
	v8::Local<v8::Context> context = pObj->CreationContext();
	if(context.IsEmpty()) return NULL;
	return context->GetIsolate();
}

int JS_GetObjDefnID(IJS_Runtime * pJSRuntime, const wchar_t* pObjName)
{
	v8::Isolate* isolate = (v8::Isolate*)pJSRuntime;
	v8::Isolate::Scope isolate_scope(isolate);

	CFX_PtrArray* pArray = (CFX_PtrArray*)isolate->GetData(0);
	if(!pArray) return -1;

	for(int i=0; i<pArray->GetSize(); i++)
	{
		CJS_ObjDefintion* pObjDef = (CJS_ObjDefintion*)pArray->GetAt(i);
		if(FXSYS_wcscmp(pObjDef->objName, pObjName) == 0)
			return i;
	}
	return -1;
}

void JS_Error(v8::Value * pError,const wchar_t * main,const wchar_t * sub)
{

}

unsigned JS_CalcHash(const wchar_t* main, unsigned nLen)
{
	return (unsigned)FX_HashCode_String_GetW(main, nLen);
}

unsigned JS_CalcHash(const wchar_t* main)
{
	return (unsigned)FX_HashCode_String_GetW(main, FXSYS_wcslen(main));
}
const wchar_t*	JS_GetTypeof(v8::Handle<v8::Value> pObj)
{
	if(pObj.IsEmpty()) return NULL;
	if(pObj->IsString())
		return VALUE_NAME_STRING;
	if(pObj->IsNumber())
		return VALUE_NAME_NUMBER;
	if(pObj->IsBoolean())
		return VALUE_NAME_BOOLEAN;
	if(pObj->IsDate())
		return VALUE_NAME_DATE;
	if(pObj->IsObject())
		return VALUE_NAME_OBJECT;
	if(pObj->IsNull())
		return VALUE_NAME_NULL;
	if(pObj->IsUndefined())
		return VALUE_NAME_UNDEFINED;
	return NULL;

}
const wchar_t*	JS_GetClassname(v8::Handle<v8::Object> pObj)
{
	return NULL;
}

void JS_SetPrivate(v8::Handle<v8::Object> pObj, void* p)
{
	JS_SetPrivate(NULL, pObj, p);
}

void* JS_GetPrivate(v8::Handle<v8::Object> pObj)
{
	return JS_GetPrivate(NULL,pObj);
}

void JS_SetPrivate(IJS_Runtime* pJSRuntime, v8::Handle<v8::Object> pObj, void* p)
{
	if(pObj.IsEmpty() || !pObj->InternalFieldCount()) return;
	v8::Handle<v8::External> ptr = v8::Handle<v8::External>::Cast(pObj->GetInternalField(0));
	CJS_PrivateData* pPrivateData  = (CJS_PrivateData*)ptr->Value();
	if(!pPrivateData) return;
	pPrivateData->pPrivate = p;
}

void* JS_GetPrivate(IJS_Runtime* pJSRuntime, v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty()) return NULL;
	v8::Local<v8::Value> value;
	if(pObj->InternalFieldCount())
		value = pObj->GetInternalField(0); 
	else
	{
		//It could be a global proxy object.
		v8::Local<v8::Value> v = pObj->GetPrototype();
		if(v->IsObject())
			value = v->ToObject()->GetInternalField(0);
	}
	if(value.IsEmpty() || value->IsUndefined()) return NULL;
	v8::Handle<v8::External> ptr = v8::Handle<v8::External>::Cast(value);
	CJS_PrivateData* pPrivateData  = (CJS_PrivateData*)ptr->Value();
	if(!pPrivateData) return NULL;
	return pPrivateData->pPrivate;
}

void JS_FreePrivate(v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty() || !pObj->InternalFieldCount()) return;
	v8::Handle<v8::External> ptr = v8::Handle<v8::External>::Cast(pObj->GetInternalField(0));
	delete (CJS_PrivateData*)ptr->Value();
	v8::Local<v8::Context> context = pObj->CreationContext();

	pObj->SetInternalField(0, v8::External::New(context->GetIsolate(), NULL));
}


v8::Handle<v8::Value> JS_GetObjectValue(v8::Handle<v8::Object> pObj)
{
	return pObj;
}

v8::Handle<v8::String> WSToJSString(IJS_Runtime* pJSRuntime, const wchar_t* PropertyName, int Len = -1)
{
	CFX_WideString ws = CFX_WideString(PropertyName,Len);
	CFX_ByteString bs = ws.UTF8Encode();
	if(!pJSRuntime) pJSRuntime = v8::Isolate::GetCurrent();
	return v8::String::NewFromUtf8(pJSRuntime, bs.c_str());
}

v8::Handle<v8::Value> JS_GetObjectElement(IJS_Runtime* pJSRuntime, v8::Handle<v8::Object> pObj,const wchar_t* PropertyName)
{
	if(pObj.IsEmpty()) return v8::Handle<v8::Value>();
	return pObj->Get(WSToJSString(pJSRuntime,PropertyName));
}

v8::Handle<v8::Array> JS_GetObjectElementNames(v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty()) return v8::Handle<v8::Array>();
	return pObj->GetPropertyNames();
}

void JS_PutObjectString(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, const wchar_t* sValue) //VT_string
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime, PropertyName), WSToJSString(pJSRuntime, sValue));
}

void JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, int nValue)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),v8::Int32::New(pJSRuntime, nValue));
}

void JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, float fValue)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),v8::Number::New(pJSRuntime, (double)fValue));
}

void JS_PutObjectNumber(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, double dValue)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),v8::Number::New(pJSRuntime, (double)dValue));
}

void JS_PutObjectBoolean(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, bool bValue)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),v8::Boolean::New(pJSRuntime, bValue));
}

void JS_PutObjectObject(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName, v8::Handle<v8::Object> pPut)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),pPut);
}

void JS_PutObjectNull(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj, const wchar_t* PropertyName)
{
	if(pObj.IsEmpty()) return;
	pObj->Set(WSToJSString(pJSRuntime,PropertyName),v8::Handle<v8::Object>());
}

v8::Handle<v8::Array> JS_NewArray(IJS_Runtime* pJSRuntime)
{
	return v8::Array::New(pJSRuntime);
}

unsigned JS_PutArrayElement(v8::Handle<v8::Array> pArray,unsigned index,v8::Handle<v8::Value> pValue,FXJSVALUETYPE eType)
{	
	if(pArray.IsEmpty()) return 0;
	pArray->Set(index, pValue);
	return 1;
}

v8::Handle<v8::Value> JS_GetArrayElemnet(v8::Handle<v8::Array> pArray,unsigned index)
{
	if(pArray.IsEmpty()) return v8::Handle<v8::Value>();
	return pArray->Get(index);
}

unsigned JS_GetArrayLength(v8::Handle<v8::Array> pArray)
{
	if(pArray.IsEmpty()) return 0;
	return pArray->Length();
}

v8::Handle<v8::Value> JS_NewNumber(IJS_Runtime* pJSRuntime,int number)
{
	return v8::Int32::New(pJSRuntime, number);
}

v8::Handle<v8::Value> JS_NewNumber(IJS_Runtime* pJSRuntime,double number)
{
	return v8::Number::New(pJSRuntime, number);
}

v8::Handle<v8::Value> JS_NewNumber(IJS_Runtime* pJSRuntime,float number)
{
	return v8::Number::New(pJSRuntime, (float)number);
}

v8::Handle<v8::Value> JS_NewBoolean(IJS_Runtime* pJSRuntime,bool b)
{
	return v8::Boolean::New(pJSRuntime, b);
}

v8::Handle<v8::Value> JS_NewObject(IJS_Runtime* pJSRuntime,v8::Handle<v8::Object> pObj)
{
	if(pObj.IsEmpty()) return v8::Handle<v8::Value>();
	return pObj->Clone();
}

v8::Handle<v8::Value> JS_NewObject2(IJS_Runtime* pJSRuntime,v8::Handle<v8::Array> pObj)
{
	if(pObj.IsEmpty()) return v8::Handle<v8::Value>();
	return pObj->Clone();
}


v8::Handle<v8::Value> JS_NewString(IJS_Runtime* pJSRuntime,const wchar_t* string)
{
	return WSToJSString(pJSRuntime, string);
}

v8::Handle<v8::Value> JS_NewString(IJS_Runtime* pJSRuntime,const wchar_t* string, unsigned nLen)
{
	return WSToJSString(pJSRuntime, string, nLen);
}

v8::Handle<v8::Value> JS_NewNull()
{
	return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> JS_NewDate(IJS_Runtime* pJSRuntime,double d)
{
	return v8::Date::New(pJSRuntime, d);
}

v8::Handle<v8::Value> JS_NewValue(IJS_Runtime* pJSRuntime)
{
	return v8::Handle<v8::Value>();
}

v8::Handle<v8::Value> JS_GetListValue(v8::Handle<v8::Value> pList, int index)
{

	if(!pList.IsEmpty() && pList->IsObject())
	{
		v8::Local<v8::Object> obj = pList->ToObject();
		return obj->Get(index);
	}
	return v8::Handle<v8::Value>();
}

int	JS_ToInt32(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return 0;
	return pValue->ToInt32()->Value();
}

bool JS_ToBoolean(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return false;
	return pValue->ToBoolean()->Value();
}

double JS_ToNumber(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return 0.0;
	return pValue->ToNumber()->Value();
}

v8::Handle<v8::Object> JS_ToObject(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return v8::Handle<v8::Object>();
	return pValue->ToObject();
}

CFX_WideString	JS_ToString(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return L"";
	v8::String::Utf8Value s(pValue->ToString());
	return CFX_WideString::FromUTF8(*s, s.length());
}

v8::Handle<v8::Array> JS_ToArray(v8::Handle<v8::Value> pValue)
{
	if(pValue.IsEmpty()) return v8::Handle<v8::Array>();
	return v8::Handle<v8::Array>::Cast(pValue->ToObject());
}

void JS_ValueCopy(v8::Handle<v8::Value>& pTo, v8::Handle<v8::Value> pFrom)
{
	pTo = pFrom;
}


//JavaScript time implement begin.

double _getLocalTZA()
{
	if(!FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
		return 0;
	time_t t = 0;
	time(&t);
	localtime(&t);
	return (double)(-(timezone * 1000));
}

int _getDaylightSavingTA(double d)
{
	if(!FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
		return 0;
	time_t t = (time_t)(d/1000);
	struct tm * tmp = localtime(&t);
	if (tmp == NULL)
		return 0;
	if (tmp->tm_isdst > 0)
		//One hour.
		return (int)60*60*1000;
	return 0;
}

double _Mod(double x, double y)
{   
	double r = fmod(x, y);
	if (r < 0) r += y;
	return r;
}

int _isfinite(double v)
{
#if _MSC_VER
	return ::_finite(v);
#else
	return std::fabs(v) < std::numeric_limits<double>::max();
#endif
}

double _toInteger(double n)
{
	return (n >= 0)? FXSYS_floor(n): -FXSYS_floor(-n);
}

bool _isLeapYear(int year)
{
	return (year%4==0)&&((year%100!=0)||(year%400!=0));
}

int _DayFromYear(int y)
{
	return (int)(365*(y - 1970.0) + FXSYS_floor((y - 1969.0)/4) - FXSYS_floor((y - 1901.0)/100)+FXSYS_floor((y - 1601.0)/400));
}

double _TimeFromYear(int y)
{
	return  ((double)86400000) * _DayFromYear(y);
}

double _TimeFromYearMonth(int y, int m)
{
	static int daysMonth[12] ={ 0,31,59,90,120,151,181,212,243,273,304,334};
	static int leapDaysMonth[12] = { 0,31,60,91,121,152,182,213,244,274,305,335};
	int* pMonth = daysMonth;
	if(_isLeapYear(y))
		pMonth = leapDaysMonth;
	return _TimeFromYear(y) + ((double)pMonth[m])*86400000;
}

int _Day(double t)
{
	return (int)FXSYS_floor(t / 86400000);
}

int _YearFromTime(double t)
{
	//estimate the time.
	int y = 1970 +(int)(t/(365.0*86400000));
	if (_TimeFromYear(y) <= t)
	{
		while(_TimeFromYear(y+1) <= t) y++;
	}
	else
		while(_TimeFromYear(y-1) > t) y--;
	return y;
}

int _DayWithinYear(double t)
{
	int year = _YearFromTime(t);
	int day = _Day(t);
	return day-_DayFromYear(year);
}

int _MonthFromTime(double t)
{
	int day = _DayWithinYear(t);
	int year = _YearFromTime(t);
	if(0<=day && day <31)
		return 0;
	if(31<=day && day< 59+_isLeapYear(year))
		return 1;
	if((59+_isLeapYear(year))<=day && day<(90+_isLeapYear(year)))
		return 2;
	if((90+_isLeapYear(year))<=day && day<(120+_isLeapYear(year)))
		return 3;
	if((120+_isLeapYear(year))<=day && day<(151+_isLeapYear(year)))
		return 4;
	if((151+_isLeapYear(year))<=day && day<(181+_isLeapYear(year)))
		return 5;
	if((181+_isLeapYear(year))<=day && day<(212+_isLeapYear(year)))
		return 6;
	if((212+_isLeapYear(year))<=day && day<(243+_isLeapYear(year)))
		return 7;
	if((243+_isLeapYear(year))<=day && day<(273+_isLeapYear(year)))
		return 8;
	if((273+_isLeapYear(year))<=day && day<(304+_isLeapYear(year)))
		return 9;
	if((304+_isLeapYear(year))<=day && day<(334+_isLeapYear(year)))
		return 10;
	if((334+_isLeapYear(year))<=day && day<(365+_isLeapYear(year)))
		return 11;

	return -1;
}

int _DateFromTime(double t)
{
	int day = _DayWithinYear(t);
	int year = _YearFromTime(t);
	bool leap = _isLeapYear(year);
	int month = _MonthFromTime(t);
	switch (month)
	{
	case 0:	 
		return day+1;
	case 1:	 
		return day-30;
	case 2:	 
		return day-58-leap;
	case 3:	 
		return day-89-leap;
	case 4:	 
		return day-119-leap;
	case 5:	 
		return day-150-leap;
	case 6:	 
		return day-180-leap;
	case 7:	 
		return day-211-leap;
	case 8:	 
		return day-242-leap;
	case 9:	 
		return day-272-leap;
	case 10: 
		return day-303-leap;
	case 11: 
		return day-333-leap;
	default:
		return 0;
	}
}

double JS_GetDateTime()
{
	if(!FSDK_IsSandBoxPolicyEnabled(FPDF_POLICY_MACHINETIME_ACCESS))
		return 0;
	time_t t = time(NULL);
	struct tm* pTm = localtime(&t);

	int year = pTm->tm_year+1900;
	double t1 = _TimeFromYear(year);

	return t1 + pTm->tm_yday*86400000.0 + pTm->tm_hour*3600000.0+pTm->tm_min*60000.0+pTm->tm_sec*1000.0;
}

int JS_GetYearFromTime(double dt)
{
	return _YearFromTime(dt);
}

int JS_GetMonthFromTime(double dt)
{
	return _MonthFromTime(dt);
}

int JS_GetDayFromTime(double dt)
{
	return _DateFromTime(dt);
}

int JS_GetHourFromTime(double dt)
{
	return (int)_Mod(FXSYS_floor((double)(dt/(60*60*1000))), 24);
}

int JS_GetMinFromTime(double dt)
{
	return (int)_Mod(FXSYS_floor((double)(dt/(60*1000))), 60);
}

int JS_GetSecFromTime(double dt)
{
	return (int)_Mod(FXSYS_floor((double)(dt/1000)), 60);
}

double JS_DateParse(const wchar_t* string)
{
	v8::Isolate* pIsolate = v8::Isolate::GetCurrent();
	v8::Isolate::Scope isolate_scope(pIsolate);
	v8::HandleScope scope(pIsolate);

	v8::Local<v8::Context> context = pIsolate->GetCurrentContext();
	
	//Use the built-in object method.
	v8::Local<v8::Value> v = context->Global()->Get(v8::String::NewFromUtf8(pIsolate, "Date"));
	if(v->IsObject())
	{
		v8::Local<v8::Object> o = v->ToObject();
		v = o->Get(v8::String::NewFromUtf8(pIsolate, "parse"));
		if(v->IsFunction())
		{
			v8::Local<v8::Function> funC = v8::Handle<v8::Function>::Cast(v);

			const int argc = 1;
			v8::Local<v8::String> timeStr = WSToJSString(pIsolate, string);
			v8::Handle<v8::Value> argv[argc] = {timeStr};
			v = funC->Call(context->Global(), argc, argv);
			if(v->IsNumber())
			{
				double date =  v->ToNumber()->Value();
				if(!_isfinite(date)) return date;
				return date + _getLocalTZA() + _getDaylightSavingTA(date);
			}

		}
	}
	return 0;
}

double JS_MakeDay(int nYear, int nMonth, int nDate)
{
	if (!_isfinite(nYear) || !_isfinite(nMonth) ||!_isfinite(nDate))
		return GetNan();
	double y = _toInteger(nYear);
	double m = _toInteger(nMonth);
	double dt = _toInteger(nDate);
	double ym = y + FXSYS_floor((double)m/12);
	double mn = _Mod(m ,12);

	double t = _TimeFromYearMonth((int)ym,(int)mn);

	if (_YearFromTime(t) != ym || _MonthFromTime(t) != mn ||_DateFromTime(t) != 1)
		return GetNan();
	return _Day(t)+dt-1;
}

double JS_MakeTime(int nHour, int nMin, int nSec, int nMs)
{
	if (!_isfinite(nHour) ||!_isfinite(nMin) ||!_isfinite(nSec) ||!_isfinite(nMs))
		return GetNan();

	double h = _toInteger(nHour);
	double m = _toInteger(nMin);
	double s = _toInteger(nSec);
	double milli = _toInteger(nMs);

	return h * 3600000 + m * 60000 + s * 1000 + milli;
}

double JS_MakeDate(double day, double time)
{
	if (!_isfinite(day) ||!_isfinite(time))
		return GetNan();

	return day * 86400000 + time;
}

bool JS_PortIsNan(double d)
{
	return d != d;
}

double JS_LocalTime(double d)
{
	return JS_GetDateTime() + _getDaylightSavingTA(d);
}

//JavaScript time implement End.
