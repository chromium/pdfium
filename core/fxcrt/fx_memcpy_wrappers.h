// Copyright 2023 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_FX_MEMCPY_WRAPPERS_H_
#define CORE_FXCRT_FX_MEMCPY_WRAPPERS_H_

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

// Wrappers to avoid the zero-length w/NULL arg gotchas in C spec. Use these
// if there is a possibility of a NULL arg (or a bad arg) that is to be ignored
// when the length is zero, otherwise just call the C Run Time Library function
// itself.
inline int FXSYS_memcmp(const void* ptr1, const void* ptr2, size_t len) {
  return len ? memcmp(ptr1, ptr2, len) : 0;
}

inline int FXSYS_wmemcmp(const wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return len ? wmemcmp(ptr1, ptr2, len) : 0;
}

inline void* FXSYS_memcpy(void* ptr1, const void* ptr2, size_t len) {
  return len ? memcpy(ptr1, ptr2, len) : ptr1;
}

inline wchar_t* FXSYS_wmemcpy(wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return len ? wmemcpy(ptr1, ptr2, len) : ptr1;
}

inline void* FXSYS_memmove(void* ptr1, const void* ptr2, size_t len) {
  return len ? memmove(ptr1, ptr2, len) : ptr1;
}

inline wchar_t* FXSYS_wmemmove(wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return len ? wmemmove(ptr1, ptr2, len) : ptr1;
}

inline void* FXSYS_memset(void* ptr1, int val, size_t len) {
  return len ? memset(ptr1, val, len) : ptr1;
}

inline wchar_t* FXSYS_wmemset(wchar_t* ptr1, int val, size_t len) {
  return len ? wmemset(ptr1, val, len) : ptr1;
}

inline const void* FXSYS_memchr(const void* ptr1, int val, size_t len) {
  return len ? memchr(ptr1, val, len) : nullptr;
}

inline const wchar_t* FXSYS_wmemchr(const wchar_t* ptr1,
                                    wchar_t val,
                                    size_t len) {
  return len ? wmemchr(ptr1, val, len) : nullptr;
}

// Overloaded functions for C++ templates
inline size_t FXSYS_len(const char* ptr) {
  return strlen(ptr);
}

inline size_t FXSYS_len(const wchar_t* ptr) {
  return wcslen(ptr);
}

inline int FXSYS_cmp(const char* ptr1, const char* ptr2, size_t len) {
  return FXSYS_memcmp(ptr1, ptr2, len);
}

inline int FXSYS_cmp(const wchar_t* ptr1, const wchar_t* ptr2, size_t len) {
  return FXSYS_wmemcmp(ptr1, ptr2, len);
}

inline const char* FXSYS_chr(const char* ptr, char ch, size_t len) {
  return reinterpret_cast<const char*>(FXSYS_memchr(ptr, ch, len));
}

inline const wchar_t* FXSYS_chr(const wchar_t* ptr, wchar_t ch, size_t len) {
  return FXSYS_wmemchr(ptr, ch, len);
}

#endif  // CORE_FXCRT_FX_MEMCPY_WRAPPERS_H_
