// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_
#define FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_

#include "PWL_Wnd.h"

class PWL_CLASS CPWL_Button : public CPWL_Wnd
{
public:
	CPWL_Button();
	virtual ~CPWL_Button();

	virtual CFX_ByteString		GetClassName() const;
	virtual void				OnCreate(PWL_CREATEPARAM & cp);
	virtual FX_BOOL				OnLButtonDown(const CPDF_Point & point, FX_DWORD nFlag);
	virtual FX_BOOL				OnLButtonUp(const CPDF_Point & point, FX_DWORD nFlag);

protected:
	FX_BOOL						m_bMouseDown;
};

#endif  // FPDFSDK_INCLUDE_PDFWINDOW_PWL_BUTTON_H_
