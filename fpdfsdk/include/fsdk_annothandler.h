// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FSDK_ANNOTHANDLER_H_
#define _FSDK_ANNOTHANDLER_H_


class CPDFDoc_Environment;
class CFFL_IFormFiller;
class CPDFSDK_PageView;
class IPDFSDK_AnnotHandler
{

public:
	virtual ~IPDFSDK_AnnotHandler() {};

	virtual CFX_ByteString		GetType() = 0;

	virtual CFX_ByteString		GetName() = 0;

	virtual FX_BOOL				CanAnswer(CPDFSDK_Annot* pAnnot) = 0;
	

	virtual CPDFSDK_Annot*		NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage) = 0;

	virtual void				ReleaseAnnot(CPDFSDK_Annot* pAnnot) = 0;

	virtual void				DeleteAnnot(CPDFSDK_Annot* pAnnot) = 0;
	

	virtual CPDF_Rect				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot) = 0;
	
	virtual FX_BOOL				HitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point) = 0;
	

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
	

	virtual FX_BOOL				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
	virtual FX_BOOL				OnRButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) = 0;
//by wjm.
	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags) = 0;
	virtual FX_BOOL				OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) = 0;
	virtual FX_BOOL				OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag) =0 ;

	virtual	void				OnDeSelected(CPDFSDK_Annot* pAnnot) = 0;
	virtual	void				OnSelected(CPDFSDK_Annot* pAnnot) = 0;

	virtual FX_BOOL				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;
	virtual FX_BOOL				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag) = 0;

};


class CPDFSDK_BFAnnotHandler:public IPDFSDK_AnnotHandler
{
public:
	CPDFSDK_BFAnnotHandler(CPDFDoc_Environment* pApp) : m_pApp(pApp), m_pFormFiller(NULL) {}
	virtual	~CPDFSDK_BFAnnotHandler() {}
public:

	virtual CFX_ByteString		GetType()  {return CFX_ByteString("Widget");}

	virtual CFX_ByteString		GetName()  {return CFX_ByteString("WidgetHandler");}
	
	virtual FX_BOOL				CanAnswer(CPDFSDK_Annot* pAnnot);

	virtual CPDFSDK_Annot*		NewAnnot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPage);

	virtual void				ReleaseAnnot(CPDFSDK_Annot* pAnnot)  ;

	virtual void				DeleteAnnot(CPDFSDK_Annot* pAnnot) {}
	

	virtual CPDF_Rect				GetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot) ;
	
	virtual FX_BOOL				HitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point);
	

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
	

	virtual FX_BOOL				OnLButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnLButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnMouseMove(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnMouseWheel(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point) ;
	virtual FX_BOOL				OnRButtonDown(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnRButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) ;
	virtual FX_BOOL				OnRButtonDblClk(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point) {return FALSE;}
	
//by wjm.
	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags);
	virtual FX_BOOL				OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
	virtual FX_BOOL				OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

	virtual	void				OnDeSelected(CPDFSDK_Annot* pAnnot) {}
	virtual	void				OnSelected(CPDFSDK_Annot* pAnnot) {}

	virtual FX_BOOL				OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
	virtual FX_BOOL				OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

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

	virtual FX_BOOL				Annot_OnLButtonDown(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual FX_BOOL				Annot_OnLButtonUp(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual FX_BOOL				Annot_OnLButtonDblClk(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	
	virtual FX_BOOL				Annot_OnMouseMove(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual FX_BOOL				Annot_OnMouseWheel(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, short zDelta, const CPDF_Point& point);
	virtual FX_BOOL				Annot_OnRButtonDown(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);
	virtual FX_BOOL				Annot_OnRButtonUp(CPDFSDK_PageView * pPageView, CPDFSDK_Annot* pAnnot, FX_DWORD nFlags, const CPDF_Point& point);


	virtual FX_BOOL				Annot_OnChar(CPDFSDK_Annot* pAnnot, FX_DWORD nChar, FX_DWORD nFlags);
	virtual FX_BOOL				Annot_OnKeyDown(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);
	virtual FX_BOOL				Annot_OnKeyUp(CPDFSDK_Annot* pAnnot, int nKeyCode, int nFlag);

	virtual FX_BOOL				Annot_OnSetFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);
	virtual FX_BOOL				Annot_OnKillFocus(CPDFSDK_Annot* pAnnot, FX_DWORD nFlag);

	virtual CPDF_Rect			Annot_OnGetViewBBox(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot);
	virtual FX_BOOL				Annot_OnHitTest(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, const CPDF_Point& point);

private:
	IPDFSDK_AnnotHandler*			GetAnnotHandler(const CFX_ByteString& sType) const;
	CPDFSDK_Annot*				GetNextAnnot(CPDFSDK_Annot* pSDKAnnot,FX_BOOL bNext);
private:
	CBA_AnnotHandlerArray		m_Handlers;
	CFX_MapByteStringToPtr		m_mapType2Handler;
	CPDFDoc_Environment*		m_pApp;
};

//#define CBF_Page2Accessible	 CFX_MapPtrTemplate<CPDFSDK_PageView*, IAccessible*>

typedef int (*AI_COMPARE) (CPDFSDK_Annot* p1, CPDFSDK_Annot* p2);

class CPDFSDK_AnnotIterator
{
public:
    CPDFSDK_AnnotIterator(CPDFSDK_PageView * pPageView, FX_BOOL bReverse,
		FX_BOOL bIgnoreTopmost=FALSE,FX_BOOL bCircle=FALSE,CFX_PtrArray* pList=NULL);
    virtual ~CPDFSDK_AnnotIterator() { }

	virtual CPDFSDK_Annot*	Next (const CPDFSDK_Annot* pCurrent) ;
	virtual CPDFSDK_Annot*	Prev (const CPDFSDK_Annot* pCurrent) ;	
	virtual CPDFSDK_Annot*	Next(int& index ) ;
	virtual CPDFSDK_Annot*	Prev(int& index ) ;
	virtual int             Count(){return m_pIteratorAnnotList.GetSize();}

	virtual FX_BOOL         InitIteratorAnnotList(CPDFSDK_PageView * pPageView,CFX_PtrArray* pList=NULL);

	void					InsertSort(CFX_PtrArray &arrayList, AI_COMPARE pCompare);

protected:
	CPDFSDK_Annot*	NextAnnot (const CPDFSDK_Annot* pCurrent) ;
	CPDFSDK_Annot*	PrevAnnot (const CPDFSDK_Annot* pCurrent) ;
	CPDFSDK_Annot*	NextAnnot(int& index ) ;
	CPDFSDK_Annot*	PrevAnnot(int& index ) ;

	CFX_PtrArray	     m_pIteratorAnnotList;
	FX_BOOL			     m_bReverse;
	FX_BOOL              m_bIgnoreTopmost;
	FX_BOOL              m_bCircle;
};



#endif

