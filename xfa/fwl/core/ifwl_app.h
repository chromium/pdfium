// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_APP_H_
#define XFA_FWL_CORE_IFWL_APP_H_

#include "core/fxcrt/include/fx_string.h"
#include "xfa/fwl/core/ifwl_thread.h"

class IFWL_AdapterNative;
class IFWL_Widget;
class IFWL_WidgetMgr;
class IFWL_ThemeProvider;
class IFWL_AdapterWidgetMgr;

class IFWL_App : public IFWL_Thread {
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

#endif  // XFA_FWL_CORE_IFWL_APP_H_
