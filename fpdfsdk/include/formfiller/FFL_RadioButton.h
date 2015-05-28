// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_RADIOBUTTON_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_RADIOBUTTON_H_

#include "FFL_FormFiller.h"

class CFFL_RadioButton : public CFFL_Button
{
public:
	CFFL_RadioButton(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot);
	virtual ~CFFL_RadioButton();
	
	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView);
  	virtual FX_BOOL				OnKeyDown(CPDFSDK_Annot* pAnnot, FX_UINT nKeyCode, FX_UINT nFlags);
	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);
	virtual FX_BOOL				OnLButtonUp(CPDFSDK_PageView *pPageView, CPDFSDK_Annot* pAnnot, FX_UINT nFlags, const CPDF_Point& point);
	virtual FX_BOOL				IsDataChanged(CPDFSDK_PageView* pPageView);
	virtual void				SaveData(CPDFSDK_PageView* pPageView);
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_RADIOBUTTON_H_
