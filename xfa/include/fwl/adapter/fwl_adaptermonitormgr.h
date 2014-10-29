// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_MONITORMGR_H
#define _FWL_ADAPTER_MONITORMGR_H
class IFWL_AdapterMonitorMgr;
typedef struct _FWL_HMONITOR {
    FX_LPVOID pData;
} *FWL_HMONITOR;
class IFWL_AdapterMonitorMgr
{
public:
    virtual FX_INT32		CountMonitors() = 0;
    virtual FWL_HMONITOR	GetMonitor(FX_INT32 nIndex) = 0;
    virtual FWL_HMONITOR	GetCurrentMonitor() = 0;
    virtual FWL_HMONITOR	GetMonitorByRect(const CFX_RectF &rect) = 0;
    virtual FWL_HMONITOR	GetMonitorByPoint(FX_FLOAT fx, FX_FLOAT fy) = 0;
    virtual	FWL_ERR			GetMonitorSize(FWL_HMONITOR hMonitor, FX_FLOAT &fx, FX_FLOAT &fy) = 0;
};
#endif
