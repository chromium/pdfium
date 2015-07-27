// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

// #include "../../include/formfiller/FormFiller.h"
#include "../../include/formfiller/FFL_FormFiller.h"
#include "../../include/formfiller/FFL_Notify.h"
// #include "../../include/formfiller/FFL_ComboBox.h"
// #include "../../include/formfiller/FFL_Module.h"

/* -------------------------------- CFFL_Notify ------------------------------ */

//#pragma warning(disable: 4800)

CFFL_Notify::CFFL_Notify(CFFL_FormFiller * pFormFiller) :
	m_bDoActioning(FALSE),
	m_nNotifyFlag(0)
{
	ASSERT(pFormFiller != NULL);
}

CFFL_Notify::~CFFL_Notify()
{
}

void CFFL_Notify::BeforeNotify()
{
	m_nNotifyFlag ++;
}


void CFFL_Notify::AfterNotify()
{
	m_nNotifyFlag --;
}

FX_BOOL CFFL_Notify::OnMouseUp(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::ButtonUp, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnMouseDown(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::ButtonDown, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnMouseEnter(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::CursorEnter, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnMouseExit(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::CursorExit, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnSetFocus(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::GetFocus, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnKillFocus(FX_BOOL & bExit)
{
	BeforeNotify();
	FX_BOOL bRet = FALSE;//DoAAction(CPDF_AAction::AActionType::LoseFocus, bExit);
	AfterNotify();
	return bRet;
}

FX_BOOL CFFL_Notify::OnCalculate()
{
	return TRUE;
}

FX_BOOL CFFL_Notify::OnFormat(int iCommitKey)
{
	return TRUE;
}

FX_BOOL CFFL_Notify::OnKeyStroke(CPDF_FormField* pFormField, int nCommitKey, CFX_WideString& strValue, CFX_WideString& strChange,
							   const CFX_WideString& strChangeEx, FX_BOOL bKeyDown, FX_BOOL bModifier,
							   FX_BOOL bShift, FX_BOOL bWillCommit, FX_BOOL bFieldFull,
							   int& nSelStart, int& nSelEnd, FX_BOOL& bRC)
{
	return TRUE;
}

FX_BOOL CFFL_Notify::OnValidate(CPDF_FormField* pFormField, CFX_WideString& strValue, CFX_WideString & strChange,
									   const CFX_WideString& strChangeEx, FX_BOOL bKeyDown, FX_BOOL bModifier,
									   FX_BOOL bShift, FX_BOOL & bRC)
{
	return TRUE;
}

FX_BOOL	CFFL_Notify::DoAAction(CPDF_AAction::AActionType eAAT, FX_BOOL & bExit)
{
    if (m_bDoActioning)
        return FALSE;

    CPDF_Action action;
    if (!FindAAction(eAAT, action))
        return FALSE;

    m_bDoActioning = TRUE;
    ExecuteActionTree(eAAT,action,bExit);
    m_bDoActioning = FALSE;
    return TRUE;
}

FX_BOOL	CFFL_Notify::ExecuteActionTree(CPDF_AAction::AActionType eAAT,CPDF_Action & action, FX_BOOL& bExit)
{
	if (!ExecuteAction(eAAT,action,bExit)) return FALSE;
	if (bExit) return TRUE;

	for (int32_t i=0,sz=action.GetSubActionsCount(); i<sz; i++)
	{
		CPDF_Action subaction = action.GetSubAction(i);
		if (!ExecuteActionTree(eAAT,subaction,bExit)) return FALSE;
		if (bExit) break;
	}

	return TRUE;
}


FX_BOOL	CFFL_Notify::FindAAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action)
{
	return FALSE;
}

FX_BOOL CFFL_Notify::FindAAction(CPDF_AAction aaction,CPDF_AAction::AActionType eAAT,CPDF_Action & action)
{
	CPDF_Action MyAction;

	if (aaction.ActionExist(eAAT))
	{
		MyAction = aaction.GetAction(eAAT);
	}
	else
		return FALSE;


	if (MyAction.GetType() == CPDF_Action::Unknown)
		return FALSE;

	action = MyAction;

	return TRUE;
}

FX_BOOL	CFFL_Notify::ExecuteAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action,FX_BOOL& bExit)
{
	return FALSE;
}
//#pragma warning(default: 4800)

