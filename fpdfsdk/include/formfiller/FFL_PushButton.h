// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FORMFILLER_FFL_PUSHBUTTON_H_
#define FPDFSDK_INCLUDE_FORMFILLER_FFL_PUSHBUTTON_H_

#include "FFL_FormFiller.h"

class CFFL_PushButton : public CFFL_Button
{
public:
	CFFL_PushButton(CPDFDoc_Environment* pApp, CPDFSDK_Annot* pAnnot);
	virtual ~CFFL_PushButton();

	virtual CPWL_Wnd*			NewPDFWindow(const PWL_CREATEPARAM& cp, CPDFSDK_PageView* pPageView);

	virtual FX_BOOL				OnChar(CPDFSDK_Annot* pAnnot, FX_UINT nChar, FX_UINT nFlags);
	virtual void				OnDraw(CPDFSDK_PageView *pPageView,  CPDFSDK_Annot* pAnnot, 
									CFX_RenderDevice* pDevice, CPDF_Matrix* pUser2Device,
									FX_DWORD dwFlags);
};

#endif  // FPDFSDK_INCLUDE_FORMFILLER_FFL_PUSHBUTTON_H_
