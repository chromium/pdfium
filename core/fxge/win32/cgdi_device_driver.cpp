// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/win32/cgdi_device_driver.h"

#include <math.h>
#include <windows.h>

#include <algorithm>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/render_defines.h"
#include "core/fxge/win32/cwin32_platform.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"
#include "third_party/base/notreached.h"

#if !defined(_SKIA_SUPPORT_)
#include "core/fxge/agg/fx_agg_driver.h"
#include "third_party/agg23/agg_clip_liang_barsky.h"
#endif

namespace {

constexpr int FillTypeToGdiFillType(CFX_FillRenderOptions::FillType fill_type) {
  return static_cast<int>(fill_type);
}

static_assert(FillTypeToGdiFillType(
                  CFX_FillRenderOptions::FillType::kEvenOdd) == ALTERNATE,
              "CFX_FillRenderOptions::FillType::kEvenOdd value mismatch");

static_assert(
    FillTypeToGdiFillType(CFX_FillRenderOptions::FillType::kWinding) == WINDING,
    "CFX_FillRenderOptions::FillType::kWinding value mismatch");

HPEN CreateExtPen(const CFX_GraphStateData* pGraphState,
                  const CFX_Matrix* pMatrix,
                  uint32_t argb) {
  DCHECK(pGraphState);

  float scale = 1.0f;
  if (pMatrix) {
    scale = fabs(pMatrix->a) > fabs(pMatrix->b) ? fabs(pMatrix->a)
                                                : fabs(pMatrix->b);
  }
  float width = std::max(scale * pGraphState->m_LineWidth, 1.0f);

  uint32_t PenStyle = PS_GEOMETRIC;
  if (!pGraphState->m_DashArray.empty())
    PenStyle |= PS_USERSTYLE;
  else
    PenStyle |= PS_SOLID;

  switch (pGraphState->m_LineCap) {
    case CFX_GraphStateData::LineCapButt:
      PenStyle |= PS_ENDCAP_FLAT;
      break;
    case CFX_GraphStateData::LineCapRound:
      PenStyle |= PS_ENDCAP_ROUND;
      break;
    case CFX_GraphStateData::LineCapSquare:
      PenStyle |= PS_ENDCAP_SQUARE;
      break;
  }
  switch (pGraphState->m_LineJoin) {
    case CFX_GraphStateData::LineJoinMiter:
      PenStyle |= PS_JOIN_MITER;
      break;
    case CFX_GraphStateData::LineJoinRound:
      PenStyle |= PS_JOIN_ROUND;
      break;
    case CFX_GraphStateData::LineJoinBevel:
      PenStyle |= PS_JOIN_BEVEL;
      break;
  }

  FX_COLORREF colorref = ArgbToColorRef(argb);
  LOGBRUSH lb;
  lb.lbColor = colorref;
  lb.lbStyle = BS_SOLID;
  lb.lbHatch = 0;
  std::vector<uint32_t> dashes;
  if (!pGraphState->m_DashArray.empty()) {
    dashes.resize(pGraphState->m_DashArray.size());
    for (size_t i = 0; i < pGraphState->m_DashArray.size(); i++) {
      dashes[i] = FXSYS_roundf(
          pMatrix ? pMatrix->TransformDistance(pGraphState->m_DashArray[i])
                  : pGraphState->m_DashArray[i]);
      dashes[i] = std::max(dashes[i], 1U);
    }
  }
  return ExtCreatePen(PenStyle, (DWORD)ceil(width), &lb,
                      pGraphState->m_DashArray.size(),
                      reinterpret_cast<const DWORD*>(dashes.data()));
}

HBRUSH CreateBrush(uint32_t argb) {
  return CreateSolidBrush(ArgbToColorRef(argb));
}

void SetPathToDC(HDC hDC, const CFX_Path* pPath, const CFX_Matrix* pMatrix) {
  BeginPath(hDC);

  pdfium::span<const CFX_Path::Point> points = pPath->GetPoints();
  for (size_t i = 0; i < points.size(); ++i) {
    CFX_PointF pos = points[i].m_Point;
    if (pMatrix)
      pos = pMatrix->Transform(pos);

    CFX_Point screen(FXSYS_roundf(pos.x), FXSYS_roundf(pos.y));
    CFX_Path::Point::Type point_type = points[i].m_Type;
    if (point_type == CFX_Path::Point::Type::kMove) {
      MoveToEx(hDC, screen.x, screen.y, nullptr);
    } else if (point_type == CFX_Path::Point::Type::kLine) {
      if (points[i].m_Point == points[i - 1].m_Point)
        screen.x++;

      LineTo(hDC, screen.x, screen.y);
    } else if (point_type == CFX_Path::Point::Type::kBezier) {
      POINT lppt[3];
      lppt[0].x = screen.x;
      lppt[0].y = screen.y;

      pos = points[i + 1].m_Point;
      if (pMatrix)
        pos = pMatrix->Transform(pos);

      lppt[1].x = FXSYS_roundf(pos.x);
      lppt[1].y = FXSYS_roundf(pos.y);

      pos = points[i + 2].m_Point;
      if (pMatrix)
        pos = pMatrix->Transform(pos);

      lppt[2].x = FXSYS_roundf(pos.x);
      lppt[2].y = FXSYS_roundf(pos.y);
      PolyBezierTo(hDC, lppt, 3);
      i += 2;
    }
    if (points[i].m_CloseFigure)
      CloseFigure(hDC);
  }
  EndPath(hDC);
}

ByteString GetBitmapInfo(const RetainPtr<CFX_DIBitmap>& pBitmap) {
  int len = sizeof(BITMAPINFOHEADER);
  if (pBitmap->GetBPP() == 1 || pBitmap->GetBPP() == 8)
    len += sizeof(DWORD) * (int)(1 << pBitmap->GetBPP());

  ByteString result;
  {
    // Span's lifetime must end before ReleaseBuffer() below.
    pdfium::span<char> cspan = result.GetBuffer(len);
    BITMAPINFOHEADER* pbmih = reinterpret_cast<BITMAPINFOHEADER*>(cspan.data());
    memset(pbmih, 0, sizeof(BITMAPINFOHEADER));
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
    pbmih->biBitCount = pBitmap->GetBPP();
    pbmih->biCompression = BI_RGB;
    pbmih->biHeight = -(int)pBitmap->GetHeight();
    pbmih->biPlanes = 1;
    pbmih->biWidth = pBitmap->GetWidth();
    if (pBitmap->GetBPP() == 8) {
      uint32_t* pPalette = (uint32_t*)(pbmih + 1);
      if (pBitmap->HasPalette()) {
        pdfium::span<const uint32_t> palette = pBitmap->GetPaletteSpan();
        for (int i = 0; i < 256; i++)
          pPalette[i] = palette[i];
      } else {
        for (int i = 0; i < 256; i++)
          pPalette[i] = ArgbEncode(0, i, i, i);
      }
    }
    if (pBitmap->GetBPP() == 1) {
      uint32_t* pPalette = (uint32_t*)(pbmih + 1);
      if (pBitmap->HasPalette()) {
        pPalette[0] = pBitmap->GetPaletteSpan()[0];
        pPalette[1] = pBitmap->GetPaletteSpan()[1];
      } else {
        pPalette[0] = 0;
        pPalette[1] = 0xffffff;
      }
    }
  }
  result.ReleaseBuffer(len);
  return result;
}

#if defined(_SKIA_SUPPORT_)
// TODO(caryclark)  This antigrain function is duplicated here to permit
// removing the last remaining dependency. Eventually, this will be elminiated
// altogether and replace by Skia code.

struct rect_base {
  float x1;
  float y1;
  float x2;
  float y2;
};

unsigned clip_liang_barsky(float x1,
                           float y1,
                           float x2,
                           float y2,
                           const rect_base& clip_box,
                           float* x,
                           float* y) {
  const float nearzero = 1e-30f;
  float deltax = x2 - x1;
  float deltay = y2 - y1;
  unsigned np = 0;
  if (deltax == 0)
    deltax = (x1 > clip_box.x1) ? -nearzero : nearzero;
  float xin;
  float xout;
  if (deltax > 0) {
    xin = clip_box.x1;
    xout = clip_box.x2;
  } else {
    xin = clip_box.x2;
    xout = clip_box.x1;
  }
  float tinx = (xin - x1) / deltax;
  if (deltay == 0)
    deltay = (y1 > clip_box.y1) ? -nearzero : nearzero;
  float yin;
  float yout;
  if (deltay > 0) {
    yin = clip_box.y1;
    yout = clip_box.y2;
  } else {
    yin = clip_box.y2;
    yout = clip_box.y1;
  }
  float tiny = (yin - y1) / deltay;
  float tin1;
  float tin2;
  if (tinx < tiny) {
    tin1 = tinx;
    tin2 = tiny;
  } else {
    tin1 = tiny;
    tin2 = tinx;
  }
  if (tin1 <= 1.0f) {
    if (0 < tin1) {
      *x++ = xin;
      *y++ = yin;
      ++np;
    }
    if (tin2 <= 1.0f) {
      float toutx = (xout - x1) / deltax;
      float touty = (yout - y1) / deltay;
      float tout1 = (toutx < touty) ? toutx : touty;
      if (tin2 > 0 || tout1 > 0) {
        if (tin2 <= tout1) {
          if (tin2 > 0) {
            if (tinx > tiny) {
              *x++ = xin;
              *y++ = y1 + (deltay * tinx);
            } else {
              *x++ = x1 + (deltax * tiny);
              *y++ = yin;
            }
            ++np;
          }
          if (tout1 < 1.0f) {
            if (toutx < touty) {
              *x++ = xout;
              *y++ = y1 + (deltay * toutx);
            } else {
              *x++ = x1 + (deltax * touty);
              *y++ = yout;
            }
          } else {
            *x++ = x2;
            *y++ = y2;
          }
          ++np;
        } else {
          if (tinx > tiny) {
            *x++ = xin;
            *y++ = yout;
          } else {
            *x++ = xout;
            *y++ = yin;
          }
          ++np;
        }
      }
    }
  }
  return np;
}
#endif  //  defined(_SKIA_SUPPORT_)

}  // namespace

