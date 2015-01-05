// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/pdfwindow/PDFWindow.h"
#include "../../include/pdfwindow/PWL_Wnd.h"
#include "../../include/pdfwindow/PWL_Utils.h"
#include "../../include/pdfwindow/PWL_ScrollBar.h"

/* -------------------------- CPWL_Timer -------------------------- */

static CFX_MapPtrTemplate<FX_INT32, CPWL_Timer*>& GetPWLTimeMap()
{
  // Leak the object at shutdown.
  static auto timeMap = new CFX_MapPtrTemplate<FX_INT32, CPWL_Timer*>;
  return *timeMap;
}

CPWL_Timer::CPWL_Timer(CPWL_TimerHandler* pAttached, IFX_SystemHandler* pSystemHandler) : 
	m_nTimerID(0),
	m_pAttached(pAttached),
	m_pSystemHandler(pSystemHandler)
{
	ASSERT(m_pAttached != NULL);
	ASSERT(m_pSystemHandler != NULL);
}

CPWL_Timer::~CPWL_Timer()
{
	KillPWLTimer();
}

FX_INT32 CPWL_Timer::SetPWLTimer(FX_INT32 nElapse)
{	
	if (m_nTimerID != 0) KillPWLTimer();
	m_nTimerID = m_pSystemHandler->SetTimer(nElapse, TimerProc);
	GetPWLTimeMap().SetAt(m_nTimerID, this);
	return m_nTimerID;
}

void CPWL_Timer::KillPWLTimer()
{
	if (m_nTimerID != 0)
	{
		m_pSystemHandler->KillTimer(m_nTimerID);
		GetPWLTimeMap().RemoveKey(m_nTimerID);
		m_nTimerID = 0;
	}
}

void CPWL_Timer::TimerProc(FX_INT32 idEvent)
{
	CPWL_Timer* pTimer = NULL;
	if (GetPWLTimeMap().Lookup(idEvent, pTimer))
	{
		if (pTimer)
		{			
			if (pTimer->m_pAttached) 
				pTimer->m_pAttached->TimerProc();
		}
	}
}

/* -------------------------- CPWL_TimerHandler -------------------------- */

CPWL_TimerHandler::CPWL_TimerHandler() : m_pTimer(NULL)
{
}

CPWL_TimerHandler::~CPWL_TimerHandler()
{
	if (m_pTimer) delete m_pTimer;
}

void CPWL_TimerHandler::BeginTimer(FX_INT32 nElapse)
{
	if (!m_pTimer)
		m_pTimer = new CPWL_Timer(this, GetSystemHandler());

	if (m_pTimer)
		m_pTimer->SetPWLTimer(nElapse);
}

void CPWL_TimerHandler::EndTimer()
{
	if (m_pTimer)
		m_pTimer->KillPWLTimer();
}

void CPWL_TimerHandler::TimerProc()
{
}

/* --------------------------- CPWL_MsgControl ---------------------------- */

class CPWL_MsgControl
{
	friend class CPWL_Wnd;

public:
	CPWL_MsgControl(CPWL_Wnd * pWnd)
	{
//		PWL_TRACE("new CPWL_MsgControl\n");
		m_pCreatedWnd = pWnd;
		Default();
	}

	~CPWL_MsgControl()
	{
//		PWL_TRACE("~CPWL_MsgControl\n");
		Default();
	}

	void Default()
	{
		m_aMousePath.RemoveAll();
		m_aKeyboardPath.RemoveAll();
		m_pMainMouseWnd = NULL;
		m_pMainKeyboardWnd = NULL;
	}

	FX_BOOL IsWndCreated(const CPWL_Wnd * pWnd) const
	{
		return m_pCreatedWnd == pWnd;
	}

	FX_BOOL IsMainCaptureMouse(const CPWL_Wnd * pWnd) const
	{
		return pWnd == m_pMainMouseWnd;
	}

	FX_BOOL IsWndCaptureMouse(const CPWL_Wnd * pWnd) const
	{
		if (pWnd)
			for( FX_INT32 i=0,sz=m_aMousePath.GetSize(); i<sz; i++)
				if (m_aMousePath.GetAt(i) == pWnd)
					return TRUE;

		return FALSE;
	}

	FX_BOOL IsMainCaptureKeyboard(const CPWL_Wnd * pWnd) const
	{
		return pWnd == m_pMainKeyboardWnd;
	}


	FX_BOOL IsWndCaptureKeyboard(const CPWL_Wnd * pWnd) const
	{
		if (pWnd)
			for( FX_INT32 i=0,sz=m_aKeyboardPath.GetSize(); i<sz; i++)
				if (m_aKeyboardPath.GetAt(i) == pWnd)
					return TRUE;
		
		return FALSE;
	}

