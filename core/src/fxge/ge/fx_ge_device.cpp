// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxge/fx_ge.h"

CFX_RenderDevice::CFX_RenderDevice() {
  m_pDeviceDriver = NULL;
  m_pBitmap = NULL;
}
CFX_RenderDevice::~CFX_RenderDevice() {
  delete m_pDeviceDriver;
}
void CFX_RenderDevice::SetDeviceDriver(IFX_RenderDeviceDriver* pDriver) {
  delete m_pDeviceDriver;
  m_pDeviceDriver = pDriver;
  InitDeviceInfo();
}
void CFX_RenderDevice::InitDeviceInfo() {
  m_Width = m_pDeviceDriver->GetDeviceCaps(FXDC_PIXEL_WIDTH);
  m_Height = m_pDeviceDriver->GetDeviceCaps(FXDC_PIXEL_HEIGHT);
  m_bpp = m_pDeviceDriver->GetDeviceCaps(FXDC_BITS_PIXEL);
  m_RenderCaps = m_pDeviceDriver->GetDeviceCaps(FXDC_RENDER_CAPS);
  m_DeviceClass = m_pDeviceDriver->GetDeviceCaps(FXDC_DEVICE_CLASS);
  if (!m_pDeviceDriver->GetClipBox(&m_ClipBox)) {
    m_ClipBox.left = 0;
    m_ClipBox.top = 0;
    m_ClipBox.right = m_Width;
    m_ClipBox.bottom = m_Height;
  }
}
FX_BOOL CFX_RenderDevice::StartRendering() {
  return m_pDeviceDriver->StartRendering();
}
void CFX_RenderDevice::EndRendering() {
  m_pDeviceDriver->EndRendering();
}
void CFX_RenderDevice::SaveState() {
  m_pDeviceDriver->SaveState();
}
void CFX_RenderDevice::RestoreState(FX_BOOL bKeepSaved) {
  m_pDeviceDriver->RestoreState(bKeepSaved);
  UpdateClipBox();
}
int CFX_RenderDevice::GetDeviceCaps(int caps_id) const {
  return m_pDeviceDriver->GetDeviceCaps(caps_id);
}
CFX_Matrix CFX_RenderDevice::GetCTM() const {
  return m_pDeviceDriver->GetCTM();
}
FX_BOOL CFX_RenderDevice::CreateCompatibleBitmap(CFX_DIBitmap* pDIB,
                                                 int width,
                                                 int height) const {
  if (m_RenderCaps & FXRC_CMYK_OUTPUT) {
    return pDIB->Create(width, height, m_RenderCaps & FXRC_ALPHA_OUTPUT
                                           ? FXDIB_Cmyka
                                           : FXDIB_Cmyk);
  }
  if (m_RenderCaps & FXRC_BYTEMASK_OUTPUT) {
    return pDIB->Create(width, height, FXDIB_8bppMask);
  }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  return pDIB->Create(width, height, m_RenderCaps & FXRC_ALPHA_OUTPUT
                                         ? FXDIB_Argb
                                         : FXDIB_Rgb32);
#else
  return pDIB->Create(
      width, height, m_RenderCaps & FXRC_ALPHA_OUTPUT ? FXDIB_Argb : FXDIB_Rgb);
#endif
}
FX_BOOL CFX_RenderDevice::SetClip_PathFill(const CFX_PathData* pPathData,
                                           const CFX_Matrix* pObject2Device,
                                           int fill_mode) {
  if (!m_pDeviceDriver->SetClip_PathFill(pPathData, pObject2Device,
                                         fill_mode)) {
    return FALSE;
  }
  UpdateClipBox();
  return TRUE;
}
FX_BOOL CFX_RenderDevice::SetClip_PathStroke(
    const CFX_PathData* pPathData,
    const CFX_Matrix* pObject2Device,
    const CFX_GraphStateData* pGraphState) {
  if (!m_pDeviceDriver->SetClip_PathStroke(pPathData, pObject2Device,
                                           pGraphState)) {
    return FALSE;
  }
  UpdateClipBox();
  return TRUE;
}
FX_BOOL CFX_RenderDevice::SetClip_Rect(const FX_RECT* pRect) {
  CFX_PathData path;
  path.AppendRect((FX_FLOAT)(pRect->left), (FX_FLOAT)(pRect->bottom),
                  (FX_FLOAT)(pRect->right), (FX_FLOAT)(pRect->top));
  if (!SetClip_PathFill(&path, NULL, FXFILL_WINDING)) {
    return FALSE;
  }
  UpdateClipBox();
  return TRUE;
}
void CFX_RenderDevice::UpdateClipBox() {
  if (m_pDeviceDriver->GetClipBox(&m_ClipBox)) {
    return;
  }
  m_ClipBox.left = 0;
  m_ClipBox.top = 0;
  m_ClipBox.right = m_Width;
  m_ClipBox.bottom = m_Height;
}
FX_BOOL CFX_RenderDevice::DrawPath(const CFX_PathData* pPathData,
                                   const CFX_Matrix* pObject2Device,
                                   const CFX_GraphStateData* pGraphState,
                                   FX_DWORD fill_color,
                                   FX_DWORD stroke_color,
                                   int fill_mode,
                                   int alpha_flag,
                                   void* pIccTransform,
                                   int blend_type) {
  uint8_t fill_alpha, stroke_alpha;
  if (FXGETFLAG_COLORTYPE(alpha_flag)) {
    fill_alpha = FXGETFLAG_ALPHA_FILL(alpha_flag);
    stroke_alpha = FXGETFLAG_ALPHA_STROKE(alpha_flag);
  } else {
    fill_alpha = FXARGB_A(fill_color);
    stroke_alpha = FXARGB_A(stroke_color);
  }
  if ((fill_mode & 3) == 0) {
    fill_alpha = 0;
  }
  if (!pGraphState) {
    stroke_alpha = 0;
  }
  if (stroke_alpha == 0 && pPathData->GetPointCount() == 2) {
    FX_PATHPOINT* pPoints = pPathData->GetPoints();
    FX_FLOAT x1, x2, y1, y2;
    if (pObject2Device) {
      pObject2Device->Transform(pPoints[0].m_PointX, pPoints[0].m_PointY, x1,
                                y1);
      pObject2Device->Transform(pPoints[1].m_PointX, pPoints[1].m_PointY, x2,
                                y2);
    } else {
      x1 = pPoints[0].m_PointX;
      y1 = pPoints[0].m_PointY;
      x2 = pPoints[1].m_PointX;
      y2 = pPoints[1].m_PointY;
    }
    DrawCosmeticLine(x1, y1, x2, y2, fill_color, fill_mode, alpha_flag,
                     pIccTransform, blend_type);
    return TRUE;
  }
  if ((pPathData->GetPointCount() == 5 || pPathData->GetPointCount() == 4) &&
      stroke_alpha == 0) {
    CFX_FloatRect rect_f;
    if (!(fill_mode & FXFILL_RECT_AA) &&
        pPathData->IsRect(pObject2Device, &rect_f)) {
      FX_RECT rect_i = rect_f.GetOutterRect();
      int width = (int)FXSYS_ceil(rect_f.right - rect_f.left);
      if (width < 1) {
        width = 1;
        if (rect_i.left == rect_i.right) {
          rect_i.right++;
        }
      }
      int height = (int)FXSYS_ceil(rect_f.top - rect_f.bottom);
      if (height < 1) {
        height = 1;
        if (rect_i.bottom == rect_i.top) {
          rect_i.bottom++;
        }
      }
      if (rect_i.Width() >= width + 1) {
        if (rect_f.left - (FX_FLOAT)(rect_i.left) >
            (FX_FLOAT)(rect_i.right) - rect_f.right) {
          rect_i.left++;
        } else {
          rect_i.right--;
        }
      }
      if (rect_i.Height() >= height + 1) {
        if (rect_f.top - (FX_FLOAT)(rect_i.top) >
            (FX_FLOAT)(rect_i.bottom) - rect_f.bottom) {
          rect_i.top++;
        } else {
          rect_i.bottom--;
        }
      }
      if (FillRect(&rect_i, fill_color, alpha_flag, pIccTransform,
                   blend_type)) {
        return TRUE;
      }
    }
  }
  if ((fill_mode & 3) && stroke_alpha == 0 && !(fill_mode & FX_FILL_STROKE) &&
      !(fill_mode & FX_FILL_TEXT_MODE)) {
    CFX_PathData newPath;
    FX_BOOL bThin = FALSE;
    if (pPathData->GetZeroAreaPath(newPath, (CFX_Matrix*)pObject2Device, bThin,
                                   m_pDeviceDriver->GetDriverType())) {
      CFX_GraphStateData graphState;
      graphState.m_LineWidth = 0.0f;
      FX_DWORD strokecolor = fill_color;
      if (bThin) {
        if (FXGETFLAG_COLORTYPE(alpha_flag)) {
          FXSETFLAG_ALPHA_STROKE(alpha_flag, fill_alpha >> 2);
        } else {
          strokecolor =
              (((fill_alpha >> 2) << 24) | (strokecolor & 0x00ffffff));
        }
      }
      CFX_Matrix* pMatrix = NULL;
      if (pObject2Device && !pObject2Device->IsIdentity()) {
        pMatrix = (CFX_Matrix*)pObject2Device;
      }
      int smooth_path = FX_ZEROAREA_FILL;
      if (fill_mode & FXFILL_NOPATHSMOOTH) {
        smooth_path |= FXFILL_NOPATHSMOOTH;
      }
      m_pDeviceDriver->DrawPath(&newPath, pMatrix, &graphState, 0, strokecolor,
                                smooth_path, alpha_flag, pIccTransform,
                                blend_type);
    }
  }
  if ((fill_mode & 3) && fill_alpha && stroke_alpha < 0xff &&
      (fill_mode & FX_FILL_STROKE)) {
    if (!(m_RenderCaps & FXRC_GET_BITS)) {
      return FALSE;
    }
    CFX_FloatRect bbox;
    if (pGraphState) {
      bbox = pPathData->GetBoundingBox(pGraphState->m_LineWidth,
                                       pGraphState->m_MiterLimit);
    } else {
      bbox = pPathData->GetBoundingBox();
    }
    if (pObject2Device) {
      bbox.Transform(pObject2Device);
    }
    CFX_Matrix ctm = GetCTM();
    FX_FLOAT fScaleX = FXSYS_fabs(ctm.a);
    FX_FLOAT fScaleY = FXSYS_fabs(ctm.d);
    FX_RECT rect = bbox.GetOutterRect();
    CFX_DIBitmap bitmap, Backdrop;
    if (!CreateCompatibleBitmap(&bitmap, FXSYS_round(rect.Width() * fScaleX),
                                FXSYS_round(rect.Height() * fScaleY))) {
      return FALSE;
    }
    if (bitmap.HasAlpha()) {
      bitmap.Clear(0);
      Backdrop.Copy(&bitmap);
    } else {
      if (!m_pDeviceDriver->GetDIBits(&bitmap, rect.left, rect.top, NULL)) {
        return FALSE;
      }
      Backdrop.Copy(&bitmap);
    }
    CFX_FxgeDevice bitmap_device;
    bitmap_device.Attach(&bitmap, 0, FALSE, &Backdrop, TRUE);
    CFX_Matrix matrix;
    if (pObject2Device) {
      matrix = *pObject2Device;
    }
    matrix.TranslateI(-rect.left, -rect.top);
    matrix.Concat(fScaleX, 0, 0, fScaleY, 0, 0);
    if (!bitmap_device.GetDeviceDriver()->DrawPath(
            pPathData, &matrix, pGraphState, fill_color, stroke_color,
            fill_mode, alpha_flag, pIccTransform, blend_type)) {
      return FALSE;
    }
    FX_RECT src_rect(0, 0, FXSYS_round(rect.Width() * fScaleX),
                     FXSYS_round(rect.Height() * fScaleY));
    return m_pDeviceDriver->SetDIBits(&bitmap, 0, &src_rect, rect.left,
                                      rect.top, FXDIB_BLEND_NORMAL);
  }
  return m_pDeviceDriver->DrawPath(pPathData, pObject2Device, pGraphState,
                                   fill_color, stroke_color, fill_mode,
                                   alpha_flag, pIccTransform, blend_type);
}
FX_BOOL CFX_RenderDevice::SetPixel(int x,
                                   int y,
                                   FX_DWORD color,
                                   int alpha_flag,
                                   void* pIccTransform) {
  if (m_pDeviceDriver->SetPixel(x, y, color, alpha_flag, pIccTransform)) {
    return TRUE;
  }
  FX_RECT rect(x, y, x + 1, y + 1);
  return FillRect(&rect, color, alpha_flag, pIccTransform);
}
FX_BOOL CFX_RenderDevice::FillRect(const FX_RECT* pRect,
                                   FX_DWORD fill_color,
                                   int alpha_flag,
                                   void* pIccTransform,
                                   int blend_type) {
  if (m_pDeviceDriver->FillRect(pRect, fill_color, alpha_flag, pIccTransform,
                                blend_type)) {
    return TRUE;
  }
  if (!(m_RenderCaps & FXRC_GET_BITS)) {
    return FALSE;
  }
  CFX_DIBitmap bitmap;
  if (!CreateCompatibleBitmap(&bitmap, pRect->Width(), pRect->Height())) {
    return FALSE;
  }
  if (!m_pDeviceDriver->GetDIBits(&bitmap, pRect->left, pRect->top)) {
    return FALSE;
  }
  if (!bitmap.CompositeRect(0, 0, pRect->Width(), pRect->Height(), fill_color,
                            alpha_flag, pIccTransform)) {
    return FALSE;
  }
  FX_RECT src_rect(0, 0, pRect->Width(), pRect->Height());
  m_pDeviceDriver->SetDIBits(&bitmap, 0, &src_rect, pRect->left, pRect->top,
                             FXDIB_BLEND_NORMAL);
  return TRUE;
}
FX_BOOL CFX_RenderDevice::DrawCosmeticLine(FX_FLOAT x1,
                                           FX_FLOAT y1,
                                           FX_FLOAT x2,
                                           FX_FLOAT y2,
                                           FX_DWORD color,
                                           int fill_mode,
                                           int alpha_flag,
                                           void* pIccTransform,
                                           int blend_type) {
  if (((m_RenderCaps & FXRC_ALPHA_PATH) &&
       (FXGETFLAG_COLORTYPE(alpha_flag) &&
        FXGETFLAG_ALPHA_FILL(alpha_flag) == 0xff)) ||
      color >= 0xff000000)
    if (m_pDeviceDriver->DrawCosmeticLine(x1, y1, x2, y2, color, alpha_flag,
                                          pIccTransform, blend_type)) {
      return TRUE;
    }
  CFX_GraphStateData graph_state;
  CFX_PathData path;
  path.SetPointCount(2);
  path.SetPoint(0, x1, y1, FXPT_MOVETO);
  path.SetPoint(1, x2, y2, FXPT_LINETO);
  return m_pDeviceDriver->DrawPath(&path, NULL, &graph_state, 0, color,
                                   fill_mode, alpha_flag, pIccTransform,
                                   blend_type);
}
FX_BOOL CFX_RenderDevice::GetDIBits(CFX_DIBitmap* pBitmap,
                                    int left,
                                    int top,
                                    void* pIccTransform) {
  if (!(m_RenderCaps & FXRC_GET_BITS)) {
    return FALSE;
  }
  return m_pDeviceDriver->GetDIBits(pBitmap, left, top, pIccTransform);
}
CFX_DIBitmap* CFX_RenderDevice::GetBackDrop() {
  return m_pDeviceDriver->GetBackDrop();
}
FX_BOOL CFX_RenderDevice::SetDIBits(const CFX_DIBSource* pBitmap,
                                    int left,
                                    int top,
                                    int blend_mode,
                                    void* pIccTransform) {
  ASSERT(!pBitmap->IsAlphaMask());
  CFX_Matrix ctm = GetCTM();
  FX_FLOAT fScaleX = FXSYS_fabs(ctm.a);
  FX_FLOAT fScaleY = FXSYS_fabs(ctm.d);
  FX_RECT dest_rect(left, top,
                    FXSYS_round(left + pBitmap->GetWidth() / fScaleX),
                    FXSYS_round(top + pBitmap->GetHeight() / fScaleY));
  dest_rect.Intersect(m_ClipBox);
  if (dest_rect.IsEmpty()) {
    return TRUE;
  }
  FX_RECT src_rect(dest_rect.left - left, dest_rect.top - top,
                   dest_rect.left - left + dest_rect.Width(),
                   dest_rect.top - top + dest_rect.Height());
  src_rect.left = FXSYS_round(src_rect.left * fScaleX);
  src_rect.top = FXSYS_round(src_rect.top * fScaleY);
  src_rect.right = FXSYS_round(src_rect.right * fScaleX);
  src_rect.bottom = FXSYS_round(src_rect.bottom * fScaleY);
  if ((blend_mode != FXDIB_BLEND_NORMAL && !(m_RenderCaps & FXRC_BLEND_MODE)) ||
      (pBitmap->HasAlpha() && !(m_RenderCaps & FXRC_ALPHA_IMAGE))) {
    if (!(m_RenderCaps & FXRC_GET_BITS)) {
      return FALSE;
    }
    int bg_pixel_width = FXSYS_round(dest_rect.Width() * fScaleX);
    int bg_pixel_height = FXSYS_round(dest_rect.Height() * fScaleY);
    CFX_DIBitmap background;
    if (!background.Create(
            bg_pixel_width, bg_pixel_height,
            (m_RenderCaps & FXRC_CMYK_OUTPUT) ? FXDIB_Cmyk : FXDIB_Rgb32)) {
      return FALSE;
    }
    if (!m_pDeviceDriver->GetDIBits(&background, dest_rect.left,
                                    dest_rect.top)) {
      return FALSE;
    }
    if (!background.CompositeBitmap(0, 0, bg_pixel_width, bg_pixel_height,
                                    pBitmap, src_rect.left, src_rect.top,
                                    blend_mode, NULL, FALSE, pIccTransform)) {
      return FALSE;
    }
    FX_RECT src_rect(0, 0, bg_pixel_width, bg_pixel_height);
    return m_pDeviceDriver->SetDIBits(&background, 0, &src_rect, dest_rect.left,
                                      dest_rect.top, FXDIB_BLEND_NORMAL);
  }
  return m_pDeviceDriver->SetDIBits(pBitmap, 0, &src_rect, dest_rect.left,
                                    dest_rect.top, blend_mode, 0,
                                    pIccTransform);
}
FX_BOOL CFX_RenderDevice::StretchDIBits(const CFX_DIBSource* pBitmap,
                                        int left,
                                        int top,
                                        int dest_width,
                                        int dest_height,
                                        FX_DWORD flags,
                                        void* pIccTransform,
                                        int blend_mode) {
  FX_RECT dest_rect(left, top, left + dest_width, top + dest_height);
  FX_RECT clip_box = m_ClipBox;
  clip_box.Intersect(dest_rect);
  if (clip_box.IsEmpty()) {
    return TRUE;
  }
  return m_pDeviceDriver->StretchDIBits(pBitmap, 0, left, top, dest_width,
                                        dest_height, &clip_box, flags, 0,
                                        pIccTransform, blend_mode);
}
FX_BOOL CFX_RenderDevice::SetBitMask(const CFX_DIBSource* pBitmap,
                                     int left,
                                     int top,
                                     FX_DWORD argb,
                                     int alpha_flag,
                                     void* pIccTransform) {
  FX_RECT src_rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
  return m_pDeviceDriver->SetDIBits(pBitmap, argb, &src_rect, left, top,
                                    FXDIB_BLEND_NORMAL, alpha_flag,
                                    pIccTransform);
}
FX_BOOL CFX_RenderDevice::StretchBitMask(const CFX_DIBSource* pBitmap,
                                         int left,
                                         int top,
                                         int dest_width,
                                         int dest_height,
                                         FX_DWORD argb,
                                         FX_DWORD flags,
                                         int alpha_flag,
                                         void* pIccTransform) {
  FX_RECT dest_rect(left, top, left + dest_width, top + dest_height);
  FX_RECT clip_box = m_ClipBox;
  clip_box.Intersect(dest_rect);
  return m_pDeviceDriver->StretchDIBits(pBitmap, argb, left, top, dest_width,
                                        dest_height, &clip_box, flags,
                                        alpha_flag, pIccTransform);
}
FX_BOOL CFX_RenderDevice::StartDIBits(const CFX_DIBSource* pBitmap,
                                      int bitmap_alpha,
                                      FX_DWORD argb,
                                      const CFX_Matrix* pMatrix,
                                      FX_DWORD flags,
                                      void*& handle,
                                      int alpha_flag,
                                      void* pIccTransform,
                                      int blend_mode) {
  return m_pDeviceDriver->StartDIBits(pBitmap, bitmap_alpha, argb, pMatrix,
                                      flags, handle, alpha_flag, pIccTransform,
                                      blend_mode);
}
FX_BOOL CFX_RenderDevice::ContinueDIBits(void* handle, IFX_Pause* pPause) {
  return m_pDeviceDriver->ContinueDIBits(handle, pPause);
}
void CFX_RenderDevice::CancelDIBits(void* handle) {
  m_pDeviceDriver->CancelDIBits(handle);
}
