// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_FWL_APPIMP_H_
#define XFA_FWL_CORE_FWL_APPIMP_H_

#include <memory>

#include "xfa/fwl/core/fwl_noteimp.h"

class CFWL_WidgetMgr;
class IFWL_AdapterNative;
class IFWL_App;
class IFWL_NoteThread;
class IFWL_ThemeProvider;
class IFWL_WidgetMgr;

class CFWL_AppImp {
 public:
  CFWL_AppImp(IFWL_App* pIface, IFWL_AdapterNative* pAdapter);
  virtual ~CFWL_AppImp();

  IFWL_App* GetInterface() const { return m_pIface; }
  CFWL_NoteDriver* GetNoteDriver() const { return m_pNoteDriver.get(); }

  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual IFWL_AdapterNative* GetAdapterNative() const;
  virtual IFWL_WidgetMgr* GetWidgetMgr() const;
  virtual IFWL_ThemeProvider* GetThemeProvider() const;
  virtual FWL_ERR SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);
  virtual FWL_ERR Exit(int32_t iExitCode = 0);

 protected:
  IFWL_AdapterNative* const m_pAdapterNative;
  std::unique_ptr<CFWL_WidgetMgr> m_pWidgetMgr;
  IFWL_ThemeProvider* m_pThemeProvider;
  std::unique_ptr<CFWL_NoteDriver> m_pNoteDriver;

 private:
  IFWL_App* const m_pIface;
};

#endif  // XFA_FWL_CORE_FWL_APPIMP_H_
