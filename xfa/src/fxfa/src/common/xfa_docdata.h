// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCDATA_H
#define _XFA_DOCDATA_H
enum XFA_DATAFORMAT {
  XFA_DATAFORMAT_XDP,
};
class IXFA_PacketExport {
 public:
  static IXFA_PacketExport* Create(CXFA_Document* pDocument,
                                   XFA_DATAFORMAT eFormat = XFA_DATAFORMAT_XDP);
  virtual ~IXFA_PacketExport() {}
  virtual void Release() = 0;
  virtual FX_BOOL Export(IFX_FileWrite* pWrite) = 0;
  virtual FX_BOOL Export(IFX_FileWrite* pWrite,
                         CXFA_Node* pNode,
                         FX_DWORD dwFlag = 0,
                         const FX_CHAR* pChecksum = NULL) = 0;
};
class IXFA_PacketImport {
 public:
  static IXFA_PacketImport* Create(CXFA_Document* pDstDoc);
  virtual ~IXFA_PacketImport() {}
  virtual void Release() = 0;
  virtual FX_BOOL ImportData(IFX_FileRead* pDataDocument) = 0;
};
#endif
