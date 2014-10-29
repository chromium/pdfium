// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_APP_LIGHT_H
#define _FWL_APP_LIGHT_H
class CFWL_Theme;
class IFWL_App;
class CFWL_App;
class CFWL_App : public CFX_Object
{
public:
    CFWL_App();
    virtual ~CFWL_App();
    FWL_ERR		Initialize();
    CFWL_Theme*		GetTheme();
    FWL_ERR		Exit(FX_INT32 iExitCode = 0);
    IFWL_App*	GetApp();
protected:
    IFWL_App	*m_pAppImp;
    CFWL_Theme	*m_pThemeProvider;
};
#endif