CGdiDeviceDriver::CGdiDeviceDriver(HDC hDC, DeviceType device_type)
    : m_hDC(hDC), m_DeviceType(device_type) {
  auto* pPlatform =
      static_cast<CWin32Platform*>(CFX_GEModule::Get()->GetPlatform());
  SetStretchBltMode(m_hDC, pPlatform->m_bHalfTone ? HALFTONE : COLORONCOLOR);
  DWORD obj_type = GetObjectType(m_hDC);
  m_bMetafileDCType = obj_type == OBJ_ENHMETADC || obj_type == OBJ_ENHMETAFILE;
  if (obj_type == OBJ_MEMDC) {
    HBITMAP hBitmap = CreateBitmap(1, 1, 1, 1, nullptr);
    hBitmap = (HBITMAP)SelectObject(m_hDC, hBitmap);
    BITMAP bitmap;
    GetObject(hBitmap, sizeof bitmap, &bitmap);
    m_nBitsPerPixel = bitmap.bmBitsPixel;
    m_Width = bitmap.bmWidth;
    m_Height = abs(bitmap.bmHeight);
    hBitmap = (HBITMAP)SelectObject(m_hDC, hBitmap);
    DeleteObject(hBitmap);
  } else {
    m_nBitsPerPixel = ::GetDeviceCaps(m_hDC, BITSPIXEL);
    m_Width = ::GetDeviceCaps(m_hDC, HORZRES);
    m_Height = ::GetDeviceCaps(m_hDC, VERTRES);
  }
  if (m_DeviceType != DeviceType::kDisplay) {
    m_RenderCaps = FXRC_BIT_MASK;
  } else {
    m_RenderCaps = FXRC_GET_BITS | FXRC_BIT_MASK;
  }
}

