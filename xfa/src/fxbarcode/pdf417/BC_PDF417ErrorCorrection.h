// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ERRORCORRECTION_H_
#define _BC_PDF417ERRORCORRECTION_H_
class CBC_PDF417ErrorCorrection;
class CBC_PDF417ErrorCorrection : public CFX_Object
{
public:
    CBC_PDF417ErrorCorrection();
    virtual ~CBC_PDF417ErrorCorrection();
    static FX_INT32 getErrorCorrectionCodewordCount(FX_INT32 errorCorrectionLevel, FX_INT32 &e);
    static FX_INT32 getRecommendedMinimumErrorCorrectionLevel(FX_INT32 n, FX_INT32 &e);
    static CFX_WideString generateErrorCorrection(CFX_WideString dataCodewords, FX_INT32 errorCorrectionLevel, FX_INT32 &e);
private:
    static FX_INT32 EC_COEFFICIENTS[][2500];
};
#endif
