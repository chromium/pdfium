// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_page.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "pageint.h"
#include <limits.h>
void sRGB_to_AdobeCMYK(FX_FLOAT R, FX_FLOAT G, FX_FLOAT B, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k)
{
    c = 1.0f - R;
    m = 1.0f - G;
    y = 1.0f - B;
    k = c;
    if (m < k) {
        k = m;
    }
    if (y < k) {
        k = y;
    }
}
CPDF_DeviceCS::CPDF_DeviceCS(int family)
{
    m_Family = family;
    if (m_Family == PDFCS_DEVICERGB) {
        m_nComponents = 3;
    } else if (m_Family == PDFCS_DEVICEGRAY) {
        m_nComponents = 1;
    } else {
        m_nComponents = 4;
    }
}
FX_BOOL CPDF_DeviceCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    if (m_Family == PDFCS_DEVICERGB) {
        R = pBuf[0];
        if (R < 0) {
            R = 0;
        } else if (R > 1) {
            R = 1;
        }
        G = pBuf[1];
        if (G < 0) {
            G = 0;
        } else if (G > 1) {
            G = 1;
        }
        B = pBuf[2];
        if (B < 0) {
            B = 0;
        } else if (B > 1) {
            B = 1;
        }
    } else if (m_Family == PDFCS_DEVICEGRAY) {
        R = *pBuf;
        if (R < 0) {
            R = 0;
        } else if (R > 1) {
            R = 1;
        }
        G = B = R;
    } else if (m_Family == PDFCS_DEVICECMYK) {
        if (!m_dwStdConversion) {
            AdobeCMYK_to_sRGB(pBuf[0], pBuf[1], pBuf[2], pBuf[3], R, G, B);
        } else {
            FX_FLOAT k = pBuf[3];
            R = 1.0f - FX_MIN(1.0f, pBuf[0] + k);
            G = 1.0f - FX_MIN(1.0f, pBuf[1] + k);
            B = 1.0f - FX_MIN(1.0f, pBuf[2] + k);
        }
    } else {
        ASSERT(m_Family == PDFCS_PATTERN);
        R = G = B = 0;
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_DeviceCS::v_GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const
{
    if (m_Family != PDFCS_DEVICECMYK) {
        return FALSE;
    }
    c = pBuf[0];
    m = pBuf[1];
    y = pBuf[2];
    k = pBuf[3];
    return TRUE;
}
FX_BOOL CPDF_DeviceCS::SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
{
    if (m_Family == PDFCS_DEVICERGB) {
        pBuf[0] = R;
        pBuf[1] = G;
        pBuf[2] = B;
        return TRUE;
    } else if (m_Family == PDFCS_DEVICEGRAY) {
        if (R == G && R == B) {
            *pBuf = R;
            return TRUE;
        } else {
            return FALSE;
        }
    } else if (m_Family == PDFCS_DEVICECMYK) {
        sRGB_to_AdobeCMYK(R, G, B, pBuf[0], pBuf[1], pBuf[2], pBuf[3]);
        return TRUE;
    }
    return FALSE;
}
FX_BOOL CPDF_DeviceCS::v_SetCMYK(FX_FLOAT* pBuf, FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k) const
{
    if (m_Family == PDFCS_DEVICERGB) {
        AdobeCMYK_to_sRGB(c, m, y, k, pBuf[0], pBuf[1], pBuf[2]);
        return TRUE;
    } else if (m_Family == PDFCS_DEVICEGRAY) {
        return FALSE;
    } else if (m_Family == PDFCS_DEVICECMYK) {
        pBuf[0] = c;
        pBuf[1] = m;
        pBuf[2] = y;
        pBuf[3] = k;
        return TRUE;
    }
    return FALSE;
}
static void ReverseRGB(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels)
{
    if (pDestBuf == pSrcBuf)
        for (int i = 0; i < pixels; i ++) {
            FX_BYTE temp = pDestBuf[2];
            pDestBuf[2] = pDestBuf[0];
            pDestBuf[0] = temp;
            pDestBuf += 3;
        }
    else
        for (int i = 0; i < pixels; i ++) {
            *pDestBuf ++ = pSrcBuf[2];
            *pDestBuf ++ = pSrcBuf[1];
            *pDestBuf ++ = pSrcBuf[0];
            pSrcBuf += 3;
        }
}
void CPDF_DeviceCS::TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    if (bTransMask && m_Family == PDFCS_DEVICECMYK) {
        for (int i = 0; i < pixels; i ++) {
            int k = 255 - pSrcBuf[3];
            pDestBuf[0] = ((255 - pSrcBuf[0]) * k) / 255;
            pDestBuf[1] = ((255 - pSrcBuf[1]) * k) / 255;
            pDestBuf[2] = ((255 - pSrcBuf[2]) * k) / 255;
            pDestBuf += 3;
            pSrcBuf += 4;
        }
        return;
    }
    if (m_Family == PDFCS_DEVICERGB) {
        ReverseRGB(pDestBuf, pSrcBuf, pixels);
    } else if (m_Family == PDFCS_DEVICEGRAY) {
        for (int i = 0; i < pixels; i ++) {
            *pDestBuf ++ = pSrcBuf[i];
            *pDestBuf ++ = pSrcBuf[i];
            *pDestBuf ++ = pSrcBuf[i];
        }
    } else {
        for (int i = 0; i < pixels; i ++) {
            if (!m_dwStdConversion) {
                AdobeCMYK_to_sRGB1(pSrcBuf[0], pSrcBuf[1], pSrcBuf[2], pSrcBuf[3], pDestBuf[2], pDestBuf[1], pDestBuf[0]);
            } else {
                FX_BYTE k = pSrcBuf[3];
                pDestBuf[2] = 255 - FX_MIN(255, pSrcBuf[0] + k);
                pDestBuf[1] = 255 - FX_MIN(255, pSrcBuf[1] + k);
                pDestBuf[0] = 255 - FX_MIN(255, pSrcBuf[2] + k);
            }
            pSrcBuf += 4;
            pDestBuf += 3;
        }
    }
}
const FX_BYTE g_sRGBSamples1[] = {
    0,   3,   6,  10,  13,  15,  18,  20,  22,  23,  25,  27,  28,  30,  31,  32,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
    49,  50,  51,  52,  53,  53,  54,  55,  56,  56,  57,  58,  58,  59,  60,  61,
    61,  62,  62,  63,  64,  64,  65,  66,  66,  67,  67,  68,  68,  69,  70,  70,
    71,  71,  72,  72,  73,  73,  74,  74,  75,  76,  76,  77,  77,  78,  78,  79,
    79,  79,  80,  80,  81,  81,  82,  82,  83,  83,  84,  84,  85,  85,  85,  86,
    86,  87,  87,  88,  88,  88,  89,  89,  90,  90,  91,  91,  91,  92,  92,  93,
    93,  93,  94,  94,  95,  95,  95,  96,  96,  97,  97,  97,  98,  98,  98,  99,
    99,  99, 100, 100, 101, 101, 101, 102, 102, 102, 103, 103, 103, 104, 104, 104,
    105, 105, 106, 106, 106, 107, 107, 107, 108, 108, 108, 109, 109, 109, 110, 110,
    110, 110, 111, 111, 111, 112, 112, 112, 113, 113, 113, 114, 114, 114, 115, 115,
    115, 115, 116, 116, 116, 117, 117, 117, 118, 118, 118, 118, 119, 119, 119, 120,
};
const FX_BYTE g_sRGBSamples2[] = {
    120, 121, 122, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136,
    137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 148, 149, 150, 151,
    152, 153, 154, 155, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 163, 164,
    165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 175, 176,
    177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184, 185, 185, 186, 187, 187,
    188, 189, 189, 190, 190, 191, 192, 192, 193, 194, 194, 195, 196, 196, 197, 197,
    198, 199, 199, 200, 200, 201, 202, 202, 203, 203, 204, 205, 205, 206, 206, 207,
    208, 208, 209, 209, 210, 210, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216,
    216, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224, 224,
    225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232, 233,
    233, 234, 234, 235, 235, 236, 236, 237, 237, 238, 238, 238, 239, 239, 240, 240,
    241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247, 248,
    248, 249, 249, 250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255,
};

