// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_JS_CONTEXT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_JS_CONTEXT_H_

#include "../../../core/include/fxcrt/fx_system.h"
#include "../../../core/include/fxcrt/fx_string.h"
#include "IJavaScript.h"

class CJS_EventHandler;
class CJS_Runtime;

class CJS_Context : public IFXJS_Context
{
public:
	CJS_Context(CJS_Runtime* pRuntime);
	virtual ~CJS_Context();

public:
	virtual bool				Compile(const CFX_WideString& script, CFX_WideString& info);
	virtual bool				RunScript(const CFX_WideString& script, CFX_WideString& info);

public:
	virtual void				OnApp_Init();

	virtual void				OnDoc_Open(CPDFSDK_Document* pDoc, const CFX_WideString& strTargetName);
	virtual void				OnDoc_WillPrint(CPDFSDK_Document* pDoc);
	virtual void				OnDoc_DidPrint(CPDFSDK_Document* pDoc);
	virtual void				OnDoc_WillSave(CPDFSDK_Document* pDoc);
	virtual void				OnDoc_DidSave(CPDFSDK_Document* pDoc);
	virtual void				OnDoc_WillClose(CPDFSDK_Document* pDoc);

	virtual void				OnPage_Open(CPDFSDK_Document* pTarget);
	virtual void				OnPage_Close(CPDFSDK_Document* pTarget);
	virtual void				OnPage_InView(CPDFSDK_Document* pTarget);
	virtual void				OnPage_OutView(CPDFSDK_Document* pTarget);

	virtual void				OnField_MouseDown(bool bModifier, bool bShift, CPDF_FormField *pTarget);
	virtual void				OnField_MouseEnter(bool bModifier, bool bShift, CPDF_FormField *pTarget);
	virtual void				OnField_MouseExit(bool bModifier, bool bShift, CPDF_FormField *pTarget);
	virtual void				OnField_MouseUp(bool bModifier, bool bShift, CPDF_FormField *pTarget);
	virtual void				OnField_Focus(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value);
	virtual void				OnField_Blur(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value);

	virtual void				OnField_Calculate(CPDF_FormField* pSource, CPDF_FormField* pTarget, CFX_WideString& Value, bool& bRc);
	virtual void				OnField_Format(CPDF_FormField* pTarget, CFX_WideString& Value, bool bWillCommit);
	virtual void				OnField_Keystroke(CFX_WideString& strChange, const CFX_WideString& strChangeEx,
									bool bKeyDown, bool bModifier, int &nSelEnd,int &nSelStart, bool bShift,
									CPDF_FormField* pTarget, CFX_WideString& Value, bool bWillCommit,
									bool bFieldFull, bool &bRc);
	virtual void				OnField_Validate(CFX_WideString& strChange, const CFX_WideString& strChangeEx, bool bKeyDown,
									bool bModifier, bool bShift, CPDF_FormField* pTarget, CFX_WideString& Value, bool& bRc);

	virtual void				OnScreen_Focus(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_Blur(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_Open(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_Close(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_MouseDown(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_MouseUp(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_MouseEnter(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_MouseExit(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_InView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
	virtual void				OnScreen_OutView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);

	virtual void				OnBookmark_MouseUp(CPDF_Bookmark* pBookMark);
	virtual void				OnLink_MouseUp(CPDFSDK_Document* pTarget);

	virtual void				OnMenu_Exec(CPDFSDK_Document* pTarget, const CFX_WideString& strTargetName);
	virtual void				OnBatchExec(CPDFSDK_Document* pTarget);
	virtual void				OnConsole_Exec();
	virtual void				OnExternal_Exec();

	virtual void				EnableMessageBox(bool bEnable) {m_bMsgBoxEnable = bEnable;}
	bool						IsMsgBoxEnabled() const {return m_bMsgBoxEnable;}

public:
	CPDFDoc_Environment*			GetReaderApp();
	CJS_Runtime*				GetJSRuntime(){return m_pRuntime;}

	bool						DoJob(int nMode, const CFX_WideString& script, CFX_WideString& info);

	CJS_EventHandler*			GetEventHandler(){return m_pEventHandler;};
	CPDFSDK_Document*			GetReaderDocument();

private:
	CJS_Runtime*				m_pRuntime;
	CJS_EventHandler*			m_pEventHandler;

	bool						m_bBusy;
	bool						m_bMsgBoxEnable;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_JS_CONTEXT_H_
