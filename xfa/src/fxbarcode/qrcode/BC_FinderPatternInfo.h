// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_FINDERPATTERNINFO_H_
#define _BC_FINDERPATTERNINFO_H_
class CBC_QRFinderPattern;
class CBC_QRFinderPatternInfo {
 private:
  CBC_QRFinderPattern* m_bottomLeft;
  CBC_QRFinderPattern* m_topLeft;
  CBC_QRFinderPattern* m_topRight;

 public:
  CBC_QRFinderPatternInfo(CFX_PtrArray* patternCenters);
  virtual ~CBC_QRFinderPatternInfo();
  CBC_QRFinderPattern* GetBottomLeft();
  CBC_QRFinderPattern* GetTopLeft();
  CBC_QRFinderPattern* GetTopRight();
};
#endif
