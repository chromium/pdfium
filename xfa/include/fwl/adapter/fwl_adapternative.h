// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_NATIVE_H
#define _FWL_ADAPTER_NATIVE_H
class IFWL_WidgetMgrDelegate;
class IFWL_AdapterWidgetMgr;
class IFWL_AdapterThreadMgr;
class IFWL_AdapterTimerMgr;
class IFWL_AdapterCursorMgr;
class IFWL_AdapterMonitorMgr;
class IFWL_AdapterClipboardMgr;

class IFWL_AdapterNative {
 public:
  virtual ~IFWL_AdapterNative() {}
  virtual IFWL_AdapterWidgetMgr* GetWidgetMgr(
      IFWL_WidgetMgrDelegate* pDelegate) = 0;
  virtual IFWL_AdapterThreadMgr* GetThreadMgr() = 0;
  virtual IFWL_AdapterTimerMgr* GetTimerMgr() = 0;
  virtual IFWL_AdapterCursorMgr* GetCursorMgr() = 0;
  virtual IFWL_AdapterMonitorMgr* GetMonitorMgr() = 0;
  virtual IFWL_AdapterClipboardMgr* GetClipboardMgr() = 0;
};
IFWL_AdapterNative* FWL_CreateFuelAdapterNative();
void FWL_ReleaseFuelAdapterNative(IFWL_AdapterNative* pAdapterNative);
#endif
