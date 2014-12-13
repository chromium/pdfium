// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_ARITH_QE_H_
#define _JBIG2_ARITH_QE_H_
typedef struct {
    unsigned int Qe;
    unsigned int NMPS;
    unsigned int NLPS;
    unsigned int nSwitch;
} JBig2ArithQe;
extern const JBig2ArithQe QeTable[];
#endif
