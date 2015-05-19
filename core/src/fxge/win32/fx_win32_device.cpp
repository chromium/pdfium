// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#if _FX_OS_ == _FX_WIN32_DESKTOP_ || _FX_OS_ == _FX_WIN64_
#include "../../../include/fxge/fx_ge_win32.h"
#include <crtdbg.h>
#include "../agg/include/fxfx_agg_clip_liang_barsky.h"
#include "dwrite_int.h"
#include "win32_int.h"
#include "../ge/text_int.h"
#include "../dib/dib_int.h"
#include "../agg/include/fx_agg_driver.h"
#include "../../../include/fxge/fx_freetype.h"
#include "../../../include/fxcodec/fx_codec.h"
class CWin32FontInfo FX_FINAL : public IFX_SystemFontInfo
{
public:
    CWin32FontInfo();
    ~CWin32FontInfo();
    virtual void		Release();
    virtual	FX_BOOL		EnumFontList(CFX_FontMapper* pMapper);
    virtual void*		MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR face, FX_BOOL& bExact);
    virtual void*		GetFont(FX_LPCSTR face)
    {
        return NULL;
    }
    virtual FX_DWORD	GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size);
    virtual void		DeleteFont(void* hFont);
    virtual	FX_BOOL		GetFaceName(void* hFont, CFX_ByteString& name);
    virtual FX_BOOL		GetFontCharset(void* hFont, int& charset);
    FX_BOOL				IsOpenTypeFromDiv(const LOGFONTA *plf);
    FX_BOOL				IsSupportFontFormDiv(const LOGFONTA* plf);
    void				AddInstalledFont(const LOGFONTA *plf, FX_DWORD FontType);
    void				GetGBPreference(CFX_ByteString& face, int weight, int picth_family);
    void				GetJapanesePreference(CFX_ByteString& face, int weight, int picth_family);
    CFX_ByteString		FindFont(const CFX_ByteString& name);
    HDC					m_hDC;
    CFX_FontMapper*		m_pMapper;
    CFX_ByteString		m_LastFamily;
    CFX_ByteString		m_KaiTi, m_FangSong;
};
CWin32FontInfo::CWin32FontInfo()
{
    m_hDC = CreateCompatibleDC(NULL);
}
CWin32FontInfo::~CWin32FontInfo()
{
    m_pMapper = NULL;
}
void CWin32FontInfo::Release()
{
    DeleteDC(m_hDC);
    delete this;
}
#define TT_MAKE_TAG(x1, x2, x3, x4) (((FX_DWORD)x1<<24)|((FX_DWORD)x2<<16)|((FX_DWORD)x3<<8)|(FX_DWORD)x4)
FX_BOOL CWin32FontInfo::IsOpenTypeFromDiv(const LOGFONTA *plf)
{
    HFONT hFont = CreateFontIndirectA(plf);
    FX_BOOL ret = FALSE;
    FX_DWORD font_size  = GetFontData(hFont, 0, NULL, 0);
    if (font_size != GDI_ERROR && font_size >= sizeof(FX_DWORD)) {
        FX_DWORD lVersion = 0;
        GetFontData(hFont, 0, (FX_BYTE*)(&lVersion), sizeof(FX_DWORD));
        lVersion = (((FX_DWORD)(FX_BYTE)(lVersion)) << 24) | ((FX_DWORD)((FX_BYTE)(lVersion >> 8))) << 16 |
                   ((FX_DWORD)((FX_BYTE)(lVersion >> 16))) << 8 | ((FX_BYTE)(lVersion >> 24));
        if (lVersion == TT_MAKE_TAG('O', 'T', 'T', 'O') ||
                lVersion == 0x00010000 ||
                lVersion == TT_MAKE_TAG('t', 't', 'c', 'f') ||
                lVersion == TT_MAKE_TAG('t', 'r', 'u', 'e') ||
                lVersion == 0x00020000) {
            ret = TRUE;
        }
    }
    DeleteFont(hFont);
    return ret;
}
FX_BOOL CWin32FontInfo::IsSupportFontFormDiv(const LOGFONTA* plf)
{
    HFONT hFont = CreateFontIndirectA(plf);
    FX_BOOL ret = FALSE;
    FX_DWORD font_size  = GetFontData(hFont, 0, NULL, 0);
    if (font_size != GDI_ERROR && font_size >= sizeof(FX_DWORD)) {
        FX_DWORD lVersion = 0;
        GetFontData(hFont, 0, (FX_BYTE*)(&lVersion), sizeof(FX_DWORD));
        lVersion = (((FX_DWORD)(FX_BYTE)(lVersion)) << 24) | ((FX_DWORD)((FX_BYTE)(lVersion >> 8))) << 16 |
                   ((FX_DWORD)((FX_BYTE)(lVersion >> 16))) << 8 | ((FX_BYTE)(lVersion >> 24));
        if (lVersion == TT_MAKE_TAG('O', 'T', 'T', 'O') ||
                lVersion == 0x00010000 ||
                lVersion == TT_MAKE_TAG('t', 't', 'c', 'f') ||
                lVersion == TT_MAKE_TAG('t', 'r', 'u', 'e') ||
                lVersion == 0x00020000) {
            ret = TRUE;
        } else if ((lVersion & 0xFFFF0000) == TT_MAKE_TAG(0x80, 0x01, 0x00, 0x00) ||
                   (lVersion & 0xFFFF0000) == TT_MAKE_TAG('%', '!', 0, 0)) {
            ret = TRUE;
        }
    }
    DeleteFont(hFont);
    return ret;
}
void CWin32FontInfo::AddInstalledFont(const LOGFONTA *plf, FX_DWORD FontType)
{
    CFX_ByteString name(plf->lfFaceName, -1);
    if (name[0] == '@') {
        return;
    }
    if (name == m_LastFamily) {
        m_pMapper->AddInstalledFont(name, plf->lfCharSet);
        return;
    }
    if (!(FontType & TRUETYPE_FONTTYPE) && !(FontType & DEVICE_FONTTYPE)) {
        return;
    }
    if (!(FontType & TRUETYPE_FONTTYPE)) {
        if (!IsSupportFontFormDiv(plf)) {
            return;
        }
    }
    m_pMapper->AddInstalledFont(name, plf->lfCharSet);
    m_LastFamily = name;
}
static int CALLBACK FontEnumProc(
    const LOGFONTA *plf,
    const TEXTMETRICA *lpntme,
    FX_DWORD FontType,
    LPARAM lParam
)
{
    CWin32FontInfo* pFontInfo = (CWin32FontInfo*)lParam;
    if (pFontInfo->m_pMapper->GetFontEnumerator()) {
        pFontInfo->m_pMapper->GetFontEnumerator()->HitFont();
    }
    pFontInfo->AddInstalledFont(plf, FontType);
    return 1;
}
FX_BOOL CWin32FontInfo::EnumFontList(CFX_FontMapper* pMapper)
{
    m_pMapper = pMapper;
    LOGFONTA lf;
    FXSYS_memset32(&lf, 0, sizeof(LOGFONTA));
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfFaceName[0] = 0;
    lf.lfPitchAndFamily = 0;
    EnumFontFamiliesExA(m_hDC, &lf, (FONTENUMPROCA)FontEnumProc, (FX_UINTPTR)this, 0);
    if (pMapper->GetFontEnumerator()) {
        pMapper->GetFontEnumerator()->Finish();
    }
    return TRUE;
}
static const struct {
    FX_LPCSTR	m_pFaceName;
    FX_LPCSTR	m_pVariantName;
}
VariantNames[] = {
    {"DFKai-SB", "\x19\x6A\x77\x69\xD4\x9A"},
};
static const struct {
    FX_LPCSTR	m_pName;
    FX_LPCSTR	m_pWinName;
    FX_BOOL		m_bBold;
    FX_BOOL		m_bItalic;
}
Base14Substs[] = {
    {"Courier", "Courier New", FALSE, FALSE},
    {"Courier-Bold", "Courier New", TRUE, FALSE},
    {"Courier-BoldOblique", "Courier New", TRUE, TRUE},
    {"Courier-Oblique", "Courier New", FALSE, TRUE},
    {"Helvetica", "Arial", FALSE, FALSE},
    {"Helvetica-Bold", "Arial", TRUE, FALSE},
    {"Helvetica-BoldOblique", "Arial", TRUE, TRUE},
    {"Helvetica-Oblique", "Arial", FALSE, TRUE},
    {"Times-Roman", "Times New Roman", FALSE, FALSE},
    {"Times-Bold", "Times New Roman", TRUE, FALSE},
    {"Times-BoldItalic", "Times New Roman", TRUE, TRUE},
    {"Times-Italic", "Times New Roman", FALSE, TRUE},
};
CFX_ByteString CWin32FontInfo::FindFont(const CFX_ByteString& name)
{
    if (m_pMapper == NULL) {
        return name;
    }
    int nFonts = m_pMapper->m_InstalledTTFonts.GetSize();
    for (int i = 0; i < nFonts; i ++) {
        CFX_ByteString thisname = m_pMapper->m_InstalledTTFonts[i];
        if (thisname[0] == ' ') {
            if (thisname.Mid(1, name.GetLength()) == name) {
                return m_pMapper->m_InstalledTTFonts[i + 1];
            }
        } else if (thisname.Left(name.GetLength()) == name) {
            return m_pMapper->m_InstalledTTFonts[i];
        }
    }
    return CFX_ByteString();
}
struct _FontNameMap {
    FX_LPCSTR	m_pSubFontName;
    FX_LPCSTR	m_pSrcFontName;
};
const _FontNameMap g_JpFontNameMap[] = {
    {"MS Mincho", "Heiseimin-W3"},
    {"MS Gothic", "Jun101-Light"},
};
extern "C" {
    static int compareString(const void* key, const void* element)
    {
        return FXSYS_stricmp((FX_LPCSTR)key, ((_FontNameMap*)element)->m_pSrcFontName);
    }
}
FX_BOOL _GetSubFontName(CFX_ByteString& name)
{
    int size = sizeof g_JpFontNameMap;
    void* pFontnameMap = (void*)g_JpFontNameMap;
    _FontNameMap* found = (_FontNameMap*)FXSYS_bsearch(name.c_str(), pFontnameMap,
                          size / sizeof (_FontNameMap), sizeof (_FontNameMap), compareString);
    if (found == NULL) {
        return FALSE;
    }
    name = found->m_pSubFontName;
    return TRUE;
}
void CWin32FontInfo::GetGBPreference(CFX_ByteString& face, int weight, int picth_family)
{
    if (face.Find("KaiTi") >= 0 || face.Find("\xbf\xac") >= 0) {
        if (m_KaiTi.IsEmpty()) {
            m_KaiTi = FindFont("KaiTi");
            if (m_KaiTi.IsEmpty()) {
                m_KaiTi = "SimSun";
            }
        }
        face = m_KaiTi;
    } else if (face.Find("FangSong") >= 0 || face.Find("\xb7\xc2\xcb\xce") >= 0) {
        if (m_FangSong.IsEmpty()) {
            m_FangSong = FindFont("FangSong");
            if (m_FangSong.IsEmpty()) {
                m_FangSong = "SimSun";
            }
        }
        face = m_FangSong;
    } else if (face.Find("SimSun") >= 0 || face.Find("\xcb\xce") >= 0) {
        face = "SimSun";
    } else if (face.Find("SimHei") >= 0 || face.Find("\xba\xda") >= 0) {
        face = "SimHei";
    } else if (!(picth_family & FF_ROMAN) && weight > 550) {
        face = "SimHei";
    } else {
        face = "SimSun";
    }
}
void CWin32FontInfo::GetJapanesePreference(CFX_ByteString& face, int weight, int picth_family)
{
    if (face.Find("Gothic") >= 0 || face.Find("\x83\x53\x83\x56\x83\x62\x83\x4e") >= 0) {
        if (face.Find("PGothic") >= 0 || face.Find("\x82\x6f\x83\x53\x83\x56\x83\x62\x83\x4e") >= 0) {
            face = "MS PGothic";
        } else if (face.Find("UI Gothic") >= 0) {
            face = "MS UI Gothic";
        } else {
            if (face.Find("HGSGothicM") >= 0 || face.Find("HGMaruGothicMPRO") >= 0) {
                face = "MS PGothic";
            } else {
                face = "MS Gothic";
            }
        }
        return;
    } else if (face.Find("Mincho") >= 0 || face.Find("\x96\xbe\x92\xa9") >= 0) {
        if (face.Find("PMincho") >= 0 || face.Find("\x82\x6f\x96\xbe\x92\xa9") >= 0) {
            face = "MS PMincho";
        } else {
            face = "MS Mincho";
        }
        return;
    }
    if (_GetSubFontName(face)) {
        return;
    }
    if (!(picth_family & FF_ROMAN) && weight > 400) {
        face = "MS PGothic";
    } else {
        face = "MS PMincho";
    }
}
void* CWin32FontInfo::MapFont(int weight, FX_BOOL bItalic, int charset, int pitch_family, FX_LPCSTR cstr_face, FX_BOOL& bExact)
{
    CFX_ByteString face = cstr_face;
    int iBaseFont;
    for (iBaseFont = 0; iBaseFont < 12; iBaseFont ++)
        if (face == CFX_ByteStringC(Base14Substs[iBaseFont].m_pName)) {
            face = Base14Substs[iBaseFont].m_pWinName;
            weight = Base14Substs[iBaseFont].m_bBold ? FW_BOLD : FW_NORMAL;
            bItalic = Base14Substs[iBaseFont].m_bItalic;
            bExact = TRUE;
            break;
        }
    if (charset == ANSI_CHARSET || charset == SYMBOL_CHARSET) {
        charset = DEFAULT_CHARSET;
    }
    int subst_pitch_family = pitch_family;
    switch (charset) {
        case SHIFTJIS_CHARSET:
            subst_pitch_family = FF_ROMAN;
            break;
        case CHINESEBIG5_CHARSET:
        case HANGUL_CHARSET:
        case GB2312_CHARSET:
            subst_pitch_family = 0;
            break;
    }
    HFONT hFont = ::CreateFontA(-10, 0, 0, 0, weight, bItalic, 0, 0, charset, OUT_TT_ONLY_PRECIS,
                                0, 0, subst_pitch_family, face);
    char facebuf[100];
    HFONT hOldFont = (HFONT)::SelectObject(m_hDC, hFont);
    int ret = ::GetTextFaceA(m_hDC, 100, facebuf);
    ::SelectObject(m_hDC, hOldFont);
    if (face.EqualNoCase(facebuf)) {
        return hFont;
    }
    int iCount = sizeof(VariantNames) / sizeof(VariantNames[0]);
    for (int i = 0; i < iCount; ++i) {
        if (face == VariantNames[i].m_pFaceName) {
            CFX_WideString wsFace = CFX_WideString::FromLocal(facebuf);
            const unsigned short* pName = (const unsigned short*)VariantNames[i].m_pVariantName;
            FX_STRSIZE len = CFX_WideString::WStringLength(pName);
            CFX_WideString wsName = CFX_WideString::FromUTF16LE(pName, len);
            if (wsFace == wsName) {
                return hFont;
            }
        }
    }
    ::DeleteObject(hFont);
    if (charset == DEFAULT_CHARSET) {
        return NULL;
    }
    switch (charset) {
        case SHIFTJIS_CHARSET:
            GetJapanesePreference(face, weight, pitch_family);
            break;
        case GB2312_CHARSET:
            GetGBPreference(face, weight, pitch_family);
            break;
        case HANGUL_CHARSET:
            face = "Gulim";
            break;
        case CHINESEBIG5_CHARSET:
            if (face.Find("MSung") >= 0) {
                face = "MingLiU";
            } else {
                face = "PMingLiU";
            }
            break;
    }
    hFont = ::CreateFontA(-10, 0, 0, 0, weight, bItalic, 0, 0, charset, OUT_TT_ONLY_PRECIS,
                          0, 0, subst_pitch_family, face);
    return hFont;
}
void CWin32FontInfo::DeleteFont(void* hFont)
{
    ::DeleteObject(hFont);
}
FX_DWORD CWin32FontInfo::GetFontData(void* hFont, FX_DWORD table, FX_LPBYTE buffer, FX_DWORD size)
{
    HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
    table = FXDWORD_FROM_MSBFIRST(table);
    size = ::GetFontData(m_hDC, table, 0, buffer, size);
    ::SelectObject(m_hDC, hOldFont);
    if (size == GDI_ERROR) {
        return 0;
    }
    return size;
}
FX_BOOL CWin32FontInfo::GetFaceName(void* hFont, CFX_ByteString& name)
{
    char facebuf[100];
    HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
    int ret = ::GetTextFaceA(m_hDC, 100, facebuf);
    ::SelectObject(m_hDC, hOldFont);
    if (ret == 0) {
        return FALSE;
    }
    name = facebuf;
    return TRUE;
}
FX_BOOL CWin32FontInfo::GetFontCharset(void* hFont, int& charset)
{
    TEXTMETRIC tm;
    HFONT hOldFont = (HFONT)::SelectObject(m_hDC, (HFONT)hFont);
    ::GetTextMetrics(m_hDC, &tm);
    ::SelectObject(m_hDC, hOldFont);
    charset = tm.tmCharSet;
    return TRUE;
}
IFX_SystemFontInfo* IFX_SystemFontInfo::CreateDefault()
{
    return new CWin32FontInfo;
}
void CFX_GEModule::InitPlatform()
{
    CWin32Platform* pPlatformData = new CWin32Platform;
    OSVERSIONINFO ver;
    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);
    pPlatformData->m_bHalfTone = ver.dwMajorVersion >= 5;
    pPlatformData->m_GdiplusExt.Load();
    m_pPlatformData = pPlatformData;
    m_pFontMgr->SetSystemFontInfo(IFX_SystemFontInfo::CreateDefault());
}
void CFX_GEModule::DestroyPlatform()
{
    if (m_pPlatformData) {
        delete (CWin32Platform*)m_pPlatformData;
    }
    m_pPlatformData = NULL;
}
CGdiDeviceDriver::CGdiDeviceDriver(HDC hDC, int device_class)
{
    m_hDC = hDC;
    m_DeviceClass = device_class;
    CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
    SetStretchBltMode(hDC, pPlatform->m_bHalfTone ? HALFTONE : COLORONCOLOR);
    if (GetObjectType(m_hDC) == OBJ_MEMDC) {
        HBITMAP hBitmap = CreateBitmap(1, 1, 1, 1, NULL);
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
    if (m_DeviceClass != FXDC_DISPLAY) {
        m_RenderCaps = FXRC_BIT_MASK;
    } else {
        m_RenderCaps = FXRC_GET_BITS | FXRC_BIT_MASK;
    }
}
int CGdiDeviceDriver::GetDeviceCaps(int caps_id)
{
    switch (caps_id) {
        case FXDC_DEVICE_CLASS:
            return m_DeviceClass;
        case FXDC_PIXEL_WIDTH:
            return m_Width;
        case FXDC_PIXEL_HEIGHT:
            return m_Height;
        case FXDC_BITS_PIXEL:
            return m_nBitsPerPixel;
        case FXDC_RENDER_CAPS:
            return m_RenderCaps;
    }
    return 0;
}
FX_LPVOID CGdiDeviceDriver::GetClipRgn()
{
    HRGN hClipRgn = CreateRectRgn(0, 0, 1, 1);
    if (::GetClipRgn(m_hDC, hClipRgn) == 0) {
        DeleteObject(hClipRgn);
        hClipRgn = NULL;
    }
    return (FX_LPVOID)hClipRgn;
}
FX_BOOL CGdiDeviceDriver::GDI_SetDIBits(const CFX_DIBitmap* pBitmap1, const FX_RECT* pSrcRect, int left, int top, void* pIccTransform)
{
    if (m_DeviceClass == FXDC_PRINTER) {
        CFX_DIBitmap* pBitmap = pBitmap1->FlipImage(FALSE, TRUE);
        if (pBitmap == NULL) {
            return FALSE;
        }
        if ((pBitmap->IsCmykImage() || pIccTransform) &&
                !pBitmap->ConvertFormat(FXDIB_Rgb, pIccTransform)) {
            return FALSE;
        }
        int width = pSrcRect->Width(), height = pSrcRect->Height();
        int pitch = pBitmap->GetPitch();
        LPBYTE pBuffer = pBitmap->GetBuffer();
        CFX_ByteString info = CFX_WindowsDIB::GetBitmapInfo(pBitmap);
        ((BITMAPINFOHEADER*)info.c_str())->biHeight *= -1;
        FX_RECT dst_rect(0, 0, width, height);
        dst_rect.Intersect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight());
        int dst_width = dst_rect.Width();
        int dst_height = dst_rect.Height();
        ::StretchDIBits(m_hDC, left, top, dst_width, dst_height,
            0, 0, dst_width, dst_height, pBuffer, (BITMAPINFO*)info.c_str(), DIB_RGB_COLORS, SRCCOPY);
        delete pBitmap;
    } else {
        CFX_DIBitmap* pBitmap = (CFX_DIBitmap*)pBitmap1;
        if ((pBitmap->IsCmykImage() || pIccTransform) &&
                (pBitmap = pBitmap->CloneConvert(FXDIB_Rgb, NULL, pIccTransform)) == NULL) {
            return FALSE;
        }
        int width = pSrcRect->Width(), height = pSrcRect->Height();
        int pitch = pBitmap->GetPitch();
        LPBYTE pBuffer = pBitmap->GetBuffer();
        CFX_ByteString info = CFX_WindowsDIB::GetBitmapInfo(pBitmap);
        ::SetDIBitsToDevice(m_hDC, left, top, width, height, pSrcRect->left, pBitmap->GetHeight() - pSrcRect->bottom,
            0, pBitmap->GetHeight(), pBuffer, (BITMAPINFO*)info.c_str(), DIB_RGB_COLORS);
        if (pBitmap != pBitmap1) {
            delete pBitmap;
        }
    }
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::GDI_StretchDIBits(const CFX_DIBitmap* pBitmap1, int dest_left, int dest_top,
        int dest_width, int dest_height, FX_DWORD flags, void* pIccTransform)
{
    CFX_DIBitmap* pBitmap = (CFX_DIBitmap*)pBitmap1;
    if (pBitmap == NULL || dest_width == 0  || dest_height == 0) {
        return FALSE;
    }
    if ((pBitmap->IsCmykImage() || pIccTransform) &&
            !pBitmap->ConvertFormat(FXDIB_Rgb, pIccTransform)) {
        return FALSE;
    }
    CFX_ByteString info = CFX_WindowsDIB::GetBitmapInfo(pBitmap);
    if ((FX_INT64)abs(dest_width) * abs(dest_height) < (FX_INT64)pBitmap1->GetWidth() * pBitmap1->GetHeight() * 4 ||
            (flags & FXDIB_INTERPOL) || (flags & FXDIB_BICUBIC_INTERPOL)) {
        SetStretchBltMode(m_hDC, HALFTONE);
    } else {
        SetStretchBltMode(m_hDC, COLORONCOLOR);
    }
    CFX_DIBitmap* pToStrechBitmap = pBitmap;
    bool del = false;
    if (m_DeviceClass == FXDC_PRINTER && ((FX_INT64)pBitmap->GetWidth() * pBitmap->GetHeight() > (FX_INT64)abs(dest_width) * abs(dest_height))) {
        pToStrechBitmap = pBitmap->StretchTo(dest_width, dest_height);
        del = true;
    }
    CFX_ByteString toStrechBitmapInfo = CFX_WindowsDIB::GetBitmapInfo(pToStrechBitmap);
    ::StretchDIBits(m_hDC, dest_left, dest_top, dest_width, dest_height,
                    0, 0, pToStrechBitmap->GetWidth(), pToStrechBitmap->GetHeight(), pToStrechBitmap->GetBuffer(),
                    (BITMAPINFO*)toStrechBitmapInfo.c_str(), DIB_RGB_COLORS, SRCCOPY);
    if (del) {
        delete pToStrechBitmap;
    }
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::GDI_StretchBitMask(const CFX_DIBitmap* pBitmap1, int dest_left, int dest_top,
        int dest_width, int dest_height, FX_DWORD bitmap_color, FX_DWORD flags,
        int alpha_flag, void* pIccTransform)
{
    CFX_DIBitmap* pBitmap = (CFX_DIBitmap*)pBitmap1;
    if (pBitmap == NULL || dest_width == 0  || dest_height == 0) {
        return FALSE;
    }
    _Color2Argb(bitmap_color, bitmap_color, alpha_flag | (1 << 24), pIccTransform);
    int width = pBitmap->GetWidth(), height = pBitmap->GetHeight();
    struct {
        BITMAPINFOHEADER	bmiHeader;
        FX_DWORD			bmiColors[2];
    } bmi;
    FXSYS_memset32(&bmi.bmiHeader, 0, sizeof (BITMAPINFOHEADER));
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

    HBRUSH hPattern = CreateSolidBrush(bitmap_color & 0xffffff);
    HBRUSH hOld = (HBRUSH)SelectObject(m_hDC, hPattern);

    
    // In PDF, when image mask is 1, use device bitmap; when mask is 0, use brush bitmap.
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
    // The boolen codes is B8. Based on http://msdn.microsoft.com/en-us/library/aa932106.aspx, the ROP3 code is 0xB8074A

    ::StretchDIBits(m_hDC, dest_left, dest_top, dest_width, dest_height,
                    0, 0, width, height, pBitmap->GetBuffer(), (BITMAPINFO*)&bmi, DIB_RGB_COLORS, 0xB8074A);

    SelectObject(m_hDC, hOld);
    DeleteObject(hPattern);

    return TRUE;
}
BOOL CGdiDeviceDriver::GetClipBox(FX_RECT* pRect)
{
    return ::GetClipBox(m_hDC, (RECT*)pRect);
}
FX_BOOL CGdiDeviceDriver::SetClipRgn(FX_LPVOID hRgn)
{
    ::SelectClipRgn(m_hDC, (HRGN)hRgn);
    return TRUE;
}
static HPEN _CreatePen(const CFX_GraphStateData* pGraphState, const CFX_AffineMatrix* pMatrix, FX_DWORD argb)
{
    FX_FLOAT width;
    FX_FLOAT scale = 1.f;
    if (pMatrix)
        scale = FXSYS_fabs(pMatrix->a) > FXSYS_fabs(pMatrix->b) ?
                FXSYS_fabs(pMatrix->a) : FXSYS_fabs(pMatrix->b);
    if (pGraphState) {
        width = scale * pGraphState->m_LineWidth;
    } else {
        width = 1.0f;
    }
    FX_DWORD PenStyle = PS_GEOMETRIC;
    if (width < 1) {
        width = 1;
    }
    if(pGraphState->m_DashCount) {
        PenStyle |= PS_USERSTYLE;
    } else {
        PenStyle |= PS_SOLID;
    }
    switch(pGraphState->m_LineCap) {
        case 0:
            PenStyle |= PS_ENDCAP_FLAT;
            break;
        case 1:
            PenStyle |= PS_ENDCAP_ROUND;
            break;
        case 2:
            PenStyle |= PS_ENDCAP_SQUARE;
            break;
    }
    switch(pGraphState->m_LineJoin) {
        case 0:
            PenStyle |= PS_JOIN_MITER;
            break;
        case 1:
            PenStyle |= PS_JOIN_ROUND;
            break;
        case 2:
            PenStyle |= PS_JOIN_BEVEL;
            break;
    }
    int a;
    FX_COLORREF rgb;
    ArgbDecode(argb, a, rgb);
    LOGBRUSH lb;
    lb.lbColor = rgb;
    lb.lbStyle = BS_SOLID;
    lb.lbHatch = 0;
    FX_DWORD* pDash = NULL;
    if (pGraphState->m_DashCount) {
        pDash = FX_Alloc(FX_DWORD, pGraphState->m_DashCount);
        for (int i = 0; i < pGraphState->m_DashCount; i ++) {
            pDash[i] = FXSYS_round(pMatrix ? pMatrix->TransformDistance(pGraphState->m_DashArray[i]) : pGraphState->m_DashArray[i]);
            if (pDash[i] < 1) {
                pDash[i] = 1;
            }
        }
    }
    HPEN hPen = ExtCreatePen(PenStyle, (DWORD)FXSYS_ceil(width), &lb, pGraphState->m_DashCount, (const DWORD*)pDash);
    if (pDash) {
        FX_Free(pDash);
    }
    return hPen;
}
static HBRUSH _CreateBrush(FX_DWORD argb)
{
    int a;
    FX_COLORREF rgb;
    ArgbDecode(argb, a, rgb);
    return CreateSolidBrush(rgb);
}
static void _SetPathToDC(HDC hDC, const CFX_PathData* pPathData, const CFX_AffineMatrix* pMatrix)
{
    BeginPath(hDC);
    int nPoints = pPathData->GetPointCount();
    FX_PATHPOINT* pPoints = pPathData->GetPoints();
    for(int i = 0; i < nPoints; i++) {
        FX_FLOAT posx = pPoints[i].m_PointX, posy = pPoints[i].m_PointY;
        if (pMatrix) {
            pMatrix->Transform(posx, posy);
        }
        int screen_x = FXSYS_round(posx), screen_y = FXSYS_round(posy);
        int point_type = pPoints[i].m_Flag & FXPT_TYPE;
        if(point_type == PT_MOVETO) {
            MoveToEx(hDC, screen_x, screen_y, NULL);
        } else if(point_type == PT_LINETO) {
            if (pPoints[i].m_PointY == pPoints[i - 1].m_PointY && pPoints[i].m_PointX == pPoints[i - 1].m_PointX) {
                screen_x ++;
            }
            LineTo(hDC, screen_x, screen_y);
        } else if(point_type == PT_BEZIERTO) {
            POINT lppt[3];
            lppt[0].x = screen_x;
            lppt[0].y = screen_y;
            posx = pPoints[i + 1].m_PointX;
            posy = pPoints[i + 1].m_PointY;
            if (pMatrix) {
                pMatrix->Transform(posx, posy);
            }
            lppt[1].x = FXSYS_round(posx);
            lppt[1].y = FXSYS_round(posy);
            posx = pPoints[i + 2].m_PointX;
            posy = pPoints[i + 2].m_PointY;
            if (pMatrix) {
                pMatrix->Transform(posx, posy);
            }
            lppt[2].x = FXSYS_round(posx);
            lppt[2].y = FXSYS_round(posy);
            PolyBezierTo(hDC, lppt, 3);
            i += 2;
        }
        if (pPoints[i].m_Flag & PT_CLOSEFIGURE) {
            CloseFigure(hDC);
        }
    }
    EndPath(hDC);
}
void CGdiDeviceDriver::DrawLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2)
{
    int flag1 = (x1 < 0) | ((x1 > m_Width) << 1) | ((y1 < 0) << 2) | ((y1 > m_Height) << 3);
    int flag2 = (x2 < 0) | ((x2 > m_Width) << 1) | ((y2 < 0) << 2) | ((y2 > m_Height) << 3);
    if (flag1 & flag2) {
        return;
    }
    if (flag1 || flag2) {
        agg::rect_base<FX_FLOAT> rect(0.0f, 0.0f, (FX_FLOAT)(m_Width), (FX_FLOAT)(m_Height));
        FX_FLOAT x[2], y[2];
        int np = agg::clip_liang_barsky<FX_FLOAT>(x1, y1, x2, y2, rect, x, y);
        if (np == 0) {
            return;
        }
        if (np == 1) {
            x2 = x[0];
            y2 = y[0];
        } else {
            x1 = x[0];
            y1 = y[0];
            x2 = x[np - 1];
            y2 = y[np - 1];
        }
    }
    MoveToEx(m_hDC, FXSYS_round(x1), FXSYS_round(y1), NULL);
    LineTo(m_hDC, FXSYS_round(x2), FXSYS_round(y2));
}
static FX_BOOL _MatrixNoScaled(const CFX_AffineMatrix* pMatrix)
{
    return pMatrix->GetA() == 1.0f && pMatrix->GetB() == 0 && pMatrix->GetC() == 0 && pMatrix->GetD() == 1.0f;
}
FX_BOOL CGdiDeviceDriver::DrawPath(const CFX_PathData* pPathData,
                                   const CFX_AffineMatrix* pMatrix,
                                   const CFX_GraphStateData* pGraphState,
                                   FX_DWORD fill_color,
                                   FX_DWORD stroke_color,
                                   int fill_mode,
                                   int alpha_flag,
                                   void* pIccTransform,
                                   int	blend_type
                                  )
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    _Color2Argb(fill_color, fill_color, alpha_flag | (1 << 24), pIccTransform);
    _Color2Argb(stroke_color, stroke_color, alpha_flag, pIccTransform);
    CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
    if ((pGraphState == NULL || stroke_color == 0) && !pPlatform->m_GdiplusExt.IsAvailable()) {
        CFX_FloatRect bbox_f = pPathData->GetBoundingBox();
        if (pMatrix) {
            bbox_f.Transform(pMatrix);
        }
        FX_RECT bbox = bbox_f.GetInnerRect();
        if (bbox.Width() <= 0) {
            return DrawCosmeticLine((FX_FLOAT)(bbox.left), (FX_FLOAT)(bbox.top), (FX_FLOAT)(bbox.left), (FX_FLOAT)(bbox.bottom + 1), fill_color,
                                    alpha_flag, pIccTransform, FXDIB_BLEND_NORMAL);
        } else if (bbox.Height() <= 0) {
            return DrawCosmeticLine((FX_FLOAT)(bbox.left), (FX_FLOAT)(bbox.top), (FX_FLOAT)(bbox.right + 1), (FX_FLOAT)(bbox.top), fill_color,
                                    alpha_flag, pIccTransform, FXDIB_BLEND_NORMAL);
        }
    }
    int fill_alpha = FXARGB_A(fill_color);
    int stroke_alpha = FXARGB_A(stroke_color);
    FX_BOOL bDrawAlpha = (fill_alpha > 0 && fill_alpha < 255) || (stroke_alpha > 0 && stroke_alpha < 255 && pGraphState);
    if (!pPlatform->m_GdiplusExt.IsAvailable() && bDrawAlpha) {
        return FALSE;
    }
    if (pPlatform->m_GdiplusExt.IsAvailable()) {
        if (bDrawAlpha || ((m_DeviceClass != FXDC_PRINTER && !(fill_mode & FXFILL_FULLCOVER)) || pGraphState && pGraphState->m_DashCount)) {
            if ( !((NULL == pMatrix || _MatrixNoScaled(pMatrix)) &&
                    pGraphState && pGraphState->m_LineWidth == 1.f &&
                    (pPathData->GetPointCount() == 5 || pPathData->GetPointCount() == 4) &&
                    pPathData->IsRect()) ) {
                if (pPlatform->m_GdiplusExt.DrawPath(m_hDC, pPathData, pMatrix, pGraphState, fill_color, stroke_color, fill_mode)) {
                    return TRUE;
                }
            }
        }
    }
    int old_fill_mode = fill_mode;
    fill_mode &= 3;
    HPEN hPen = NULL;
    HBRUSH hBrush = NULL;
    if (pGraphState && stroke_alpha) {
        SetMiterLimit(m_hDC, pGraphState->m_MiterLimit, NULL);
        hPen = _CreatePen(pGraphState, pMatrix, stroke_color);
        hPen = (HPEN)SelectObject(m_hDC, hPen);
    }
    if (fill_mode && fill_alpha) {
        SetPolyFillMode(m_hDC, fill_mode);
        hBrush = _CreateBrush(fill_color);
        hBrush = (HBRUSH)SelectObject(m_hDC, hBrush);
    }
    if (pPathData->GetPointCount() == 2 && pGraphState && pGraphState->m_DashCount) {
        FX_FLOAT x1 = pPathData->GetPointX(0), y1 = pPathData->GetPointY(0);
        if (pMatrix) {
            pMatrix->Transform(x1, y1);
        }
        FX_FLOAT x2 = pPathData->GetPointX(1), y2 = pPathData->GetPointY(1);
        if (pMatrix) {
            pMatrix->Transform(x2, y2);
        }
        DrawLine(x1, y1, x2, y2);
    } else {
        _SetPathToDC(m_hDC, pPathData, pMatrix);
        if (pGraphState && stroke_alpha) {
            if (fill_mode && fill_alpha) {
                if (old_fill_mode & FX_FILL_TEXT_MODE) {
                    StrokeAndFillPath(m_hDC);
                } else {
                    FillPath(m_hDC);
                    _SetPathToDC(m_hDC, pPathData, pMatrix);
                    StrokePath(m_hDC);
                }
            } else {
                StrokePath(m_hDC);
            }
        } else if (fill_mode && fill_alpha) {
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
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::FillRect(const FX_RECT* pRect, FX_DWORD fill_color, int alpha_flag, void* pIccTransform, int blend_type)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    _Color2Argb(fill_color, fill_color, alpha_flag | (1 << 24), pIccTransform);
    int alpha;
    FX_COLORREF rgb;
    ArgbDecode(fill_color, alpha, rgb);
    if (alpha == 0) {
        return TRUE;
    }
    if (alpha < 255) {
        return FALSE;
    }
    HBRUSH hBrush = CreateSolidBrush(rgb);
    ::FillRect(m_hDC, (RECT*)pRect, hBrush);
    DeleteObject(hBrush);
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::SetClip_PathFill(const CFX_PathData* pPathData,
        const CFX_AffineMatrix* pMatrix,
        int fill_mode
                                          )
{
    if (pPathData->GetPointCount() == 5) {
        CFX_FloatRect rectf;
        if (pPathData->IsRect(pMatrix, &rectf)) {
            FX_RECT rect = rectf.GetOutterRect();
            IntersectClipRect(m_hDC, rect.left, rect.top, rect.right, rect.bottom);
            return TRUE;
        }
    }
    _SetPathToDC(m_hDC, pPathData, pMatrix);
    SetPolyFillMode(m_hDC, fill_mode & 3);
    SelectClipPath(m_hDC, RGN_AND);
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::SetClip_PathStroke(const CFX_PathData* pPathData,
        const CFX_AffineMatrix* pMatrix,
        const CFX_GraphStateData* pGraphState
                                            )
{
    HPEN hPen = _CreatePen(pGraphState, pMatrix, 0xff000000);
    hPen = (HPEN)SelectObject(m_hDC, hPen);
    _SetPathToDC(m_hDC, pPathData, pMatrix);
    WidenPath(m_hDC);
    SetPolyFillMode(m_hDC, WINDING);
    FX_BOOL ret = SelectClipPath(m_hDC, RGN_AND);
    hPen = (HPEN)SelectObject(m_hDC, hPen);
    DeleteObject(hPen);
    return ret;
}
FX_BOOL CGdiDeviceDriver::DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
        int alpha_flag, void* pIccTransform, int	blend_type)
{
    if (blend_type != FXDIB_BLEND_NORMAL) {
        return FALSE;
    }
    _Color2Argb(color, color, alpha_flag | (1 << 24), pIccTransform);
    int a;
    FX_COLORREF rgb;
    ArgbDecode(color, a, rgb);
    if (a == 0) {
        return TRUE;
    }
    HPEN hPen = CreatePen(PS_SOLID, 1, rgb);
    hPen = (HPEN)SelectObject(m_hDC, hPen);
    MoveToEx(m_hDC, FXSYS_round(x1), FXSYS_round(y1), NULL);
    LineTo(m_hDC, FXSYS_round(x2), FXSYS_round(y2));
    hPen = (HPEN)SelectObject(m_hDC, hPen);
    DeleteObject(hPen);
    return TRUE;
}
FX_BOOL CGdiDeviceDriver::DeleteDeviceRgn(FX_LPVOID pRgn)
{
    DeleteObject((HGDIOBJ)pRgn);
    return TRUE;
}
CGdiDisplayDriver::CGdiDisplayDriver(HDC hDC) : CGdiDeviceDriver(hDC, FXDC_DISPLAY)
{
    CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
    if (pPlatform->m_GdiplusExt.IsAvailable()) {
        m_RenderCaps |= FXRC_ALPHA_PATH | FXRC_ALPHA_IMAGE;
    }
}
FX_BOOL CGdiDisplayDriver::GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform, FX_BOOL bDEdge)
{
    FX_BOOL ret = FALSE;
    int width = pBitmap->GetWidth();
    int height = pBitmap->GetHeight();
    HBITMAP hbmp = CreateCompatibleBitmap(m_hDC, width, height);
    HDC hDCMemory = CreateCompatibleDC(m_hDC);
    HBITMAP holdbmp  = (HBITMAP)SelectObject(hDCMemory, hbmp);
    BitBlt(hDCMemory, 0, 0, width, height, m_hDC, left, top, SRCCOPY);
    SelectObject(hDCMemory, holdbmp);
    BITMAPINFO bmi;
    FXSYS_memset32(&bmi, 0, sizeof bmi);
    bmi.bmiHeader.biSize = sizeof bmi.bmiHeader;
    bmi.bmiHeader.biBitCount = pBitmap->GetBPP();
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biWidth = width;
    if (!CFX_GEModule::Get()->GetCodecModule() || !CFX_GEModule::Get()->GetCodecModule()->GetIccModule()) {
        pIccTransform = NULL;
    }
    if (pBitmap->GetBPP() > 8 && !pBitmap->IsCmykImage() && pIccTransform == NULL) {
        ret = ::GetDIBits(hDCMemory, hbmp, 0, height, pBitmap->GetBuffer(), &bmi, DIB_RGB_COLORS) == height;
    } else {
        CFX_DIBitmap bitmap;
        if (bitmap.Create(width, height, FXDIB_Rgb)) {
            bmi.bmiHeader.biBitCount = 24;
            ::GetDIBits(hDCMemory, hbmp, 0, height, bitmap.GetBuffer(), &bmi, DIB_RGB_COLORS);
            ret = pBitmap->TransferBitmap(0, 0, width, height, &bitmap, 0, 0, pIccTransform);
        } else {
            ret = FALSE;
        }
    }
    if (pBitmap->HasAlpha() && ret) {
        pBitmap->LoadChannel(FXDIB_Alpha, 0xff);
    }
    DeleteObject(hbmp);
    DeleteObject(hDCMemory);
    return ret;
}
FX_BOOL CGdiDisplayDriver::SetDIBits(const CFX_DIBSource* pSource, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                                     int alpha_flag, void* pIccTransform)
{
    ASSERT(blend_type == FXDIB_BLEND_NORMAL);
    if (pSource->IsAlphaMask()) {
        int width = pSource->GetWidth(), height = pSource->GetHeight();
        int alpha = FXGETFLAG_COLORTYPE(alpha_flag) ? FXGETFLAG_ALPHA_FILL(alpha_flag) : FXARGB_A(color);
        FX_BOOL bGDI = pSource->GetBPP() == 1 && alpha == 255;
        if (!bGDI) {
            CFX_DIBitmap background;
            if (!background.Create(width, height, FXDIB_Rgb32) ||
                    !GetDIBits(&background, left, top, NULL) ||
                    !background.CompositeMask(0, 0, width, height, pSource, color, 0, 0, FXDIB_BLEND_NORMAL, NULL, FALSE, alpha_flag, pIccTransform)) {
                return FALSE;
            }
            FX_RECT src_rect(0, 0, width, height);
            return SetDIBits(&background, 0, &src_rect, left, top, FXDIB_BLEND_NORMAL, 0, NULL);
        }
        FX_RECT clip_rect(left, top, left + pSrcRect->Width(), top + pSrcRect->Height());
        return StretchDIBits(pSource, color, left - pSrcRect->left, top - pSrcRect->top, width, height,
                             &clip_rect, 0, alpha_flag, pIccTransform, FXDIB_BLEND_NORMAL);
    } else {
        int width = pSrcRect->Width(), height = pSrcRect->Height();
        if (pSource->HasAlpha()) {
            CFX_DIBitmap bitmap;
            if (!bitmap.Create(width, height, FXDIB_Rgb) ||
                    !GetDIBits(&bitmap, left, top, NULL) ||
                    !bitmap.CompositeBitmap(0, 0, width, height, pSource, pSrcRect->left, pSrcRect->top, FXDIB_BLEND_NORMAL, NULL, FALSE, pIccTransform)) {
                return FALSE;
            }
            FX_RECT src_rect(0, 0, width, height);
            return SetDIBits(&bitmap, 0, &src_rect, left, top, FXDIB_BLEND_NORMAL, 0, NULL);
        }
        CFX_DIBExtractor temp(pSource);
        CFX_DIBitmap* pBitmap = temp;
        if (pBitmap) {
            return GDI_SetDIBits(pBitmap, pSrcRect, left, top, pIccTransform);
        }
    }
    return FALSE;
}
FX_BOOL CGdiDisplayDriver::UseFoxitStretchEngine(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
        int dest_width, int dest_height, const FX_RECT* pClipRect, int render_flags,
        int alpha_flag, void* pIccTransform, int blend_type)
{
    FX_RECT bitmap_clip = *pClipRect;
    if (dest_width < 0) {
        dest_left += dest_width;
    }
    if (dest_height < 0) {
        dest_top += dest_height;
    }
    bitmap_clip.Offset(-dest_left, -dest_top);
    CFX_DIBitmap* pStretched = pSource->StretchTo(dest_width, dest_height, render_flags, &bitmap_clip);
    if (pStretched == NULL) {
        return TRUE;
    }
    FX_RECT src_rect(0, 0, pStretched->GetWidth(), pStretched->GetHeight());
    FX_BOOL ret = SetDIBits(pStretched, color, &src_rect, pClipRect->left, pClipRect->top, FXDIB_BLEND_NORMAL, alpha_flag, pIccTransform);
    delete pStretched;
    return ret;
}
FX_BOOL CGdiDisplayDriver::StretchDIBits(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
        int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
        int alpha_flag, void* pIccTransform, int blend_type)
{
    ASSERT(pSource != NULL && pClipRect != NULL);
    if (flags || dest_width > 10000 || dest_width < -10000 || dest_height > 10000 || dest_height < -10000)
        return UseFoxitStretchEngine(pSource, color, dest_left, dest_top, dest_width, dest_height,
                                     pClipRect, flags, alpha_flag, pIccTransform, blend_type);
    if (pSource->IsAlphaMask()) {
        FX_RECT image_rect;
        image_rect.left = dest_width > 0 ? dest_left : dest_left + dest_width;
        image_rect.right = dest_width > 0 ? dest_left + dest_width : dest_left;
        image_rect.top = dest_height > 0 ? dest_top : dest_top + dest_height;
        image_rect.bottom = dest_height > 0 ? dest_top + dest_height : dest_top;
        FX_RECT clip_rect = image_rect;
        clip_rect.Intersect(*pClipRect);
        clip_rect.Offset(-image_rect.left, -image_rect.top);
        int clip_width = clip_rect.Width(), clip_height = clip_rect.Height();
        CFX_DIBitmap* pStretched = pSource->StretchTo(dest_width, dest_height, flags, &clip_rect);
        if (pStretched == NULL) {
            return TRUE;
        }
        CFX_DIBitmap background;
        if (!background.Create(clip_width, clip_height, FXDIB_Rgb32) ||
                !GetDIBits(&background, image_rect.left + clip_rect.left, image_rect.top + clip_rect.top, NULL) ||
                !background.CompositeMask(0, 0, clip_width, clip_height, pStretched, color, 0, 0, FXDIB_BLEND_NORMAL, NULL, FALSE, alpha_flag, pIccTransform)) {
            delete pStretched;
            return FALSE;
        }
        FX_RECT src_rect(0, 0, clip_width, clip_height);
        FX_BOOL ret = SetDIBits(&background, 0, &src_rect, image_rect.left + clip_rect.left, image_rect.top + clip_rect.top, FXDIB_BLEND_NORMAL, 0, NULL);
        delete pStretched;
        return ret;
    } else {
        if (pSource->HasAlpha()) {
            CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
            if (pPlatform->m_GdiplusExt.IsAvailable() && pIccTransform == NULL && !pSource->IsCmykImage()) {
                CFX_DIBExtractor temp(pSource);
                CFX_DIBitmap* pBitmap = temp;
                if (pBitmap == NULL) {
                    return FALSE;
                }
                return pPlatform->m_GdiplusExt.StretchDIBits(m_hDC, pBitmap, dest_left, dest_top, dest_width, dest_height, pClipRect, flags);
            }
            return UseFoxitStretchEngine(pSource, color, dest_left, dest_top, dest_width, dest_height,
                                         pClipRect, flags, alpha_flag, pIccTransform, blend_type);
        }
        CFX_DIBExtractor temp(pSource);
        CFX_DIBitmap* pBitmap = temp;
        if (pBitmap) {
            return GDI_StretchDIBits(pBitmap, dest_left, dest_top, dest_width, dest_height, flags, pIccTransform);
        }
    }
    return FALSE;
}
#define GET_PS_FEATURESETTING        4121
#define FEATURESETTING_PSLEVEL       2
int GetPSLevel(HDC hDC)
{
    int device_type = ::GetDeviceCaps(hDC, TECHNOLOGY);
    if (device_type != DT_RASPRINTER) {
        return 0;
    }
    FX_DWORD esc = GET_PS_FEATURESETTING;
    if (ExtEscape(hDC, QUERYESCSUPPORT, sizeof esc, (char*)&esc, 0, NULL)) {
        int param = FEATURESETTING_PSLEVEL;
        if (ExtEscape(hDC, GET_PS_FEATURESETTING, sizeof(int), (char*)&param, sizeof(int), (char*)&param) > 0) {
            return param;
        }
    }
    esc = POSTSCRIPT_IDENTIFY;
    if (ExtEscape(hDC, QUERYESCSUPPORT, sizeof esc, (char*)&esc, 0, NULL) == 0) {
        esc = POSTSCRIPT_DATA;
        if (ExtEscape(hDC, QUERYESCSUPPORT, sizeof esc, (char*)&esc, 0, NULL)) {
            return 2;
        }
        return 0;
    }
    esc = PSIDENT_GDICENTRIC;
    if (ExtEscape(hDC, POSTSCRIPT_IDENTIFY, sizeof(FX_DWORD), (char*)&esc, 0, NULL) <= 0) {
        return 2;
    }
    esc = GET_PS_FEATURESETTING;
    if (ExtEscape(hDC, QUERYESCSUPPORT, sizeof esc, (char*)&esc, 0, NULL)) {
        int param = FEATURESETTING_PSLEVEL;
        if (ExtEscape(hDC, GET_PS_FEATURESETTING, sizeof(int), (char*)&param, sizeof(int), (char*)&param) > 0) {
            return param;
        }
    }
    return 2;
}
int CFX_WindowsDevice::m_psLevel = 2;
CFX_WindowsDevice::CFX_WindowsDevice(HDC hDC, FX_BOOL bCmykOutput, FX_BOOL bForcePSOutput, int psLevel)
{
    m_bForcePSOutput = bForcePSOutput;
    m_psLevel = psLevel;
    if (bForcePSOutput) {
        IFX_RenderDeviceDriver* pDriver = new CPSPrinterDriver;
        ((CPSPrinterDriver*)pDriver)->Init(hDC, psLevel, bCmykOutput);
        SetDeviceDriver(pDriver);
        return;
    }
    SetDeviceDriver(CreateDriver(hDC, bCmykOutput));
}
HDC CFX_WindowsDevice::GetDC() const
{
    IFX_RenderDeviceDriver *pRDD = GetDeviceDriver();
    if (!pRDD) {
        return NULL;
    }
    return (HDC)pRDD->GetPlatformSurface();
}
IFX_RenderDeviceDriver* CFX_WindowsDevice::CreateDriver(HDC hDC, FX_BOOL bCmykOutput)
{
    int device_type = ::GetDeviceCaps(hDC, TECHNOLOGY);
    int obj_type = ::GetObjectType(hDC);
    int device_class;
    if (device_type == DT_RASPRINTER || device_type == DT_PLOTTER || obj_type == OBJ_ENHMETADC) {
        device_class = FXDC_PRINTER;
    } else {
        device_class = FXDC_DISPLAY;
    }
    if (device_class == FXDC_PRINTER) {
        return new CGdiPrinterDriver(hDC);
    }
    return new CGdiDisplayDriver(hDC);
}
CFX_WinBitmapDevice::CFX_WinBitmapDevice(int width, int height, FXDIB_Format format)
{
    BITMAPINFOHEADER bmih;
    FXSYS_memset32(&bmih, 0, sizeof (BITMAPINFOHEADER));
    bmih.biSize = sizeof(BITMAPINFOHEADER);
    bmih.biBitCount = format & 0xff;
    bmih.biHeight = -height;
    bmih.biPlanes = 1;
    bmih.biWidth = width;
    FX_LPBYTE pBuffer;
    m_hBitmap = CreateDIBSection(NULL, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (FX_LPVOID*)&pBuffer, NULL, 0);
    if (m_hBitmap == NULL) {
        return;
    }
    CFX_DIBitmap* pBitmap = new CFX_DIBitmap;
    pBitmap->Create(width, height, format, pBuffer);
    SetBitmap(pBitmap);
    m_hDC = ::CreateCompatibleDC(NULL);
    m_hOldBitmap = (HBITMAP)SelectObject(m_hDC, m_hBitmap);
    IFX_RenderDeviceDriver* pDriver = new CGdiDisplayDriver(m_hDC);
    SetDeviceDriver(pDriver);
}
CFX_WinBitmapDevice::~CFX_WinBitmapDevice()
{
    if (m_hDC) {
        SelectObject(m_hDC, m_hOldBitmap);
        DeleteDC(m_hDC);
    }
    if (m_hBitmap) {
        DeleteObject(m_hBitmap);
    }
    delete GetBitmap();
}
#endif
