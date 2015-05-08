// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_dib.h"
#include "dib_int.h"
int SDP_Table[513] = {
    256, 256, 256, 256, 256, 256, 256, 256, 256, 255, 255, 255, 255, 255, 255, 254, 254, 254, 254,
    253, 253, 253, 252, 252, 252, 251, 251, 251, 250, 250, 249, 249, 249, 248, 248, 247, 247, 246,
    246, 245, 244, 244, 243, 243, 242, 242, 241, 240, 240, 239, 238, 238, 237, 236, 236, 235, 234,
    233, 233, 232, 231, 230, 230, 229, 228, 227, 226, 226, 225, 224, 223, 222, 221, 220, 219, 218,
    218, 217, 216, 215, 214, 213, 212, 211, 210, 209, 208, 207, 206, 205, 204, 203, 202, 201, 200,
    199, 198, 196, 195, 194, 193, 192, 191, 190, 189, 188, 186, 185, 184, 183, 182, 181, 179, 178,
    177, 176, 175, 173, 172, 171, 170, 169, 167, 166, 165, 164, 162, 161, 160, 159, 157, 156, 155,
    154, 152, 151, 150, 149, 147, 146, 145, 143, 142, 141, 140, 138, 137, 136, 134, 133, 132, 130,
    129, 128, 126, 125, 124, 122, 121, 120, 119, 117, 116, 115, 113, 112, 111, 109, 108, 107, 105,
    104, 103, 101, 100, 99, 97, 96, 95, 93, 92, 91, 89, 88, 87, 85, 84, 83, 81, 80, 79, 77, 76, 75,
    73, 72, 71, 69, 68, 67, 66, 64, 63, 62, 60, 59, 58, 57, 55, 54, 53, 52, 50, 49, 48, 47, 45, 44,
    43, 42, 40, 39, 38, 37, 36, 34, 33, 32, 31, 30, 28, 27, 26, 25, 24, 23, 21, 20, 19, 18, 17, 16,
    15, 14, 13, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, -1, -2, -3, -4, -5, -6, -7, -7, -8, -9, -10,
    -11, -12, -12, -13, -14, -15, -15, -16, -17, -17, -18, -19, -19, -20, -21, -21, -22, -22, -23, -24,
    -24, -25, -25, -26, -26, -27, -27, -27, -28, -28, -29, -29, -30, -30, -30, -31, -31, -31, -32, -32,
    -32, -33, -33, -33, -33, -34, -34, -34, -34, -35, -35, -35, -35, -35, -36, -36, -36, -36, -36, -36,
    -36, -36, -36, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37, -37,
    -37, -37, -37, -37, -36, -36, -36, -36, -36, -36, -36, -36, -36, -35, -35, -35, -35, -35, -35, -34,
    -34, -34, -34, -34, -33, -33, -33, -33, -33, -32, -32, -32, -32, -31, -31, -31, -31, -30, -30, -30,
    -30, -29, -29, -29, -29, -28, -28, -28, -27, -27, -27, -27, -26, -26, -26, -25, -25, -25, -24, -24,
    -24, -23, -23, -23, -22, -22, -22, -22, -21, -21, -21, -20, -20, -20, -19, -19, -19, -18, -18, -18,
    -17, -17, -17, -16, -16, -16, -15, -15, -15, -14, -14, -14, -13, -13, -13, -12, -12, -12, -11, -11,
    -11, -10, -10, -10, -9, -9, -9, -9, -8, -8, -8, -7, -7, -7, -7, -6, -6, -6, -6, -5, -5, -5, -5, -4,
    -4, -4, -4, -3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
class CFX_BilinearMatrix : public CPDF_FixedMatrix
{
public:
    CFX_BilinearMatrix(const CFX_AffineMatrix& src, int bits): CPDF_FixedMatrix(src, bits)
    {}
    inline void	Transform(int x, int y, int& x1, int& y1, int&res_x, int&res_y)
    {
        x1 = a * x + c * y + e + base / 2;
        y1 = b * x + d * y + f + base / 2;
        res_x = x1 % base;
        res_y = y1 % base;
        if (res_x < 0 && res_x > -base) {
            res_x = base + res_x;
        }
        if (res_y < 0 && res_x > -base) {
            res_y = base + res_y;
        }
        x1 /= base;
        y1 /= base;
    }
};
CFX_DIBitmap* CFX_DIBSource::SwapXY(FX_BOOL bXFlip, FX_BOOL bYFlip, const FX_RECT* pDestClip) const
{
    FX_RECT dest_clip(0, 0, m_Height, m_Width);
    if (pDestClip) {
        dest_clip.Intersect(*pDestClip);
    }
    if (dest_clip.IsEmpty()) {
        return NULL;
    }
    CFX_DIBitmap* pTransBitmap = new CFX_DIBitmap;
    int result_height = dest_clip.Height(), result_width = dest_clip.Width();
    if (!pTransBitmap->Create(result_width, result_height, GetFormat())) {
        delete pTransBitmap;
        return NULL;
    }
    pTransBitmap->CopyPalette(m_pPalette);
    int dest_pitch = pTransBitmap->GetPitch();
    FX_LPBYTE dest_buf = pTransBitmap->GetBuffer();
    int row_start = bXFlip ? m_Height - dest_clip.right : dest_clip.left;
    int row_end = bXFlip ? m_Height - dest_clip.left : dest_clip.right;
    int col_start = bYFlip ? m_Width - dest_clip.bottom : dest_clip.top;
    int col_end = bYFlip ? m_Width - dest_clip.top : dest_clip.bottom;
    if (GetBPP() == 1) {
        FXSYS_memset8(dest_buf, 0xff, dest_pitch * result_height);
        for (int row = row_start; row < row_end; row ++) {
            FX_LPCBYTE src_scan = GetScanline(row);
            int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) - dest_clip.left;
            FX_LPBYTE dest_scan = dest_buf;
            if (bYFlip) {
                dest_scan += (result_height - 1) * dest_pitch;
            }
            int dest_step = bYFlip ? -dest_pitch : dest_pitch;
            for (int col = col_start; col < col_end; col ++) {
                if (!(src_scan[col / 8] & (1 << (7 - col % 8)))) {
                    dest_scan[dest_col / 8] &= ~(1 << (7 - dest_col % 8));
                }
                dest_scan += dest_step;
            }
        }
    } else {
        int nBytes = GetBPP() / 8;
        int dest_step = bYFlip ? -dest_pitch : dest_pitch;
        if (nBytes == 3) {
            dest_step -= 2;
        }
        for (int row = row_start; row < row_end; row ++) {
            int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) - dest_clip.left;
            FX_LPBYTE dest_scan = dest_buf + dest_col * nBytes;
            if (bYFlip) {
                dest_scan += (result_height - 1) * dest_pitch;
            }
            if (nBytes == 4) {
                FX_DWORD* src_scan = (FX_DWORD*)GetScanline(row) + col_start;
                for (int col = col_start; col < col_end; col ++) {
                    *(FX_DWORD*)dest_scan = *src_scan++;
                    dest_scan += dest_step;
                }
            } else {
                FX_LPCBYTE src_scan = GetScanline(row) + col_start * nBytes;
                if (nBytes == 1)
                    for (int col = col_start; col < col_end; col ++) {
                        *dest_scan = *src_scan++;
                        dest_scan += dest_step;
                    }
                else
                    for (int col = col_start; col < col_end; col ++) {
                        *dest_scan++ = *src_scan++;
                        *dest_scan++ = *src_scan++;
                        *dest_scan = *src_scan++;
                        dest_scan += dest_step;
                    }
            }
        }
    }
    if (m_pAlphaMask) {
        dest_pitch = pTransBitmap->m_pAlphaMask->GetPitch();
        dest_buf = pTransBitmap->m_pAlphaMask->GetBuffer();
        int dest_step = bYFlip ? -dest_pitch : dest_pitch;
        for (int row = row_start; row < row_end; row ++) {
            int dest_col = (bXFlip ? dest_clip.right - (row - row_start) - 1 : row) - dest_clip.left;
            FX_LPBYTE dest_scan = dest_buf + dest_col;
            if (bYFlip) {
                dest_scan += (result_height - 1) * dest_pitch;
            }
            FX_LPCBYTE src_scan = m_pAlphaMask->GetScanline(row) + col_start;
            for (int col = col_start; col < col_end; col ++) {
                *dest_scan = *src_scan++;
                dest_scan += dest_step;
            }
        }
    }
    return pTransBitmap;
}
#define FIX16_005 0.05f
FX_RECT _FXDIB_SwapClipBox(FX_RECT& clip, int width, int height, FX_BOOL bFlipX, FX_BOOL bFlipY)
{
    FX_RECT rect;
    if (bFlipY) {
        rect.left = height - clip.top;
        rect.right = height - clip.bottom;
    } else {
        rect.left = clip.top;
        rect.right = clip.bottom;
    }
    if (bFlipX) {
        rect.top = width - clip.left;
        rect.bottom = width - clip.right;
    } else {
        rect.top = clip.left;
        rect.bottom = clip.right;
    }
    rect.Normalize();
    return rect;
}
CFX_DIBitmap* CFX_DIBSource::TransformTo(const CFX_AffineMatrix* pDestMatrix, int& result_left, int& result_top,
        FX_DWORD flags, const FX_RECT* pDestClip) const
{
    CFX_ImageTransformer transformer;
    transformer.Start(this, pDestMatrix, flags, pDestClip);
    transformer.Continue(NULL);
    result_left = transformer.m_ResultLeft;
    result_top = transformer.m_ResultTop;
    CFX_DIBitmap* pTransformed = transformer.m_Storer.Detach();
    return pTransformed;
}
CFX_DIBitmap* CFX_DIBSource::StretchTo(int dest_width, int dest_height, FX_DWORD flags, const FX_RECT* pClip) const
{
    FX_RECT clip_rect(0, 0, FXSYS_abs(dest_width), FXSYS_abs(dest_height));
    if (pClip) {
        clip_rect.Intersect(*pClip);
    }
    if (clip_rect.IsEmpty()) {
        return NULL;
    }
    if (dest_width == m_Width && dest_height == m_Height) {
        return Clone(&clip_rect);
    }
    CFX_ImageStretcher stretcher;
    CFX_BitmapStorer storer;
    if (stretcher.Start(&storer, this, dest_width, dest_height, clip_rect, flags)) {
        stretcher.Continue(NULL);
    }
    return storer.Detach();
}
CFX_ImageTransformer::CFX_ImageTransformer()
{
    m_Status = 0;
    m_pMatrix = NULL;
}
CFX_ImageTransformer::~CFX_ImageTransformer()
{
}
FX_BOOL CFX_ImageTransformer::Start(const CFX_DIBSource* pSrc, const CFX_AffineMatrix* pDestMatrix, int flags, const FX_RECT* pDestClip)
{
    m_pMatrix = (CFX_AffineMatrix*)pDestMatrix;
    CFX_FloatRect unit_rect = pDestMatrix->GetUnitRect();
    FX_RECT result_rect = unit_rect.GetClosestRect();
    FX_RECT result_clip = result_rect;
    if (pDestClip) {
        result_clip.Intersect(*pDestClip);
    }
    if (result_clip.IsEmpty()) {
        return FALSE;
    }
    m_ResultLeft = result_clip.left;
    m_ResultTop = result_clip.top;
    m_ResultWidth = result_clip.Width();
    m_ResultHeight = result_clip.Height();
    m_Flags = flags;
    if (FXSYS_fabs(pDestMatrix->a) < FXSYS_fabs(pDestMatrix->b) / 20 &&
            FXSYS_fabs(pDestMatrix->d) < FXSYS_fabs(pDestMatrix->c) / 20 &&
            FXSYS_fabs(pDestMatrix->a) < 0.5f && FXSYS_fabs(pDestMatrix->d) < 0.5f) {
        int dest_width = result_rect.Width();
        int dest_height = result_rect.Height();
        result_clip.Offset(-result_rect.left, -result_rect.top);
        result_clip = _FXDIB_SwapClipBox(result_clip, dest_width, dest_height, pDestMatrix->c > 0, pDestMatrix->b < 0);
        m_Stretcher.Start(&m_Storer, pSrc, dest_height, dest_width, result_clip, flags);
        m_Status = 1;
        return TRUE;
    }
    if (FXSYS_fabs(pDestMatrix->b) < FIX16_005 && FXSYS_fabs(pDestMatrix->c) < FIX16_005) {
        int dest_width = pDestMatrix->a > 0 ? (int)FXSYS_ceil(pDestMatrix->a) : (int)FXSYS_floor(pDestMatrix->a);
        int dest_height = pDestMatrix->d > 0 ? (int) - FXSYS_ceil(pDestMatrix->d) : (int) - FXSYS_floor(pDestMatrix->d);
        result_clip.Offset(-result_rect.left, -result_rect.top);
        m_Stretcher.Start(&m_Storer, pSrc, dest_width, dest_height, result_clip, flags);
        m_Status = 2;
        return TRUE;
    }
    int stretch_width = (int)FXSYS_ceil(FXSYS_sqrt2(pDestMatrix->a, pDestMatrix->b));
    int stretch_height = (int)FXSYS_ceil(FXSYS_sqrt2(pDestMatrix->c, pDestMatrix->d));
    CFX_AffineMatrix stretch2dest(1.0f, 0.0f, 0.0f, -1.0f, 0.0f, (FX_FLOAT)(stretch_height));
    stretch2dest.Concat(pDestMatrix->a / stretch_width, pDestMatrix->b / stretch_width,
                        pDestMatrix->c / stretch_height, pDestMatrix->d / stretch_height, pDestMatrix->e, pDestMatrix->f);
    m_dest2stretch.SetReverse(stretch2dest);
    CFX_FloatRect clip_rect_f(result_clip);
    clip_rect_f.Transform(&m_dest2stretch);
    m_StretchClip = clip_rect_f.GetOutterRect();
    m_StretchClip.Intersect(0, 0, stretch_width, stretch_height);
    m_Stretcher.Start(&m_Storer, pSrc, stretch_width, stretch_height, m_StretchClip, flags);
    m_Status = 3;
    return TRUE;
}
FX_BYTE _bilinear_interpol(FX_LPCBYTE buf, int row_offset_l, int row_offset_r,
                           int src_col_l, int src_col_r, int res_x, int res_y,
                           int bpp, int c_offset)
{
    int i_resx = 255 - res_x;
    int col_bpp_l = src_col_l * bpp;
    int col_bpp_r = src_col_r * bpp;
    FX_LPCBYTE buf_u = buf + row_offset_l + c_offset;
    FX_LPCBYTE buf_d = buf + row_offset_r + c_offset;
    FX_LPCBYTE src_pos0 = buf_u + col_bpp_l;
    FX_LPCBYTE src_pos1 = buf_u + col_bpp_r;
    FX_LPCBYTE src_pos2 = buf_d + col_bpp_l;
    FX_LPCBYTE src_pos3 = buf_d + col_bpp_r;
    FX_BYTE r_pos_0 = (*src_pos0 * i_resx + *src_pos1 * res_x) >> 8;
    FX_BYTE r_pos_1 = (*src_pos2 * i_resx + *src_pos3 * res_x) >> 8;
    return (r_pos_0 * (255 - res_y) + r_pos_1 * res_y) >> 8;
}
FX_BYTE _bicubic_interpol(FX_LPCBYTE buf, int pitch, int pos_pixel[], int u_w[], int v_w[], int res_x, int res_y,
                          int bpp, int c_offset)
{
    int s_result = 0;
    for (int i = 0; i < 4; i ++) {
        int a_result = 0;
        for (int j = 0; j < 4; j ++) {
            a_result += u_w[j] * (*(FX_BYTE*)(buf + pos_pixel[i + 4] * pitch + pos_pixel[j] * bpp + c_offset));
        }
        s_result += a_result * v_w[i];
    }
    s_result >>= 16;
    return (FX_BYTE)(s_result < 0 ? 0 : s_result > 255 ? 255 : s_result);
}
void _bicubic_get_pos_weight(int pos_pixel[], int u_w[], int v_w[], int src_col_l, int src_row_l,
                             int res_x, int res_y, int stretch_width, int stretch_height)
{
    pos_pixel[0] = src_col_l - 1;
    pos_pixel[1] = src_col_l;
    pos_pixel[2] = src_col_l + 1;
    pos_pixel[3] = src_col_l + 2;
    pos_pixel[4] = src_row_l - 1;
    pos_pixel[5] = src_row_l;
    pos_pixel[6] = src_row_l + 1;
    pos_pixel[7] = src_row_l + 2;
    for (int i = 0 ; i < 4; i ++) {
        if (pos_pixel[i] < 0) {
            pos_pixel[i] = 0;
        }
        if (pos_pixel[i] >= stretch_width) {
            pos_pixel[i] = stretch_width - 1;
        }
        if (pos_pixel[i + 4] < 0) {
            pos_pixel[i + 4] = 0;
        }
        if (pos_pixel[i + 4] >= stretch_height) {
            pos_pixel[i + 4] = stretch_height - 1;
        }
    }
    u_w[0] = SDP_Table[256 + res_x];
    u_w[1] = SDP_Table[res_x];
    u_w[2] = SDP_Table[256 - res_x];
    u_w[3] = SDP_Table[512 - res_x];
    v_w[0] = SDP_Table[256 + res_y];
    v_w[1] = SDP_Table[res_y];
    v_w[2] = SDP_Table[256 - res_y];
    v_w[3] = SDP_Table[512 - res_y];
}
FXDIB_Format _GetTransformedFormat(const CFX_DIBSource* pDrc)
{
    FXDIB_Format format = pDrc->GetFormat();
    if (pDrc->IsAlphaMask()) {
        format = FXDIB_8bppMask;
    } else if (format >= 1025) {
        format = FXDIB_Cmyka;
    } else if (format <= 32 || format == FXDIB_Argb) {
        format = FXDIB_Argb;
    } else {
        format = FXDIB_Rgba;
    }
    return format;
}
FX_BOOL CFX_ImageTransformer::Continue(IFX_Pause* pPause)
{
    if (m_Status == 1) {
        if (m_Stretcher.Continue(pPause)) {
            return TRUE;
        }
        if (m_Storer.GetBitmap()) {
            m_Storer.Replace(m_Storer.GetBitmap()->SwapXY(m_pMatrix->c > 0, m_pMatrix->b < 0));
        }
        return FALSE;
    } else if (m_Status == 2) {
        return m_Stretcher.Continue(pPause);
    } else if (m_Status != 3) {
        return FALSE;
    }
    if (m_Stretcher.Continue(pPause)) {
        return TRUE;
    }
    int stretch_width = m_StretchClip.Width();
    int stretch_height = m_StretchClip.Height();
    if (m_Storer.GetBitmap() == NULL) {
        return FALSE;
    }
    FX_LPCBYTE stretch_buf = m_Storer.GetBitmap()->GetBuffer();
    FX_LPCBYTE stretch_buf_mask = NULL;
    if (m_Storer.GetBitmap()->m_pAlphaMask) {
        stretch_buf_mask = m_Storer.GetBitmap()->m_pAlphaMask->GetBuffer();
    }
    int stretch_pitch = m_Storer.GetBitmap()->GetPitch();
    CFX_DIBitmap* pTransformed = new CFX_DIBitmap;
    FXDIB_Format transformF = _GetTransformedFormat(m_Stretcher.m_pSource);
    if (!pTransformed->Create(m_ResultWidth, m_ResultHeight, transformF)) {
        delete pTransformed;
        return FALSE;
    }
    pTransformed->Clear(0);
    if (pTransformed->m_pAlphaMask) {
        pTransformed->m_pAlphaMask->Clear(0);
    }
    CFX_AffineMatrix result2stretch(1.0f, 0.0f, 0.0f, 1.0f, (FX_FLOAT)(m_ResultLeft), (FX_FLOAT)(m_ResultTop));
    result2stretch.Concat(m_dest2stretch);
    result2stretch.TranslateI(-m_StretchClip.left, -m_StretchClip.top);
    if (stretch_buf_mask == NULL && pTransformed->m_pAlphaMask) {
        pTransformed->m_pAlphaMask->Clear(0xff000000);
    } else if (pTransformed->m_pAlphaMask) {
        int stretch_pitch_mask = m_Storer.GetBitmap()->m_pAlphaMask->GetPitch();
        if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
            CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_BYTE* dest_pos_mask = (FX_BYTE*)pTransformed->m_pAlphaMask->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col_l, src_row_l, res_x, res_y;
                    result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                    if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                        if (src_col_l == stretch_width) {
                            src_col_l--;
                        }
                        if (src_row_l == stretch_height) {
                            src_row_l--;
                        }
                        int src_col_r = src_col_l + 1;
                        int src_row_r = src_row_l + 1;
                        if (src_col_r == stretch_width) {
                            src_col_r--;
                        }
                        if (src_row_r == stretch_height) {
                            src_row_r--;
                        }
                        int row_offset_l = src_row_l * stretch_pitch_mask;
                        int row_offset_r = src_row_r * stretch_pitch_mask;
                        *dest_pos_mask = _bilinear_interpol(stretch_buf_mask, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, 1, 0);
                    }
                    dest_pos_mask++;
                }
            }
        } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
            CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_BYTE* dest_pos_mask = (FX_BYTE*)pTransformed->m_pAlphaMask->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col_l, src_row_l, res_x, res_y;
                    result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                    if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                        int pos_pixel[8];
                        int u_w[4], v_w[4];
                        if (src_col_l == stretch_width) {
                            src_col_l--;
                        }
                        if (src_row_l == stretch_height) {
                            src_row_l--;
                        }
                        _bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l, res_x, res_y, stretch_width, stretch_height);
                        *dest_pos_mask = _bicubic_interpol(stretch_buf_mask, stretch_pitch_mask, pos_pixel, u_w, v_w, res_x, res_y, 1, 0);
                    }
                    dest_pos_mask++;
                }
            }
        } else {
            CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_BYTE* dest_pos_mask = (FX_BYTE*)pTransformed->m_pAlphaMask->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col, src_row;
                    result2stretch_fix.Transform(col, row, src_col, src_row);
                    if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 && src_row <= stretch_height) {
                        if (src_col == stretch_width) {
                            src_col --;
                        }
                        if (src_row == stretch_height) {
                            src_row --;
                        }
                        *dest_pos_mask = stretch_buf_mask[src_row * stretch_pitch_mask + src_col];
                    }
                    dest_pos_mask++;
                }
            }
        }
    }
    if (m_Storer.GetBitmap()->IsAlphaMask()) {
        if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
            CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_LPBYTE dest_scan = (FX_LPBYTE)pTransformed->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col_l, src_row_l, res_x, res_y;
                    result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                    if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                        if (src_col_l == stretch_width) {
                            src_col_l--;
                        }
                        if (src_row_l == stretch_height) {
                            src_row_l--;
                        }
                        int src_col_r = src_col_l + 1;
                        int src_row_r = src_row_l + 1;
                        if (src_col_r == stretch_width) {
                            src_col_r--;
                        }
                        if (src_row_r == stretch_height) {
                            src_row_r--;
                        }
                        int row_offset_l = src_row_l * stretch_pitch;
                        int row_offset_r = src_row_r * stretch_pitch;
                        *dest_scan = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, 1, 0);
                    }
                    dest_scan ++;
                }
            }
        } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
            CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_LPBYTE dest_scan = (FX_LPBYTE)pTransformed->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col_l, src_row_l, res_x, res_y;
                    result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                    if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                        int pos_pixel[8];
                        int u_w[4], v_w[4];
                        if (src_col_l == stretch_width) {
                            src_col_l--;
                        }
                        if (src_row_l == stretch_height) {
                            src_row_l--;
                        }
                        _bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l, res_x, res_y, stretch_width, stretch_height);
                        *dest_scan = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, 1, 0);
                    }
                    dest_scan ++;
                }
            }
        } else {
            CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
            for (int row = 0; row < m_ResultHeight; row ++) {
                FX_LPBYTE dest_scan = (FX_LPBYTE)pTransformed->GetScanline(row);
                for (int col = 0; col < m_ResultWidth; col ++) {
                    int src_col, src_row;
                    result2stretch_fix.Transform(col, row, src_col, src_row);
                    if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 && src_row <= stretch_height) {
                        if (src_col == stretch_width) {
                            src_col --;
                        }
                        if (src_row == stretch_height) {
                            src_row --;
                        }
                        FX_LPCBYTE src_pixel = stretch_buf + stretch_pitch * src_row + src_col;
                        *dest_scan = *src_pixel;
                    }
                    dest_scan ++;
                }
            }
        }
    } else {
        int Bpp = m_Storer.GetBitmap()->GetBPP() / 8;
        int destBpp = pTransformed->GetBPP() / 8;
        if (Bpp == 1) {
            FX_DWORD argb[256];
            FX_ARGB* pPal = m_Storer.GetBitmap()->GetPalette();
            if (pPal) {
                for (int i = 0; i < 256; i ++) {
                    argb[i] = pPal[i];
                }
            } else {
                if (m_Storer.GetBitmap()->IsCmykImage())
                    for (int i = 0; i < 256; i ++) {
                        argb[i] = 255 - i;
                    }
                else
                    for (int i = 0; i < 256; i ++) {
                        argb[i] = 0xff000000 | (i * 0x010101);
                    }
            }
            if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
                CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col_l, src_row_l, res_x, res_y;
                        result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                        if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                            if (src_col_l == stretch_width) {
                                src_col_l--;
                            }
                            if (src_row_l == stretch_height) {
                                src_row_l--;
                            }
                            int src_col_r = src_col_l + 1;
                            int src_row_r = src_row_l + 1;
                            if (src_col_r == stretch_width) {
                                src_col_r--;
                            }
                            if (src_row_r == stretch_height) {
                                src_row_r--;
                            }
                            int row_offset_l = src_row_l * stretch_pitch;
                            int row_offset_r = src_row_r * stretch_pitch;
                            FX_DWORD r_bgra_cmyk = argb[_bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, 1, 0)];
                            if (transformF == FXDIB_Rgba) {
                                dest_pos[0] = (FX_BYTE)(r_bgra_cmyk >> 24);
                                dest_pos[1] = (FX_BYTE)(r_bgra_cmyk >> 16);
                                dest_pos[2] = (FX_BYTE)(r_bgra_cmyk >> 8);
                            } else {
                                *(FX_DWORD*)dest_pos = r_bgra_cmyk;
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col_l, src_row_l, res_x, res_y;
                        result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                        if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                            int pos_pixel[8];
                            int u_w[4], v_w[4];
                            if (src_col_l == stretch_width) {
                                src_col_l--;
                            }
                            if (src_row_l == stretch_height) {
                                src_row_l--;
                            }
                            _bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l, res_x, res_y, stretch_width, stretch_height);
                            FX_DWORD r_bgra_cmyk = argb[_bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, 1, 0)];
                            if (transformF == FXDIB_Rgba) {
                                dest_pos[0] = (FX_BYTE)(r_bgra_cmyk >> 24);
                                dest_pos[1] = (FX_BYTE)(r_bgra_cmyk >> 16);
                                dest_pos[2] = (FX_BYTE)(r_bgra_cmyk >> 8);
                            } else {
                                *(FX_DWORD*)dest_pos = r_bgra_cmyk;
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            } else {
                CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col, src_row;
                        result2stretch_fix.Transform(col, row, src_col, src_row);
                        if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 && src_row <= stretch_height) {
                            if (src_col == stretch_width) {
                                src_col --;
                            }
                            if (src_row == stretch_height) {
                                src_row --;
                            }
                            FX_DWORD r_bgra_cmyk = argb[stretch_buf[src_row * stretch_pitch + src_col]];
                            if (transformF == FXDIB_Rgba) {
                                dest_pos[0] = (FX_BYTE)(r_bgra_cmyk >> 24);
                                dest_pos[1] = (FX_BYTE)(r_bgra_cmyk >> 16);
                                dest_pos[2] = (FX_BYTE)(r_bgra_cmyk >> 8);
                            } else {
                                *(FX_DWORD*)dest_pos = r_bgra_cmyk;
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            }
        } else {
            FX_BOOL bHasAlpha = m_Storer.GetBitmap()->HasAlpha();
            int destBpp = pTransformed->GetBPP() / 8;
            if (!(m_Flags & FXDIB_DOWNSAMPLE) && !(m_Flags & FXDIB_BICUBIC_INTERPOL)) {
                CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col_l, src_row_l, res_x, res_y, r_pos_k_r = 0;
                        result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                        if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                            if (src_col_l == stretch_width) {
                                src_col_l--;
                            }
                            if (src_row_l == stretch_height) {
                                src_row_l--;
                            }
                            int src_col_r = src_col_l + 1;
                            int src_row_r = src_row_l + 1;
                            if (src_col_r == stretch_width) {
                                src_col_r--;
                            }
                            if (src_row_r == stretch_height) {
                                src_row_r--;
                            }
                            int row_offset_l = src_row_l * stretch_pitch;
                            int row_offset_r = src_row_r * stretch_pitch;
                            FX_BYTE r_pos_red_y_r   = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 2);
                            FX_BYTE r_pos_green_m_r = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 1);
                            FX_BYTE r_pos_blue_c_r  = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 0);
                            if (bHasAlpha) {
                                if (transformF != FXDIB_Argb) {
                                    if (transformF == FXDIB_Rgba) {
                                        dest_pos[0] = r_pos_blue_c_r;
                                        dest_pos[1] = r_pos_green_m_r;
                                        dest_pos[2] = r_pos_red_y_r;
                                    } else {
                                        r_pos_k_r = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 3);
                                        *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r, r_pos_red_y_r, r_pos_k_r));
                                    }
                                } else {
                                    FX_BYTE r_pos_a_r = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 3);
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(r_pos_a_r, r_pos_red_y_r, r_pos_green_m_r, r_pos_blue_c_r));
                                }
                            } else {
                                r_pos_k_r = 0xff;
                                if (transformF == FXDIB_Cmyka) {
                                    r_pos_k_r = _bilinear_interpol(stretch_buf, row_offset_l, row_offset_r, src_col_l, src_col_r, res_x, res_y, Bpp, 3);
                                    *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r, r_pos_red_y_r, r_pos_k_r));
                                } else {
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(r_pos_k_r, r_pos_red_y_r, r_pos_green_m_r, r_pos_blue_c_r));
                                }
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            } else if (m_Flags & FXDIB_BICUBIC_INTERPOL) {
                CFX_BilinearMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col_l, src_row_l, res_x, res_y, r_pos_k_r = 0;
                        result2stretch_fix.Transform(col, row, src_col_l, src_row_l, res_x, res_y);
                        if (src_col_l >= 0 && src_col_l <= stretch_width && src_row_l >= 0 && src_row_l <= stretch_height) {
                            int pos_pixel[8];
                            int u_w[4], v_w[4];
                            if (src_col_l == stretch_width) {
                                src_col_l--;
                            }
                            if (src_row_l == stretch_height) {
                                src_row_l--;
                            }
                            _bicubic_get_pos_weight(pos_pixel, u_w, v_w, src_col_l, src_row_l, res_x, res_y, stretch_width, stretch_height);
                            FX_BYTE r_pos_red_y_r   = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 2);
                            FX_BYTE r_pos_green_m_r = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 1);
                            FX_BYTE r_pos_blue_c_r  = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 0);
                            if (bHasAlpha) {
                                if (transformF != FXDIB_Argb) {
                                    if (transformF == FXDIB_Rgba) {
                                        dest_pos[0] = r_pos_blue_c_r;
                                        dest_pos[1] = r_pos_green_m_r;
                                        dest_pos[2] = r_pos_red_y_r;
                                    } else {
                                        r_pos_k_r = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 3);
                                        *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r, r_pos_red_y_r, r_pos_k_r));
                                    }
                                } else {
                                    FX_BYTE r_pos_a_r = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 3);
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(r_pos_a_r, r_pos_red_y_r, r_pos_green_m_r, r_pos_blue_c_r));
                                }
                            } else {
                                r_pos_k_r = 0xff;
                                if (transformF == FXDIB_Cmyka) {
                                    r_pos_k_r = _bicubic_interpol(stretch_buf, stretch_pitch, pos_pixel, u_w, v_w, res_x, res_y, Bpp, 3);
                                    *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(r_pos_blue_c_r, r_pos_green_m_r, r_pos_red_y_r, r_pos_k_r));
                                } else {
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(r_pos_k_r, r_pos_red_y_r, r_pos_green_m_r, r_pos_blue_c_r));
                                }
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            } else {
                CPDF_FixedMatrix result2stretch_fix(result2stretch, 8);
                for (int row = 0; row < m_ResultHeight; row ++) {
                    FX_BYTE* dest_pos = (FX_BYTE*)pTransformed->GetScanline(row);
                    for (int col = 0; col < m_ResultWidth; col ++) {
                        int src_col, src_row;
                        result2stretch_fix.Transform(col, row, src_col, src_row);
                        if (src_col >= 0 && src_col <= stretch_width && src_row >= 0 && src_row <= stretch_height) {
                            if (src_col == stretch_width) {
                                src_col --;
                            }
                            if (src_row == stretch_height) {
                                src_row --;
                            }
                            FX_LPCBYTE src_pos = stretch_buf + src_row * stretch_pitch + src_col * Bpp;
                            if (bHasAlpha) {
                                if (transformF != FXDIB_Argb) {
                                    if (transformF == FXDIB_Rgba) {
                                        dest_pos[0] = src_pos[0];
                                        dest_pos[1] = src_pos[1];
                                        dest_pos[2] = src_pos[2];
                                    } else {
                                        *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(src_pos[0], src_pos[1], src_pos[2], src_pos[3]));
                                    }
                                } else {
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(src_pos[3], src_pos[2], src_pos[1], src_pos[0]));
                                }
                            } else {
                                if (transformF == FXDIB_Cmyka) {
                                    *(FX_DWORD*)dest_pos = FXCMYK_TODIB(CmykEncode(src_pos[0], src_pos[1], src_pos[2], src_pos[3]));
                                } else {
                                    *(FX_DWORD*)dest_pos = FXARGB_TODIB(FXARGB_MAKE(0xff, src_pos[2], src_pos[1], src_pos[0]));
                                }
                            }
                        }
                        dest_pos += destBpp;
                    }
                }
            }
        }
    }
    m_Storer.Replace(pTransformed);
    return FALSE;
}