	void SetFocus(CPWL_Wnd * pWnd)
	{
		m_aKeyboardPath.RemoveAll();

		if (pWnd)
		{
			m_pMainKeyboardWnd = pWnd;

			CPWL_Wnd * pParent = pWnd;
			while (pParent)
			{
				m_aKeyboardPath.Add(pParent);
				pParent = pParent->GetParentWindow();
			}

			pWnd->OnSetFocus();
		}
	}

	void KillFocus()
	{
		if (m_aKeyboardPath.GetSize() > 0)
			if (CPWL_Wnd* pWnd = m_aKeyboardPath.GetAt(0))
				pWnd->OnKillFocus();

		m_pMainKeyboardWnd = NULL;
		m_aKeyboardPath.RemoveAll();
	}

	void SetCapture(CPWL_Wnd * pWnd)
	{
		m_aMousePath.RemoveAll();

		if (pWnd)
		{
			m_pMainMouseWnd = pWnd;

			CPWL_Wnd * pParent = pWnd;
			while (pParent)
			{
				m_aMousePath.Add(pParent);
				pParent = pParent->GetParentWindow();
			}
		}
	}

	void ReleaseCapture()
	{
		m_pMainMouseWnd = NULL;
		m_aMousePath.RemoveAll();
	}

private:
	CFX_ArrayTemplate<CPWL_Wnd*>	m_aMousePath;
	CFX_ArrayTemplate<CPWL_Wnd*>	m_aKeyboardPath;
	CPWL_Wnd*						m_pCreatedWnd;
	CPWL_Wnd*						m_pMainMouseWnd;
	CPWL_Wnd*						m_pMainKeyboardWnd;
};

/* --------------------------- CPWL_Wnd ---------------------------- */

CPWL_Wnd::CPWL_Wnd() :
	m_pVScrollBar(NULL),
	m_rcWindow(),
	m_rcClip(),
	m_bCreated(FALSE),			
	m_bVisible(FALSE),
	m_bNotifying(FALSE),
	m_bEnabled(TRUE)
{
}

CPWL_Wnd::~CPWL_Wnd()
{
	ASSERT(m_bCreated == FALSE);
}

CFX_ByteString CPWL_Wnd::GetClassName() const
{
	return "CPWL_Wnd";
}

void CPWL_Wnd::Create(const PWL_CREATEPARAM & cp)
{
	if (!IsValid())
	{
		m_sPrivateParam = cp;

		OnCreate(m_sPrivateParam);

		m_sPrivateParam.rcRectWnd.Normalize();
		m_rcWindow = m_sPrivateParam.rcRectWnd;
		m_rcClip = CPWL_Utils::InflateRect(m_rcWindow,1.0f);

		CreateMsgControl();

		if (m_sPrivateParam.pParentWnd)
			m_sPrivateParam.pParentWnd->OnNotify(this, PNM_ADDCHILD);

		PWL_CREATEPARAM ccp = m_sPrivateParam;

		ccp.dwFlags &= 0xFFFF0000L; //remove sub styles
		ccp.mtChild = CPDF_Matrix(1,0,0,1,0,0);
		
		CreateScrollBar(ccp);
		CreateChildWnd(ccp); 

		m_bVisible = HasFlag(PWS_VISIBLE);

		OnCreated();

		RePosChildWnd();
		m_bCreated = TRUE;
	}
}

void CPWL_Wnd::OnCreate(PWL_CREATEPARAM & cp)
{
}

void CPWL_Wnd::OnCreated()
{
}

void CPWL_Wnd::OnDestroy()
{
}

void CPWL_Wnd::Destroy()
{
	KillFocus();

	OnDestroy();

	if (m_bCreated)
	{
		for (FX_INT32 i = m_aChildren.GetSize()-1; i >= 0; i --)
		{
			if (CPWL_Wnd * pChild = m_aChildren[i])
			{
				pChild->Destroy();
				delete pChild;
				pChild = NULL;
			}
		}

		if (m_sPrivateParam.pParentWnd)
			m_sPrivateParam.pParentWnd->OnNotify(this, PNM_REMOVECHILD);
		m_bCreated = FALSE;
	}

	DestroyMsgControl();

	FXSYS_memset(&m_sPrivateParam, 0, sizeof(PWL_CREATEPARAM));
	m_aChildren.RemoveAll();	
	m_pVScrollBar = NULL;
}

void CPWL_Wnd::Move(const CPDF_Rect & rcNew, FX_BOOL bReset,FX_BOOL bRefresh)
{
	if (IsValid())
	{
		CPDF_Rect rcOld = this->GetWindowRect();

		m_rcWindow = rcNew;
		m_rcWindow.Normalize();
		//m_rcClip = CPWL_Utils::InflateRect(m_rcWindow,1.0f); //for special caret 

		if (rcOld.left != rcNew.left || rcOld.right != rcNew.right ||
			rcOld.top != rcNew.top || rcOld.bottom != rcNew.bottom)
		{
			if (bReset)
			{
				RePosChildWnd();	
			}	

		}
		if (bRefresh)
		{	
			InvalidateRectMove(rcOld,rcNew);
		}

		m_sPrivateParam.rcRectWnd = m_rcWindow;
	}
}

