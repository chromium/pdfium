// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_WND_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_WND_H_

#include "../../../core/include/fxcrt/fx_basic.h"
#include "../../../core/include/fpdfdoc/fpdf_doc.h"
#include "../fx_systemhandler.h"

class CPWL_MsgControl;
class CPWL_ScrollBar;
class CPWL_Timer;
class CPWL_TimerHandler;
class CPWL_Wnd;
class IFX_Edit_FontMap;
class IFX_SystemHandler;
class IPWL_Provider;
class IPWL_SpellCheck;

#ifdef FX_READER_DLL
	#ifdef PWL_EXPORT
			#define PWL_CLASS		__declspec(dllexport)
			#define PWL_FUNCTION	PWL_CLASS	
		#else
			#define PWL_CLASS
			#define PWL_FUNCTION
	#endif
#else
	#define PWL_CLASS
	#define PWL_FUNCTION
#endif

//window styles
#define PWS_CHILD							0x80000000L
#define PWS_BORDER							0x40000000L
#define PWS_BACKGROUND						0x20000000L
#define PWS_HSCROLL							0x10000000L
#define PWS_VSCROLL							0x08000000L
#define PWS_VISIBLE							0x04000000L
#define PWS_DISABLE							0x02000000L
#define PWS_READONLY						0x01000000L
#define PWS_AUTOFONTSIZE					0x00800000L
#define PWS_AUTOTRANSPARENT					0x00400000L
#define PWS_NOREFRESHCLIP					0x00200000L

//edit and label styles
#define PES_MULTILINE						0x0001L
#define PES_PASSWORD						0x0002L
#define PES_LEFT							0x0004L
#define PES_RIGHT							0x0008L
#define PES_MIDDLE							0x0010L
#define PES_TOP								0x0020L
#define PES_BOTTOM							0x0040L
#define PES_CENTER							0x0080L
#define PES_CHARARRAY						0x0100L
#define PES_AUTOSCROLL						0x0200L
#define PES_AUTORETURN						0x0400L
#define PES_UNDO							0x0800L
#define PES_RICH							0x1000L
#define PES_SPELLCHECK						0x2000L
#define PES_TEXTOVERFLOW					0x4000L
#define PES_NOREAD							0x8000L

//listbox styles
#define PLBS_MULTIPLESEL					0x0001L
#define PLBS_HOVERSEL						0x0008L

//combobox styles
#define PCBS_ALLOWCUSTOMTEXT				0x0001L

//richedit styles
#define PRES_MULTILINE						0x0001L
#define PRES_AUTORETURN						0x0002L
#define PRES_AUTOSCROLL						0x0004L
#define PRES_SPELLCHECK						0x0008L
#define PRES_UNDO							0x0100L
#define PRES_MULTIPAGES						0x0200L
#define PRES_TEXTOVERFLOW					0x0400L

//border style
#define PBS_SOLID							0
#define PBS_DASH							1
#define PBS_BEVELED							2
#define PBS_INSET							3
#define PBS_UNDERLINED						4
#define PBS_SHADOW							5

//notification messages
#define PNM_ADDCHILD						0x00000000L
#define PNM_REMOVECHILD						0x00000001L
#define PNM_SETSCROLLINFO					0x00000002L
#define PNM_SETSCROLLPOS					0x00000003L
#define PNM_SCROLLWINDOW					0x00000004L
#define PNM_LBUTTONDOWN						0x00000005L
#define PNM_LBUTTONUP						0x00000006L
#define PNM_MOUSEMOVE						0x00000007L
#define	PNM_NOTERESET						0x00000008L
#define PNM_SETCARETINFO					0x00000009L
#define PNM_SELCHANGED						0x0000000AL
#define	PNM_NOTEEDITCHANGED					0x0000000BL

#define PWL_CLASSNAME_EDIT					"CPWL_Edit"

struct CPWL_Dash
{
	CPWL_Dash(FX_INT32 dash, FX_INT32 gap, FX_INT32 phase) : nDash(dash), nGap(gap), nPhase(phase)
	{}

