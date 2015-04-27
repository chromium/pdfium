// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/javascript/JavaScript.h"
#include "../../include/javascript/IJavaScript.h"
#include "../../include/javascript/JS_GlobalData.h"

#define JS_MAXGLOBALDATA			(1024 * 4 - 8)

/* --------------------- CJS_GlobalVariableArray --------------------- */

CJS_GlobalVariableArray::CJS_GlobalVariableArray()
{
}

CJS_GlobalVariableArray::~CJS_GlobalVariableArray()
{
	Empty();
}

void CJS_GlobalVariableArray::Copy(const CJS_GlobalVariableArray& array)
{
	Empty();
	for (int i=0,sz=array.Count(); i<sz; i++)
	{
		CJS_KeyValue* pOldObjData = array.GetAt(i);
		ASSERT(pOldObjData != NULL);

		switch (pOldObjData->nType)
		{
		case JS_GLOBALDATA_TYPE_NUMBER:
			{
				CJS_KeyValue* pNewObjData = new CJS_KeyValue;
				pNewObjData->sKey = pOldObjData->sKey;
				pNewObjData->nType = pOldObjData->nType;
				pNewObjData->dData = pOldObjData->dData;
				Add(pNewObjData);
			}
			break;
		case JS_GLOBALDATA_TYPE_BOOLEAN:
			{
				CJS_KeyValue* pNewObjData = new CJS_KeyValue;
				pNewObjData->sKey = pOldObjData->sKey;
				pNewObjData->nType = pOldObjData->nType;
				pNewObjData->bData = pOldObjData->bData;
				Add(pNewObjData);
			}
			break;
		case JS_GLOBALDATA_TYPE_STRING:
			{
				CJS_KeyValue* pNewObjData = new CJS_KeyValue;
				pNewObjData->sKey = pOldObjData->sKey;
				pNewObjData->nType = pOldObjData->nType;
				pNewObjData->sData = pOldObjData->sData;
				Add(pNewObjData);
			}
			break;
		case JS_GLOBALDATA_TYPE_OBJECT:
			{
				CJS_KeyValue* pNewObjData = new CJS_KeyValue;
				pNewObjData->sKey = pOldObjData->sKey;
				pNewObjData->nType = pOldObjData->nType;
				pNewObjData->objData.Copy(pOldObjData->objData);
				Add(pNewObjData);
			}
		case JS_GLOBALDATA_TYPE_NULL:
			{
				CJS_KeyValue* pNewObjData = new CJS_KeyValue;
				pNewObjData->sKey = pOldObjData->sKey;
				pNewObjData->nType = pOldObjData->nType;
				Add(pNewObjData);
			}
		}
	}
}

void CJS_GlobalVariableArray::Add(CJS_KeyValue* p)
{
	array.Add(p);
}

int CJS_GlobalVariableArray::Count() const
{
	return array.GetSize();
}

CJS_KeyValue* CJS_GlobalVariableArray::GetAt(int index) const
{
	return array.GetAt(index);
}

void CJS_GlobalVariableArray::Empty()
{
	for (int i=0,sz=array.GetSize(); i<sz; i++)
		delete array.GetAt(i);
	array.RemoveAll();
}

/* -------------------------- CJS_GlobalData -------------------------- */

#define READER_JS_GLOBALDATA_FILENAME				L"Reader_JsGlobal.Data"
#define PHANTOM_JS_GLOBALDATA_FILENAME				L"Phantom_JsGlobal.Data"
#define SDK_JS_GLOBALDATA_FILENAME					L"SDK_JsGlobal.Data"

static const FX_BYTE JS_RC4KEY[] = {0x19,0xa8,0xe8,0x01,0xf6,0xa8,0xb6,0x4d,0x82,0x04,
							0x45,0x6d,0xb4,0xcf,0xd7,0x77,0x67,0xf9,0x75,0x9f,
							0xf0,0xe0,0x1e,0x51,0xee,0x46,0xfd,0x0b,0xc9,0x93,
							0x25,0x55,0x4a,0xee,0xe0,0x16,0xd0,0xdf,0x8c,0xfa,
							0x2a,0xa9,0x49,0xfd,0x97,0x1c,0x0e,0x22,0x13,0x28,
							0x7c,0xaf,0xc4,0xfc,0x9c,0x12,0x65,0x8c,0x4e,0x5b,
							0x04,0x75,0x89,0xc9,0xb1,0xed,0x50,0xca,0x96,0x6f,
							0x1a,0x7a,0xfe,0x58,0x5d,0xec,0x19,0x4a,0xf6,0x35,
							0x6a,0x97,0x14,0x00,0x0e,0xd0,0x6b,0xbb,0xd5,0x75,
							0x55,0x8b,0x6e,0x6b,0x19,0xa0,0xf8,0x77,0xd5,0xa3
							};

