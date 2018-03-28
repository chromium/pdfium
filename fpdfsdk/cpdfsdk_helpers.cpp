// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_helpers.h"

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/fpdf_parser_decode.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "core/fpdfdoc/cpdf_metadata.h"
#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_element.h"
#include "public/fpdf_ext.h"

namespace {

FPDF_DOCUMENT FPDFDocumentFromUnderlying(UnderlyingDocumentType* doc) {
  return static_cast<FPDF_DOCUMENT>(doc);
}

bool RaiseUnSupportError(int nError) {
  CFSDK_UnsupportInfo_Adapter* pAdapter =
      CPDF_ModuleMgr::Get()->GetUnsupportInfoAdapter();
  if (!pAdapter)
    return false;

  UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(pAdapter->GetUnspInfo());
  if (info && info->FSDK_UnSupport_Handler)
    info->FSDK_UnSupport_Handler(info, nError);
  return true;
}

bool CheckSharedForm(const CXML_Element* pElement, ByteString cbName) {
  size_t count = pElement->CountAttrs();
  for (size_t i = 0; i < count; ++i) {
    ByteString space;
    ByteString name;
    WideString value;
    pElement->GetAttrByIndex(i, &space, &name, &value);
    if (space == "xmlns" && name == "adhocwf" &&
        value == L"http://ns.adobe.com/AcrobatAdhocWorkflow/1.0/") {
      CXML_Element* pVersion =
          pElement->GetElement("adhocwf", cbName.AsStringView(), 0);
      if (!pVersion)
        continue;
      CXML_Content* pContent = ToContent(pVersion->GetChild(0));
      if (!pContent)
        continue;
      switch (pContent->m_Content.GetInteger()) {
        case 1:
          RaiseUnSupportError(FPDF_UNSP_DOC_SHAREDFORM_ACROBAT);
          break;
        case 2:
          RaiseUnSupportError(FPDF_UNSP_DOC_SHAREDFORM_FILESYSTEM);
          break;
        case 0:
          RaiseUnSupportError(FPDF_UNSP_DOC_SHAREDFORM_EMAIL);
          break;
      }
    }
  }

  size_t nCount = pElement->CountChildren();
  for (size_t i = 0; i < nCount; ++i) {
    CXML_Element* pChild = ToElement(pElement->GetChild(i));
    if (pChild && CheckSharedForm(pChild, cbName))
      return true;
  }
  return false;
}

#ifdef PDF_ENABLE_XFA
class FPDF_FileHandlerContext : public IFX_SeekableStream {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  ~FPDF_FileHandlerContext() override;

  // IFX_SeekableStream:
  FX_FILESIZE GetSize() override;
  bool IsEOF() override;
  FX_FILESIZE GetPosition() override;
  bool ReadBlock(void* buffer, FX_FILESIZE offset, size_t size) override;
  size_t ReadBlock(void* buffer, size_t size) override;
  bool WriteBlock(const void* buffer, FX_FILESIZE offset, size_t size) override;
  bool Flush() override;

  void SetPosition(FX_FILESIZE pos) { m_nCurPos = pos; }

 protected:
  explicit FPDF_FileHandlerContext(FPDF_FILEHANDLER* pFS);

