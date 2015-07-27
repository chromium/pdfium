// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_
#define FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_

#include "../../core/include/fxcrt/fx_basic.h"

class CFFL_IFormFiller;
class CFX_RenderDevice;
class CPDFDoc_Environment;
class CPDFSDK_Annot;
class CPDFSDK_PageView;
class CPDF_Annot;
class CPDF_Matrix;
class CPDF_Point;
class CPDF_Rect;

class IPDFSDK_AnnotHandler
{

public:
	virtual ~IPDFSDK_AnnotHandler() {};

	virtual CFX_ByteString		GetType() = 0;

	virtual CFX_ByteString		GetName() = 0;

	virtual bool				CanAnswer(CPDFSDK_Annot* pAnnot) = 0;


	virtual CPDFSDK_Annot*		NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage) = 0;

	virtual void				ReleaseAnnot(CPDFSDK_Annot* pAnnot) = 0;

	virtual void				DeleteAnnot(CPDFSDK_Annot* pAnnot) = 0;


	virtual CPDF_Rect				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot) = 0;

	virtual bool				HitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point) = 0;


	virtual void				OnDraw(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
		CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
		FX_DWORD dwFlags) = 0;

	virtual void				OnDrawSleep(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
		CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
		const CPDF_Rect& rcWindow, FX_DWORD dwFlags) = 0;




	virtual void				OnCreate(CPDFSDK_Annot* pAnnot) = 0;

	virtual void				OnLoad(CPDFSDK_Annot* pAnnot) = 0;

	virtual void				OnDelete(CPDFSDK_Annot* pAnnot) = 0;

	virtual void				OnRelease(CPDFSDK_Annot* pAnnot) = 0;


	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;


	virtual bool				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point) = 0;
	virtual bool				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual bool				OnRButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
//by wjm.
	virtual bool				OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags) = 0;
	virtual bool				OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) = 0;
	virtual bool				OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) =0 ;

	virtual	void				OnDeSelected(CPDFSDK_Annot* pAnnot) = 0;
	virtual	void				OnSelected(CPDFSDK_Annot* pAnnot) = 0;

	virtual bool				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;
	virtual bool				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;

};


class CPDFSDK_BFAnnotHandler:public IPDFSDK_AnnotHandler
{
public:
	CPDFSDK_BFAnnotHandler(CPDFDoc_Environment* pApp) : m_pApp(pApp), m_pFormFiller(NULL) {}
	virtual	~CPDFSDK_BFAnnotHandler() {}
public:

	virtual CFX_ByteString		GetType()  {return CFX_ByteString("Widget");}

	virtual CFX_ByteString		GetName()  {return CFX_ByteString("WidgetHandler");}

	virtual bool				CanAnswer(CPDFSDK_Annot* pAnnot);

	virtual CPDFSDK_Annot*		NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage);

	virtual void				ReleaseAnnot(CPDFSDK_Annot* pAnnot)  ;

	virtual void				DeleteAnnot(CPDFSDK_Annot* pAnnot) {}


	virtual CPDF_Rect				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot) ;

	virtual bool				HitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point);


	virtual void				OnDraw(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
		CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
		 FX_DWORD dwFlags) ;

	virtual void				OnDrawSleep(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot,
		CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
		const CPDF_Rect& rcWindow, FX_DWORD dwFlags) {}


	virtual void				OnCreate(CPDFSDK_Annot* pAnnot) ;

	virtual void				OnLoad(CPDFSDK_Annot* pAnnot) ;

	virtual void				OnDelete(CPDFSDK_Annot* pAnnot) {}

	virtual void				OnRelease(CPDFSDK_Annot* pAnnot) {}


	virtual void				OnMouseEnter(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) ;
	virtual void				OnMouseExit(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) ;


	virtual bool				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point) ;
	virtual bool				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual bool				OnRButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) {return false;}

//by wjm.
	virtual bool				OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags);
	virtual bool				OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
	virtual bool				OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

	virtual	void				OnDeSelected(CPDFSDK_Annot* pAnnot) {}
	virtual	void				OnSelected(CPDFSDK_Annot* pAnnot) {}

	virtual bool				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
	virtual bool				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

	void						SetFormFiller(CFFL_IFormFiller* pFiller){m_pFormFiller = pFiller;}
	CFFL_IFormFiller*			GetFormFiller() {return m_pFormFiller;}
