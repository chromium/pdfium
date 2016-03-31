// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fgas/crt/fgas_system.h"

#include <algorithm>

#include "core/fxcrt/include/fx_system.h"

#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN32_MOBILE_ || \
    _FX_OS_ == _FX_WIN64_
#include <io.h>
#elif _FX_OS_ == _FX_LINUX_DESKTOP_ || _FX_OS_ == _FX_LINUX_Mini_
#include <sys/times.h>
#endif

namespace {

inline FX_BOOL FX_isupper(int32_t ch) {
  return ch >= 'A' && ch <= 'Z';
}

inline int32_t FX_tolower(int32_t ch) {
  return FX_isupper(ch) ? (ch + 0x20) : ch;
}

}  // namespace

int32_t FX_wcsnicmp(const FX_WCHAR* s1, const FX_WCHAR* s2, size_t count) {
  FXSYS_assert(s1 != NULL && s2 != NULL && count > 0);
  FX_WCHAR wch1 = 0;
  FX_WCHAR wch2 = 0;
  while (count-- > 0) {
    wch1 = (FX_WCHAR)FX_tolower(*s1++);
    wch2 = (FX_WCHAR)FX_tolower(*s2++);
    if (wch1 != wch2) {
      break;
    }
  }
  return wch1 - wch2;
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
  uint32_t dwPos = ::SetFilePointer(hFile, 0, 0, FILE_CURRENT);
  ::SetFilePointer(hFile, size, 0, FILE_BEGIN);
  FX_BOOL bRet = ::SetEndOfFile(hFile);
  ::SetFilePointer(hFile, (int32_t)dwPos, 0, FILE_BEGIN);
  return bRet;
#else
  return FALSE;
#endif
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