void  CPWL_Wnd::InvalidateRectMove(const CPDF_Rect & rcOld, const CPDF_Rect & rcNew)
{
	CPDF_Rect rcUnion = rcOld;
	rcUnion.Union(rcNew);

	InvalidateRect(&rcUnion);

	/*
	CPDF_Rect SubArray[4]; 

	rcOld.Substract4(rcNew,SubArray);
	for (FX_INT32 i=0;i<4;i++)
	{
		if (SubArray[i].left == 0 &&
			SubArray[i].right == 0 &&
			SubArray[i].top == 0 &&
			SubArray[i].bottom == 0)continue;

		InvalidateRect(&CPWL_Utils::InflateRect(SubArray[i],2));
	}

	rcNew.Substract4(rcOld,SubArray);
	for (FX_INT32 j=0;j<4;j++)
	{
		if (SubArray[j].left == 0 &&
			SubArray[j].right == 0 &&
			SubArray[j].top == 0 &&
			SubArray[j].bottom == 0)continue;

		InvalidateRect(&CPWL_Utils::InflateRect(SubArray[j],2));
	}
	*/
}

void CPWL_Wnd::GetAppearanceStream(CFX_ByteString & sAppStream)
{
	if (IsValid())
	{
		CFX_ByteTextBuf sTextBuf;
		GetAppearanceStream(sTextBuf);
		sAppStream += sTextBuf.GetByteString();
	}
}

void CPWL_Wnd::GetAppearanceStream(CFX_ByteTextBuf & sAppStream)
{
	if (IsValid() && IsVisible())
	{
		GetThisAppearanceStream(sAppStream);
		GetChildAppearanceStream(sAppStream);
	}
}

//if don't set,Get default apperance stream
void CPWL_Wnd::GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream)
{
	CPDF_Rect rectWnd = GetWindowRect();
	if (!rectWnd.IsEmpty())
	{
		CFX_ByteTextBuf sThis;

		if (HasFlag(PWS_BACKGROUND))
			sThis << CPWL_Utils::GetRectFillAppStream(rectWnd,this->GetBackgroundColor());

		if (HasFlag(PWS_BORDER))
			sThis << CPWL_Utils::GetBorderAppStream(rectWnd,
									(FX_FLOAT)GetBorderWidth(),
									GetBorderColor(),
									this->GetBorderLeftTopColor(this->GetBorderStyle()),
									this->GetBorderRightBottomColor(this->GetBorderStyle()),
									this->GetBorderStyle(),
									this->GetBorderDash());

		sAppStream << sThis;
	}
}

void CPWL_Wnd::GetChildAppearanceStream(CFX_ByteTextBuf & sAppStream)
{
	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))
		{
			pChild->GetAppearanceStream(sAppStream);
		}
	}
}

void CPWL_Wnd::DrawAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	if (IsValid() && IsVisible())
	{
		DrawThisAppearance(pDevice,pUser2Device);
		DrawChildAppearance(pDevice,pUser2Device);
	}
}

void CPWL_Wnd::DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	CPDF_Rect rectWnd = GetWindowRect();
	if (!rectWnd.IsEmpty())
	{		
		if (HasFlag(PWS_BACKGROUND))
		{
			CPDF_Rect rcClient = CPWL_Utils::DeflateRect(rectWnd,(FX_FLOAT)(GetBorderWidth()+GetInnerBorderWidth()));
			CPWL_Utils::DrawFillRect(pDevice,pUser2Device,rcClient,this->GetBackgroundColor(),GetTransparency());
		}

		if (HasFlag(PWS_BORDER))
			CPWL_Utils::DrawBorder(pDevice,
								pUser2Device,
								rectWnd,
								(FX_FLOAT)GetBorderWidth(),
								GetBorderColor(),
								this->GetBorderLeftTopColor(this->GetBorderStyle()),
								this->GetBorderRightBottomColor(this->GetBorderStyle()),
								this->GetBorderStyle(),
								this->GetBorderDash(),
								GetTransparency());								
	}
}

void CPWL_Wnd::DrawChildAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device)
{
	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))
		{
			CPDF_Matrix mt = pChild->GetChildMatrix();
			if (mt.IsIdentity())
			{
				pChild->DrawAppearance(pDevice,pUser2Device);
			}
			else
			{
				mt.Concat(*pUser2Device);
				pChild->DrawAppearance(pDevice,&mt);
			}
		}
	}
}

