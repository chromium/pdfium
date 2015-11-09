// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_FX_SYSTEMHANDLER_H_
#define FPDFSDK_INCLUDE_FX_SYSTEMHANDLER_H_

#include "core/include/fpdfapi/fpdf_parser.h"
#include "core/include/fxcrt/fx_coordinates.h"
#include "core/include/fxcrt/fx_system.h"

class CPDF_Document;
class CPDF_Font;

typedef void* FX_HWND;
typedef void* FX_HMENU;
typedef void (*TimerCallback)(int32_t idEvent);

typedef struct _FX_SYSTEMTIME {
  _FX_SYSTEMTIME()
      : wYear(0),
        wMonth(0),
        wDayOfWeek(0),
        wDay(0),
        wHour(0),
        wMinute(0),
        wSecond(0),
        wMilliseconds(0) {}
  FX_WORD wYear;
  FX_WORD wMonth;
  FX_WORD wDayOfWeek;
  FX_WORD wDay;
  FX_WORD wHour;
  FX_WORD wMinute;
  FX_WORD wSecond;
  FX_WORD wMilliseconds;
} FX_SYSTEMTIME;

// cursor style
#define FXCT_ARROW 0
#define FXCT_NESW 1
#define FXCT_NWSE 2
#define FXCT_VBEAM 3
#define FXCT_HBEAM 4
#define FXCT_HAND 5

class IFX_SystemHandler {
 public:
  virtual ~IFX_SystemHandler() {}
  virtual void InvalidateRect(FX_HWND hWnd, FX_RECT rect) = 0;
  virtual void OutputSelectedRect(void* pFormFiller, CPDF_Rect& rect) = 0;

  virtual FX_BOOL IsSelectionImplemented() = 0;

  virtual CFX_WideString GetClipboardText(FX_HWND hWnd) = 0;
  virtual FX_BOOL SetClipboardText(FX_HWND hWnd, CFX_WideString string) = 0;

  virtual void ClientToScreen(FX_HWND hWnd, int32_t& x, int32_t& y) = 0;
  virtual void ScreenToClient(FX_HWND hWnd, int32_t& x, int32_t& y) = 0;

  /*cursor style
  FXCT_ARROW
  FXCT_NESW
  FXCT_NWSE
  FXCT_VBEAM
  FXCT_HBEAM
  FXCT_HAND
  */
  virtual void SetCursor(int32_t nCursorType) = 0;

  virtual FX_HMENU CreatePopupMenu() = 0;
  virtual FX_BOOL AppendMenuItem(FX_HMENU hMenu,
                                 int32_t nIDNewItem,
                                 CFX_WideString string) = 0;
  virtual FX_BOOL EnableMenuItem(FX_HMENU hMenu,
                                 int32_t nIDItem,
                                 FX_BOOL bEnabled) = 0;
  virtual int32_t TrackPopupMenu(FX_HMENU hMenu,
                                 int32_t x,
                                 int32_t y,
                                 FX_HWND hParent) = 0;
  virtual void DestroyMenu(FX_HMENU hMenu) = 0;

  virtual CFX_ByteString GetNativeTrueTypeFont(int32_t nCharset) = 0;
  virtual FX_BOOL FindNativeTrueTypeFont(int32_t nCharset,
                                         CFX_ByteString sFontFaceName) = 0;
  virtual CPDF_Font* AddNativeTrueTypeFontToPDF(CPDF_Document* pDoc,
                                                CFX_ByteString sFontFaceName,
                                                uint8_t nCharset) = 0;

  virtual int32_t SetTimer(int32_t uElapse, TimerCallback lpTimerFunc) = 0;
  virtual void KillTimer(int32_t nID) = 0;

  virtual FX_BOOL IsSHIFTKeyDown(FX_DWORD nFlag) = 0;
  virtual FX_BOOL IsCTRLKeyDown(FX_DWORD nFlag) = 0;
  virtual FX_BOOL IsALTKeyDown(FX_DWORD nFlag) = 0;
  virtual FX_BOOL IsINSERTKeyDown(FX_DWORD nFlag) = 0;

  virtual FX_SYSTEMTIME GetLocalTime() = 0;

  virtual int32_t GetCharSet() = 0;
  virtual void SetCharSet(int32_t nCharSet) = 0;
};

#endif  // FPDFSDK_INCLUDE_FX_SYSTEMHANDLER_H_