CJS_GlobalData::CJS_GlobalData(CPDFDoc_Environment* pApp)
{
// 	IBaseAnnot* pBaseAnnot = IBaseAnnot::GetBaseAnnot(m_pApp);
// 	ASSERT(pBaseAnnot != NULL);
// 
// 	m_sFilePath = pBaseAnnot->GetUserPath();
	m_sFilePath += SDK_JS_GLOBALDATA_FILENAME;

	LoadGlobalPersistentVariables();
}

CJS_GlobalData::~CJS_GlobalData()
{
	SaveGlobalPersisitentVariables();

	for (int i=0,sz=m_arrayGlobalData.GetSize(); i<sz; i++)
		delete m_arrayGlobalData.GetAt(i);

	m_arrayGlobalData.RemoveAll();
}

int	CJS_GlobalData::FindGlobalVariable(FX_LPCSTR propname)
{
	ASSERT(propname != NULL);

	int nRet = -1;

	for (int i=0,sz=m_arrayGlobalData.GetSize(); i<sz; i++)
	{
		CJS_GlobalData_Element* pTemp = m_arrayGlobalData.GetAt(i);
		if (pTemp->data.sKey[0] == *propname && pTemp->data.sKey == propname)
		{
			nRet = i;
			break;
		}
	}

	return nRet;
}

CJS_GlobalData_Element* CJS_GlobalData::GetGlobalVariable(FX_LPCSTR propname)
{
	ASSERT(propname != NULL);

	int	nFind = FindGlobalVariable(propname);

	if (nFind >= 0)
		return m_arrayGlobalData.GetAt(nFind);
	else
		return NULL;
}

void CJS_GlobalData::SetGlobalVariableNumber(FX_LPCSTR propname, double dData)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return;

	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->data.nType = JS_GLOBALDATA_TYPE_NUMBER;
		pData->data.dData = dData;
	}
	else
	{
		CJS_GlobalData_Element* pNewData = new CJS_GlobalData_Element;
		pNewData->data.sKey = sPropName;
		pNewData->data.nType = JS_GLOBALDATA_TYPE_NUMBER;
		pNewData->data.dData = dData;

		m_arrayGlobalData.Add(pNewData);
	}
}

void CJS_GlobalData::SetGlobalVariableBoolean(FX_LPCSTR propname, bool bData)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return;

	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->data.nType = JS_GLOBALDATA_TYPE_BOOLEAN;
		pData->data.bData = bData;
	}
	else
	{
		CJS_GlobalData_Element* pNewData = new CJS_GlobalData_Element;
		pNewData->data.sKey = sPropName;
		pNewData->data.nType = JS_GLOBALDATA_TYPE_BOOLEAN;
		pNewData->data.bData = bData;

		m_arrayGlobalData.Add(pNewData);
	}
}

void CJS_GlobalData::SetGlobalVariableString(FX_LPCSTR propname, const CFX_ByteString& sData)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return;

	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->data.nType = JS_GLOBALDATA_TYPE_STRING;
		pData->data.sData = sData;
	}
	else
	{
		CJS_GlobalData_Element* pNewData = new CJS_GlobalData_Element;
		pNewData->data.sKey = sPropName;
		pNewData->data.nType = JS_GLOBALDATA_TYPE_STRING;
		pNewData->data.sData = sData;

		m_arrayGlobalData.Add(pNewData);
	}
}

void CJS_GlobalData::SetGlobalVariableObject(FX_LPCSTR propname, const CJS_GlobalVariableArray& array)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return;

	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->data.nType = JS_GLOBALDATA_TYPE_OBJECT;
		pData->data.objData.Copy(array);
	}
	else
	{
		CJS_GlobalData_Element* pNewData = new CJS_GlobalData_Element;
		pNewData->data.sKey = sPropName;
		pNewData->data.nType = JS_GLOBALDATA_TYPE_OBJECT;
		pNewData->data.objData.Copy(array);
		
		m_arrayGlobalData.Add(pNewData);
	}
}

void CJS_GlobalData::SetGlobalVariableNull(FX_LPCSTR propname)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;
	
	sPropName.TrimLeft();
	sPropName.TrimRight();
	
	if (sPropName.GetLength() == 0) return;
	
	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->data.nType = JS_GLOBALDATA_TYPE_NULL;
	}
	else
	{
		CJS_GlobalData_Element* pNewData = new CJS_GlobalData_Element;
		pNewData->data.sKey = sPropName;
		pNewData->data.nType = JS_GLOBALDATA_TYPE_NULL;
		
		m_arrayGlobalData.Add(pNewData);
	}
}