void CPWL_Wnd::InvalidateRect(CPDF_Rect* pRect)
{
	if (IsValid())
	{
		CPDF_Rect rcRefresh = pRect ? *pRect : GetWindowRect();

		if (!HasFlag(PWS_NOREFRESHCLIP))
		{
			CPDF_Rect rcClip = GetClipRect();		
			if (!rcClip.IsEmpty())
			{
				rcRefresh.Intersect(rcClip);
			}
		}

		FX_RECT rcWin = PWLtoWnd(rcRefresh);
		rcWin.left -= PWL_INVALIDATE_INFLATE;
		rcWin.top -= PWL_INVALIDATE_INFLATE;
		rcWin.right += PWL_INVALIDATE_INFLATE;
		rcWin.bottom += PWL_INVALIDATE_INFLATE;

		if (IFX_SystemHandler* pSH = GetSystemHandler())
		{
			if (FX_HWND hWnd = GetAttachedHWnd())
			{
				pSH->InvalidateRect(hWnd, rcWin);
			}
		}
	}
}

#define PWL_IMPLEMENT_KEY_METHOD(key_method_name)\
FX_BOOL CPWL_Wnd::key_method_name(FX_WORD nChar, FX_DWORD nFlag)\
{\
	if (IsValid() && IsVisible() && IsEnabled())\
	{\
		if (IsWndCaptureKeyboard(this))\
		{\
			for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)\
			{\
				if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))\
				{\
					if (IsWndCaptureKeyboard(pChild))\
					{\
						return pChild->key_method_name(nChar,nFlag);\
					}\
				}\
			}\
		}\
	}\
	return FALSE;\
}

#define PWL_IMPLEMENT_MOUSE_METHOD(mouse_method_name)\
FX_BOOL CPWL_Wnd::mouse_method_name(const CPDF_Point & point, FX_DWORD nFlag)\
{\
	if (IsValid() && IsVisible() && IsEnabled())\
	{\
		if (IsWndCaptureMouse(this))\
		{\
			for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)\
			{\
				if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))\
				{\
					if (IsWndCaptureMouse(pChild))\
					{\
						return pChild->mouse_method_name(pChild->ParentToChild(point),nFlag);\
					}\
				}\
			}\
			SetCursor();\
		}\
		else\
		{\
			for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)\
			{\
				if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))\
				{\
					if (pChild->WndHitTest(pChild->ParentToChild(point)))\
					{\
						return pChild->mouse_method_name(pChild->ParentToChild(point),nFlag);\
					}\
				}\
			}\
			if (this->WndHitTest(point))\
				SetCursor();\
		}\
	}\
	return FALSE;\
}

PWL_IMPLEMENT_KEY_METHOD(OnKeyDown)
PWL_IMPLEMENT_KEY_METHOD(OnKeyUp)
PWL_IMPLEMENT_KEY_METHOD(OnChar)

PWL_IMPLEMENT_MOUSE_METHOD(OnLButtonDblClk)
PWL_IMPLEMENT_MOUSE_METHOD(OnLButtonDown)
PWL_IMPLEMENT_MOUSE_METHOD(OnLButtonUp)
PWL_IMPLEMENT_MOUSE_METHOD(OnMButtonDblClk)
PWL_IMPLEMENT_MOUSE_METHOD(OnMButtonDown)
PWL_IMPLEMENT_MOUSE_METHOD(OnMButtonUp)
PWL_IMPLEMENT_MOUSE_METHOD(OnRButtonDblClk)
PWL_IMPLEMENT_MOUSE_METHOD(OnRButtonDown)
PWL_IMPLEMENT_MOUSE_METHOD(OnRButtonUp)
PWL_IMPLEMENT_MOUSE_METHOD(OnMouseMove)

FX_BOOL	CPWL_Wnd::OnMouseWheel(short zDelta, const CPDF_Point & point, FX_DWORD nFlag)
{
	if (IsValid() && IsVisible() && IsEnabled())
	{
		SetCursor();
		if (IsWndCaptureKeyboard(this))
		{
			for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
			{
				if (CPWL_Wnd * pChild = m_aChildren.GetAt(i))
				{
					if (IsWndCaptureKeyboard(pChild))
					{
						return pChild->OnMouseWheel(zDelta,pChild->ParentToChild(point), nFlag);
					}
				}
			}
		}
	}
	return FALSE;
}

void CPWL_Wnd::AddChild(CPWL_Wnd * pWnd)
{
	m_aChildren.Add(pWnd);
}

void CPWL_Wnd::RemoveChild(CPWL_Wnd * pWnd)
{
	for (FX_INT32 i = m_aChildren.GetSize()-1; i >= 0; i --)
	{
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
		{
			if (pChild == pWnd)
			{
				m_aChildren.RemoveAt(i);
				break;
			}
		}
	}
}

void CPWL_Wnd::OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam, FX_INTPTR lParam)
{
	switch (msg)
	{
	case PNM_ADDCHILD:
		this->AddChild(pWnd);
		break;
	case PNM_REMOVECHILD:
		this->RemoveChild(pWnd);
		break;
	default:
		break;
	}
}

FX_BOOL CPWL_Wnd::IsValid() const
{
	return m_bCreated;
}

PWL_CREATEPARAM CPWL_Wnd::GetCreationParam() const
{
	return m_sPrivateParam;
}

