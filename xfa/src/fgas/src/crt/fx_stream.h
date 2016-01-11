// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_STREAM_IMP
#define _FX_STREAM_IMP
class CFX_StreamImp;
class CFX_FileStreamImp;
class CFX_BufferStreamImp;
class CFX_FileReadStreamImp;
class CFX_BufferReadStreamImp;
class CFX_FileWriteStreamImp;
class CFX_Stream;
class CFX_TextStream;
class CFX_FileRead;
class CFX_FileWrite;
class CFX_BufferAccImp;
class CFX_StreamImp : public CFX_ThreadLock {
 public:
  virtual void Release() { delete this; }
  virtual FX_DWORD GetAccessModes() const { return m_dwAccess; }
  virtual int32_t GetLength() const = 0;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset) = 0;
  virtual int32_t GetPosition() = 0;
  virtual FX_BOOL IsEOF() const = 0;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) = 0;
  virtual int32_t ReadString(FX_WCHAR* pStr,
                             int32_t iMaxLength,
                             FX_BOOL& bEOS) = 0;
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) = 0;
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength) = 0;
  virtual void Flush() = 0;
  virtual FX_BOOL SetLength(int32_t iLength) = 0;

 protected:
  CFX_StreamImp();
  virtual ~CFX_StreamImp() {}
  FX_DWORD m_dwAccess;
};
class CFX_FileStreamImp : public CFX_StreamImp {
 public:
  CFX_FileStreamImp();
  virtual ~CFX_FileStreamImp();
  FX_BOOL LoadFile(const FX_WCHAR* pszSrcFileName, FX_DWORD dwAccess);
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition();
  virtual FX_BOOL IsEOF() const;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr, int32_t iMaxLength, FX_BOOL& bEOS);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength);
  virtual void Flush();
  virtual FX_BOOL SetLength(int32_t iLength);

 protected:
  FXSYS_FILE* m_hFile;
  int32_t m_iLength;
};
class CFX_BufferStreamImp : public CFX_StreamImp {
 public:
  CFX_BufferStreamImp();
  virtual ~CFX_BufferStreamImp() {}
  FX_BOOL LoadBuffer(uint8_t* pData, int32_t iTotalSize, FX_DWORD dwAccess);
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition();
  virtual FX_BOOL IsEOF() const;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr, int32_t iMaxLength, FX_BOOL& bEOS);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength);
  virtual void Flush() {}
  virtual FX_BOOL SetLength(int32_t iLength) { return FALSE; }

 protected:
  uint8_t* m_pData;
  int32_t m_iTotalSize;
  int32_t m_iPosition;
  int32_t m_iLength;
};
class CFX_FileReadStreamImp : public CFX_StreamImp {
 public:
  CFX_FileReadStreamImp();
  virtual ~CFX_FileReadStreamImp() {}
  FX_BOOL LoadFileRead(IFX_FileRead* pFileRead, FX_DWORD dwAccess);
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition() { return m_iPosition; }
  virtual FX_BOOL IsEOF() const;

  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr, int32_t iMaxLength, FX_BOOL& bEOS);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
    return 0;
  }
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength) {
    return 0;
  }
  virtual void Flush() {}
  virtual FX_BOOL SetLength(int32_t iLength) { return FALSE; }

 protected:
  IFX_FileRead* m_pFileRead;
  int32_t m_iPosition;
  int32_t m_iLength;
};
class CFX_BufferReadStreamImp : public CFX_StreamImp {
 public:
  CFX_BufferReadStreamImp();
  ~CFX_BufferReadStreamImp();
  FX_BOOL LoadBufferRead(IFX_BufferRead* pBufferRead,
                         int32_t iFileSize,
                         FX_DWORD dwAccess,
                         FX_BOOL bReleaseBufferRead);

  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition() { return m_iPosition; }
  virtual FX_BOOL IsEOF() const;

  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr, int32_t iMaxLength, FX_BOOL& bEOS);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize) {
    return 0;
  }
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength) {
    return 0;
  }
  virtual void Flush() {}
  virtual FX_BOOL SetLength(int32_t iLength) { return FALSE; }

 private:
  IFX_BufferRead* m_pBufferRead;
  FX_BOOL m_bReleaseBufferRead;
  int32_t m_iPosition;
  int32_t m_iBufferSize;
};
class CFX_FileWriteStreamImp : public CFX_StreamImp {
 public:
  CFX_FileWriteStreamImp();
  virtual ~CFX_FileWriteStreamImp() {}
  FX_BOOL LoadFileWrite(IFX_FileWrite* pFileWrite, FX_DWORD dwAccess);
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition() { return m_iPosition; }
  virtual FX_BOOL IsEOF() const;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize) { return 0; }
  virtual int32_t ReadString(FX_WCHAR* pStr,
                             int32_t iMaxLength,
                             FX_BOOL& bEOS) {
    return 0;
  }
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength);
  virtual void Flush();
  virtual FX_BOOL SetLength(int32_t iLength) { return FALSE; }

 protected:
  IFX_FileWrite* m_pFileWrite;
  int32_t m_iPosition;
};
enum FX_STREAMTYPE {
  FX_SREAMTYPE_Unknown = 0,
  FX_STREAMTYPE_File,
  FX_STREAMTYPE_Buffer,
  FX_STREAMTYPE_Stream,
  FX_STREAMTYPE_BufferRead,
};
class CFX_Stream : public IFX_Stream, public CFX_ThreadLock {
 public:
  CFX_Stream();
  ~CFX_Stream();
  FX_BOOL LoadFile(const FX_WCHAR* pszSrcFileName, FX_DWORD dwAccess);
  FX_BOOL LoadBuffer(uint8_t* pData, int32_t iTotalSize, FX_DWORD dwAccess);
  FX_BOOL LoadFileRead(IFX_FileRead* pFileRead, FX_DWORD dwAccess);
  FX_BOOL LoadFileWrite(IFX_FileWrite* pFileWrite, FX_DWORD dwAccess);
  FX_BOOL LoadBufferRead(IFX_BufferRead* pBufferRead,
                         int32_t iFileSize,
                         FX_DWORD dwAccess,
                         FX_BOOL bReleaseBufferRead);
  virtual void Release();
  virtual IFX_Stream* Retain();
  virtual FX_DWORD GetAccessModes() const { return m_dwAccess; }
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition();
  virtual FX_BOOL IsEOF() const;
  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr,
                             int32_t iMaxLength,
                             FX_BOOL& bEOS,
                             int32_t const* pByteSize = NULL);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength);
  virtual void Flush();
  virtual FX_BOOL SetLength(int32_t iLength);
  virtual int32_t GetBOM(uint8_t bom[4]) const;
  virtual FX_WORD GetCodePage() const;
  virtual FX_WORD SetCodePage(FX_WORD wCodePage);
  virtual void Lock() { CFX_ThreadLock::Lock(); }
  virtual void Unlock() { CFX_ThreadLock::Unlock(); }
  virtual IFX_Stream* CreateSharedStream(FX_DWORD dwAccess,
                                         int32_t iOffset,
                                         int32_t iLength);

 protected:
  FX_STREAMTYPE m_eStreamType;
  CFX_StreamImp* m_pStreamImp;
  FX_DWORD m_dwAccess;
  int32_t m_iTotalSize;
  int32_t m_iPosition;
  int32_t m_iStart;
  int32_t m_iLength;
  int32_t m_iRefCount;
};
class CFX_TextStream : public IFX_Stream, public CFX_ThreadLock {
 public:
  CFX_TextStream(IFX_Stream* pStream, FX_BOOL bDelStream);
  ~CFX_TextStream();
  virtual void Release();
  virtual IFX_Stream* Retain();

