// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_BASEWIDGET_FWL_BARCODEIMP_H_
#define XFA_FWL_BASEWIDGET_FWL_BARCODEIMP_H_

#include "xfa/fwl/basewidget/fwl_editimp.h"
#include "xfa/fwl/basewidget/ifwl_barcode.h"
#include "xfa/fwl/basewidget/ifwl_scrollbar.h"
#include "xfa/fwl/basewidget/ifx_barcode.h"

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
  virtual uint32_t GetClassID() const;
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
  uint32_t m_dwStatus;
  BC_TYPE m_type;
  friend class CFWL_BarcodeImpDelegate;
};

class CFWL_BarcodeImpDelegate : public CFWL_EditImpDelegate {
 public:
  CFWL_BarcodeImpDelegate(CFWL_BarcodeImp* pOwner);
  FWL_ERR OnProcessEvent(CFWL_Event* pEvent) override;
};

#endif  // XFA_FWL_BASEWIDGET_FWL_BARCODEIMP_H_
