// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "pageint.h"

#include "core/include/fdrm/fx_crypt.h"
#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_page.h"
#include "core/src/fpdfapi/fpdf_font/font_int.h"

class CPDF_PageModule : public IPDF_PageModule {
 public:
  CPDF_PageModule()
      : m_StockGrayCS(nullptr, PDFCS_DEVICEGRAY),
        m_StockRGBCS(nullptr, PDFCS_DEVICERGB),
        m_StockCMYKCS(nullptr, PDFCS_DEVICECMYK),
        m_StockPatternCS(nullptr) {}

 private:
  ~CPDF_PageModule() override {}

  CPDF_DocPageData* CreateDocData(CPDF_Document* pDoc) override {
    return new CPDF_DocPageData(pDoc);
  }

  void ReleaseDoc(CPDF_Document* pDoc) override;
  void ClearDoc(CPDF_Document* pDoc) override;

  CPDF_FontGlobals* GetFontGlobals() override { return &m_FontGlobals; }

  void ClearStockFont(CPDF_Document* pDoc) override {
    m_FontGlobals.Clear(pDoc);
  }

  CPDF_ColorSpace* GetStockCS(int family) override;
  void NotifyCJKAvailable() override;

  CPDF_FontGlobals m_FontGlobals;
  CPDF_DeviceCS m_StockGrayCS;
  CPDF_DeviceCS m_StockRGBCS;
  CPDF_DeviceCS m_StockCMYKCS;
  CPDF_PatternCS m_StockPatternCS;
};

CPDF_ColorSpace* CPDF_PageModule::GetStockCS(int family) {
  if (family == PDFCS_DEVICEGRAY) {
    return &m_StockGrayCS;
  }
  if (family == PDFCS_DEVICERGB) {
    return &m_StockRGBCS;
  }
  if (family == PDFCS_DEVICECMYK) {
    return &m_StockCMYKCS;
  }
  if (family == PDFCS_PATTERN) {
    return &m_StockPatternCS;
  }
  return NULL;
}

void CPDF_ModuleMgr::InitPageModule() {
  m_pPageModule.reset(new CPDF_PageModule);
}

void CPDF_PageModule::ReleaseDoc(CPDF_Document* pDoc) {
  delete pDoc->GetPageData();
}
void CPDF_PageModule::ClearDoc(CPDF_Document* pDoc) {
  pDoc->GetPageData()->Clear(FALSE);
}
void CPDF_PageModule::NotifyCJKAvailable() {
  m_FontGlobals.m_CMapManager.ReloadAll();
}

CPDF_Font* CPDF_Document::LoadFont(CPDF_Dictionary* pFontDict) {
  ASSERT(pFontDict);
  return GetValidatePageData()->GetFont(pFontDict, FALSE);
}

CPDF_StreamAcc* CPDF_Document::LoadFontFile(CPDF_Stream* pStream) {
  return GetValidatePageData()->GetFontFileStreamAcc(pStream);
}

CPDF_ColorSpace* _CSFromName(const CFX_ByteString& name);
CPDF_ColorSpace* CPDF_Document::LoadColorSpace(CPDF_Object* pCSObj,
                                               CPDF_Dictionary* pResources) {
  return GetValidatePageData()->GetColorSpace(pCSObj, pResources);
}
CPDF_Pattern* CPDF_Document::LoadPattern(CPDF_Object* pPatternObj,
                                         FX_BOOL bShading,
                                         const CFX_Matrix* matrix) {
  return GetValidatePageData()->GetPattern(pPatternObj, bShading, matrix);
}
CPDF_IccProfile* CPDF_Document::LoadIccProfile(CPDF_Stream* pStream) {
  return GetValidatePageData()->GetIccProfile(pStream);
}
CPDF_Image* CPDF_Document::LoadImageF(CPDF_Object* pObj) {
  if (!pObj) {
    return NULL;
  }
  FXSYS_assert(pObj->GetObjNum());
  return GetValidatePageData()->GetImage(pObj);
}
void CPDF_Document::RemoveColorSpaceFromPageData(CPDF_Object* pCSObj) {
  if (!pCSObj) {
    return;
  }
  GetPageData()->ReleaseColorSpace(pCSObj);
}
CPDF_DocPageData::CPDF_DocPageData(CPDF_Document* pPDFDoc)
    : m_pPDFDoc(pPDFDoc), m_bForceClear(FALSE) {}

