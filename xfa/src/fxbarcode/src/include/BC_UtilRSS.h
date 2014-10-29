// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_UTILRESS_H_
#define _BC_UTILRESS_H_
class CBC_RssPair;
class CBC_UtilRSS : public CFX_Object
{
public:
    virtual ~CBC_UtilRSS();
    static FX_INT32 GetRSSvalue(CFX_Int32Array &widths, FX_INT32 maxWidth, FX_BOOL noNarrow);

protected:
    static CFX_Int32Array *GetRssWidths(FX_INT32 val, FX_INT32 n, FX_INT32 elements, FX_INT32 maxWidth, FX_BOOL noNarrow);
    static FX_INT32 Combins(FX_INT32 n, FX_INT32 r);
    static CFX_Int32Array *Elements(CFX_Int32Array &eDist, FX_INT32 N, FX_INT32 K);
private:
    CBC_UtilRSS();
};
#endif