CPWL_Wnd* CPWL_Wnd::GetParentWindow() const
{
	return m_sPrivateParam.pParentWnd;
}

CPDF_Rect CPWL_Wnd::GetOriginWindowRect() const
{
	return m_sPrivateParam.rcRectWnd;
}

CPDF_Rect CPWL_Wnd::GetWindowRect() const
{
	return m_rcWindow;
}

CPDF_Rect CPWL_Wnd::GetClientRect() const
{
	CPDF_Rect rcWindow = GetWindowRect();
	CPDF_Rect rcClient = CPWL_Utils::DeflateRect(rcWindow,(FX_FLOAT)(GetBorderWidth()+GetInnerBorderWidth()));

	if (CPWL_ScrollBar * pVSB = this->GetVScrollBar())
		rcClient.right -= pVSB->GetScrollBarWidth();

	rcClient.Normalize();

	if (rcWindow.Contains(rcClient))
		return rcClient;
	else
		return CPDF_Rect();
}

CPDF_Point CPWL_Wnd::GetCenterPoint() const
{
	CPDF_Rect rcClient = GetClientRect();

	return CPDF_Point((rcClient.left + rcClient.right) * 0.5f,
		(rcClient.top + rcClient.bottom) * 0.5f);
}

CPDF_Rect CPWL_Wnd::GetClientCenterSquare() const
{
	return CPWL_Utils::GetCenterSquare(GetClientRect());
}

CPDF_Rect CPWL_Wnd::GetWindowCenterSquare() const
{
	return CPWL_Utils::GetCenterSquare(CPWL_Utils::DeflateRect(GetWindowRect(),0.1f));
}

FX_BOOL CPWL_Wnd::HasFlag(FX_DWORD dwFlags) const
{
	return (m_sPrivateParam.dwFlags & dwFlags) != 0;
}

void CPWL_Wnd::RemoveFlag(FX_DWORD dwFlags)
{
	m_sPrivateParam.dwFlags &= ~dwFlags;
}

void CPWL_Wnd::AddFlag(FX_DWORD dwFlags)
{
	m_sPrivateParam.dwFlags |= dwFlags;
}

CPWL_Color CPWL_Wnd::GetBackgroundColor() const
{
	return m_sPrivateParam.sBackgroundColor;
}

void CPWL_Wnd::SetBackgroundColor(const CPWL_Color & color)
{
	m_sPrivateParam.sBackgroundColor = color;
}

void CPWL_Wnd::SetTextColor(const CPWL_Color & color)
{
	m_sPrivateParam.sTextColor = color;
}

void CPWL_Wnd::SetTextStrokeColor(const CPWL_Color & color)
{
	m_sPrivateParam.sTextStrokeColor = color;
}

CPWL_Color CPWL_Wnd::GetTextColor() const
{
	return m_sPrivateParam.sTextColor;
}

CPWL_Color CPWL_Wnd::GetTextStrokeColor() const
{
	return m_sPrivateParam.sTextStrokeColor;
}

FX_INT32 CPWL_Wnd::GetBorderStyle() const
{
	return m_sPrivateParam.nBorderStyle;
}

void CPWL_Wnd::SetBorderStyle(FX_INT32 nBorderStyle)
{
	if (HasFlag(PWS_BORDER))
		m_sPrivateParam.nBorderStyle = nBorderStyle;
}

FX_INT32 CPWL_Wnd::GetBorderWidth() const
{
	if (HasFlag(PWS_BORDER))
		return m_sPrivateParam.dwBorderWidth;

	return 0;
}

FX_INT32 CPWL_Wnd::GetInnerBorderWidth() const
{
	/*
	switch (GetBorderStyle())
	{
	case PBS_BEVELED:
	case PBS_INSET:
		return GetBorderWidth() / 2;
	}
	*/
	return 0;
}

void CPWL_Wnd::SetBorderWidth(FX_INT32 nBorderWidth)
{
	if (HasFlag(PWS_BORDER))
		m_sPrivateParam.dwBorderWidth = nBorderWidth;
}

CPWL_Color CPWL_Wnd::GetBorderColor() const
{
	if (HasFlag(PWS_BORDER))
		return m_sPrivateParam.sBorderColor;

	return CPWL_Color();
}

void CPWL_Wnd::SetBorderColor(const CPWL_Color & color)
{
	if (HasFlag(PWS_BORDER))
		m_sPrivateParam.sBorderColor = color;
}

CPWL_Dash CPWL_Wnd::GetBorderDash() const
{
	return m_sPrivateParam.sDash;
}

void* CPWL_Wnd::GetAttachedData() const
{
	return m_sPrivateParam.pAttachedData;
}

void CPWL_Wnd::SetBorderDash(const CPWL_Dash & sDash)
{
	if (HasFlag(PWS_BORDER))
		m_sPrivateParam.sDash = sDash;
}

CPWL_ScrollBar* CPWL_Wnd::GetVScrollBar() const
{
	if (HasFlag(PWS_VSCROLL))
		return m_pVScrollBar;

	return NULL;
}

