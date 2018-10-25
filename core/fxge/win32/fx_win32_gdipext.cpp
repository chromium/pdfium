// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <windows.h>

#include <algorithm>
#include <memory>
#include <sstream>

#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_gemodule.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/win32/cfx_windowsdib.h"
#include "core/fxge/win32/win32_int.h"

// Has to come before gdiplus.h
namespace Gdiplus {
using std::min;
using std::max;
}  // namespace Gdiplus

#include <gdiplus.h>  // NOLINT

// TODO(thestig): Remove the infrequently used ones.
using Gdiplus::CombineMode;
using Gdiplus::DashCap;
using Gdiplus::DashCapFlat;
using Gdiplus::DashCapRound;
using Gdiplus::FillModeAlternate;
using Gdiplus::FillModeWinding;
using Gdiplus::GdiplusStartupInput;
using Gdiplus::GdiplusStartupOutput;
using Gdiplus::GpBitmap;
using Gdiplus::GpBrush;
using Gdiplus::GpDashCap;
using Gdiplus::GpFillMode;
using Gdiplus::GpGraphics;
using Gdiplus::GpImage;
using Gdiplus::GpLineCap;
using Gdiplus::GpLineJoin;
using Gdiplus::GpMatrix;
using Gdiplus::GpPath;
using Gdiplus::GpPen;
using Gdiplus::GpPoint;
using Gdiplus::GpPointF;
using Gdiplus::GpRect;
using Gdiplus::GpRegion;
using Gdiplus::GpSolidFill;
using Gdiplus::GpStatus;
using Gdiplus::GpUnit;
using Gdiplus::ImageLockModeRead;
using Gdiplus::InterpolationMode;
using Gdiplus::InterpolationModeBilinear;
using Gdiplus::InterpolationModeHighQuality;
using Gdiplus::InterpolationModeNearestNeighbor;
using Gdiplus::LineCap;
using Gdiplus::LineCapFlat;
using Gdiplus::LineCapRound;
using Gdiplus::LineCapSquare;
using Gdiplus::LineJoin;
using Gdiplus::LineJoinBevel;
using Gdiplus::LineJoinMiterClipped;
using Gdiplus::LineJoinRound;
using Gdiplus::PathPointTypeBezier;
using Gdiplus::PathPointTypeCloseSubpath;
using Gdiplus::PathPointTypeLine;
using Gdiplus::PathPointTypeStart;
using Gdiplus::PixelOffsetMode;
using Gdiplus::PixelOffsetModeHalf;
using Gdiplus::REAL;
using Gdiplus::SmoothingMode;
using Gdiplus::SmoothingModeAntiAlias;
using Gdiplus::SmoothingModeNone;
using Gdiplus::UnitPixel;
using Gdiplus::UnitWorld;

#define GdiFillType2Gdip(fill_type) \
  (fill_type == ALTERNATE ? FillModeAlternate : FillModeWinding)

