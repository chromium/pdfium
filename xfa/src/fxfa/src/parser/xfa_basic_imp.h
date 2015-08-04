// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_BASIC_IMP
#define _XFA_BASIC_IMP
typedef struct _XFA_NOTSUREATTRIBUTE {
  XFA_ELEMENT eElement;
  XFA_ATTRIBUTE eAttribute;
  XFA_ATTRIBUTETYPE eType;
  void* pValue;
} XFA_NOTSUREATTRIBUTE, *XFA_LPNOTSUREATTRIBUTE;
typedef XFA_NOTSUREATTRIBUTE const* XFA_LPCNOTSUREATTRIBUTE;
XFA_LPCNOTSUREATTRIBUTE XFA_GetNotsureAttribute(
    XFA_ELEMENT eElement,
    XFA_ATTRIBUTE eAttribute,
    XFA_ATTRIBUTETYPE eType = XFA_ATTRIBUTETYPE_NOTSURE);
class CXFA_WideTextRead : public IFX_Stream {
 public:
  CXFA_WideTextRead(const CFX_WideString& wsBuffer);
  virtual void Release();
  virtual IFX_Stream* Retain();

  virtual FX_DWORD GetAccessModes() const;
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition();
  virtual FX_BOOL IsEOF() const;

  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) { return 0; }
  virtual int32_t ReadString(FX_WCHAR* pStr,
                             int32_t iMaxLength,
                             FX_BOOL& bEOS,
                             int32_t const* pByteSize = NULL);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
    return 0;
  }
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength) {
    return 0;
  }
  virtual void Flush() {}
  virtual FX_BOOL SetLength(int32_t iLength) { return FALSE; }

  virtual int32_t GetBOM(uint8_t bom[4]) const { return 0; }
  virtual FX_WORD GetCodePage() const;
  virtual FX_WORD SetCodePage(FX_WORD wCodePage);

  virtual void Lock() {}
  virtual void Unlock() {}

  virtual IFX_Stream* CreateSharedStream(FX_DWORD dwAccess,
                                         int32_t iOffset,
                                         int32_t iLength) {
    return NULL;
  }

  CFX_WideString GetSrcText() const { return m_wsBuffer; }

 protected:
  CFX_WideString m_wsBuffer;
  int32_t m_iPosition;
  int32_t m_iRefCount;
};
#endif
