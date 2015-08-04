// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_UTILRESS_H_
#define _BC_UTILRESS_H_
class CBC_RssPair;
class CBC_UtilRSS {
 public:
  virtual ~CBC_UtilRSS();
  static int32_t GetRSSvalue(CFX_Int32Array& widths,
                             int32_t maxWidth,
                             FX_BOOL noNarrow);

 protected:
  static CFX_Int32Array* GetRssWidths(int32_t val,
                                      int32_t n,
                                      int32_t elements,
                                      int32_t maxWidth,
                                      FX_BOOL noNarrow);
  static int32_t Combins(int32_t n, int32_t r);
  static CFX_Int32Array* Elements(CFX_Int32Array& eDist, int32_t N, int32_t K);

 private:
  CBC_UtilRSS();
};
#endif
