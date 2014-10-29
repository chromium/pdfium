// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_APP_H
#define _FWL_APP_H
class IFWL_NoteThread;
class IFWL_AdapterNative;
class IFWL_Widget;
class IFWL_WidgetMgr;
class IFWL_ThemeProvider;
class IFWL_AdapterWidgetMgr;
class IFWL_App;
class IFWL_App : public IFWL_NoteThread
{
public:
    static IFWL_App* Create(IFWL_AdapterNative *pAdapter);
    virtual FWL_ERR				Initialize() = 0;
    virtual FWL_ERR				Finalize() = 0;
    virtual IFWL_AdapterNative*	GetAdapterNative() = 0;
    virtual IFWL_WidgetMgr*		GetWidgetMgr() = 0;
    virtual FWL_ERR				SetThemeProvider(IFWL_ThemeProvider* pThemeProvider) = 0;
    virtual FWL_ERR				Exit(FX_INT32 iExitCode = 0) = 0;
};
IFWL_App* FWL_GetApp();
void FWL_SetApp(IFWL_App *pApp);
IFWL_AdapterNative*	FWL_GetAdapterNative();
IFWL_AdapterWidgetMgr* FWL_GetAdapterWidgetMgr();
IFWL_ThemeProvider*	FWL_GetThemeProvider();
extern FWL_ERR FWL_Execute(FX_WSTR wsExecutable, FX_WSTR wsParameters);
FWL_ERR FWL_SetFullScreen(IFWL_Widget *pWidget, FX_BOOL bFullScreen);
FX_BOOL FWL_AppIsActived();
#endif
