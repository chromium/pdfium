// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <limits.h>
#include "../../../include/fxcrt/fx_basic.h"
#include "../../../include/fxcrt/fx_coordinates.h"
#include "../../../src/fxcrt/fx_safe_types.h"
#include "JBig2_Image.h"

CJBig2_Image::CJBig2_Image(FX_INT32 w, FX_INT32 h)
{
    m_nWidth	= w;
    m_nHeight	= h;
    if (m_nWidth <= 0 || m_nHeight <= 0 || m_nWidth > INT_MAX - 31) {
        m_pData = NULL;
        m_bNeedFree = FALSE;
        return;
    }
    m_nStride  = ((w + 31) >> 5) << 2;
    if (m_nStride * m_nHeight > 0 && 104857600 / (int)m_nStride > m_nHeight) {
        m_pData = (FX_BYTE *)m_pModule->JBig2_Malloc2(m_nStride, m_nHeight);
    } else {
        m_pData = NULL;
    }
    m_bNeedFree = TRUE;
}
CJBig2_Image::CJBig2_Image(FX_INT32 w, FX_INT32 h, FX_INT32 stride, FX_BYTE*pBuf)
{
    m_nWidth = w;
    m_nHeight = h;
    m_nStride = stride;
    m_pData = pBuf;
    m_bNeedFree = FALSE;
}
CJBig2_Image::CJBig2_Image(CJBig2_Image &im)
{
    m_pModule = im.m_pModule;
    m_nWidth	= im.m_nWidth;
    m_nHeight	= im.m_nHeight;
    m_nStride	= im.m_nStride;
    if (im.m_pData) {
        m_pData = (FX_BYTE*)m_pModule->JBig2_Malloc2(m_nStride, m_nHeight);
        JBIG2_memcpy(m_pData, im.m_pData, m_nStride * m_nHeight);
    } else {
        m_pData = NULL;
    }
    m_bNeedFree = TRUE;
}
CJBig2_Image::~CJBig2_Image()
{
    if(m_bNeedFree && m_pData) {
        m_pModule->JBig2_Free(m_pData);
    }
}
FX_BOOL CJBig2_Image::getPixel(FX_INT32 x, FX_INT32 y)
{
    if (!m_pData) {
        return 0;
    }
    FX_INT32 m, n;
    if(x < 0 || x >= m_nWidth) {
        return 0;
    }
    if(y < 0 || y >= m_nHeight) {
        return 0;
    }
    m = y * m_nStride + (x >> 3);
    n = x & 7;
    return ((m_pData[m] >> (7 - n)) & 1);
}

