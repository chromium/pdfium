// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_STREAM
#define _FX_STREAM
class IFX_Stream;
IFX_FileRead*	FX_CreateFileRead(IFX_Stream *pBaseStream, FX_BOOL bReleaseStream = FALSE);
#ifdef FX_FILESIZE
IFX_FileRead*	FX_CreateFileRead(IFX_BufferRead *pBufferRead, FX_FILESIZE iFileSize = -1, FX_BOOL bReleaseStream = TRUE);
#else
IFX_FileRead*	FX_CreateFileRead(IFX_BufferRead *pBufferRead, FX_INT32 iFileSize = -1, FX_BOOL bReleaseStream = TRUE);
#endif
IFX_FileWrite*	FX_CreateFileWrite(IFX_Stream *pBaseStream, FX_BOOL bReleaseStream = FALSE);
enum FX_STREAMACCESS {
    FX_STREAMACCESS_Binary		= 0x00,
    FX_STREAMACCESS_Text		= 0x01,
    FX_STREAMACCESS_Read		= 0x02,
    FX_STREAMACCESS_Write		= 0x04,
    FX_STREAMACCESS_Truncate	= 0x10,
    FX_STREAMACCESS_Append		= 0x20,
    FX_STREAMACCESS_Create		= 0x80,
};
enum FX_STREAMSEEK {
    FX_STREAMSEEK_Begin			= 0 ,
    FX_STREAMSEEK_Current			,
    FX_STREAMSEEK_End				,
};
class IFX_Stream
{
public:
    static IFX_Stream*			CreateStream(IFX_FileRead *pFileRead, FX_DWORD dwAccess);
    static IFX_Stream*			CreateStream(IFX_FileWrite *pFileWrite, FX_DWORD dwAccess);
    static IFX_Stream*			CreateStream(FX_LPCWSTR pszFileName, FX_DWORD dwAccess);
    static IFX_Stream*			CreateStream(FX_LPBYTE pData, FX_INT32 length, FX_DWORD dwAccess);
    static IFX_Stream*			CreateStream(IFX_BufferRead *pBufferRead, FX_DWORD dwAccess, FX_INT32 iFileSize = -1, FX_BOOL bReleaseBufferRead = TRUE);
    static IFX_Stream*			CreateTextStream(IFX_Stream *pBaseStream, FX_BOOL bDeleteOnRelease);
    virtual void				Release() = 0;
    virtual IFX_Stream*			Retain() = 0;
    virtual FX_DWORD			GetAccessModes() const = 0;
    virtual FX_INT32			GetLength() const = 0;
    virtual FX_INT32			Seek(FX_STREAMSEEK eSeek, FX_INT32 iOffset) = 0;
    virtual FX_INT32			GetPosition() = 0;
    virtual FX_BOOL				IsEOF() const = 0;
    virtual FX_INT32			ReadData(FX_LPBYTE pBuffer, FX_INT32 iBufferSize) = 0;
    virtual FX_INT32			ReadString(FX_LPWSTR pStr, FX_INT32 iMaxLength, FX_BOOL &bEOS, FX_INT32 const *pByteSize = NULL) = 0;
    virtual FX_INT32			WriteData(FX_LPCBYTE pBuffer, FX_INT32 iBufferSize) = 0;
    virtual FX_INT32			WriteString(FX_LPCWSTR pStr, FX_INT32 iLength) = 0;
    virtual void				Flush() = 0;
    virtual FX_BOOL				SetLength(FX_INT32 iLength) = 0;
    virtual FX_INT32			GetBOM(FX_BYTE bom[4]) const = 0;
    virtual FX_WORD				GetCodePage() const = 0;
    virtual FX_WORD				SetCodePage(FX_WORD wCodePage) = 0;
    virtual void				Lock() = 0;
    virtual void				Unlock() = 0;
    virtual IFX_Stream*			CreateSharedStream(FX_DWORD dwAccess, FX_INT32 iOffset, FX_INT32 iLength) = 0;
};
#endif
