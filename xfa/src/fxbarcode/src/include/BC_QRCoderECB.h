// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_QRCODERECB_H_
#define _BC_QRCODERECB_H_
class CBC_QRCoderECB;
class CBC_QRCoderECB  : public CFX_Object
{
private:
    FX_INT32 m_count;
    FX_INT32 m_dataCodeWords;
public:
    CBC_QRCoderECB(FX_INT32 count, FX_INT32 dataCodeWords);
    virtual ~CBC_QRCoderECB();
    FX_INT32 GetCount();
    FX_INT32 GetDataCodeWords();
};
#endif
