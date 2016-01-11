// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <algorithm>

#include "xfa/src/fgas/src/fgas_base.h"
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
#include <io.h>
#elif _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_LINUX_Mini_
#include <sys/times.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
FX_FLOAT FX_tan(FX_FLOAT a) {
  return (FX_FLOAT)tan(a);
}
FX_FLOAT FX_log(FX_FLOAT b, FX_FLOAT x) {
  return FXSYS_log(x) / FXSYS_log(b);
}
FX_WCHAR* FX_wcsncpy(FX_WCHAR* dstStr, const FX_WCHAR* srcStr, size_t count) {
  FXSYS_assert(dstStr != NULL && srcStr != NULL && count > 0);
  for (size_t i = 0; i < count; ++i)
    if ((dstStr[i] = srcStr[i]) == L'\0') {
      break;
    }
  return dstStr;
}
int32_t FX_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count) {
  FXSYS_assert(s1 != NULL && s2 != NULL && count > 0);
  FX_WCHAR wch1 = 0, wch2 = 0;
  while (count-- > 0) {
    wch1 = (FX_WCHAR)FX_tolower(*s1++);
    wch2 = (FX_WCHAR)FX_tolower(*s2++);
    if (wch1 != wch2) {
      break;
    }
  }
  return wch1 - wch2;
}
int32_t FX_strnicmp(const FX_CHAR* s1, const FX_CHAR* s2, size_t count) {
  FXSYS_assert(s1 != NULL && s2 != NULL && count > 0);
  FX_CHAR ch1 = 0, ch2 = 0;
  while (count-- > 0) {
    ch1 = (FX_CHAR)FX_tolower(*s1++);
    ch2 = (FX_CHAR)FX_tolower(*s2++);
    if (ch1 != ch2) {
      break;
    }
  }
  return ch1 - ch2;
}
int32_t FX_filelength(FXSYS_FILE* file) {
  FXSYS_assert(file != NULL);
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_
  return _filelength(_fileno(file));
#else
  int32_t iPos = FXSYS_ftell(file);
  FXSYS_fseek(file, 0, FXSYS_SEEK_END);
  int32_t iLen = FXSYS_ftell(file);
  FXSYS_fseek(file, iPos, FXSYS_SEEK_SET);
  return iLen;
#endif
}
FX_BOOL FX_fsetsize(FXSYS_FILE* file, int32_t size) {
  FXSYS_assert(file != NULL);
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_
  return _chsize(_fileno(file), size) == 0;
#elif _FX_OS_ == _FX_WIN32_MOBILE_
  HANDLE hFile = _fileno(file);
  FX_DWORD dwPos = ::SetFilePointer(hFile, 0, 0, FILE_CURRENT);
  ::SetFilePointer(hFile, size, 0, FILE_BEGIN);
  FX_BOOL bRet = ::SetEndOfFile(hFile);
  ::SetFilePointer(hFile, (int32_t)dwPos, 0, FILE_BEGIN);
  return bRet;
#else
  return FALSE;
#endif
}
FX_FLOAT FX_strtof(const FX_CHAR* pcsStr, int32_t iLength, int32_t* pUsedLen) {
  FXSYS_assert(pcsStr != NULL);
  if (iLength < 0) {
    iLength = FXSYS_strlen(pcsStr);
  }
  return FX_wcstof(CFX_WideString::FromLocal(pcsStr, iLength), iLength,
                   pUsedLen);
}
FX_FLOAT FX_wcstof(const FX_WCHAR* pwsStr, int32_t iLength, int32_t* pUsedLen) {
  FXSYS_assert(pwsStr != NULL);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pwsStr);
  }
  if (iLength == 0) {
    return 0.0f;
  }
  int32_t iUsedLen = 0;
  FX_BOOL bNegtive = FALSE;
  switch (pwsStr[iUsedLen]) {
    case '-':
      bNegtive = TRUE;
    case '+':
      iUsedLen++;
      break;
  }
  FX_FLOAT fValue = 0.0f;
  while (iUsedLen < iLength) {
    FX_WCHAR wch = pwsStr[iUsedLen];
    if (wch >= L'0' && wch <= L'9') {
      fValue = fValue * 10.0f + (wch - L'0');
    } else {
      break;
    }
    iUsedLen++;
  }
  if (iUsedLen < iLength && pwsStr[iUsedLen] == L'.') {
    FX_FLOAT fPrecise = 0.1f;
    while (++iUsedLen < iLength) {
      FX_WCHAR wch = pwsStr[iUsedLen];
      if (wch >= L'0' && wch <= L'9') {
        fValue += (wch - L'0') * fPrecise;
        fPrecise *= 0.1f;
      } else {
        break;
      }
    }
  }
  if (pUsedLen) {
    *pUsedLen = iUsedLen;
  }
  return bNegtive ? -fValue : fValue;
}
void FX_memset(void* pBuf, int32_t iValue, size_t size) {
  FXSYS_assert(pBuf != NULL && size > 0 && (size & 0x03) == 0);
  FXSYS_assert((((size_t)pBuf) & 0x03) == 0);
  FX_DWORD* pStart = (FX_DWORD*)pBuf;
  FX_DWORD* pEnd = pStart + (size >> 2);
  while (pStart < pEnd) {
    *pStart++ = iValue;
  }
}
void FX_memcpy(void* pDst, const void* pSrc, size_t size) {
  FXSYS_assert(pDst != NULL && pSrc != NULL && size > 0 && (size & 0x03) == 0);
  FXSYS_assert((((size_t)pDst) & 0x03) == 0 && (((size_t)pSrc) & 0x03) == 0);
  FX_DWORD* pStart = (FX_DWORD*)pDst;
  FX_DWORD* pEnd = pStart + (size >> 2);
  FX_DWORD* pValue = (FX_DWORD*)pSrc;
  while (pStart < pEnd) {
    *pStart++ = *pValue++;
  }
}
FX_BOOL FX_IsRelativePath(const CFX_WideStringC& wsUrl) {
  int32_t iUrlLen = wsUrl.GetLength();
  if (iUrlLen == 0) {
    return TRUE;
  }
  for (int32_t i = std::min(5, iUrlLen) - 1; i >= 0; --i)
    if (wsUrl.GetAt(i) == ':') {
      return FALSE;
    }
  return TRUE;
}
FX_BOOL FX_JoinPath(const CFX_WideStringC& wsBasePath,
                    const CFX_WideStringC& wsRelativePath,
                    CFX_WideString& wsAbsolutePath) {
  if (!FX_IsRelativePath(wsRelativePath)) {
    wsAbsolutePath = wsRelativePath;
    return TRUE;
  }
  const FX_WCHAR* pRelStart = wsRelativePath.GetPtr();
  const FX_WCHAR* pRelEnd = pRelStart + wsRelativePath.GetLength();
  if (pRelStart < pRelEnd) {
    switch (*pRelStart) {
      case '#':
        wsAbsolutePath = CFX_WideString(wsBasePath, wsRelativePath);
        return wsAbsolutePath.GetLength() > 0;
      case '/':
      case '\\':
        wsAbsolutePath = wsRelativePath;
        return wsAbsolutePath.GetLength() > 0;
    }
  }
  int32_t nBackCount = 0;
  for (;;) {
    if (pRelStart >= pRelEnd) {
      wsAbsolutePath = wsBasePath;
      return TRUE;
    }
    if (*pRelStart != '.') {
      break;
    }
    if (pRelStart + 1 < pRelEnd &&
        (pRelStart[1] == '/' || pRelStart[1] == '\\')) {
      pRelStart += 2;
    } else if (pRelStart + 2 < pRelEnd && pRelStart[1] == '.' &&
               (pRelStart[2] == '/' || pRelStart[2] == '\\')) {
      pRelStart += 3;
      nBackCount++;
    } else {
      return FALSE;
    }
  }
  const FX_WCHAR* pBaseStart = wsBasePath.GetPtr();
  const FX_WCHAR* pBaseEnd = pBaseStart + wsBasePath.GetLength();
  while (pBaseStart < (--pBaseEnd) && *pBaseEnd != '/' && *pBaseEnd != '\\')
    ;
  if (pBaseStart == pBaseEnd) {
    wsAbsolutePath = CFX_WideStringC(pRelStart, pRelEnd - pRelStart);
    return wsAbsolutePath.GetLength() > 0;
  }
  while (nBackCount > 0) {
    if (pBaseStart >= (--pBaseEnd)) {
      return FALSE;
    } else if (*pBaseEnd == '/' || *pBaseEnd == '\\')
      if ((--nBackCount) <= 0) {
        break;
      }
  }
  wsAbsolutePath =
      CFX_WideString(CFX_WideStringC(pBaseStart, pBaseEnd - pBaseStart + 1),
                     CFX_WideStringC(pRelStart, pRelEnd - pRelStart));
  return wsAbsolutePath.GetLength() > 0;
}
#ifdef __cplusplus
};
#endif