void CPWL_Wnd::CreateScrollBar(const PWL_CREATEPARAM & cp)
{
	CreateVScrollBar(cp);
}

void CPWL_Wnd::CreateVScrollBar(const PWL_CREATEPARAM & cp)
{
	if (!m_pVScrollBar && HasFlag(PWS_VSCROLL))
	{
		PWL_CREATEPARAM scp = cp;

		//flags
		scp.dwFlags = PWS_CHILD| PWS_BACKGROUND | PWS_AUTOTRANSPARENT | PWS_NOREFRESHCLIP;
		
		scp.pParentWnd = this;
		scp.sBackgroundColor = PWL_DEFAULT_WHITECOLOR;
		scp.eCursorType = FXCT_ARROW;
		scp.nTransparency = PWL_SCROLLBAR_TRANSPARANCY;

		if ((m_pVScrollBar = new CPWL_ScrollBar(SBT_VSCROLL)))
			m_pVScrollBar->Create(scp);
	}
}

void CPWL_Wnd::SetCapture()
{
	if (CPWL_MsgControl * pMsgCtrl = GetMsgControl())
		pMsgCtrl->SetCapture(this);
}

void CPWL_Wnd::ReleaseCapture()
{
	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
			pChild->ReleaseCapture();

	if (CPWL_MsgControl * pMsgCtrl = GetMsgControl())
		pMsgCtrl->ReleaseCapture();
}

void CPWL_Wnd::SetFocus()
{
	if (CPWL_MsgControl * pMsgCtrl = GetMsgControl())
	{
		if (!pMsgCtrl->IsMainCaptureKeyboard(this))
			pMsgCtrl->KillFocus();
		pMsgCtrl->SetFocus(this);
	}
}

void CPWL_Wnd::KillFocus()
{
	if (CPWL_MsgControl * pMsgCtrl = GetMsgControl())
	{
		if (pMsgCtrl->IsWndCaptureKeyboard(this))
			pMsgCtrl->KillFocus();
	}
}

void CPWL_Wnd::OnSetFocus()
{
}

void CPWL_Wnd::OnKillFocus()
{
}

FX_BOOL	CPWL_Wnd::WndHitTest(const CPDF_Point & point) const
{
	return IsValid() && IsVisible() && GetWindowRect().Contains(point.x,point.y);
}

FX_BOOL CPWL_Wnd::ClientHitTest(const CPDF_Point & point) const
{
	return IsValid() && IsVisible() && GetClientRect().Contains(point.x,point.y);
}

const CPWL_Wnd * CPWL_Wnd::GetRootWnd() const
{
	if (m_sPrivateParam.pParentWnd)
		return m_sPrivateParam.pParentWnd->GetRootWnd();
	else
		return this;
}

void CPWL_Wnd::SetVisible(FX_BOOL bVisible)
{
	if (IsValid())
	{
		for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
		{
			if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
			{
				pChild->SetVisible(bVisible);
			}
		}

		if (bVisible != m_bVisible)
		{
			m_bVisible = bVisible;
			RePosChildWnd();		
			InvalidateRect();
		}	
	}
}

void CPWL_Wnd::SetClipRect(const CPDF_Rect & rect)
{
	m_rcClip = rect;
	m_rcClip.Normalize();
}

CPDF_Rect CPWL_Wnd::GetClipRect() const
{
	return m_rcClip;
}

FX_BOOL	CPWL_Wnd::IsReadOnly() const
{
	return HasFlag(PWS_READONLY);
}

void CPWL_Wnd::RePosChildWnd()
{
	CPDF_Rect rcContent = CPWL_Utils::DeflateRect(GetWindowRect(),(FX_FLOAT)(GetBorderWidth()+GetInnerBorderWidth()));

	CPWL_ScrollBar * pVSB = this->GetVScrollBar();

	CPDF_Rect rcVScroll = CPDF_Rect(rcContent.right - PWL_SCROLLBAR_WIDTH,
							rcContent.bottom,
							rcContent.right-1.0f,
							rcContent.top);

	if (pVSB) pVSB->Move(rcVScroll,TRUE,FALSE);
}

void CPWL_Wnd::CreateChildWnd(const PWL_CREATEPARAM & cp)
{
}

void CPWL_Wnd::SetCursor()
{
	if (IsValid()) 
	{
		if (IFX_SystemHandler* pSH = GetSystemHandler())
		{
			FX_INT32 nCursorType = this->GetCreationParam().eCursorType;
			pSH->SetCursor(nCursorType);
		}
	}
}

void CPWL_Wnd::CreateMsgControl()
{
	if (!m_sPrivateParam.pMsgControl)
		m_sPrivateParam.pMsgControl = new CPWL_MsgControl(this);
}

void CPWL_Wnd::DestroyMsgControl()
{
	if (CPWL_MsgControl* pMsgControl = GetMsgControl())
		if (pMsgControl->IsWndCreated(this))
			delete pMsgControl;
}

