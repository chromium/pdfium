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
class CFX_StreamImp : public CFX_ThreadLock, public CFX_Object
{
public:
    virtual void			Release()
    {
        delete this;
    }
    virtual FX_DWORD		GetAccessModes() const
    {
        return m_dwAccess;
    }
    virtual FX_INT32		GetLength() const = 0;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset) = 0;
    virtual FX_INT32		GetPosition() = 0;
    virtual FX_BOOL			IsEOF() const = 0;
    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize) = 0;
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS) = 0;
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize) = 0;
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength) = 0;
    virtual void			Flush() = 0;
    virtual FX_BOOL			SetLength(FX_INT32 iLength) = 0;
protected:
    CFX_StreamImp();
    virtual ~CFX_StreamImp() {}
    FX_DWORD				m_dwAccess;
};
class CFX_FileStreamImp : public CFX_StreamImp
{
public:
    CFX_FileStreamImp();
    virtual ~CFX_FileStreamImp();
    FX_BOOL			LoadFile(FX_LPCWSTR pszSrcFileName, FX_DWORD dwAccess);
    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition();
    virtual FX_BOOL			IsEOF() const;
    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS);
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength);
    virtual void			Flush();
    virtual FX_BOOL			SetLength(FX_INT32 iLength);
protected:
    FXSYS_FILE	*m_hFile;
    FX_INT32	m_iLength;
};
class CFX_BufferStreamImp : public CFX_StreamImp
{
public:
    CFX_BufferStreamImp();
    virtual ~CFX_BufferStreamImp() {}
    FX_BOOL			LoadBuffer(FX_LPBYTE pData, FX_INT32 iTotalSize, FX_DWORD dwAccess);
    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition();
    virtual FX_BOOL			IsEOF() const;
    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS);
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength);
    virtual void			Flush() {}
    virtual FX_BOOL			SetLength(FX_INT32 iLength)
    {
        return FALSE;
    }
protected:
    FX_LPBYTE	m_pData;
    FX_INT32	m_iTotalSize;
    FX_INT32	m_iPosition;
    FX_INT32	m_iLength;
};
class CFX_FileReadStreamImp : public CFX_StreamImp
{
public:
    CFX_FileReadStreamImp();
    virtual ~CFX_FileReadStreamImp() {}
    FX_BOOL			LoadFileRead(IFX_FileRead *pFileRead, FX_DWORD dwAccess);
    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition()
    {
        return m_iPosition;
    }
    virtual FX_BOOL			IsEOF() const;

    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS);
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize)
    {
        return 0;
    }
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength)
    {
        return 0;
    }
    virtual void			Flush() {}
    virtual FX_BOOL			SetLength(FX_INT32 iLength)
    {
        return FALSE;
    }
protected:
    IFX_FileRead			*m_pFileRead;
    FX_INT32				m_iPosition;
    FX_INT32				m_iLength;
};
class CFX_BufferReadStreamImp : public CFX_StreamImp
{
public:
    CFX_BufferReadStreamImp();
    ~CFX_BufferReadStreamImp();
    FX_BOOL			LoadBufferRead(IFX_BufferRead *pBufferRead, FX_INT32 iFileSize, FX_DWORD dwAccess, FX_BOOL bReleaseBufferRead);

    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition()
    {
        return m_iPosition;
    }
    virtual FX_BOOL			IsEOF() const;

    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS);
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize)
    {
        return 0;
    }
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength)
    {
        return 0;
    }
    virtual void			Flush() {}
    virtual FX_BOOL			SetLength(FX_INT32 iLength)
    {
        return FALSE;
    }
private:
    IFX_BufferRead			*m_pBufferRead;
    FX_BOOL					m_bReleaseBufferRead;
    FX_INT32				m_iPosition;
    FX_INT32				m_iBufferSize;
};
class CFX_FileWriteStreamImp : public CFX_StreamImp
{
public:
    CFX_FileWriteStreamImp();
    virtual ~CFX_FileWriteStreamImp() {}
    FX_BOOL			LoadFileWrite(IFX_FileWrite *pFileWrite, FX_DWORD dwAccess);
    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition()
    {
        return m_iPosition;
    }
    virtual FX_BOOL			IsEOF() const;
    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize)
    {
        return 0;
    }
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS)
    {
        return 0;
    }
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength);
    virtual void			Flush();
    virtual FX_BOOL			SetLength(FX_INT32 iLength)
    {
        return FALSE;
    }
