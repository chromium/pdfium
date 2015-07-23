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
	m_bDoActioning(false),
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

bool CFFL_Notify::OnMouseUp(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::ButtonUp, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnMouseDown(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::ButtonDown, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnMouseEnter(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::CursorEnter, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnMouseExit(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::CursorExit, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnSetFocus(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::GetFocus, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnKillFocus(bool & bExit)
{
	BeforeNotify();
	bool bRet = false;//DoAAction(CPDF_AAction::AActionType::LoseFocus, bExit);
	AfterNotify();
	return bRet;
}

bool CFFL_Notify::OnCalculate()
{
	return true;
}

bool CFFL_Notify::OnFormat(int iCommitKey)
{
	return true;
}

bool CFFL_Notify::OnKeyStroke(CPDF_FormField* pFormField, int nCommitKey, CFX_WideString& strValue, CFX_WideString& strChange,
							   const CFX_WideString& strChangeEx, bool bKeyDown, bool bModifier,
							   bool bShift, bool bWillCommit, bool bFieldFull,
							   int& nSelStart, int& nSelEnd, bool& bRC)
{
	return true;
}

bool CFFL_Notify::OnValidate(CPDF_FormField* pFormField, CFX_WideString& strValue, CFX_WideString & strChange,
									   const CFX_WideString& strChangeEx, bool bKeyDown, bool bModifier,
									   bool bShift, bool & bRC)
{
	return true;
}

bool	CFFL_Notify::DoAAction(CPDF_AAction::AActionType eAAT, bool & bExit)
{
    if (m_bDoActioning)
        return false;

    CPDF_Action action;
    if (!FindAAction(eAAT, action))
        return false;

    m_bDoActioning = true;
    ExecuteActionTree(eAAT,action,bExit);
    m_bDoActioning = false;
    return true;
}

bool	CFFL_Notify::ExecuteActionTree(CPDF_AAction::AActionType eAAT,CPDF_Action & action, bool& bExit)
{
	if (!ExecuteAction(eAAT,action,bExit)) return false;
	if (bExit) return true;

	for (int32_t i=0,sz=action.GetSubActionsCount(); i<sz; i++)
	{
		CPDF_Action subaction = action.GetSubAction(i);
		if (!ExecuteActionTree(eAAT,subaction,bExit)) return false;
		if (bExit) break;
	}

	return true;
}


bool	CFFL_Notify::FindAAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action)
{
	return false;
}

bool CFFL_Notify::FindAAction(CPDF_AAction aaction,CPDF_AAction::AActionType eAAT,CPDF_Action & action)
{
	CPDF_Action MyAction;

	if (aaction.ActionExist(eAAT))
	{
		MyAction = aaction.GetAction(eAAT);
	}
	else
		return false;


	if (MyAction.GetType() == CPDF_Action::Unknown)
		return false;

	action = MyAction;

	return true;
}

bool	CFFL_Notify::ExecuteAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action,bool& bExit)
{
	return false;
}
//#pragma warning(default: 4800)

