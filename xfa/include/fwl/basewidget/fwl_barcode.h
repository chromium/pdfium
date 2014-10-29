// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_BARCODE_H
#define _FWL_BARCODE_H
#include "fwl_edit.h"
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_BarcodeDP;
class IFWL_Barcode;
#define FWL_CLASS_Barcode				L"FWL_BARCODE"
#define FWL_CLASSHASH_Barcode			366886968
#define FWL_BCDATTRIBUTE_CHARENCODING		(1L << 0)
#define FWL_BCDATTRIBUTE_MODULEHEIGHT		(1L << 1)
#define FWL_BCDATTRIBUTE_MODULEWIDTH		(1L << 2)
#define FWL_BCDATTRIBUTE_DATALENGTH			(1L << 3)
#define FWL_BCDATTRIBUTE_CALCHECKSUM		(1L << 4)
#define FWL_BCDATTRIBUTE_PRINTCHECKSUM		(1L << 5)
#define FWL_BCDATTRIBUTE_TEXTLOCATION		(1L << 6)
#define FWL_BCDATTRIBUTE_WIDENARROWRATIO	(1L << 7)
#define FWL_BCDATTRIBUTE_STARTCHAR			(1L << 8)
#define FWL_BCDATTRIBUTE_ENDCHAR			(1L << 9)
#define FWL_BCDATTRIBUTE_VERSION			(1L << 10)
#define FWL_BCDATTRIBUTE_ECLEVEL			(1L << 11)
#define FWL_BCDATTRIBUTE_TRUNCATED			(1L << 12)
#define FWL_PART_BCD_Border				1
#define FWL_PART_BCD_Edge				2
#define FWL_PART_BCD_Background			3
#define FWL_BCUPDATECMD_Data			FWL_WGTUPDATECMD_User
class IFWL_BarcodeDP : public IFWL_EditDP
{
public:
    virtual BC_CHAR_ENCODING	GetCharEncoding() = 0;
    virtual FX_INT32			GetModuleHeight() = 0;
    virtual FX_INT32			GetModuleWidth() = 0;
    virtual FX_INT32			GetDataLength() = 0;
    virtual FX_INT32			GetCalChecksum() = 0;
    virtual FX_BOOL				GetPrintChecksum() = 0;
    virtual BC_TEXT_LOC			GetTextLocation() = 0;
    virtual FX_INT32			GetWideNarrowRatio() = 0;
    virtual FX_CHAR				GetStartChar() = 0;
    virtual FX_CHAR				GetEndChar() = 0;
    virtual FX_INT32			GetVersion() = 0;
    virtual FX_INT32			GetErrorCorrectionLevel() = 0;
    virtual FX_BOOL				GetTruncated() = 0;
    virtual FX_DWORD			GetBarcodeAttributeMask() = 0;
};
class IFWL_Barcode : public IFWL_Edit
{
public:
    static IFWL_Barcode* Create();
    FWL_ERR		Initialize(IFWL_Widget *pOuter = NULL);
    FWL_ERR		Initialize(const CFWL_WidgetImpProperties &properties, IFWL_Widget *pOuter = NULL);
    void		SetType(BC_TYPE type);
    FX_BOOL		IsProtectedType();
protected:
    IFWL_Barcode();
    virtual ~IFWL_Barcode();
};
#endif
