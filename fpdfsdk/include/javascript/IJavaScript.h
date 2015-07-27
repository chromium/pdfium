// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_
#define FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_

#include "../../../core/include/fxcrt/fx_string.h"
#include "../../../core/include/fxcrt/fx_system.h"

class CPDF_Bookmark;
class CPDF_FormField;
class CPDFSDK_Annot;
class CPDFSDK_Document;

class IFXJS_Context
{
public:
        virtual ~IFXJS_Context() { }
	virtual bool				Compile(const CFX_WideString& script, CFX_WideString& info) = 0;
	virtual bool				RunScript(const CFX_WideString& script, CFX_WideString& info) = 0;

	virtual void				OnApp_Init() = 0;

	virtual void				OnDoc_Open(CPDFSDK_Document* pDoc, const CFX_WideString& strTargetName) = 0;
	virtual void				OnDoc_WillPrint(CPDFSDK_Document* pDoc) = 0;
	virtual void				OnDoc_DidPrint(CPDFSDK_Document* pDoc) = 0;
	virtual void				OnDoc_WillSave(CPDFSDK_Document* pDoc) = 0;
	virtual void				OnDoc_DidSave(CPDFSDK_Document* pDoc) = 0;
	virtual void				OnDoc_WillClose(CPDFSDK_Document* pDoc) = 0;

	virtual void				OnPage_Open(CPDFSDK_Document* pTarget) = 0;
	virtual void				OnPage_Close(CPDFSDK_Document* pTarget) = 0;
	virtual void				OnPage_InView(CPDFSDK_Document* pTarget) = 0;
	virtual void				OnPage_OutView(CPDFSDK_Document* pTarget) = 0;

	virtual void				OnField_MouseDown(bool bModifier, bool bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseEnter(bool bModifier, bool bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseExit(bool bModifier, bool bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseUp(bool bModifier, bool bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_Focus(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value) = 0;
	virtual void				OnField_Blur(bool bModifier, bool bShift, CPDF_FormField* pTarget, const CFX_WideString& Value) = 0;

	virtual void				OnField_Calculate(CPDF_FormField* pSource, CPDF_FormField* pTarget, CFX_WideString& Value, bool& bRc) = 0;
	virtual void				OnField_Format(CPDF_FormField* pTarget, CFX_WideString& Value, bool bWillCommit) = 0;
	virtual void				OnField_Keystroke(CFX_WideString& strChange, const CFX_WideString& strChangeEx,
									bool KeyDown, bool bModifier, int &nSelEnd,int &nSelStart, bool bShift,
									CPDF_FormField* pTarget, CFX_WideString& Value, bool bWillCommit,
									bool bFieldFull, bool &bRc) = 0;
	virtual void				OnField_Validate(CFX_WideString& strChange, const CFX_WideString& strChangeEx, bool bKeyDown,
									bool bModifier, bool bShift, CPDF_FormField* pTarget, CFX_WideString& Value, bool& bRc) = 0;

	virtual void				OnScreen_Focus(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Blur(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Open(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Close(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseDown(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseUp(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseEnter(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseExit(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_InView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_OutView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen) = 0;

	virtual void				OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) = 0;
	virtual void				OnLink_MouseUp(CPDFSDK_Document* pTarget) = 0;

	virtual void				OnMenu_Exec(CPDFSDK_Document* pTarget, const CFX_WideString &) = 0;
	virtual void				OnBatchExec(CPDFSDK_Document* pTarget) = 0;
	virtual void				OnConsole_Exec() = 0;
	virtual void				OnExternal_Exec() = 0;

	virtual void				EnableMessageBox(bool bEnable) = 0;
};

class IFXJS_Runtime
{
public:
	virtual IFXJS_Context*		NewContext() = 0;
	virtual void				ReleaseContext(IFXJS_Context * pContext) = 0;
	virtual IFXJS_Context*		GetCurrentContext() = 0;

	virtual void				SetReaderDocument(CPDFSDK_Document* pReaderDoc) = 0;
	virtual	CPDFSDK_Document*	GetReaderDocument() = 0;

protected:
         ~IFXJS_Runtime() { }
};

class CPDFDoc_Environment;
class CJS_GlobalData;

class CJS_RuntimeFactory
{
public:
	CJS_RuntimeFactory():m_bInit(false),m_nRef(0),m_pGlobalData(NULL),m_nGlobalDataCount(0) {}
	~CJS_RuntimeFactory();
	IFXJS_Runtime*					NewJSRuntime(CPDFDoc_Environment* pApp);
	void							DeleteJSRuntime(IFXJS_Runtime* pRuntime);
	void							AddRef();
	void							Release();

	CJS_GlobalData*					NewGlobalData(CPDFDoc_Environment* pApp);
	void							ReleaseGlobalData();
private:
	bool m_bInit;
	int m_nRef;
	CJS_GlobalData*					m_pGlobalData;
	int32_t						m_nGlobalDataCount;
};

#endif  // FPDFSDK_INCLUDE_JAVASCRIPT_IJAVASCRIPT_H_