	FX_INT32			nDash;
	FX_INT32			nGap;
	FX_INT32			nPhase;
};

struct PWL_CLASS CPWL_Color
{
	CPWL_Color(FX_INT32 type = COLORTYPE_TRANSPARENT, FX_FLOAT color1 = 0.0f, FX_FLOAT color2 = 0.0f, FX_FLOAT color3 = 0.0f, FX_FLOAT color4 = 0.0f)
		: nColorType(type), fColor1(color1), fColor2(color2), fColor3(color3), fColor4(color4)
	{}

	CPWL_Color(FX_INT32 r, FX_INT32 g, FX_INT32 b) :
		nColorType(COLORTYPE_RGB), fColor1(r/255.0f), fColor2(g/255.0f), fColor3(b/255.0f), fColor4(0)
	{}
	
	void ConvertColorType(FX_INT32 nColorType);

	/*
	COLORTYPE_TRANSPARENT
	COLORTYPE_RGB
	COLORTYPE_CMYK
	COLORTYPE_GRAY
	*/
	FX_INT32					nColorType; 
	FX_FLOAT					fColor1,fColor2,fColor3,fColor4;
};

inline FX_BOOL operator == (const CPWL_Color &c1, const CPWL_Color &c2)
{
	return c1.nColorType == c2.nColorType && 
		c1.fColor1 - c2.fColor1 < 0.0001 && c1.fColor1 - c2.fColor1 > -0.0001 &&
		c1.fColor2 - c2.fColor2 < 0.0001 && c1.fColor2 - c2.fColor2 > -0.0001 &&
		c1.fColor3 - c2.fColor3 < 0.0001 && c1.fColor3 - c2.fColor3 > -0.0001 &&
		c1.fColor4 - c2.fColor4 < 0.0001 && c1.fColor4 - c2.fColor4 > -0.0001;
}

inline FX_BOOL operator != (const CPWL_Color &c1, const CPWL_Color &c2)
{
	return !operator == (c1, c2);
}

#define PWL_SCROLLBAR_WIDTH					12.0f
#define PWL_SCROLLBAR_BUTTON_WIDTH			9.0f
#define PWL_SCROLLBAR_POSBUTTON_MINWIDTH	2.0f
#define PWL_SCROLLBAR_TRANSPARANCY			150
#define PWL_SCROLLBAR_BKCOLOR				CPWL_Color(COLORTYPE_RGB,220.0f/255.0f,220.0f/255.0f,220.0f/255.0f)
#define PWL_DEFAULT_SELTEXTCOLOR			CPWL_Color(COLORTYPE_RGB,1,1,1)
#define PWL_DEFAULT_SELBACKCOLOR			CPWL_Color(COLORTYPE_RGB,0,51.0f/255.0f,113.0f/255.0f)
#define PWL_DEFAULT_BACKCOLOR				PWL_DEFAULT_SELTEXTCOLOR
#define PWL_DEFAULT_TEXTCOLOR				CPWL_Color(COLORTYPE_RGB,0,0,0)
#define PWL_DEFAULT_FONTSIZE				9.0f
#define PWL_DEFAULT_BLACKCOLOR				CPWL_Color(COLORTYPE_GRAY,0)
#define PWL_DEFAULT_WHITECOLOR				CPWL_Color(COLORTYPE_GRAY,1)
#define PWL_DEFAULT_HEAVYGRAYCOLOR			CPWL_Color(COLORTYPE_GRAY,0.50)
#define PWL_DEFAULT_LIGHTGRAYCOLOR			CPWL_Color(COLORTYPE_GRAY,0.75)
#define PWL_TRIANGLE_HALFLEN				2.0f
#define	PWL_CBBUTTON_TRIANGLE_HALFLEN		3.0f
#define PWL_INVALIDATE_INFLATE				2

class IPWL_SpellCheck
{
public:
        virtual ~IPWL_SpellCheck() { }
	virtual FX_BOOL							CheckWord(FX_LPCSTR sWord) = 0;
	virtual void							SuggestWords(FX_LPCSTR sWord, CFX_ByteStringArray & sSuggest) = 0;	
};