CPDF_DocPageData::~CPDF_DocPageData() {
  Clear(FALSE);
  Clear(TRUE);

  for (auto& it : m_PatternMap)
    delete it.second;
  m_PatternMap.clear();

  for (auto& it : m_FontMap)
    delete it.second;
  m_FontMap.clear();

  for (auto& it : m_ColorSpaceMap)
    delete it.second;
  m_ColorSpaceMap.clear();
}

void CPDF_DocPageData::Clear(FX_BOOL bForceRelease) {
  m_bForceClear = bForceRelease;

  for (auto& it : m_PatternMap) {
    CPDF_CountedPattern* ptData = it.second;
    if (!ptData->get())
      continue;

    if (bForceRelease || ptData->use_count() < 2) {
      ptData->get()->SetForceClear(bForceRelease);
      ptData->clear();
    }
  }

  for (auto& it : m_FontMap) {
    CPDF_CountedFont* fontData = it.second;
    if (!fontData->get())
      continue;

    if (bForceRelease || fontData->use_count() < 2) {
      fontData->clear();
    }
  }

  for (auto& it : m_ColorSpaceMap) {
    CPDF_CountedColorSpace* csData = it.second;
    if (!csData->get())
      continue;

    if (bForceRelease || csData->use_count() < 2) {
      csData->get()->ReleaseCS();
      csData->reset(nullptr);
    }
  }

  for (auto it = m_IccProfileMap.begin(); it != m_IccProfileMap.end();) {
    auto curr_it = it++;
    CPDF_CountedIccProfile* ipData = curr_it->second;
    if (!ipData->get())
      continue;

    if (bForceRelease || ipData->use_count() < 2) {
      for (auto hash_it = m_HashProfileMap.begin();
           hash_it != m_HashProfileMap.end(); ++hash_it) {
        if (curr_it->first == hash_it->second) {
          m_HashProfileMap.erase(hash_it);
          break;
        }
      }
      delete ipData->get();
      delete ipData;
      m_IccProfileMap.erase(curr_it);
    }
  }

  for (auto it = m_FontFileMap.begin(); it != m_FontFileMap.end();) {
    auto curr_it = it++;
    CPDF_CountedStreamAcc* ftData = curr_it->second;
    if (!ftData->get())
      continue;

    if (bForceRelease || ftData->use_count() < 2) {
      delete ftData->get();
      delete ftData;
      m_FontFileMap.erase(curr_it);
    }
  }

  for (auto it = m_ImageMap.begin(); it != m_ImageMap.end();) {
    auto curr_it = it++;
    CPDF_CountedImage* imageData = curr_it->second;
    if (!imageData->get())
      continue;

    if (bForceRelease || imageData->use_count() < 2) {
      delete imageData->get();
      delete imageData;
      m_ImageMap.erase(curr_it);
    }
  }
}

CPDF_Font* CPDF_DocPageData::GetFont(CPDF_Dictionary* pFontDict,
                                     FX_BOOL findOnly) {
  if (!pFontDict) {
    return NULL;
  }
  if (findOnly) {
    auto it = m_FontMap.find(pFontDict);
    if (it != m_FontMap.end() && it->second->get()) {
      return it->second->AddRef();
    }
    return nullptr;
  }

  CPDF_CountedFont* fontData = nullptr;
  auto it = m_FontMap.find(pFontDict);
  if (it != m_FontMap.end()) {
    fontData = it->second;
    if (fontData->get()) {
      return fontData->AddRef();
    }
  }

  CPDF_Font* pFont = CPDF_Font::CreateFontF(m_pPDFDoc, pFontDict);
  if (!pFont) {
    return nullptr;
  }
  if (!fontData) {
    fontData = new CPDF_CountedFont(pFont);
    m_FontMap[pFontDict] = fontData;
  } else {
    fontData->reset(pFont);
  }
  return fontData->AddRef();
}

