// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_FORMFILLER_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_FORMFILLER_H_

#include "FFL_IFormFiller.h"
#include "FFL_CBA_Fontmap.h"

class CPDFSDK_Annot;
class CFFL_FormFiller;
class CFFL_Notify;
class CPDFDoc_Environment;
class CPDFSDK_PageView;
class CPDFSDK_Document;
class CPDFSDK_Widget;


#define CFFL_PageView2PDFWindow		CFX_MapPtrTemplate<CPDFSDK_PageView*, CPWL_Wnd*>

struct FFL_KeyStrokeData
{
	CFX_WideString		swValue;
	FX_BOOL				bFull;
	int					nSelStart;
	int					nSelEnd;
};



class CFFL_FormFiller : /*public IBA_AnnotFiller,*/ public IPWL_Provider, public CPWL_TimerHandler
{
public:
	CFFL_FormFiller(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot);
	virtual ~CFFL_FormFiller();

	virtual FX_RECT				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot, 
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									/*const CRect& rcWindow, */FX_DWORD dwFlags);
	virtual void				OnDrawDeactive(CPDFSDK_PageView *pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot, 
								CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
								/*const CRect& rcWindow, */FX_DWORD dwFlags);

	virtual void				OnCreate(CPDFSDK_Annot* pAnnot);
	virtual void				OnLoad(CPDFSDK_Annot* pAnnot);
	virtual void				OnDelete(CPDFSDK_Annot* pAnnot);

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);

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

	virtual FX_BOOL				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);
	virtual FX_BOOL				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

	virtual FX_BOOL				CanCopy(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanCut(CPDFSDK_Document* pDocument); 
	virtual FX_BOOL				CanPaste(CPDFSDK_Document* pDocument); 

	virtual void				DoCopy(CPDFSDK_Document* pDocument); 
	virtual void				DoCut(CPDFSDK_Document* pDocument); 
	virtual void				DoPaste(CPDFSDK_Document* pDocument); 

public: //CPWL_TimerHandler
	virtual void				TimerProc();
	virtual IFX_SystemHandler*	GetSystemHandler() const;

public:
	virtual CPDF_Matrix			GetWindowMatrix(void* pAttachedData);
	virtual CFX_WideString		LoadPopupMenuString(int nIndex);

 	virtual void				GetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type,
 									PDFSDK_FieldAction& fa);
 	virtual void				SetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type, 
 									const PDFSDK_FieldAction& fa);
 	virtual FX_BOOL				IsActionDataChanged(CPDF_AAction::AActionType type, const PDFSDK_FieldAction& faOld, 
 									const PDFSDK_FieldAction& faNew);

	virtual void				SaveState(CPDFSDK_PageView* pPageView);
	virtual void				RestoreState(CPDFSDK_PageView* pPageView);

	virtual CPWL_Wnd* 			ResetPDFWindow(CPDFSDK_PageView* pPageView, FX_BOOL bRestoreValue);

	virtual void				OnKeyStroke(FX_BOOL bKeyDown);

	CPDF_Matrix					GetCurMatrix();

	CPDF_Rect					FFLtoPWL(const CPDF_Rect& rect);
	CPDF_Rect					PWLtoFFL(const CPDF_Rect& rect);
	CPDF_Point					FFLtoPWL(const CPDF_Point& point);
	CPDF_Point					PWLtoFFL(const CPDF_Point& point);

	CPDF_Point					WndtoPWL(CPDFSDK_PageView* pPageView, const CPDF_Point& pt);
	CPDF_Rect					FFLtoWnd(CPDFSDK_PageView* pPageView, const CPDF_Rect& rect);

	void						SetWindowRect(CPDFSDK_PageView* pPageView, const CPDF_Rect& rcWindow);
	CPDF_Rect					GetWindowRect(CPDFSDK_PageView* pPageView);

	static void					FFL_FreeData(void* pData);

	FX_BOOL						CommitData(CPDFSDK_PageView* pPageView, FX_UINT nFlag);
	virtual FX_BOOL				IsDataChanged(CPDFSDK_PageView* pPageView);
	virtual void				SaveData(CPDFSDK_PageView* pPageView);

	virtual void				GetKeyStrokeData(CPDFSDK_PageView* pPageView, FFL_KeyStrokeData& data);

public:
	CPWL_Wnd*					GetPDFWindow(CPDFSDK_PageView* pPageView, FX_BOOL bNew);
	void						DestroyPDFWindow(CPDFSDK_PageView* pPageView);
	void						EscapeFiller(CPDFSDK_PageView* pPageView, FX_BOOL bDestroyPDFWindow);

	virtual	PWL_CREATEPARAM		GetCreateParam();
	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView) = 0;
	virtual CPDF_Rect			GetFocusBox(CPDFSDK_PageView* pPageView);

public:
	FX_BOOL						IsValid() const;
	CPDF_Rect					GetPDFWindowRect() const;

	CPDFSDK_PageView*			GetCurPageView();
	void						SetChangeMark();

	virtual void				InvalidateRect(double left, double top, double right, double bottom);
	CPDFDoc_Environment*		GetApp(){return m_pApp;}
	CPDFSDK_Annot*				GetSDKAnnot() {return m_pAnnot;}	
protected:
	CPDFDoc_Environment*		m_pApp;
	CPDFSDK_Widget*				m_pWidget;
	CPDFSDK_Annot*				m_pAnnot;

	FX_BOOL						m_bValid;
	CFFL_PageView2PDFWindow		m_Maps;
	CPDF_Point					m_ptOldPos;
};

class CFFL_Button : public CFFL_FormFiller
{
public:
	CFFL_Button(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pWidget);
	virtual ~CFFL_Button();

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual FX_BOOL				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView/*, HDC hDC*/, CPDFSDK_Annot* pAnnot, 
								CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
								/*const CRect& rcWindow,*/ FX_DWORD dwFlags);

	virtual	void				OnDrawDeactive(CPDFSDK_PageView *pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot, 
								CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
								/*const CRect& rcWindow, */FX_DWORD dwFlags);
protected:
	FX_BOOL						m_bMouseIn;
	FX_BOOL						m_bMouseDown;
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_FORMFILLER_H_
