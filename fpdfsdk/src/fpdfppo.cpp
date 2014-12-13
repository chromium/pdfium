// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../include/fpdfppo.h"
#include "../include/fsdk_define.h"

class CPDF_PageOrganizer
{
public:
	CPDF_PageOrganizer();
	~CPDF_PageOrganizer();
	
public:
	FX_BOOL				PDFDocInit(CPDF_Document *pDestPDFDoc, CPDF_Document *pSrcPDFDoc);
	FX_BOOL				ExportPage(CPDF_Document *pSrcPDFDoc, CFX_WordArray* nPageNum, CPDF_Document *pDestPDFDoc, int nIndex);
	CPDF_Object*		PageDictGetInheritableTag(CPDF_Dictionary *pDict, CFX_ByteString nSrctag);
	FX_BOOL				UpdateReference(CPDF_Object *pObj, CPDF_Document *pDoc, CFX_MapPtrToPtr* pMapPtrToPtr);
	int					GetNewObjId(CPDF_Document *pDoc, CFX_MapPtrToPtr* pMapPtrToPtr, CPDF_Reference *pRef);
	
};


CPDF_PageOrganizer::CPDF_PageOrganizer()
{

}

CPDF_PageOrganizer::~CPDF_PageOrganizer()
{

}

FX_BOOL CPDF_PageOrganizer::PDFDocInit(CPDF_Document *pDestPDFDoc, CPDF_Document *pSrcPDFDoc)
{
	if(!pDestPDFDoc || !pSrcPDFDoc)
		return false;
	
	CPDF_Dictionary* pNewRoot = pDestPDFDoc->GetRoot();
	if(!pNewRoot)	return FALSE;
	
	//Set the document information////////////////////////////////////////////
	
	CPDF_Dictionary* DInfoDict = pDestPDFDoc->GetInfo();
	
	if(!DInfoDict)
		return FALSE;
	
	CFX_ByteString producerstr;
	producerstr.Format("PDFium");
	DInfoDict->SetAt("Producer", new CPDF_String(producerstr));

	//Set type////////////////////////////////////////////////////////////////
	CFX_ByteString cbRootType = pNewRoot->GetString("Type","");
	if( cbRootType.Equal("") )
	{
		pNewRoot->SetAt("Type", new CPDF_Name("Catalog"));
	}
	
	CPDF_Dictionary* pNewPages = (CPDF_Dictionary*)(pNewRoot->GetElement("Pages")? pNewRoot->GetElement("Pages")->GetDirect() : NULL);
	if(!pNewPages)
	{
		pNewPages = new CPDF_Dictionary;
		FX_DWORD NewPagesON = pDestPDFDoc->AddIndirectObject(pNewPages);
		pNewRoot->SetAt("Pages", new CPDF_Reference(pDestPDFDoc, NewPagesON));
	}
	
	CFX_ByteString cbPageType = pNewPages->GetString("Type","");
	if(cbPageType.Equal(""))
	{
		pNewPages->SetAt("Type", new CPDF_Name("Pages"));
	}

	CPDF_Array* pKeysArray = pNewPages->GetArray("Kids");
	if(pKeysArray == NULL)
	{
		CPDF_Array* pNewKids = new CPDF_Array;
		FX_DWORD Kidsobjnum = -1;
		Kidsobjnum = pDestPDFDoc->AddIndirectObject(pNewKids);//, Kidsobjnum, Kidsgennum);
		
		pNewPages->SetAt("Kids", new CPDF_Reference(pDestPDFDoc, Kidsobjnum));//, Kidsgennum));
		pNewPages->SetAt("Count", new CPDF_Number(0));		
	}

	return true;
}