namespace {

enum {
  FuncId_GdipCreatePath2,
  FuncId_GdipSetPenDashArray,
  FuncId_GdipSetPenLineJoin,
  FuncId_GdipCreateFromHDC,
  FuncId_GdipSetPageUnit,
  FuncId_GdipSetSmoothingMode,
  FuncId_GdipCreateSolidFill,
  FuncId_GdipFillPath,
  FuncId_GdipDeleteBrush,
  FuncId_GdipCreatePen1,
  FuncId_GdipSetPenMiterLimit,
  FuncId_GdipDrawPath,
  FuncId_GdipDeletePen,
  FuncId_GdipDeletePath,
  FuncId_GdipDeleteGraphics,
  FuncId_GdipCreateBitmapFromFileICM,
  FuncId_GdipCreateBitmapFromStreamICM,
  FuncId_GdipGetImageHeight,
  FuncId_GdipGetImageWidth,
  FuncId_GdipGetImagePixelFormat,
  FuncId_GdipBitmapLockBits,
  FuncId_GdipGetImagePaletteSize,
  FuncId_GdipGetImagePalette,
  FuncId_GdipBitmapUnlockBits,
  FuncId_GdipDisposeImage,
  FuncId_GdipCreateBitmapFromScan0,
  FuncId_GdipSetImagePalette,
  FuncId_GdipSetInterpolationMode,
  FuncId_GdipDrawImagePointsI,
  FuncId_GdiplusStartup,
  FuncId_GdipDrawLineI,
  FuncId_GdipCreatePath,
  FuncId_GdipSetPathFillMode,
  FuncId_GdipSetClipRegion,
  FuncId_GdipWidenPath,
  FuncId_GdipAddPathLine,
  FuncId_GdipAddPathRectangle,
  FuncId_GdipDeleteRegion,
  FuncId_GdipSetPenLineCap197819,
  FuncId_GdipSetPenDashOffset,
  FuncId_GdipCreateMatrix2,
  FuncId_GdipDeleteMatrix,
  FuncId_GdipSetWorldTransform,
  FuncId_GdipSetPixelOffsetMode,
};

LPCSTR g_GdipFuncNames[] = {
    "GdipCreatePath2",
    "GdipSetPenDashArray",
    "GdipSetPenLineJoin",
    "GdipCreateFromHDC",
    "GdipSetPageUnit",
    "GdipSetSmoothingMode",
    "GdipCreateSolidFill",
    "GdipFillPath",
    "GdipDeleteBrush",
    "GdipCreatePen1",
    "GdipSetPenMiterLimit",
    "GdipDrawPath",
    "GdipDeletePen",
    "GdipDeletePath",
    "GdipDeleteGraphics",
    "GdipCreateBitmapFromFileICM",
    "GdipCreateBitmapFromStreamICM",
    "GdipGetImageHeight",
    "GdipGetImageWidth",
    "GdipGetImagePixelFormat",
    "GdipBitmapLockBits",
    "GdipGetImagePaletteSize",
    "GdipGetImagePalette",
    "GdipBitmapUnlockBits",
    "GdipDisposeImage",
    "GdipCreateBitmapFromScan0",
    "GdipSetImagePalette",
    "GdipSetInterpolationMode",
    "GdipDrawImagePointsI",
    "GdiplusStartup",
    "GdipDrawLineI",
    "GdipCreatePath",
    "GdipSetPathFillMode",
    "GdipSetClipRegion",
    "GdipWidenPath",
    "GdipAddPathLine",
    "GdipAddPathRectangle",
    "GdipDeleteRegion",
    "GdipSetPenLineCap197819",
    "GdipSetPenDashOffset",
    "GdipCreateMatrix2",
    "GdipDeleteMatrix",
    "GdipSetWorldTransform",
    "GdipSetPixelOffsetMode",
};
static_assert(FX_ArraySize(g_GdipFuncNames) ==
                  static_cast<size_t>(FuncId_GdipSetPixelOffsetMode) + 1,
              "g_GdipFuncNames has wrong size");

typedef GpStatus(WINGDIPAPI* FuncType_GdipCreatePath2)(GDIPCONST GpPointF*,
                                                       GDIPCONST BYTE*,
                                                       INT,
                                                       GpFillMode,
                                                       GpPath** path);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPenDashArray)(GpPen* pen,
                                                           GDIPCONST REAL* dash,
                                                           INT count);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPenLineJoin)(GpPen* pen,
                                                          GpLineJoin lineJoin);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateFromHDC)(HDC hdc,
                                                         GpGraphics** graphics);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPageUnit)(GpGraphics* graphics,
                                                       GpUnit unit);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetSmoothingMode)(
    GpGraphics* graphics,
    SmoothingMode smoothingMode);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateSolidFill)(Gdiplus::ARGB color,
                                                           GpSolidFill** brush);
typedef GpStatus(WINGDIPAPI* FuncType_GdipFillPath)(GpGraphics* graphics,
                                                    GpBrush* brush,
                                                    GpPath* path);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeleteBrush)(GpBrush* brush);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreatePen1)(Gdiplus::ARGB color,
                                                      REAL width,
                                                      GpUnit unit,
                                                      GpPen** pen);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPenMiterLimit)(GpPen* pen,
                                                            REAL miterLimit);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDrawPath)(GpGraphics* graphics,
                                                    GpPen* pen,
                                                    GpPath* path);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeletePen)(GpPen* pen);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeletePath)(GpPath* path);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeleteGraphics)(GpGraphics* graphics);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateBitmapFromFileICM)(
    GDIPCONST WCHAR* filename,
    GpBitmap** bitmap);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateBitmapFromStreamICM)(
    IStream* stream,
    GpBitmap** bitmap);
typedef GpStatus(WINGDIPAPI* FuncType_GdipGetImageWidth)(GpImage* image,
                                                         UINT* width);
typedef GpStatus(WINGDIPAPI* FuncType_GdipGetImageHeight)(GpImage* image,
                                                          UINT* height);
typedef GpStatus(WINGDIPAPI* FuncType_GdipGetImagePixelFormat)(
    GpImage* image,
    Gdiplus::PixelFormat* format);
typedef GpStatus(WINGDIPAPI* FuncType_GdipBitmapLockBits)(
    GpBitmap* bitmap,
    GDIPCONST GpRect* rect,
    UINT flags,
    Gdiplus::PixelFormat format,
    Gdiplus::BitmapData* lockedBitmapData);
typedef GpStatus(WINGDIPAPI* FuncType_GdipGetImagePalette)(
    GpImage* image,
    Gdiplus::ColorPalette* palette,
    INT size);
typedef GpStatus(WINGDIPAPI* FuncType_GdipGetImagePaletteSize)(GpImage* image,
                                                               INT* size);
typedef GpStatus(WINGDIPAPI* FuncType_GdipBitmapUnlockBits)(
    GpBitmap* bitmap,
    Gdiplus::BitmapData* lockedBitmapData);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDisposeImage)(GpImage* image);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateBitmapFromScan0)(
    INT width,
    INT height,
    INT stride,
    Gdiplus::PixelFormat format,
    BYTE* scan0,
    GpBitmap** bitmap);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetImagePalette)(
    GpImage* image,
    GDIPCONST Gdiplus::ColorPalette* palette);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetInterpolationMode)(
    GpGraphics* graphics,
    InterpolationMode interpolationMode);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDrawImagePointsI)(
    GpGraphics* graphics,
    GpImage* image,
    GDIPCONST GpPoint* dstpoints,
    INT count);
