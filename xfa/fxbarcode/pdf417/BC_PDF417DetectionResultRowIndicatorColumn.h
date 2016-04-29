// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXBARCODE_PDF417_BC_PDF417DETECTIONRESULTROWINDICATORCOLUMN_H_
#define XFA_FXBARCODE_PDF417_BC_PDF417DETECTIONRESULTROWINDICATORCOLUMN_H_

class CBC_BarcodeMetadata;
class CBC_BoundingBox;

class CBC_DetectionResultRowIndicatorColumn : public CBC_DetectionResultColumn {
 public:
  CBC_DetectionResultRowIndicatorColumn(CBC_BoundingBox* boundingBox,
                                        FX_BOOL isLeft);
  virtual ~CBC_DetectionResultRowIndicatorColumn();
  void setRowNumbers();
  int32_t adjustCompleteIndicatorColumnRowNumbers(
      CBC_BarcodeMetadata barcodeMetadata);
  CFX_Int32Array* getRowHeights(int32_t& e);
  int32_t adjustIncompleteIndicatorColumnRowNumbers(
      CBC_BarcodeMetadata barcodeMetadata);
  CBC_BarcodeMetadata* getBarcodeMetadata();
  FX_BOOL isLeft();
  CFX_ByteString toString();

 private:
  void removeIncorrectCodewords(CFX_ArrayTemplate<CBC_Codeword*>* codewords,
                                CBC_BarcodeMetadata barcodeMetadata);

  FX_BOOL m_isLeft;
};

#endif  // XFA_FXBARCODE_PDF417_BC_PDF417DETECTIONRESULTROWINDICATORCOLUMN_H_