  FPDF_FILEHANDLER* m_pFS;
  FX_FILESIZE m_nCurPos;
};

FPDF_FileHandlerContext::FPDF_FileHandlerContext(FPDF_FILEHANDLER* pFS) {
  m_pFS = pFS;
  m_nCurPos = 0;
}

FPDF_FileHandlerContext::~FPDF_FileHandlerContext() {
  if (m_pFS && m_pFS->Release)
    m_pFS->Release(m_pFS->clientData);
}

FX_FILESIZE FPDF_FileHandlerContext::GetSize() {
  if (m_pFS && m_pFS->GetSize)
    return (FX_FILESIZE)m_pFS->GetSize(m_pFS->clientData);
  return 0;
}

bool FPDF_FileHandlerContext::IsEOF() {
  return m_nCurPos >= GetSize();
}

FX_FILESIZE FPDF_FileHandlerContext::GetPosition() {
  return m_nCurPos;
}

bool FPDF_FileHandlerContext::ReadBlock(void* buffer,
                                        FX_FILESIZE offset,
                                        size_t size) {
  if (!buffer || !size || !m_pFS->ReadBlock)
    return false;

  if (m_pFS->ReadBlock(m_pFS->clientData, (FPDF_DWORD)offset, buffer,
                       (FPDF_DWORD)size) == 0) {
    m_nCurPos = offset + size;
    return true;
  }
  return false;
}

size_t FPDF_FileHandlerContext::ReadBlock(void* buffer, size_t size) {
  if (!buffer || !size || !m_pFS->ReadBlock)
    return 0;

  FX_FILESIZE nSize = GetSize();
  if (m_nCurPos >= nSize)
    return 0;
  FX_FILESIZE dwAvail = nSize - m_nCurPos;
  if (dwAvail < (FX_FILESIZE)size)
    size = static_cast<size_t>(dwAvail);
  if (m_pFS->ReadBlock(m_pFS->clientData, (FPDF_DWORD)m_nCurPos, buffer,
                       (FPDF_DWORD)size) == 0) {
    m_nCurPos += size;
    return size;
  }

  return 0;
}

bool FPDF_FileHandlerContext::WriteBlock(const void* buffer,
                                         FX_FILESIZE offset,
                                         size_t size) {
  if (!m_pFS || !m_pFS->WriteBlock)
    return false;

  if (m_pFS->WriteBlock(m_pFS->clientData, (FPDF_DWORD)offset, buffer,
                        (FPDF_DWORD)size) == 0) {
    m_nCurPos = offset + size;
    return true;
  }
  return false;
}

bool FPDF_FileHandlerContext::Flush() {
  if (!m_pFS || !m_pFS->Flush)
    return true;

  return m_pFS->Flush(m_pFS->clientData) == 0;
}
#endif  // PDF_ENABLE_XFA

}  // namespace

UnderlyingDocumentType* UnderlyingFromFPDFDocument(FPDF_DOCUMENT doc) {
  return static_cast<UnderlyingDocumentType*>(doc);
}

UnderlyingPageType* UnderlyingFromFPDFPage(FPDF_PAGE page) {
  return static_cast<UnderlyingPageType*>(page);
}

CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc) {
#ifdef PDF_ENABLE_XFA
  return doc ? UnderlyingFromFPDFDocument(doc)->GetPDFDoc() : nullptr;
#else   // PDF_ENABLE_XFA
  return UnderlyingFromFPDFDocument(doc);
#endif  // PDF_ENABLE_XFA
}

FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc) {
#ifdef PDF_ENABLE_XFA
  return doc ? FPDFDocumentFromUnderlying(
                   new CPDFXFA_Context(pdfium::WrapUnique(doc)))
             : nullptr;
#else   // PDF_ENABLE_XFA
  return FPDFDocumentFromUnderlying(doc);
#endif  // PDF_ENABLE_XFA
}

CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page) {
#ifdef PDF_ENABLE_XFA
  return page ? UnderlyingFromFPDFPage(page)->GetPDFPage() : nullptr;
#else   // PDF_ENABLE_XFA
  return UnderlyingFromFPDFPage(page);
#endif  // PDF_ENABLE_XFA
}

CPDF_PageObject* CPDFPageObjectFromFPDFPageObject(FPDF_PAGEOBJECT page_object) {
  return static_cast<CPDF_PageObject*>(page_object);
}

ByteString CFXByteStringFromFPDFWideString(FPDF_WIDESTRING wide_string) {
  return WideString::FromUTF16LE(wide_string,
                                 WideString::WStringLength(wide_string))
      .UTF8Encode();
}

CFX_DIBitmap* CFXBitmapFromFPDFBitmap(FPDF_BITMAP bitmap) {
  return static_cast<CFX_DIBitmap*>(bitmap);
}

