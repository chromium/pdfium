// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_BARCODE_H_
#define XFA_FWL_CORE_CFWL_BARCODE_H_

#include "xfa/fwl/core/cfwl_edit.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_barcode.h"

class CFWL_Widget;
class CFWL_WidgetProperties;

class CFWL_Barcode : public CFWL_Edit {
 public:
  CFWL_Barcode(const IFWL_App*);
  ~CFWL_Barcode() override;

  IFWL_Barcode* GetWidget() override;
  const IFWL_Barcode* GetWidget() const override;

  void Initialize(const CFWL_WidgetProperties* pProperties);

  void SetType(BC_TYPE type);
  FX_BOOL IsProtectedType();

  void SetCharEncoding(BC_CHAR_ENCODING encoding) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_CHARENCODING;
    m_barcodeData.m_eCharEncoding = encoding;
  }
  void SetModuleHeight(int32_t height) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_MODULEHEIGHT;
    m_barcodeData.m_nModuleHeight = height;
  }
  void SetModuleWidth(int32_t width) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_MODULEWIDTH;
    m_barcodeData.m_nModuleWidth = width;
  }
  void SetDataLength(int32_t dataLength) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_DATALENGTH;
    m_barcodeData.m_nDataLength = dataLength;
    GetWidget()->SetLimit(dataLength);
  }
  void SetCalChecksum(FX_BOOL calChecksum) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_CALCHECKSUM;
    m_barcodeData.m_bCalChecksum = calChecksum;
  }
  void SetPrintChecksum(FX_BOOL printChecksum) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_PRINTCHECKSUM;
    m_barcodeData.m_bPrintChecksum = printChecksum;
  }
  void SetTextLocation(BC_TEXT_LOC location) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_TEXTLOCATION;
    m_barcodeData.m_eTextLocation = location;
  }
  void SetWideNarrowRatio(int32_t ratio) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_WIDENARROWRATIO;
    m_barcodeData.m_nWideNarrowRatio = ratio;
  }
  void SetStartChar(FX_CHAR startChar) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_STARTCHAR;
    m_barcodeData.m_cStartChar = startChar;
  }
  void SetEndChar(FX_CHAR endChar) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_ENDCHAR;
    m_barcodeData.m_cEndChar = endChar;
  }
  void SetVersion(int32_t version) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_VERSION;
    m_barcodeData.m_nVersion = version;
  }
  void SetErrorCorrectionLevel(int32_t ecLevel) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_ECLEVEL;
    m_barcodeData.m_nECLevel = ecLevel;
  }
  void SetTruncated(FX_BOOL truncated) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_TRUNCATED;
    m_barcodeData.m_bTruncated = truncated;
  }
  void ResetBarcodeAttributes() {
    m_barcodeData.m_dwAttributeMask = FWL_BCDATTRIBUTE_NONE;
  }

 protected:
  class CFWL_BarcodeDP : public IFWL_BarcodeDP {
   public:
    CFWL_BarcodeDP();

    // IFWL_DataProvider
    FWL_Error GetCaption(IFWL_Widget* pWidget,
                         CFX_WideString& wsCaption) override;

    // IFWL_BarcodeDP
    BC_CHAR_ENCODING GetCharEncoding() const override;
    int32_t GetModuleHeight() const override;
    int32_t GetModuleWidth() const override;
    int32_t GetDataLength() const override;
    FX_BOOL GetCalChecksum() const override;
    FX_BOOL GetPrintChecksum() const override;
    BC_TEXT_LOC GetTextLocation() const override;
    int32_t GetWideNarrowRatio() const override;
    FX_CHAR GetStartChar() const override;
    FX_CHAR GetEndChar() const override;
    int32_t GetVersion() const override;
    int32_t GetErrorCorrectionLevel() const override;
    FX_BOOL GetTruncated() const override;
    uint32_t GetBarcodeAttributeMask() const override;

    BC_CHAR_ENCODING m_eCharEncoding;
    int32_t m_nModuleHeight;
    int32_t m_nModuleWidth;
    int32_t m_nDataLength;
    FX_BOOL m_bCalChecksum;
    FX_BOOL m_bPrintChecksum;
    BC_TEXT_LOC m_eTextLocation;
    int32_t m_nWideNarrowRatio;
    FX_CHAR m_cStartChar;
    FX_CHAR m_cEndChar;
    int32_t m_nVersion;
    int32_t m_nECLevel;
    FX_BOOL m_bTruncated;
    uint32_t m_dwAttributeMask;
  };

  CFWL_BarcodeDP m_barcodeData;
};

#endif  // XFA_FWL_CORE_CFWL_BARCODE_H_
