// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xfa/src/fxbarcode/barcode.h"
#include "xfa/src/fxbarcode/BC_Reader.h"
#include "xfa/src/fxbarcode/BC_Writer.h"
#include "xfa/src/fxbarcode/BC_DecoderResult.h"
#include "xfa/src/fxbarcode/BC_LuminanceSource.h"
#include "xfa/src/fxbarcode/BC_BufferedImageLuminanceSource.h"
#include "xfa/src/fxbarcode/BC_Binarizer.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/BC_UtilCodingConvert.h"
#include "xfa/src/fxbarcode/BC_ResultPoint.h"
#include "xfa/src/fxbarcode/BC_BinaryBitmap.h"
#include "xfa/src/fxbarcode/BC_TwoDimWriter.h"
#include "xfa/src/fxbarcode/common/BC_GlobalHistogramBinarizer.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitArray.h"
#include "xfa/src/fxbarcode/common/BC_CommonBitMatrix.h"
#include "xfa/src/fxbarcode/common/BC_CommonDecoderResult.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixReader.h"
#include "xfa/src/fxbarcode/datamatrix/BC_DataMatrixWriter.h"
#include "xfa/src/fxbarcode/oned/BC_OneDReader.h"
#include "xfa/src/fxbarcode/oned/BC_OneDimReader.h"
#include "xfa/src/fxbarcode/oned/BC_OneDimWriter.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode39Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode39Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCodaBarReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCodaBarWriter.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode128Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedCode128Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN8Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN8Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN13Reader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedEAN13Writer.h"
#include "xfa/src/fxbarcode/oned/BC_OnedUPCAReader.h"
#include "xfa/src/fxbarcode/oned/BC_OnedUPCAWriter.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectorResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Compaction.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Writer.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Detector.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectorResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Codeword.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Common.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BarcodeValue.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BarcodeMetadata.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417BoundingBox.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResultColumn.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResultRowIndicatorColumn.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DetectionResult.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417CodewordDecoder.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECModulusPoly.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECModulusGF.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ECErrorCorrection.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417DecodedBitStreamParser.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417ScanningDecoder.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417Reader.h"
#include "xfa/src/fxbarcode/pdf417/BC_PDF417HighLevelEncoder.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCodeReader.h"
#include "xfa/src/fxbarcode/qrcode/BC_QRCodeWriter.h"
CBC_CodeBase::CBC_CodeBase() {}
CBC_CodeBase::~CBC_CodeBase() {}
FX_BOOL CBC_CodeBase::SetCharEncoding(int32_t encoding) {
  if (m_pBCWriter) {
    return m_pBCWriter->SetCharEncoding(encoding);
  }
  return FALSE;
}
FX_BOOL CBC_CodeBase::SetModuleHeight(int32_t moduleHeight) {
  if (m_pBCWriter) {
    return m_pBCWriter->SetModuleHeight(moduleHeight);
  }
  return FALSE;
}
FX_BOOL CBC_CodeBase::SetModuleWidth(int32_t moduleWidth) {
  if (m_pBCWriter) {
    return m_pBCWriter->SetModuleWidth(moduleWidth);
  }
  return FALSE;
}
FX_BOOL CBC_CodeBase::SetHeight(int32_t height) {
  if (m_pBCWriter) {
    return m_pBCWriter->SetHeight(height);
  }
  return FALSE;
}
FX_BOOL CBC_CodeBase::SetWidth(int32_t width) {
  if (m_pBCWriter) {
    return m_pBCWriter->SetWidth(width);
  }
  return FALSE;
}
void CBC_CodeBase::SetBackgroundColor(FX_ARGB backgroundColor) {
  if (m_pBCWriter) {
    m_pBCWriter->SetBackgroundColor(backgroundColor);
  }
}
void CBC_CodeBase::SetBarcodeColor(FX_ARGB foregroundColor) {
  if (m_pBCWriter) {
    m_pBCWriter->SetBarcodeColor(foregroundColor);
  }
}
CBC_OneCode::CBC_OneCode(){};
CBC_OneCode::~CBC_OneCode() {}
FX_BOOL CBC_OneCode::CheckContentValidity(const CFX_WideStringC& contents) {
  if (m_pBCWriter) {
    return ((CBC_OneDimWriter*)m_pBCWriter)->CheckContentValidity(contents);
  }
  return FALSE;
}
CFX_WideString CBC_OneCode::FilterContents(const CFX_WideStringC& contents) {
  CFX_WideString tmp;
  if (m_pBCWriter == NULL) {
    return tmp;
  }
  return ((CBC_OneDimWriter*)m_pBCWriter)->FilterContents(contents);
}
void CBC_OneCode::SetPrintChecksum(FX_BOOL checksum) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetPrintChecksum(checksum);
  }
}
void CBC_OneCode::SetDataLength(int32_t length) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetDataLength(length);
  }
}
void CBC_OneCode::SetCalChecksum(FX_BOOL calc) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetCalcChecksum(calc);
  }
}
FX_BOOL CBC_OneCode::SetFont(CFX_Font* cFont) {
  if (m_pBCWriter) {
    return ((CBC_OneDimWriter*)m_pBCWriter)->SetFont(cFont);
  }
  return FALSE;
}
void CBC_OneCode::SetFontSize(FX_FLOAT size) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetFontSize(size);
  }
}
void CBC_OneCode::SetFontStyle(int32_t style) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetFontStyle(style);
  }
}
void CBC_OneCode::SetFontColor(FX_ARGB color) {
  if (m_pBCWriter) {
    ((CBC_OneDimWriter*)m_pBCWriter)->SetFontColor(color);
  }
}
CBC_Code39::CBC_Code39() {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedCode39Reader);
  m_pBCWriter = (CBC_Writer*)new (CBC_OnedCode39Writer);
}
CBC_Code39::CBC_Code39(FX_BOOL usingCheckDigit) {
  m_pBCReader = (CBC_Reader*)new CBC_OnedCode39Reader(usingCheckDigit);
  m_pBCWriter = (CBC_Writer*)new CBC_OnedCode39Writer;
}
CBC_Code39::CBC_Code39(FX_BOOL usingCheckDigit, FX_BOOL extendedMode) {
  m_pBCReader =
      (CBC_Reader*)new CBC_OnedCode39Reader(usingCheckDigit, extendedMode);
  m_pBCWriter = (CBC_Writer*)new CBC_OnedCode39Writer(extendedMode);
}
CBC_Code39::~CBC_Code39() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_Code39::Encode(const CFX_WideStringC& contents,
                           FX_BOOL isDevice,
                           int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_CODE_39;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString filtercontents =
      ((CBC_OnedCode39Writer*)m_pBCWriter)->FilterContents(contents);
  CFX_WideString renderContents =
      ((CBC_OnedCode39Writer*)m_pBCWriter)->RenderTextContents(contents);
  m_renderContents = renderContents;
  CFX_ByteString byteString = filtercontents.UTF8Encode();
  uint8_t* data = static_cast<CBC_OnedCode39Writer*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(renderContents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Code39::RenderDevice(CFX_RenderDevice* device,
                                 const CFX_Matrix* matirx,
                                 int32_t& e) {
  CFX_WideString renderCon = ((CBC_OnedCode39Writer*)m_pBCWriter)
                                 ->encodedContents(m_renderContents, e);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, renderCon, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Code39::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  CFX_WideString renderCon = ((CBC_OnedCode39Writer*)m_pBCWriter)
                                 ->encodedContents(m_renderContents, e);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, renderCon, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_Code39::Decode(uint8_t* buf,
                                  int32_t width,
                                  int32_t hight,
                                  int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_Code39::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
FX_BOOL CBC_Code39::SetTextLocation(BC_TEXT_LOC location) {
  if (m_pBCWriter) {
    return ((CBC_OnedCode39Writer*)m_pBCWriter)->SetTextLocation(location);
  }
  return FALSE;
}
FX_BOOL CBC_Code39::SetWideNarrowRatio(int32_t ratio) {
  if (m_pBCWriter) {
    return ((CBC_OnedCode39Writer*)m_pBCWriter)->SetWideNarrowRatio(ratio);
  }
  return FALSE;
}
CBC_Codabar::CBC_Codabar() {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedCodaBarReader);
  m_pBCWriter = (CBC_Writer*)new (CBC_OnedCodaBarWriter);
}
CBC_Codabar::~CBC_Codabar() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_Codabar::SetStartChar(FX_CHAR start) {
  if (m_pBCWriter) {
    return ((CBC_OnedCodaBarWriter*)m_pBCWriter)->SetStartChar(start);
  }
  return FALSE;
}
FX_BOOL CBC_Codabar::SetEndChar(FX_CHAR end) {
  if (m_pBCWriter) {
    return ((CBC_OnedCodaBarWriter*)m_pBCWriter)->SetEndChar(end);
  }
  return FALSE;
}
FX_BOOL CBC_Codabar::SetTextLocation(BC_TEXT_LOC location) {
  return ((CBC_OnedCodaBarWriter*)m_pBCWriter)->SetTextLocation(location);
}
FX_BOOL CBC_Codabar::SetWideNarrowRatio(int32_t ratio) {
  if (m_pBCWriter) {
    return ((CBC_OnedCodaBarWriter*)m_pBCWriter)->SetWideNarrowRatio(ratio);
  }
  return FALSE;
}
FX_BOOL CBC_Codabar::Encode(const CFX_WideStringC& contents,
                            FX_BOOL isDevice,
                            int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_CODABAR;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString filtercontents =
      ((CBC_OneDimWriter*)m_pBCWriter)->FilterContents(contents);
  CFX_ByteString byteString = filtercontents.UTF8Encode();
  m_renderContents = filtercontents;
  uint8_t* data = static_cast<CBC_OnedCodaBarWriter*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(filtercontents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Codabar::RenderDevice(CFX_RenderDevice* device,
                                  const CFX_Matrix* matirx,
                                  int32_t& e) {
  CFX_WideString renderCon =
      ((CBC_OnedCodaBarWriter*)m_pBCWriter)->encodedContents(m_renderContents);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, renderCon, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Codabar::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  CFX_WideString renderCon =
      ((CBC_OnedCodaBarWriter*)m_pBCWriter)->encodedContents(m_renderContents);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, renderCon, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_Codabar::Decode(uint8_t* buf,
                                   int32_t width,
                                   int32_t hight,
                                   int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_Codabar::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
CBC_Code128::CBC_Code128(BC_TYPE type) {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedCode128Reader);
  m_pBCWriter = (CBC_Writer*)new CBC_OnedCode128Writer(type);
}
CBC_Code128::~CBC_Code128() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_Code128::SetTextLocation(BC_TEXT_LOC location) {
  if (m_pBCWriter) {
    return ((CBC_OnedCode128Writer*)m_pBCWriter)->SetTextLocation(location);
  }
  return FALSE;
}
FX_BOOL CBC_Code128::Encode(const CFX_WideStringC& contents,
                            FX_BOOL isDevice,
                            int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_CODE_128;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString content = contents;
  if (contents.GetLength() % 2 &&
      ((CBC_OnedCode128Writer*)m_pBCWriter)->GetType() == BC_CODE128_C) {
    content += '0';
  }
  CFX_WideString encodeContents =
      ((CBC_OnedCode128Writer*)m_pBCWriter)->FilterContents(content);
  m_renderContents = encodeContents;
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  uint8_t* data = static_cast<CBC_OnedCode128Writer*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(encodeContents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Code128::RenderDevice(CFX_RenderDevice* device,
                                  const CFX_Matrix* matirx,
                                  int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_Code128::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_Code128::Decode(uint8_t* buf,
                                   int32_t width,
                                   int32_t hight,
                                   int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_Code128::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
CBC_EAN8::CBC_EAN8() {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedEAN8Reader);
  m_pBCWriter = (CBC_Writer*)new (CBC_OnedEAN8Writer);
}
CBC_EAN8::~CBC_EAN8() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
CFX_WideString CBC_EAN8::Preprocess(const CFX_WideStringC& contents) {
  CFX_WideString encodeContents =
      ((CBC_OnedEAN8Writer*)m_pBCWriter)->FilterContents(contents);
  int32_t length = encodeContents.GetLength();
  if (length <= 7) {
    for (int32_t i = 0; i < 7 - length; i++) {
      encodeContents = FX_WCHAR('0') + encodeContents;
    }
    CFX_ByteString byteString = encodeContents.UTF8Encode();
    int32_t checksum =
        ((CBC_OnedEAN8Writer*)m_pBCWriter)->CalcChecksum(byteString);
    encodeContents += FX_WCHAR(checksum - 0 + '0');
  }
  if (length > 8) {
    encodeContents = encodeContents.Mid(0, 8);
  }
  return encodeContents;
}
FX_BOOL CBC_EAN8::Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_EAN_8;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString encodeContents = Preprocess(contents);
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  m_renderContents = encodeContents;
  uint8_t* data = static_cast<CBC_OnedEAN8Writer*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(encodeContents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_EAN8::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matirx,
                               int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_EAN8::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_EAN8::Decode(uint8_t* buf,
                                int32_t width,
                                int32_t hight,
                                int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_EAN8::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
CBC_EAN13::CBC_EAN13() {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedEAN13Reader);
  m_pBCWriter = (CBC_Writer*)new (CBC_OnedEAN13Writer);
}
CBC_EAN13::~CBC_EAN13() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
CFX_WideString CBC_EAN13::Preprocess(const CFX_WideStringC& contents) {
  CFX_WideString encodeContents =
      ((CBC_OnedEAN8Writer*)m_pBCWriter)->FilterContents(contents);
  int32_t length = encodeContents.GetLength();
  if (length <= 12) {
    for (int32_t i = 0; i < 12 - length; i++) {
      encodeContents = FX_WCHAR('0') + encodeContents;
    }
    CFX_ByteString byteString = encodeContents.UTF8Encode();
    int32_t checksum =
        ((CBC_OnedEAN13Writer*)m_pBCWriter)->CalcChecksum(byteString);
    byteString += checksum - 0 + '0';
    encodeContents = byteString.UTF8Decode();
  }
  if (length > 13) {
    encodeContents = encodeContents.Mid(0, 13);
  }
  return encodeContents;
}
FX_BOOL CBC_EAN13::Encode(const CFX_WideStringC& contents,
                          FX_BOOL isDevice,
                          int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_EAN_13;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString encodeContents = Preprocess(contents);
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  m_renderContents = encodeContents;
  uint8_t* data = static_cast<CBC_OnedEAN13Writer*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(encodeContents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_EAN13::RenderDevice(CFX_RenderDevice* device,
                                const CFX_Matrix* matirx,
                                int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_EAN13::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_EAN13::Decode(uint8_t* buf,
                                 int32_t width,
                                 int32_t hight,
                                 int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_EAN13::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
CBC_UPCA::CBC_UPCA() {
  m_pBCReader = (CBC_Reader*)new (CBC_OnedUPCAReader);
  ((CBC_OnedUPCAReader*)m_pBCReader)->Init();
  m_pBCWriter = (CBC_Writer*)new (CBC_OnedUPCAWriter);
}
CBC_UPCA::~CBC_UPCA() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
CFX_WideString CBC_UPCA::Preprocess(const CFX_WideStringC& contents) {
  CFX_WideString encodeContents =
      ((CBC_OnedEAN8Writer*)m_pBCWriter)->FilterContents(contents);
  int32_t length = encodeContents.GetLength();
  if (length <= 11) {
    for (int32_t i = 0; i < 11 - length; i++) {
      encodeContents = FX_WCHAR('0') + encodeContents;
    }
    CFX_ByteString byteString = encodeContents.UTF8Encode();
    int32_t checksum =
        ((CBC_OnedUPCAWriter*)m_pBCWriter)->CalcChecksum(byteString);
    byteString += checksum - 0 + '0';
    encodeContents = byteString.UTF8Decode();
  }
  if (length > 12) {
    encodeContents = encodeContents.Mid(0, 12);
  }
  return encodeContents;
}
FX_BOOL CBC_UPCA::Encode(const CFX_WideStringC& contents,
                         FX_BOOL isDevice,
                         int32_t& e) {
  if (contents.IsEmpty()) {
    e = BCExceptionNoContents;
    return FALSE;
  }
  BCFORMAT format = BCFORMAT_UPC_A;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString encodeContents = Preprocess(contents);
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  m_renderContents = encodeContents;
  ((CBC_OnedUPCAWriter*)m_pBCWriter)->Init();
  uint8_t* data = static_cast<CBC_OnedUPCAWriter*>(m_pBCWriter)
                      ->Encode(byteString, format, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderResult(encodeContents, data, outWidth, isDevice, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_UPCA::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matirx,
                               int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderDeviceResult(device, matirx, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_UPCA::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_OneDimWriter*)m_pBCWriter)
      ->RenderBitmapResult(pOutBitmap, m_renderContents, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_UPCA::Decode(uint8_t* buf,
                                int32_t width,
                                int32_t hight,
                                int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_UPCA::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString str = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(str, str.GetLength());
}
CBC_QRCode::CBC_QRCode() {
  m_pBCReader = (CBC_Reader*)new (CBC_QRCodeReader);
  ((CBC_QRCodeReader*)m_pBCReader)->Init();
  m_pBCWriter = (CBC_Writer*)new (CBC_QRCodeWriter);
}
CBC_QRCode::~CBC_QRCode() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_QRCode::SetVersion(int32_t version) {
  if (version < 0 || version > 40) {
    return FALSE;
  }
  if (m_pBCWriter == NULL) {
    return FALSE;
  }
  return ((CBC_QRCodeWriter*)m_pBCWriter)->SetVersion(version);
}
FX_BOOL CBC_QRCode::SetErrorCorrectionLevel(int32_t level) {
  if (level < 0 || level > 3) {
    return FALSE;
  }
  if (m_pBCWriter == NULL) {
    return FALSE;
  }
  return ((CBC_TwoDimWriter*)m_pBCWriter)->SetErrorCorrectionLevel(level);
}
FX_BOOL CBC_QRCode::Encode(const CFX_WideStringC& contents,
                           FX_BOOL isDevice,
                           int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  uint8_t* data = ((CBC_QRCodeWriter*)m_pBCWriter)
                      ->Encode(contents, ((CBC_QRCodeWriter*)m_pBCWriter)
                                             ->GetErrorCorrectionLevel(),
                               outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderResult(data, outWidth, outHeight, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_QRCode::RenderDevice(CFX_RenderDevice* device,
                                 const CFX_Matrix* matirx,
                                 int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderDeviceResult(device, matirx);
  return TRUE;
}
FX_BOOL CBC_QRCode::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderBitmapResult(pOutBitmap, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_QRCode::Decode(uint8_t* buf,
                                  int32_t width,
                                  int32_t hight,
                                  int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_QRCode::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString retStr = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(retStr, retStr.GetLength());
}
CBC_PDF417I::CBC_PDF417I() {
  m_pBCReader = (CBC_Reader*)new (CBC_PDF417Reader);
  m_pBCWriter = (CBC_Writer*)new (CBC_PDF417Writer);
}
CBC_PDF417I::~CBC_PDF417I() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_PDF417I::SetErrorCorrectionLevel(int32_t level) {
  ((CBC_PDF417Writer*)m_pBCWriter)->SetErrorCorrectionLevel(level);
  return TRUE;
}
void CBC_PDF417I::SetTruncated(FX_BOOL truncated) {
  ((CBC_PDF417Writer*)m_pBCWriter)->SetTruncated(truncated);
}
FX_BOOL CBC_PDF417I::Encode(const CFX_WideStringC& contents,
                            FX_BOOL isDevice,
                            int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  uint8_t* data = ((CBC_PDF417Writer*)m_pBCWriter)
                      ->Encode(contents, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderResult(data, outWidth, outHeight, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_PDF417I::RenderDevice(CFX_RenderDevice* device,
                                  const CFX_Matrix* matirx,
                                  int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderDeviceResult(device, matirx);
  return TRUE;
}
FX_BOOL CBC_PDF417I::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderBitmapResult(pOutBitmap, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_PDF417I::Decode(uint8_t* buf,
                                   int32_t width,
                                   int32_t hight,
                                   int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_PDF417I::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString bytestring = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(bytestring, bytestring.GetLength());
}
CBC_DataMatrix::CBC_DataMatrix() {
  m_pBCReader = (CBC_Reader*)new (CBC_DataMatrixReader);
  ((CBC_DataMatrixReader*)m_pBCReader)->Init();
  m_pBCWriter = (CBC_Writer*)new (CBC_DataMatrixWriter);
}
CBC_DataMatrix::~CBC_DataMatrix() {
  if (m_pBCReader) {
    delete (m_pBCReader);
    m_pBCReader = NULL;
  }
  if (m_pBCWriter) {
    delete (m_pBCWriter);
    m_pBCWriter = NULL;
  }
}
FX_BOOL CBC_DataMatrix::Encode(const CFX_WideStringC& contents,
                               FX_BOOL isDevice,
                               int32_t& e) {
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  uint8_t* data = ((CBC_DataMatrixWriter*)m_pBCWriter)
                      ->Encode(contents, outWidth, outHeight, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderResult(data, outWidth, outHeight, e);
  FX_Free(data);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
FX_BOOL CBC_DataMatrix::RenderDevice(CFX_RenderDevice* device,
                                     const CFX_Matrix* matirx,
                                     int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderDeviceResult(device, matirx);
  return TRUE;
}
FX_BOOL CBC_DataMatrix::RenderBitmap(CFX_DIBitmap*& pOutBitmap, int32_t& e) {
  ((CBC_TwoDimWriter*)m_pBCWriter)->RenderBitmapResult(pOutBitmap, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FALSE);
  return TRUE;
}
CFX_WideString CBC_DataMatrix::Decode(uint8_t* buf,
                                      int32_t width,
                                      int32_t hight,
                                      int32_t& e) {
  CFX_WideString str;
  return str;
}
CFX_WideString CBC_DataMatrix::Decode(CFX_DIBitmap* pBitmap, int32_t& e) {
  CBC_BufferedImageLuminanceSource source(pBitmap);
  CBC_GlobalHistogramBinarizer binarizer(&source);
  CBC_BinaryBitmap bitmap(&binarizer);
  CFX_ByteString retStr = m_pBCReader->Decode(&bitmap, 0, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, FX_WSTRC(L""));
  return CFX_WideString::FromUTF8(retStr, retStr.GetLength());
}