FX_BOOL CPDF_PageOrganizer::ExportPage(CPDF_Document *pSrcPDFDoc, CFX_WordArray* nPageNum, 
												CPDF_Document *pDestPDFDoc,int nIndex)
{
	int curpage =nIndex;

	CFX_MapPtrToPtr* pMapPtrToPtr = new CFX_MapPtrToPtr;
	pMapPtrToPtr->InitHashTable(1001);

	for(int i=0; i<nPageNum->GetSize(); i++)
	{
		
		CPDF_Dictionary* pCurPageDict = pDestPDFDoc->CreateNewPage(curpage);
		CPDF_Dictionary* pSrcPageDict = pSrcPDFDoc->GetPage(nPageNum->GetAt(i)-1);
		if(!pSrcPageDict || !pCurPageDict)
		{
			delete pMapPtrToPtr;
			return FALSE;
		}
		
		// Clone the page dictionary///////////
		FX_POSITION	SrcPos = pSrcPageDict->GetStartPos();
		while (SrcPos)
		{
			CFX_ByteString cbSrcKeyStr;
			CPDF_Object* pObj = pSrcPageDict->GetNextElement(SrcPos, cbSrcKeyStr);
			if(cbSrcKeyStr.Compare(("Type")) && cbSrcKeyStr.Compare(("Parent")))
			{
				if(pCurPageDict->KeyExist(cbSrcKeyStr))
					pCurPageDict->RemoveAt(cbSrcKeyStr);
				pCurPageDict->SetAt(cbSrcKeyStr, pObj->Clone());
			}
		}
		
		//inheritable item///////////////////////
		CPDF_Object* pInheritable = NULL;
		//1	MediaBox  //required
		if(!pCurPageDict->KeyExist("MediaBox"))
		{
			
			pInheritable = PageDictGetInheritableTag(pSrcPageDict, "MediaBox");
			if(!pInheritable) 
			{
				//Search the "CropBox" from source page dictionary, if not exists,we take the letter size.
				pInheritable = PageDictGetInheritableTag(pSrcPageDict, "CropBox");
				if(pInheritable)
					pCurPageDict->SetAt("MediaBox", pInheritable->Clone());
				else
				{
					//Make the default size to be letter size (8.5'x11')
					CPDF_Array* pArray = new CPDF_Array;
					pArray->AddNumber(0);
					pArray->AddNumber(0);
					pArray->AddNumber(612);
					pArray->AddNumber(792);
					pCurPageDict->SetAt("MediaBox", pArray);
				}
			}
			else
				pCurPageDict->SetAt("MediaBox", pInheritable->Clone());
		}
		//2 Resources //required
		if(!pCurPageDict->KeyExist("Resources"))
		{
			pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Resources");
			if(!pInheritable) 
			{
				delete pMapPtrToPtr;
				return FALSE;
			}
			pCurPageDict->SetAt("Resources", pInheritable->Clone());
		}
		//3 CropBox  //Optional
		if(!pCurPageDict->KeyExist("CropBox"))
		{
			pInheritable = PageDictGetInheritableTag(pSrcPageDict, "CropBox");
			if(pInheritable) 
				pCurPageDict->SetAt("CropBox", pInheritable->Clone());
		}
		//4 Rotate  //Optional
		if(!pCurPageDict->KeyExist("Rotate"))
		{
			pInheritable = PageDictGetInheritableTag(pSrcPageDict, "Rotate");
			if(pInheritable) 
				pCurPageDict->SetAt("Rotate", pInheritable->Clone());
		}

		/////////////////////////////////////////////
		//Update the reference
		FX_DWORD dwOldPageObj = pSrcPageDict->GetObjNum();
		FX_DWORD dwNewPageObj = pCurPageDict->GetObjNum();
		
		pMapPtrToPtr->SetAt((FX_LPVOID)(FX_UINTPTR)dwOldPageObj, (FX_LPVOID)(FX_UINTPTR)dwNewPageObj);

		this->UpdateReference(pCurPageDict, pDestPDFDoc, pMapPtrToPtr);
		curpage++;
	}

	delete pMapPtrToPtr;
	return TRUE;
}

CPDF_Object* CPDF_PageOrganizer::PageDictGetInheritableTag(CPDF_Dictionary *pDict, CFX_ByteString nSrctag)
{
	if(!pDict || !pDict->KeyExist("Type") || nSrctag.IsEmpty())	
		return NULL;

	CPDF_Object* pType = pDict->GetElement("Type")->GetDirect();
	if(!pType || pType->GetType() != PDFOBJ_NAME)	return NULL;

	if(pType->GetString().Compare("Page"))	return NULL;

	if(!pDict->KeyExist("Parent"))	return NULL;
	CPDF_Object* pParent = pDict->GetElement("Parent")->GetDirect();
	if(!pParent || pParent->GetType() != PDFOBJ_DICTIONARY)	return NULL;
	
	CPDF_Dictionary* pp = (CPDF_Dictionary*)pParent;
	
	if(pDict->KeyExist((const char*)nSrctag))	
		return pDict->GetElement((const char*)nSrctag);
	while (pp)
	{
		if(pp->KeyExist((const char*)nSrctag))	
			return pp->GetElement((const char*)nSrctag);
		else if (pp->KeyExist("Parent"))
		{
			pp = (CPDF_Dictionary*)pp->GetElement("Parent")->GetDirect();
			if (pp->GetType() == PDFOBJ_NULL) break;
		}
		else break;
	}
	
	return NULL;
}

