// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "barcode.h"
#include "include/BC_PDF417ResultMetadata.h"
CBC_PDF417ResultMetadata::CBC_PDF417ResultMetadata()
{
}
CBC_PDF417ResultMetadata::~CBC_PDF417ResultMetadata()
{
}
FX_INT32 CBC_PDF417ResultMetadata::getSegmentIndex()
{
    return m_segmentIndex;
}
void CBC_PDF417ResultMetadata::setSegmentIndex(FX_INT32 segmentIndex)
{
    m_segmentIndex = segmentIndex;
}
CFX_ByteString CBC_PDF417ResultMetadata::getFileId()
{
    return m_fileId;
}
void CBC_PDF417ResultMetadata::setFileId(CFX_ByteString fileId)
{
    m_fileId = fileId;
}
CFX_Int32Array& CBC_PDF417ResultMetadata::getOptionalData()
{
    return m_optionalData;
}
void CBC_PDF417ResultMetadata::setOptionalData(CFX_Int32Array &optionalData)
{
    m_optionalData.Copy(optionalData);
}
FX_BOOL CBC_PDF417ResultMetadata::isLastSegment()
{
    return m_lastSegment;
}
void CBC_PDF417ResultMetadata::setLastSegment(FX_BOOL lastSegment)
{
    m_lastSegment = lastSegment;
}
