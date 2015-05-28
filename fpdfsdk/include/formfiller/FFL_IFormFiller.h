// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_IFORMFILLER_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_IFORMFILLER_H_

#include "FormFiller.h"

class CFFL_FormFiller;
class CFFL_PrivateData;

#define CFFL_Widget2Filler		CFX_MapPtrTemplate<CPDFSDK_Annot*, CFFL_FormFiller*>

// #define IsALTpressed()			(GetKeyState(VK_MENU) < 0)
// #define IsCTRLpressed()			(GetKeyState(VK_CONTROL) < 0)
// #define IsSHIFTpressed()		(GetKeyState(VK_SHIFT)&0x8000)
// #define IsINSERTpressed()		(GetKeyState(VK_INSERT) & 0x01)	
// #define VK_SHIFT          0x10
// #define VK_CONTROL        0x11
// #define VK_MENU           0x12
// #define VK_RETURN         0x0D
// #define VK_SPACE          0x20
// #define VK_ESCAPE         0x1B



class CFFL_IFormFiller :/* public IBA_AnnotFiller, */public IPWL_Filler_Notify//, 
//	public IUndo_EventHandler, public IClipboard_Handler
{
public:
	CFFL_IFormFiller(CPDFDoc_Environment* pApp);
	virtual ~CFFL_IFormFiller();

	virtual FX_BOOL				Annot_HitTest(CPDFSDK_PageView* pPageView,CPDFSDK_Annot* pAnnot, CPDF_Point point);
	virtual FX_RECT				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot, 
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									/*const CRect& rcWindow,*/ FX_DWORD dwFlags);


	virtual void				OnCreate(CPDFSDK_Annot* pAnnot);
	virtual void				OnLoad(CPDFSDK_Annot* pAnnot);
	virtual void				OnDelete(CPDFSDK_Annot* pAnnot);

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

	virtual FX_BOOL				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, short zDelta, const CPDF_Point& point);
	virtual FX_BOOL				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnRButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);

	virtual FX_BOOL				OnKeyDown(CPDFSDK_Annot* pAnnot, FX_UINT nKeyCode, FX_UINT nFlags);
	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);

	virtual	void				OnDeSelected(CPDFSDK_Annot* pAnnot);
	virtual	void				OnSelected(CPDFSDK_Annot* pAnnot);

	virtual FX_BOOL				OnSetFocus(CPDFSDK_Annot* pAnnot,FX_UINT nFlag);
	virtual FX_BOOL				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

public:
	virtual void				QueryWherePopup(void* pPrivateData, FX_FLOAT fPopupMin,FX_FLOAT fPopupMax, FX_INT32 & nRet, FX_FLOAT & fPopupRet);
	virtual void				OnBeforeKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_INT32 nKeyCode,
										CFX_WideString & strChange, const CFX_WideString& strChangeEx, 
										int nSelStart, int nSelEnd,
										FX_BOOL bKeyDown, FX_BOOL & bRC, FX_BOOL & bExit, FX_DWORD nFlag);
	virtual void				OnAfterKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_BOOL & bExit, FX_DWORD nFlag) ;

	virtual void				OnSetWindowRect(void* pPrivateData, const CPDF_Rect & rcWindow);
	virtual void				OnKeyStroke(FX_BOOL bEditOrList, void* pPrivateData, FX_INT32 nKeyCode, CFX_WideString & strChange, 
									const CFX_WideString& strChangeEx, FX_BOOL bKeyDown, FX_BOOL & bRC, FX_BOOL & bExit);

public:
	virtual void				BeforeUndo(CPDFSDK_Document* pDocument);
	virtual void				BeforeRedo(CPDFSDK_Document* pDocument);
	virtual void				AfterUndo(CPDFSDK_Document* pDocument);
	virtual void				AfterRedo(CPDFSDK_Document* pDocument);

public:
	virtual FX_BOOL				CanCopy(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanCut(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanPaste(CPDFSDK_Document* pDocument); 

	virtual void				DoCopy(CPDFSDK_Document* pDocument); 
	virtual void				DoCut(CPDFSDK_Document* pDocument); 
	virtual void				DoPaste(CPDFSDK_Document* pDocument); 

public:
	CFFL_FormFiller*			GetFormFiller(CPDFSDK_Annot* pAnnot, FX_BOOL bRegister);
	void						RemoveFormFiller(CPDFSDK_Annot* pAnnot);

	static FX_BOOL				IsVisible(CPDFSDK_Widget* pWidget);
	static FX_BOOL				IsReadOnly(CPDFSDK_Widget* pWidget);
	static FX_BOOL				IsFillingAllowed(CPDFSDK_Widget* pWidget);
 	static FX_BOOL				IsValidAnnot(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot);

	void						OnKeyStrokeCommit(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bRC, FX_BOOL& bExit, FX_DWORD nFlag);
	void						OnValidate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bRC, FX_BOOL& bExit, FX_DWORD nFlag);

	void						OnCalculate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bExit, FX_DWORD nFlag);
	void						OnFormat(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bExit, FX_DWORD nFlag);
	void						OnButtonUp(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, FX_BOOL& bReset, FX_BOOL& bExit,FX_UINT nFlag);
// 	static LRESULT CALLBACK		FFL_WndProc(
// 									  int code,       // hook code
// 									  WPARAM wParam,  // virtual-key code
// 									  LPARAM lParam   // keystroke-message information
// 										);
// 	static MSG					GetLastMessage();
	static int					GetCommitKey();
	static FX_BOOL				GetKeyDown();


public:
// 	static MSG					g_Msg;
// 	static HHOOK				m_hookSheet;

private:
	void						UnRegisterFormFiller(CPDFSDK_Annot* pAnnot);
	void						SetFocusAnnotTab(CPDFSDK_Annot* pWidget, FX_BOOL bSameField, FX_BOOL bNext);

private:
	CPDFDoc_Environment*				m_pApp;
	CFFL_Widget2Filler			m_Maps;
	FX_BOOL						m_bNotifying;
};

class CFFL_PrivateData
{
public:
	CPDFSDK_Widget*			pWidget;
	CPDFSDK_PageView*	pPageView;
	int					nWidgetAge;
	int					nValueAge;
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_IFORMFILLER_H_

