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

class IFWL_App : public IFWL_NoteThread {
 public:
  static IFWL_App* Create(IFWL_AdapterNative* pAdapter);

  FWL_ERR Initialize();
  FWL_ERR Finalize();
  IFWL_AdapterNative* GetAdapterNative();
  IFWL_WidgetMgr* GetWidgetMgr();
  IFWL_ThemeProvider* GetThemeProvider();
  FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  FWL_ERR Exit(int32_t iExitCode);

 private:
  IFWL_App() {}
};
IFWL_App* FWL_GetApp();
void FWL_SetApp(IFWL_App* pApp);
IFWL_AdapterNative* FWL_GetAdapterNative();
IFWL_AdapterWidgetMgr* FWL_GetAdapterWidgetMgr();
IFWL_ThemeProvider* FWL_GetThemeProvider();
extern FWL_ERR FWL_Execute(const CFX_WideStringC& wsExecutable,
                           const CFX_WideStringC& wsParameters);
FWL_ERR FWL_SetFullScreen(IFWL_Widget* pWidget, FX_BOOL bFullScreen);
FX_BOOL FWL_AppIsActived();
#endif
