// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_BARCODEMETADATA_H_
#define _BC_BARCODEMETADATA_H_
class CBC_BarcodeMetadata;
class CBC_BarcodeMetadata : public CFX_Object
{
public:
    CBC_BarcodeMetadata(FX_INT32 columnCount, FX_INT32 rowCountUpperPart, FX_INT32 rowCountLowerPart, FX_INT32 errorCorrectionLevel);
    virtual ~CBC_BarcodeMetadata();
    FX_INT32 getColumnCount();
    FX_INT32 getErrorCorrectionLevel();
    FX_INT32 getRowCount();
    FX_INT32 getRowCountUpperPart();
    FX_INT32 getRowCountLowerPart();
private:
    FX_INT32 m_columnCount;
    FX_INT32 m_errorCorrectionLevel;
    FX_INT32 m_rowCountUpperPart;
    FX_INT32 m_rowCountLowerPart;
    FX_INT32 m_rowCount;
};
#endif
