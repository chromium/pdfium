// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_APP_IMP_H
#define _FWL_APP_IMP_H
class CFWL_NoteThread;
class CFWL_WidgetMgr;
class IFWL_AdapterNative;
class IFWL_WidgetMgr;
class IFWL_ThemeProvider;
class IFWL_App;
class CFWL_AppImp;
class CFWL_AppImp : public CFWL_NoteThread
{
public:
    CFWL_AppImp(IFWL_AdapterNative *pAdapter);
    virtual ~CFWL_AppImp();
    virtual FWL_ERR				Initialize();
    virtual FWL_ERR				Finalize();
    virtual IFWL_AdapterNative*	GetAdapterNative();
    virtual IFWL_WidgetMgr*		GetWidgetMgr();
    virtual FWL_ERR				SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
    virtual FWL_ERR				Exit(FX_INT32 iExitCode = 0);
    IFWL_ThemeProvider*			GetThemeProvider();
protected:
    CFWL_WidgetMgr		*m_pWidgetMgr;
    IFWL_AdapterNative	*m_pAdapterNative;
    IFWL_ThemeProvider	*m_pThemeProvider;
    FX_BOOL				m_bFuelAdapter;
};
#endif