FX_INT32 CJBig2_Image::setPixel(FX_INT32 x, FX_INT32 y, FX_BOOL v)
{
    if (!m_pData) {
        return 0;
    }
    FX_INT32 m, n;
    if(x < 0 || x >= m_nWidth) {
        return 0;
    }
    if(y < 0 || y >= m_nHeight) {
        return 0;
    }
    m = y * m_nStride + (x >> 3);
    n = x & 7;
    if(v) {
        m_pData[m] |= 1 << (7 - n);
    } else {
        m_pData[m] &= ~(1 << (7 - n));
    }
    return 1;
}
void CJBig2_Image::copyLine(FX_INT32 hTo, FX_INT32 hFrom)
{
    if (!m_pData) {
        return;
    }
    if(hFrom < 0 || hFrom >= m_nHeight) {
        JBIG2_memset(m_pData + hTo * m_nStride, 0, m_nStride);
    } else {
        JBIG2_memcpy(m_pData + hTo * m_nStride, m_pData + hFrom * m_nStride, m_nStride);
    }
}
void CJBig2_Image::fill(FX_BOOL v)
{
    if (!m_pData) {
        return;
    }
    JBIG2_memset(m_pData, v ? 0xff : 0, m_nStride * m_nHeight);
}
FX_BOOL CJBig2_Image::composeTo(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op)
{
    if (!m_pData) {
        return FALSE;
    }
    return composeTo_opt2(pDst, x, y, op);
}
FX_BOOL CJBig2_Image::composeTo(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op, const FX_RECT* pSrcRect)
{
    if (!m_pData) {
        return FALSE;
    }
    if (NULL == pSrcRect || *pSrcRect == FX_RECT(0, 0, m_nWidth, m_nHeight)) {
        return composeTo_opt2(pDst, x, y, op);
    }
    return composeTo_opt2(pDst, x, y, op, pSrcRect);
}
FX_BOOL CJBig2_Image::composeTo_unopt(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op)
{
    FX_INT32 w, h, dx, dy;
    FX_INT32 i, j;
    w = m_nWidth;
    h = m_nHeight;
    dx = dy = 0;
    if(x < 0) {
        dx += -x;
        w  -= -x;
        x = 0;
    }
    if(y < 0) {
        dy += -y;
        h  -= -y;
        y = 0;
    }
    if(x + w > pDst->m_nWidth) {
        w = pDst->m_nWidth - x;
    }
    if(y + h > pDst->m_nHeight) {
        h = pDst->m_nHeight - y;
    }
    switch(op) {
        case JBIG2_COMPOSE_OR:
            for(j = 0; j < h; j++) {
                for(i = 0; i < w; i++) {
                    pDst->setPixel(x + i, y + j,
                                   (getPixel(i + dx, j + dy) | pDst->getPixel(x + i, y + j)) & 1);
                }
            }
            break;
        case JBIG2_COMPOSE_AND:
            for(j = 0; j < h; j++) {
                for(i = 0; i < w; i++) {
                    pDst->setPixel(x + i, y + j,
                                   (getPixel(i + dx, j + dy) & pDst->getPixel(x + i, y + j)) & 1);
                }
            }
            break;
        case JBIG2_COMPOSE_XOR:
            for(j = 0; j < h; j++) {
                for(i = 0; i < w; i++) {
                    pDst->setPixel(x + i, y + j,
                                   (getPixel(i + dx, j + dy) ^ pDst->getPixel(x + i, y + j)) & 1);
                }
            }
            break;
        case JBIG2_COMPOSE_XNOR:
            for(j = 0; j < h; j++) {
                for(i = 0; i < w; i++) {
                    pDst->setPixel(x + i, y + j,
                                   (~(getPixel(i + dx, j + dy) ^ pDst->getPixel(x + i, y + j))) & 1);
                }
            }
            break;
        case JBIG2_COMPOSE_REPLACE:
            for(j = 0; j < h; j++) {
                for(i = 0; i < w; i++) {
                    pDst->setPixel(x + i, y + j, getPixel(i + dx, j + dy));
                }
            }
            break;
    }
    return TRUE;
}