FX_BOOL CPDF_PageOrganizer::UpdateReference(CPDF_Object *pObj, CPDF_Document *pDoc, 
										 CFX_MapPtrToPtr* pMapPtrToPtr)
{
	switch (pObj->GetType())
	{
	case PDFOBJ_REFERENCE:
		{
			CPDF_Reference* pReference = (CPDF_Reference*)pObj;
			int newobjnum = GetNewObjId(pDoc, pMapPtrToPtr, pReference);
			if (newobjnum == 0) return FALSE;
			pReference->SetRef(pDoc, newobjnum);//, 0);
			break;
		}
	case PDFOBJ_DICTIONARY:
		{
			CPDF_Dictionary* pDict = (CPDF_Dictionary*)pObj;
			
			FX_POSITION pos = pDict->GetStartPos();
			while(pos)
			{
				CFX_ByteString key("");
				CPDF_Object* pNextObj = pDict->GetNextElement(pos, key);
				if (!FXSYS_strcmp(key, "Parent") || !FXSYS_strcmp(key, "Prev") || !FXSYS_strcmp(key, "First"))
					continue;
				if(pNextObj)
				{
					if(!UpdateReference(pNextObj, pDoc, pMapPtrToPtr))
						pDict->RemoveAt(key);
				}
				else
					return FALSE;
			}
			break;
		}
	case	PDFOBJ_ARRAY:
		{
			CPDF_Array* pArray = (CPDF_Array*)pObj;
			FX_DWORD count = pArray->GetCount();
			for(FX_DWORD i = 0; i < count; i ++)
			{
				CPDF_Object* pNextObj = pArray->GetElement(i);
				if(pNextObj)
				{
					if(!UpdateReference(pNextObj, pDoc, pMapPtrToPtr))
						return FALSE;
				}
				else
					return FALSE;
			}
			break;
		}
	case	PDFOBJ_STREAM:
		{
			CPDF_Stream* pStream = (CPDF_Stream*)pObj;
			CPDF_Dictionary* pDict = pStream->GetDict();
			if(pDict)
			{
				if(!UpdateReference(pDict, pDoc, pMapPtrToPtr))
					return FALSE;
			}
			else
				return FALSE;
			break;
		}
	default:	break;
	}

	return TRUE;
}

int	CPDF_PageOrganizer::GetNewObjId(CPDF_Document *pDoc, CFX_MapPtrToPtr* pMapPtrToPtr,
									CPDF_Reference *pRef)
{
	size_t dwObjnum = 0;
	if(!pRef)
		return 0;
	dwObjnum = pRef->GetRefObjNum();
	
	size_t dwNewObjNum = 0;
	
	pMapPtrToPtr->Lookup((FX_LPVOID)dwObjnum, (FX_LPVOID&)dwNewObjNum);
	if(dwNewObjNum)
	{
		return (int)dwNewObjNum;
	}
	else
	{
		CPDF_Object* pDirect = pRef->GetDirect();
		if(!pDirect)
		{
			return 0;
		}

		CPDF_Object* pClone = pDirect->Clone();
		if(!pClone)
		{
			return 0;
		}
		
		if(pClone->GetType() == PDFOBJ_DICTIONARY)
		{
			CPDF_Dictionary* pDictClone = (CPDF_Dictionary*)pClone;
			if(pDictClone->KeyExist("Type"))
			{
				CFX_ByteString strType = pDictClone->GetString("Type");
				if(!FXSYS_stricmp(strType, "Pages"))
				{
					pDictClone->Release();
					return 4;
				}
				else if(!FXSYS_stricmp(strType, "Page"))
				{
					pDictClone->Release();
					return  0;
				}
			}
		}
		dwNewObjNum = pDoc->AddIndirectObject(pClone);//, onum, gnum);
		pMapPtrToPtr->SetAt((FX_LPVOID)dwObjnum, (FX_LPVOID)dwNewObjNum);
		
		if(!UpdateReference(pClone, pDoc, pMapPtrToPtr))
		{
			pClone->Release();
			return 0;
		}
		return (int)dwNewObjNum;
	}
	return 0;
}