  virtual FX_DWORD GetAccessModes() const;
  virtual int32_t GetLength() const;
  virtual int32_t Seek(FX_STREAMSEEK eSeek, int32_t iOffset);
  virtual int32_t GetPosition();
  virtual FX_BOOL IsEOF() const;

  virtual int32_t ReadData(uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t ReadString(FX_WCHAR* pStr,
                             int32_t iMaxLength,
                             FX_BOOL& bEOS,
                             int32_t const* pByteSize = NULL);
  virtual int32_t WriteData(const uint8_t* pBuffer, int32_t iBufferSize);
  virtual int32_t WriteString(const FX_WCHAR* pStr, int32_t iLength);
  virtual void Flush();
  virtual FX_BOOL SetLength(int32_t iLength);

  virtual int32_t GetBOM(uint8_t bom[4]) const;
  virtual FX_WORD GetCodePage() const;
  virtual FX_WORD SetCodePage(FX_WORD wCodePage);

  virtual void Lock() { CFX_ThreadLock::Lock(); }
  virtual void Unlock() { CFX_ThreadLock::Unlock(); }

  virtual IFX_Stream* CreateSharedStream(FX_DWORD dwAccess,
                                         int32_t iOffset,
                                         int32_t iLength);

 protected:
  FX_WORD m_wCodePage;
  int32_t m_wBOMLength;
  FX_DWORD m_dwBOM;
  uint8_t* m_pBuf;
  int32_t m_iBufSize;
  FX_BOOL m_bDelStream;
  IFX_Stream* m_pStreamImp;
  int32_t m_iRefCount;
  void InitStream();
};

class CFGAS_FileRead : public IFX_FileRead {
 public:
  CFGAS_FileRead(IFX_Stream* pStream, FX_BOOL bReleaseStream);
  virtual ~CFGAS_FileRead();
  virtual void Release() { delete this; }
  virtual FX_FILESIZE GetSize();
  virtual FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);

 protected:
  FX_BOOL m_bReleaseStream;
  IFX_Stream* m_pStream;
};

class CFX_BufferAccImp : public IFX_FileRead {
 public:
  CFX_BufferAccImp(IFX_BufferRead* pBufferRead,
                   FX_FILESIZE iFileSize,
                   FX_BOOL bReleaseStream);
  virtual ~CFX_BufferAccImp();
  virtual void Release() { delete this; }
  virtual FX_FILESIZE GetSize();
  virtual FX_BOOL ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);

 protected:
  IFX_BufferRead* m_pBufferRead;
  FX_BOOL m_bReleaseStream;
  FX_FILESIZE m_iBufSize;
};

class CFGAS_FileWrite : public IFX_FileWrite {
 public:
  CFGAS_FileWrite(IFX_Stream* pStream, FX_BOOL bReleaseStream);
  virtual ~CFGAS_FileWrite();
  virtual void Release() { delete this; }
  virtual FX_FILESIZE GetSize();
  virtual FX_BOOL Flush();
  virtual FX_BOOL WriteBlock(const void* pData, size_t size);
  virtual FX_BOOL WriteBlock(const void* pData,
                             FX_FILESIZE offset,
                             size_t size);

 protected:
  IFX_Stream* m_pStream;
  FX_BOOL m_bReleaseStream;
};

#endif