typedef Gdiplus::Status(WINAPI* FuncType_GdiplusStartup)(
    OUT uintptr_t* token,
    const GdiplusStartupInput* input,
    OUT GdiplusStartupOutput* output);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDrawLineI)(GpGraphics* graphics,
                                                     GpPen* pen,
                                                     int x1,
                                                     int y1,
                                                     int x2,
                                                     int y2);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreatePath)(GpFillMode brushMode,
                                                      GpPath** path);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPathFillMode)(GpPath* path,
                                                           GpFillMode fillmode);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetClipRegion)(
    GpGraphics* graphics,
    GpRegion* region,
    CombineMode combineMode);
typedef GpStatus(WINGDIPAPI* FuncType_GdipWidenPath)(GpPath* nativePath,
                                                     GpPen* pen,
                                                     GpMatrix* matrix,
                                                     REAL flatness);
typedef GpStatus(WINGDIPAPI* FuncType_GdipAddPathLine)(GpPath* path,
                                                       REAL x1,
                                                       REAL y1,
                                                       REAL x2,
                                                       REAL y2);
typedef GpStatus(WINGDIPAPI* FuncType_GdipAddPathRectangle)(GpPath* path,
                                                            REAL x,
                                                            REAL y,
                                                            REAL width,
                                                            REAL height);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeleteRegion)(GpRegion* region);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPenLineCap197819)(
    GpPen* pen,
    GpLineCap startCap,
    GpLineCap endCap,
    GpDashCap dashCap);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPenDashOffset)(GpPen* pen,
                                                            REAL offset);
typedef GpStatus(WINGDIPAPI* FuncType_GdipCreateMatrix2)(REAL m11,
                                                         REAL m12,
                                                         REAL m21,
                                                         REAL m22,
                                                         REAL dx,
                                                         REAL dy,
                                                         GpMatrix** matrix);
typedef GpStatus(WINGDIPAPI* FuncType_GdipDeleteMatrix)(GpMatrix* matrix);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetWorldTransform)(
    GpGraphics* graphics,
    GpMatrix* matrix);
typedef GpStatus(WINGDIPAPI* FuncType_GdipSetPixelOffsetMode)(
    GpGraphics* graphics,
    PixelOffsetMode pixelOffsetMode);
