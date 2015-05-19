// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_render.h"
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#include "../../../src/fxcrt/fx_safe_types.h"
#include "../fpdf_page/pageint.h"
#include "render_int.h"

namespace {

unsigned int _GetBits8(FX_LPCBYTE pData, int bitpos, int nbits)
{
    unsigned int byte = pData[bitpos / 8];
    if (nbits == 8) {
        return byte;
    } else if (nbits == 4) {
        return (bitpos % 8) ? (byte & 0x0f) : (byte >> 4);
    } else if (nbits == 2) {
        return (byte >> (6 - bitpos % 8)) & 0x03;
    } else if (nbits == 1) {
        return (byte >> (7 - bitpos % 8)) & 0x01;
    } else if (nbits == 16) {
        return byte * 256 + pData[bitpos / 8 + 1];
    }
    return 0;
}

FX_SAFE_DWORD CalculatePitch8(FX_DWORD bpc,
                              FX_DWORD components,
                              int width,
                              int height)
{
    FX_SAFE_DWORD pitch = bpc;
    pitch *= components;
    pitch *= width;
    pitch += 7;
    pitch /= 8;
    pitch *= height;
    return pitch;
}

FX_SAFE_DWORD CalculatePitch32(int bpp, int width)
{
    FX_SAFE_DWORD pitch = bpp;
    pitch *= width;
    pitch += 31;
    pitch /= 8;
    return pitch;
}

}  // namespace

