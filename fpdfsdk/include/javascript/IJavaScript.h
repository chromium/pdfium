// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _IJAVASCRIPT_H_
#define _IJAVASCRIPT_H_

class IFXJS_Context
{
public:
        virtual ~IFXJS_Context() { }
	virtual FX_BOOL				Compile(const CFX_WideString& script, CFX_WideString& info) = 0;
	virtual FX_BOOL				RunScript(const CFX_WideString& script, CFX_WideString& info) = 0;

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

	virtual void				OnField_MouseDown(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseEnter(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseExit(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_MouseUp(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget) = 0;
	virtual void				OnField_Focus(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget, const CFX_WideString& Value) = 0;
	virtual void				OnField_Blur(FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget, const CFX_WideString& Value) = 0;

	virtual void				OnField_Calculate(CPDF_FormField* pSource, CPDF_FormField* pTarget, CFX_WideString& Value, FX_BOOL& bRc) = 0;
	virtual void				OnField_Format(int nCommitKey, CPDF_FormField* pTarget, CFX_WideString& Value, FX_BOOL bWillCommit) = 0;
	virtual void				OnField_Keystroke(int nCommitKey, CFX_WideString& strChange, const CFX_WideString& strChangeEx,
									FX_BOOL KeyDown, FX_BOOL bModifier, int &nSelEnd,int &nSelStart, FX_BOOL bShift,
									CPDF_FormField* pTarget, CFX_WideString& Value, FX_BOOL bWillCommit, 
									FX_BOOL bFieldFull, FX_BOOL &bRc) = 0;
	virtual void				OnField_Validate(CFX_WideString& strChange, const CFX_WideString& strChangeEx, FX_BOOL bKeyDown,
									FX_BOOL bModifier, FX_BOOL bShift, CPDF_FormField* pTarget, CFX_WideString& Value, FX_BOOL& bRc) = 0;

	virtual void				OnScreen_Focus(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Blur(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Open(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_Close(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseDown(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseUp(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseEnter(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_MouseExit(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_InView(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;
	virtual void				OnScreen_OutView(FX_BOOL bModifier, FX_BOOL bShift, CPDFSDK_Annot* pScreen) = 0;

	virtual void				OnBookmark_MouseUp(CPDF_Bookmark* pBookMark) = 0;
	virtual void				OnLink_MouseUp(CPDFSDK_Document* pTarget) = 0;

	virtual void				OnMenu_Exec(CPDFSDK_Document* pTarget, const CFX_WideString &) = 0;
	virtual void				OnBatchExec(CPDFSDK_Document* pTarget) = 0;
	virtual void				OnConsole_Exec() = 0;
	virtual void				OnExternal_Exec() = 0;

	virtual void				EnableMessageBox(FX_BOOL bEnable) = 0;
};

class IFXJS_Runtime
{
public:
	virtual IFXJS_Context*		NewContext() = 0;
	virtual void				ReleaseContext(IFXJS_Context * pContext) = 0;
	virtual IFXJS_Context*		GetCurrentContext() = 0;

	virtual void				SetReaderDocument(CPDFSDK_Document* pReaderDoc) = 0;
	virtual	CPDFSDK_Document*	GetReaderDocument() = 0;	

	virtual void				GetObjectNames(CFX_WideStringArray& array) = 0;
	virtual void				GetObjectConsts(const CFX_WideString& swObjName, CFX_WideStringArray& array) = 0;
	virtual void				GetObjectProps(const CFX_WideString& swObjName, CFX_WideStringArray& array) = 0;
	virtual void				GetObjectMethods(const CFX_WideString& swObjName, CFX_WideStringArray& array) = 0;

	virtual void				Exit() = 0;
	virtual void				Enter() = 0;
	virtual FX_BOOL				IsEntered() = 0;

protected:
         ~IFXJS_Runtime() { }
};

class CPDFDoc_Environment;
class CJS_GlobalData;

class CJS_RuntimeFactory
{
public:
	CJS_RuntimeFactory():m_bInit(FALSE),m_nRef(0),m_pGlobalData(NULL),m_nGlobalDataCount(0) {}
	~CJS_RuntimeFactory();
	IFXJS_Runtime*					NewJSRuntime(CPDFDoc_Environment* pApp);
	void							DeleteJSRuntime(IFXJS_Runtime* pRuntime);
	void							AddRef();
	void							Release();

	CJS_GlobalData*					NewGlobalData(CPDFDoc_Environment* pApp);
	void							ReleaseGlobalData();
private:
	FX_BOOL m_bInit;
	int m_nRef;
	CJS_GlobalData*					m_pGlobalData;
	FX_INT32						m_nGlobalDataCount;
};

#endif //_IJAVASCRIPT_H_