#define CallFunc(funcname) \
  ((FuncType_##funcname)GdiplusExt.m_Functions[FuncId_##funcname])

GpBrush* GdipCreateBrushImpl(DWORD argb) {
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  GpSolidFill* solidBrush = nullptr;
  CallFunc(GdipCreateSolidFill)((Gdiplus::ARGB)argb, &solidBrush);
  return solidBrush;
}

void OutputImage(GpGraphics* pGraphics,
                 const RetainPtr<CFX_DIBitmap>& pBitmap,
                 const FX_RECT* pSrcRect,
                 int dest_left,
                 int dest_top,
                 int dest_width,
                 int dest_height) {
  int src_width = pSrcRect->Width(), src_height = pSrcRect->Height();
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  if (pBitmap->GetBPP() == 1 && (pSrcRect->left % 8)) {
    FX_RECT new_rect(0, 0, src_width, src_height);
    RetainPtr<CFX_DIBitmap> pCloned = pBitmap->Clone(pSrcRect);
    if (!pCloned)
      return;
    OutputImage(pGraphics, pCloned, &new_rect, dest_left, dest_top, dest_width,
                dest_height);
    return;
  }
  int src_pitch = pBitmap->GetPitch();
  uint8_t* scan0 = pBitmap->GetBuffer() + pSrcRect->top * src_pitch +
                   pBitmap->GetBPP() * pSrcRect->left / 8;
  GpBitmap* bitmap = nullptr;
  switch (pBitmap->GetFormat()) {
    case FXDIB_Argb:
      CallFunc(GdipCreateBitmapFromScan0)(src_width, src_height, src_pitch,
                                          PixelFormat32bppARGB, scan0, &bitmap);
      break;
    case FXDIB_Rgb32:
      CallFunc(GdipCreateBitmapFromScan0)(src_width, src_height, src_pitch,
                                          PixelFormat32bppRGB, scan0, &bitmap);
      break;
    case FXDIB_Rgb:
      CallFunc(GdipCreateBitmapFromScan0)(src_width, src_height, src_pitch,
                                          PixelFormat24bppRGB, scan0, &bitmap);
      break;
    case FXDIB_8bppRgb: {
      CallFunc(GdipCreateBitmapFromScan0)(src_width, src_height, src_pitch,
                                          PixelFormat8bppIndexed, scan0,
                                          &bitmap);
      UINT pal[258];
      pal[0] = 0;
      pal[1] = 256;
      for (int i = 0; i < 256; i++)
        pal[i + 2] = pBitmap->GetPaletteArgb(i);
      CallFunc(GdipSetImagePalette)(bitmap, (Gdiplus::ColorPalette*)pal);
      break;
    }
    case FXDIB_1bppRgb: {
      CallFunc(GdipCreateBitmapFromScan0)(src_width, src_height, src_pitch,
                                          PixelFormat1bppIndexed, scan0,
                                          &bitmap);
      break;
    }
  }
  if (dest_height < 0) {
    dest_height--;
  }
  if (dest_width < 0) {
    dest_width--;
  }
  Gdiplus::Point destinationPoints[] = {
      Gdiplus::Point(dest_left, dest_top),
      Gdiplus::Point(dest_left + dest_width, dest_top),
      Gdiplus::Point(dest_left, dest_top + dest_height)};
  CallFunc(GdipDrawImagePointsI)(pGraphics, bitmap, destinationPoints, 3);
  CallFunc(GdipDisposeImage)(bitmap);
}

GpPen* GdipCreatePenImpl(const CFX_GraphStateData* pGraphState,
                         const CFX_Matrix* pMatrix,
                         DWORD argb,
                         bool bTextMode) {
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  float width = pGraphState->m_LineWidth;
  if (!bTextMode) {
    float unit = pMatrix
                     ? 1.0f / ((pMatrix->GetXUnit() + pMatrix->GetYUnit()) / 2)
                     : 1.0f;
    width = std::max(width, unit);
  }
  GpPen* pPen = nullptr;
  CallFunc(GdipCreatePen1)((Gdiplus::ARGB)argb, width, UnitWorld, &pPen);
  LineCap lineCap = LineCapFlat;
  DashCap dashCap = DashCapFlat;
  bool bDashExtend = false;
  switch (pGraphState->m_LineCap) {
    case CFX_GraphStateData::LineCapButt:
      lineCap = LineCapFlat;
      break;
    case CFX_GraphStateData::LineCapRound:
      lineCap = LineCapRound;
      dashCap = DashCapRound;
      bDashExtend = true;
      break;
    case CFX_GraphStateData::LineCapSquare:
      lineCap = LineCapSquare;
      bDashExtend = true;
      break;
  }
  CallFunc(GdipSetPenLineCap197819)(pPen, lineCap, lineCap, dashCap);
  LineJoin lineJoin = LineJoinMiterClipped;
  switch (pGraphState->m_LineJoin) {
    case CFX_GraphStateData::LineJoinMiter:
      lineJoin = LineJoinMiterClipped;
      break;
    case CFX_GraphStateData::LineJoinRound:
      lineJoin = LineJoinRound;
      break;
    case CFX_GraphStateData::LineJoinBevel:
      lineJoin = LineJoinBevel;
      break;
  }
  CallFunc(GdipSetPenLineJoin)(pPen, lineJoin);
  if (!pGraphState->m_DashArray.empty()) {
    float* pDashArray =
        FX_Alloc(float, FxAlignToBoundary<2>(pGraphState->m_DashArray.size()));
    int nCount = 0;
    float on_leftover = 0, off_leftover = 0;
    for (size_t i = 0; i < pGraphState->m_DashArray.size(); i += 2) {
      float on_phase = pGraphState->m_DashArray[i];
      float off_phase;
      if (i == pGraphState->m_DashArray.size() - 1)
        off_phase = on_phase;
      else
        off_phase = pGraphState->m_DashArray[i + 1];
      on_phase /= width;
      off_phase /= width;
      if (on_phase + off_phase <= 0.00002f) {
        on_phase = 1.0f / 10;
        off_phase = 1.0f / 10;
      }
      if (bDashExtend) {
        if (off_phase < 1)
          off_phase = 0;
        else
          off_phase -= 1;
        on_phase += 1;
      }
      if (on_phase == 0 || off_phase == 0) {
        if (nCount == 0) {
          on_leftover += on_phase;
          off_leftover += off_phase;
        } else {
          pDashArray[nCount - 2] += on_phase;
          pDashArray[nCount - 1] += off_phase;
        }
      } else {
        pDashArray[nCount++] = on_phase + on_leftover;
        on_leftover = 0;
        pDashArray[nCount++] = off_phase + off_leftover;
        off_leftover = 0;
      }
    }
    CallFunc(GdipSetPenDashArray)(pPen, pDashArray, nCount);
    float phase = pGraphState->m_DashPhase;
    if (bDashExtend) {
      if (phase < 0.5f)
        phase = 0;
      else
        phase -= 0.5f;
    }
    CallFunc(GdipSetPenDashOffset)(pPen, phase);
    FX_Free(pDashArray);
    pDashArray = nullptr;
  }
  CallFunc(GdipSetPenMiterLimit)(pPen, pGraphState->m_MiterLimit);
  return pPen;
}

bool IsSmallTriangle(Gdiplus::PointF* points,
                     const CFX_Matrix* pMatrix,
                     int& v1,
                     int& v2) {
  int pairs[] = {1, 2, 0, 2, 0, 1};
  for (int i = 0; i < 3; i++) {
    int pair1 = pairs[i * 2];
    int pair2 = pairs[i * 2 + 1];

    CFX_PointF p1(points[pair1].X, points[pair1].Y);
    CFX_PointF p2(points[pair2].X, points[pair2].Y);
    if (pMatrix) {
      p1 = pMatrix->Transform(p1);
      p2 = pMatrix->Transform(p2);
    }

    CFX_PointF diff = p1 - p2;
    float distance_square = (diff.x * diff.x) + (diff.y * diff.y);
    if (distance_square < (1.0f * 2 + 1.0f / 4)) {
      v1 = i;
      v2 = pair1;
      return true;
    }
  }
  return false;
}

class GpStream final : public IStream {
 public:
  GpStream() : m_RefCount(1), m_ReadPos(0) {}
  ~GpStream() = default;

  // IUnknown
  HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,
                                           void** ppvObject) override {
    if (iid == __uuidof(IUnknown) || iid == __uuidof(IStream) ||
        iid == __uuidof(ISequentialStream)) {
      *ppvObject = static_cast<IStream*>(this);
      AddRef();
      return S_OK;
    }
    return E_NOINTERFACE;
  }
  ULONG STDMETHODCALLTYPE AddRef() override {
    return (ULONG)InterlockedIncrement(&m_RefCount);
  }
  ULONG STDMETHODCALLTYPE Release() override {
    ULONG res = (ULONG)InterlockedDecrement(&m_RefCount);
    if (res == 0) {
      delete this;
    }
    return res;
  }

  // ISequentialStream
  HRESULT STDMETHODCALLTYPE Read(void* output,
                                 ULONG cb,
                                 ULONG* pcbRead) override {
    if (pcbRead)
      *pcbRead = 0;

    if (m_ReadPos >= m_InterStream.tellp())
      return HRESULT_FROM_WIN32(ERROR_END_OF_MEDIA);

    size_t bytes_left = m_InterStream.tellp() - m_ReadPos;
    size_t bytes_out =
        std::min(pdfium::base::checked_cast<size_t>(cb), bytes_left);
    memcpy(output, m_InterStream.str().c_str() + m_ReadPos, bytes_out);
    m_ReadPos += bytes_out;
    if (pcbRead)
      *pcbRead = (ULONG)bytes_out;

    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Write(const void* input,
                                  ULONG cb,
                                  ULONG* pcbWritten) override {
    if (cb <= 0) {
      if (pcbWritten)
        *pcbWritten = 0;
      return S_OK;
    }
    m_InterStream.write(reinterpret_cast<const char*>(input), cb);
    if (pcbWritten)
      *pcbWritten = cb;
    return S_OK;
  }

  // IStream
  HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE CopyTo(IStream*,
                                   ULARGE_INTEGER,
                                   ULARGE_INTEGER*,
                                   ULARGE_INTEGER*) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE Commit(DWORD) override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE Revert() override { return E_NOTIMPL; }
  HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER,
                                       ULARGE_INTEGER,
                                       DWORD) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER,
                                         ULARGE_INTEGER,
                                         DWORD) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE Clone(IStream** stream) override {
    return E_NOTIMPL;
  }
  HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove,
                                 DWORD dwOrigin,
                                 ULARGE_INTEGER* lpNewFilePointer) override {
    std::streamoff start;
    std::streamoff new_read_position;
    switch (dwOrigin) {
      case STREAM_SEEK_SET:
        start = 0;
        break;
      case STREAM_SEEK_CUR:
        start = m_ReadPos;
        break;
      case STREAM_SEEK_END:
        if (m_InterStream.tellp() < 0)
          return STG_E_SEEKERROR;
        start = m_InterStream.tellp();
        break;
      default:
        return STG_E_INVALIDFUNCTION;
    }
    new_read_position = start + liDistanceToMove.QuadPart;
    if (new_read_position > m_InterStream.tellp())
      return STG_E_SEEKERROR;

    m_ReadPos = new_read_position;
    if (lpNewFilePointer)
      lpNewFilePointer->QuadPart = m_ReadPos;

    return S_OK;
  }
  HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg,
                                 DWORD grfStatFlag) override {
    if (!pStatstg)
      return STG_E_INVALIDFUNCTION;

    ZeroMemory(pStatstg, sizeof(STATSTG));

    if (m_InterStream.tellp() < 0)
      return STG_E_SEEKERROR;

    pStatstg->cbSize.QuadPart = m_InterStream.tellp();
    return S_OK;
  }

 private:
  LONG m_RefCount;
  std::streamoff m_ReadPos;
  std::ostringstream m_InterStream;
};

