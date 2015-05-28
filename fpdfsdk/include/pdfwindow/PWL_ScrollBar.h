// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_SCROLLBAR_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_SCROLLBAR_H_

#include "PWL_Wnd.h"

class CPWL_SBButton;
class CPWL_ScrollBar;

struct PWL_SCROLL_INFO
{
public:
	PWL_SCROLL_INFO() : fContentMin(0.0f), fContentMax(0.0f), fPlateWidth(0.0f), fBigStep(0.0f), fSmallStep(0.0f)
	{
	}
	FX_FLOAT					fContentMin;
	FX_FLOAT					fContentMax;	
	FX_FLOAT					fPlateWidth;	
	FX_FLOAT					fBigStep;
	FX_FLOAT					fSmallStep;
};

enum PWL_SCROLLBAR_TYPE
{
	SBT_HSCROLL,
	SBT_VSCROLL
};

enum PWL_SBBUTTON_TYPE
{
	PSBT_MIN,
	PSBT_MAX,
	PSBT_POS
};

class CPWL_SBButton : public CPWL_Wnd  
{
public:
	CPWL_SBButton(PWL_SCROLLBAR_TYPE eScrollBarType,PWL_SBBUTTON_TYPE eButtonType);
	virtual ~CPWL_SBButton();

public:
	virtual CFX_ByteString		GetClassName() const;
	virtual void				OnCreate(PWL_CREATEPARAM & cp);
	virtual void				GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void				DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);
	virtual FX_BOOL				OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL				OnMouseMove(const CPDF_Point & point, FX_DWORD nFlag);

protected:
	PWL_SCROLLBAR_TYPE			m_eScrollBarType;
	PWL_SBBUTTON_TYPE			m_eSBButtonType;

	FX_BOOL						m_bMouseDown;
};

struct PWL_FLOATRANGE
{
public:
	PWL_FLOATRANGE();
	PWL_FLOATRANGE(FX_FLOAT min,FX_FLOAT max);
	void Default();
	void Set(FX_FLOAT min,FX_FLOAT max);
	FX_BOOL	In(FX_FLOAT x) const;
	FX_FLOAT GetWidth() const;

	FX_FLOAT fMin,fMax;
};

struct PWL_SCROLL_PRIVATEDATA
{
public:
	PWL_SCROLL_PRIVATEDATA();

	void Default();
	void SetScrollRange(FX_FLOAT min,FX_FLOAT max);
	void SetClientWidth(FX_FLOAT width);
	void SetSmallStep(FX_FLOAT step);
	void SetBigStep(FX_FLOAT step);
	FX_BOOL SetPos(FX_FLOAT pos);

	void AddSmall();
	void SubSmall();
	void AddBig();
	void SubBig();

	PWL_FLOATRANGE				ScrollRange;
	FX_FLOAT					fClientWidth;
	FX_FLOAT					fScrollPos;
	FX_FLOAT					fBigStep;
	FX_FLOAT					fSmallStep;
};

class CPWL_ScrollBar : public CPWL_Wnd  
{
public:
	CPWL_ScrollBar(PWL_SCROLLBAR_TYPE sbType = SBT_HSCROLL);
	virtual ~CPWL_ScrollBar();

public:
	virtual CFX_ByteString		GetClassName() const;
	virtual void				OnCreate(PWL_CREATEPARAM & cp);
	virtual void				RePosChildWnd();
	virtual void				GetThisAppearanceStream(CFX_ByteTextBuf & sAppStream);
	virtual void				DrawThisAppearance(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device);

	virtual FX_BOOL				OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);
	virtual void				OnNotify(CPWL_Wnd* pWnd, FX_DWORD msg, FX_INTPTR wParam = 0, FX_INTPTR lParam = 0);

	virtual void				CreateChildWnd(const PWL_CREATEPARAM & cp);
	
	FX_FLOAT					GetScrollBarWidth() const;	
	PWL_SCROLLBAR_TYPE			GetScrollBarType() const {return m_sbType;};

	void						SetNotifyForever(FX_BOOL bForever) {m_bNotifyForever = bForever;}

protected:			
	void						SetScrollRange(FX_FLOAT fMin,FX_FLOAT fMax,FX_FLOAT fClientWidth);
	void						SetScrollPos(FX_FLOAT fPos);
	void						MovePosButton(FX_BOOL bRefresh);
	void						SetScrollStep(FX_FLOAT fBigStep,FX_FLOAT fSmallStep);
	void						NotifyScrollWindow();
	CPDF_Rect					GetScrollArea() const;

private:
	void						CreateButtons(const PWL_CREATEPARAM & cp);

	void						OnMinButtonLBDown(const CPDF_Point & point);
	void						OnMinButtonLBUp(const CPDF_Point & point);
	void						OnMinButtonMouseMove(const CPDF_Point & point);

	void						OnMaxButtonLBDown(const CPDF_Point & point);
	void						OnMaxButtonLBUp(const CPDF_Point & point);
	void						OnMaxButtonMouseMove(const CPDF_Point & point);

	void						OnPosButtonLBDown(const CPDF_Point & point);
	void						OnPosButtonLBUp(const CPDF_Point & point);
	void						OnPosButtonMouseMove(const CPDF_Point & point);

	FX_FLOAT					TrueToFace(FX_FLOAT);
	FX_FLOAT					FaceToTrue(FX_FLOAT);

	virtual	void				TimerProc();

private:
	PWL_SCROLLBAR_TYPE			m_sbType;
	PWL_SCROLL_INFO				m_OriginInfo;
	CPWL_SBButton*				m_pMinButton;
	CPWL_SBButton*				m_pMaxButton;
	CPWL_SBButton*				m_pPosButton;
	PWL_SCROLL_PRIVATEDATA		m_sData;
	FX_BOOL						m_bMouseDown;	
	FX_BOOL						m_bMinOrMax;
	FX_BOOL						m_bNotifyForever;
	FX_FLOAT					m_nOldPos;
	FX_FLOAT					m_fOldPosButton;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_SCROLLBAR_H_
