// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_IFORMFILLER_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_IFORMFILLER_H_

#include <map>

#include "FormFiller.h"

class CFFL_FormFiller;
class CFFL_PrivateData;

class CFFL_IFormFiller : public IPWL_Filler_Notify
{
public:
	CFFL_IFormFiller(CPDFDoc_Environment* pApp);
	virtual ~CFFL_IFormFiller();

	virtual bool				Annot_HitTest(CPDFSDK_PageView* pPageView,CPDFSDK_Annot* pAnnot, CPDF_Point point);
	virtual FX_RECT				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView, /*HDC hDC,*/ CPDFSDK_Annot* pAnnot,
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									/*const CRect& rcWindow,*/ FX_DWORD dwFlags);


	virtual void				OnCreate(CPDFSDK_Annot* pAnnot);
	virtual void				OnLoad(CPDFSDK_Annot* pAnnot);
	virtual void				OnDelete(CPDFSDK_Annot* pAnnot);

	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag);
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

	virtual bool				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, short zDelta, const CPDF_Point& point);
	virtual bool				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual bool				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);

	virtual bool				OnKeyDown(CPDFSDK_Annot* pAnnot, FX_UINT nKeyCode, FX_UINT nFlags);
	virtual bool				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);

	virtual bool				OnSetFocus(CPDFSDK_Annot* pAnnot,FX_UINT nFlag);
	virtual bool				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_UINT nFlag);

	virtual void				QueryWherePopup(void* pPrivateData, FX_FLOAT fPopupMin,FX_FLOAT fPopupMax, int32_t & nRet, FX_FLOAT & fPopupRet);
	virtual void				OnBeforeKeyStroke(bool bEditOrList, void* pPrivateData, int32_t nKeyCode,
										CFX_WideString & strChange, const CFX_WideString& strChangeEx,
										int nSelStart, int nSelEnd,
										bool bKeyDown, bool & bRC, bool & bExit, FX_DWORD nFlag);
	virtual void				OnAfterKeyStroke(bool bEditOrList, void* pPrivateData, bool & bExit, FX_DWORD nFlag) ;

	CFFL_FormFiller*			GetFormFiller(CPDFSDK_Annot* pAnnot, bool bRegister);
	void						RemoveFormFiller(CPDFSDK_Annot* pAnnot);

	static bool				IsVisible(CPDFSDK_Widget* pWidget);
	static bool				IsReadOnly(CPDFSDK_Widget* pWidget);
	static bool				IsFillingAllowed(CPDFSDK_Widget* pWidget);
 	static bool				IsValidAnnot(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot);

	void						OnKeyStrokeCommit(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, bool& bRC, bool& bExit, FX_DWORD nFlag);
	void						OnValidate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, bool& bRC, bool& bExit, FX_DWORD nFlag);

	void						OnCalculate(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, bool& bExit, FX_DWORD nFlag);
	void						OnFormat(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, bool& bExit, FX_DWORD nFlag);
	void						OnButtonUp(CPDFSDK_Widget* pWidget, CPDFSDK_PageView* pPageView, bool& bReset, bool& bExit,FX_UINT nFlag);

private:
    using CFFL_Widget2Filler = std::map<CPDFSDK_Annot*, CFFL_FormFiller*>;

    void UnRegisterFormFiller(CPDFSDK_Annot* pAnnot);

    CPDFDoc_Environment* m_pApp;
    CFFL_Widget2Filler m_Maps;
    bool m_bNotifying;
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