FX_BOOL CJS_GlobalData::SetGlobalVariablePersistent(FX_LPCSTR propname, FX_BOOL bPersistent)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return FALSE;

	if (CJS_GlobalData_Element* pData = GetGlobalVariable(sPropName))
	{
		pData->bPersistent = bPersistent;
		return TRUE;
	}

	return FALSE;
}

FX_BOOL CJS_GlobalData::DeleteGlobalVariable(FX_LPCSTR propname)
{
	ASSERT(propname != NULL);
	CFX_ByteString sPropName = propname;

	sPropName.TrimLeft();
	sPropName.TrimRight();

	if (sPropName.GetLength() == 0) return FALSE;

	int	nFind = FindGlobalVariable(sPropName);

	if (nFind >= 0)
	{
		delete m_arrayGlobalData.GetAt(nFind);
		m_arrayGlobalData.RemoveAt(nFind);
		return TRUE;
	}

	return FALSE;
}

FX_INT32 CJS_GlobalData::GetSize() const
{
	return m_arrayGlobalData.GetSize();
}

CJS_GlobalData_Element* CJS_GlobalData::GetAt(int index) const
{
	return m_arrayGlobalData.GetAt(index);
}

void CJS_GlobalData::LoadGlobalPersistentVariables()
{
	FX_LPBYTE pBuffer = NULL;
	FX_INT32 nLength = 0;

	LoadFileBuffer(m_sFilePath.c_str(), pBuffer, nLength);
	CRYPT_ArcFourCryptBlock(pBuffer, nLength, JS_RC4KEY, sizeof(JS_RC4KEY));

	if (pBuffer)
	{
		FX_LPBYTE p = pBuffer;
		FX_WORD wType = *((FX_WORD*)p);
		p += sizeof(FX_WORD);

		//FX_WORD wTemp = (FX_WORD)(('X' << 8) | 'F');

		if (wType == (FX_WORD)(('X' << 8) | 'F'))
		{
			FX_WORD wVersion = *((FX_WORD*)p);
			p += sizeof(FX_WORD);

			ASSERT(wVersion <= 2);

			FX_DWORD dwCount = *((FX_DWORD*)p);
			p += sizeof(FX_DWORD);

			FX_DWORD dwSize = *((FX_DWORD*)p);
			p += sizeof(FX_DWORD);

			if (dwSize == nLength - sizeof(FX_WORD) * 2 - sizeof(FX_DWORD)* 2)
			{
				for (FX_INT32 i=0,sz=dwCount; i<sz; i++)
				{
					if (p > pBuffer + nLength)
						break;

					FX_DWORD dwNameLen = *((FX_DWORD*)p);
					p += sizeof(FX_DWORD);

					if (p + dwNameLen > pBuffer + nLength)
						break;

					CFX_ByteString sEntry = CFX_ByteString(p, dwNameLen);
					p += sizeof(char) * dwNameLen;

					FX_WORD wDataType = *((FX_WORD*)p);
					p += sizeof(FX_WORD);

					switch (wDataType)
					{
					case JS_GLOBALDATA_TYPE_NUMBER:
						{
							double dData = 0;
							switch (wVersion)
							{
							case 1:
								{
									FX_DWORD dwData = *((FX_DWORD*)p);
									p += sizeof(FX_DWORD);
									dData = dwData;
								}
								break;
							case 2:
								{
									dData = *((double*)p);
									p += sizeof(double);
								}
								break;
							}
							SetGlobalVariableNumber(sEntry, dData);
							SetGlobalVariablePersistent(sEntry, TRUE);
						}
						break;
					case JS_GLOBALDATA_TYPE_BOOLEAN:
						{
							FX_WORD wData = *((FX_WORD*)p);
							p += sizeof(FX_WORD);
							SetGlobalVariableBoolean(sEntry, (bool)(wData == 1));
							SetGlobalVariablePersistent(sEntry, TRUE);
						}
						break;
					case JS_GLOBALDATA_TYPE_STRING:
						{
							FX_DWORD dwLength = *((FX_DWORD*)p);
							p += sizeof(FX_DWORD);

							if (p + dwLength > pBuffer + nLength)
								break;

							SetGlobalVariableString(sEntry, CFX_ByteString(p, dwLength));
							SetGlobalVariablePersistent(sEntry, TRUE);
							p += sizeof(char) * dwLength;
						}
						break;
					case JS_GLOBALDATA_TYPE_NULL:
						{
							SetGlobalVariableNull(sEntry);
							SetGlobalVariablePersistent(sEntry, TRUE);
						}
					}
				}
			}
		}
		FX_Free(pBuffer);
	}
}