private:

	CPDFDoc_Environment*		m_pApp;
	CFFL_IFormFiller*			m_pFormFiller;
};

#define CBA_AnnotHandlerArray CFX_ArrayTemplate<IPDFSDK_AnnotHandler*>
class CPDFSDK_AnnotHandlerMgr
{
public:
	// Destroy the handler
	CPDFSDK_AnnotHandlerMgr(CPDFDoc_Environment* pApp);
	virtual ~CPDFSDK_AnnotHandlerMgr() ;

public:
	void						RegisterAnnotHandler(IPDFSDK_AnnotHandler* pAnnotHandler);
	void						UnRegisterAnnotHandler(IPDFSDK_AnnotHandler* pAnnotHandler);

	virtual CPDFSDK_Annot*		NewAnnot(CPDF_Annot * pAnnot, CPDFSDK_PageView *pPageView);
	virtual void				ReleaseAnnot(CPDFSDK_Annot * pAnnot);

	virtual void				Annot_OnCreate(CPDFSDK_Annot* pAnnot);
	virtual void				Annot_OnLoad(CPDFSDK_Annot* pAnnot);
public:
	IPDFSDK_AnnotHandler*		GetAnnotHandler(CPDFSDK_Annot* pAnnot) const;
	virtual void				Annot_OnDraw(CPDFSDK_PageView* pPageView, CPDFSDK_Annot* pAnnot,
		CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,FX_DWORD dwFlags);

	virtual void				Annot_OnMouseEnter(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags);
	virtual void				Annot_OnMouseExit(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags);

	virtual bool				Annot_OnLButtonDown(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual bool				Annot_OnLButtonUp(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual bool				Annot_OnLButtonDblClk(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);

	virtual bool				Annot_OnMouseMove(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual bool				Annot_OnMouseWheel(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point);
	virtual bool				Annot_OnRButtonDown(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual bool				Annot_OnRButtonUp(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);


	virtual bool				Annot_OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags);
	virtual bool				Annot_OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
	virtual bool				Annot_OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

	virtual bool				Annot_OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
	virtual bool				Annot_OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

	virtual CPDF_Rect			Annot_OnGetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual bool				Annot_OnHitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point);

private:
	IPDFSDK_AnnotHandler*			GetAnnotHandler(const CFX_ByteString& sType) const;
	CPDFSDK_Annot*				GetNextAnnot(CPDFSDK_Annot* pSDKAnnot,bool bNext);
private:
	CBA_AnnotHandlerArray		m_Handlers;
	CFX_MapByteStringToPtr		m_mapType2Handler;
	CPDFDoc_Environment*		m_pApp;
};

typedef int (*AI_COMPARE) (CPDFSDK_Annot* p1, CPDFSDK_Annot* p2);

class CPDFSDK_AnnotIterator
{
public:
    CPDFSDK_AnnotIterator(CPDFSDK_PageView * pPageView, bool bReverse,
		bool bIgnoreTopmost=false,bool bCircle=false,CFX_PtrArray* pList=NULL);
    virtual ~CPDFSDK_AnnotIterator() { }

	virtual CPDFSDK_Annot*	Next (const CPDFSDK_Annot* pCurrent) ;
	virtual CPDFSDK_Annot*	Prev (const CPDFSDK_Annot* pCurrent) ;
	virtual CPDFSDK_Annot*	Next(int& index ) ;
	virtual CPDFSDK_Annot*	Prev(int& index ) ;
	virtual int             Count(){return m_pIteratorAnnotList.GetSize();}

	virtual bool         InitIteratorAnnotList(CPDFSDK_PageView * pPageView,CFX_PtrArray* pList=NULL);

	void					InsertSort(CFX_PtrArray &arrayList, AI_COMPARE pCompare);

protected:
	CPDFSDK_Annot*	NextAnnot (const CPDFSDK_Annot* pCurrent) ;
	CPDFSDK_Annot*	PrevAnnot (const CPDFSDK_Annot* pCurrent) ;
	CPDFSDK_Annot*	NextAnnot(int& index ) ;
	CPDFSDK_Annot*	PrevAnnot(int& index ) ;

	CFX_PtrArray	     m_pIteratorAnnotList;
	bool			     m_bReverse;
	bool              m_bIgnoreTopmost;
	bool              m_bCircle;
};

#endif  // FPDFSDK_INCLUDE_FSDK_ANNOTHANDLER_H_