void CheckUnSupportAnnot(CPDF_Document* pDoc, const CPDF_Annot* pPDFAnnot) {
  CPDF_Annot::Subtype nAnnotSubtype = pPDFAnnot->GetSubtype();
  if (nAnnotSubtype == CPDF_Annot::Subtype::THREED) {
    RaiseUnSupportError(FPDF_UNSP_ANNOT_3DANNOT);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::SCREEN) {
    const CPDF_Dictionary* pAnnotDict = pPDFAnnot->GetAnnotDict();
    ByteString cbString;
    if (pAnnotDict->KeyExist("IT"))
      cbString = pAnnotDict->GetStringFor("IT");
    if (cbString.Compare("Img") != 0)
      RaiseUnSupportError(FPDF_UNSP_ANNOT_SCREEN_MEDIA);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::MOVIE) {
    RaiseUnSupportError(FPDF_UNSP_ANNOT_MOVIE);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::SOUND) {
    RaiseUnSupportError(FPDF_UNSP_ANNOT_SOUND);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::RICHMEDIA) {
    RaiseUnSupportError(FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::FILEATTACHMENT) {
    RaiseUnSupportError(FPDF_UNSP_ANNOT_ATTACHMENT);
  } else if (nAnnotSubtype == CPDF_Annot::Subtype::WIDGET) {
    const CPDF_Dictionary* pAnnotDict = pPDFAnnot->GetAnnotDict();
    ByteString cbString;
    if (pAnnotDict->KeyExist("FT"))
      cbString = pAnnotDict->GetStringFor("FT");
    if (cbString.Compare("Sig") == 0)
      RaiseUnSupportError(FPDF_UNSP_ANNOT_SIG);
  }
}

void CheckUnSupportError(CPDF_Document* pDoc, uint32_t err_code) {
  // Security
  if (err_code == FPDF_ERR_SECURITY) {
    RaiseUnSupportError(FPDF_UNSP_DOC_SECURITY);
    return;
  }
  if (!pDoc)
    return;

  // Portfolios and Packages
  const CPDF_Dictionary* pRootDict = pDoc->GetRoot();
  if (pRootDict) {
    ByteString cbString;
    if (pRootDict->KeyExist("Collection")) {
      RaiseUnSupportError(FPDF_UNSP_DOC_PORTABLECOLLECTION);
      return;
    }
    if (pRootDict->KeyExist("Names")) {
      CPDF_Dictionary* pNameDict = pRootDict->GetDictFor("Names");
      if (pNameDict && pNameDict->KeyExist("EmbeddedFiles")) {
        RaiseUnSupportError(FPDF_UNSP_DOC_ATTACHMENT);
        return;
      }
      if (pNameDict && pNameDict->KeyExist("JavaScript")) {
        CPDF_Dictionary* pJSDict = pNameDict->GetDictFor("JavaScript");
        CPDF_Array* pArray = pJSDict ? pJSDict->GetArrayFor("Names") : nullptr;
        if (pArray) {
          for (size_t i = 0; i < pArray->GetCount(); i++) {
            ByteString cbStr = pArray->GetStringAt(i);
            if (cbStr.Compare("com.adobe.acrobat.SharedReview.Register") == 0) {
              RaiseUnSupportError(FPDF_UNSP_DOC_SHAREDREVIEW);
              return;
            }
          }
        }
      }
    }
  }

  // SharedForm
  CPDF_Metadata metaData(pDoc);
  const CXML_Element* pElement = metaData.GetRoot();
  if (pElement)
    CheckSharedForm(pElement, "workflowType");

#ifndef PDF_ENABLE_XFA
  // XFA Forms
  CPDF_InterForm interform(pDoc);
  if (interform.HasXFAForm())
    RaiseUnSupportError(FPDF_UNSP_DOC_XFAFORM);
#endif  // PDF_ENABLE_XFA
}

#ifndef _WIN32
int g_LastError;
void SetLastError(int err) {
  g_LastError = err;
}

int GetLastError() {
  return g_LastError;
}
#endif  // _WIN32

void ProcessParseError(CPDF_Parser::Error err) {
  uint32_t err_code = FPDF_ERR_SUCCESS;
  // Translate FPDFAPI error code to FPDFVIEW error code
  switch (err) {
    case CPDF_Parser::SUCCESS:
      err_code = FPDF_ERR_SUCCESS;
      break;
    case CPDF_Parser::FILE_ERROR:
      err_code = FPDF_ERR_FILE;
      break;
    case CPDF_Parser::FORMAT_ERROR:
      err_code = FPDF_ERR_FORMAT;
      break;
    case CPDF_Parser::PASSWORD_ERROR:
      err_code = FPDF_ERR_PASSWORD;
      break;
    case CPDF_Parser::HANDLER_ERROR:
      err_code = FPDF_ERR_SECURITY;
      break;
  }
  SetLastError(err_code);
}

// 0 bit: FPDF_POLICY_MACHINETIME_ACCESS
static uint32_t foxit_sandbox_policy = 0xFFFFFFFF;

void FSDK_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS: {
      if (enable)
        foxit_sandbox_policy |= 0x01;
      else
        foxit_sandbox_policy &= 0xFFFFFFFE;
    } break;
    default:
      break;
  }
}

