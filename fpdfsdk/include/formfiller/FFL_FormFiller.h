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

class CFFL_FormFiller : public IPWL_Provider, public CPWL_TimerHandler
{
public:
	CFFL_FormFiller(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot);
	virtual ~CFFL_FormFiller();

	virtual FX_RECT				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									FX_DWORD dwFlags);
	virtual void				OnDrawDeactive(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									FX_DWORD dwFlags);

	virtual void				OnCreate(CPDFSDK_Annot* pAnnot);
	virtual void				OnLoad(CPDFSDK_Annot* pAnnot);
	virtual void				OnDelete(CPDFSDK_Annot* pAnnot);

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);

	virtual bool				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, short zDelta, const CPDF_Point& point);
	virtual bool				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);

	virtual bool				OnKeyDown(CPDFSDK_Annot* pAnnot, FX_UINT nKeyCode, FX_UINT nFlags);
	virtual bool				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);

	virtual bool				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);
	virtual bool				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

	virtual bool				CanCopy(CPDFSDK_Document* pDocument);
	virtual bool				CanCut(CPDFSDK_Document* pDocument);
	virtual bool				CanPaste(CPDFSDK_Document* pDocument);

	virtual void				DoCopy(CPDFSDK_Document* pDocument);
	virtual void				DoCut(CPDFSDK_Document* pDocument);
	virtual void				DoPaste(CPDFSDK_Document* pDocument);

        // CPWL_TimerHandler
	virtual void				TimerProc();
	virtual IFX_SystemHandler*	GetSystemHandler() const;

	virtual CPDF_Matrix			GetWindowMatrix(void* pAttachedData);
	virtual CFX_WideString		LoadPopupMenuString(int nIndex);

 	virtual void				GetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type,
 									PDFSDK_FieldAction& fa);
 	virtual void				SetActionData(CPDFSDK_PageView* pPageView, CPDF_AAction::AActionType type,
 									const PDFSDK_FieldAction& fa);
 	virtual bool				IsActionDataChanged(CPDF_AAction::AActionType type, const PDFSDK_FieldAction& faOld,
 									const PDFSDK_FieldAction& faNew);

	virtual void				SaveState(CPDFSDK_PageView* pPageView);
	virtual void				RestoreState(CPDFSDK_PageView* pPageView);

	virtual CPWL_Wnd* 			ResetPDFWindow(CPDFSDK_PageView* pPageView, bool bRestoreValue);

	virtual void				OnKeyStroke(bool bKeyDown);

	CPDF_Matrix					GetCurMatrix();

	CPDF_Rect					FFLtoPWL(const CPDF_Rect& rect);
	CPDF_Rect					PWLtoFFL(const CPDF_Rect& rect);
	CPDF_Point					FFLtoPWL(const CPDF_Point& point);
	CPDF_Point					PWLtoFFL(const CPDF_Point& point);

	CPDF_Point					WndtoPWL(CPDFSDK_PageView* pPageView, const CPDF_Point& pt);
	CPDF_Rect					FFLtoWnd(CPDFSDK_PageView* pPageView, const CPDF_Rect& rect);

	void						SetWindowRect(CPDFSDK_PageView* pPageView, const CPDF_Rect& rcWindow);
	CPDF_Rect					GetWindowRect(CPDFSDK_PageView* pPageView);

	bool						CommitData(CPDFSDK_PageView* pPageView, FX_UINT nFlag);
	virtual bool				IsDataChanged(CPDFSDK_PageView* pPageView);
	virtual void				SaveData(CPDFSDK_PageView* pPageView);

	CPWL_Wnd*					GetPDFWindow(CPDFSDK_PageView* pPageView, bool bNew);
	void						DestroyPDFWindow(CPDFSDK_PageView* pPageView);
	void						EscapeFiller(CPDFSDK_PageView* pPageView, bool bDestroyPDFWindow);

	virtual	PWL_CREATEPARAM		GetCreateParam();
	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView) = 0;
	virtual CPDF_Rect			GetFocusBox(CPDFSDK_PageView* pPageView);

	bool						IsValid() const;
	CPDF_Rect					GetPDFWindowRect() const;

	CPDFSDK_PageView*			GetCurPageView();
	void						SetChangeMark();

	virtual void				InvalidateRect(double left, double top, double right, double bottom);
	CPDFDoc_Environment*		GetApp(){return m_pApp;}
	CPDFSDK_Annot*				GetSDKAnnot() {return m_pAnnot;}

protected:
    using CFFL_PageView2PDFWindow = std::map<CPDFSDK_PageView*, CPWL_Wnd*>;

    CPDFDoc_Environment* m_pApp;
    CPDFSDK_Widget* m_pWidget;
    CPDFSDK_Annot* m_pAnnot;

    bool m_bValid;
    CFFL_PageView2PDFWindow m_Maps;
    CPDF_Point m_ptOldPos;
};

class CFFL_Button : public CFFL_FormFiller
{
public:
	CFFL_Button(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pWidget);
	virtual ~CFFL_Button();

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual bool				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									FX_DWORD dwFlags);

	virtual	void				OnDrawDeactive(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									FX_DWORD dwFlags);
protected:
	bool						m_bMouseIn;
	bool						m_bMouseDown;
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_FORMFILLER_H_