CPWL_MsgControl* CPWL_Wnd::GetMsgControl() const
{
	return m_sPrivateParam.pMsgControl;
}

FX_BOOL CPWL_Wnd::IsCaptureMouse() const
{
	return IsWndCaptureMouse(this);
}

FX_BOOL CPWL_Wnd::IsWndCaptureMouse(const CPWL_Wnd * pWnd) const
{
	if (CPWL_MsgControl * pCtrl = GetMsgControl())
		return pCtrl->IsWndCaptureMouse(pWnd);

	return FALSE;
}

FX_BOOL CPWL_Wnd::IsWndCaptureKeyboard(const CPWL_Wnd * pWnd) const
{
	if (CPWL_MsgControl * pCtrl = GetMsgControl())
		return pCtrl->IsWndCaptureKeyboard(pWnd);

	return FALSE;
}

FX_BOOL CPWL_Wnd::IsFocused() const
{
	if (CPWL_MsgControl * pCtrl = GetMsgControl())
		return pCtrl->IsMainCaptureKeyboard(this);

	return FALSE;
}

CPDF_Rect CPWL_Wnd::GetFocusRect() const
{
	return CPWL_Utils::InflateRect(this->GetWindowRect(),1);
}

FX_FLOAT CPWL_Wnd::GetFontSize() const
{
	return this->m_sPrivateParam.fFontSize;
}

void CPWL_Wnd::SetFontSize(FX_FLOAT fFontSize)
{
	this->m_sPrivateParam.fFontSize = fFontSize;
}

IFX_SystemHandler* CPWL_Wnd::GetSystemHandler() const
{
	return m_sPrivateParam.pSystemHandler;
}

IPWL_FocusHandler* CPWL_Wnd::GetFocusHandler() const
{
	return m_sPrivateParam.pFocusHandler;
}

IPWL_Provider* CPWL_Wnd::GetProvider() const
{
	return m_sPrivateParam.pProvider;
}

IFX_Edit_FontMap* CPWL_Wnd::GetFontMap() const
{
	return m_sPrivateParam.pFontMap;
}

CPWL_Color CPWL_Wnd::GetBorderLeftTopColor(FX_INT32 nBorderStyle) const
{
	CPWL_Color color;

	switch (nBorderStyle)
	{
		case PBS_SOLID:
			break;
		case PBS_DASH:
			break;
		case PBS_BEVELED:
			color = CPWL_Color(COLORTYPE_GRAY,1);
			break;
		case PBS_INSET:
			color = CPWL_Color(COLORTYPE_GRAY,0.5f);
			break;
		case PBS_UNDERLINED:
			break;
	}

	return color;
}

CPWL_Color CPWL_Wnd::GetBorderRightBottomColor(FX_INT32 nBorderStyle) const
{
	CPWL_Color color;

	switch (nBorderStyle)
	{
		case PBS_SOLID:
			break;
		case PBS_DASH:
			break;
		case PBS_BEVELED:
			color = CPWL_Utils::DevideColor(GetBackgroundColor(),2);
			break;
		case PBS_INSET:
			color = CPWL_Color(COLORTYPE_GRAY,0.75f);
			break;
		case PBS_UNDERLINED:
			break;
	}

	return color;
}

/* ----------------------------------------------------------------- */

FX_INT32 CPWL_Wnd::GetTransparency()
{
	return m_sPrivateParam.nTransparency;
}

void CPWL_Wnd::SetTransparency(FX_INT32 nTransparency)
{
	for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
	{
		if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
		{
			pChild->SetTransparency(nTransparency);
		}
	}

	m_sPrivateParam.nTransparency = nTransparency;
}

CPDF_Matrix	CPWL_Wnd::GetWindowMatrix() const
{
	CPDF_Matrix mt = this->GetChildToRoot();

	if (IPWL_Provider* pProvider = GetProvider())
	{
		mt.Concat(pProvider->GetWindowMatrix(GetAttachedData()));
		return mt;
	}

/*
	if (CReader_App* pApp = CPWL_Module::GetReaderApp())
		if (CReader_Document* pDocument = pApp->GetCurrentDocument())
			if (CReader_DocView* pDocView = pDocument->GetCurrentDocView())
			{
				CPDF_Matrix mtPageView;
				pDocView->GetCurrentMatrix(mtPageView);
				mt.Concat(mtPageView);
				return mt;
			}		
			
*/

	return mt;
}

void CPWL_Wnd::PWLtoWnd(const CPDF_Point& point, FX_INT32& x, FX_INT32& y) const
{
	CPDF_Matrix mt = GetWindowMatrix();
	CPDF_Point pt = point;
	mt.Transform(pt.x,pt.y);
	x = (FX_INT32)(pt.x+0.5);
	y = (FX_INT32)(pt.y+0.5);
}