FX_BOOL CJBig2_Image::composeTo_opt(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op)
{
    FX_INT32 x0, x1, y0, y1, xx, yy;
    FX_BYTE *pLineSrc, *pLineDst, *srcPtr, *destPtr;
    FX_DWORD src0, src1, src, dest, s1, s2, m1, m2, m3;
    FX_BOOL oneByte;
    if (!m_pData) {
        return FALSE;
    }
    if (y < 0) {
        y0 = -y;
    } else {
        y0 = 0;
    }
    if (y + m_nHeight > pDst->m_nHeight) {
        y1 = pDst->m_nHeight - y;
    } else {
        y1 = m_nHeight;
    }
    if (y0 >= y1) {
        return FALSE;
    }
    if (x >= 0) {
        x0 = x & ~7;
    } else {
        x0 = 0;
    }
    x1 = x + m_nWidth;
    if (x1 > pDst->m_nWidth) {
        x1 = pDst->m_nWidth;
    }
    if (x0 >= x1) {
        return FALSE;
    }
    s1 = x & 7;
    s2 = 8 - s1;
    m1 = 0xff >> (x1 & 7);
    m2 = 0xff << (((x1 & 7) == 0) ? 0 : 8 - (x1 & 7));
    m3 = (0xff >> s1) & m2;
    oneByte = x0 == ((x1 - 1) & ~7);
    pLineDst = pDst->m_pData + y * pDst->m_nStride;
    pLineSrc = m_pData + y0 * m_nStride;
    if(oneByte) {
        if(x >= 0) {
            switch(op) {
                case JBIG2_COMPOSE_OR: {
                        for (yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            dest = *destPtr;
                            dest |= (*srcPtr >> s1) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_AND: {
                        for (yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            dest = *destPtr;
                            dest &= ((0xff00 | *srcPtr) >> s1) | m1;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XOR: {
                        for (yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            dest = *destPtr;
                            dest ^= (*srcPtr >> s1) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XNOR: {
                        for (yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            dest = *destPtr;
                            dest ^= ((*srcPtr ^ 0xff) >> s1) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_REPLACE: {
                        for (yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            dest = *destPtr;
                            dest = (dest & ~m3) | ((*srcPtr >> s1) & m3);
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
            }
        } else {
            switch(op) {
                case JBIG2_COMPOSE_OR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            dest = *destPtr;
                            dest |= *srcPtr & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_AND: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            dest = *destPtr;
                            dest &= *srcPtr | m1;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            dest = *destPtr;
                            dest ^= *srcPtr & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XNOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            dest = *destPtr;
                            dest ^= (*srcPtr ^ 0xff) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_REPLACE: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            dest = *destPtr;
                            dest = (*srcPtr & m2) | (dest & m1);
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
            }
        }
    } else {
        if(x >= 0) {
            switch(op) {
                case JBIG2_COMPOSE_OR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            src1 = *srcPtr++;
                            dest = *destPtr;
                            dest |= src1 >> s1;
                            *destPtr++ = (FX_BYTE)dest;
                            xx = x0 + 8;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest |= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest |= src & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_AND: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            src1 = *srcPtr++;
                            dest = *destPtr;
                            dest &= (0xff00 | src1) >> s1;
                            *destPtr++ = (FX_BYTE)dest;
                            xx = x0 + 8;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest &= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest &= src | m1;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            src1 = *srcPtr++;
                            dest = *destPtr;
                            dest ^= src1 >> s1;
                            *destPtr++ = (FX_BYTE)dest;
                            xx = x0 + 8;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest ^= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest ^= src & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XNOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            src1 = *srcPtr++;
                            dest = *destPtr;
                            dest ^= (src1 ^ 0xff) >> s1;
                            *destPtr++ = (FX_BYTE)dest;
                            xx = x0 + 8;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest ^= src ^ 0xff;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest ^= (src ^ 0xff) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_REPLACE: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst + (x >> 3);
                            srcPtr = pLineSrc;
                            src1 = *srcPtr++;
                            dest = *destPtr;
                            dest = (dest & (0xff << s2)) | (src1 >> s1);
                            *destPtr++ = (FX_BYTE)dest;
                            xx = x0 + 8;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest = src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest = (src & m2) | (dest & m1);
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
            }
        } else {
            switch(op) {
                case JBIG2_COMPOSE_OR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            src1 = *srcPtr++;
                            xx = x0;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest |= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest |= src & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_AND: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            src1 = *srcPtr++;
                            xx = x0;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest &= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest &= src | m1;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            src1 = *srcPtr++;
                            xx = x0;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest ^= src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest ^= src & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_XNOR: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            src1 = *srcPtr++;
                            xx = x0;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest ^= src ^ 0xff;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest ^= (src ^ 0xff) & m2;
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
                case JBIG2_COMPOSE_REPLACE: {
                        for(yy = y0; yy < y1; ++yy) {
                            destPtr = pLineDst;
                            srcPtr = pLineSrc + (-x >> 3);
                            src1 = *srcPtr++;
                            xx = x0;
                            for (; xx < x1 - 8; xx += 8) {
                                dest = *destPtr;
                                src0 = src1;
                                src1 = *srcPtr++;
                                src = (((src0 << 8) | src1) >> s1) & 0xff;
                                dest = src;
                                *destPtr++ = (FX_BYTE)dest;
                            }
                            dest = *destPtr;
                            src0 = src1;
                            if(srcPtr - pLineSrc < m_nStride) {
                                src1 = *srcPtr++;
                            } else {
                                src1 = 0;
                            }
                            src = (((src0 << 8) | src1) >> s1) & 0xff;
                            dest = (src & m2) | (dest & m1);
                            *destPtr = (FX_BYTE)dest;
                            pLineDst += pDst->m_nStride;
                            pLineSrc += m_nStride;
                        }
                    }
                    break;
            }
        }
    }
    return TRUE;
}
FX_BOOL CJBig2_Image::composeFrom(FX_INT32 x, FX_INT32 y, CJBig2_Image *pSrc, JBig2ComposeOp op)
{
    if (!m_pData) {
        return FALSE;
    }
    return pSrc->composeTo(this, x, y, op);
}
FX_BOOL CJBig2_Image::composeFrom(FX_INT32 x, FX_INT32 y, CJBig2_Image *pSrc, JBig2ComposeOp op, const FX_RECT* pSrcRect)
{
    if (!m_pData) {
        return FALSE;
    }
    return pSrc->composeTo(this, x, y, op, pSrcRect);
}
CJBig2_Image *CJBig2_Image::subImage_unopt(FX_INT32 x, FX_INT32 y, FX_INT32 w, FX_INT32 h)
{
    CJBig2_Image *pImage;
    FX_INT32 i, j;
    JBIG2_ALLOC(pImage, CJBig2_Image(w, h));
    for(j = 0; j < h; j++) {
        for(i = 0; i < w; i++) {
            pImage->setPixel(i, j, getPixel(x + i, y + j));
        }
    }
    return pImage;
}
#define JBIG2_GETDWORD(buf)	((FX_DWORD)(((buf)[0] << 24) | ((buf)[1] << 16) | ((buf)[2] << 8) | (buf)[3]))
CJBig2_Image *CJBig2_Image::subImage(FX_INT32 x, FX_INT32 y, FX_INT32 w, FX_INT32 h)
{
    CJBig2_Image *pImage;
    FX_INT32 m, n, j;
    FX_BYTE *pLineSrc, *pLineDst;
    FX_DWORD wTmp;
    FX_BYTE *pSrc, *pSrcEnd, *pDst, *pDstEnd;
    if (w == 0 || h == 0) {
        return NULL;
    }
    JBIG2_ALLOC(pImage, CJBig2_Image(w, h));
    if (!m_pData) {
        pImage->fill(0);
        return pImage;
    }
    if (!pImage->m_pData) {
        return pImage;
    }
    pLineSrc = m_pData + m_nStride * y;
    pLineDst = pImage->m_pData;
    m = (x >> 5) << 2;
    n = x & 31;
    if(n == 0) {
        for(j = 0; j < h; j++) {
            pSrc = pLineSrc + m;
            pSrcEnd = pLineSrc + m_nStride;
            pDst = pLineDst;
            pDstEnd = pLineDst + pImage->m_nStride;
            for(; pDst < pDstEnd; pSrc += 4, pDst += 4) {
                *((FX_DWORD *)pDst) = *((FX_DWORD *)pSrc);
            }
            pLineSrc += m_nStride;
            pLineDst += pImage->m_nStride;
        }
    } else {
        for(j = 0; j < h; j++) {
            pSrc = pLineSrc + m;
            pSrcEnd = pLineSrc + m_nStride;
            pDst = pLineDst;
            pDstEnd = pLineDst + pImage->m_nStride;
            for(; pDst < pDstEnd; pSrc += 4, pDst += 4) {
                if(pSrc + 4 < pSrcEnd) {
                    wTmp = (JBIG2_GETDWORD(pSrc) << n) | (JBIG2_GETDWORD(pSrc + 4) >> (32 - n));
                } else {
                    wTmp = JBIG2_GETDWORD(pSrc) << n;
                }
                pDst[0] = (FX_BYTE)(wTmp >> 24);
                pDst[1] = (FX_BYTE)(wTmp >> 16);
                pDst[2] = (FX_BYTE)(wTmp >> 8);
                pDst[3] = (FX_BYTE)wTmp;
            }
            pLineSrc += m_nStride;
            pLineDst += pImage->m_nStride;
        }
    }
    return pImage;
}
void CJBig2_Image::expand(FX_INT32 h, FX_BOOL v)
{
    if (!m_pData || h <= m_nHeight) {
        return;
    }
    FX_DWORD dwH = pdfium::base::checked_cast<FX_DWORD>(h);
    FX_DWORD dwStride = pdfium::base::checked_cast<FX_DWORD>(m_nStride);
    FX_DWORD dwHeight = pdfium::base::checked_cast<FX_DWORD>(m_nHeight);
    FX_SAFE_DWORD safeMemSize = dwH; 
    safeMemSize *= dwStride;
    if (!safeMemSize.IsValid()) {
        return;
    }
    //The guaranteed reallocated memory is to be < 4GB (unsigned int). 
    m_pData = (FX_BYTE*)m_pModule->JBig2_Realloc(m_pData, safeMemSize.ValueOrDie());
    //The result of dwHeight * dwStride doesn't overflow after the 
    //checking of safeMemSize.
    //The same as the result of (dwH - dwHeight) * dwStride) because
    //dwH - dwHeight is always less than dwH(h) which is checked in
    //the calculation of dwH * dwStride. 
    JBIG2_memset(m_pData + dwHeight * dwStride, v ? 0xff : 0, (dwH - dwHeight) * dwStride);
    m_nHeight = h;
}
FX_BOOL CJBig2_Image::composeTo_opt2(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op)
{
    FX_INT32 xs0 = 0, ys0  = 0, xs1  = 0, ys1   = 0, xd0    = 0, yd0          = 0, xd1      = 0, 
             yd1 = 0, xx   = 0, yy   = 0, w     = 0, h      = 0, middleDwords = 0, lineLeft = 0;

    FX_DWORD s1  = 0, d1   = 0, d2   = 0, shift = 0, shift1 = 0, shift2       = 0, 
             tmp = 0, tmp1 = 0, tmp2 = 0, maskL = 0, maskR  = 0, maskM        = 0;

    FX_BYTE *lineSrc = NULL, *lineDst = NULL, *sp = NULL, *dp = NULL;

    if (!m_pData) {
        return FALSE;
    }
    if (x < -1048576 || x > 1048576 || y < -1048576 || y > 1048576) {
        return FALSE;
    }
    if(y < 0) {
        ys0 = -y;
    }
    if(y + m_nHeight > pDst->m_nHeight) {
        ys1 = pDst->m_nHeight - y;
    } else {
        ys1 = m_nHeight;
    }
    if(x < 0) {
        xs0 = -x;
    }
    if(x + m_nWidth > pDst->m_nWidth) {
        xs1 = pDst->m_nWidth - x;
    } else {
        xs1 = m_nWidth;
    }
    if((ys0 >= ys1) || (xs0 >= xs1)) {
        return 0;
    }
    w = xs1 - xs0;
    h = ys1 - ys0;
    if(y >= 0) {
        yd0 = y;
    } 
    if(x >= 0) {
        xd0 = x;
    }
    xd1 = xd0 + w;
    yd1 = yd0 + h;
    d1 = xd0 & 31;
    d2 = xd1 & 31;
    s1 = xs0 & 31;
    maskL = 0xffffffff >> d1;
    maskR = 0xffffffff << ((32 - (xd1 & 31)) % 32);
    maskM = maskL & maskR;
    lineSrc = m_pData + ys0 * m_nStride + ((xs0 >> 5) << 2);
    lineLeft = m_nStride - ((xs0 >> 5) << 2);
    lineDst = pDst->m_pData + yd0 * pDst->m_nStride + ((xd0 >> 5) << 2);
    if((xd0 & ~31) == ((xd1 - 1) & ~31)) {
        if((xs0 & ~31) == ((xs1 - 1) & ~31)) {
            if(s1 > d1) {
                shift = s1 - d1;
                for(yy = yd0; yy < yd1; yy++) {
                    tmp1 = JBIG2_GETDWORD(lineSrc) << shift;
                    tmp2 = JBIG2_GETDWORD(lineDst);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                            break;
                    }
                    lineDst[0] = (FX_BYTE)(tmp >> 24);
                    lineDst[1] = (FX_BYTE)(tmp >> 16);
                    lineDst[2] = (FX_BYTE)(tmp >> 8);
                    lineDst[3] = (FX_BYTE)tmp;
                    lineSrc += m_nStride;
                    lineDst += pDst->m_nStride;
                }
            } else {
                shift = d1 - s1;
                for(yy = yd0; yy < yd1; yy++) {
                    tmp1 = JBIG2_GETDWORD(lineSrc) >> shift;
                    tmp2 = JBIG2_GETDWORD(lineDst);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                            break;
                    }
                    lineDst[0] = (FX_BYTE)(tmp >> 24);
                    lineDst[1] = (FX_BYTE)(tmp >> 16);
                    lineDst[2] = (FX_BYTE)(tmp >> 8);
                    lineDst[3] = (FX_BYTE)tmp;
                    lineSrc += m_nStride;
                    lineDst += pDst->m_nStride;
                }
            }
        } else {
            shift1 = s1 - d1;
            shift2 = 32 - shift1;
            for(yy = yd0; yy < yd1; yy++) {
                tmp1 = (JBIG2_GETDWORD(lineSrc) << shift1) | (JBIG2_GETDWORD(lineSrc + 4) >> shift2);
                tmp2 = JBIG2_GETDWORD(lineDst);
                switch(op) {
                    case JBIG2_COMPOSE_OR:
                        tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_AND:
                        tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_XOR:
                        tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_XNOR:
                        tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                        break;
                    case JBIG2_COMPOSE_REPLACE:
                        tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                        break;
                }
                lineDst[0] = (FX_BYTE)(tmp >> 24);
                lineDst[1] = (FX_BYTE)(tmp >> 16);
                lineDst[2] = (FX_BYTE)(tmp >> 8);
                lineDst[3] = (FX_BYTE)tmp;
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        }
    } else {
        if(s1 > d1) {
            shift1 = s1 - d1;
            shift2 = 32 - shift1;
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (JBIG2_GETDWORD(sp + 4) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (JBIG2_GETDWORD(sp + 4) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (
                               ((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        } else if(s1 == d1) {
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        } else {
            shift1 = d1 - s1;
            shift2 = 32 - shift1;
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp) >> shift1;
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift2) | ((JBIG2_GETDWORD(sp + 4)) >> shift1);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift2) | (
                               ((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >> shift1);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        }
    }
    return 1;
}
FX_BOOL CJBig2_Image::composeTo_opt2(CJBig2_Image *pDst, FX_INT32 x, FX_INT32 y, JBig2ComposeOp op, const FX_RECT* pSrcRect)
{
    FX_INT32 xs0, ys0, xs1, ys1, xd0, yd0, xd1, yd1, xx, yy, w, h, middleDwords, lineLeft;
    FX_DWORD s1, d1, d2, shift, shift1, shift2, tmp, tmp1, tmp2, maskL, maskR, maskM;
    FX_BYTE *lineSrc, *lineDst, *sp, *dp;
    FX_INT32 sw, sh;
    if (!m_pData) {
        return FALSE;
    }
    if (x < -1048576 || x > 1048576 || y < -1048576 || y > 1048576) {
        return FALSE;
    }
    sw = pSrcRect->Width();
    sh = pSrcRect->Height();
    if(y < 0) {
        ys0 = -y;
    } else {
        ys0 = 0;
    }
    if(y + sh > pDst->m_nHeight) {
        ys1 = pDst->m_nHeight - y;
    } else {
        ys1 = sh;
    }
    if(x < 0) {
        xs0 = -x;
    } else {
        xs0 = 0;
    }
    if(x + sw > pDst->m_nWidth) {
        xs1 = pDst->m_nWidth - x;
    } else {
        xs1 = sw;
    }
    if((ys0 >= ys1) || (xs0 >= xs1)) {
        return 0;
    }
    w = xs1 - xs0;
    h = ys1 - ys0;
    if(y < 0) {
        yd0 = 0;
    } else {
        yd0 = y;
    }
    if(x < 0) {
        xd0 = 0;
    } else {
        xd0 = x;
    }
    xd1 = xd0 + w;
    yd1 = yd0 + h;
    d1 = xd0 & 31;
    d2 = xd1 & 31;
    s1 = xs0 & 31;
    maskL = 0xffffffff >> d1;
    maskR = 0xffffffff << ((32 - (xd1 & 31)) % 32);
    maskM = maskL & maskR;
    lineSrc = m_pData + (pSrcRect->top + ys0) * m_nStride + (((xs0 + pSrcRect->left) >> 5) << 2);
    lineLeft = m_nStride - ((xs0 >> 5) << 2);
    lineDst = pDst->m_pData + yd0 * pDst->m_nStride + ((xd0 >> 5) << 2);
    if((xd0 & ~31) == ((xd1 - 1) & ~31)) {
        if((xs0 & ~31) == ((xs1 - 1) & ~31)) {
            if(s1 > d1) {
                shift = s1 - d1;
                for(yy = yd0; yy < yd1; yy++) {
                    tmp1 = JBIG2_GETDWORD(lineSrc) << shift;
                    tmp2 = JBIG2_GETDWORD(lineDst);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                            break;
                    }
                    lineDst[0] = (FX_BYTE)(tmp >> 24);
                    lineDst[1] = (FX_BYTE)(tmp >> 16);
                    lineDst[2] = (FX_BYTE)(tmp >> 8);
                    lineDst[3] = (FX_BYTE)tmp;
                    lineSrc += m_nStride;
                    lineDst += pDst->m_nStride;
                }
            } else {
                shift = d1 - s1;
                for(yy = yd0; yy < yd1; yy++) {
                    tmp1 = JBIG2_GETDWORD(lineSrc) >> shift;
                    tmp2 = JBIG2_GETDWORD(lineDst);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                            break;
                    }
                    lineDst[0] = (FX_BYTE)(tmp >> 24);
                    lineDst[1] = (FX_BYTE)(tmp >> 16);
                    lineDst[2] = (FX_BYTE)(tmp >> 8);
                    lineDst[3] = (FX_BYTE)tmp;
                    lineSrc += m_nStride;
                    lineDst += pDst->m_nStride;
                }
            }
        } else {
            shift1 = s1 - d1;
            shift2 = 32 - shift1;
            for(yy = yd0; yy < yd1; yy++) {
                tmp1 = (JBIG2_GETDWORD(lineSrc) << shift1) | (JBIG2_GETDWORD(lineSrc + 4) >> shift2);
                tmp2 = JBIG2_GETDWORD(lineDst);
                switch(op) {
                    case JBIG2_COMPOSE_OR:
                        tmp = (tmp2 & ~maskM) | ((tmp1 | tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_AND:
                        tmp = (tmp2 & ~maskM) | ((tmp1 & tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_XOR:
                        tmp = (tmp2 & ~maskM) | ((tmp1 ^ tmp2) & maskM);
                        break;
                    case JBIG2_COMPOSE_XNOR:
                        tmp = (tmp2 & ~maskM) | ((~(tmp1 ^ tmp2)) & maskM);
                        break;
                    case JBIG2_COMPOSE_REPLACE:
                        tmp = (tmp2 & ~maskM) | (tmp1 & maskM);
                        break;
                }
                lineDst[0] = (FX_BYTE)(tmp >> 24);
                lineDst[1] = (FX_BYTE)(tmp >> 16);
                lineDst[2] = (FX_BYTE)(tmp >> 8);
                lineDst[3] = (FX_BYTE)tmp;
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        }
    } else {
        if(s1 > d1) {
            shift1 = s1 - d1;
            shift2 = 32 - shift1;
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (JBIG2_GETDWORD(sp + 4) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (JBIG2_GETDWORD(sp + 4) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift1) | (
                               ((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >> shift2);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        } else if(s1 == d1) {
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        } else {
            shift1 = d1 - s1;
            shift2 = 32 - shift1;
            middleDwords = (xd1 >> 5) - ((xd0 + 31) >> 5);
            for(yy = yd0; yy < yd1; yy++) {
                sp = lineSrc;
                dp = lineDst;
                if(d1 != 0) {
                    tmp1 = JBIG2_GETDWORD(sp) >> shift1;
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 | tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskL) | ((tmp1 & tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskL) | ((tmp1 ^ tmp2) & maskL);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskL) | ((~(tmp1 ^ tmp2)) & maskL);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskL) | (tmp1 & maskL);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    dp += 4;
                }
                for(xx = 0; xx < middleDwords; xx++) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift2) | ((JBIG2_GETDWORD(sp + 4)) >> shift1);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = tmp1 | tmp2;
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = tmp1 & tmp2;
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = tmp1 ^ tmp2;
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = ~(tmp1 ^ tmp2);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = tmp1;
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                    sp += 4;
                    dp += 4;
                }
                if(d2 != 0) {
                    tmp1 = (JBIG2_GETDWORD(sp) << shift2) | (
                               ((sp + 4) < lineSrc + lineLeft ? JBIG2_GETDWORD(sp + 4) : 0) >> shift1);
                    tmp2 = JBIG2_GETDWORD(dp);
                    switch(op) {
                        case JBIG2_COMPOSE_OR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 | tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_AND:
                            tmp = (tmp2 & ~maskR) | ((tmp1 & tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XOR:
                            tmp = (tmp2 & ~maskR) | ((tmp1 ^ tmp2) & maskR);
                            break;
                        case JBIG2_COMPOSE_XNOR:
                            tmp = (tmp2 & ~maskR) | ((~(tmp1 ^ tmp2)) & maskR);
                            break;
                        case JBIG2_COMPOSE_REPLACE:
                            tmp = (tmp2 & ~maskR) | (tmp1 & maskR);
                            break;
                    }
                    dp[0] = (FX_BYTE)(tmp >> 24);
                    dp[1] = (FX_BYTE)(tmp >> 16);
                    dp[2] = (FX_BYTE)(tmp >> 8);
                    dp[3] = (FX_BYTE)tmp;
                }
                lineSrc += m_nStride;
                lineDst += pDst->m_nStride;
            }
        }
    }
    return 1;
}