CFX_DIBSource* CPDF_Image::LoadDIBSource(CFX_DIBSource** ppMask, FX_DWORD* pMatteColor, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask) const
{
    CPDF_DIBSource* pSource = new CPDF_DIBSource;
    if (pSource->Load(m_pDocument, m_pStream, (CPDF_DIBSource**)ppMask, pMatteColor, NULL, NULL, bStdCS, GroupFamily, bLoadMask)) {
        return pSource;
    }
    delete pSource;
    return NULL;
}
CFX_DIBSource* CPDF_Image::DetachBitmap()
{
    CFX_DIBSource* pBitmap = m_pDIBSource;
    m_pDIBSource = NULL;
    return pBitmap;
}
CFX_DIBSource* CPDF_Image::DetachMask()
{
    CFX_DIBSource* pBitmap = m_pMask;
    m_pMask = NULL;
    return pBitmap;
}
FX_BOOL CPDF_Image::StartLoadDIBSource(CPDF_Dictionary* pFormResource, CPDF_Dictionary* pPageResource, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask)
{
    m_pDIBSource = new CPDF_DIBSource;
    int ret = ((CPDF_DIBSource*)m_pDIBSource)->StartLoadDIBSource(m_pDocument, m_pStream, TRUE, pFormResource, pPageResource, bStdCS, GroupFamily, bLoadMask);
    if (ret == 2) {
        return TRUE;
    }
    if (!ret) {
        delete m_pDIBSource;
        m_pDIBSource = NULL;
        return FALSE;
    }
    m_pMask = ((CPDF_DIBSource*)m_pDIBSource)->DetachMask();
    m_MatteColor = ((CPDF_DIBSource*)m_pDIBSource)->m_MatteColor;
    return FALSE;
}
FX_BOOL CPDF_Image::Continue(IFX_Pause* pPause)
{
    int ret = ((CPDF_DIBSource*)m_pDIBSource)->ContinueLoadDIBSource(pPause);
    if (ret == 2) {
        return TRUE;
    }
    if (!ret) {
        delete m_pDIBSource;
        m_pDIBSource = NULL;
        return FALSE;
    }
    m_pMask = ((CPDF_DIBSource*)m_pDIBSource)->DetachMask();
    m_MatteColor = ((CPDF_DIBSource*)m_pDIBSource)->m_MatteColor;
    return FALSE;
}
CPDF_DIBSource::CPDF_DIBSource()
{
    m_pDocument = NULL;
    m_pStreamAcc = NULL;
    m_pDict = NULL;
    m_bpp = 0;
    m_Width = m_Height = 0;
    m_pColorSpace = NULL;
    m_bDefaultDecode = TRUE;
    m_bImageMask = FALSE;
    m_bDoBpcCheck = TRUE;
    m_pPalette = NULL;
    m_pCompData = NULL;
    m_bColorKey = FALSE;
    m_pMaskedLine = m_pLineBuf = NULL;
    m_pCachedBitmap = NULL;
    m_pDecoder = NULL;
    m_nComponents = 0;
    m_bpc = 0;
    m_bLoadMask = FALSE;
    m_Family = 0;
    m_pMask = NULL;
    m_MatteColor = 0;
    m_pJbig2Context = NULL;
    m_pGlobalStream = NULL;
    m_bStdCS = FALSE;
    m_pMaskStream = NULL;
    m_Status = 0;
    m_bHasMask = FALSE;
}
CPDF_DIBSource::~CPDF_DIBSource()
{
    delete m_pStreamAcc;
    if (m_pMaskedLine) {
        FX_Free(m_pMaskedLine);
    }
    if (m_pLineBuf) {
        FX_Free(m_pLineBuf);
    }
    delete m_pCachedBitmap;
    delete m_pDecoder;
    if (m_pCompData) {
        FX_Free(m_pCompData);
    }
    CPDF_ColorSpace* pCS = m_pColorSpace;
    if (pCS && m_pDocument) {
        m_pDocument->GetPageData()->ReleaseColorSpace(pCS->GetArray());
    }
    if (m_pJbig2Context) {
        ICodec_Jbig2Module* pJbig2Module = CPDF_ModuleMgr::Get()->GetJbig2Module();
        pJbig2Module->DestroyJbig2Context(m_pJbig2Context);
    }
    delete m_pGlobalStream;
}
CFX_DIBitmap* CPDF_DIBSource::GetBitmap() const
{
    if (m_pCachedBitmap) {
        return m_pCachedBitmap;
    }
    return Clone();
}
void CPDF_DIBSource::ReleaseBitmap(CFX_DIBitmap* pBitmap) const
{
    if (pBitmap && pBitmap != m_pCachedBitmap) {
        delete pBitmap;
    }
}
FX_BOOL CPDF_DIBSource::Load(CPDF_Document* pDoc, const CPDF_Stream* pStream, CPDF_DIBSource** ppMask,
                             FX_DWORD* pMatteColor, CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask)
{
    if (pStream == NULL) {
        return FALSE;
    }
    m_pDocument = pDoc;
    m_pDict = pStream->GetDict();
    if (m_pDict == NULL) {
        return FALSE;
    }
    m_pStream = pStream;
    m_Width = m_pDict->GetInteger(FX_BSTRC("Width"));
    m_Height = m_pDict->GetInteger(FX_BSTRC("Height"));
    if (m_Width <= 0 || m_Height <= 0 || m_Width > 0x01ffff || m_Height > 0x01ffff) {
        return FALSE;
    }
    m_GroupFamily = GroupFamily;
    m_bLoadMask = bLoadMask;
    if (!LoadColorInfo(m_pStream->GetObjNum() != 0 ? NULL : pFormResources, pPageResources)) {
        return FALSE;
    }
    if (m_bDoBpcCheck && (m_bpc == 0 || m_nComponents == 0)) {
        return FALSE;
    }
    FX_SAFE_DWORD src_pitch =
        CalculatePitch8(m_bpc, m_nComponents, m_Width, m_Height);
    if (!src_pitch.IsValid()) {
        return FALSE;
    }
    m_pStreamAcc = new CPDF_StreamAcc;
    m_pStreamAcc->LoadAllData(pStream, FALSE, src_pitch.ValueOrDie(), TRUE);
    if (m_pStreamAcc->GetSize() == 0 || m_pStreamAcc->GetData() == NULL) {
        return FALSE;
    }
    if (!CreateDecoder()) {
        return FALSE;
    }
    if (m_bImageMask) {
        m_bpp = 1;
        m_bpc = 1;
        m_nComponents = 1;
        m_AlphaFlag = 1;
    } else if (m_bpc * m_nComponents == 1) {
        m_bpp = 1;
    } else if (m_bpc * m_nComponents <= 8) {
        m_bpp = 8;
    } else {
        m_bpp = 24;
    }
    FX_SAFE_DWORD pitch = CalculatePitch32(m_bpp, m_Width);
    if (!pitch.IsValid()) {
        return FALSE;
    }
    m_pLineBuf = FX_Alloc(FX_BYTE, pitch.ValueOrDie());
    if (m_pColorSpace && bStdCS) {
        m_pColorSpace->EnableStdConversion(TRUE);
    }
    LoadPalette();
    if (m_bColorKey) {
        m_bpp = 32;
        m_AlphaFlag = 2;
        pitch = CalculatePitch32(m_bpp, m_Width);
        if (!pitch.IsValid()) {
            return FALSE;
        }
        m_pMaskedLine = FX_Alloc(FX_BYTE, pitch.ValueOrDie());
    }
    m_Pitch = pitch.ValueOrDie();
    if (ppMask) {
        *ppMask = LoadMask(*pMatteColor);
    }
    if (m_pColorSpace && bStdCS) {
        m_pColorSpace->EnableStdConversion(FALSE);
    }
    return TRUE;
}
int	CPDF_DIBSource::ContinueToLoadMask()
{
    if (m_bImageMask) {
        m_bpp = 1;
        m_bpc = 1;
        m_nComponents = 1;
        m_AlphaFlag = 1;
    } else if (m_bpc * m_nComponents == 1) {
        m_bpp = 1;
    } else if (m_bpc * m_nComponents <= 8) {
        m_bpp = 8;
    } else {
        m_bpp = 24;
    }
    if (!m_bpc || !m_nComponents) {
        return 0;
    }
    FX_SAFE_DWORD pitch = CalculatePitch32(m_bpp, m_Width);
    if (!pitch.IsValid()) {
        return 0;
    }
    m_pLineBuf = FX_Alloc(FX_BYTE, pitch.ValueOrDie());
    if (m_pColorSpace && m_bStdCS) {
        m_pColorSpace->EnableStdConversion(TRUE);
    }
    LoadPalette();
    if (m_bColorKey) {
        m_bpp = 32;
        m_AlphaFlag = 2;
        pitch = CalculatePitch32(m_bpp, m_Width);
        if (!pitch.IsValid()) {
            return 0;
        }
        m_pMaskedLine = FX_Alloc(FX_BYTE, pitch.ValueOrDie());
    }
    m_Pitch = pitch.ValueOrDie();
    return 1;
}
int	CPDF_DIBSource::StartLoadDIBSource(CPDF_Document* pDoc, const CPDF_Stream* pStream, FX_BOOL bHasMask,
                                       CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources,
                                       FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask)
{
    if (pStream == NULL) {
        return 0;
    }
    m_pDocument = pDoc;
    m_pDict = pStream->GetDict();
    m_pStream = pStream;
    m_bStdCS = bStdCS;
    m_bHasMask = bHasMask;
    m_Width = m_pDict->GetInteger(FX_BSTRC("Width"));
    m_Height = m_pDict->GetInteger(FX_BSTRC("Height"));
    if (m_Width <= 0 || m_Height <= 0 || m_Width > 0x01ffff || m_Height > 0x01ffff) {
        return 0;
    }
    m_GroupFamily = GroupFamily;
    m_bLoadMask = bLoadMask;
    if (!LoadColorInfo(m_pStream->GetObjNum() != 0 ? NULL : pFormResources, pPageResources)) {
        return 0;
    }
    if (m_bDoBpcCheck && (m_bpc == 0 || m_nComponents == 0)) {
        return 0;
    }
    FX_SAFE_DWORD src_pitch =
        CalculatePitch8(m_bpc, m_nComponents, m_Width, m_Height);
    if (!src_pitch.IsValid()) {
        return 0;
    }
    m_pStreamAcc = new CPDF_StreamAcc;
    m_pStreamAcc->LoadAllData(pStream, FALSE, src_pitch.ValueOrDie(), TRUE);
    if (m_pStreamAcc->GetSize() == 0 || m_pStreamAcc->GetData() == NULL) {
        return 0;
    }
    int ret = CreateDecoder();
    if (ret != 1) {
        if (!ret) {
            return ret;
        }
        if (!ContinueToLoadMask()) {
            return 0;
        }
        if (m_bHasMask) {
            StratLoadMask();
        }
        return ret;
    }
    if (!ContinueToLoadMask()) {
        return 0;
    }
    if (m_bHasMask) {
        ret = StratLoadMask();
    }
    if (ret == 2) {
        return ret;
    }
    if (m_pColorSpace && m_bStdCS) {
        m_pColorSpace->EnableStdConversion(FALSE);
    }
    return ret;
}
int	CPDF_DIBSource::ContinueLoadDIBSource(IFX_Pause* pPause)
{
    FXCODEC_STATUS ret;
    if (m_Status == 1) {
        const CFX_ByteString& decoder = m_pStreamAcc->GetImageDecoder();
        if (decoder == FX_BSTRC("JPXDecode")) {
            return 0;
        }
        ICodec_Jbig2Module* pJbig2Module = CPDF_ModuleMgr::Get()->GetJbig2Module();
        if (m_pJbig2Context == NULL) {
            m_pJbig2Context = pJbig2Module->CreateJbig2Context();
            if (m_pStreamAcc->GetImageParam()) {
                CPDF_Stream* pGlobals = m_pStreamAcc->GetImageParam()->GetStream(FX_BSTRC("JBIG2Globals"));
                if (pGlobals) {
                    m_pGlobalStream = new CPDF_StreamAcc;
                    m_pGlobalStream->LoadAllData(pGlobals, FALSE);
                }
            }
            ret = pJbig2Module->StartDecode(m_pJbig2Context, m_Width, m_Height, m_pStreamAcc->GetData(), m_pStreamAcc->GetSize(),
                                            m_pGlobalStream ? m_pGlobalStream->GetData() : NULL, m_pGlobalStream ? m_pGlobalStream->GetSize() : 0, m_pCachedBitmap->GetBuffer(),
                                            m_pCachedBitmap->GetPitch(), pPause);
            if (ret < 0) {
                delete m_pCachedBitmap;
                m_pCachedBitmap = NULL;
                delete m_pGlobalStream;
                m_pGlobalStream = NULL;
                pJbig2Module->DestroyJbig2Context(m_pJbig2Context);
                m_pJbig2Context = NULL;
                return 0;
            }
            if (ret == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
                return 2;
            }
            int ret1 = 1;
            if (m_bHasMask) {
                ret1 = ContinueLoadMaskDIB(pPause);
                m_Status = 2;
            }
            if (ret1 == 2) {
                return ret1;
            }
            if (m_pColorSpace && m_bStdCS) {
                m_pColorSpace->EnableStdConversion(FALSE);
            }
            return ret1;
        }
        FXCODEC_STATUS ret = pJbig2Module->ContinueDecode(m_pJbig2Context, pPause);
        if (ret < 0) {
            delete m_pCachedBitmap;
            m_pCachedBitmap = NULL;
            delete m_pGlobalStream;
            m_pGlobalStream = NULL;
            pJbig2Module->DestroyJbig2Context(m_pJbig2Context);
            m_pJbig2Context = NULL;
            return 0;
        }
        if (ret == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
            return 2;
        }
        int ret1 = 1;
        if (m_bHasMask) {
            ret1 = ContinueLoadMaskDIB(pPause);
            m_Status = 2;
        }
        if (ret1 == 2) {
            return ret1;
        }
        if (m_pColorSpace && m_bStdCS) {
            m_pColorSpace->EnableStdConversion(FALSE);
        }
        return ret1;
    } else if (m_Status == 2) {
        return ContinueLoadMaskDIB(pPause);
    }
    return 0;
}
FX_BOOL CPDF_DIBSource::LoadColorInfo(CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources)
{
    m_bpc_orig = m_pDict->GetInteger(FX_BSTRC("BitsPerComponent"));
    if (m_pDict->GetInteger("ImageMask")) {
        m_bImageMask = TRUE;
    }
    if (m_bImageMask || !m_pDict->KeyExist(FX_BSTRC("ColorSpace"))) {
        if (!m_bImageMask) {
            CPDF_Object* pFilter = m_pDict->GetElementValue(FX_BSTRC("Filter"));
            if (pFilter) {
                CFX_ByteString filter;
                if (pFilter->GetType() == PDFOBJ_NAME) {
                    filter = pFilter->GetString();
                    if (filter == FX_BSTRC("JPXDecode")) {
                        m_bDoBpcCheck = FALSE;
                        return TRUE;
                    }
                } else if (pFilter->GetType() == PDFOBJ_ARRAY) {
                    CPDF_Array* pArray = (CPDF_Array*)pFilter;
                    if (pArray->GetString(pArray->GetCount() - 1) == FX_BSTRC("JPXDecode")) {
                        m_bDoBpcCheck = FALSE;
                        return TRUE;
                    }
                }
            }
        }
        m_bImageMask = TRUE;
        m_bpc = m_nComponents = 1;
        CPDF_Array* pDecode = m_pDict->GetArray(FX_BSTRC("Decode"));
        m_bDefaultDecode = pDecode == NULL || pDecode->GetInteger(0) == 0;
        return TRUE;
    }
    CPDF_Object* pCSObj = m_pDict->GetElementValue(FX_BSTRC("ColorSpace"));
    if (pCSObj == NULL) {
        return FALSE;
    }
    CPDF_DocPageData* pDocPageData = m_pDocument->GetPageData();
    if (pFormResources) {
        m_pColorSpace = pDocPageData->GetColorSpace(pCSObj, pFormResources);
    }
    if (m_pColorSpace == NULL) {
        m_pColorSpace = pDocPageData->GetColorSpace(pCSObj, pPageResources);
    }
    if (m_pColorSpace == NULL) {
        return FALSE;
    }
    m_Family = m_pColorSpace->GetFamily();
    m_nComponents = m_pColorSpace->CountComponents();
    if (m_Family == PDFCS_ICCBASED && pCSObj->GetType() == PDFOBJ_NAME) {
        CFX_ByteString cs = pCSObj->GetString();
        if (cs == FX_BSTRC("DeviceGray")) {
            m_nComponents = 1;
        } else if (cs == FX_BSTRC("DeviceRGB")) {
            m_nComponents = 3;
        } else if (cs == FX_BSTRC("DeviceCMYK")) {
            m_nComponents = 4;
        }
    }
    ValidateDictParam();
    m_pCompData = GetDecodeAndMaskArray(m_bDefaultDecode, m_bColorKey);
    if (m_pCompData == NULL) {
        return FALSE;
    }
    return TRUE;
}
DIB_COMP_DATA* CPDF_DIBSource::GetDecodeAndMaskArray(FX_BOOL& bDefaultDecode, FX_BOOL& bColorKey)
{
    if (m_pColorSpace == NULL) {
        return NULL;
    }
    DIB_COMP_DATA* pCompData = FX_Alloc(DIB_COMP_DATA, m_nComponents);
    int max_data = (1 << m_bpc) - 1;
    CPDF_Array* pDecode = m_pDict->GetArray(FX_BSTRC("Decode"));
    if (pDecode) {
        for (FX_DWORD i = 0; i < m_nComponents; i ++) {
            pCompData[i].m_DecodeMin = pDecode->GetNumber(i * 2);
            FX_FLOAT max = pDecode->GetNumber(i * 2 + 1);
            pCompData[i].m_DecodeStep = (max - pCompData[i].m_DecodeMin) / max_data;
            FX_FLOAT def_value, def_min, def_max;
            m_pColorSpace->GetDefaultValue(i, def_value, def_min, def_max);
            if (m_Family == PDFCS_INDEXED) {
                def_max = (FX_FLOAT)max_data;
            }
            if (def_min != pCompData[i].m_DecodeMin || def_max != max) {
                bDefaultDecode = FALSE;
            }
        }
    } else {
        for (FX_DWORD i = 0; i < m_nComponents; i ++) {
            FX_FLOAT def_value;
            m_pColorSpace->GetDefaultValue(i, def_value, pCompData[i].m_DecodeMin, pCompData[i].m_DecodeStep);
            if (m_Family == PDFCS_INDEXED) {
                pCompData[i].m_DecodeStep = (FX_FLOAT)max_data;
            }
            pCompData[i].m_DecodeStep = (pCompData[i].m_DecodeStep - pCompData[i].m_DecodeMin) / max_data;
        }
    }
    if (!m_pDict->KeyExist(FX_BSTRC("SMask"))) {
        CPDF_Object* pMask = m_pDict->GetElementValue(FX_BSTRC("Mask"));
        if (pMask == NULL) {
            return pCompData;
        }
        if (pMask->GetType() == PDFOBJ_ARRAY) {
            CPDF_Array* pArray = (CPDF_Array*)pMask;
            if (pArray->GetCount() >= m_nComponents * 2) {
                for (FX_DWORD i = 0; i < m_nComponents; i++) {
                    int min_num = pArray->GetInteger(i * 2);
                    int max_num = pArray->GetInteger(i * 2 + 1);
                    pCompData[i].m_ColorKeyMin = FX_MAX(min_num, 0);
                    pCompData[i].m_ColorKeyMax = FX_MIN(max_num, max_data);
                }
            }
            bColorKey = TRUE;
        }
    }
    return pCompData;
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFaxDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        const CPDF_Dictionary* pParams);
ICodec_ScanlineDecoder* FPDFAPI_CreateFlateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        int nComps, int bpc, const CPDF_Dictionary* pParams);
int CPDF_DIBSource::CreateDecoder()
{
    const CFX_ByteString& decoder = m_pStreamAcc->GetImageDecoder();
    if (decoder.IsEmpty()) {
        return 1;
    }
    if (m_bDoBpcCheck && m_bpc == 0) {
        return 0;
    }
    FX_LPCBYTE src_data = m_pStreamAcc->GetData();
    FX_DWORD src_size = m_pStreamAcc->GetSize();
    const CPDF_Dictionary* pParams = m_pStreamAcc->GetImageParam();
    if (decoder == FX_BSTRC("CCITTFaxDecode")) {
        m_pDecoder = FPDFAPI_CreateFaxDecoder(src_data, src_size, m_Width, m_Height, pParams);
    } else if (decoder == FX_BSTRC("DCTDecode")) {
        m_pDecoder = CPDF_ModuleMgr::Get()->GetJpegModule()->CreateDecoder(src_data, src_size, m_Width, m_Height,
                     m_nComponents, pParams ? pParams->GetInteger(FX_BSTR("ColorTransform"), 1) : 1);
        if (!m_pDecoder) {
            FX_BOOL bTransform = FALSE;
            int comps, bpc;
            ICodec_JpegModule* pJpegModule = CPDF_ModuleMgr::Get()->GetJpegModule();
            if (pJpegModule->LoadInfo(src_data, src_size, m_Width, m_Height, comps, bpc, bTransform)) {
                if (m_nComponents != comps) {
                    FX_Free(m_pCompData);
                    m_nComponents = comps;
                    if (m_Family == PDFCS_LAB && m_nComponents != 3) {
                        m_pCompData = NULL;
                        return 0;
                    }
                    m_pCompData = GetDecodeAndMaskArray(m_bDefaultDecode, m_bColorKey);
                    if (m_pCompData == NULL) {
                        return 0;
                    }
                }
                m_bpc = bpc;
                m_pDecoder = CPDF_ModuleMgr::Get()->GetJpegModule()->CreateDecoder(src_data, src_size, m_Width, m_Height,
                             m_nComponents, bTransform);
            }
        }
    } else if (decoder == FX_BSTRC("FlateDecode")) {
        m_pDecoder = FPDFAPI_CreateFlateDecoder(src_data, src_size, m_Width, m_Height, m_nComponents, m_bpc, pParams);
    } else if (decoder == FX_BSTRC("JPXDecode")) {
        LoadJpxBitmap();
        return m_pCachedBitmap != NULL ? 1 : 0;
    } else if (decoder == FX_BSTRC("JBIG2Decode")) {
        m_pCachedBitmap = new CFX_DIBitmap;
        if (!m_pCachedBitmap->Create(m_Width, m_Height, m_bImageMask ? FXDIB_1bppMask : FXDIB_1bppRgb)) {
            delete m_pCachedBitmap;
            m_pCachedBitmap = NULL;
            return 0;
        }
        m_Status = 1;
        return 2;
    } else if (decoder == FX_BSTRC("RunLengthDecode")) {
        m_pDecoder = CPDF_ModuleMgr::Get()->GetCodecModule()->GetBasicModule()->CreateRunLengthDecoder(src_data, src_size, m_Width, m_Height, m_nComponents, m_bpc);
    }
    if (!m_pDecoder)
        return 0;

    FX_SAFE_DWORD requested_pitch =
        CalculatePitch8(m_bpc, m_nComponents, m_Width, 1);
    if (!requested_pitch.IsValid()) {
        return 0;
    }
    FX_SAFE_DWORD provided_pitch = CalculatePitch8(m_pDecoder->GetBPC(),
                                                   m_pDecoder->CountComps(),
                                                   m_pDecoder->GetWidth(),
                                                   1);
    if (!provided_pitch.IsValid()) {
        return 0;
    }
    if (provided_pitch.ValueOrDie() < requested_pitch.ValueOrDie()) {
        return 0;
    }
    return 1;
}
void CPDF_DIBSource::LoadJpxBitmap()
{
    ICodec_JpxModule* pJpxModule = CPDF_ModuleMgr::Get()->GetJpxModule();
    if (pJpxModule == NULL) {
        return;
    }
    FX_LPVOID ctx = pJpxModule->CreateDecoder(m_pStreamAcc->GetData(), m_pStreamAcc->GetSize(), m_pColorSpace != NULL);
    if (ctx == NULL) {
        return;
    }
    FX_DWORD width = 0, height = 0, codestream_nComps = 0, image_nComps = 0;
    pJpxModule->GetImageInfo(ctx, width, height, codestream_nComps, image_nComps);
    if ((int)width < m_Width || (int)height < m_Height) {
        pJpxModule->DestroyDecoder(ctx);
        return;
    }
    int output_nComps;
    FX_BOOL bTranslateColor, bSwapRGB = FALSE;
    if (m_pColorSpace) {
        if (codestream_nComps != (FX_DWORD)m_pColorSpace->CountComponents()) {
            return;
        }
        output_nComps = codestream_nComps;
        bTranslateColor = FALSE;
        if (m_pColorSpace == CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB)) {
            bSwapRGB = TRUE;
            m_pColorSpace = NULL;
        }
    } else {
        bTranslateColor = TRUE;
        if (image_nComps) {
            output_nComps = image_nComps;
        } else {
            output_nComps = codestream_nComps;
        }
        if (output_nComps == 3) {
            bSwapRGB = TRUE;
        } else if (output_nComps == 4) {
            m_pColorSpace = CPDF_ColorSpace::GetStockCS(PDFCS_DEVICECMYK);
            bTranslateColor = FALSE;
        }
        m_nComponents = output_nComps;
    }
    FXDIB_Format format;
    if (output_nComps == 1) {
        format = FXDIB_8bppRgb;
    } else if (output_nComps <= 3) {
        format = FXDIB_Rgb;
    } else if (output_nComps == 4) {
        format = FXDIB_Rgb32;
    } else {
        width = (width * output_nComps + 2) / 3;
        format = FXDIB_Rgb;
    }
    m_pCachedBitmap = new CFX_DIBitmap;
    if (!m_pCachedBitmap->Create(width, height, format)) {
        delete m_pCachedBitmap;
        m_pCachedBitmap = NULL;
        return;
    }
    m_pCachedBitmap->Clear(0xFFFFFFFF);
    FX_LPBYTE output_offsets = FX_Alloc(FX_BYTE, output_nComps);
    for (int i = 0; i < output_nComps; i ++) {
        output_offsets[i] = i;
    }
    if (bSwapRGB) {
        output_offsets[0] = 2;
        output_offsets[2] = 0;
    }
    if (!pJpxModule->Decode(ctx, m_pCachedBitmap->GetBuffer(), m_pCachedBitmap->GetPitch(), bTranslateColor, output_offsets)) {
        delete m_pCachedBitmap;
        m_pCachedBitmap = NULL;
        return;
    }
    FX_Free(output_offsets);
    pJpxModule->DestroyDecoder(ctx);
    if (m_pColorSpace && m_pColorSpace->GetFamily() == PDFCS_INDEXED && m_bpc < 8) {
        int scale = 8 - m_bpc;
        for (FX_DWORD row = 0; row < height; row ++) {
            FX_LPBYTE scanline = (FX_LPBYTE)m_pCachedBitmap->GetScanline(row);
            for (FX_DWORD col = 0; col < width; col ++) {
                *scanline = (*scanline) >> scale;
                scanline++;
            }
        }
    }
    m_bpc = 8;
}
CPDF_DIBSource* CPDF_DIBSource::LoadMask(FX_DWORD& MatteColor)
{
    MatteColor = 0xffffffff;
    CPDF_Stream* pSoftMask = m_pDict->GetStream(FX_BSTRC("SMask"));
    if (pSoftMask) {
        CPDF_Array* pMatte = pSoftMask->GetDict()->GetArray(FX_BSTRC("Matte"));
        if (pMatte != NULL && m_pColorSpace && (FX_DWORD)m_pColorSpace->CountComponents() <= m_nComponents) {
            FX_FLOAT* pColor = FX_Alloc(FX_FLOAT, m_nComponents);
            for (FX_DWORD i = 0; i < m_nComponents; i ++) {
                pColor[i] = pMatte->GetFloat(i);
            }
            FX_FLOAT R, G, B;
            m_pColorSpace->GetRGB(pColor, R, G, B);
            FX_Free(pColor);
            MatteColor = FXARGB_MAKE(0, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255));
        }
        return LoadMaskDIB(pSoftMask);
    }
    CPDF_Object* pMask = m_pDict->GetElementValue(FX_BSTRC("Mask"));
    if (pMask == NULL) {
        return NULL;
    }
    if (pMask->GetType() == PDFOBJ_STREAM) {
        return LoadMaskDIB((CPDF_Stream*)pMask);
    }
    return NULL;
}
int	CPDF_DIBSource::StratLoadMask()
{
    m_MatteColor = 0xffffffff;
    m_pMaskStream = m_pDict->GetStream(FX_BSTRC("SMask"));
    if (m_pMaskStream) {
        CPDF_Array* pMatte = m_pMaskStream->GetDict()->GetArray(FX_BSTRC("Matte"));
        if (pMatte != NULL && m_pColorSpace && (FX_DWORD)m_pColorSpace->CountComponents() <= m_nComponents) {
            FX_FLOAT R, G, B;
            FX_FLOAT* pColor = FX_Alloc(FX_FLOAT, m_nComponents);
            for (FX_DWORD i = 0; i < m_nComponents; i ++) {
                pColor[i] = pMatte->GetFloat(i);
            }
            m_pColorSpace->GetRGB(pColor, R, G, B);
            FX_Free(pColor);
            m_MatteColor = FXARGB_MAKE(0, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255));
        }
        return StartLoadMaskDIB();
    }
    m_pMaskStream = m_pDict->GetElementValue(FX_BSTRC("Mask"));
    if (m_pMaskStream == NULL) {
        return 1;
    }
    if (m_pMaskStream->GetType() == PDFOBJ_STREAM) {
        return StartLoadMaskDIB();
    }
    return 1;
}
int	CPDF_DIBSource::ContinueLoadMaskDIB(IFX_Pause* pPause)
{
    if (m_pMask == NULL) {
        return 1;
    }
    int ret = m_pMask->ContinueLoadDIBSource(pPause);
    if (ret == 2) {
        return ret;
    }
    if (m_pColorSpace && m_bStdCS) {
        m_pColorSpace->EnableStdConversion(FALSE);
    }
    if (!ret) {
        delete m_pMask;
        m_pMask = NULL;
        return ret;
    }
    return 1;
}
CPDF_DIBSource*	CPDF_DIBSource::DetachMask()
{
    CPDF_DIBSource* pDIBSource = m_pMask;
    m_pMask = NULL;
    return pDIBSource;
}
CPDF_DIBSource* CPDF_DIBSource::LoadMaskDIB(CPDF_Stream* pMask)
{
    CPDF_DIBSource* pMaskSource = new CPDF_DIBSource;
    if (!pMaskSource->Load(m_pDocument, pMask, NULL, NULL, NULL, NULL, TRUE)) {
        delete pMaskSource;
        return NULL;
    }
    return pMaskSource;
}
int CPDF_DIBSource::StartLoadMaskDIB()
{
    m_pMask = new CPDF_DIBSource;
    int ret = m_pMask->StartLoadDIBSource(m_pDocument, (CPDF_Stream*)m_pMaskStream, FALSE, NULL, NULL, TRUE);
    if (ret == 2) {
        if (m_Status == 0) {
            m_Status = 2;
        }
        return 2;
    }
    if (!ret) {
        delete m_pMask;
        m_pMask = NULL;
        return 1;
    }
    return 1;
}
void CPDF_DIBSource::LoadPalette()
{
    if (m_bpc == 0) {
        return;
    }
    if (m_bpc * m_nComponents > 8) {
        return;
    }
    if (m_pColorSpace == NULL) {
        return;
    }
    if (m_bpc * m_nComponents == 1) {
        if (m_bDefaultDecode && (m_Family == PDFCS_DEVICEGRAY || m_Family == PDFCS_DEVICERGB)) {
            return;
        }
        if (m_pColorSpace->CountComponents() > 3) {
            return;
        }
        FX_FLOAT color_values[3];
        color_values[0] = m_pCompData[0].m_DecodeMin;
        color_values[1] = color_values[2] = color_values[0];
        FX_FLOAT R=0.0f, G=0.0f, B=0.0f;
        m_pColorSpace->GetRGB(color_values, R, G, B);
        FX_ARGB argb0 = ArgbEncode(255, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255));
        color_values[0] += m_pCompData[0].m_DecodeStep;
        color_values[1] += m_pCompData[0].m_DecodeStep;
        color_values[2] += m_pCompData[0].m_DecodeStep;
        m_pColorSpace->GetRGB(color_values, R, G, B);
        FX_ARGB argb1 = ArgbEncode(255, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255));
        if (argb0 != 0xFF000000 || argb1 != 0xFFFFFFFF) {
            SetPaletteArgb(0, argb0);
            SetPaletteArgb(1, argb1);
        }
        return;
    }
    if (m_pColorSpace == CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY) && m_bpc == 8 && m_bDefaultDecode) {
    } else {
        int palette_count = 1 << (m_bpc * m_nComponents);
        CFX_FixedBufGrow<FX_FLOAT, 16> color_values(m_nComponents);
        FX_FLOAT* color_value = color_values;
        for (int i = 0; i < palette_count; i ++) {
            int color_data = i;
            for (FX_DWORD j = 0; j < m_nComponents; j ++) {
                int encoded_component = color_data % (1 << m_bpc);
                color_data /= 1 << m_bpc;
                color_value[j] = m_pCompData[j].m_DecodeMin + m_pCompData[j].m_DecodeStep * encoded_component;
            }
            FX_FLOAT R = 0, G = 0, B = 0;
            if (m_nComponents == 1 && m_Family == PDFCS_ICCBASED && m_pColorSpace->CountComponents() > 1) {
                int nComponents = m_pColorSpace->CountComponents();
                FX_FLOAT* temp_buf = FX_Alloc(FX_FLOAT, nComponents);
                for (int i = 0; i < nComponents; i++) {
                    temp_buf[i] = *color_value;
                }
                m_pColorSpace->GetRGB(temp_buf, R, G, B);
                FX_Free(temp_buf);
            } else {
                m_pColorSpace->GetRGB(color_value, R, G, B);
            }
            SetPaletteArgb(i, ArgbEncode(255, FXSYS_round(R * 255), FXSYS_round(G * 255), FXSYS_round(B * 255)));
        }
    }
}
void CPDF_DIBSource::ValidateDictParam()
{
    m_bpc = m_bpc_orig;
    CPDF_Object * pFilter = m_pDict->GetElementValue(FX_BSTRC("Filter"));
    if (pFilter) {
        if (pFilter->GetType() == PDFOBJ_NAME) {
            CFX_ByteString filter = pFilter->GetString();
            if (filter == FX_BSTRC("CCITTFaxDecode") || filter == FX_BSTRC("JBIG2Decode")) {
                m_bpc = 1;
                m_nComponents = 1;
            }
            if (filter == FX_BSTRC("RunLengthDecode") || filter == FX_BSTRC("DCTDecode")) {
                m_bpc = 8;
            }
        } else if (pFilter->GetType() == PDFOBJ_ARRAY) {
            CPDF_Array *pArray = (CPDF_Array *)pFilter;
            if (pArray->GetString(pArray->GetCount() - 1) == FX_BSTRC("CCITTFaxDecode") ||
                    pArray->GetString(pArray->GetCount() - 1) == FX_BSTRC("JBIG2Decode")) {
                m_bpc = 1;
                m_nComponents = 1;
            }
            if (pArray->GetString(pArray->GetCount() - 1) == FX_BSTRC("DCTDecode")) {
                // Previously, pArray->GetString(pArray->GetCount() - 1) == FX_BSTRC("RunLengthDecode") was checked in the "if" statement as well,
                // but too many documents don't conform to it.
                m_bpc = 8;
            }
        }
    }
    if (m_bpc != 1 && m_bpc != 2 && m_bpc != 4 && m_bpc != 8 && m_bpc != 16) {
        m_bpc = 0;
    }
}
#define NORMALCOLOR_MAX(color, max) (color) > (max) ? (max) : (color) < 0 ? 0 : (color);
void CPDF_DIBSource::TranslateScanline24bpp(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan) const
{
    if (m_bpc == 0) {
        return;
    }
    int max_data = (1 << m_bpc) - 1;
    if (m_bDefaultDecode) {
        if (m_Family == PDFCS_DEVICERGB || m_Family == PDFCS_CALRGB) {
            FX_LPCBYTE src_pos = src_scan;
            switch (m_bpc) {
                case 16:
                    for (int col = 0; col < m_Width; col ++) {
                        *dest_scan++ = src_pos[4];
                        *dest_scan++ = src_pos[2];
                        *dest_scan++ = *src_pos;
                        src_pos += 6;
                    }
                    break;
                case 8:
                    for (int column = 0; column < m_Width; column ++) {
                        *dest_scan++ = src_pos[2];
                        *dest_scan++ = src_pos[1];
                        *dest_scan++ = *src_pos;
                        src_pos += 3;
                    }
                    break;
                default:
                    int src_bit_pos = 0;
                    int dest_byte_pos = 0;
                    for (int column = 0; column < m_Width; column ++) {
                        int R = _GetBits8(src_scan, src_bit_pos, m_bpc);
                        src_bit_pos += m_bpc;
                        int G = _GetBits8(src_scan, src_bit_pos, m_bpc);
                        src_bit_pos += m_bpc;
                        int B = _GetBits8(src_scan, src_bit_pos, m_bpc);
                        src_bit_pos += m_bpc;
                        R = NORMALCOLOR_MAX(R, max_data);
                        G = NORMALCOLOR_MAX(G, max_data);
                        B = NORMALCOLOR_MAX(B, max_data);
                        dest_scan[dest_byte_pos] = B * 255 / max_data;
                        dest_scan[dest_byte_pos + 1] = G * 255 / max_data;
                        dest_scan[dest_byte_pos + 2] = R * 255 / max_data;
                        dest_byte_pos += 3;
                    }
                    break;
            }
            return;
        } else if (m_bpc == 8) {
            if (m_nComponents == m_pColorSpace->CountComponents())
                m_pColorSpace->TranslateImageLine(dest_scan, src_scan, m_Width, m_Width, m_Height,
                                                  m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK);
            return;
        }
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> color_values1(m_nComponents);
    FX_FLOAT* color_values = color_values1;
    FX_FLOAT R = 0.0f, G = 0.0f, B = 0.0f;
    if (m_bpc == 8) {
        int src_byte_pos = 0;
        int dest_byte_pos = 0;
        for (int column = 0; column < m_Width; column ++) {
            for (FX_DWORD color = 0; color < m_nComponents; color ++) {
                int data = src_scan[src_byte_pos ++];
                color_values[color] = m_pCompData[color].m_DecodeMin +
                                      m_pCompData[color].m_DecodeStep * data;
            }
            if (m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK) {
                FX_FLOAT k = 1.0f - color_values[3];
                R = (1.0f - color_values[0]) * k;
                G = (1.0f - color_values[1]) * k;
                B = (1.0f - color_values[2]) * k;
            } else {
                m_pColorSpace->GetRGB(color_values, R, G, B);
            }
            R = NORMALCOLOR_MAX(R, 1);
            G = NORMALCOLOR_MAX(G, 1);
            B = NORMALCOLOR_MAX(B, 1);
            dest_scan[dest_byte_pos] = (FX_INT32)(B * 255);
            dest_scan[dest_byte_pos + 1] = (FX_INT32)(G * 255);
            dest_scan[dest_byte_pos + 2] = (FX_INT32)(R * 255);
            dest_byte_pos += 3;
        }
    } else {
        int src_bit_pos = 0;
        int dest_byte_pos = 0;
        for (int column = 0; column < m_Width; column ++) {
            for (FX_DWORD color = 0; color < m_nComponents; color ++) {
                int data = _GetBits8(src_scan, src_bit_pos, m_bpc);
                color_values[color] = m_pCompData[color].m_DecodeMin +
                                      m_pCompData[color].m_DecodeStep * data;
                src_bit_pos += m_bpc;
            }
            if (m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK) {
                FX_FLOAT k = 1.0f - color_values[3];
                R = (1.0f - color_values[0]) * k;
                G = (1.0f - color_values[1]) * k;
                B = (1.0f - color_values[2]) * k;
            } else {
                m_pColorSpace->GetRGB(color_values, R, G, B);
            }
            R = NORMALCOLOR_MAX(R, 1);
            G = NORMALCOLOR_MAX(G, 1);
            B = NORMALCOLOR_MAX(B, 1);
            dest_scan[dest_byte_pos] = (FX_INT32)(B * 255);
            dest_scan[dest_byte_pos + 1] = (FX_INT32)(G * 255);
            dest_scan[dest_byte_pos + 2] = (FX_INT32)(R * 255);
            dest_byte_pos += 3;
        }
    }
}
FX_LPBYTE CPDF_DIBSource::GetBuffer() const
{
    if (m_pCachedBitmap) {
        return m_pCachedBitmap->GetBuffer();
    }
    return NULL;
}
FX_LPCBYTE CPDF_DIBSource::GetScanline(int line) const
{
    if (m_bpc == 0) {
        return NULL;
    }
    FX_SAFE_DWORD src_pitch = CalculatePitch8(m_bpc, m_nComponents, m_Width, 1);
    if (!src_pitch.IsValid())
        return NULL;
    FX_DWORD src_pitch_value = src_pitch.ValueOrDie();
    FX_LPCBYTE pSrcLine = NULL;
    if (m_pCachedBitmap) {
        if (line >= m_pCachedBitmap->GetHeight()) {
            line = m_pCachedBitmap->GetHeight() - 1;
        }
        pSrcLine = m_pCachedBitmap->GetScanline(line);
    } else if (m_pDecoder) {
        pSrcLine = m_pDecoder->GetScanline(line);
    } else {
        if (m_pStreamAcc->GetSize() >= (line + 1) * src_pitch_value) {
            pSrcLine = m_pStreamAcc->GetData() + line * src_pitch_value;
        }
    }
    if (pSrcLine == NULL) {
        FX_LPBYTE pLineBuf = m_pMaskedLine ? m_pMaskedLine : m_pLineBuf;
        FXSYS_memset8(pLineBuf, 0xff, m_Pitch);
        return pLineBuf;
    }
    if (m_bpc * m_nComponents == 1) {
        if (m_bImageMask && m_bDefaultDecode) {
            for (FX_DWORD i = 0; i < src_pitch_value; i++) {
                m_pLineBuf[i] = ~pSrcLine[i];
            }
        } else if (m_bColorKey) {
            FX_DWORD reset_argb, set_argb;
            reset_argb = m_pPalette ? m_pPalette[0] : 0xff000000;
            set_argb = m_pPalette ? m_pPalette[1] : 0xffffffff;
            if (m_pCompData[0].m_ColorKeyMin == 0) {
                reset_argb = 0;
            }
            if (m_pCompData[0].m_ColorKeyMax == 1) {
                set_argb = 0;
            }
            set_argb = FXARGB_TODIB(set_argb);
            reset_argb = FXARGB_TODIB(reset_argb);
            FX_DWORD* dest_scan = (FX_DWORD*)m_pMaskedLine;
            for (int col = 0; col < m_Width; col ++) {
                if (pSrcLine[col / 8] & (1 << (7 - col % 8))) {
                    *dest_scan = set_argb;
                } else {
                    *dest_scan = reset_argb;
                }
                dest_scan ++;
            }
            return m_pMaskedLine;
        } else {
            FXSYS_memcpy32(m_pLineBuf, pSrcLine, src_pitch_value);
        }
        return m_pLineBuf;
    }
    if (m_bpc * m_nComponents <= 8) {
        if (m_bpc == 8) {
            FXSYS_memcpy32(m_pLineBuf, pSrcLine, src_pitch_value);
        } else {
            int src_bit_pos = 0;
            for (int col = 0; col < m_Width; col ++) {
                int color_index = 0;
                for (FX_DWORD color = 0; color < m_nComponents; color ++) {
                    int data = _GetBits8(pSrcLine, src_bit_pos, m_bpc);
                    color_index |= data << (color * m_bpc);
                    src_bit_pos += m_bpc;
                }
                m_pLineBuf[col] = color_index;
            }
        }
        if (m_bColorKey) {
            FX_LPBYTE pDestPixel = m_pMaskedLine;
            FX_LPCBYTE pSrcPixel = m_pLineBuf;
            for (int col = 0; col < m_Width; col ++) {
                FX_BYTE index = *pSrcPixel++;
                if (m_pPalette) {
                    *pDestPixel++ = FXARGB_B(m_pPalette[index]);
                    *pDestPixel++ = FXARGB_G(m_pPalette[index]);
                    *pDestPixel++ = FXARGB_R(m_pPalette[index]);
                } else {
                    *pDestPixel++ = index;
                    *pDestPixel++ = index;
                    *pDestPixel++ = index;
                }
                *pDestPixel = (index < m_pCompData[0].m_ColorKeyMin || index > m_pCompData[0].m_ColorKeyMax) ? 0xff : 0;
                pDestPixel ++ ;
            }
            return m_pMaskedLine;
        }
        return m_pLineBuf;
    }
    if (m_bColorKey) {
        if (m_nComponents == 3 && m_bpc == 8) {
            FX_LPBYTE alpha_channel = m_pMaskedLine + 3;
            for (int col = 0; col < m_Width; col ++) {
                FX_LPCBYTE pPixel = pSrcLine + col * 3;
                alpha_channel[col * 4] = (pPixel[0] < m_pCompData[0].m_ColorKeyMin || pPixel[0] > m_pCompData[0].m_ColorKeyMax ||
                                          pPixel[1] < m_pCompData[1].m_ColorKeyMin || pPixel[1] > m_pCompData[1].m_ColorKeyMax ||
                                          pPixel[2] < m_pCompData[2].m_ColorKeyMin || pPixel[2] > m_pCompData[2].m_ColorKeyMax) ? 0xff : 0;
            }
        } else {
            FXSYS_memset8(m_pMaskedLine, 0xff, m_Pitch);
        }
    }
    if (m_pColorSpace) {
        TranslateScanline24bpp(m_pLineBuf, pSrcLine);
        pSrcLine = m_pLineBuf;
    }
    if (m_bColorKey) {
        FX_LPCBYTE pSrcPixel = pSrcLine;
        FX_LPBYTE pDestPixel = m_pMaskedLine;
        for (int col = 0; col < m_Width; col ++) {
            *pDestPixel++ = *pSrcPixel++;
            *pDestPixel++ = *pSrcPixel++;
            *pDestPixel++ = *pSrcPixel++;
            pDestPixel ++;
        }
        return m_pMaskedLine;
    }
    return pSrcLine;
}
FX_BOOL CPDF_DIBSource::SkipToScanline(int line, IFX_Pause* pPause) const
{
    if (m_pDecoder) {
        return m_pDecoder->SkipToScanline(line, pPause);
    }
    return FALSE;
}
void CPDF_DIBSource::DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
                                        int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const
{
    if (line < 0 || dest_scan == NULL || dest_bpp <= 0 ||
        dest_width <= 0 || clip_left < 0 || clip_width <= 0) {
        return;
    }

    FX_DWORD src_width = m_Width;
    FX_SAFE_DWORD pitch = CalculatePitch8(m_bpc, m_nComponents, m_Width, 1);
    if (!pitch.IsValid()) {
        return;
    }

    FX_LPCBYTE pSrcLine = NULL;
    if (m_pCachedBitmap) {
        pSrcLine = m_pCachedBitmap->GetScanline(line);
    } else if (m_pDecoder) {
        pSrcLine = m_pDecoder->GetScanline(line);
    } else {
        FX_DWORD src_pitch = pitch.ValueOrDie();
        pitch *= (line+1);
        if (!pitch.IsValid()) {
            return;
        }

        if (m_pStreamAcc->GetSize() >= pitch.ValueOrDie()) {
            pSrcLine = m_pStreamAcc->GetData() + line * src_pitch;
        }
    }
    int orig_Bpp = m_bpc * m_nComponents / 8;
    int dest_Bpp = dest_bpp / 8;
    if (pSrcLine == NULL) {
        FXSYS_memset32(dest_scan, 0xff, dest_Bpp * clip_width);
        return;
    }

    FX_SAFE_INT32 max_src_x = clip_left;
    max_src_x += clip_width - 1;
    max_src_x *= src_width;
    max_src_x /= dest_width;
    if (!max_src_x.IsValid()) {
        return;
    }

    CFX_FixedBufGrow<FX_BYTE, 128> temp(orig_Bpp);
    if (m_bpc * m_nComponents == 1) {
        FX_DWORD set_argb = (FX_DWORD) - 1, reset_argb = 0;
        if (m_bImageMask) {
            if (m_bDefaultDecode) {
                set_argb = 0;
                reset_argb = (FX_DWORD) - 1;
            }
        } else if (m_bColorKey) {
            reset_argb = m_pPalette ? m_pPalette[0] : 0xff000000;
            set_argb = m_pPalette ? m_pPalette[1] : 0xffffffff;
            if (m_pCompData[0].m_ColorKeyMin == 0) {
                reset_argb = 0;
            }
            if (m_pCompData[0].m_ColorKeyMax == 1) {
                set_argb = 0;
            }
            set_argb = FXARGB_TODIB(set_argb);
            reset_argb = FXARGB_TODIB(reset_argb);
            for (int i = 0; i < clip_width; i ++) {
                FX_DWORD src_x = (clip_left + i) * src_width / dest_width;
                if (bFlipX) {
                    src_x = src_width - src_x - 1;
                }
                src_x %= src_width;
                if (pSrcLine[src_x / 8] & (1 << (7 - src_x % 8))) {
                    ((FX_DWORD*)dest_scan)[i] = set_argb;
                } else {
                    ((FX_DWORD*)dest_scan)[i] = reset_argb;
                }
            }
            return;
        } else {
            if (dest_Bpp == 1) {
            } else if (m_pPalette) {
                reset_argb = m_pPalette[0];
                set_argb = m_pPalette[1];
            }
        }
        for (int i = 0; i < clip_width; i ++) {
            FX_DWORD src_x = (clip_left + i) * src_width / dest_width;
            if (bFlipX) {
                src_x = src_width - src_x - 1;
            }
            src_x %= src_width;
            int dest_pos = i * dest_Bpp;
            if (pSrcLine[src_x / 8] & (1 << (7 - src_x % 8))) {
                if (dest_Bpp == 1) {
                    dest_scan[dest_pos] = (FX_BYTE)set_argb;
                } else if (dest_Bpp == 3) {
                    dest_scan[dest_pos] = FXARGB_B(set_argb);
                    dest_scan[dest_pos + 1] = FXARGB_G(set_argb);
                    dest_scan[dest_pos + 2] = FXARGB_R(set_argb);
                } else {
                    *(FX_DWORD*)(dest_scan + dest_pos) = set_argb;
                }
            } else {
                if (dest_Bpp == 1) {
                    dest_scan[dest_pos] = (FX_BYTE)reset_argb;
                } else if (dest_Bpp == 3) {
                    dest_scan[dest_pos] = FXARGB_B(reset_argb);
                    dest_scan[dest_pos + 1] = FXARGB_G(reset_argb);
                    dest_scan[dest_pos + 2] = FXARGB_R(reset_argb);
                } else {
                    *(FX_DWORD*)(dest_scan + dest_pos) = reset_argb;
                }
            }
        }
        return;
    } else if (m_bpc * m_nComponents <= 8) {
        if (m_bpc < 8) {
            int src_bit_pos = 0;
            for (FX_DWORD col = 0; col < src_width; col ++) {
                int color_index = 0;
                for (FX_DWORD color = 0; color < m_nComponents; color ++) {
                    int data = _GetBits8(pSrcLine, src_bit_pos, m_bpc);
                    color_index |= data << (color * m_bpc);
                    src_bit_pos += m_bpc;
                }
                m_pLineBuf[col] = color_index;
            }
            pSrcLine = m_pLineBuf;
        }
        if (m_bColorKey) {
            for (int i = 0; i < clip_width; i ++) {
                FX_DWORD src_x = (clip_left + i) * src_width / dest_width;
                if (bFlipX) {
                    src_x = src_width - src_x - 1;
                }
                src_x %= src_width;
                FX_LPBYTE pDestPixel = dest_scan + i * 4;
                FX_BYTE index = pSrcLine[src_x];
                if (m_pPalette) {
                    *pDestPixel++ = FXARGB_B(m_pPalette[index]);
                    *pDestPixel++ = FXARGB_G(m_pPalette[index]);
                    *pDestPixel++ = FXARGB_R(m_pPalette[index]);
                } else {
                    *pDestPixel++ = index;
                    *pDestPixel++ = index;
                    *pDestPixel++ = index;
                }
                *pDestPixel = (index < m_pCompData[0].m_ColorKeyMin || index > m_pCompData[0].m_ColorKeyMax) ? 0xff : 0;
            }
            return;
        }
        for (int i = 0; i < clip_width; i ++) {
            FX_DWORD src_x = (clip_left + i) * src_width / dest_width;
            if (bFlipX) {
                src_x = src_width - src_x - 1;
            }
            src_x %= src_width;
            FX_BYTE index = pSrcLine[src_x];
            if (dest_Bpp == 1) {
                dest_scan[i] = index;
            } else {
                int dest_pos = i * dest_Bpp;
                FX_ARGB argb = m_pPalette[index];
                dest_scan[dest_pos] = FXARGB_B(argb);
                dest_scan[dest_pos + 1] = FXARGB_G(argb);
                dest_scan[dest_pos + 2] = FXARGB_R(argb);
            }
        }
        return;
    } else {
        int last_src_x = -1;
        FX_ARGB last_argb;
        FX_FLOAT orig_Not8Bpp = (FX_FLOAT)m_bpc * (FX_FLOAT)m_nComponents / 8.0f;
        FX_FLOAT unit_To8Bpc = 255.0f / ((1 << m_bpc) - 1);
        for (int i = 0; i < clip_width; i ++) {
            int dest_x = clip_left + i;
            FX_DWORD src_x = (bFlipX ? (dest_width - dest_x - 1) : dest_x) * (FX_INT64)src_width / dest_width;
            src_x %= src_width;
            FX_LPCBYTE pSrcPixel = NULL;
            if (m_bpc % 8 == 0) {
                pSrcPixel = pSrcLine + src_x * orig_Bpp;
            } else {
                pSrcPixel = pSrcLine + (int)(src_x * orig_Not8Bpp);
            }
            FX_LPBYTE pDestPixel = dest_scan + i * dest_Bpp;
            FX_ARGB argb;
            if (src_x == last_src_x) {
                argb = last_argb;
            } else {
                if (m_pColorSpace) {
                    FX_BYTE color[4];
                    if (!m_bDefaultDecode) {
                        for (int i = 0; i < m_nComponents; i ++) {
                            int color_value = (int)((m_pCompData[i].m_DecodeMin + m_pCompData[i].m_DecodeStep * (FX_FLOAT)pSrcPixel[i]) * 255.0f + 0.5f);
                            temp[i] = color_value > 255 ? 255 : (color_value < 0 ? 0 : color_value);
                        }
                        m_pColorSpace->TranslateImageLine(color, temp, 1, 0, 0, m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK);
                    } else {
                        if (m_bpc < 8) {
                            int src_bit_pos = 0;
                            if (src_x % 2) {
                                src_bit_pos = 4;
                            }
                            for (FX_DWORD i = 0; i < m_nComponents; i ++) {
                                temp[i] = (FX_BYTE)(_GetBits8(pSrcPixel, src_bit_pos, m_bpc) * unit_To8Bpc);
                                src_bit_pos += m_bpc;
                            }
                            m_pColorSpace->TranslateImageLine(color, temp, 1, 0, 0, m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK);
                        } else {
                            m_pColorSpace->TranslateImageLine(color, pSrcPixel, 1, 0, 0, m_bLoadMask && m_GroupFamily == PDFCS_DEVICECMYK && m_Family == PDFCS_DEVICECMYK);
                        }
                    }
                    argb = FXARGB_MAKE(0xff, color[2], color[1], color[0]);
                } else {
                    argb = FXARGB_MAKE(0xff, pSrcPixel[2], pSrcPixel[1], pSrcPixel[0]);
                }
                if (m_bColorKey) {
                    int alpha = 0xff;
                    if (m_nComponents == 3 && m_bpc == 8) {
                        alpha = (pSrcPixel[0] < m_pCompData[0].m_ColorKeyMin ||
                                 pSrcPixel[0] > m_pCompData[0].m_ColorKeyMax ||
                                 pSrcPixel[1] < m_pCompData[1].m_ColorKeyMin ||
                                 pSrcPixel[1] > m_pCompData[1].m_ColorKeyMax ||
                                 pSrcPixel[2] < m_pCompData[2].m_ColorKeyMin ||
                                 pSrcPixel[2] > m_pCompData[2].m_ColorKeyMax) ? 0xff : 0;
                    }
                    argb &= 0xffffff;
                    argb |= alpha << 24;
                }
                last_src_x = src_x;
                last_argb = argb;
            }
            if (dest_Bpp == 4) {
                *(FX_DWORD*)pDestPixel = FXARGB_TODIB(argb);
            } else {
                *pDestPixel++ = FXARGB_B(argb);
                *pDestPixel++ = FXARGB_G(argb);
                *pDestPixel = FXARGB_R(argb);
            }
        }
    }
}
void CPDF_DIBSource::SetDownSampleSize(int dest_width, int dest_height) const
{
    if (m_pDecoder) {
        m_pDecoder->DownScale(dest_width, dest_height);
        ((CPDF_DIBSource*)this)->m_Width = m_pDecoder->GetWidth();
        ((CPDF_DIBSource*)this)->m_Height = m_pDecoder->GetHeight();
    }
}
void CPDF_DIBSource::ClearImageData()
{
    if (m_pDecoder) {
        m_pDecoder->ClearImageData();
    }
}
CPDF_ProgressiveImageLoaderHandle::CPDF_ProgressiveImageLoaderHandle()
{
    m_pImageLoader = NULL;
    m_pCache = NULL;
    m_pImage = NULL;
}
CPDF_ProgressiveImageLoaderHandle::~CPDF_ProgressiveImageLoaderHandle()
{
}
FX_BOOL CPDF_ProgressiveImageLoaderHandle::Start(CPDF_ImageLoader* pImageLoader, const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus, FX_INT32 nDownsampleWidth, FX_INT32 nDownsampleHeight)
{
    m_pImageLoader = pImageLoader;
    m_pCache = pCache;
    m_pImage = (CPDF_ImageObject*)pImage;
    m_nDownsampleWidth = nDownsampleWidth;
    m_nDownsampleHeight = nDownsampleHeight;
    FX_BOOL ret;
    if (pCache) {
        ret = pCache->StartGetCachedBitmap(pImage->m_pImage->GetStream(), bStdCS, GroupFamily, bLoadMask, pRenderStatus, m_nDownsampleWidth, m_nDownsampleHeight);
        if (ret == FALSE) {
            m_pImageLoader->m_bCached = TRUE;
            m_pImageLoader->m_pBitmap = pCache->m_pCurImageCache->DetachBitmap();
            m_pImageLoader->m_pMask = pCache->m_pCurImageCache->DetachMask();
            m_pImageLoader->m_MatteColor = pCache->m_pCurImageCache->m_MatteColor;
        }
    } else {
        ret = pImage->m_pImage->StartLoadDIBSource(pRenderStatus->m_pFormResource, pRenderStatus->m_pPageResource, bStdCS, GroupFamily, bLoadMask);
        if (ret == FALSE) {
            m_pImageLoader->m_bCached = FALSE;
            m_pImageLoader->m_pBitmap = m_pImage->m_pImage->DetachBitmap();
            m_pImageLoader->m_pMask = m_pImage->m_pImage->DetachMask();
            m_pImageLoader->m_MatteColor = m_pImage->m_pImage->m_MatteColor;
        }
    }
    return ret;
}
FX_BOOL CPDF_ProgressiveImageLoaderHandle::Continue(IFX_Pause* pPause)
{
    FX_BOOL ret;
    if (m_pCache) {
        ret = m_pCache->Continue(pPause);
        if (ret == FALSE) {
            m_pImageLoader->m_bCached = TRUE;
            m_pImageLoader->m_pBitmap = m_pCache->m_pCurImageCache->DetachBitmap();
            m_pImageLoader->m_pMask = m_pCache->m_pCurImageCache->DetachMask();
            m_pImageLoader->m_MatteColor = m_pCache->m_pCurImageCache->m_MatteColor;
        }
    } else {
        ret = m_pImage->m_pImage->Continue(pPause);
        if (ret == FALSE) {
            m_pImageLoader->m_bCached = FALSE;
            m_pImageLoader->m_pBitmap = m_pImage->m_pImage->DetachBitmap();
            m_pImageLoader->m_pMask = m_pImage->m_pImage->DetachMask();
            m_pImageLoader->m_MatteColor = m_pImage->m_pImage->m_MatteColor;
        }
    }
    return ret;
}
FX_BOOL CPDF_ImageLoader::Load(const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus)
{
    if (pImage == NULL) {
        return FALSE;
    }
    if (pCache) {
        pCache->GetCachedBitmap(pImage->m_pImage->GetStream(), m_pBitmap, m_pMask, m_MatteColor, bStdCS, GroupFamily, bLoadMask, pRenderStatus, m_nDownsampleWidth, m_nDownsampleHeight);
        m_bCached = TRUE;
    } else {
        m_pBitmap = pImage->m_pImage->LoadDIBSource(&m_pMask, &m_MatteColor, bStdCS, GroupFamily, bLoadMask);
        m_bCached = FALSE;
    }
    return FALSE;
}
FX_BOOL CPDF_ImageLoader::StartLoadImage(const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_LPVOID& LoadHandle, FX_BOOL bStdCS, FX_DWORD GroupFamily, FX_BOOL bLoadMask, CPDF_RenderStatus* pRenderStatus, FX_INT32 nDownsampleWidth, FX_INT32 nDownsampleHeight)
{
    m_nDownsampleWidth = nDownsampleWidth;
    m_nDownsampleHeight = nDownsampleHeight;
    CPDF_ProgressiveImageLoaderHandle* pLoaderHandle = new CPDF_ProgressiveImageLoaderHandle;
    FX_BOOL ret = pLoaderHandle->Start(this, pImage, pCache, bStdCS, GroupFamily, bLoadMask, pRenderStatus, m_nDownsampleWidth, m_nDownsampleHeight);
    LoadHandle = pLoaderHandle;
    return ret;
}
FX_BOOL	CPDF_ImageLoader::Continue(FX_LPVOID LoadHandle, IFX_Pause* pPause)
{
    return ((CPDF_ProgressiveImageLoaderHandle*)LoadHandle)->Continue(pPause);
}
CPDF_ImageLoader::~CPDF_ImageLoader()
{
    if (!m_bCached) {
        delete m_pBitmap;
        delete m_pMask;
    }
}
