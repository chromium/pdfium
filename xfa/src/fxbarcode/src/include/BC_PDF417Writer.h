// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417WRITER_H_
#define _BC_PDF417WRITER_H_
class CBC_TwoDimWriter;
class CBC_PDF417Writer;
class CBC_PDF417Writer : public CBC_TwoDimWriter
{
public:
    CBC_PDF417Writer();
    virtual ~CBC_PDF417Writer();
    FX_BYTE *	Encode(const CFX_WideString &contents, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);

    FX_BYTE *	Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 &e);
    FX_BYTE *	Encode(const CFX_ByteString &contents, BCFORMAT format, FX_INT32 &outWidth, FX_INT32 &outHeight, FX_INT32 hints, FX_INT32 &e);
    FX_BOOL		SetErrorCorrectionLevel(FX_INT32 level);
    void		SetTruncated(FX_BOOL truncated);
private:
    void		rotateArray(CFX_ByteArray& bitarray, FX_INT32 width, FX_INT32 height);
    FX_BOOL		m_bTruncated;
};
#endif