CPDF_Font* CPDF_DocPageData::GetStandardFont(const CFX_ByteStringC& fontName,
                                             CPDF_FontEncoding* pEncoding) {
  if (fontName.IsEmpty())
    return nullptr;

  for (auto& it : m_FontMap) {
    CPDF_CountedFont* fontData = it.second;
    CPDF_Font* pFont = fontData->get();
    if (!pFont)
      continue;
    if (pFont->GetBaseFont() != fontName)
      continue;
    if (pFont->IsEmbedded())
      continue;
    if (pFont->GetFontType() != PDFFONT_TYPE1)
      continue;
    if (pFont->GetFontDict()->KeyExist("Widths"))
      continue;

    CPDF_Type1Font* pT1Font = pFont->GetType1Font();
    if (pEncoding && !pT1Font->GetEncoding()->IsIdentical(pEncoding))
      continue;

    return fontData->AddRef();
  }

  CPDF_Dictionary* pDict = new CPDF_Dictionary;
  pDict->SetAtName("Type", "Font");
  pDict->SetAtName("Subtype", "Type1");
  pDict->SetAtName("BaseFont", fontName);
  if (pEncoding) {
    pDict->SetAt("Encoding", pEncoding->Realize());
  }
  m_pPDFDoc->AddIndirectObject(pDict);
  CPDF_Font* pFont = CPDF_Font::CreateFontF(m_pPDFDoc, pDict);
  if (!pFont) {
    return nullptr;
  }
  CPDF_CountedFont* fontData = new CPDF_CountedFont(pFont);
  m_FontMap[pDict] = fontData;
  return fontData->AddRef();
}

void CPDF_DocPageData::ReleaseFont(CPDF_Dictionary* pFontDict) {
  if (!pFontDict)
    return;

  auto it = m_FontMap.find(pFontDict);
  if (it == m_FontMap.end())
    return;

  CPDF_CountedFont* fontData = it->second;
  if (fontData->get()) {
    fontData->RemoveRef();
    if (fontData->use_count() == 0) {
      fontData->clear();
    }
  }
}

CPDF_ColorSpace* CPDF_DocPageData::GetColorSpace(
    CPDF_Object* pCSObj,
    const CPDF_Dictionary* pResources) {
  if (!pCSObj)
    return nullptr;

  if (pCSObj->IsName()) {
    CFX_ByteString name = pCSObj->GetConstString();
    CPDF_ColorSpace* pCS = _CSFromName(name);
    if (!pCS && pResources) {
      CPDF_Dictionary* pList = pResources->GetDict("ColorSpace");
      if (pList) {
        pCSObj = pList->GetElementValue(name);
        return GetColorSpace(pCSObj, nullptr);
      }
    }
    if (!pCS || !pResources)
      return pCS;

    CPDF_Dictionary* pColorSpaces = pResources->GetDict("ColorSpace");
    if (!pColorSpaces)
      return pCS;

    CPDF_Object* pDefaultCS = nullptr;
    switch (pCS->GetFamily()) {
      case PDFCS_DEVICERGB:
        pDefaultCS = pColorSpaces->GetElementValue("DefaultRGB");
        break;
      case PDFCS_DEVICEGRAY:
        pDefaultCS = pColorSpaces->GetElementValue("DefaultGray");
        break;
      case PDFCS_DEVICECMYK:
        pDefaultCS = pColorSpaces->GetElementValue("DefaultCMYK");
        break;
    }
    return pDefaultCS ? GetColorSpace(pDefaultCS, nullptr) : pCS;
  }

  CPDF_Array* pArray = pCSObj->AsArray();
  if (!pArray || pArray->GetCount() == 0)
    return nullptr;
  if (pArray->GetCount() == 1)
    return GetColorSpace(pArray->GetElementValue(0), pResources);

  CPDF_CountedColorSpace* csData = nullptr;
  auto it = m_ColorSpaceMap.find(pCSObj);
  if (it != m_ColorSpaceMap.end()) {
    csData = it->second;
    if (csData->get()) {
      return csData->AddRef();
    }
  }

  CPDF_ColorSpace* pCS = CPDF_ColorSpace::Load(m_pPDFDoc, pArray);
  if (!pCS)
    return nullptr;

  if (!csData) {
    csData = new CPDF_CountedColorSpace(pCS);
    m_ColorSpaceMap[pCSObj] = csData;
  } else {
    csData->reset(pCS);
  }
  return csData->AddRef();
}

