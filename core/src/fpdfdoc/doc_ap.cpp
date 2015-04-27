// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfdoc/fpdf_doc.h"
#include "../../include/fpdfdoc/fpdf_vt.h"
#include "pdf_vt.h"
#include "../../include/fpdfdoc/fpdf_ap.h"
FX_BOOL FPDF_GenerateAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict)
{
    if (!pAnnotDict || pAnnotDict->GetConstString("Subtype") != FX_BSTRC("Widget")) {
        return FALSE;
    }
    CFX_ByteString field_type = FPDF_GetFieldAttr(pAnnotDict, "FT")->GetString();
    FX_DWORD flags = FPDF_GetFieldAttr(pAnnotDict, "Ff")? FPDF_GetFieldAttr(pAnnotDict, "Ff")->GetInteger() : 0;
    if (field_type == "Tx") {
        return CPVT_GenerateAP::GenerateTextFieldAP(pDoc, pAnnotDict);
    } else if (field_type == "Ch") {
        if (flags & (1 << 17)) {
            return CPVT_GenerateAP::GenerateComboBoxAP(pDoc, pAnnotDict);
        } else {
            return CPVT_GenerateAP::GenerateListBoxAP(pDoc, pAnnotDict);
        }
    } else if (field_type == "Btn") {
        if (!(flags & (1 << 16))) {
            if (!pAnnotDict->KeyExist("AS")) {
                if (CPDF_Dictionary* pParentDict = pAnnotDict->GetDict("Parent")) {
                    if (pParentDict->KeyExist("AS")) {
                        pAnnotDict->SetAtString("AS", pParentDict->GetString("AS"));
                    }
                }
            }
        }
    }
    return FALSE;
}
class CPVT_FontMap : public IPVT_FontMap
{
public:
    CPVT_FontMap(CPDF_Document * pDoc, CPDF_Dictionary * pResDict, CPDF_Font * pDefFont,
                 const CFX_ByteString & sDefFontAlias);
    virtual ~CPVT_FontMap();
    CPDF_Font*						GetPDFFont(FX_INT32 nFontIndex);
    CFX_ByteString					GetPDFFontAlias(FX_INT32 nFontIndex);
    static void						GetAnnotSysPDFFont(CPDF_Document * pDoc, CPDF_Dictionary * pResDict,
            CPDF_Font * & pSysFont, CFX_ByteString & sSysFontAlias);
private:
    CPDF_Document*					m_pDocument;
    CPDF_Dictionary*				m_pResDict;
    CPDF_Font*						m_pDefFont;
    CFX_ByteString					m_sDefFontAlias;
    CPDF_Font*						m_pSysFont;
    CFX_ByteString					m_sSysFontAlias;
};
CPVT_FontMap::CPVT_FontMap(CPDF_Document * pDoc, CPDF_Dictionary * pResDict, CPDF_Font * pDefFont,
                           const CFX_ByteString & sDefFontAlias) :
    m_pDocument(pDoc),
    m_pResDict(pResDict),
    m_pDefFont(pDefFont),
    m_sDefFontAlias(sDefFontAlias),
    m_pSysFont(NULL),
    m_sSysFontAlias()
{
}
CPVT_FontMap::~CPVT_FontMap()
{
}
extern CPDF_Font*		AddNativeInterFormFont(CPDF_Dictionary*& pFormDict, CPDF_Document* pDocument, CFX_ByteString& csNameTag);
void CPVT_FontMap::GetAnnotSysPDFFont(CPDF_Document * pDoc, CPDF_Dictionary * pResDict,
                                      CPDF_Font * & pSysFont, CFX_ByteString & sSysFontAlias)
{
    if (pDoc && pResDict) {
        CFX_ByteString sFontAlias;
        CPDF_Dictionary* pFormDict = pDoc->GetRoot()->GetDict("AcroForm");
        if (CPDF_Font * pPDFFont = AddNativeInterFormFont(pFormDict, pDoc, sSysFontAlias)) {
            if (CPDF_Dictionary * pFontList = pResDict->GetDict("Font")) {
                if (!pFontList->KeyExist(sSysFontAlias)) {
                    pFontList->SetAtReference(sSysFontAlias, pDoc, pPDFFont->GetFontDict());
                }
            }
            pSysFont = pPDFFont;
        }
    }
}
CPDF_Font* CPVT_FontMap::GetPDFFont(FX_INT32 nFontIndex)
{
    switch (nFontIndex) {
        case 0:
            return m_pDefFont;
        case 1:
            if (!m_pSysFont) {
                GetAnnotSysPDFFont(m_pDocument, m_pResDict, m_pSysFont, m_sSysFontAlias);
            }
            return m_pSysFont;
    }
    return NULL;
}
CFX_ByteString CPVT_FontMap::GetPDFFontAlias(FX_INT32 nFontIndex)
{
    switch (nFontIndex) {
        case 0:
            return m_sDefFontAlias;
        case 1:
            if (!m_pSysFont) {
                GetAnnotSysPDFFont(m_pDocument, m_pResDict, m_pSysFont, m_sSysFontAlias);
            }
            return m_sSysFontAlias;
    }
    return "";
}
CPVT_Provider::CPVT_Provider(IPVT_FontMap * pFontMap) : m_pFontMap(pFontMap)
{
    ASSERT (m_pFontMap != NULL);
}
CPVT_Provider::~CPVT_Provider()
{
}
FX_INT32 CPVT_Provider::GetCharWidth(FX_INT32 nFontIndex, FX_WORD word, FX_INT32 nWordStyle)
{
    if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
        FX_DWORD charcode = pPDFFont->CharCodeFromUnicode(word);
        if (charcode != -1) {
            return pPDFFont->GetCharWidthF(charcode);
        }
    }
    return 0;
}
FX_INT32 CPVT_Provider::GetTypeAscent(FX_INT32 nFontIndex)
{
    if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
        return pPDFFont->GetTypeAscent();
    }
    return 0;
}
FX_INT32 CPVT_Provider::GetTypeDescent(FX_INT32 nFontIndex)
{
    if (CPDF_Font* pPDFFont = m_pFontMap->GetPDFFont(nFontIndex)) {
        return pPDFFont->GetTypeDescent();
    }
    return 0;
}
FX_INT32 CPVT_Provider::GetWordFontIndex(FX_WORD word, FX_INT32 charset, FX_INT32 nFontIndex)
{
    if (CPDF_Font* pDefFont = m_pFontMap->GetPDFFont(0)) {
        if (pDefFont->CharCodeFromUnicode(word) != -1) {
            return 0;
        }
    }
    if (CPDF_Font* pSysFont = m_pFontMap->GetPDFFont(1))
        if (pSysFont->CharCodeFromUnicode(word) != -1) {
            return 1;
        }
    return -1;
}
FX_BOOL CPVT_Provider::IsLatinWord(FX_WORD word)
{
    if ((word >= 0x61 && word <= 0x7A) || (word >= 0x41 && word <= 0x5A) || word == 0x2D || word == 0x27) {
        return TRUE;
    }
    return FALSE;
}
FX_INT32 CPVT_Provider::GetDefaultFontIndex()
{
    return 0;
}
static CFX_ByteString GetPDFWordString(IPVT_FontMap * pFontMap, FX_INT32 nFontIndex, FX_WORD Word, FX_WORD SubWord)
{
    CFX_ByteString sWord;
    if (SubWord > 0) {
        sWord.Format("%c", SubWord);
    } else {
        if (pFontMap) {
            if (CPDF_Font * pPDFFont = pFontMap->GetPDFFont(nFontIndex)) {
                if (pPDFFont->GetBaseFont().Compare("Symbol") == 0 || pPDFFont->GetBaseFont().Compare("ZapfDingbats") == 0) {
                    sWord.Format("%c", Word);
                } else {
                    FX_DWORD dwCharCode = pPDFFont->CharCodeFromUnicode(Word);
                    if (dwCharCode != -1) {
                        pPDFFont->AppendChar(sWord, dwCharCode);
                    }
                }
            }
        }
    }
    return sWord;
}
static CFX_ByteString GetWordRenderString(const CFX_ByteString & strWords)
{
    if (strWords.GetLength() > 0) {
        return PDF_EncodeString(strWords) + " Tj\n";
    }
    return "";
}
static CFX_ByteString GetFontSetString(IPVT_FontMap * pFontMap, FX_INT32 nFontIndex, FX_FLOAT fFontSize)
{
    CFX_ByteTextBuf sRet;
    if (pFontMap) {
        CFX_ByteString sFontAlias = pFontMap->GetPDFFontAlias(nFontIndex);
        if (sFontAlias.GetLength() > 0 && fFontSize > 0 ) {
            sRet << "/" << sFontAlias << " " << fFontSize << " Tf\n";
        }
    }
    return sRet.GetByteString();
}
static CPVT_Color ParseColor(const CFX_ByteString & str)
{
    CPDF_SimpleParser syntax(str);
    syntax.SetPos(0);
    if (syntax.FindTagParam("g", 1)) {
        return CPVT_Color(CT_GRAY, FX_atof(syntax.GetWord()));
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam("rg", 3)) {
        FX_FLOAT f1 = FX_atof(syntax.GetWord());
        FX_FLOAT f2 = FX_atof(syntax.GetWord());
        FX_FLOAT f3 = FX_atof(syntax.GetWord());
        return CPVT_Color(CT_RGB, f1, f2, f3);
    }
    syntax.SetPos(0);
    if (syntax.FindTagParam("k", 4)) {
        FX_FLOAT f1 = FX_atof(syntax.GetWord());
        FX_FLOAT f2 = FX_atof(syntax.GetWord());
        FX_FLOAT f3 = FX_atof(syntax.GetWord());
        FX_FLOAT f4 = FX_atof(syntax.GetWord());
        return CPVT_Color(CT_CMYK, f1, f2, f3, f4);
    }
    return CPVT_Color(CT_TRANSPARENT);
}
static CPVT_Color ParseColor(const CPDF_Array & array)
{
    CPVT_Color rt;
    switch (array.GetCount()) {
        case 1:
            rt = CPVT_Color(CT_GRAY, array.GetFloat(0));
            break;
        case 3:
            rt = CPVT_Color(CT_RGB, array.GetFloat(0), array.GetFloat(1), array.GetFloat(2));
            break;
        case 4:
            rt = CPVT_Color(CT_CMYK, array.GetFloat(0), array.GetFloat(1), array.GetFloat(2), array.GetFloat(3));
            break;
    }
    return rt;
}
static FX_BOOL GenerateWidgetAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict, const FX_INT32 & nWidgetType)
{
    CPDF_Dictionary* pFormDict = NULL;
    if (CPDF_Dictionary * pRootDict = pDoc->GetRoot()) {
        pFormDict = pRootDict->GetDict("AcroForm");
    }
    if (!pFormDict) {
        return FALSE;
    }
    CFX_ByteString DA;
    if (CPDF_Object* pDAObj = FPDF_GetFieldAttr(pAnnotDict, "DA")) {
        DA = pDAObj->GetString();
    }
    if (DA.IsEmpty()) {
        DA = pFormDict->GetString("DA");
    }
    if (DA.IsEmpty()) {
        return FALSE;
    }
    CPDF_SimpleParser syntax(DA);
    syntax.FindTagParam("Tf", 2);
    CFX_ByteString sFontName = syntax.GetWord();
    sFontName = PDF_NameDecode(sFontName);
    if (sFontName.IsEmpty()) {
        return FALSE;
    }
    FX_FLOAT fFontSize = FX_atof(syntax.GetWord());
    CPVT_Color crText = ParseColor(DA);
    FX_BOOL bUseFormRes = FALSE;
    CPDF_Dictionary * pFontDict = NULL;
    CPDF_Dictionary* pDRDict = pAnnotDict->GetDict(FX_BSTRC("DR"));
    if (pDRDict == NULL) {
        pDRDict = pFormDict->GetDict(FX_BSTRC("DR"));
        bUseFormRes = TRUE;
    }
    CPDF_Dictionary * pDRFontDict = NULL;
    if (pDRDict && (pDRFontDict = pDRDict->GetDict("Font"))) {
        pFontDict = pDRFontDict->GetDict(sFontName.Mid(1));
        if (!pFontDict && !bUseFormRes) {
            pDRDict = pFormDict->GetDict(FX_BSTRC("DR"));
            pDRFontDict = pDRDict->GetDict("Font");
            if (pDRFontDict) {
                pFontDict = pDRFontDict->GetDict(sFontName.Mid(1));
            }
        }
    }
    if (!pDRFontDict) {
        return FALSE;
    }
    if (!pFontDict) {
        pFontDict = CPDF_Dictionary::Create();
        if (pFontDict == NULL) {
            return FALSE;
        }
        pFontDict->SetAtName(FX_BSTRC("Type"), "Font");
        pFontDict->SetAtName(FX_BSTRC("Subtype"), "Type1");
        pFontDict->SetAtName(FX_BSTRC("BaseFont"), "Helvetica");
        pFontDict->SetAtName(FX_BSTRC("Encoding"), "WinAnsiEncoding");
        pDoc->AddIndirectObject(pFontDict);
        pDRFontDict->SetAtReference(sFontName.Mid(1), pDoc, pFontDict);
    }
    CPDF_Font* pDefFont = pDoc->LoadFont(pFontDict);
    if (!pDefFont) {
        return FALSE;
    }
    CPDF_Rect rcAnnot = pAnnotDict->GetRect("Rect");
    FX_INT32 nRotate = 0;
    if (CPDF_Dictionary * pMKDict = pAnnotDict->GetDict("MK")) {
        nRotate = pMKDict->GetInteger("R");
    }
    CPDF_Rect rcBBox;
    CPDF_Matrix matrix;
    switch (nRotate % 360) {
        case 0:
            rcBBox = CPDF_Rect(0, 0, rcAnnot.right - rcAnnot.left, rcAnnot.top - rcAnnot.bottom);
            break;
        case 90:
            matrix = CPDF_Matrix(0, 1, -1, 0, rcAnnot.right - rcAnnot.left, 0);
            rcBBox = CPDF_Rect(0, 0, rcAnnot.top - rcAnnot.bottom, rcAnnot.right - rcAnnot.left);
            break;
        case 180:
            matrix = CPDF_Matrix(-1, 0, 0, -1, rcAnnot.right - rcAnnot.left, rcAnnot.top - rcAnnot.bottom);
            rcBBox = CPDF_Rect(0, 0, rcAnnot.right - rcAnnot.left, rcAnnot.top - rcAnnot.bottom);
            break;
        case 270:
            matrix = CPDF_Matrix(0, -1, 1, 0, 0, rcAnnot.top - rcAnnot.bottom);
            rcBBox = CPDF_Rect(0, 0, rcAnnot.top - rcAnnot.bottom, rcAnnot.right - rcAnnot.left);
            break;
    }
    FX_INT32 nBorderStyle = PBS_SOLID;
    FX_FLOAT fBorderWidth = 1;
    CPVT_Dash dsBorder(3, 0, 0);
    CPVT_Color crLeftTop, crRightBottom;
    if (CPDF_Dictionary * pBSDict = pAnnotDict->GetDict("BS")) {
        if (pBSDict->KeyExist("W")) {
            fBorderWidth = pBSDict->GetNumber("W");
        }
        if (CPDF_Array * pArray = pBSDict->GetArray("D")) {
            dsBorder = CPVT_Dash(pArray->GetInteger(0), pArray->GetInteger(1), pArray->GetInteger(2));
        }
        switch (pBSDict->GetString("S").GetAt(0)) {
            case 'S':
                nBorderStyle = PBS_SOLID;
                break;
            case 'D':
                nBorderStyle = PBS_DASH;
                break;
            case 'B':
                nBorderStyle = PBS_BEVELED;
                fBorderWidth *= 2;
                crLeftTop = CPVT_Color(CT_GRAY, 1);
                crRightBottom = CPVT_Color(CT_GRAY, 0.5);
                break;
            case 'I':
                nBorderStyle = PBS_INSET;
                fBorderWidth *= 2;
                crLeftTop = CPVT_Color(CT_GRAY, 0.5);
                crRightBottom = CPVT_Color(CT_GRAY, 0.75);
                break;
            case 'U':
                nBorderStyle = PBS_UNDERLINED;
                break;
        }
    }
    CPVT_Color crBorder, crBG;
    if (CPDF_Dictionary * pMKDict = pAnnotDict->GetDict("MK")) {
        if (CPDF_Array * pArray = pMKDict->GetArray("BC")) {
            crBorder = ParseColor(*pArray);
        }
        if (CPDF_Array * pArray = pMKDict->GetArray("BG")) {
            crBG = ParseColor(*pArray);
        }
    }
    CFX_ByteTextBuf sAppStream;
    CFX_ByteString sBG = CPVT_GenerateAP::GenerateColorAP(crBG, TRUE);
    if (sBG.GetLength() > 0) {
        sAppStream << "q\n" << sBG << rcBBox.left << " " << rcBBox.bottom << " "
                   << rcBBox.Width() << " " << rcBBox.Height() << " re f\n" << "Q\n";
    }
    CFX_ByteString sBorderStream = CPVT_GenerateAP::GenerateBorderAP(rcBBox, fBorderWidth,
                                   crBorder, crLeftTop, crRightBottom, nBorderStyle, dsBorder);
    if (sBorderStream.GetLength() > 0) {
        sAppStream << "q\n" << sBorderStream << "Q\n";
    }
    CPDF_Rect rcBody = CPDF_Rect(rcBBox.left + fBorderWidth, rcBBox.bottom + fBorderWidth,
                                 rcBBox.right - fBorderWidth, rcBBox.top - fBorderWidth);
    rcBody.Normalize();
    CPDF_Dictionary* pAPDict = pAnnotDict->GetDict("AP");
    if (pAPDict == NULL) {
        pAPDict = CPDF_Dictionary::Create();
        if (pAPDict == NULL) {
            return FALSE;
        }
        pAnnotDict->SetAt("AP", pAPDict);
    }
    CPDF_Stream* pNormalStream = pAPDict->GetStream("N");
    if (pNormalStream == NULL) {
        pNormalStream = CPDF_Stream::Create(NULL, 0, NULL);
        if (pNormalStream == NULL) {
            return FALSE;
        }
        FX_INT32 objnum = pDoc->AddIndirectObject(pNormalStream);
        pAnnotDict->GetDict("AP")->SetAtReference("N", pDoc, objnum);
    }
    CPDF_Dictionary * pStreamDict = pNormalStream->GetDict();
    if (pStreamDict) {
        pStreamDict->SetAtMatrix("Matrix", matrix);
        pStreamDict->SetAtRect("BBox", rcBBox);
        CPDF_Dictionary* pStreamResList = pStreamDict->GetDict("Resources");
        if (pStreamResList) {
            CPDF_Dictionary* pStreamResFontList = pStreamResList->GetDict("Font");
            if (!pStreamResFontList) {
                pStreamResFontList = CPDF_Dictionary::Create();
                if (pStreamResFontList == NULL) {
                    return FALSE;
                }
                pStreamResList->SetAt("Font", pStreamResFontList);
            }
            if (!pStreamResFontList->KeyExist(sFontName)) {
                pStreamResFontList->SetAtReference(sFontName, pDoc, pFontDict);
            }
        } else {
            pStreamDict->SetAt("Resources", pFormDict->GetDict("DR")->Clone());
            pStreamResList = pStreamDict->GetDict("Resources");
        }
    }
    switch (nWidgetType) {
        case 0: {
                CFX_WideString swValue = FPDF_GetFieldAttr(pAnnotDict, "V")? FPDF_GetFieldAttr(pAnnotDict, "V")->GetUnicodeText() : CFX_WideString();
                FX_INT32 nAlign = FPDF_GetFieldAttr(pAnnotDict, "Q")? FPDF_GetFieldAttr(pAnnotDict, "Q")->GetInteger() : 0;
                FX_DWORD dwFlags = FPDF_GetFieldAttr(pAnnotDict, "Ff")? FPDF_GetFieldAttr(pAnnotDict, "Ff")->GetInteger() : 0;
                FX_DWORD dwMaxLen = FPDF_GetFieldAttr(pAnnotDict, "MaxLen") ? FPDF_GetFieldAttr(pAnnotDict, "MaxLen")->GetInteger() : 0;
                CPVT_FontMap map(pDoc, pStreamDict ? pStreamDict->GetDict("Resources") : NULL , pDefFont, sFontName.Right(sFontName.GetLength() - 1));
                CPVT_Provider prd(&map);
                CPDF_VariableText vt;
                vt.SetProvider(&prd);
                vt.SetPlateRect(rcBody);
                vt.SetAlignment(nAlign);
                if (IsFloatZero(fFontSize)) {
                    vt.SetAutoFontSize(TRUE);
                } else {
                    vt.SetFontSize(fFontSize);
                }
                FX_BOOL bMultiLine = (dwFlags >> 12) & 1;
                if (bMultiLine) {
                    vt.SetMultiLine(TRUE);
                    vt.SetAutoReturn(TRUE);
                }
                FX_WORD subWord = 0;
                if ((dwFlags >> 13) & 1) {
                    subWord = '*';
                    vt.SetPasswordChar(subWord);
                }
                FX_BOOL bCharArray = (dwFlags >> 24) & 1;
                if (bCharArray) {
                    vt.SetCharArray(dwMaxLen);
                } else {
                    vt.SetLimitChar(dwMaxLen);
                }
                vt.Initialize();
                vt.SetText(swValue.c_str());
                vt.RearrangeAll();
                CPDF_Rect rcContent = vt.GetContentRect();
                CPDF_Point ptOffset(0.0f, 0.0f);
                if (!bMultiLine) {
                    ptOffset = CPDF_Point(0.0f, (rcContent.Height() - rcBody.Height()) / 2.0f);
                }
                CFX_ByteString sBody = CPVT_GenerateAP::GenerateEditAP(&map, vt.GetIterator(), ptOffset, !bCharArray, subWord);
                if (sBody.GetLength() > 0) {
                    sAppStream << "/Tx BMC\n" << "q\n";
                    if (rcContent.Width() > rcBody.Width() ||
                            rcContent.Height() > rcBody.Height()) {
                        sAppStream << rcBody.left << " " << rcBody.bottom << " "
                                   << rcBody.Width() << " " << rcBody.Height() << " re\nW\nn\n";
                    }
                    sAppStream << "BT\n" << CPVT_GenerateAP::GenerateColorAP(crText, TRUE) << sBody << "ET\n" << "Q\nEMC\n";
                }
            }
            break;
        case 1: {
                CFX_WideString swValue = FPDF_GetFieldAttr(pAnnotDict, "V") ? FPDF_GetFieldAttr(pAnnotDict, "V")->GetUnicodeText() : CFX_WideString();
                CPVT_FontMap map(pDoc, pStreamDict ? pStreamDict->GetDict("Resources"):NULL, pDefFont, sFontName.Right(sFontName.GetLength() - 1));
                CPVT_Provider prd(&map);
                CPDF_VariableText vt;
                vt.SetProvider(&prd);
                CPDF_Rect rcButton = rcBody;
                rcButton.left = rcButton.right - 13;
                rcButton.Normalize();
                CPDF_Rect rcEdit = rcBody;
                rcEdit.right = rcButton.left;
                rcEdit.Normalize();
                vt.SetPlateRect(rcEdit);
                if (IsFloatZero(fFontSize)) {
                    vt.SetAutoFontSize(TRUE);
                } else {
                    vt.SetFontSize(fFontSize);
                }
                vt.Initialize();
                vt.SetText(swValue.c_str());
                vt.RearrangeAll();
                CPDF_Rect rcContent = vt.GetContentRect();
                CPDF_Point ptOffset = CPDF_Point(0.0f, (rcContent.Height() - rcEdit.Height()) / 2.0f);
                CFX_ByteString sEdit = CPVT_GenerateAP::GenerateEditAP(&map, vt.GetIterator(), ptOffset, TRUE, 0);
                if (sEdit.GetLength() > 0) {
                    sAppStream << "/Tx BMC\n" << "q\n";
                    sAppStream << rcEdit.left << " " << rcEdit.bottom << " "
                               << rcEdit.Width() << " " << rcEdit.Height() << " re\nW\nn\n";
                    sAppStream << "BT\n" << CPVT_GenerateAP::GenerateColorAP(crText, TRUE) << sEdit << "ET\n" << "Q\nEMC\n";
                }
                CFX_ByteString sButton = CPVT_GenerateAP::GenerateColorAP(CPVT_Color(CT_RGB, 220.0f / 255.0f, 220.0f / 255.0f, 220.0f / 255.0f), TRUE);
                if (sButton.GetLength() > 0 && !rcButton.IsEmpty()) {
                    sAppStream << "q\n" << sButton;
                    sAppStream << rcButton.left << " " << rcButton.bottom << " "
                               << rcButton.Width() << " " << rcButton.Height() << " re f\n";
                    sAppStream << "Q\n";
                    CFX_ByteString sButtonBorder = CPVT_GenerateAP::GenerateBorderAP(rcButton, 2, CPVT_Color(CT_GRAY, 0), CPVT_Color(CT_GRAY, 1), CPVT_Color(CT_GRAY, 0.5), PBS_BEVELED, CPVT_Dash(3, 0, 0));
                    if (sButtonBorder.GetLength() > 0) {
                        sAppStream << "q\n" << sButtonBorder << "Q\n";
                    }
                    CPDF_Point ptCenter = CPDF_Point((rcButton.left + rcButton.right) / 2, (rcButton.top + rcButton.bottom) / 2);
                    if (IsFloatBigger(rcButton.Width(), 6) && IsFloatBigger(rcButton.Height(), 6)) {
                        sAppStream << "q\n" << " 0 g\n";
                        sAppStream << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " m\n";
                        sAppStream << ptCenter.x + 3 << " " << ptCenter.y + 1.5f << " l\n";
                        sAppStream << ptCenter.x << " " << ptCenter.y - 1.5f << " l\n";
                        sAppStream << ptCenter.x - 3 << " " << ptCenter.y + 1.5f << " l f\n";
                        sAppStream << sButton << "Q\n";
                    }
                }
            }
            break;
        case 2: {
                CPVT_FontMap map(pDoc, pStreamDict ? pStreamDict->GetDict("Resources"):NULL, pDefFont, sFontName.Right(sFontName.GetLength() - 1));
                CPVT_Provider prd(&map);
                CPDF_Array * pOpts = FPDF_GetFieldAttr(pAnnotDict, "Opt") ? FPDF_GetFieldAttr(pAnnotDict, "Opt")->GetArray() : NULL;
                CPDF_Array * pSels = FPDF_GetFieldAttr(pAnnotDict, "I") ? FPDF_GetFieldAttr(pAnnotDict, "I")->GetArray() : NULL;
                FX_INT32 nTop = FPDF_GetFieldAttr(pAnnotDict, "TI") ? FPDF_GetFieldAttr(pAnnotDict, "TI")->GetInteger() : 0;
                CFX_ByteTextBuf sBody;
                if (pOpts) {
                    FX_FLOAT fy = rcBody.top;
                    for (FX_INT32 i = nTop, sz = pOpts->GetCount(); i < sz; i++) {
                        if (IsFloatSmaller(fy, rcBody.bottom)) {
                            break;
                        }
                        if (CPDF_Object* pOpt = pOpts->GetElementValue(i)) {
                            CFX_WideString swItem;
                            if (pOpt->GetType() == PDFOBJ_STRING) {
                                swItem = pOpt->GetUnicodeText();
                            } else if (pOpt->GetType() == PDFOBJ_ARRAY) {
                                swItem = ((CPDF_Array*)pOpt)->GetElementValue(1)->GetUnicodeText();
                            }
                            FX_BOOL bSelected = FALSE;
                            if (pSels) {
                                for (FX_DWORD s = 0, ssz = pSels->GetCount(); s < ssz; s++) {
                                    if (i == pSels->GetInteger(s)) {
                                        bSelected = TRUE;
                                        break;
                                    }
                                }
                            }
                            CPDF_VariableText vt;
                            vt.SetProvider(&prd);
                            vt.SetPlateRect(CPDF_Rect(rcBody.left, 0.0f, rcBody.right, 0.0f));
                            if (IsFloatZero(fFontSize)) {
                                vt.SetFontSize(12.0f);
                            } else {
                                vt.SetFontSize(fFontSize);
                            }
                            vt.Initialize();
                            vt.SetText(swItem.c_str());
                            vt.RearrangeAll();
                            FX_FLOAT fItemHeight = vt.GetContentRect().Height();
                            if (bSelected) {
                                CPDF_Rect rcItem = CPDF_Rect(rcBody.left, fy - fItemHeight, rcBody.right, fy);
                                sBody << "q\n" << CPVT_GenerateAP::GenerateColorAP(CPVT_Color(CT_RGB, 0, 51.0f / 255.0f, 113.0f / 255.0f), TRUE)
                                      << rcItem.left << " " << rcItem.bottom << " " << rcItem.Width() << " " << rcItem.Height() << " re f\n" << "Q\n";
                                sBody << "BT\n" << CPVT_GenerateAP::GenerateColorAP(CPVT_Color(CT_GRAY, 1), TRUE) << CPVT_GenerateAP::GenerateEditAP(&map, vt.GetIterator(), CPDF_Point(0.0f, fy), TRUE, 0) << "ET\n";
                            } else {
                                sBody << "BT\n" << CPVT_GenerateAP::GenerateColorAP(crText, TRUE) << CPVT_GenerateAP::GenerateEditAP(&map, vt.GetIterator(), CPDF_Point(0.0f, fy), TRUE, 0) << "ET\n";
                            }
                            fy -= fItemHeight;
                        }
                    }
                }
                if (sBody.GetSize() > 0) {
                    sAppStream << "/Tx BMC\n" << "q\n";
                    sAppStream << rcBody.left << " " << rcBody.bottom << " "
                               << rcBody.Width() << " " << rcBody.Height() << " re\nW\nn\n";
                    sAppStream << sBody.GetByteString() << "Q\nEMC\n";
                }
            }
            break;
    }
    if (pNormalStream) {
        pNormalStream->SetData((FX_BYTE*)sAppStream.GetBuffer(), sAppStream.GetSize(), FALSE, FALSE);
        pStreamDict = pNormalStream->GetDict();
        if (pStreamDict) {
            pStreamDict->SetAtMatrix("Matrix", matrix);
            pStreamDict->SetAtRect("BBox", rcBBox);
            CPDF_Dictionary* pStreamResList = pStreamDict->GetDict("Resources");
            if (pStreamResList) {
                CPDF_Dictionary* pStreamResFontList = pStreamResList->GetDict("Font");
                if (!pStreamResFontList) {
                    pStreamResFontList = CPDF_Dictionary::Create();
                    if (pStreamResFontList == NULL) {
                        return FALSE;
                    }
                    pStreamResList->SetAt("Font", pStreamResFontList);
                }
                if (!pStreamResFontList->KeyExist(sFontName)) {
                    pStreamResFontList->SetAtReference(sFontName, pDoc, pFontDict);
                }
            } else {
                pStreamDict->SetAt("Resources", pFormDict->GetDict("DR")->Clone());
                pStreamResList = pStreamDict->GetDict("Resources");
            }
        }
    }
    return TRUE;
}
FX_BOOL CPVT_GenerateAP::GenerateTextFieldAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict)
{
    return GenerateWidgetAP(pDoc, pAnnotDict, 0);
}
FX_BOOL CPVT_GenerateAP::GenerateComboBoxAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict)
{
    return GenerateWidgetAP(pDoc, pAnnotDict, 1);
}
FX_BOOL CPVT_GenerateAP::GenerateListBoxAP(CPDF_Document* pDoc, CPDF_Dictionary* pAnnotDict)
{
    return GenerateWidgetAP(pDoc, pAnnotDict, 2);
}
CFX_ByteString CPVT_GenerateAP::GenerateEditAP(IPVT_FontMap * pFontMap, IPDF_VariableText_Iterator* pIterator, const CPDF_Point & ptOffset, FX_BOOL bContinuous, FX_WORD SubWord, const CPVT_WordRange * pVisible)
{
    CFX_ByteTextBuf sEditStream, sLineStream, sWords;
    CPDF_Point ptOld(0.0f, 0.0f), ptNew(0.0f, 0.0f);
    FX_INT32 nCurFontIndex = -1;
    if (pIterator) {
        if (pVisible) {
            pIterator->SetAt(pVisible->BeginPos);
        } else {
            pIterator->SetAt(0);
        }
        CPVT_WordPlace oldplace;
        while (pIterator->NextWord()) {
            CPVT_WordPlace place = pIterator->GetAt();
            if (pVisible && place.WordCmp(pVisible->EndPos) > 0) {
                break;
            }
            if (bContinuous) {
                if (place.LineCmp(oldplace) != 0) {
                    if (sWords.GetSize() > 0) {
                        sLineStream << GetWordRenderString(sWords.GetByteString());
                        sEditStream << sLineStream;
                        sLineStream.Clear();
                        sWords.Clear();
                    }
                    CPVT_Word word;
                    if (pIterator->GetWord(word)) {
                        ptNew = CPDF_Point(word.ptWord.x + ptOffset.x, word.ptWord.y + ptOffset.y);
                    } else {
                        CPVT_Line line;
                        pIterator->GetLine(line);
                        ptNew = CPDF_Point(line.ptLine.x + ptOffset.x, line.ptLine.y + ptOffset.y);
                    }
                    if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
                        sLineStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y << " Td\n";
                        ptOld = ptNew;
                    }
                }
                CPVT_Word word;
                if (pIterator->GetWord(word)) {
                    if (word.nFontIndex != nCurFontIndex) {
                        if (sWords.GetSize() > 0) {
                            sLineStream << GetWordRenderString(sWords.GetByteString());
                            sWords.Clear();
                        }
                        sLineStream << GetFontSetString(pFontMap, word.nFontIndex, word.fFontSize);
                        nCurFontIndex = word.nFontIndex;
                    }
                    sWords << GetPDFWordString(pFontMap, nCurFontIndex, word.Word, SubWord);
                }
                oldplace = place;
            } else {
                CPVT_Word word;
                if (pIterator->GetWord(word)) {
                    ptNew = CPDF_Point(word.ptWord.x + ptOffset.x, word.ptWord.y + ptOffset.y);
                    if (ptNew.x != ptOld.x || ptNew.y != ptOld.y) {
                        sEditStream << ptNew.x - ptOld.x << " " << ptNew.y - ptOld.y << " Td\n";
                        ptOld = ptNew;
                    }
                    if (word.nFontIndex != nCurFontIndex) {
                        sEditStream << GetFontSetString(pFontMap, word.nFontIndex, word.fFontSize);
                        nCurFontIndex = word.nFontIndex;
                    }
                    sEditStream << GetWordRenderString(GetPDFWordString(pFontMap, nCurFontIndex, word.Word, SubWord));
                }
            }
        }
        if (sWords.GetSize() > 0) {
            sLineStream << GetWordRenderString(sWords.GetByteString());
            sEditStream << sLineStream;
            sWords.Clear();
        }
    }
    return sEditStream.GetByteString();
}
CFX_ByteString CPVT_GenerateAP::GenerateBorderAP(const CPDF_Rect & rect, FX_FLOAT fWidth,
        const CPVT_Color & color, const CPVT_Color & crLeftTop, const CPVT_Color & crRightBottom,
        FX_INT32 nStyle, const CPVT_Dash & dash)
{
    CFX_ByteTextBuf sAppStream;
    CFX_ByteString sColor;
    FX_FLOAT fLeft = rect.left;
    FX_FLOAT fRight = rect.right;
    FX_FLOAT fTop = rect.top;
    FX_FLOAT fBottom = rect.bottom;
    if (fWidth > 0.0f) {
        FX_FLOAT fHalfWidth = fWidth / 2.0f;
        switch (nStyle) {
            default:
            case PBS_SOLID:
                sColor = GenerateColorAP(color, TRUE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fLeft << " " << fBottom << " " << fRight - fLeft << " " << fTop - fBottom << " re\n";
                    sAppStream << fLeft + fWidth << " " << fBottom + fWidth << " "
                               << fRight - fLeft - fWidth * 2 << " " << fTop - fBottom - fWidth * 2 << " re\n";
                    sAppStream << "f*\n";
                }
                break;
            case PBS_DASH:
                sColor = GenerateColorAP(color, FALSE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fWidth << " w" << " [" << dash.nDash << " " << dash.nGap << "] " << dash.nPhase << " d\n";
                    sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2 << " m\n";
                    sAppStream << fLeft + fWidth / 2 << " " << fTop - fWidth / 2 << " l\n";
                    sAppStream << fRight - fWidth / 2 << " " << fTop - fWidth / 2 << " l\n";
                    sAppStream << fRight - fWidth / 2 << " " << fBottom + fWidth / 2 << " l\n";
                    sAppStream << fLeft + fWidth / 2 << " " << fBottom + fWidth / 2 << " l S\n";
                }
                break;
            case PBS_BEVELED:
            case PBS_INSET:
                sColor = GenerateColorAP(crLeftTop, TRUE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " m\n";
                    sAppStream << fLeft + fHalfWidth << " " << fTop - fHalfWidth << " l\n";
                    sAppStream << fRight - fHalfWidth << " " << fTop - fHalfWidth << " l\n";
                    sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l\n";
                    sAppStream << fLeft + fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l\n";
                    sAppStream << fLeft + fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l f\n";
                }
                sColor = GenerateColorAP(crRightBottom, TRUE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fRight - fHalfWidth << " " <<	fTop - fHalfWidth << " m\n";
                    sAppStream << fRight - fHalfWidth << " " <<	fBottom + fHalfWidth << " l\n";
                    sAppStream << fLeft + fHalfWidth << " " << 	fBottom + fHalfWidth << " l\n";
                    sAppStream << fLeft + fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l\n";
                    sAppStream << fRight - fHalfWidth * 2 << " " << fBottom + fHalfWidth * 2 << " l\n";
                    sAppStream << fRight - fHalfWidth * 2 << " " << fTop - fHalfWidth * 2 << " l f\n";
                }
                sColor = GenerateColorAP(color, TRUE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fLeft << " " << fBottom << " " <<	fRight - fLeft << " " << fTop - fBottom << " re\n";
                    sAppStream << fLeft + fHalfWidth << " " << fBottom + fHalfWidth << " "
                               << fRight - fLeft - fHalfWidth * 2 << " " << fTop - fBottom - fHalfWidth * 2 << " re f*\n";
                }
                break;
            case PBS_UNDERLINED:
                sColor = GenerateColorAP(color, FALSE);
                if (sColor.GetLength() > 0) {
                    sAppStream << sColor;
                    sAppStream << fWidth << " w\n";
                    sAppStream << fLeft << " " << fBottom + fWidth / 2 << " m\n";
                    sAppStream << fRight << " " << fBottom + fWidth / 2 << " l S\n";
                }
                break;
        }
    }
    return sAppStream.GetByteString();
}
CFX_ByteString CPVT_GenerateAP::GenerateColorAP(const CPVT_Color & color, const FX_BOOL & bFillOrStroke)
{
    CFX_ByteTextBuf sColorStream;
    switch (color.nColorType) {
        case CT_RGB:
            sColorStream << color.fColor1 << " " << color.fColor2 << " " << color.fColor3 << " "
                         << (bFillOrStroke ? "rg" : "RG") << "\n";
            break;
        case CT_GRAY:
            sColorStream << color.fColor1 << " " << (bFillOrStroke ? "g" : "G") << "\n";
            break;
        case CT_CMYK:
            sColorStream << color.fColor1 << " " << color.fColor2 << " " << color.fColor3 << " " << color.fColor4 << " "
                         << (bFillOrStroke ? "k" : "K") << "\n";
            break;
    }
    return sColorStream.GetByteString();
}
