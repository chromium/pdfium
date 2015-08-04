// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417READER_H_
#define _BC_PDF417READER_H_
class CBC_PDF417ResultMetadata {
 public:
  CBC_PDF417ResultMetadata();
  virtual ~CBC_PDF417ResultMetadata();
  int32_t getSegmentIndex();
  void setSegmentIndex(int32_t segmentIndex);
  CFX_ByteString getFileId();
  void setFileId(CFX_ByteString fileId);
  CFX_Int32Array& getOptionalData();
  void setOptionalData(CFX_Int32Array& optionalData);
  FX_BOOL isLastSegment();
  void setLastSegment(FX_BOOL lastSegment);

 private:
  int32_t m_segmentIndex;
  CFX_ByteString m_fileId;
  CFX_Int32Array m_optionalData;
  FX_BOOL m_lastSegment;
};
#endif
