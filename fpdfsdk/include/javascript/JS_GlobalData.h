// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_GLOBALDATA_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_GLOBALDATA_H_

#include "../../../core/include/fxcrt/fx_basic.h"

#define JS_GLOBALDATA_TYPE_NUMBER		0
#define JS_GLOBALDATA_TYPE_BOOLEAN		1
#define JS_GLOBALDATA_TYPE_STRING		2
#define JS_GLOBALDATA_TYPE_OBJECT		3
#define JS_GLOBALDATA_TYPE_NULL			4

class CJS_KeyValue;
class CPDFDoc_Environment;

class CJS_GlobalVariableArray
{
public:
	CJS_GlobalVariableArray();
	virtual ~CJS_GlobalVariableArray();

	void			Add(CJS_KeyValue* p);
	int				Count() const;
	CJS_KeyValue*	GetAt(int index) const;
	void			Copy(const CJS_GlobalVariableArray& array);

	void			Empty();

private:
	CFX_ArrayTemplate<CJS_KeyValue*> array;
};

class CJS_KeyValue
{
public:
	CJS_KeyValue(){}
	virtual ~CJS_KeyValue(){}

	CFX_ByteString					sKey;
	int								nType; //0:int 1:bool 2:string 3:obj
	double							dData;
	bool							bData;
	CFX_ByteString					sData;
	CJS_GlobalVariableArray			objData;
};

class CJS_GlobalData_Element
{
public:
	CJS_GlobalData_Element(){}
	virtual ~CJS_GlobalData_Element(){}

	CJS_KeyValue			data;
	FX_BOOL					bPersistent;
};

class CJS_GlobalData
{
public:
	CJS_GlobalData(CPDFDoc_Environment* pApp);
	virtual ~CJS_GlobalData();

public:
	void								SetGlobalVariableNumber(FX_LPCSTR propname, double dData);
	void								SetGlobalVariableBoolean(FX_LPCSTR propname, bool bData);
	void								SetGlobalVariableString(FX_LPCSTR propname, const CFX_ByteString& sData);
	void								SetGlobalVariableObject(FX_LPCSTR propname, const CJS_GlobalVariableArray& array);
	void								SetGlobalVariableNull(FX_LPCSTR propname);

	FX_BOOL								SetGlobalVariablePersistent(FX_LPCSTR propname, FX_BOOL bPersistent);
	FX_BOOL								DeleteGlobalVariable(FX_LPCSTR propname);
	
	FX_INT32							GetSize() const;
	CJS_GlobalData_Element*				GetAt(int index) const;

private:
	void								LoadGlobalPersistentVariables();
	void								SaveGlobalPersisitentVariables();
	
	CJS_GlobalData_Element*				GetGlobalVariable(FX_LPCSTR propname);	
	int									FindGlobalVariable(FX_LPCSTR propname);

	void								LoadFileBuffer(FX_LPCWSTR sFilePath, FX_LPBYTE& pBuffer, FX_INT32& nLength);
	void								WriteFileBuffer(FX_LPCWSTR sFilePath, FX_LPCSTR pBuffer, FX_INT32 nLength);
	void								MakeByteString(const CFX_ByteString& name, CJS_KeyValue* pData, CFX_BinaryBuf& sData);

private:
	CFX_ArrayTemplate<CJS_GlobalData_Element*>	m_arrayGlobalData;
	CFX_WideString								m_sFilePath;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_GLOBALDATA_H_