struct PREVIEW3_DIBITMAP {
  BITMAPINFO* pbmi;
  int Stride;
  LPBYTE pScan0;
  GpBitmap* pBitmap;
  Gdiplus::BitmapData* pBitmapData;
  GpStream* pStream;
};

PREVIEW3_DIBITMAP* LoadDIBitmap(WINDIB_Open_Args_ args) {
  GpBitmap* pBitmap;
  GpStream* pStream = nullptr;
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  Gdiplus::Status status = Gdiplus::Ok;
  if (args.flags == WINDIB_OPEN_PATHNAME) {
    status = CallFunc(GdipCreateBitmapFromFileICM)((wchar_t*)args.path_name,
                                                   &pBitmap);
  } else {
    if (args.memory_size == 0 || !args.memory_base)
      return nullptr;

    pStream = new GpStream;
    pStream->Write(args.memory_base, (ULONG)args.memory_size, nullptr);
    status = CallFunc(GdipCreateBitmapFromStreamICM)(pStream, &pBitmap);
  }
  if (status != Gdiplus::Ok) {
    if (pStream)
      pStream->Release();

    return nullptr;
  }
  UINT height, width;
  CallFunc(GdipGetImageHeight)(pBitmap, &height);
  CallFunc(GdipGetImageWidth)(pBitmap, &width);
  Gdiplus::PixelFormat pixel_format;
  CallFunc(GdipGetImagePixelFormat)(pBitmap, &pixel_format);
  int info_size = sizeof(BITMAPINFOHEADER);
  int bpp = 24;
  int dest_pixel_format = PixelFormat24bppRGB;
  if (pixel_format == PixelFormat1bppIndexed) {
    info_size += 8;
    bpp = 1;
    dest_pixel_format = PixelFormat1bppIndexed;
  } else if (pixel_format == PixelFormat8bppIndexed) {
    info_size += 1024;
    bpp = 8;
    dest_pixel_format = PixelFormat8bppIndexed;
  } else if (pixel_format == PixelFormat32bppARGB) {
    bpp = 32;
    dest_pixel_format = PixelFormat32bppARGB;
  }
  LPBYTE buf = FX_Alloc(BYTE, info_size);
  BITMAPINFOHEADER* pbmih = (BITMAPINFOHEADER*)buf;
  pbmih->biBitCount = bpp;
  pbmih->biCompression = BI_RGB;
  pbmih->biHeight = -(int)height;
  pbmih->biPlanes = 1;
  pbmih->biWidth = width;
  Gdiplus::Rect rect(0, 0, width, height);
  Gdiplus::BitmapData* pBitmapData = FX_Alloc(Gdiplus::BitmapData, 1);
  CallFunc(GdipBitmapLockBits)(pBitmap, &rect, ImageLockModeRead,
                               dest_pixel_format, pBitmapData);
  if (pixel_format == PixelFormat1bppIndexed ||
      pixel_format == PixelFormat8bppIndexed) {
    DWORD* ppal = (DWORD*)(buf + sizeof(BITMAPINFOHEADER));
    struct {
      UINT flags;
      UINT Count;
      DWORD Entries[256];
    } pal;
    int size = 0;
    CallFunc(GdipGetImagePaletteSize)(pBitmap, &size);
    CallFunc(GdipGetImagePalette)(pBitmap, (Gdiplus::ColorPalette*)&pal, size);
    int entries = pixel_format == PixelFormat1bppIndexed ? 2 : 256;
    for (int i = 0; i < entries; i++) {
      ppal[i] = pal.Entries[i] & 0x00ffffff;
    }
  }
  PREVIEW3_DIBITMAP* pInfo = FX_Alloc(PREVIEW3_DIBITMAP, 1);
  pInfo->pbmi = (BITMAPINFO*)buf;
  pInfo->pScan0 = (LPBYTE)pBitmapData->Scan0;
  pInfo->Stride = pBitmapData->Stride;
  pInfo->pBitmap = pBitmap;
  pInfo->pBitmapData = pBitmapData;
  pInfo->pStream = pStream;
  return pInfo;
}