protected:
    IFX_FileWrite			*m_pFileWrite;
    FX_INT32				m_iPosition;
};
enum FX_STREAMTYPE {
    FX_SREAMTYPE_Unknown	=  0,
    FX_STREAMTYPE_File			,
    FX_STREAMTYPE_Buffer		,
    FX_STREAMTYPE_Stream		,
    FX_STREAMTYPE_BufferRead	,
};
class CFX_Stream : public IFX_Stream, public CFX_ThreadLock, public CFX_Object
{
public:
    CFX_Stream();
    ~CFX_Stream();
    FX_BOOL			LoadFile(FX_LPCWSTR pszSrcFileName, FX_DWORD dwAccess);
    FX_BOOL			LoadBuffer(FX_LPBYTE pData, FX_INT32 iTotalSize, FX_DWORD dwAccess);
    FX_BOOL			LoadFileRead(IFX_FileRead *pFileRead, FX_DWORD dwAccess);
    FX_BOOL			LoadFileWrite(IFX_FileWrite *pFileWrite, FX_DWORD dwAccess);
    FX_BOOL			LoadBufferRead(IFX_BufferRead *pBufferRead, FX_INT32 iFileSize, FX_DWORD dwAccess, FX_BOOL bReleaseBufferRead);
    virtual void			Release();
    virtual IFX_Stream*		Retain();
    virtual FX_DWORD		GetAccessModes() const
    {
        return m_dwAccess;
    }
    virtual FX_INT32		GetLength() const;
    virtual FX_INT32		Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32		GetPosition();
    virtual FX_BOOL			IsEOF() const;
    virtual FX_INT32		ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS, FX_INT32 const *pByteSize = NULL);
    virtual FX_INT32		WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32		WriteString(FX_LPCWSTR pStr, FX_INT32 iLength);
    virtual void			Flush();
    virtual FX_BOOL			SetLength(FX_INT32 iLength);
    virtual FX_INT32		GetBOM(FX_BYTE bom[4]) const;
    virtual FX_WORD			GetCodePage() const;
    virtual FX_WORD			SetCodePage(FX_WORD wCodePage);
    virtual void			Lock()
    {
        CFX_ThreadLock::Lock();
    }
    virtual void			Unlock()
    {
        CFX_ThreadLock::Unlock();
    }
    virtual IFX_Stream*		CreateSharedStream(FX_DWORD dwAccess, FX_INT32 iOffset, FX_INT32 iLength);
protected:
    FX_STREAMTYPE			m_eStreamType;
    CFX_StreamImp			*m_pStreamImp;
    FX_DWORD				m_dwAccess;
    FX_INT32				m_iTotalSize;
    FX_INT32				m_iPosition;
    FX_INT32				m_iStart;
    FX_INT32				m_iLength;
    FX_INT32				m_iRefCount;
};
class CFX_TextStream : public IFX_Stream, public CFX_ThreadLock, public CFX_Object
{
public:
    CFX_TextStream(IFX_Stream *pStream, FX_BOOL bDelStream);
    ~CFX_TextStream();
    virtual void				Release();
    virtual IFX_Stream*			Retain();

    virtual FX_DWORD			GetAccessModes() const;
    virtual FX_INT32			GetLength() const;
    virtual FX_INT32			Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset);
    virtual FX_INT32			GetPosition();
    virtual FX_BOOL				IsEOF() const;

    virtual FX_INT32			ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32			ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS, FX_INT32 const *pByteSize = NULL);
    virtual FX_INT32			WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize);
    virtual FX_INT32			WriteString(FX_LPCWSTR pStr, FX_INT32 iLength);
    virtual void				Flush();
    virtual FX_BOOL				SetLength(FX_INT32 iLength);

    virtual FX_INT32			GetBOM(FX_BYTE bom[4]) const;
    virtual FX_WORD				GetCodePage() const;
    virtual FX_WORD				SetCodePage(FX_WORD wCodePage);

    virtual void				Lock()
    {
        CFX_ThreadLock::Lock();
    }
    virtual void				Unlock()
    {
        CFX_ThreadLock::Unlock();
    }

    virtual IFX_Stream*			CreateSharedStream(FX_DWORD dwAccess, FX_INT32 iOffset, FX_INT32 iLength);
