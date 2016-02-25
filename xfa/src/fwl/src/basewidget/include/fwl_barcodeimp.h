// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_SRC_FWL_SRC_BASEWIDGET_INCLUDE_FWL_BARCODEIMP_H_
#define XFA_SRC_FWL_SRC_BASEWIDGET_INCLUDE_FWL_BARCODEIMP_H_

#include "xfa/include/fwl/basewidget/fwl_barcode.h"
#include "xfa/include/fwl/basewidget/fwl_scrollbar.h"
#include "xfa/include/fwl/basewidget/fxmath_barcode.h"
#include "xfa/src/fwl/src/basewidget/include/fwl_editimp.h"

class CFWL_WidgetImpProperties;
class CFWL_BarcodeImpDelegate;
class IFWL_Widget;

#define XFA_BCS_NeedUpdate 0x0001
#define XFA_BCS_EncodeSuccess 0x0002

class CFWL_BarcodeImp : public CFWL_EditImp {
 public:
  CFWL_BarcodeImp(const CFWL_WidgetImpProperties& properties,
                  IFWL_Widget* pOuter);
  virtual ~CFWL_BarcodeImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FWL_ERR Update();
  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);
  virtual FWL_ERR SetText(const CFX_WideString& wsText);
  virtual void SetType(BC_TYPE type);
  FX_BOOL IsProtectedType();

 protected:
  void GenerateBarcodeImageCache();
  void CreateBarcodeEngine();
  void ReleaseBarcodeEngine();
  IFX_Barcode* m_pBarcodeEngine;
  FX_DWORD m_dwStatus;
  BC_TYPE m_type;
  friend class CFWL_BarcodeImpDelegate;
};

class CFWL_BarcodeImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_BarcodeImpDelegate(CFWL_BarcodeImp* pOwner);
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
};

#endif  // XFA_SRC_FWL_SRC_BASEWIDGET_INCLUDE_FWL_BARCODEIMP_H_