void FreeDIBitmap(PREVIEW3_DIBITMAP* pInfo) {
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  CallFunc(GdipBitmapUnlockBits)(pInfo->pBitmap, pInfo->pBitmapData);
  CallFunc(GdipDisposeImage)(pInfo->pBitmap);
  FX_Free(pInfo->pBitmapData);
  FX_Free((LPBYTE)pInfo->pbmi);
  if (pInfo->pStream)
    pInfo->pStream->Release();
  FX_Free(pInfo);
}

}  // namespace

CGdiplusExt::CGdiplusExt() {}

CGdiplusExt::~CGdiplusExt() {
  FreeLibrary(m_GdiModule);
  FreeLibrary(m_hModule);
}

void CGdiplusExt::Load() {
  char buf[MAX_PATH];
  GetSystemDirectoryA(buf, MAX_PATH);
  ByteString dllpath = buf;
  dllpath += "\\GDIPLUS.DLL";
  m_hModule = LoadLibraryA(dllpath.c_str());
  if (!m_hModule)
    return;

  m_Functions.resize(FX_ArraySize(g_GdipFuncNames));
  for (size_t i = 0; i < FX_ArraySize(g_GdipFuncNames); ++i) {
    m_Functions[i] = GetProcAddress(m_hModule, g_GdipFuncNames[i]);
    if (!m_Functions[i]) {
      m_hModule = nullptr;
      return;
    }
  }

  uintptr_t gdiplusToken;
  GdiplusStartupInput gdiplusStartupInput;
  ((FuncType_GdiplusStartup)m_Functions[FuncId_GdiplusStartup])(
      &gdiplusToken, &gdiplusStartupInput, nullptr);
  m_GdiModule = LoadLibraryA("GDI32.DLL");
}