CGdiDeviceDriver::~CGdiDeviceDriver() = default;

DeviceType CGdiDeviceDriver::GetDeviceType() const {
  return m_DeviceType;
}

int CGdiDeviceDriver::GetDeviceCaps(int caps_id) const {
  switch (caps_id) {
    case FXDC_PIXEL_WIDTH:
      return m_Width;
    case FXDC_PIXEL_HEIGHT:
      return m_Height;
    case FXDC_BITS_PIXEL:
      return m_nBitsPerPixel;
    case FXDC_RENDER_CAPS:
      return m_RenderCaps;
    default:
      NOTREACHED();
      return 0;
  }
}

void CGdiDeviceDriver::SaveState() {
  SaveDC(m_hDC);
}

void CGdiDeviceDriver::RestoreState(bool bKeepSaved) {
  RestoreDC(m_hDC, -1);
  if (bKeepSaved)
    SaveDC(m_hDC);
}

bool CGdiDeviceDriver::GDI_SetDIBits(const RetainPtr<CFX_DIBitmap>& pBitmap1,
                                     const FX_RECT& src_rect,
                                     int left,
                                     int top) {
  if (m_DeviceType == DeviceType::kPrinter) {
    RetainPtr<CFX_DIBitmap> pBitmap = pBitmap1->FlipImage(false, true);
    if (!pBitmap)
      return false;

    LPBYTE pBuffer = pBitmap->GetBuffer();
    ByteString info = GetBitmapInfo(pBitmap);
    ((BITMAPINFOHEADER*)info.c_str())->biHeight *= -1;
    FX_RECT dst_rect(0, 0, src_rect.Width(), src_rect.Height());
    dst_rect.Intersect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
    int dst_width = dst_rect.Width();
    int dst_height = dst_rect.Height();
    ::StretchDIBits(m_hDC, left, top, dst_width, dst_height, 0, 0, dst_width,
                    dst_height, pBuffer, (BITMAPINFO*)info.c_str(),
                    DIB_RGB_COLORS, SRCCOPY);
    return true;
  }

  RetainPtr<CFX_DIBitmap> pBitmap = pBitmap1;
  LPBYTE pBuffer = pBitmap->GetBuffer();
  ByteString info = GetBitmapInfo(pBitmap);
  ::SetDIBitsToDevice(m_hDC, left, top, src_rect.Width(), src_rect.Height(),
                      src_rect.left, pBitmap->GetHeight() - src_rect.bottom, 0,
                      pBitmap->GetHeight(), pBuffer, (BITMAPINFO*)info.c_str(),
                      DIB_RGB_COLORS);
  return true;
}