static FX_FLOAT RGB_Conversion(FX_FLOAT colorComponent)
{
    if (colorComponent > 1) {
        colorComponent = 1;
    }
    if (colorComponent < 0) {
        colorComponent = 0;
    }
    int scale = (int)(colorComponent * 1023);
    if (scale < 0) {
        scale = 0;
    }
    if (scale < 192) {
        colorComponent = (g_sRGBSamples1[scale] / 255.0f);
    }
    else {
        colorComponent = (g_sRGBSamples2[scale / 4 - 48] / 255.0f);
    }
    return colorComponent;
}

static void XYZ_to_sRGB(FX_FLOAT X, FX_FLOAT Y, FX_FLOAT Z, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B)
{
    FX_FLOAT R1 = 3.2410f * X - 1.5374f * Y - 0.4986f * Z;
    FX_FLOAT G1 = -0.9692f * X + 1.8760f * Y + 0.0416f * Z;
    FX_FLOAT B1 =  0.0556f * X - 0.2040f * Y + 1.0570f * Z;

    R = RGB_Conversion(R1);
    G = RGB_Conversion(G1);
    B = RGB_Conversion(B1);
}

static void XYZ_to_sRGB_WhitePoint(FX_FLOAT X, FX_FLOAT Y, FX_FLOAT Z, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B, FX_FLOAT Xw, FX_FLOAT Yw, FX_FLOAT Zw)
{
    // The following RGB_xyz is based on
    // sRGB value {Rx,Ry}={0.64, 0.33}, {Gx,Gy}={0.30, 0.60}, {Bx,By}={0.15, 0.06}

    FX_FLOAT Rx = 0.64f, Ry = 0.33f;
    FX_FLOAT Gx = 0.30f, Gy = 0.60f;
    FX_FLOAT Bx = 0.15f, By = 0.06f;
    CFX_Matrix_3by3 RGB_xyz(Rx, Gx, Bx, Ry, Gy, By, 1 - Rx - Ry, 1 - Gx - Gy, 1 - Bx - By);
    CFX_Vector_3by1 whitePoint(Xw, Yw, Zw);
    CFX_Vector_3by1 XYZ(X, Y, Z);

    CFX_Vector_3by1 RGB_Sum_XYZ = RGB_xyz.Inverse().TransformVector(whitePoint);
    CFX_Matrix_3by3 RGB_SUM_XYZ_DIAG(RGB_Sum_XYZ.a, 0, 0, 0, RGB_Sum_XYZ.b, 0, 0, 0, RGB_Sum_XYZ.c);
    CFX_Matrix_3by3 M = RGB_xyz.Multiply(RGB_SUM_XYZ_DIAG);
    CFX_Vector_3by1 RGB = M.Inverse().TransformVector(XYZ);

    R = RGB_Conversion(RGB.a);
    G = RGB_Conversion(RGB.b);
    B = RGB_Conversion(RGB.c);
}
class CPDF_CalGray : public CPDF_ColorSpace
{
public:
    CPDF_CalGray();
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual void		TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;
    FX_BOOL				SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const;
    FX_FLOAT			m_WhitePoint[3];
    FX_FLOAT			m_BlackPoint[3];
    FX_FLOAT			m_Gamma;
};
CPDF_CalGray::CPDF_CalGray()
{
    m_Family = PDFCS_CALGRAY;
    m_nComponents = 1;
}
FX_BOOL CPDF_CalGray::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Dictionary* pDict = pArray->GetDict(1);
    CPDF_Array* pParam = pDict->GetArray(FX_BSTRC("WhitePoint"));
    int i;
    for (i = 0; i < 3; i ++) {
        m_WhitePoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    pParam = pDict->GetArray(FX_BSTRC("BlackPoint"));
    for (i = 0; i < 3; i ++) {
        m_BlackPoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    m_Gamma = pDict->GetNumber(FX_BSTRC("Gamma"));
    if (m_Gamma == 0) {
        m_Gamma = 1.0f;
    }
    return TRUE;
}
FX_BOOL CPDF_CalGray::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    R = G = B = *pBuf;
    return TRUE;
}
FX_BOOL CPDF_CalGray::SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
{
    if (R == G && R == B) {
        *pBuf = R;
        return TRUE;
    } else {
        return FALSE;
    }
}
void CPDF_CalGray::TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    for (int i = 0; i < pixels; i ++) {
        *pDestBuf ++ = pSrcBuf[i];
        *pDestBuf ++ = pSrcBuf[i];
        *pDestBuf ++ = pSrcBuf[i];
    }
}
class CPDF_CalRGB : public CPDF_ColorSpace
{
public:
    CPDF_CalRGB();
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual void		TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;
    FX_BOOL				SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const;
    FX_FLOAT			m_WhitePoint[3];
    FX_FLOAT			m_BlackPoint[3];
    FX_FLOAT			m_Gamma[3];
    FX_FLOAT			m_Matrix[9];
    FX_BOOL				m_bGamma, m_bMatrix;
};
CPDF_CalRGB::CPDF_CalRGB()
{
    m_Family = PDFCS_CALRGB;
    m_nComponents = 3;
}
FX_BOOL CPDF_CalRGB::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Dictionary* pDict = pArray->GetDict(1);
    CPDF_Array* pParam = pDict->GetArray(FX_BSTRC("WhitePoint"));
    int i;
    for (i = 0; i < 3; i ++) {
        m_WhitePoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    pParam = pDict->GetArray(FX_BSTRC("BlackPoint"));
    for (i = 0; i < 3; i ++) {
        m_BlackPoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    pParam = pDict->GetArray(FX_BSTRC("Gamma"));
    if (pParam) {
        m_bGamma = TRUE;
        for (i = 0; i < 3; i ++) {
            m_Gamma[i] = pParam->GetNumber(i);
        }
    } else {
        m_bGamma = FALSE;
    }
    pParam = pDict->GetArray(FX_BSTRC("Matrix"));
    if (pParam) {
        m_bMatrix = TRUE;
        for (i = 0; i < 9; i ++) {
            m_Matrix[i] = pParam->GetNumber(i);
        }
    } else {
        m_bMatrix = FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_CalRGB::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    FX_FLOAT A_ = pBuf[0];
    FX_FLOAT B_ = pBuf[1];
    FX_FLOAT C_ = pBuf[2];
    if (m_bGamma) {
        A_ = (FX_FLOAT)FXSYS_pow(A_, m_Gamma[0]);
        B_ = (FX_FLOAT)FXSYS_pow(B_, m_Gamma[1]);
        C_ = (FX_FLOAT)FXSYS_pow(C_, m_Gamma[2]);
    }
    FX_FLOAT X, Y, Z;
    if (m_bMatrix) {
        X = m_Matrix[0] * A_ + m_Matrix[3] * B_ + m_Matrix[6] * C_;
        Y = m_Matrix[1] * A_ + m_Matrix[4] * B_ + m_Matrix[7] * C_;
        Z = m_Matrix[2] * A_ + m_Matrix[5] * B_ + m_Matrix[8] * C_;
    } else {
        X = A_;
        Y = B_;
        Z = C_;
    }
    XYZ_to_sRGB_WhitePoint(X, Y, Z, R, G, B, m_WhitePoint[0], m_WhitePoint[1], m_WhitePoint[2]);
    return TRUE;
}
FX_BOOL CPDF_CalRGB::SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
{
    pBuf[0] = R;
    pBuf[1] = G;
    pBuf[2] = B;
    return TRUE;
}
void CPDF_CalRGB::TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    if (bTransMask) {
        FX_FLOAT Cal[3];
        FX_FLOAT R, G, B;
        for(int i = 0; i < pixels; i ++) {
            Cal[0] = ((FX_FLOAT)pSrcBuf[2]) / 255;
            Cal[1] = ((FX_FLOAT)pSrcBuf[1]) / 255;
            Cal[2] = ((FX_FLOAT)pSrcBuf[0]) / 255;
            GetRGB(Cal, R, G, B);
            pDestBuf[0] = FXSYS_round(B * 255);
            pDestBuf[1] = FXSYS_round(G * 255);
            pDestBuf[2] = FXSYS_round(R * 255);
            pSrcBuf += 3;
            pDestBuf += 3;
        }
    }
    ReverseRGB(pDestBuf, pSrcBuf, pixels);
}
class CPDF_LabCS : public CPDF_ColorSpace
{
public:
    CPDF_LabCS()
    {
        m_Family = PDFCS_LAB;
        m_nComponents = 3;
    }
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual void		GetDefaultValue(int iComponent, FX_FLOAT& value, FX_FLOAT& min, FX_FLOAT& max) const;
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    FX_BOOL		SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const;
    virtual void		TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;
    FX_FLOAT	m_WhitePoint[3];
    FX_FLOAT	m_BlackPoint[3];
    FX_FLOAT	m_Ranges[4];
};
FX_BOOL CPDF_LabCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Dictionary* pDict = pArray->GetDict(1);
    if (!pDict) {
        return FALSE;
    }
    CPDF_Array* pParam = pDict->GetArray(FX_BSTRC("WhitePoint"));
    int i;
    for (i = 0; i < 3; i ++) {
        m_WhitePoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    pParam = pDict->GetArray(FX_BSTRC("BlackPoint"));
    for (i = 0; i < 3; i ++) {
        m_BlackPoint[i] = pParam ? pParam->GetNumber(i) : 0;
    }
    pParam = pDict->GetArray(FX_BSTRC("Range"));
    const FX_FLOAT def_ranges[4] = { -100 * 1.0f, 100 * 1.0f, -100 * 1.0f, 100 * 1.0f};
    for (i = 0; i < 4; i ++) {
        m_Ranges[i] = pParam ? pParam->GetNumber(i) : def_ranges[i];
    }
    return TRUE;
}
void CPDF_LabCS::GetDefaultValue(int iComponent, FX_FLOAT& value, FX_FLOAT& min, FX_FLOAT& max) const
{
    assert(iComponent < 3);
    value = 0;
    if (iComponent == 0) {
        min = 0;
        max = 100 * 1.0f;
    } else {
        min = m_Ranges[iComponent * 2 - 2];
        max = m_Ranges[iComponent * 2 - 1];
        if (value < min) {
            value = min;
        } else if (value > max) {
            value = max;
        }
    }
}
FX_BOOL CPDF_LabCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    FX_FLOAT Lstar = pBuf[0];
    FX_FLOAT astar = pBuf[1];
    FX_FLOAT bstar = pBuf[2];
    FX_FLOAT M = (Lstar + 16.0f) / 116.0f;
    FX_FLOAT L = M + astar / 500.0f;
    FX_FLOAT N = M - bstar / 200.0f;
    FX_FLOAT X, Y, Z;
    if (L < 0.2069f) {
        X = 0.957f * 0.12842f * (L - 0.1379f);
    } else {
        X = 0.957f * L * L * L;
    }
    if (M < 0.2069f) {
        Y = 0.12842f * (M - 0.1379f);
    } else {
        Y = M * M * M;
    }
    if (N < 0.2069f) {
        Z = 1.0889f * 0.12842f * (N - 0.1379f);
    } else {
        Z = 1.0889f * N * N * N;
    }
    XYZ_to_sRGB(X, Y, Z, R, G, B);
    return TRUE;
}
FX_BOOL CPDF_LabCS::SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
{
    return FALSE;
}
void CPDF_LabCS::TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    for (int i = 0; i < pixels; i ++) {
        FX_FLOAT lab[3];
        FX_FLOAT R, G, B;
        lab[0] = (pSrcBuf[0] * 100 / 255.0f);
        lab[1] = (FX_FLOAT)(pSrcBuf[1] - 128);
        lab[2] = (FX_FLOAT)(pSrcBuf[2] - 128);
        GetRGB(lab, R, G, B);
        pDestBuf[0] = (FX_INT32)(B * 255);
        pDestBuf[1] = (FX_INT32)(G * 255);
        pDestBuf[2] = (FX_INT32)(R * 255);
        pDestBuf += 3;
        pSrcBuf += 3;
    }
}
CPDF_IccProfile::CPDF_IccProfile(FX_LPCBYTE pData, FX_DWORD dwSize):
    m_bsRGB(FALSE),
    m_pTransform(NULL),
    m_nSrcComponents(0)
{
    if (dwSize == 3144 && FXSYS_memcmp32(pData + 0x190, "sRGB IEC61966-2.1", 17) == 0) {
        m_bsRGB = TRUE;
        m_nSrcComponents = 3;
    }
    else if (CPDF_ModuleMgr::Get()->GetIccModule()) {
        m_pTransform = CPDF_ModuleMgr::Get()->GetIccModule()->CreateTransform_sRGB(pData, dwSize, m_nSrcComponents);
    }
}
CPDF_IccProfile::~CPDF_IccProfile()
{
    if (m_pTransform) {
        CPDF_ModuleMgr::Get()->GetIccModule()->DestroyTransform(m_pTransform);
    }
}
class CPDF_ICCBasedCS : public CPDF_ColorSpace
{
public:
    CPDF_ICCBasedCS();
    virtual ~CPDF_ICCBasedCS();
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    void				GetDefaultValue(int i, FX_FLOAT& min, FX_FLOAT& max) const
    {
        min = m_pRanges[i * 2];
        max = m_pRanges[i * 2 + 1];
    }
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    FX_BOOL				v_GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const;
    FX_BOOL				SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const;
    virtual void		EnableStdConversion(FX_BOOL bEnabled);
    virtual void		TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;
    FX_FLOAT*		m_pRanges;
    CPDF_IccProfile*	m_pProfile;
    CPDF_ColorSpace*	m_pAlterCS;
    FX_LPBYTE			m_pCache;
    FX_BOOL				m_bOwn;
};
CPDF_ICCBasedCS::CPDF_ICCBasedCS()
{
    m_pAlterCS = NULL;
    m_pProfile = NULL;
    m_Family = PDFCS_ICCBASED;
    m_pCache = NULL;
    m_pRanges = NULL;
    m_bOwn = FALSE;
}
CPDF_ICCBasedCS::~CPDF_ICCBasedCS()
{
    if (m_pCache) {
        FX_Free(m_pCache);
    }
    if (m_pRanges) {
        FX_Free(m_pRanges);
    }
    if (m_pAlterCS && m_bOwn) {
        m_pAlterCS->ReleaseCS();
    }
    if (m_pProfile && m_pDocument) {
        m_pDocument->GetPageData()->ReleaseIccProfile(NULL, m_pProfile);
    }
}
FX_BOOL CPDF_ICCBasedCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Stream* pStream = pArray->GetStream(1);
    if (pStream == NULL) {
        return FALSE;
    }
    m_pProfile = pDoc->LoadIccProfile(pStream);
    if (!m_pProfile) {
        return FALSE;
    }
    m_nComponents = m_pProfile->GetComponents(); //Try using the nComponents from ICC profile
    CPDF_Dictionary* pDict = pStream->GetDict();
    if (m_pProfile->m_pTransform == NULL) { // No valid ICC profile or using sRGB
        CPDF_Object* pAlterCSObj = pDict ? pDict->GetElementValue(FX_BSTRC("Alternate")) : NULL;
        if (pAlterCSObj) {
            CPDF_ColorSpace* pAlterCS = CPDF_ColorSpace::Load(pDoc, pAlterCSObj);
            if (pAlterCS) {
                if (m_nComponents == 0) { // NO valid ICC profile
                    if (pAlterCS->CountComponents() > 0) { // Use Alternative colorspace
                        m_nComponents = pAlterCS->CountComponents();
                        m_pAlterCS = pAlterCS;
                        m_bOwn = TRUE;
                    }
                    else { // No valid alternative colorspace
                        pAlterCS->ReleaseCS();
                        FX_INT32 nDictComponents = pDict ? pDict->GetInteger(FX_BSTRC("N")) : 0;
                        if (nDictComponents != 1 && nDictComponents != 3 && nDictComponents != 4) {
                            return FALSE;
                        }
                        m_nComponents = nDictComponents;
                    }

                }
                else { // Using sRGB
                    if (pAlterCS->CountComponents() != m_nComponents) {
                        pAlterCS->ReleaseCS();
                    }
                    else {
                        m_pAlterCS = pAlterCS;
                        m_bOwn = TRUE;
                    }
                }
            }
        }
        if (!m_pAlterCS) {
            if (m_nComponents == 1) {
                m_pAlterCS = GetStockCS(PDFCS_DEVICEGRAY);
            }
            else if (m_nComponents == 3) {
                m_pAlterCS = GetStockCS(PDFCS_DEVICERGB);
            }
            else if (m_nComponents == 4) {
                m_pAlterCS = GetStockCS(PDFCS_DEVICECMYK);
            }
        }
    }
    CPDF_Array* pRanges = pDict->GetArray(FX_BSTRC("Range"));
    m_pRanges = FX_Alloc2D(FX_FLOAT, m_nComponents, 2);
    for (int i = 0; i < m_nComponents * 2; i ++) {
        if (pRanges) {
            m_pRanges[i] = pRanges->GetNumber(i);
        } else if (i % 2) {
            m_pRanges[i] = 1.0f;
        } else {
            m_pRanges[i] = 0;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_ICCBasedCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    if (m_pProfile && m_pProfile->m_bsRGB) {
        R = pBuf[0];
        G = pBuf[1];
        B = pBuf[2];
        return TRUE;
    }
    ICodec_IccModule *pIccModule = CPDF_ModuleMgr::Get()->GetIccModule();
    if (m_pProfile->m_pTransform == NULL || pIccModule == NULL) {
        if (m_pAlterCS) {
            m_pAlterCS->GetRGB(pBuf, R, G, B);
        } else {
            R = G = B = 0.0f;
        }
        return TRUE;
    }
    FX_FLOAT rgb[3];
    pIccModule->SetComponents(m_nComponents);
    pIccModule->Translate(m_pProfile->m_pTransform, pBuf, rgb);
    R = rgb[0];
    G = rgb[1];
    B = rgb[2];
    return TRUE;
}
FX_BOOL CPDF_ICCBasedCS::v_GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const
{
    if (m_nComponents != 4) {
        return FALSE;
    }
    c = pBuf[0];
    m = pBuf[1];
    y = pBuf[2];
    k = pBuf[3];
    return TRUE;
}
FX_BOOL CPDF_ICCBasedCS::SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
{
    return FALSE;
}
void CPDF_ICCBasedCS::EnableStdConversion(FX_BOOL bEnabled)
{
    CPDF_ColorSpace::EnableStdConversion(bEnabled);
    if (m_pAlterCS) {
        m_pAlterCS->EnableStdConversion(bEnabled);
    }
}
void CPDF_ICCBasedCS::TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    if (m_pProfile->m_bsRGB) {
        ReverseRGB(pDestBuf, pSrcBuf, pixels);
    } else if (m_pProfile->m_pTransform) {
        int nMaxColors = 1;
        for (int i = 0; i < m_nComponents; i ++) {
            nMaxColors *= 52;
        }
        if (m_nComponents > 3 || image_width * image_height < nMaxColors * 3 / 2) {
            CPDF_ModuleMgr::Get()->GetIccModule()->TranslateScanline(m_pProfile->m_pTransform, pDestBuf, pSrcBuf, pixels);
        } else {
            if (m_pCache == NULL) {
                ((CPDF_ICCBasedCS*)this)->m_pCache = FX_Alloc2D(FX_BYTE, nMaxColors, 3);
                FX_LPBYTE temp_src = FX_Alloc2D(FX_BYTE, nMaxColors, m_nComponents);
                FX_LPBYTE pSrc = temp_src;
                for (int i = 0; i < nMaxColors; i ++) {
                    FX_DWORD color = i;
                    FX_DWORD order = nMaxColors / 52;
                    for (int c = 0; c < m_nComponents; c ++) {
                        *pSrc++ = (FX_BYTE)(color / order * 5);
                        color %= order;
                        order /= 52;
                    }
                }
                CPDF_ModuleMgr::Get()->GetIccModule()->TranslateScanline(m_pProfile->m_pTransform, m_pCache, temp_src, nMaxColors);
                FX_Free(temp_src);
            }
            for (int i = 0; i < pixels; i ++) {
                int index = 0;
                for (int c = 0; c < m_nComponents; c ++) {
                    index = index * 52 + (*pSrcBuf) / 5;
                    pSrcBuf ++;
                }
                index *= 3;
                *pDestBuf++ = m_pCache[index];
                *pDestBuf++ = m_pCache[index + 1];
                *pDestBuf++ = m_pCache[index + 2];
            }
        }
    } else if (m_pAlterCS) {
        m_pAlterCS->TranslateImageLine(pDestBuf, pSrcBuf, pixels, image_width, image_height);
    }
}
class CPDF_IndexedCS : public CPDF_ColorSpace
{
public:
    CPDF_IndexedCS();
    virtual ~CPDF_IndexedCS();
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    void				GetDefaultValue(int iComponent, FX_FLOAT& min, FX_FLOAT& max) const
    {
        min = 0;
        max = (FX_FLOAT)m_MaxIndex;
    }
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual CPDF_ColorSpace*	GetBaseCS() const
    {
        return m_pBaseCS;
    }
    virtual void		EnableStdConversion(FX_BOOL bEnabled);
    CPDF_ColorSpace*	m_pBaseCS;
    CPDF_CountedColorSpace*     m_pCountedBaseCS;
    int					m_nBaseComponents;
    int					m_MaxIndex;
    CFX_ByteString		m_Table;
    FX_FLOAT*		m_pCompMinMax;
};
CPDF_IndexedCS::CPDF_IndexedCS()
{
    m_pBaseCS = NULL;
    m_pCountedBaseCS = NULL;
    m_Family = PDFCS_INDEXED;
    m_nComponents = 1;
    m_pCompMinMax = NULL;
}
CPDF_IndexedCS::~CPDF_IndexedCS()
{
    if (m_pCompMinMax) {
        FX_Free(m_pCompMinMax);
    }
    CPDF_ColorSpace* pCS = m_pCountedBaseCS ? m_pCountedBaseCS->m_Obj : NULL;
    if (pCS && m_pDocument) {
        m_pDocument->GetPageData()->ReleaseColorSpace(pCS->GetArray());
    }
}
FX_BOOL CPDF_IndexedCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    if (pArray->GetCount() < 4) {
        return FALSE;
    }
    CPDF_Object* pBaseObj = pArray->GetElementValue(1);
    if (pBaseObj == m_pArray) {
        return FALSE;
    }
    CPDF_DocPageData* pDocPageData = pDoc->GetPageData();
    m_pBaseCS = pDocPageData->GetColorSpace(pBaseObj, NULL);
    if (m_pBaseCS == NULL) {
        return FALSE;
    }
    m_pCountedBaseCS = pDocPageData->FindColorSpacePtr(m_pBaseCS->GetArray());
    m_nBaseComponents = m_pBaseCS->CountComponents();
    m_pCompMinMax = FX_Alloc2D(FX_FLOAT, m_nBaseComponents, 2);
    FX_FLOAT defvalue;
    for (int i = 0; i < m_nBaseComponents; i ++) {
        m_pBaseCS->GetDefaultValue(i, defvalue, m_pCompMinMax[i * 2], m_pCompMinMax[i * 2 + 1]);
        m_pCompMinMax[i * 2 + 1] -= m_pCompMinMax[i * 2];
    }
    m_MaxIndex = pArray->GetInteger(2);
    CPDF_Object* pTableObj = pArray->GetElementValue(3);
    if (pTableObj == NULL) {
        return FALSE;
    }
    if (pTableObj->GetType() == PDFOBJ_STRING) {
        m_Table = ((CPDF_String*)pTableObj)->GetString();
    } else if (pTableObj->GetType() == PDFOBJ_STREAM) {
        CPDF_StreamAcc acc;
        acc.LoadAllData((CPDF_Stream*)pTableObj, FALSE);
        m_Table = CFX_ByteStringC(acc.GetData(), acc.GetSize());
    }
    return TRUE;
}
FX_BOOL CPDF_IndexedCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    int index = (FX_INT32)(*pBuf);
    if (index < 0 || index > m_MaxIndex) {
        return FALSE;
    }
    if (m_nBaseComponents) {
        if (index == INT_MAX || (index + 1) > INT_MAX / m_nBaseComponents ||
                (index + 1)*m_nBaseComponents > (int)m_Table.GetLength()) {
            R = G = B = 0;
            return FALSE;
        }
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> Comps(m_nBaseComponents);
    FX_FLOAT* comps = Comps;
    FX_LPCBYTE pTable = m_Table;
    for (int i = 0; i < m_nBaseComponents; i ++) {
        comps[i] = m_pCompMinMax[i * 2] + m_pCompMinMax[i * 2 + 1] * pTable[index * m_nBaseComponents + i] / 255;
    }
    m_pBaseCS->GetRGB(comps, R, G, B);
    return TRUE;
}
void CPDF_IndexedCS::EnableStdConversion(FX_BOOL bEnabled)
{
    CPDF_ColorSpace::EnableStdConversion(bEnabled);
    if (m_pBaseCS) {
        m_pBaseCS->EnableStdConversion(bEnabled);
    }
}
#define MAX_PATTERN_COLORCOMPS	16
typedef struct _PatternValue {
    CPDF_Pattern*	m_pPattern;
    CPDF_CountedPattern*	m_pCountedPattern;
    int				m_nComps;
    FX_FLOAT		m_Comps[MAX_PATTERN_COLORCOMPS];
} PatternValue;
CPDF_PatternCS::CPDF_PatternCS()
{
    m_Family = PDFCS_PATTERN;
    m_pBaseCS = NULL;
    m_nComponents = 1;
    m_pCountedBaseCS = NULL;
}
CPDF_PatternCS::~CPDF_PatternCS()
{
    CPDF_ColorSpace* pCS = m_pCountedBaseCS ? m_pCountedBaseCS->m_Obj : NULL;
    if (pCS && m_pDocument) {
	    m_pDocument->GetPageData()->ReleaseColorSpace(pCS->GetArray());
    }
}
FX_BOOL CPDF_PatternCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Object* pBaseCS = pArray->GetElementValue(1);
    if (pBaseCS == m_pArray) {
        return FALSE;
    }
    CPDF_DocPageData* pDocPageData = pDoc->GetPageData();
    m_pBaseCS = pDocPageData->GetColorSpace(pBaseCS, NULL);
    if (m_pBaseCS) {
        if (m_pBaseCS->GetFamily() == PDFCS_PATTERN) {
            return FALSE;
        }
        m_pCountedBaseCS = pDocPageData->FindColorSpacePtr(m_pBaseCS->GetArray());
        m_nComponents = m_pBaseCS->CountComponents() + 1;
        if (m_pBaseCS->CountComponents() > MAX_PATTERN_COLORCOMPS) {
            return FALSE;
        }
    } else {
        m_nComponents = 1;
    }
    return TRUE;
}
FX_BOOL CPDF_PatternCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    if (m_pBaseCS) {
        ASSERT(m_pBaseCS->GetFamily() != PDFCS_PATTERN);
        PatternValue* pvalue = (PatternValue*)pBuf;
        if (m_pBaseCS->GetRGB(pvalue->m_Comps, R, G, B)) {
            return TRUE;
        }
    }
    R = G = B = 0.75f;
    return FALSE;
}
class CPDF_SeparationCS : public CPDF_ColorSpace
{
public:
    CPDF_SeparationCS();
    virtual ~CPDF_SeparationCS();
    virtual void		GetDefaultValue(int iComponent, FX_FLOAT& value, FX_FLOAT& min, FX_FLOAT& max) const
    {
        value = 1.0f;
        min = 0;
        max = 1.0f;
    }
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual void		EnableStdConversion(FX_BOOL bEnabled);
    CPDF_ColorSpace*	m_pAltCS;
    CPDF_Function*		m_pFunc;
    enum {None, All, Colorant}	m_Type;
};
CPDF_SeparationCS::CPDF_SeparationCS()
{
    m_Family = PDFCS_SEPARATION;
    m_pAltCS = NULL;
    m_pFunc = NULL;
    m_nComponents = 1;
}
CPDF_SeparationCS::~CPDF_SeparationCS()
{
    if (m_pAltCS) {
        m_pAltCS->ReleaseCS();
    }
    if (m_pFunc) {
        delete m_pFunc;
    }
}
FX_BOOL CPDF_SeparationCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CFX_ByteString name = pArray->GetString(1);
    if (name == FX_BSTRC("None")) {
        m_Type = None;
    } else {
        m_Type = Colorant;
        CPDF_Object* pAltCS = pArray->GetElementValue(2);
        if (pAltCS == m_pArray) {
            return FALSE;
        }
        m_pAltCS = Load(pDoc, pAltCS);
        CPDF_Object* pFuncObj = pArray->GetElementValue(3);
        if (pFuncObj && pFuncObj->GetType() != PDFOBJ_NAME) {
            m_pFunc = CPDF_Function::Load(pFuncObj);
        }
        if (m_pFunc && m_pAltCS && m_pFunc->CountOutputs() < m_pAltCS->CountComponents()) {
            delete m_pFunc;
            m_pFunc = NULL;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_SeparationCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    if (m_Type == None) {
        return FALSE;
    }
    if (m_pFunc == NULL) {
        if (m_pAltCS == NULL) {
            return FALSE;
        }
        int nComps = m_pAltCS->CountComponents();
        CFX_FixedBufGrow<FX_FLOAT, 16> results(nComps);
        for (int i = 0; i < nComps; i ++) {
            results[i] = *pBuf;
        }
        m_pAltCS->GetRGB(results, R, G, B);
        return TRUE;
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> results(m_pFunc->CountOutputs());
    int nresults = 0;
    m_pFunc->Call(pBuf, 1, results, nresults);
    if (nresults == 0) {
        return FALSE;
    }
    if (m_pAltCS) {
        m_pAltCS->GetRGB(results, R, G, B);
        return TRUE;
    } else {
        R = G = B = 0;
        return FALSE;
    }
}
void CPDF_SeparationCS::EnableStdConversion(FX_BOOL bEnabled)
{
    CPDF_ColorSpace::EnableStdConversion(bEnabled);
    if (m_pAltCS) {
        m_pAltCS->EnableStdConversion(bEnabled);
    }
}
class CPDF_DeviceNCS : public CPDF_ColorSpace
{
public:
    CPDF_DeviceNCS();
    virtual ~CPDF_DeviceNCS();
    virtual void		GetDefaultValue(int iComponent, FX_FLOAT& value, FX_FLOAT& min, FX_FLOAT& max) const
    {
        value = 1.0f;
        min = 0;
        max = 1.0f;
    }
    virtual FX_BOOL	v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual FX_BOOL	GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual void	EnableStdConversion(FX_BOOL bEnabled);
    CPDF_ColorSpace*	m_pAltCS;
    CPDF_Function*		m_pFunc;
};
CPDF_DeviceNCS::CPDF_DeviceNCS()
{
    m_Family = PDFCS_DEVICEN;
    m_pAltCS = NULL;
    m_pFunc = NULL;
}
CPDF_DeviceNCS::~CPDF_DeviceNCS()
{
    if (m_pFunc) {
        delete m_pFunc;
    }
    if (m_pAltCS) {
        m_pAltCS->ReleaseCS();
    }
}
FX_BOOL CPDF_DeviceNCS::v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
{
    CPDF_Object* pObj = pArray->GetElementValue(1);
    if (!pObj) {
        return FALSE;
    }
    if (pObj->GetType() != PDFOBJ_ARRAY) {
        return FALSE;
    }
    m_nComponents = ((CPDF_Array*)pObj)->GetCount();
    CPDF_Object* pAltCS = pArray->GetElementValue(2);
    if (!pAltCS || pAltCS == m_pArray) {
        return FALSE;
    }
    m_pAltCS = Load(pDoc, pAltCS);
    m_pFunc = CPDF_Function::Load(pArray->GetElementValue(3));
    if (m_pAltCS == NULL || m_pFunc == NULL) {
        return FALSE;
    }
    if (m_pFunc->CountOutputs() < m_pAltCS->CountComponents()) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_DeviceNCS::GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const
{
    if (m_pFunc == NULL) {
        return FALSE;
    }
    CFX_FixedBufGrow<FX_FLOAT, 16> results(m_pFunc->CountOutputs());
    int nresults = 0;
    m_pFunc->Call(pBuf, m_nComponents, results, nresults);
    if (nresults == 0) {
        return FALSE;
    }
    m_pAltCS->GetRGB(results, R, G, B);
    return TRUE;
}
void CPDF_DeviceNCS::EnableStdConversion(FX_BOOL bEnabled)
{
    CPDF_ColorSpace::EnableStdConversion(bEnabled);
    if (m_pAltCS) {
        m_pAltCS->EnableStdConversion(bEnabled);
    }
}
CPDF_ColorSpace* CPDF_ColorSpace::GetStockCS(int family)
{
    return CPDF_ModuleMgr::Get()->GetPageModule()->GetStockCS(family);;
}
CPDF_ColorSpace* _CSFromName(const CFX_ByteString& name)
{
    if (name == FX_BSTRC("DeviceRGB") || name == FX_BSTRC("RGB")) {
        return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICERGB);
    }
    if (name == FX_BSTRC("DeviceGray") || name == FX_BSTRC("G")) {
        return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICEGRAY);
    }
    if (name == FX_BSTRC("DeviceCMYK") || name == FX_BSTRC("CMYK")) {
        return CPDF_ColorSpace::GetStockCS(PDFCS_DEVICECMYK);
    }
    if (name == FX_BSTRC("Pattern")) {
        return CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN);
    }
    return NULL;
}
CPDF_ColorSpace* CPDF_ColorSpace::Load(CPDF_Document* pDoc, CPDF_Object* pObj)
{
    if (pObj == NULL) {
        return NULL;
    }
    if (pObj->GetType() == PDFOBJ_NAME) {
        return _CSFromName(pObj->GetString());
    }
    if (pObj->GetType() == PDFOBJ_STREAM) {
        CPDF_Dictionary *pDict = ((CPDF_Stream *)pObj)->GetDict();
        if (!pDict) {
            return NULL;
        }
        CPDF_ColorSpace *pRet = NULL;
        FX_POSITION pos = pDict->GetStartPos();
        while (pos) {
            CFX_ByteString bsKey;
            CPDF_Object *pValue = pDict->GetNextElement(pos, bsKey);
            if (pValue && pValue->GetType() == PDFOBJ_NAME) {
                pRet = _CSFromName(pValue->GetString());
            }
            if (pRet) {
                return pRet;
            }
        }
        return NULL;
    }
    if (pObj->GetType() != PDFOBJ_ARRAY) {
        return NULL;
    }
    CPDF_Array* pArray = (CPDF_Array*)pObj;
    if (pArray->GetCount() == 0) {
        return NULL;
    }
    CPDF_Object *pFamilyObj = pArray->GetElementValue(0);
    if (!pFamilyObj) {
        return NULL;
    }
    CFX_ByteString familyname = pFamilyObj->GetString();
    if (pArray->GetCount() == 1) {
        return _CSFromName(familyname);
    }
    CPDF_ColorSpace* pCS = NULL;
    FX_DWORD id = familyname.GetID();
    if (id == FXBSTR_ID('C', 'a', 'l', 'G')) {
        pCS = new CPDF_CalGray();
    } else if (id == FXBSTR_ID('C', 'a', 'l', 'R')) {
        pCS = new CPDF_CalRGB();
    } else if (id == FXBSTR_ID('L', 'a', 'b', 0)) {
        pCS = new CPDF_LabCS();
    } else if (id == FXBSTR_ID('I', 'C', 'C', 'B')) {
        pCS = new CPDF_ICCBasedCS();
    } else if (id == FXBSTR_ID('I', 'n', 'd', 'e') || id == FXBSTR_ID('I', 0, 0, 0)) {
        pCS = new CPDF_IndexedCS();
    } else if (id == FXBSTR_ID('S', 'e', 'p', 'a')) {
        pCS = new CPDF_SeparationCS();
    } else if (id == FXBSTR_ID('D', 'e', 'v', 'i')) {
        pCS = new CPDF_DeviceNCS();
    } else if (id == FXBSTR_ID('P', 'a', 't', 't')) {
        pCS = new CPDF_PatternCS();
    } else {
        return NULL;
    }
    pCS->m_pDocument = pDoc;
    pCS->m_pArray = pArray;
    if (!pCS->v_Load(pDoc, pArray)) {
        pCS->ReleaseCS();
        return NULL;
    }
    return pCS;
}
CPDF_ColorSpace::CPDF_ColorSpace()
{
    m_Family = 0;
    m_pArray = NULL;
    m_dwStdConversion = 0;
    m_pDocument = NULL;
}
void CPDF_ColorSpace::ReleaseCS()
{
    if (this == GetStockCS(PDFCS_DEVICERGB)) {
        return;
    }
    if (this == GetStockCS(PDFCS_DEVICEGRAY)) {
        return;
    }
    if (this == GetStockCS(PDFCS_DEVICECMYK)) {
        return;
    }
    if (this == GetStockCS(PDFCS_PATTERN)) {
        return;
    }
    delete this;
}
int CPDF_ColorSpace::GetBufSize() const
{
    if (m_Family == PDFCS_PATTERN) {
        return sizeof(PatternValue);
    }
    return m_nComponents * sizeof(FX_FLOAT);
}
FX_FLOAT* CPDF_ColorSpace::CreateBuf()
{
    int size = GetBufSize();
    FX_BYTE* pBuf = FX_Alloc(FX_BYTE, size);
    return (FX_FLOAT*)pBuf;
}
FX_BOOL CPDF_ColorSpace::sRGB() const
{
    if (m_Family == PDFCS_DEVICERGB) {
        return TRUE;
    }
    if (m_Family != PDFCS_ICCBASED) {
        return FALSE;
    }
    CPDF_ICCBasedCS* pCS = (CPDF_ICCBasedCS*)this;
    return pCS->m_pProfile->m_bsRGB;
}
FX_BOOL CPDF_ColorSpace::GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const
{
    if (v_GetCMYK(pBuf, c, m, y, k)) {
        return TRUE;
    }
    FX_FLOAT R, G, B;
    if (!GetRGB(pBuf, R, G, B)) {
        return FALSE;
    }
    sRGB_to_AdobeCMYK(R, G, B, c, m, y, k);
    return TRUE;
}
FX_BOOL CPDF_ColorSpace::SetCMYK(FX_FLOAT* pBuf, FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k) const
{
    if (v_SetCMYK(pBuf, c, m, y, k)) {
        return TRUE;
    }
    FX_FLOAT R, G, B;
    AdobeCMYK_to_sRGB(c, m, y, k, R, G, B);
    return SetRGB(pBuf, R, G, B);
}
void CPDF_ColorSpace::GetDefaultColor(FX_FLOAT* buf) const
{
    if (buf == NULL || m_Family == PDFCS_PATTERN) {
        return;
    }
    FX_FLOAT min, max;
    for (int i = 0; i < m_nComponents; i ++) {
        GetDefaultValue(i, buf[i], min, max);
    }
}
int CPDF_ColorSpace::GetMaxIndex() const
{
    if (m_Family != PDFCS_INDEXED) {
        return 0;
    }
    CPDF_IndexedCS* pCS = (CPDF_IndexedCS*)this;
    return pCS->m_MaxIndex;
}
void CPDF_ColorSpace::TranslateImageLine(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf, int pixels, int image_width, int image_height, FX_BOOL bTransMask) const
{
    CFX_FixedBufGrow<FX_FLOAT, 16> srcbuf(m_nComponents);
    FX_FLOAT* src = srcbuf;
    FX_FLOAT R, G, B;
    for (int i = 0; i < pixels; i ++) {
        for (int j = 0; j < m_nComponents; j ++)
            if (m_Family == PDFCS_INDEXED) {
                src[j] = (FX_FLOAT)(*src_buf ++);
            } else {
                src[j] = (FX_FLOAT)(*src_buf ++) / 255;
            }
        GetRGB(src, R, G, B);
        *dest_buf ++ = (FX_INT32)(B * 255);
        *dest_buf ++ = (FX_INT32)(G * 255);
        *dest_buf ++ = (FX_INT32)(R * 255);
    }
}
void CPDF_ColorSpace::EnableStdConversion(FX_BOOL bEnabled)
{
    if (bEnabled) {
        m_dwStdConversion ++;
    } else if (m_dwStdConversion) {
        m_dwStdConversion --;
    }
}
CPDF_Color::CPDF_Color(int family)
{
    m_pCS = CPDF_ColorSpace::GetStockCS(family);
    int nComps = 3;
    if (family == PDFCS_DEVICEGRAY) {
        nComps = 1;
    } else if (family == PDFCS_DEVICECMYK) {
        nComps = 4;
    }
    m_pBuffer = FX_Alloc(FX_FLOAT, nComps);
    for (int i = 0; i < nComps; i ++) {
        m_pBuffer[i] = 0;
    }
}
CPDF_Color::~CPDF_Color()
{
    ReleaseBuffer();
    ReleaseColorSpace();
}
void CPDF_Color::ReleaseBuffer()
{
    if (!m_pBuffer) {
        return;
    }
    if (m_pCS->GetFamily() == PDFCS_PATTERN) {
        PatternValue* pvalue = (PatternValue*)m_pBuffer;
        CPDF_Pattern* pPattern = pvalue->m_pCountedPattern ? pvalue->m_pCountedPattern->m_Obj : NULL;
        if (pPattern && pPattern->m_pDocument) {
            CPDF_DocPageData *pPageData = pPattern->m_pDocument->GetPageData();
            if (pPageData) {
                pPageData->ReleasePattern(pPattern->m_pPatternObj);
            }
        }
    }
    FX_Free(m_pBuffer);
    m_pBuffer = NULL;
}
void CPDF_Color::ReleaseColorSpace()
{
    if (m_pCS && m_pCS->m_pDocument && m_pCS->GetArray()) {
        m_pCS->m_pDocument->GetPageData()->ReleaseColorSpace(m_pCS->GetArray());
        m_pCS = NULL;
    }
}
void CPDF_Color::SetColorSpace(CPDF_ColorSpace* pCS)
{
    if (m_pCS == pCS) {
        if (m_pBuffer == NULL) {
            m_pBuffer = pCS->CreateBuf();
        }
        ReleaseColorSpace();
        m_pCS = pCS;
        return;
    }
    ReleaseBuffer();
    ReleaseColorSpace();
    m_pCS = pCS;
    if (m_pCS) {
        m_pBuffer = pCS->CreateBuf();
        pCS->GetDefaultColor(m_pBuffer);
    }
}
void CPDF_Color::SetValue(FX_FLOAT* comps)
{
    if (m_pBuffer == NULL) {
        return;
    }
    if (m_pCS->GetFamily() != PDFCS_PATTERN) {
        FXSYS_memcpy32(m_pBuffer, comps, m_pCS->CountComponents() * sizeof(FX_FLOAT));
    }
}
void CPDF_Color::SetValue(CPDF_Pattern* pPattern, FX_FLOAT* comps, int ncomps)
{
    if (ncomps > MAX_PATTERN_COLORCOMPS) {
        return;
    }
    if (m_pCS == NULL || m_pCS->GetFamily() != PDFCS_PATTERN) {
        if (m_pBuffer) {
            FX_Free(m_pBuffer);
        }
        m_pCS = CPDF_ColorSpace::GetStockCS(PDFCS_PATTERN);
        m_pBuffer = m_pCS->CreateBuf();
    }
    CPDF_DocPageData *pDocPageData = NULL;
    PatternValue* pvalue = (PatternValue*)m_pBuffer;
    if (pvalue->m_pPattern && pvalue->m_pPattern->m_pDocument) {
        pDocPageData = pvalue->m_pPattern->m_pDocument->GetPageData();
        if (pDocPageData) {
            pDocPageData->ReleasePattern(pvalue->m_pPattern->m_pPatternObj);
        }
    }
    pvalue->m_nComps = ncomps;
    pvalue->m_pPattern = pPattern;
    if (ncomps) {
        FXSYS_memcpy32(pvalue->m_Comps, comps, ncomps * sizeof(FX_FLOAT));
    }
    pvalue->m_pCountedPattern = NULL;
    if (pPattern && pPattern->m_pDocument)
    {
        if (!pDocPageData) {
            pDocPageData = pPattern->m_pDocument->GetPageData();
        }
        pvalue->m_pCountedPattern = pDocPageData->FindPatternPtr(pPattern->m_pPatternObj);
    }
}
void CPDF_Color::Copy(const CPDF_Color* pSrc)
{
    ReleaseBuffer();
    ReleaseColorSpace();
    m_pCS = pSrc->m_pCS;
    if (m_pCS && m_pCS->m_pDocument) {
        CPDF_Array* pArray = m_pCS->GetArray();
        if (pArray) {
            m_pCS = m_pCS->m_pDocument->GetPageData()->GetCopiedColorSpace(pArray);
        }
    }
    if (m_pCS == NULL) {
        return;
    }
    m_pBuffer = m_pCS->CreateBuf();
    FXSYS_memcpy32(m_pBuffer, pSrc->m_pBuffer, m_pCS->GetBufSize());
    if (m_pCS->GetFamily() == PDFCS_PATTERN) {
        PatternValue* pvalue = (PatternValue*)m_pBuffer;
        if (pvalue->m_pPattern && pvalue->m_pPattern->m_pDocument) {
            pvalue->m_pPattern = pvalue->m_pPattern->m_pDocument->GetPageData()->GetPattern(pvalue->m_pPattern->m_pPatternObj, FALSE, &pvalue->m_pPattern->m_ParentMatrix);
        }
    }
}
FX_BOOL CPDF_Color::GetRGB(int& R, int& G, int& B) const
{
    if (m_pCS == NULL || m_pBuffer == NULL) {
        return FALSE;
    }
    FX_FLOAT r=0.0f, g=0.0f, b=0.0f;
    if (!m_pCS->GetRGB(m_pBuffer, r, g, b)) {
        return FALSE;
    }
    R = (FX_INT32)(r * 255 + 0.5f);
    G = (FX_INT32)(g * 255 + 0.5f);
    B = (FX_INT32)(b * 255 + 0.5f);
    return TRUE;
}
CPDF_Pattern* CPDF_Color::GetPattern() const
{
    if (m_pBuffer == NULL || m_pCS->GetFamily() != PDFCS_PATTERN) {
        return NULL;
    }
    PatternValue* pvalue = (PatternValue*)m_pBuffer;
    return pvalue->m_pPattern;
}
CPDF_ColorSpace* CPDF_Color::GetPatternCS() const
{
    if (m_pBuffer == NULL || m_pCS->GetFamily() != PDFCS_PATTERN) {
        return NULL;
    }
    return m_pCS->GetBaseCS();
}
FX_FLOAT* CPDF_Color::GetPatternColor() const
{
    if (m_pBuffer == NULL || m_pCS->GetFamily() != PDFCS_PATTERN) {
        return NULL;
    }
    PatternValue* pvalue = (PatternValue*)m_pBuffer;
    return pvalue->m_nComps ? pvalue->m_Comps : NULL;
}
FX_BOOL CPDF_Color::IsEqual(const CPDF_Color& other) const
{
    if (m_pCS != other.m_pCS || m_pCS == NULL) {
        return FALSE;
    }
    return FXSYS_memcmp32(m_pBuffer, other.m_pBuffer, m_pCS->GetBufSize()) == 0;
}
