// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/xfa_ffapp.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"
#include "xfa/fxfa/app/xfa_fwladapter.h"
#include "xfa/fxfa/app/xfa_fwltheme.h"
#include "xfa/fxfa/xfa_ffdoc.h"
#include "xfa/fxfa/xfa_ffdochandler.h"
#include "xfa/fxfa/xfa_ffwidgethandler.h"
#include "xfa/fxfa/xfa_fontmgr.h"

namespace {

class CXFA_FileRead : public IFX_SeekableReadStream {
 public:
  explicit CXFA_FileRead(const std::vector<CPDF_Stream*>& streams);
  ~CXFA_FileRead() override;

  // IFX_SeekableReadStream
  FX_FILESIZE GetSize() override;
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;

 private:
  std::vector<std::unique_ptr<CPDF_StreamAcc>> m_Data;
};

CXFA_FileRead::CXFA_FileRead(const std::vector<CPDF_Stream*>& streams) {
  for (CPDF_Stream* pStream : streams) {
    m_Data.push_back(pdfium::MakeUnique<CPDF_StreamAcc>());
    m_Data.back()->LoadAllData(pStream);
  }
}

CXFA_FileRead::~CXFA_FileRead() {}

FX_FILESIZE CXFA_FileRead::GetSize() {
  uint32_t dwSize = 0;
  for (const auto& acc : m_Data)
    dwSize += acc->GetSize();
  return dwSize;
}

bool CXFA_FileRead::ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) {
  int32_t iCount = pdfium::CollectionSize<int32_t>(m_Data);
  int32_t index = 0;
  while (index < iCount) {
    const auto& acc = m_Data[index];
    FX_FILESIZE dwSize = acc->GetSize();
    if (offset < dwSize)
      break;

    offset -= dwSize;
    index++;
  }
  while (index < iCount) {
    const auto& acc = m_Data[index];
    uint32_t dwSize = acc->GetSize();
    size_t dwRead = std::min(size, static_cast<size_t>(dwSize - offset));
    FXSYS_memcpy(buffer, acc->GetData() + offset, dwRead);
    size -= dwRead;
    if (size == 0)
      return true;

    buffer = (uint8_t*)buffer + dwRead;
    offset = 0;
    index++;
  }
  return false;
}

}  // namespace

CFX_RetainPtr<IFX_SeekableReadStream> MakeSeekableReadStream(
    const std::vector<CPDF_Stream*>& streams) {
  return CFX_RetainPtr<IFX_SeekableReadStream>(new CXFA_FileRead(streams));
}

CXFA_FFApp::CXFA_FFApp(IXFA_AppProvider* pProvider)
    : m_pProvider(pProvider),
      m_pWidgetMgrDelegate(nullptr),
      m_pFWLApp(new CFWL_App(this)) {}

CXFA_FFApp::~CXFA_FFApp() {}

CXFA_FFDocHandler* CXFA_FFApp::GetDocHandler() {
  if (!m_pDocHandler)
    m_pDocHandler = pdfium::MakeUnique<CXFA_FFDocHandler>();
  return m_pDocHandler.get();
}

std::unique_ptr<CXFA_FFDoc> CXFA_FFApp::CreateDoc(
    IXFA_DocEnvironment* pDocEnvironment,
    CPDF_Document* pPDFDoc) {
  if (!pPDFDoc)
    return nullptr;

  auto pDoc = pdfium::MakeUnique<CXFA_FFDoc>(this, pDocEnvironment);
  if (!pDoc->OpenDoc(pPDFDoc))
    return nullptr;

  return pDoc;
}

void CXFA_FFApp::SetDefaultFontMgr(std::unique_ptr<CXFA_DefFontMgr> pFontMgr) {
  if (!m_pFontMgr)
    m_pFontMgr = pdfium::MakeUnique<CXFA_FontMgr>();
  m_pFontMgr->SetDefFontMgr(std::move(pFontMgr));
}

CXFA_FontMgr* CXFA_FFApp::GetXFAFontMgr() const {
  return m_pFontMgr.get();
}

CFGAS_FontMgr* CXFA_FFApp::GetFDEFontMgr() {
  if (!m_pFDEFontMgr) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    m_pFDEFontMgr = CFGAS_FontMgr::Create(FX_GetDefFontEnumerator());
#else
    m_pFontSource = pdfium::MakeUnique<CFX_FontSourceEnum_File>();
    m_pFDEFontMgr = CFGAS_FontMgr::Create(m_pFontSource.get());
#endif
  }
  return m_pFDEFontMgr.get();
}

CXFA_FWLTheme* CXFA_FFApp::GetFWLTheme() {
  if (!m_pFWLTheme)
    m_pFWLTheme = pdfium::MakeUnique<CXFA_FWLTheme>(this);
  return m_pFWLTheme.get();
}

CXFA_FWLAdapterWidgetMgr* CXFA_FFApp::GetWidgetMgr(
    CFWL_WidgetMgrDelegate* pDelegate) {
  if (!m_pAdapterWidgetMgr) {
    m_pAdapterWidgetMgr = pdfium::MakeUnique<CXFA_FWLAdapterWidgetMgr>();
    pDelegate->OnSetCapability(FWL_WGTMGR_DisableForm);
    m_pWidgetMgrDelegate = pDelegate;
  }
  return m_pAdapterWidgetMgr.get();
}

IFWL_AdapterTimerMgr* CXFA_FFApp::GetTimerMgr() const {
  return m_pProvider->GetTimerMgr();
}

void CXFA_FFApp::ClearEventTargets() {
  m_pFWLApp->GetNoteDriver()->ClearEventTargets(false);
}