/*
struct js_global_datafile_header
{
	FX_WORD type; //FX ('X' << 8) | 'F'
	FX_WORD version; //1.0
	FX_DWORD datacount;
};
struct js_global_datafile_data
{
	FX_WORD type;
	FX_DWORD nData;
	FX_WORD bData;
	FX_DWORD nStrLen;
	char* pStr;
};
*/

void CJS_GlobalData::SaveGlobalPersisitentVariables()
{
	FX_DWORD nCount = 0;
	CFX_BinaryBuf sData;

	for (int i=0,sz=m_arrayGlobalData.GetSize(); i<sz; i++)
	{
		CJS_GlobalData_Element* pElement = m_arrayGlobalData.GetAt(i);
		ASSERT(pElement != NULL);

		if (pElement->bPersistent)
		{
			CFX_BinaryBuf sElement;
			MakeByteString(pElement->data.sKey, &pElement->data, sElement);

			if (sData.GetSize() + sElement.GetSize() > JS_MAXGLOBALDATA)
				break;

			sData.AppendBlock(sElement.GetBuffer(), sElement.GetSize());
			nCount++;
		}
	}

	CFX_BinaryBuf sFile;

	FX_WORD wType = (FX_WORD)(('X' << 8) | 'F');
	sFile.AppendBlock(&wType, sizeof(FX_WORD));
	FX_WORD wVersion = 2;
	sFile.AppendBlock(&wVersion, sizeof(FX_WORD));
	sFile.AppendBlock(&nCount, sizeof(FX_DWORD));
	FX_DWORD dwSize = sData.GetSize();
	sFile.AppendBlock(&dwSize, sizeof(FX_DWORD));

	sFile.AppendBlock(sData.GetBuffer(), sData.GetSize());

	CRYPT_ArcFourCryptBlock(sFile.GetBuffer(), sFile.GetSize(), JS_RC4KEY, sizeof(JS_RC4KEY));
	WriteFileBuffer(m_sFilePath.c_str(), (FX_LPCSTR)sFile.GetBuffer(), sFile.GetSize());
}

void CJS_GlobalData::LoadFileBuffer(FX_LPCWSTR sFilePath, FX_LPBYTE& pBuffer, FX_INT32& nLength)
{
//UnSupport.
}

void CJS_GlobalData::WriteFileBuffer(FX_LPCWSTR sFilePath, FX_LPCSTR pBuffer, FX_INT32 nLength)
{
//UnSupport.
}

void CJS_GlobalData::MakeByteString(const CFX_ByteString& name, CJS_KeyValue* pData, CFX_BinaryBuf& sData)
{
	ASSERT(pData != NULL);

	FX_WORD wType = (FX_WORD)pData->nType;

	switch (wType)
	{
	case JS_GLOBALDATA_TYPE_NUMBER:
		{
			FX_DWORD dwNameLen = (FX_DWORD)name.GetLength();
			sData.AppendBlock(&dwNameLen, sizeof(FX_DWORD));
			sData.AppendString(name);

			sData.AppendBlock(&wType, sizeof(FX_WORD));
			double dData = pData->dData;
			sData.AppendBlock(&dData, sizeof(double));
		}
		break;
	case JS_GLOBALDATA_TYPE_BOOLEAN:
		{
			FX_DWORD dwNameLen = (FX_DWORD)name.GetLength();
			sData.AppendBlock(&dwNameLen, sizeof(FX_DWORD));
			sData.AppendString(name);

			sData.AppendBlock(&wType, sizeof(FX_WORD));
			FX_WORD wData = (FX_WORD)pData->bData;
			sData.AppendBlock(&wData, sizeof(FX_WORD));
		}
		break;
	case JS_GLOBALDATA_TYPE_STRING:
		{
			FX_DWORD dwNameLen = (FX_DWORD)name.GetLength();
			sData.AppendBlock(&dwNameLen, sizeof(FX_DWORD));
			sData.AppendString(name);

			sData.AppendBlock(&wType, sizeof(FX_WORD));

			FX_DWORD dwDataLen = (FX_DWORD)pData->sData.GetLength();
			sData.AppendBlock(&dwDataLen, sizeof(FX_DWORD));
			sData.AppendString(pData->sData);
		}
		break;
	case JS_GLOBALDATA_TYPE_NULL:
		{
			FX_DWORD dwNameLen = (FX_DWORD)name.GetLength();
			sData.AppendBlock(&dwNameLen, sizeof(FX_DWORD));
			sData.AppendString(name);

			sData.AppendBlock(&wType, sizeof(FX_DWORD));
		}
		break;
	default:
		break;
	}
}