FPDF_BOOL ParserPageRangeString(CFX_ByteString rangstring, CFX_WordArray* pageArray,int nCount)
{

	if(rangstring.GetLength() != 0)
	{
		rangstring.Remove(' ');
		int nLength = rangstring.GetLength();
		CFX_ByteString cbCompareString("0123456789-,");
		for(int i=0; i<nLength; i++)
		{
			if(cbCompareString.Find(rangstring[i]) == -1)
				return FALSE;
		}
		CFX_ByteString cbMidRange;
		int nStringFrom = 0;
		int nStringTo=0;
		while(nStringTo < nLength)
		{
			nStringTo = rangstring.Find(',',nStringFrom);
			if(nStringTo == -1)
			{
				nStringTo = nLength;
			}
			cbMidRange = rangstring.Mid(nStringFrom,nStringTo-nStringFrom);
			
			int nMid = cbMidRange.Find('-');
			if(nMid == -1)
			{
				long lPageNum = atol(cbMidRange);
				if(lPageNum <= 0 || lPageNum > nCount)
					return FALSE;
				pageArray->Add((FX_WORD)lPageNum);
			}
			else
			{
				int nStartPageNum = atol(cbMidRange.Mid(0,nMid));
				if (nStartPageNum ==0)
				{
					return FALSE;
				}


				nMid = nMid+1;
				int nEnd = cbMidRange.GetLength()-nMid;

				if(nEnd ==0)return FALSE;
				
				//				int nEndPageNum = (nEnd == 0)?nCount:atol(cbMidRange.Mid(nMid,nEnd));
				int nEndPageNum = atol(cbMidRange.Mid(nMid,nEnd));
				
				if(nStartPageNum < 0 ||nStartPageNum >nEndPageNum|| nEndPageNum > nCount)
				{
					return FALSE;
				}
				else
				{
					for(int nIndex=nStartPageNum; nIndex <= nEndPageNum; nIndex ++)
						pageArray->Add(nIndex);
				}
			}
			nStringFrom = nStringTo +1;
		}
	}
	return TRUE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_ImportPages(FPDF_DOCUMENT dest_doc,FPDF_DOCUMENT src_doc, 
											 FPDF_BYTESTRING pagerange, int index)
{
	if(dest_doc == NULL || src_doc == NULL )
		return FALSE;
	CFX_WordArray pageArray;
	CPDF_Document* pSrcDoc = (CPDF_Document*)src_doc;
	int nCount = pSrcDoc->GetPageCount();
	if(pagerange)
	{
		if(ParserPageRangeString(pagerange,&pageArray,nCount) == FALSE)
			return FALSE;
	}
	else
	{
		for(int i=1; i<=nCount; i++)
		{
			pageArray.Add(i);
		}
	}
	
	CPDF_Document* pDestDoc = (CPDF_Document*)dest_doc;
	CPDF_PageOrganizer pageOrg;

	pageOrg.PDFDocInit(pDestDoc,pSrcDoc);

	if(pageOrg.ExportPage(pSrcDoc,&pageArray,pDestDoc,index))
		return TRUE;
	return FALSE;
}

DLLEXPORT FPDF_BOOL STDCALL FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc)
{
	if(src_doc == NULL || dest_doc == NULL)
		return false;
	CPDF_Document* pSrcDoc = (CPDF_Document*)src_doc;
	CPDF_Dictionary* pSrcDict = pSrcDoc->GetRoot();
	pSrcDict = pSrcDict->GetDict(FX_BSTRC("ViewerPreferences"));;
	if(!pSrcDict)
		return FALSE;
	CPDF_Document* pDstDoc = (CPDF_Document*)dest_doc;
	CPDF_Dictionary* pDstDict = pDstDoc->GetRoot();
	if(!pDstDict)
		return FALSE;
	pDstDict->SetAt(FX_BSTRC("ViewerPreferences"), pSrcDict->Clone(TRUE));
	return TRUE;
}