class IPWL_Provider
{
public:
        virtual ~IPWL_Provider() { }

	//get a matrix which map user space to CWnd client space
	virtual CPDF_Matrix						GetWindowMatrix(void* pAttachedData) = 0;

	/*
	0 L"&Undo\tCtrl+Z"
	1 L"&Redo\tCtrl+Shift+Z"
	2 L"Cu&t\tCtrl+X"
	3 L"&Copy\tCtrl+C"
	4 L"&Paste\tCtrl+V"
	5 L"&Delete"
	6  L"&Select All\tCtrl+A"
	*/
	virtual CFX_WideString					LoadPopupMenuString(FX_INT32 nIndex) = 0;
};

class IPWL_FocusHandler
{
public:
        virtual ~IPWL_FocusHandler() { }
	virtual void							OnSetFocus(CPWL_Wnd* pWnd) = 0;
	virtual void							OnKillFocus(CPWL_Wnd* pWnd) = 0;
};

struct PWL_CREATEPARAM
{
public:
	PWL_CREATEPARAM() : rcRectWnd(0,0,0,0),
		pSystemHandler(NULL),
		pFontMap(NULL),
		pProvider(NULL),
		pFocusHandler(NULL),
		dwFlags(0),
		sBackgroundColor(),
		hAttachedWnd(NULL),
		pSpellCheck(NULL),
		nBorderStyle(PBS_SOLID),
		dwBorderWidth(1),
		sBorderColor(),
		sTextColor(),
		sTextStrokeColor(),
		nTransparency(255),
		fFontSize(PWL_DEFAULT_FONTSIZE),
		sDash(3,0,0),	
		pAttachedData(NULL),
		pParentWnd(NULL),
		pMsgControl(NULL),
		eCursorType(FXCT_ARROW),
		mtChild(1,0,0,1,0,0)
	{
	}

	CPDF_Rect				rcRectWnd;				//required	
	IFX_SystemHandler*		pSystemHandler;			//required
	IFX_Edit_FontMap*		pFontMap;				//required for text window
	IPWL_Provider*			pProvider;				//required for self coordinate
	IPWL_FocusHandler*		pFocusHandler;			//optional
	FX_DWORD				dwFlags;				//optional
	CPWL_Color				sBackgroundColor;		//optional
	FX_HWND					hAttachedWnd;			//required for no-reader framework
	IPWL_SpellCheck*		pSpellCheck;			//required for spellchecking
	FX_INT32				nBorderStyle;			//optional
	FX_INT32				dwBorderWidth;			//optional
	CPWL_Color				sBorderColor;			//optional
	CPWL_Color				sTextColor;				//optional
	CPWL_Color				sTextStrokeColor;		//optional
	FX_INT32				nTransparency;			//optional
	FX_FLOAT				fFontSize;				//optional
	CPWL_Dash				sDash;					//optional
	void*					pAttachedData;			//optional
	CPWL_Wnd*				pParentWnd;				//ignore
	CPWL_MsgControl*		pMsgControl;			//ignore
	FX_INT32				eCursorType;			//ignore
	CPDF_Matrix				mtChild;				//ignore
};

class CPWL_Timer
{
public:
	CPWL_Timer(CPWL_TimerHandler* pAttached, IFX_SystemHandler* pSystemHandler);
	virtual ~CPWL_Timer();

	FX_INT32							SetPWLTimer(FX_INT32 nElapse);
	void								KillPWLTimer();
	static void 						TimerProc(FX_INT32 idEvent);

private:
	FX_INT32							m_nTimerID;	
	CPWL_TimerHandler*					m_pAttached;
	IFX_SystemHandler*					m_pSystemHandler;
};

class PWL_CLASS CPWL_TimerHandler
{
public:
	CPWL_TimerHandler();
	virtual ~CPWL_TimerHandler();

	void								BeginTimer(FX_INT32 nElapse);
	void								EndTimer();
	virtual void						TimerProc();
	virtual IFX_SystemHandler*			GetSystemHandler() const = 0;

private:
	CPWL_Timer*							m_pTimer;
};