protected:
    FX_WORD		m_wCodePage;
    FX_WORD		m_wBOMLength;
    FX_DWORD	m_dwBOM;
    FX_LPBYTE	m_pBuf;
    FX_INT32	m_iBufSize;
    FX_BOOL		m_bDelStream;
    IFX_Stream	*m_pStreamImp;
    FX_INT32	m_iRefCount;
    void		InitStream();
};
#ifdef FX_FILESIZE
class CFGAS_FileRead : public IFX_FileRead, public CFX_Object
{
public:
    CFGAS_FileRead(IFX_Stream *pStream, FX_BOOL bReleaseStream);
    virtual ~CFGAS_FileRead();
    virtual void			Release()
    {
        delete this;
    }
    virtual FX_FILESIZE		GetSize();
    virtual FX_BOOL			ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
protected:
    FX_BOOL					m_bReleaseStream;
    IFX_Stream				*m_pStream;
};
#else
class CFGAS_FileRead : public IFX_FileRead, public CFX_Object
{
public:
    CFGAS_FileRead(IFX_Stream *pStream, FX_BOOL bReleaseStream);
    virtual ~CFGAS_FileRead();

    virtual void			Release()
    {
        delete this;
    }
    virtual FX_DWORD		GetSize();
    virtual FX_BOOL			ReadBlock(void* buffer, FX_DWORD offset, FX_DWORD size);

protected:
    FX_BOOL					m_bReleaseStream;
    IFX_Stream				*m_pStream;
};
#endif
#ifdef FX_FILESIZE
class CFX_BufferAccImp : public IFX_FileRead, public CFX_Object
{
public:
    CFX_BufferAccImp(IFX_BufferRead *pBufferRead, FX_FILESIZE iFileSize, FX_BOOL bReleaseStream);
    virtual ~CFX_BufferAccImp();
    virtual void			Release()
    {
        delete this;
    }
    virtual FX_FILESIZE		GetSize();
    virtual FX_BOOL			ReadBlock(void* buffer, FX_FILESIZE offset, size_t size);
protected:
    IFX_BufferRead			*m_pBufferRead;
    FX_BOOL					m_bReleaseStream;
    FX_FILESIZE				m_iBufSize;
};
#else
class CFX_BufferAccImp : public IFX_FileRead, public CFX_Object
{
public:
    CFX_BufferAccImp(IFX_BufferRead *pBufferRead, FX_INT32 iFileSize, FX_BOOL bReleaseStream);
    virtual ~CFX_BufferAccImp();

    virtual void			Release()
    {
        delete this;
    }
    virtual FX_DWORD		GetSize();
    virtual FX_BOOL			ReadBlock(void* buffer, FX_DWORD offset, FX_DWORD size);

protected:
    IFX_BufferRead			*m_pBufferRead;
    FX_BOOL					m_bReleaseStream;
    FX_INT32				m_iBufSize;
};
#endif
#ifdef FX_FILESIZE
class CFGAS_FileWrite : public IFX_FileWrite, public CFX_Object
{
public:
    CFGAS_FileWrite(IFX_Stream *pStream, FX_BOOL bReleaseStream);
    virtual ~CFGAS_FileWrite();
    virtual void			Release()
    {
        delete this;
    }
    virtual FX_FILESIZE		GetSize();
    virtual FX_BOOL			Flush();
    virtual	FX_BOOL			WriteBlock(const void* pData, size_t size);
    virtual	FX_BOOL			WriteBlock(const void* pData, FX_FILESIZE offset, size_t size);
protected:
    IFX_Stream				*m_pStream;
    FX_BOOL					m_bReleaseStream;
};
#else
class CFGAS_FileWrite : public IFX_FileWrite, public CFX_Object
{
public:
    CFGAS_FileWrite(IFX_Stream *pStream, FX_BOOL bReleaseStream);
    virtual ~CFGAS_FileWrite();

    virtual void			Release()
    {
        delete this;
    }
    virtual FX_DWORD		GetSize();
    virtual FX_DWORD		Flush();
    virtual	FX_BOOL			WriteBlock(const void* pData, FX_DWORD size);
    virtual	FX_BOOL			WriteBlock(const void* pData, FX_DWORD offset, FX_DWORD size);

protected:
    IFX_Stream				*m_pStream;
    FX_BOOL					m_bReleaseStream;
};
#endif
#endif
