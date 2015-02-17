// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _GLOBAL_H_
#define _GLOBAL_H_

class CJS_GlobalData;

struct js_global_data
{
	js_global_data()
	{
		nType = 0;
		dData = 0;
		bData = FALSE;
		sData = "";
		bPersistent = FALSE;
		bDeleted = FALSE;
	}

	~js_global_data()
	{
		pData.Reset();
	}
	int					nType; //0:int 1:bool 2:string 3:obj
	double				dData;
	bool				bData;
	CFX_ByteString		sData;
	v8::Persistent<v8::Object>  pData;
	bool				bPersistent;
	bool				bDeleted;
};

class global_alternate : public CJS_EmbedObj
{
public:
	global_alternate(CJS_Object* pJSObject);
	virtual ~global_alternate();

public:
	FX_BOOL						setPersistent(IFXJS_Context* cc, const CJS_Parameters& params, CJS_Value& vRet, JS_ErrorString& sError);

public:
	FX_BOOL						QueryProperty(FX_LPCWSTR propname);
	FX_BOOL						DoProperty(IFXJS_Context* cc, FX_LPCWSTR propname, CJS_PropValue & vp, JS_ErrorString & sError);
	FX_BOOL						DelProperty(IFXJS_Context* cc, FX_LPCWSTR propname, JS_ErrorString & sError);

	void						Initial(CPDFDoc_Environment* pApp);

private:
	void						UpdateGlobalPersistentVariables();
	void						CommitGlobalPersisitentVariables();
	void						DestroyGlobalPersisitentVariables();
	FX_BOOL						SetGlobalVariables(FX_LPCSTR propname, int nType, 
									double dData, bool bData, const CFX_ByteString& sData, JSObject pData, bool bDefaultPersistent);

	void						ObjectToArray(v8::Handle<v8::Object> pObj, CJS_GlobalVariableArray& array);
	void						PutObjectProperty(v8::Handle<v8::Object> obj, CJS_KeyValue* pData);

private:
	CFX_MapByteStringToPtr		m_mapGlobal;
	CFX_WideString				m_sFilePath;
	CJS_GlobalData*				m_pGlobalData;
	CPDFDoc_Environment*				m_pApp;
};


class CJS_Global : public CJS_Object
{
public:
	CJS_Global(JSFXObject pObject) : CJS_Object(pObject) {};
	virtual ~CJS_Global(void){};

	virtual FX_BOOL	InitInstance(IFXJS_Context* cc);	

	DECLARE_SPECIAL_JS_CLASS(CJS_Global);

	JS_SPECIAL_STATIC_METHOD(setPersistent, global_alternate, global);

};

#endif //_GLOBAL_H_