bool CGdiplusExt::StretchDIBits(HDC hDC,
                                const RetainPtr<CFX_DIBitmap>& pBitmap,
                                int dest_left,
                                int dest_top,
                                int dest_width,
                                int dest_height,
                                const FX_RECT* pClipRect,
                                int flags) {
  GpGraphics* pGraphics;
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  CallFunc(GdipCreateFromHDC)(hDC, &pGraphics);
  CallFunc(GdipSetPageUnit)(pGraphics, UnitPixel);
  if (flags & FXDIB_NOSMOOTH) {
    CallFunc(GdipSetInterpolationMode)(pGraphics,
                                       InterpolationModeNearestNeighbor);
  } else if (pBitmap->GetWidth() > abs(dest_width) / 2 ||
             pBitmap->GetHeight() > abs(dest_height) / 2) {
    CallFunc(GdipSetInterpolationMode)(pGraphics, InterpolationModeHighQuality);
  } else {
    CallFunc(GdipSetInterpolationMode)(pGraphics, InterpolationModeBilinear);
  }
  FX_RECT src_rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
  OutputImage(pGraphics, pBitmap, &src_rect, dest_left, dest_top, dest_width,
              dest_height);
  CallFunc(GdipDeleteGraphics)(pGraphics);
  CallFunc(GdipDeleteGraphics)(pGraphics);
  return true;
}

