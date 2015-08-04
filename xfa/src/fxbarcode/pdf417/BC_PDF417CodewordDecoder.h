// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417CODEWORDDECODER_H_
#define _BC_PDF417CODEWORDDECODER_H_
class CBC_PDF417CodewordDecoder;
#define column 8
class CBC_PDF417Common;
class CBC_PDF417CodewordDecoder {
 public:
  CBC_PDF417CodewordDecoder();
  virtual ~CBC_PDF417CodewordDecoder();
  static void Initialize();
  static void Finalize();
  static int32_t getDecodedValue(CFX_Int32Array& moduleBitCount);

 private:
  static FX_FLOAT RATIOS_TABLE[][8];
  static CFX_Int32Array* sampleBitCounts(CFX_Int32Array& moduleBitCount);
  static int32_t getDecodedCodewordValue(CFX_Int32Array& moduleBitCount);
  static int32_t getBitValue(CFX_Int32Array& moduleBitCount);
  static int32_t getClosestDecodedValue(CFX_Int32Array& moduleBitCount);
};
#endif
