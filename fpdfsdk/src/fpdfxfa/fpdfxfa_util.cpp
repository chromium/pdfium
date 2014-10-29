// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fsdk_define.h"
#include "../../include/fsdk_mgr.h"
#include "../../include/fpdfxfa/fpdfxfa_util.h"

FX_BOOL FPDF_HasXFAField(CPDF_Document* pPDFDoc, int& docType)
{
	if (!pPDFDoc)
		return FALSE;

	CPDF_Dictionary* pRoot = pPDFDoc->GetRoot();
	if (!pRoot)
		return FALSE;

	CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
	if (!pAcroForm) 
		return FALSE;

	CPDF_Object* pXFA = pAcroForm->GetElement("XFA");
	if (!pXFA) 
		return FALSE;

	FX_BOOL bDymasticXFA = FALSE;
	bDymasticXFA = pRoot->GetBoolean("NeedsRendering", FALSE);

	if(bDymasticXFA)
		docType = DOCTYPE_DYNIMIC_XFA;
	else
		docType = DOCTYPE_STATIC_XFA;

	return TRUE;
}

CFX_PtrArray CXFA_FWLAdapterTimerMgr::ms_timerArray;

FWL_ERR CXFA_FWLAdapterTimerMgr::Start(IFWL_Timer *pTimer, FX_DWORD dwElapse, FWL_HTIMER &hTimer, FX_BOOL bImmediately /* = TRUE */)
{
	if (m_pEnv)
	{
		FX_UINT32 uIDEvent = m_pEnv->FFI_SetTimer(dwElapse, TimerProc);
		CFWL_TimerInfo *pInfo = FX_NEW CFWL_TimerInfo;
		pInfo->uIDEvent = uIDEvent;
		pInfo->pTimer = pTimer;
		ms_timerArray.Add(pInfo);

		hTimer = (FWL_HTIMER)pInfo;
		return FWL_ERR_Succeeded;
	}
	
	return FWL_ERR_Indefinite;
}

FWL_ERR CXFA_FWLAdapterTimerMgr::Stop(FWL_HTIMER hTimer)
{
	if (!hTimer) return FWL_ERR_Indefinite;

	if (m_pEnv)
	{
		CFWL_TimerInfo *pInfo = (CFWL_TimerInfo*)hTimer;

		m_pEnv->FFI_KillTimer(pInfo->uIDEvent);

		FX_INT32 index = ms_timerArray.Find(pInfo);
		if (index >= 0)
		{
			ms_timerArray.RemoveAt(index);
			delete pInfo;
		}
		return FWL_ERR_Succeeded;
	}

	return FWL_ERR_Indefinite;
}

void CXFA_FWLAdapterTimerMgr::TimerProc(FX_INT32 idEvent)
{
	CFWL_TimerInfo *pInfo = NULL;
	FX_INT32 iCount = CXFA_FWLAdapterTimerMgr::ms_timerArray.GetSize();
	for (FX_INT32 i = 0; i < iCount; i++)
	{
		CFWL_TimerInfo *pTemp = (CFWL_TimerInfo*)CXFA_FWLAdapterTimerMgr::ms_timerArray.GetAt(i);
		if (pTemp->uIDEvent == idEvent)
		{
			pInfo = pTemp; break;
		}
	}
	if (pInfo)
	{
		pInfo->pTimer->Run((FWL_HTIMER)pInfo);
	}
}