CPDF_ColorSpace* CPDF_DocPageData::GetCopiedColorSpace(CPDF_Object* pCSObj) {
  if (!pCSObj)
    return nullptr;

  auto it = m_ColorSpaceMap.find(pCSObj);
  if (it != m_ColorSpaceMap.end())
    return it->second->AddRef();

  return nullptr;
}

void CPDF_DocPageData::ReleaseColorSpace(CPDF_Object* pColorSpace) {
  if (!pColorSpace)
    return;

  auto it = m_ColorSpaceMap.find(pColorSpace);
  if (it == m_ColorSpaceMap.end())
    return;

  CPDF_CountedColorSpace* csData = it->second;
  if (csData->get()) {
    csData->RemoveRef();
    if (csData->use_count() == 0) {
      csData->get()->ReleaseCS();
      csData->reset(nullptr);
    }
  }
}

CPDF_Pattern* CPDF_DocPageData::GetPattern(CPDF_Object* pPatternObj,
                                           FX_BOOL bShading,
                                           const CFX_Matrix* matrix) {
  if (!pPatternObj)
    return nullptr;

  CPDF_CountedPattern* ptData = nullptr;
  auto it = m_PatternMap.find(pPatternObj);
  if (it != m_PatternMap.end()) {
    ptData = it->second;
    if (ptData->get()) {
      return ptData->AddRef();
    }
  }
  CPDF_Pattern* pPattern = nullptr;
  if (bShading) {
    pPattern =
        new CPDF_ShadingPattern(m_pPDFDoc, pPatternObj, bShading, matrix);
  } else {
    CPDF_Dictionary* pDict = pPatternObj ? pPatternObj->GetDict() : nullptr;
    if (pDict) {
      int type = pDict->GetInteger("PatternType");
      if (type == 1) {
        pPattern = new CPDF_TilingPattern(m_pPDFDoc, pPatternObj, matrix);
      } else if (type == 2) {
        pPattern =
            new CPDF_ShadingPattern(m_pPDFDoc, pPatternObj, FALSE, matrix);
      }
    }
  }
  if (!pPattern)
    return nullptr;

  if (!ptData) {
    ptData = new CPDF_CountedPattern(pPattern);
    m_PatternMap[pPatternObj] = ptData;
  } else {
    ptData->reset(pPattern);
  }
  return ptData->AddRef();
}

void CPDF_DocPageData::ReleasePattern(CPDF_Object* pPatternObj) {
  if (!pPatternObj)
    return;

  auto it = m_PatternMap.find(pPatternObj);
  if (it == m_PatternMap.end())
    return;

  CPDF_CountedPattern* ptData = it->second;
  if (ptData->get()) {
    ptData->RemoveRef();
    if (ptData->use_count() == 0) {
      ptData->clear();
    }
  }
}

CPDF_Image* CPDF_DocPageData::GetImage(CPDF_Object* pImageStream) {
  if (!pImageStream)
    return nullptr;

  const FX_DWORD dwImageObjNum = pImageStream->GetObjNum();
  auto it = m_ImageMap.find(dwImageObjNum);
  if (it != m_ImageMap.end()) {
    return it->second->AddRef();
  }

  CPDF_Image* pImage = new CPDF_Image(m_pPDFDoc);
  pImage->LoadImageF(pImageStream->AsStream(), FALSE);

  CPDF_CountedImage* imageData = new CPDF_CountedImage(pImage);
  m_ImageMap[dwImageObjNum] = imageData;
  return imageData->AddRef();
}

void CPDF_DocPageData::ReleaseImage(CPDF_Object* pImageStream) {
  if (!pImageStream || !pImageStream->GetObjNum())
    return;

  auto it = m_ImageMap.find(pImageStream->GetObjNum());
  if (it == m_ImageMap.end())
    return;

  CPDF_CountedImage* image = it->second;
  if (!image)
    return;

  image->RemoveRef();
  if (image->use_count() == 0) {
    delete image->get();
    delete image;
    m_ImageMap.erase(it);
  }
}

