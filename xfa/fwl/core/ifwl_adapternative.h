// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_ADAPTERNATIVE_H_
#define XFA_FWL_CORE_IFWL_ADAPTERNATIVE_H_

class IFWL_WidgetMgrDelegate;
class IFWL_AdapterWidgetMgr;
class IFWL_AdapterThreadMgr;
class IFWL_AdapterTimerMgr;

class IFWL_AdapterNative {
 public:
  virtual ~IFWL_AdapterNative() {}
  virtual IFWL_AdapterWidgetMgr* GetWidgetMgr(
      IFWL_WidgetMgrDelegate* pDelegate) = 0;
  virtual IFWL_AdapterThreadMgr* GetThreadMgr() = 0;
  virtual IFWL_AdapterTimerMgr* GetTimerMgr() = 0;
};

#endif  // XFA_FWL_CORE_IFWL_ADAPTERNATIVE_H_
