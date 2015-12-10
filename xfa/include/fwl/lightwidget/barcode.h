// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_BARCODE_LIGHT_H
#define _FWL_BARCODE_LIGHT_H
class CFWL_Widget;
class CFWL_WidgetProperties;
class IFWL_BarcodeDP;
class CFWL_Edit;
class CFWL_Barcode;
class CFWL_BarcodeDP;
class CFWL_Barcode : public CFWL_Edit {
 public:
  static CFWL_Barcode* Create();
  FWL_ERR Initialize(const CFWL_WidgetProperties* pProperties = NULL);
  void SetType(BC_TYPE type);
  FX_BOOL IsProtectedType();

 public:
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
    static_cast<IFWL_Barcode*>(m_pIface)->SetLimit(dataLength);
  }
  void SetCalChecksum(int32_t calChecksum) {
    m_barcodeData.m_dwAttributeMask |= FWL_BCDATTRIBUTE_CALCHECKSUM;
    m_barcodeData.m_nCalChecksum = calChecksum;
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
  void ResetBarcodeAttributes() { m_barcodeData.m_dwAttributeMask = 0; }

 protected:
  CFWL_Barcode();
  virtual ~CFWL_Barcode();
  class CFWL_BarcodeDP : public IFWL_BarcodeDP {
   public:
    virtual FWL_ERR GetCaption(IFWL_Widget* pWidget, CFX_WideString& wsCaption);
    BC_CHAR_ENCODING m_eCharEncoding;
    virtual BC_CHAR_ENCODING GetCharEncoding() { return m_eCharEncoding; }
    int32_t m_nModuleHeight, m_nModuleWidth;
    virtual int32_t GetModuleHeight() { return m_nModuleHeight; }
    virtual int32_t GetModuleWidth() { return m_nModuleWidth; }
    int32_t m_nDataLength;
    virtual int32_t GetDataLength() { return m_nDataLength; }
    int32_t m_nCalChecksum;
    virtual int32_t GetCalChecksum() { return m_nCalChecksum; }
    FX_BOOL m_bPrintChecksum;
    virtual FX_BOOL GetPrintChecksum() { return m_bPrintChecksum; }

    BC_TEXT_LOC m_eTextLocation;
    virtual BC_TEXT_LOC GetTextLocation() { return m_eTextLocation; }
    int32_t m_nWideNarrowRatio;
    virtual int32_t GetWideNarrowRatio() { return m_nWideNarrowRatio; }
    FX_CHAR m_cStartChar, m_cEndChar;
    virtual FX_CHAR GetStartChar() { return m_cStartChar; }
    virtual FX_CHAR GetEndChar() { return m_cEndChar; }
    int32_t m_nVersion;
    virtual int32_t GetVersion() { return m_nVersion; }
    int32_t m_nECLevel;
    virtual int32_t GetErrorCorrectionLevel() { return m_nECLevel; }
    FX_BOOL m_bTruncated;
    virtual FX_BOOL GetTruncated() { return m_bTruncated; }
    FX_DWORD m_dwAttributeMask;
    virtual FX_DWORD GetBarcodeAttributeMask() { return m_dwAttributeMask; }

   public:
    CFWL_BarcodeDP() : m_dwAttributeMask(0) {}
  };
  CFWL_BarcodeDP m_barcodeData;
};
#endif