CPDF_IccProfile* CPDF_DocPageData::GetIccProfile(
    CPDF_Stream* pIccProfileStream) {
  if (!pIccProfileStream)
    return NULL;

  auto it = m_IccProfileMap.find(pIccProfileStream);
  if (it != m_IccProfileMap.end()) {
    return it->second->AddRef();
  }

  CPDF_StreamAcc stream;
  stream.LoadAllData(pIccProfileStream, FALSE);
  uint8_t digest[20];
  CRYPT_SHA1Generate(stream.GetData(), stream.GetSize(), digest);
  auto hash_it = m_HashProfileMap.find(CFX_ByteStringC(digest, 20));
  if (hash_it != m_HashProfileMap.end()) {
    auto it_copied_stream = m_IccProfileMap.find(hash_it->second);
    return it_copied_stream->second->AddRef();
  }
  CPDF_IccProfile* pProfile =
      new CPDF_IccProfile(stream.GetData(), stream.GetSize());
  CPDF_CountedIccProfile* ipData = new CPDF_CountedIccProfile(pProfile);
  m_IccProfileMap[pIccProfileStream] = ipData;
  m_HashProfileMap[CFX_ByteStringC(digest, 20)] = pIccProfileStream;
  return ipData->AddRef();
}

void CPDF_DocPageData::ReleaseIccProfile(CPDF_IccProfile* pIccProfile) {
  ASSERT(pIccProfile);

  for (auto it = m_IccProfileMap.begin(); it != m_IccProfileMap.end(); ++it) {
    CPDF_CountedIccProfile* profile = it->second;
    if (profile->get() != pIccProfile)
      continue;

    profile->RemoveRef();
    if (profile->use_count() == 0) {
      delete profile->get();
      delete profile;
      m_IccProfileMap.erase(it);
      return;
    }
  }
}

CPDF_StreamAcc* CPDF_DocPageData::GetFontFileStreamAcc(
    CPDF_Stream* pFontStream) {
  ASSERT(pFontStream);

  auto it = m_FontFileMap.find(pFontStream);
  if (it != m_FontFileMap.end())
    return it->second->AddRef();

  CPDF_Dictionary* pFontDict = pFontStream->GetDict();
  int32_t org_size = pFontDict->GetInteger("Length1") +
                     pFontDict->GetInteger("Length2") +
                     pFontDict->GetInteger("Length3");
  if (org_size < 0)
    org_size = 0;

  CPDF_StreamAcc* pFontFile = new CPDF_StreamAcc;
  pFontFile->LoadAllData(pFontStream, FALSE, org_size);

  CPDF_CountedStreamAcc* ftData = new CPDF_CountedStreamAcc(pFontFile);
  m_FontFileMap[pFontStream] = ftData;
  return ftData->AddRef();
}

void CPDF_DocPageData::ReleaseFontFileStreamAcc(CPDF_Stream* pFontStream,
                                                FX_BOOL bForce) {
  if (!pFontStream)
    return;

  auto it = m_FontFileMap.find(pFontStream);
  if (it == m_FontFileMap.end())
    return;

  CPDF_CountedStreamAcc* findData = it->second;
  if (!findData)
    return;

  findData->RemoveRef();
  if (findData->use_count() == 0 || bForce) {
    delete findData->get();
    delete findData;
    m_FontFileMap.erase(it);
  }
}

CPDF_CountedColorSpace* CPDF_DocPageData::FindColorSpacePtr(
    CPDF_Object* pCSObj) const {
  if (!pCSObj)
    return nullptr;

  auto it = m_ColorSpaceMap.find(pCSObj);
  return it != m_ColorSpaceMap.end() ? it->second : nullptr;
}

CPDF_CountedPattern* CPDF_DocPageData::FindPatternPtr(
    CPDF_Object* pPatternObj) const {
  if (!pPatternObj)
    return nullptr;

  auto it = m_PatternMap.find(pPatternObj);
  return it != m_PatternMap.end() ? it->second : nullptr;
}
