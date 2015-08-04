// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_DOCUMENT_SERIALIZE_H_
#define _XFA_DOCUMENT_SERIALIZE_H_
class CXFA_DataImporter : public IXFA_PacketImport {
 public:
  CXFA_DataImporter(CXFA_Document* pDocument);
  virtual void Release() { delete this; }
  virtual FX_BOOL ImportData(IFX_FileRead* pDataDocument);

 protected:
  CXFA_Document* m_pDocument;
};
class CXFA_DataExporter : public IXFA_PacketExport {
 public:
  CXFA_DataExporter(CXFA_Document* pDocument);
  virtual void Release() { delete this; }
  virtual FX_BOOL Export(IFX_FileWrite* pWrite);
  virtual FX_BOOL Export(IFX_FileWrite* pWrite,
                         CXFA_Node* pNode,
                         FX_DWORD dwFlag = 0,
                         const FX_CHAR* pChecksum = NULL);

 protected:
  FX_BOOL Export(IFX_Stream* pStream,
                 CXFA_Node* pNode,
                 FX_DWORD dwFlag,
                 const FX_CHAR* pChecksum);
  CXFA_Document* m_pDocument;
};
#endif
