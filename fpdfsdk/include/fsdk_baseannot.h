// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FSDK_BASEANNOT_H_
#define FPDFSDK_INCLUDE_FSDK_BASEANNOT_H_

#if _FX_OS_ == _FX_ANDROID_
#include "time.h"
#else
#include <ctime>
#endif

#include "../../core/include/fpdfdoc/fpdf_doc.h"
#include "../../core/include/fxcrt/fx_basic.h"
#include "fx_systemhandler.h"

class CPDFSDK_PageView;
class CPDF_Annot;
class CPDF_Page;
class CPDF_Rect;
class CPDF_Matrix;
class CPDF_RenderOptions;
class CFX_RenderDevice;

#define CFX_IntArray				CFX_ArrayTemplate<int>

class  CPDFSDK_DateTime 
{
public:
	CPDFSDK_DateTime();
	CPDFSDK_DateTime(const CFX_ByteString& dtStr);
	CPDFSDK_DateTime(const CPDFSDK_DateTime& datetime);
	CPDFSDK_DateTime(const FX_SYSTEMTIME& st);
	
	
	CPDFSDK_DateTime&	operator = (const CPDFSDK_DateTime& datetime);
	CPDFSDK_DateTime&	operator = (const FX_SYSTEMTIME& st);
	FX_BOOL				operator == (CPDFSDK_DateTime& datetime);
	FX_BOOL				operator != (CPDFSDK_DateTime& datetime);
	FX_BOOL				operator > (CPDFSDK_DateTime& datetime);
	FX_BOOL				operator >= (CPDFSDK_DateTime& datetime);
	FX_BOOL				operator < (CPDFSDK_DateTime& datetime);
	FX_BOOL				operator <= (CPDFSDK_DateTime& datetime);	
						operator time_t();
	
	CPDFSDK_DateTime&	FromPDFDateTimeString(const CFX_ByteString& dtStr);
	CFX_ByteString		ToCommonDateTimeString();
	CFX_ByteString		ToPDFDateTimeString();
	void				ToSystemTime(FX_SYSTEMTIME& st);
	CPDFSDK_DateTime	ToGMT();
	CPDFSDK_DateTime&	AddDays(short days);
	CPDFSDK_DateTime&	AddSeconds(int seconds);
	
	void				ResetDateTime();
	
	struct FX_DATETIME
	{
		FX_SHORT	year;
		FX_BYTE		month;
		FX_BYTE		day;
		FX_BYTE		hour;
		FX_BYTE		minute;
		FX_BYTE		second;
		FX_INT8 	tzHour;
		FX_BYTE		tzMinute;
	}dt;
};

class CPDFSDK_Annot
{
public:
	CPDFSDK_Annot(CPDF_Annot* pAnnot, CPDFSDK_PageView* pPageView);
	virtual ~CPDFSDK_Annot();
public:
	virtual FX_FLOAT			GetMinWidth() const;
	virtual FX_FLOAT			GetMinHeight() const;
	//define layout order to 5.
	virtual int					GetLayoutOrder() const { return 5; }

public:
	CPDF_Annot*					GetPDFAnnot();
	
	void						SetPage(CPDFSDK_PageView* pPageView);
	CPDFSDK_PageView*			GetPageView();	
	FX_DWORD					GetFlags();
	
	// Tab Order	
	int							GetTabOrder();
	void						SetTabOrder(int iTabOrder);
	
	// Selection
	FX_BOOL						IsSelected();
	void						SetSelected(FX_BOOL bSelected);
	
	CFX_ByteString				GetType() const;
	virtual CFX_ByteString		GetSubType() const;

	CPDF_Page*					GetPDFPage();

public:
	CPDF_Dictionary*			GetAnnotDict() const;
	
	void						SetRect(const CPDF_Rect& rect);
	CPDF_Rect					GetRect() const;
	
	void						SetContents(const CFX_WideString& sContents);
	CFX_WideString				GetContents() const;
	
	void						SetAnnotName(const CFX_WideString& sName);
	CFX_WideString				GetAnnotName() const;
	
	void						SetModifiedDate(const FX_SYSTEMTIME& st);
	FX_SYSTEMTIME				GetModifiedDate() const;

	void						SetFlags(int nFlags);
	int							GetFlags() const;

	void						SetAppState(const CFX_ByteString& str);
	CFX_ByteString				GetAppState() const;
	
	void						SetStructParent(int key);
	int							GetStructParent() const;
	
	//border
	void						SetBorderWidth(int nWidth);
	int							GetBorderWidth() const;
	
	//BBS_SOLID
	//BBS_DASH
	//BBS_BEVELED
	//BBS_INSET
	//BBS_UNDERLINE
	
	void						SetBorderStyle(int nStyle);
	int							GetBorderStyle() const;
	
	void						SetBorderDash(const CFX_IntArray& array);
	void						GetBorderDash(CFX_IntArray& array) const;
	
	//The background of the annotation's icon when closed
	//The title bar of the annotation's pop-up window
	//The border of a link annotation
	
	void						SetColor(FX_COLORREF color);
	void						RemoveColor();
	FX_BOOL						GetColor(FX_COLORREF& color) const;
	
	FX_BOOL						IsVisible() const;
	//action

	CPDF_Action					GetAction() const;
	void						SetAction(const CPDF_Action& a);
	void						RemoveAction();
	
	CPDF_AAction				GetAAction() const;
	void						SetAAction(const CPDF_AAction& aa);
	void						RemoveAAction();
	
	virtual CPDF_Action			GetAAction(CPDF_AAction::AActionType eAAT);
	
public:
	FX_BOOL						IsAppearanceValid();
	FX_BOOL						IsAppearanceValid(CPDF_Annot::AppearanceMode mode);
	void						DrawAppearance(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
		CPDF_Annot::AppearanceMode mode, const CPDF_RenderOptions* pOptions);
	void						DrawBorder(CFX_RenderDevice* pDevice, const CPDF_Matrix* pUser2Device,
		const CPDF_RenderOptions* pOptions);
	
	void						ClearCachedAP();
	
	void						WriteAppearance(const CFX_ByteString& sAPType, const CPDF_Rect& rcBBox, 
		const CPDF_Matrix& matrix, const CFX_ByteString& sContents,
		const CFX_ByteString& sAPState = "");

public:
	virtual void			Annot_OnDraw(CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,CPDF_RenderOptions* pOptions);
public:


private:
	FX_BOOL CreateFormFiller();
protected:
	CPDF_Annot*			m_pAnnot;
	CPDFSDK_PageView*	m_pPageView;
	FX_BOOL				m_bSelected;
	int					m_nTabOrder;
	
};

#endif  // FPDFSDK_INCLUDE_FSDK_BASEANNOT_H_
