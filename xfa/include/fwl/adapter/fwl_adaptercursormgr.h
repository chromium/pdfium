// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_ADAPTER_CURSORMGR_H
#define _FWL_ADAPTER_CURSORMGR_H
class CFX_DIBitmap;
class IFWL_AdapterCursorMgr;
enum FWL_CURSORTYPE {
  FWL_CURSORTYPE_Arrow = 0,
  FWL_CURSORTYPE_Cross,
  FWL_CURSORTYPE_Hand,
  FWL_CURSORTYPE_InputBeam,
  FWL_CURSORTYPE_Wait,
  FWL_CURSORTYPE_SizeAll,
  FWL_CURSORTYPE_SizeNWSE,
  FWL_CURSORTYPE_SizeNESW,
  FWL_CURSORTYPE_SizeWE,
  FWL_CURSORTYPE_SizeNS,
  FWL_CURSORTYPE_Prohibition,
  FWL_CURSORTYPE_Help
};
typedef struct _FWL_HCURSOR { void* pData; } * FWL_HCURSOR;

class IFWL_AdapterCursorMgr {
 public:
  virtual ~IFWL_AdapterCursorMgr() {}
  virtual FWL_HCURSOR GetSystemCursor(FWL_CURSORTYPE eCursorType) = 0;
  virtual FWL_HCURSOR GetCustomCursor(const CFX_DIBitmap* pBitmap,
                                      FX_FLOAT xHotspot = 0,
                                      FX_FLOAT yHotspot = 0) = 0;
  virtual FWL_ERR SetCursor(FWL_HCURSOR hCursor) = 0;
  virtual FWL_ERR ShowCursor(FX_BOOL bShow) = 0;
  virtual FWL_ERR GetCursorPos(CFX_PointF& pt) = 0;
};
#endif