FPDF_BOOL FSDK_IsSandBoxPolicyEnabled(FPDF_DWORD policy) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS:
      return !!(foxit_sandbox_policy & 0x01);
    default:
      return false;
  }
}

unsigned long DecodeStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen) {
  ASSERT(stream);
  uint8_t* data = stream->GetRawData();
  uint32_t len = stream->GetRawSize();
  CPDF_Dictionary* dict = stream->GetDict();
  CPDF_Object* decoder = dict ? dict->GetDirectObjectFor("Filter") : nullptr;
  if (decoder && (decoder->IsArray() || decoder->IsName())) {
    // Decode the stream if one or more stream filters are specified.
    uint8_t* decoded_data = nullptr;
    uint32_t decoded_len = 0;
    ByteString dummy_last_decoder;
    CPDF_Dictionary* dummy_last_param;
    if (PDF_DataDecode(data, len, dict, dict->GetIntegerFor("DL"), false,
                       &decoded_data, &decoded_len, &dummy_last_decoder,
                       &dummy_last_param)) {
      if (buffer && buflen >= decoded_len)
        memcpy(buffer, decoded_data, decoded_len);

      // Free the buffer for the decoded data if it was allocated by
      // PDF_DataDecode(). Note that for images with a single image-specific
      // filter, |decoded_data| is directly assigned to be |data|, so
      // |decoded_data| does not need to be freed.
      if (decoded_data != data)
        FX_Free(decoded_data);

      return decoded_len;
    }
  }
  // Copy the raw data and return its length if there is no valid filter
  // specified or if decoding failed.
  if (buffer && buflen >= len)
    memcpy(buffer, data, len);

  return len;
}

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const WideString& text,
                                                  void* buffer,
                                                  unsigned long buflen) {
  ByteString encoded_text = text.UTF16LE_Encode();
  unsigned long len = encoded_text.GetLength();
  if (buffer && len <= buflen)
    memcpy(buffer, encoded_text.c_str(), len);
  return len;
}

void FSRECTFFromCFXFloatRect(const CFX_FloatRect& rect, FS_RECTF* out_rect) {
  out_rect->left = rect.left;
  out_rect->top = rect.top;
  out_rect->right = rect.right;
  out_rect->bottom = rect.bottom;
}

CFX_FloatRect CFXFloatRectFromFSRECTF(const FS_RECTF& rect) {
  return CFX_FloatRect(rect.left, rect.bottom, rect.right, rect.top);
}

const CPDF_Array* GetQuadPointsArrayFromDictionary(CPDF_Dictionary* dict) {
  return dict ? dict->GetArrayFor("QuadPoints") : nullptr;
}

bool GetQuadPointsFromDictionary(CPDF_Dictionary* dict,
                                 size_t quad_index,
                                 FS_QUADPOINTSF* quad_points) {
  ASSERT(quad_points);

  const CPDF_Array* pArray = GetQuadPointsArrayFromDictionary(dict);
  if (!pArray || quad_index >= pArray->GetCount() / 8)
    return false;

  quad_index *= 8;
  quad_points->x1 = pArray->GetNumberAt(quad_index);
  quad_points->y1 = pArray->GetNumberAt(quad_index + 1);
  quad_points->x2 = pArray->GetNumberAt(quad_index + 2);
  quad_points->y2 = pArray->GetNumberAt(quad_index + 3);
  quad_points->x3 = pArray->GetNumberAt(quad_index + 4);
  quad_points->y3 = pArray->GetNumberAt(quad_index + 5);
  quad_points->x4 = pArray->GetNumberAt(quad_index + 6);
  quad_points->y4 = pArray->GetNumberAt(quad_index + 7);
  return true;
}

#ifdef PDF_ENABLE_XFA
RetainPtr<IFX_SeekableStream> MakeSeekableStream(
    FPDF_FILEHANDLER* pFilehandler) {
  return pdfium::MakeRetain<FPDF_FileHandlerContext>(pFilehandler);
}
#endif  // PDF_ENABLE_XFA
