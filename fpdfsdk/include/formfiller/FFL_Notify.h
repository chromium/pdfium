// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_NOTIFY_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_NOTIFY_H_

#include "../../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../../core/include/fxcrt/fx_string.h"

class CFFL_FormFiller;
class CPDF_FormField;

class CFFL_Notify
{
public:
	CFFL_Notify(CFFL_FormFiller * pFormFiller);
	virtual ~CFFL_Notify();

public:
	bool									OnSetFocus(bool & bExit);
	bool									OnMouseEnter(bool & bExit);
	bool									OnMouseDown(bool & bExit);
	bool									OnMouseUp(bool & bExit);
	bool									OnMouseExit(bool & bExit);
	bool									OnKillFocus(bool & bExit);

	bool									OnCalculate();
	bool									OnFormat(int iCommitKey);
	bool									OnValidate(CPDF_FormField* pFormField, CFX_WideString& strValue, CFX_WideString & strChange,
											   const CFX_WideString& strChangeEx, bool bKeyDown, bool bModifier,
											   bool bShift, bool & bRC);
	bool									OnKeyStroke(CPDF_FormField* pFormField, int nCommitKey, CFX_WideString& strValue, CFX_WideString& strChange,
											   const CFX_WideString& strChangeEx, bool bKeyDown, bool bModifier,
											   bool bShift, bool bWillCommit, bool bFieldFull,
											   int& nSelStart, int& nSelEnd, bool& bRC);

	void									BeforeNotify();
	void									AfterNotify();
	bool									IsNotifying() const {return m_nNotifyFlag > 0;}

private:
 	bool									DoAAction(CPDF_AAction::AActionType eAAT, bool & bExit);
 	bool									FindAAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action);
 	bool									FindAAction(CPDF_AAction aaction,CPDF_AAction::AActionType eAAT,CPDF_Action & action);
 	bool									ExecuteActionTree(CPDF_AAction::AActionType eAAT, CPDF_Action & action, bool& bExit);
 	bool									ExecuteAction(CPDF_AAction::AActionType eAAT,CPDF_Action & action,bool& bExit);

	bool									m_bDoActioning;
	int32_t								m_nNotifyFlag;
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_NOTIFY_H_