bool CGdiDeviceDriver::GDI_StretchDIBits(
    const RetainPtr<CFX_DIBitmap>& pBitmap1,
    int dest_left,
    int dest_top,
    int dest_width,
    int dest_height,
    const FXDIB_ResampleOptions& options) {
  RetainPtr<CFX_DIBitmap> pBitmap = pBitmap1;
  if (!pBitmap || dest_width == 0 || dest_height == 0)
    return false;

  ByteString info = GetBitmapInfo(pBitmap);
  if ((int64_t)abs(dest_width) * abs(dest_height) <
          (int64_t)pBitmap1->GetWidth() * pBitmap1->GetHeight() * 4 ||
      options.bInterpolateBilinear) {
    SetStretchBltMode(m_hDC, HALFTONE);
  } else {
    SetStretchBltMode(m_hDC, COLORONCOLOR);
  }
  RetainPtr<CFX_DIBitmap> pToStrechBitmap = pBitmap;
  if (m_DeviceType == DeviceType::kPrinter &&
      ((int64_t)pBitmap->GetWidth() * pBitmap->GetHeight() >
       (int64_t)abs(dest_width) * abs(dest_height))) {
    pToStrechBitmap = pBitmap->StretchTo(dest_width, dest_height,
                                         FXDIB_ResampleOptions(), nullptr);
  }
  ByteString toStrechBitmapInfo = GetBitmapInfo(pToStrechBitmap);
  ::StretchDIBits(m_hDC, dest_left, dest_top, dest_width, dest_height, 0, 0,
                  pToStrechBitmap->GetWidth(), pToStrechBitmap->GetHeight(),
                  pToStrechBitmap->GetBuffer(),
                  (BITMAPINFO*)toStrechBitmapInfo.c_str(), DIB_RGB_COLORS,
                  SRCCOPY);
  return true;
}