FX_RECT CPWL_Wnd::PWLtoWnd(const CPDF_Rect & rect) const
{
	CPDF_Rect rcTemp = rect;
	CPDF_Matrix mt = GetWindowMatrix();
	mt.TransformRect(rcTemp);	
	return FX_RECT((FX_INT32)(rcTemp.left+0.5), (FX_INT32)(rcTemp.bottom+0.5), (FX_INT32)(rcTemp.right+0.5), (FX_INT32)(rcTemp.top+0.5));
}

FX_HWND CPWL_Wnd::GetAttachedHWnd() const
{
	return m_sPrivateParam.hAttachedWnd;
}

CPDF_Point CPWL_Wnd::ChildToParent(const CPDF_Point& point) const
{
	CPDF_Matrix mt = GetChildMatrix();
	if (mt.IsIdentity())
		return point;
	else
	{
		CPDF_Point pt = point;
		mt.Transform(pt.x,pt.y);
		return pt;
	}
}

CPDF_Rect CPWL_Wnd::ChildToParent(const CPDF_Rect& rect) const
{
	CPDF_Matrix mt = GetChildMatrix();
	if (mt.IsIdentity())
		return rect;
	else
	{
		CPDF_Rect rc = rect;
		mt.TransformRect(rc);
		return rc;
	}
}

CPDF_Point CPWL_Wnd::ParentToChild(const CPDF_Point& point) const
{
	CPDF_Matrix mt = GetChildMatrix();
	if (mt.IsIdentity())
		return point;
	else
	{
		mt.SetReverse(mt);
		CPDF_Point pt = point;
		mt.Transform(pt.x,pt.y);
		return pt;
	}
}

CPDF_Rect CPWL_Wnd::ParentToChild(const CPDF_Rect& rect) const
{
	CPDF_Matrix mt = GetChildMatrix();
	if (mt.IsIdentity())
		return rect;
	else
	{
		mt.SetReverse(mt);
		CPDF_Rect rc = rect;
		mt.TransformRect(rc);
		return rc;
	}
}

CPDF_Matrix CPWL_Wnd::GetChildToRoot() const
{
	CPDF_Matrix mt(1,0,0,1,0,0);
	
	if (HasFlag(PWS_CHILD))
	{
		const CPWL_Wnd* pParent = this;
		while (pParent)
		{
			mt.Concat(pParent->GetChildMatrix());
			pParent = pParent->GetParentWindow();
		}
	}

	return mt;
}

CPDF_Matrix CPWL_Wnd::GetChildMatrix() const
{
	if (HasFlag(PWS_CHILD))
		return m_sPrivateParam.mtChild;

	return CPDF_Matrix(1,0,0,1,0,0);
}

void CPWL_Wnd::SetChildMatrix(const CPDF_Matrix& mt)
{
	m_sPrivateParam.mtChild = mt;
}

const CPWL_Wnd*	CPWL_Wnd::GetFocused() const
{
	if (CPWL_MsgControl * pMsgCtrl = GetMsgControl())
	{
		return pMsgCtrl->m_pMainKeyboardWnd;
	}

	return NULL;
}

void CPWL_Wnd::EnableWindow(FX_BOOL bEnable)
{
	if (m_bEnabled != bEnable)
	{
		for (FX_INT32 i=0,sz=m_aChildren.GetSize(); i<sz; i++)
		{
			if (CPWL_Wnd* pChild = m_aChildren.GetAt(i))
			{
				pChild->EnableWindow(bEnable);
			}
		}

		this->m_bEnabled = bEnable;

		if (bEnable)
			this->OnEnabled();
		else
			this->OnDisabled();
	}
}

FX_BOOL CPWL_Wnd::IsEnabled()
{
	return m_bEnabled;
}

void CPWL_Wnd::OnEnabled()
{
}

void CPWL_Wnd::OnDisabled()
{
}

FX_BOOL CPWL_Wnd::IsCTRLpressed(FX_DWORD nFlag) const
{
	if (IFX_SystemHandler* pSystemHandler = GetSystemHandler())
	{
		return pSystemHandler->IsCTRLKeyDown(nFlag);
	}

	return FALSE;
}

FX_BOOL	CPWL_Wnd::IsSHIFTpressed(FX_DWORD nFlag) const
{
	if (IFX_SystemHandler* pSystemHandler = GetSystemHandler())
	{
		return pSystemHandler->IsSHIFTKeyDown(nFlag);
	}

	return FALSE;
}

FX_BOOL	CPWL_Wnd::IsALTpressed(FX_DWORD nFlag) const
{
	if (IFX_SystemHandler* pSystemHandler = GetSystemHandler())
	{
		return pSystemHandler->IsALTKeyDown(nFlag);
	}

	return FALSE;
}

FX_BOOL	CPWL_Wnd::IsINSERTpressed(FX_DWORD nFlag) const
{
	if (IFX_SystemHandler* pSystemHandler = GetSystemHandler())
	{
		return pSystemHandler->IsINSERTKeyDown(nFlag);
	}

	return FALSE;
}