bool CGdiplusExt::DrawPath(HDC hDC,
                           const CFX_PathData* pPathData,
                           const CFX_Matrix* pObject2Device,
                           const CFX_GraphStateData* pGraphState,
                           uint32_t fill_argb,
                           uint32_t stroke_argb,
                           int fill_mode) {
  auto& pPoints = pPathData->GetPoints();
  if (pPoints.empty())
    return true;

  GpGraphics* pGraphics = nullptr;
  CGdiplusExt& GdiplusExt =
      ((CWin32Platform*)CFX_GEModule::Get()->GetPlatformData())->m_GdiplusExt;
  CallFunc(GdipCreateFromHDC)(hDC, &pGraphics);
  CallFunc(GdipSetPageUnit)(pGraphics, UnitPixel);
  CallFunc(GdipSetPixelOffsetMode)(pGraphics, PixelOffsetModeHalf);
  GpMatrix* pMatrix = nullptr;
  if (pObject2Device) {
    CallFunc(GdipCreateMatrix2)(pObject2Device->a, pObject2Device->b,
                                pObject2Device->c, pObject2Device->d,
                                pObject2Device->e, pObject2Device->f, &pMatrix);
    CallFunc(GdipSetWorldTransform)(pGraphics, pMatrix);
  }
  Gdiplus::PointF* points = FX_Alloc(Gdiplus::PointF, pPoints.size());
  BYTE* types = FX_Alloc(BYTE, pPoints.size());
  int nSubPathes = 0;
  bool bSubClose = false;
  int pos_subclose = 0;
  bool bSmooth = false;
  int startpoint = 0;
  for (size_t i = 0; i < pPoints.size(); i++) {
    points[i].X = pPoints[i].m_Point.x;
    points[i].Y = pPoints[i].m_Point.y;

    CFX_PointF pos = pPoints[i].m_Point;
    if (pObject2Device)
      pos = pObject2Device->Transform(pos);

    if (pos.x > 50000 * 1.0f)
      points[i].X = 50000 * 1.0f;
    if (pos.x < -50000 * 1.0f)
      points[i].X = -50000 * 1.0f;
    if (pos.y > 50000 * 1.0f)
      points[i].Y = 50000 * 1.0f;
    if (pos.y < -50000 * 1.0f)
      points[i].Y = -50000 * 1.0f;

    FXPT_TYPE point_type = pPoints[i].m_Type;
    if (point_type == FXPT_TYPE::MoveTo) {
      types[i] = PathPointTypeStart;
      nSubPathes++;
      bSubClose = false;
      startpoint = i;
    } else if (point_type == FXPT_TYPE::LineTo) {
      types[i] = PathPointTypeLine;
      if (pPoints[i - 1].IsTypeAndOpen(FXPT_TYPE::MoveTo) &&
          (i == pPoints.size() - 1 ||
           pPoints[i + 1].IsTypeAndOpen(FXPT_TYPE::MoveTo)) &&
          points[i].Y == points[i - 1].Y && points[i].X == points[i - 1].X) {
        points[i].X += 0.01f;
        continue;
      }
      if (!bSmooth && points[i].X != points[i - 1].X &&
          points[i].Y != points[i - 1].Y)
        bSmooth = true;
    } else if (point_type == FXPT_TYPE::BezierTo) {
      types[i] = PathPointTypeBezier;
      bSmooth = true;
    }
    if (pPoints[i].m_CloseFigure) {
      if (bSubClose)
        types[pos_subclose] &= ~PathPointTypeCloseSubpath;
      else
        bSubClose = true;
      pos_subclose = i;
      types[i] |= PathPointTypeCloseSubpath;
      if (!bSmooth && points[i].X != points[startpoint].X &&
          points[i].Y != points[startpoint].Y)
        bSmooth = true;
    }
  }
  if (fill_mode & FXFILL_NOPATHSMOOTH) {
    bSmooth = false;
    CallFunc(GdipSetSmoothingMode)(pGraphics, SmoothingModeNone);
  } else if (!(fill_mode & FXFILL_FULLCOVER)) {
    if (!bSmooth && (fill_mode & 3))
      bSmooth = true;

    if (bSmooth || (pGraphState && pGraphState->m_LineWidth > 2))
      CallFunc(GdipSetSmoothingMode)(pGraphics, SmoothingModeAntiAlias);
  }
  int new_fill_mode = fill_mode & 3;
  if (pPoints.size() == 4 && !pGraphState) {
    int v1, v2;
    if (IsSmallTriangle(points, pObject2Device, v1, v2)) {
      GpPen* pPen = nullptr;
      CallFunc(GdipCreatePen1)(fill_argb, 1.0f, UnitPixel, &pPen);
      CallFunc(GdipDrawLineI)(
          pGraphics, pPen, FXSYS_round(points[v1].X), FXSYS_round(points[v1].Y),
          FXSYS_round(points[v2].X), FXSYS_round(points[v2].Y));
      CallFunc(GdipDeletePen)(pPen);
      return true;
    }
  }
  GpPath* pGpPath = nullptr;
  CallFunc(GdipCreatePath2)(points, types, pPoints.size(),
                            GdiFillType2Gdip(new_fill_mode), &pGpPath);
  if (!pGpPath) {
    if (pMatrix)
      CallFunc(GdipDeleteMatrix)(pMatrix);

    FX_Free(points);
    FX_Free(types);
    CallFunc(GdipDeleteGraphics)(pGraphics);
    return false;
  }
  if (new_fill_mode) {
    GpBrush* pBrush = GdipCreateBrushImpl(fill_argb);
    CallFunc(GdipSetPathFillMode)(pGpPath, GdiFillType2Gdip(new_fill_mode));
    CallFunc(GdipFillPath)(pGraphics, pBrush, pGpPath);
    CallFunc(GdipDeleteBrush)(pBrush);
  }
  if (pGraphState && stroke_argb) {
    GpPen* pPen = GdipCreatePenImpl(pGraphState, pObject2Device, stroke_argb,
                                    !!(fill_mode & FX_STROKE_TEXT_MODE));
    if (nSubPathes == 1) {
      CallFunc(GdipDrawPath)(pGraphics, pPen, pGpPath);
    } else {
      int iStart = 0;
      for (size_t i = 0; i < pPoints.size(); i++) {
        if (i == pPoints.size() - 1 || types[i + 1] == PathPointTypeStart) {
          GpPath* pSubPath;
          CallFunc(GdipCreatePath2)(points + iStart, types + iStart,
                                    i - iStart + 1,
                                    GdiFillType2Gdip(new_fill_mode), &pSubPath);
          iStart = i + 1;
          CallFunc(GdipDrawPath)(pGraphics, pPen, pSubPath);
          CallFunc(GdipDeletePath)(pSubPath);
        }
      }
    }
    CallFunc(GdipDeletePen)(pPen);
  }
  if (pMatrix)
    CallFunc(GdipDeleteMatrix)(pMatrix);
  FX_Free(points);
  FX_Free(types);
  CallFunc(GdipDeletePath)(pGpPath);
  CallFunc(GdipDeleteGraphics)(pGraphics);
  return true;
}

RetainPtr<CFX_DIBitmap> CGdiplusExt::LoadDIBitmap(WINDIB_Open_Args_ args) {
  PREVIEW3_DIBITMAP* pInfo = ::LoadDIBitmap(args);
  if (!pInfo)
    return nullptr;

  int height = abs(pInfo->pbmi->bmiHeader.biHeight);
  int width = pInfo->pbmi->bmiHeader.biWidth;
  int dest_pitch = (width * pInfo->pbmi->bmiHeader.biBitCount + 31) / 32 * 4;
  LPBYTE pData = FX_Alloc2D(BYTE, dest_pitch, height);
  if (dest_pitch == pInfo->Stride) {
    memcpy(pData, pInfo->pScan0, dest_pitch * height);
  } else {
    for (int i = 0; i < height; i++) {
      memcpy(pData + dest_pitch * i, pInfo->pScan0 + pInfo->Stride * i,
             dest_pitch);
    }
  }
  RetainPtr<CFX_DIBitmap> pDIBitmap = FX_WindowsDIB_LoadFromBuf(
      pInfo->pbmi, pData, pInfo->pbmi->bmiHeader.biBitCount == 32);
  FX_Free(pData);
  FreeDIBitmap(pInfo);
  return pDIBitmap;
}