bool CGdiDeviceDriver::GDI_StretchBitMask(
    const RetainPtr<CFX_DIBitmap>& pBitmap1,
    int dest_left,
    int dest_top,
    int dest_width,
    int dest_height,
    uint32_t bitmap_color) {
  RetainPtr<CFX_DIBitmap> pBitmap = pBitmap1;
  if (!pBitmap || dest_width == 0 || dest_height == 0)
    return false;

  int width = pBitmap->GetWidth(), height = pBitmap->GetHeight();
  struct {
    BITMAPINFOHEADER bmiHeader;
    uint32_t bmiColors[2];
  } bmi;
  memset(&bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
  bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biBitCount = 1;
  bmi.bmiHeader.biCompression = BI_RGB;
  bmi.bmiHeader.biHeight = -height;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biWidth = width;
  if (m_nBitsPerPixel != 1) {
    SetStretchBltMode(m_hDC, HALFTONE);
  }
  bmi.bmiColors[0] = 0xffffff;
  bmi.bmiColors[1] = 0;

  HBRUSH hPattern = CreateBrush(bitmap_color);
  HBRUSH hOld = (HBRUSH)SelectObject(m_hDC, hPattern);

  // In PDF, when image mask is 1, use device bitmap; when mask is 0, use brush
  // bitmap.
  // A complete list of the boolen operations is as follows:

  /* P(bitmap_color)    S(ImageMask)    D(DeviceBitmap)    Result
   *        0                 0                0              0
   *        0                 0                1              0
   *        0                 1                0              0
   *        0                 1                1              1
   *        1                 0                0              1
   *        1                 0                1              1
   *        1                 1                0              0
   *        1                 1                1              1
   */
  // The boolen codes is B8. Based on
  // http://msdn.microsoft.com/en-us/library/aa932106.aspx, the ROP3 code is
  // 0xB8074A

  ::StretchDIBits(m_hDC, dest_left, dest_top, dest_width, dest_height, 0, 0,
                  width, height, pBitmap->GetBuffer(), (BITMAPINFO*)&bmi,
                  DIB_RGB_COLORS, 0xB8074A);

  SelectObject(m_hDC, hOld);
  DeleteObject(hPattern);

  return true;
}

bool CGdiDeviceDriver::GetClipBox(FX_RECT* pRect) {
  return !!(::GetClipBox(m_hDC, (RECT*)pRect));
}

void CGdiDeviceDriver::DrawLine(float x1, float y1, float x2, float y2) {
  if (!m_bMetafileDCType) {  // EMF drawing is not bound to the DC.
    int startOutOfBoundsFlag = (x1 < 0) | ((x1 > m_Width) << 1) |
                               ((y1 < 0) << 2) | ((y1 > m_Height) << 3);
    int endOutOfBoundsFlag = (x2 < 0) | ((x2 > m_Width) << 1) |
                             ((y2 < 0) << 2) | ((y2 > m_Height) << 3);
    if (startOutOfBoundsFlag & endOutOfBoundsFlag)
      return;

    if (startOutOfBoundsFlag || endOutOfBoundsFlag) {
      float x[2];
      float y[2];
      int np;
#if defined(_SKIA_SUPPORT_)
      // TODO(caryclark) temporary replacement of antigrain in line function
      // to permit removing antigrain altogether
      rect_base rect = {0.0f, 0.0f, (float)(m_Width), (float)(m_Height)};
      np = clip_liang_barsky(x1, y1, x2, y2, rect, x, y);
#else
      pdfium::agg::rect_base<float> rect(0.0f, 0.0f, (float)(m_Width),
                                         (float)(m_Height));
      np = pdfium::agg::clip_liang_barsky<float>(x1, y1, x2, y2, rect, x, y);
#endif
      if (np == 0)
        return;

      if (np == 1) {
        x2 = x[0];
        y2 = y[0];
      } else {
        DCHECK_EQ(np, 2);
        x1 = x[0];
        y1 = y[0];
        x2 = x[1];
        y2 = y[1];
      }
    }
  }

  MoveToEx(m_hDC, FXSYS_roundf(x1), FXSYS_roundf(y1), nullptr);
  LineTo(m_hDC, FXSYS_roundf(x2), FXSYS_roundf(y2));
}

bool CGdiDeviceDriver::DrawPath(const CFX_Path* pPath,
                                const CFX_Matrix* pMatrix,
                                const CFX_GraphStateData* pGraphState,
                                uint32_t fill_color,
                                uint32_t stroke_color,
                                const CFX_FillRenderOptions& fill_options,
                                BlendMode blend_type) {
  if (blend_type != BlendMode::kNormal)
    return false;

  auto* pPlatform =
      static_cast<CWin32Platform*>(CFX_GEModule::Get()->GetPlatform());
  if (!(pGraphState || stroke_color == 0) &&
      !pPlatform->m_GdiplusExt.IsAvailable()) {
    CFX_FloatRect bbox_f = pPath->GetBoundingBox();
    if (pMatrix)
      bbox_f = pMatrix->TransformRect(bbox_f);

    FX_RECT bbox = bbox_f.GetInnerRect();
    if (bbox.Width() <= 0) {
      return DrawCosmeticLine(CFX_PointF(bbox.left, bbox.top),
                              CFX_PointF(bbox.left, bbox.bottom + 1),
                              fill_color, BlendMode::kNormal);
    }
    if (bbox.Height() <= 0) {
      return DrawCosmeticLine(CFX_PointF(bbox.left, bbox.top),
                              CFX_PointF(bbox.right + 1, bbox.top), fill_color,
                              BlendMode::kNormal);
    }
  }
  int fill_alpha = FXARGB_A(fill_color);
  int stroke_alpha = FXARGB_A(stroke_color);
  bool bDrawAlpha = (fill_alpha > 0 && fill_alpha < 255) ||
                    (stroke_alpha > 0 && stroke_alpha < 255 && pGraphState);
  if (!pPlatform->m_GdiplusExt.IsAvailable() && bDrawAlpha)
    return false;

  if (pPlatform->m_GdiplusExt.IsAvailable()) {
    if (bDrawAlpha ||
        ((m_DeviceType != DeviceType::kPrinter && !fill_options.full_cover) ||
         (pGraphState && !pGraphState->m_DashArray.empty()))) {
      if (!((!pMatrix || !pMatrix->WillScale()) && pGraphState &&
            pGraphState->m_LineWidth == 1.0f && pPath->IsRect())) {
        if (pPlatform->m_GdiplusExt.DrawPath(m_hDC, pPath, pMatrix, pGraphState,
                                             fill_color, stroke_color,
                                             fill_options)) {
          return true;
        }
      }
    }
  }
  const bool fill =
      fill_options.fill_type != CFX_FillRenderOptions::FillType::kNoFill;
  HPEN hPen = nullptr;
  HBRUSH hBrush = nullptr;
  if (pGraphState && stroke_alpha) {
    SetMiterLimit(m_hDC, pGraphState->m_MiterLimit, nullptr);
    hPen = CreateExtPen(pGraphState, pMatrix, stroke_color);
    hPen = (HPEN)SelectObject(m_hDC, hPen);
  }
  if (fill && fill_alpha) {
    SetPolyFillMode(m_hDC, FillTypeToGdiFillType(fill_options.fill_type));
    hBrush = CreateBrush(fill_color);
    hBrush = (HBRUSH)SelectObject(m_hDC, hBrush);
  }
  if (pPath->GetPoints().size() == 2 && pGraphState &&
      !pGraphState->m_DashArray.empty()) {
    CFX_PointF pos1 = pPath->GetPoint(0);
    CFX_PointF pos2 = pPath->GetPoint(1);
    if (pMatrix) {
      pos1 = pMatrix->Transform(pos1);
      pos2 = pMatrix->Transform(pos2);
    }
    DrawLine(pos1.x, pos1.y, pos2.x, pos2.y);
  } else {
    SetPathToDC(m_hDC, pPath, pMatrix);
    if (pGraphState && stroke_alpha) {
      if (fill && fill_alpha) {
        if (fill_options.text_mode) {
          StrokeAndFillPath(m_hDC);
        } else {
          FillPath(m_hDC);
          SetPathToDC(m_hDC, pPath, pMatrix);
          StrokePath(m_hDC);
        }
      } else {
        StrokePath(m_hDC);
      }
    } else if (fill && fill_alpha) {
      FillPath(m_hDC);
    }
  }
  if (hPen) {
    hPen = (HPEN)SelectObject(m_hDC, hPen);
    DeleteObject(hPen);
  }
  if (hBrush) {
    hBrush = (HBRUSH)SelectObject(m_hDC, hBrush);
    DeleteObject(hBrush);
  }
  return true;
}

bool CGdiDeviceDriver::FillRectWithBlend(const FX_RECT& rect,
                                         uint32_t fill_color,
                                         BlendMode blend_type) {
  if (blend_type != BlendMode::kNormal)
    return false;

  int alpha;
  FX_COLORREF colorref;
  std::tie(alpha, colorref) = ArgbToAlphaAndColorRef(fill_color);
  if (alpha == 0)
    return true;

  if (alpha < 255)
    return false;

  HBRUSH hBrush = CreateSolidBrush(colorref);
  const RECT* pRect = reinterpret_cast<const RECT*>(&rect);
  ::FillRect(m_hDC, pRect, hBrush);
  DeleteObject(hBrush);
  return true;
}

void CGdiDeviceDriver::SetBaseClip(const FX_RECT& rect) {
  m_BaseClipBox = rect;
}

bool CGdiDeviceDriver::SetClip_PathFill(
    const CFX_Path* pPath,
    const CFX_Matrix* pMatrix,
    const CFX_FillRenderOptions& fill_options) {
  Optional<CFX_FloatRect> maybe_rectf = pPath->GetRect(pMatrix);
  if (maybe_rectf.has_value()) {
    FX_RECT rect = maybe_rectf.value().GetOuterRect();
    // Can easily apply base clip to protect against wildly large rectangular
    // clips. crbug.com/1019026
    if (m_BaseClipBox.has_value())
      rect.Intersect(m_BaseClipBox.value());
    return IntersectClipRect(m_hDC, rect.left, rect.top, rect.right,
                             rect.bottom) != ERROR;
  }
  SetPathToDC(m_hDC, pPath, pMatrix);
  SetPolyFillMode(m_hDC, FillTypeToGdiFillType(fill_options.fill_type));
  SelectClipPath(m_hDC, RGN_AND);
  return true;
}

bool CGdiDeviceDriver::SetClip_PathStroke(
    const CFX_Path* pPath,
    const CFX_Matrix* pMatrix,
    const CFX_GraphStateData* pGraphState) {
  HPEN hPen = CreateExtPen(pGraphState, pMatrix, 0xff000000);
  hPen = (HPEN)SelectObject(m_hDC, hPen);
  SetPathToDC(m_hDC, pPath, pMatrix);
  WidenPath(m_hDC);
  SetPolyFillMode(m_hDC, WINDING);
  bool ret = !!SelectClipPath(m_hDC, RGN_AND);
  hPen = (HPEN)SelectObject(m_hDC, hPen);
  DeleteObject(hPen);
  return ret;
}

bool CGdiDeviceDriver::DrawCosmeticLine(const CFX_PointF& ptMoveTo,
                                        const CFX_PointF& ptLineTo,
                                        uint32_t color,
                                        BlendMode blend_type) {
  if (blend_type != BlendMode::kNormal)
    return false;

  int alpha;
  FX_COLORREF colorref;
  std::tie(alpha, colorref) = ArgbToAlphaAndColorRef(color);
  if (alpha == 0)
    return true;

  HPEN hPen = CreatePen(PS_SOLID, 1, colorref);
  hPen = (HPEN)SelectObject(m_hDC, hPen);
  MoveToEx(m_hDC, FXSYS_roundf(ptMoveTo.x), FXSYS_roundf(ptMoveTo.y), nullptr);
  LineTo(m_hDC, FXSYS_roundf(ptLineTo.x), FXSYS_roundf(ptLineTo.y));
  hPen = (HPEN)SelectObject(m_hDC, hPen);
  DeleteObject(hPen);
  return true;
}
