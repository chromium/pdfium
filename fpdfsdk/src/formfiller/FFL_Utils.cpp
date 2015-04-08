// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "../../include/formfiller/FormFiller.h"
#include "../../include/formfiller/FFL_Utils.h"

CPDF_Rect CFFL_Utils::MaxRect(const CPDF_Rect & rect1,const CPDF_Rect & rect2)
{
	CPDF_Rect rcRet;

	rcRet.left = std::min(rect1.left, rect2.left);
	rcRet.bottom = std::min(rect1.bottom, rect2.bottom);
	rcRet.right = std::max(rect1.right, rect2.right);
	rcRet.top = std::max(rect1.top, rect2.top);

	return rcRet;
}

CPDF_Rect CFFL_Utils::InflateRect(const CPDF_Rect & crRect,const FX_FLOAT & fSize)
{
	CPDF_Rect crNew(crRect.left - fSize,
					crRect.bottom - fSize,
					crRect.right + fSize,
					crRect.top + fSize);
	crNew.Normalize();
	return crNew;
}

CPDF_Rect CFFL_Utils::DeflateRect(const CPDF_Rect & crRect,const FX_FLOAT & fSize)
{
	CPDF_Rect crNew(crRect.left + fSize,
					crRect.bottom + fSize,
					crRect.right - fSize,
					crRect.top - fSize);
	crNew.Normalize();
	return crNew;
}

FX_BOOL CFFL_Utils::TraceObject(CPDF_Object* pObj)
{
	if (!pObj) return FALSE;

	FX_DWORD	dwObjNum = pObj->GetObjNum();
	switch (pObj->GetType())
	{
	case PDFOBJ_ARRAY:
		{
			CPDF_Array* pArray = (CPDF_Array*)pObj;
			for (FX_DWORD i = 0; i < pArray->GetCount(); i ++)
			{
				CPDF_Object* pElement = pArray->GetElementValue(i);				
				TraceObject(pElement);
			}
		}
		break;

	case PDFOBJ_DICTIONARY:
		{
			CPDF_Dictionary* pDict = (CPDF_Dictionary*)pObj;

			FX_POSITION fPos = pDict->GetStartPos();
			CFX_ByteString csKey;
			do
			{
				CPDF_Object* pElement = pDict->GetNextElement(fPos, csKey);
 				//TRACE(csKey + "\n");
				if (!pElement) break;
				TraceObject(pElement);
			}while (TRUE);
		}
		break;

	case PDFOBJ_STREAM:
		{
			CPDF_Stream* pStream = (CPDF_Stream*)pObj;
			CPDF_Dictionary* pDict = pStream->GetDict();
			TraceObject(pDict);
		}
		break;

	case PDFOBJ_REFERENCE:
		{
			CPDF_Object* pDirectObj = pObj->GetDirect();
			TraceObject(pDirectObj);
		}
		break;

	case PDFOBJ_BOOLEAN:
		break;
	case PDFOBJ_NUMBER:
		//TRACE("%d\n",(FX_INT32)pObj);
		break;
	case PDFOBJ_STRING:
		//TRACE(((CPDF_String*)pObj)->GetString() + "\n");
		break;
	case PDFOBJ_NAME:
		//TRACE(((CPDF_Name*)pObj)->GetString() + "\n");
		break;
	case PDFOBJ_NULL:
//	case PDFOBJ_KEYWORD:
//	case PDFOBJ_EOF:
	default:
		break;
	}
	if (dwObjNum == 0) return FALSE;

	return TRUE;
}

