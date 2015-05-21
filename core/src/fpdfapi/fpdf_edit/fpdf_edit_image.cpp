// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "../../../include/fpdfapi/fpdf_render.h"
#include "../fpdf_page/pageint.h"
#include "../fpdf_render/render_int.h"
CPDF_Dictionary* CPDF_Image::InitJPEG(FX_LPBYTE pData, FX_DWORD size)
{
    FX_INT32 width, height, color_trans, num_comps, bits;
    if (!CPDF_ModuleMgr::Get()->GetJpegModule()->
            LoadInfo(pData, size, width, height, num_comps, bits, color_trans)) {
        return NULL;
    }
    CPDF_Dictionary* pDict = new CPDF_Dictionary;
    pDict->SetAtName("Type", "XObject");
    pDict->SetAtName("Subtype", "Image");
    pDict->SetAtInteger("Width", width);
    pDict->SetAtInteger("Height", height);
    FX_LPCSTR csname = NULL;
    if (num_comps == 1) {
        csname = "DeviceGray";
    } else if (num_comps == 3) {
        csname = "DeviceRGB";
    } else if (num_comps == 4) {
        csname = "DeviceCMYK";
        CPDF_Array* pDecode = CPDF_Array::Create();
        for (int n = 0; n < 4; n ++) {
            pDecode->AddInteger(1);
            pDecode->AddInteger(0);
        }
        pDict->SetAt(FX_BSTRC("Decode"), pDecode);
    }
    pDict->SetAtName("ColorSpace", csname);
    pDict->SetAtInteger("BitsPerComponent", bits);
    pDict->SetAtName("Filter", "DCTDecode");
    if (!color_trans) {
        CPDF_Dictionary* pParms = new CPDF_Dictionary;
        pDict->SetAt("DecodeParms", pParms);
        pParms->SetAtInteger("ColorTransform", 0);
    }
    m_bIsMask = FALSE;
    m_Width = width;
    m_Height = height;
    if (m_pStream == NULL) {
        m_pStream = new CPDF_Stream(NULL, 0, NULL);
    }
    return pDict;
}
void CPDF_Image::SetJpegImage(FX_LPBYTE pData, FX_DWORD size)
{
    CPDF_Dictionary *pDict = InitJPEG(pData, size);
    if (!pDict) {
        return;
    }
    m_pStream->InitStream(pData, size, pDict);
}
void CPDF_Image::SetJpegImage(IFX_FileRead *pFile)
{
    FX_DWORD size = (FX_DWORD)pFile->GetSize();
    if (!size) {
        return;
    }
    FX_DWORD dwEstimateSize = size;
    if (dwEstimateSize > 8192) {
        dwEstimateSize = 8192;
    }
    FX_LPBYTE pData = FX_Alloc(FX_BYTE, dwEstimateSize);
    pFile->ReadBlock(pData, 0, dwEstimateSize);
    CPDF_Dictionary *pDict = InitJPEG(pData, dwEstimateSize);
    FX_Free(pData);
    if (!pDict && size > dwEstimateSize) {
        pData = FX_Alloc(FX_BYTE, size);
        pFile->ReadBlock(pData, 0, size);
        pDict = InitJPEG(pData, size);
        FX_Free(pData);
    }
    if (!pDict) {
        return;
    }
    m_pStream->InitStream(pFile, pDict);
}
void _DCTEncodeBitmap(CPDF_Dictionary *pBitmapDict, const CFX_DIBitmap* pBitmap, int quality, FX_LPBYTE &buf, FX_STRSIZE &size)
{
}
void _JBIG2EncodeBitmap(CPDF_Dictionary *pBitmapDict, const CFX_DIBitmap *pBitmap, CPDF_Document *pDoc, FX_LPBYTE &buf, FX_STRSIZE &size, FX_BOOL bLossLess)
{
}
void CPDF_Image::SetImage(const CFX_DIBitmap* pBitmap, FX_INT32 iCompress, IFX_FileWrite *pFileWrite, IFX_FileRead *pFileRead, const CFX_DIBitmap* pMask, const CPDF_ImageSetParam* pParam)
{
    FX_INT32 BitmapWidth = pBitmap->GetWidth();
    FX_INT32 BitmapHeight = pBitmap->GetHeight();
    if (BitmapWidth < 1 || BitmapHeight < 1) {
        return;
    }
    FX_LPBYTE src_buf = pBitmap->GetBuffer();
    FX_INT32 src_pitch = pBitmap->GetPitch();
    FX_INT32 bpp = pBitmap->GetBPP();
    FX_BOOL bUseMatte = pParam && pParam->pMatteColor && (pBitmap->GetFormat() == FXDIB_Argb);
    CPDF_Dictionary* pDict = new CPDF_Dictionary;
    pDict->SetAtName(FX_BSTRC("Type"), FX_BSTRC("XObject"));
    pDict->SetAtName(FX_BSTRC("Subtype"), FX_BSTRC("Image"));
    pDict->SetAtInteger(FX_BSTRC("Width"), BitmapWidth);
    pDict->SetAtInteger(FX_BSTRC("Height"), BitmapHeight);
    FX_LPBYTE dest_buf = NULL;
    FX_STRSIZE dest_pitch = 0, dest_size = 0, opType = -1;
    if (bpp == 1) {
        FX_INT32 reset_a = 0, reset_r = 0, reset_g = 0, reset_b = 0;
        FX_INT32 set_a = 0, set_r = 0, set_g = 0, set_b = 0;
        if (!pBitmap->IsAlphaMask()) {
            ArgbDecode(pBitmap->GetPaletteArgb(0), reset_a, reset_r, reset_g, reset_b);
            ArgbDecode(pBitmap->GetPaletteArgb(1), set_a, set_r, set_g, set_b);
        }
        if (set_a == 0 || reset_a == 0) {
            pDict->SetAt(FX_BSTRC("ImageMask"), new CPDF_Boolean(TRUE));
            if (reset_a == 0) {
                CPDF_Array* pArray = new CPDF_Array;
                pArray->AddInteger(1);
                pArray->AddInteger(0);
                pDict->SetAt(FX_BSTRC("Decode"), pArray);
            }
        } else {
            CPDF_Array* pCS = new CPDF_Array;
            pCS->AddName(FX_BSTRC("Indexed"));
            pCS->AddName(FX_BSTRC("DeviceRGB"));
            pCS->AddInteger(1);
            CFX_ByteString ct;
            FX_LPSTR pBuf = ct.GetBuffer(6);
            pBuf[0] = (FX_CHAR)reset_r;
            pBuf[1] = (FX_CHAR)reset_g;
            pBuf[2] = (FX_CHAR)reset_b;
            pBuf[3] = (FX_CHAR)set_r;
            pBuf[4] = (FX_CHAR)set_g;
            pBuf[5] = (FX_CHAR)set_b;
            ct.ReleaseBuffer(6);
            pCS->Add(CPDF_String::Create(ct, TRUE));
            pDict->SetAt(FX_BSTRC("ColorSpace"), pCS);
        }
        pDict->SetAtInteger(FX_BSTRC("BitsPerComponent"), 1);
        dest_pitch = (BitmapWidth + 7) / 8;
        if ((iCompress & 0x03) == PDF_IMAGE_NO_COMPRESS) {
            opType = 1;
        } else {
            opType = 0;
        }
    } else if (bpp == 8) {
        FX_INT32 iPalette = pBitmap->GetPaletteSize();
        if (iPalette > 0) {
            CPDF_Array* pCS = new CPDF_Array;
            m_pDocument->AddIndirectObject(pCS);
            pCS->AddName(FX_BSTRC("Indexed"));
            pCS->AddName(FX_BSTRC("DeviceRGB"));
            pCS->AddInteger(iPalette - 1);
            FX_LPBYTE pColorTable = FX_Alloc2D(FX_BYTE, iPalette, 3);
            FX_LPBYTE ptr = pColorTable;
            for (FX_INT32 i = 0; i < iPalette; i ++) {
                FX_DWORD argb = pBitmap->GetPaletteArgb(i);
                ptr[0] = (FX_BYTE)(argb >> 16);
                ptr[1] = (FX_BYTE)(argb >> 8);
                ptr[2] = (FX_BYTE)argb;
                ptr += 3;
            }
            CPDF_Stream *pCTS = CPDF_Stream::Create(pColorTable, iPalette * 3, CPDF_Dictionary::Create());
            m_pDocument->AddIndirectObject(pCTS);
            pCS->AddReference(m_pDocument, pCTS);
            pDict->SetAtReference(FX_BSTRC("ColorSpace"), m_pDocument, pCS);
        } else {
            pDict->SetAtName(FX_BSTRC("ColorSpace"), FX_BSTRC("DeviceGray"));
        }
        pDict->SetAtInteger(FX_BSTRC("BitsPerComponent"), 8);
        if ((iCompress & 0x03) == PDF_IMAGE_NO_COMPRESS) {
            dest_pitch = BitmapWidth;
            opType = 1;
        } else {
            opType = 0;
        }
    } else {
        pDict->SetAtName(FX_BSTRC("ColorSpace"), FX_BSTRC("DeviceRGB"));
        pDict->SetAtInteger(FX_BSTRC("BitsPerComponent"), 8);
        if ((iCompress & 0x03) == PDF_IMAGE_NO_COMPRESS) {
            dest_pitch = BitmapWidth * 3;
            opType = 2;
        } else {
            opType = 0;
        }
    }
    const CFX_DIBitmap* pMaskBitmap = NULL;
    FX_BOOL bDeleteMask = FALSE;
    if (pBitmap->HasAlpha()) {
        pMaskBitmap = pBitmap->GetAlphaMask();
        bDeleteMask = TRUE;
    }
    if (!pMaskBitmap && pMask) {
        FXDIB_Format maskFormat = pMask->GetFormat();
        if (maskFormat == FXDIB_1bppMask || maskFormat == FXDIB_8bppMask) {
            pMaskBitmap = pMask;
        }
    }
    if (pMaskBitmap) {
        FX_INT32 maskWidth = pMaskBitmap->GetWidth();
        FX_INT32 maskHeight = pMaskBitmap->GetHeight();
        FX_LPBYTE mask_buf = NULL;
        FX_STRSIZE mask_size;
        CPDF_Dictionary* pMaskDict = new CPDF_Dictionary;
        pMaskDict->SetAtName(FX_BSTRC("Type"), FX_BSTRC("XObject"));
        pMaskDict->SetAtName(FX_BSTRC("Subtype"), FX_BSTRC("Image"));
        pMaskDict->SetAtInteger(FX_BSTRC("Width"), maskWidth);
        pMaskDict->SetAtInteger(FX_BSTRC("Height"), maskHeight);
        pMaskDict->SetAtName(FX_BSTRC("ColorSpace"), FX_BSTRC("DeviceGray"));
        pMaskDict->SetAtInteger(FX_BSTRC("BitsPerComponent"), 8);
        if (pMaskBitmap->GetBPP() == 8 && (iCompress & PDF_IMAGE_MASK_LOSSY_COMPRESS) != 0) {
            _DCTEncodeBitmap(pMaskDict, pMaskBitmap, pParam ? pParam->nQuality : 75, mask_buf, mask_size);
        } else if (pMaskBitmap->GetFormat() == FXDIB_1bppMask) {
            _JBIG2EncodeBitmap(pMaskDict, pMaskBitmap, m_pDocument, mask_buf, mask_size, TRUE);
        } else {
            mask_buf = FX_Alloc2D(FX_BYTE, maskHeight, maskWidth);
            mask_size = maskHeight * maskWidth;  // Safe since checked alloc returned.
            for (FX_INT32 a = 0; a < maskHeight; a ++) {
                FXSYS_memcpy32(mask_buf + a * maskWidth, pMaskBitmap->GetScanline(a), maskWidth);
            }
        }
        pMaskDict->SetAtInteger(FX_BSTRC("Length"), mask_size);
        if (bUseMatte) {
            int a, r, g, b;
            ArgbDecode(*(pParam->pMatteColor), a, r, g, b);
            CPDF_Array* pMatte = new CPDF_Array;
            pMatte->AddInteger(r);
            pMatte->AddInteger(g);
            pMatte->AddInteger(b);
            pMaskDict->SetAt(FX_BSTRC("Matte"), pMatte);
        }
        CPDF_Stream* pMaskStream = new CPDF_Stream(mask_buf, mask_size, pMaskDict);
        m_pDocument->AddIndirectObject(pMaskStream);
        pDict->SetAtReference(FX_BSTRC("SMask"), m_pDocument, pMaskStream);
        if (bDeleteMask) {
            delete pMaskBitmap;
        }
    }
    FX_BOOL bStream = pFileWrite != NULL && pFileRead != NULL;
    if (opType == 0) {
        if (iCompress & PDF_IMAGE_LOSSLESS_COMPRESS) {
            if (pBitmap->GetBPP() == 1) {
                _JBIG2EncodeBitmap(pDict, pBitmap, m_pDocument, dest_buf, dest_size, TRUE);
            }
        } else {
            if (pBitmap->GetBPP() == 1) {
                _JBIG2EncodeBitmap(pDict, pBitmap, m_pDocument, dest_buf, dest_size, FALSE);
            } else if (pBitmap->GetBPP() >= 8 && pBitmap->GetPalette() != NULL) {
                CFX_DIBitmap *pNewBitmap = new CFX_DIBitmap();
                pNewBitmap->Copy(pBitmap);
                pNewBitmap->ConvertFormat(FXDIB_Rgb);
                SetImage(pNewBitmap, iCompress, pFileWrite, pFileRead);
                if (pDict) {
                    pDict->Release();
                    pDict = NULL;
                }
                if (dest_buf) {
                    FX_Free(dest_buf);
                    dest_buf = NULL;
                }
                dest_size = 0;
                delete pNewBitmap;
                return;
            } else {
                if (bUseMatte) {
                    CFX_DIBitmap *pNewBitmap = new CFX_DIBitmap();
                    pNewBitmap->Create(BitmapWidth, BitmapHeight, FXDIB_Argb);
                    FX_LPBYTE dst_buf = pNewBitmap->GetBuffer();
                    FX_INT32 src_offset = 0;
                    for (FX_INT32 row = 0; row < BitmapHeight; row ++) {
                        src_offset = row * src_pitch;
                        for (FX_INT32 column = 0; column < BitmapWidth; column ++) {
                            FX_FLOAT alpha = src_buf[src_offset + 3] / 255.0f;
                            dst_buf[src_offset] = (FX_BYTE)(src_buf[src_offset] * alpha);
                            dst_buf[src_offset + 1] = (FX_BYTE)(src_buf[src_offset + 1] * alpha);
                            dst_buf[src_offset + 2] = (FX_BYTE)(src_buf[src_offset + 2] * alpha);
                            dst_buf[src_offset + 3] = (FX_BYTE)(src_buf[src_offset + 3]);
                            src_offset += 4;
                        }
                    }
                    _DCTEncodeBitmap(pDict, pNewBitmap, pParam ? pParam->nQuality : 75, dest_buf, dest_size);
                    delete pNewBitmap;
                } else {
                    _DCTEncodeBitmap(pDict, pBitmap, pParam ? pParam->nQuality : 75, dest_buf, dest_size);
                }
            }
        }
        if (bStream) {
            pFileWrite->WriteBlock(dest_buf, dest_size);
            FX_Free(dest_buf);
            dest_buf = NULL;
        }
    } else if (opType == 1) {
        if (!bStream) {
            dest_buf = FX_Alloc2D(FX_BYTE, dest_pitch, BitmapHeight);
            dest_size = dest_pitch * BitmapHeight;  // Safe since checked alloc returned.
        }
        FX_LPBYTE pDest = dest_buf;
        for (FX_INT32 i = 0; i < BitmapHeight; i ++) {
            if (!bStream) {
                FXSYS_memcpy32(pDest, src_buf, dest_pitch);
                pDest += dest_pitch;
            } else {
                pFileWrite->WriteBlock(src_buf, dest_pitch);
            }
            src_buf += src_pitch;
        }
    } else if (opType == 2) {
        if (!bStream) {
            dest_buf = FX_Alloc2D(FX_BYTE, dest_pitch, BitmapHeight);
            dest_size = dest_pitch * BitmapHeight;  // Safe since checked alloc returned.
        } else {
            dest_buf = FX_Alloc(FX_BYTE, dest_pitch);
        }
        FX_LPBYTE pDest = dest_buf;
        FX_INT32 src_offset = 0;
        FX_INT32 dest_offset = 0;
        for (FX_INT32 row = 0; row < BitmapHeight; row ++) {
            src_offset = row * src_pitch;
            for (FX_INT32 column = 0; column < BitmapWidth; column ++) {
                FX_FLOAT alpha = bUseMatte ? src_buf[src_offset + 3] / 255.0f : 1;
                pDest[dest_offset] = (FX_BYTE)(src_buf[src_offset + 2] * alpha);
                pDest[dest_offset + 1] = (FX_BYTE)(src_buf[src_offset + 1] * alpha);
                pDest[dest_offset + 2] = (FX_BYTE)(src_buf[src_offset] * alpha);
                dest_offset += 3;
                src_offset += bpp == 24 ? 3 : 4;
            }
            if (bStream) {
                pFileWrite->WriteBlock(pDest, dest_pitch);
                pDest = dest_buf;
            } else {
                pDest += dest_pitch;
            }
            dest_offset = 0;
        }
        if (bStream) {
            FX_Free(dest_buf);
            dest_buf = NULL;
        }
    }
    if (m_pStream == NULL) {
        m_pStream = new CPDF_Stream(NULL, 0, NULL);
    }
    if (!bStream) {
        m_pStream->InitStream(dest_buf, dest_size, pDict);
    } else {
        pFileWrite->Flush();
        m_pStream->InitStream(pFileRead, pDict);
    }
    m_bIsMask = pBitmap->IsAlphaMask();
    m_Width = BitmapWidth;
    m_Height = BitmapHeight;
    if (dest_buf) {
        FX_Free(dest_buf);
    }
}
void CPDF_Image::ResetCache(CPDF_Page* pPage, const CFX_DIBitmap* pBitmap)
{
    pPage->GetRenderCache()->ResetBitmap(m_pStream, pBitmap);
}