class PWL_CLASS CPWL_Wnd : public CPWL_TimerHandler
{
	friend class CPWL_MsgControl;
public:
	CPWL_Wnd();
	virtual ~CPWL_Wnd();

	void							Create(const PWL_CREATEPARAM & cp);
	virtual CFX_ByteString			GetClassName() const;	
	void							Destroy();
	void							Move(const CPDF_Rect & rcNew,FX_BOOL bReset,FX_BOOL bRefresh);
	virtual void					InvalidateRect(CPDF_Rect* pRect = NULL);

	void							GetAppearanceStream(CFX_ByteString & sAppStream);
	void							DrawAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual FX_BOOL					OnKeyDown(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnKeyUp(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnChar(FX_WORD nChar, FX_DWORD nFlag);
	virtual FX_BOOL					OnLButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnRButtonDblClk(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnRButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnRButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMouseMove(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL					OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag);

	virtual void					SetFocus();
	virtual void					KillFocus();
	void							SetCapture();
	void							ReleaseCapture();

	virtual void					OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam = 0, FX_INTPTR lParam = 0);
	virtual void					SetTextColor(const CPWL_Color & color);
	virtual void					SetTextStrokeColor(const CPWL_Color & color);
	virtual void					SetVisible(FX_BOOL bVisible);

	virtual CPDF_Rect				GetFocusRect() const;
	virtual CPWL_Color				GetBackgroundColor() const;
	virtual CPWL_Color				GetBorderColor() const;
	virtual CPWL_Color				GetTextColor() const;
	virtual CPWL_Color				GetTextStrokeColor() const;
	virtual FX_FLOAT				GetFontSize() const;
	virtual FX_INT32				GetInnerBorderWidth() const;
	virtual CPWL_Color				GetBorderLeftTopColor(FX_INT32 nBorderStyle) const;
	virtual CPWL_Color				GetBorderRightBottomColor(FX_INT32 nBorderStyle) const;

	virtual FX_BOOL					IsModified() const {return FALSE;}

	virtual void					SetFontSize(FX_FLOAT fFontSize);	

	void							SetBackgroundColor(const CPWL_Color & color);		
	void							SetBorderColor(const CPWL_Color & color);
	void							SetBorderWidth(FX_INT32 nBorderWidth);
	void							SetClipRect(const CPDF_Rect & rect);
	void							SetBorderStyle(FX_INT32 eBorderStyle);	
	void							SetBorderDash(const CPWL_Dash & sDash);

	CPDF_Rect						GetOriginWindowRect() const;
	virtual CPDF_Rect				GetWindowRect() const;
	virtual CPDF_Rect				GetClientRect() const;
	CPDF_Point						GetCenterPoint() const;
	CPDF_Rect						GetClientCenterSquare() const;
	CPDF_Rect						GetWindowCenterSquare() const;		
	FX_INT32						GetBorderWidth() const;		
	FX_BOOL							IsVisible() const {return m_bVisible;}	
	FX_BOOL							HasFlag(FX_DWORD dwFlags) const;
	void							AddFlag(FX_DWORD dwFlags);
	void							RemoveFlag(FX_DWORD dwFlags);
	CPDF_Rect						GetClipRect() const;		
	CPWL_Wnd*						GetParentWindow() const;		
	FX_INT32						GetBorderStyle() const;	
	CPWL_Dash						GetBorderDash() const;
	void*							GetAttachedData() const;
	
	FX_BOOL							WndHitTest(const CPDF_Point & point) const;
	FX_BOOL							ClientHitTest(const CPDF_Point & point) const;
	FX_BOOL							IsCaptureMouse() const;

	const CPWL_Wnd*					GetFocused() const;
	FX_BOOL							IsFocused() const;
	FX_BOOL							IsReadOnly() const;
	CPWL_ScrollBar*					GetVScrollBar() const;

	IFX_Edit_FontMap*				GetFontMap() const;
	IPWL_Provider*					GetProvider() const;
	virtual IFX_SystemHandler*		GetSystemHandler() const;
	IPWL_FocusHandler*				GetFocusHandler() const;
	
	FX_INT32						GetTransparency();
	void							SetTransparency(FX_INT32 nTransparency);

	CPDF_Matrix						GetChildToRoot() const;
	CPDF_Matrix						GetChildMatrix() const;
	void							SetChildMatrix(const CPDF_Matrix& mt);
	CPDF_Matrix						GetWindowMatrix() const;

	virtual CPDF_Point				ChildToParent(const CPDF_Point& point) const;
	virtual CPDF_Rect				ChildToParent(const CPDF_Rect& rect) const;
	virtual CPDF_Point				ParentToChild(const CPDF_Point& point) const;
	virtual CPDF_Rect				ParentToChild(const CPDF_Rect& rect) const;

	//those methods only implemented by listctrl item
	virtual FX_FLOAT				GetItemHeight(FX_FLOAT fLimitWidth) {return 0;} 
	virtual FX_FLOAT				GetItemLeftMargin() {return 0;} 
	virtual FX_FLOAT				GetItemRightMargin() {return 0;} 

	void							EnableWindow(FX_BOOL bEnable);
	FX_BOOL							IsEnabled();
	virtual void					SetCursor();

protected:	
	virtual void					CreateChildWnd(const PWL_CREATEPARAM & cp);
	virtual void					RePosChildWnd();
	void							GetAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void					GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void					GetChildAppearanceStream(CFX_ByteTextBuf & sAppStream);

	virtual void					DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual void					DrawChildAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual void					OnCreate(PWL_CREATEPARAM & cp);
	virtual void					OnCreated();
	virtual void					OnDestroy();

	virtual void					OnSetFocus();
	virtual void					OnKillFocus();

	virtual void					OnEnabled();
	virtual void					OnDisabled();

	void							SetNotifyFlag(FX_BOOL bNotifying = TRUE){m_bNotifying = bNotifying;};
 
	FX_BOOL							IsValid() const;
	PWL_CREATEPARAM					GetCreationParam() const;		
	FX_BOOL							IsNotifying() const {return m_bNotifying;}	
	
	void							InvalidateRectMove(const CPDF_Rect & rcOld, const CPDF_Rect & rcNew);

	void							PWLtoWnd(const CPDF_Point & point, FX_INT32& x, FX_INT32& y) const;
	FX_RECT							PWLtoWnd(const CPDF_Rect & rect) const;	
	FX_HWND							GetAttachedHWnd() const;
	
	FX_BOOL							IsWndCaptureMouse(const CPWL_Wnd * pWnd) const;
	FX_BOOL							IsWndCaptureKeyboard(const CPWL_Wnd * pWnd) const;	
	const CPWL_Wnd*					GetRootWnd() const;	

	FX_BOOL							IsCTRLpressed(FX_DWORD nFlag) const;
	FX_BOOL							IsSHIFTpressed(FX_DWORD nFlag) const;
	FX_BOOL							IsALTpressed(FX_DWORD nFlag) const;
	FX_BOOL							IsINSERTpressed(FX_DWORD nFlag) const;

private:
	void							AddChild(CPWL_Wnd * pWnd);
	void							RemoveChild(CPWL_Wnd * pWnd);

	void							CreateScrollBar(const PWL_CREATEPARAM & cp);
	void							CreateVScrollBar(const PWL_CREATEPARAM & cp);	

	void							AjustStyle();	
	void							CreateMsgControl();
	void							DestroyMsgControl();
	
	CPWL_MsgControl*				GetMsgControl() const;	
	
protected:
	CFX_ArrayTemplate<CPWL_Wnd*>	m_aChildren;

private:
	PWL_CREATEPARAM					m_sPrivateParam;
	
	CPWL_ScrollBar*					m_pVScrollBar;

	CPDF_Rect						m_rcWindow;
	CPDF_Rect						m_rcClip;

	FX_BOOL							m_bCreated;			
	FX_BOOL							m_bVisible;
	FX_BOOL							m_bNotifying;	
	FX_BOOL							m_bEnabled;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_WND_H_
