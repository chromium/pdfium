// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_helpers.h"

#include "build/build_config.h"
#include "constants/form_fields.h"
#include "constants/stream_dict_common.h"
#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fpdfdoc/cpdf_metadata.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "public/fpdf_ext.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#endif

namespace {

constexpr char kQuadPoints[] = "QuadPoints";

// 0 bit: FPDF_POLICY_MACHINETIME_ACCESS
uint32_t g_sandbox_policy = 0xFFFFFFFF;

#if !defined(OS_WIN)
int g_last_error = 0;
#endif

bool RaiseUnsupportedError(int nError) {
  auto* pAdapter = CPDF_ModuleMgr::Get()->GetUnsupportInfoAdapter();
  if (!pAdapter)
    return false;

  UNSUPPORT_INFO* info = static_cast<UNSUPPORT_INFO*>(pAdapter->info());
  if (info && info->FSDK_UnSupport_Handler)
    info->FSDK_UnSupport_Handler(info, nError);
  return true;
}

unsigned long GetStreamMaybeCopyAndReturnLengthImpl(const CPDF_Stream* stream,
                                                    void* buffer,
                                                    unsigned long buflen,
                                                    bool decode) {
  ASSERT(stream);
  auto stream_acc = pdfium::MakeRetain<CPDF_StreamAcc>(stream);

  if (decode)
    stream_acc->LoadAllDataFiltered();
  else
    stream_acc->LoadAllDataRaw();

  const auto stream_data_size = stream_acc->GetSize();
  if (!buffer || buflen < stream_data_size)
    return stream_data_size;

  memcpy(buffer, stream_acc->GetData(), stream_data_size);
  return stream_data_size;
}

#ifdef PDF_ENABLE_XFA
class FPDF_FileHandlerContext final : public IFX_SeekableStream {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  // IFX_SeekableStream:
  FX_FILESIZE GetSize() override;
  bool IsEOF() override;
  FX_FILESIZE GetPosition() override;
  bool ReadBlockAtOffset(void* buffer,
                         FX_FILESIZE offset,
                         size_t size) override;
  size_t ReadBlock(void* buffer, size_t size) override;
  bool WriteBlockAtOffset(const void* buffer,
                          FX_FILESIZE offset,
                          size_t size) override;
  bool Flush() override;

  void SetPosition(FX_FILESIZE pos) { m_nCurPos = pos; }

 private:
  explicit FPDF_FileHandlerContext(FPDF_FILEHANDLER* pFS);
  ~FPDF_FileHandlerContext() override;

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

bool FPDF_FileHandlerContext::ReadBlockAtOffset(void* buffer,
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

bool FPDF_FileHandlerContext::WriteBlockAtOffset(const void* buffer,
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

IPDF_Page* IPDFPageFromFPDFPage(FPDF_PAGE page) {
  return reinterpret_cast<IPDF_Page*>(page);
}

FPDF_PAGE FPDFPageFromIPDFPage(IPDF_Page* page) {
  return reinterpret_cast<FPDF_PAGE>(page);
}

CPDF_Document* CPDFDocumentFromFPDFDocument(FPDF_DOCUMENT doc) {
  return reinterpret_cast<CPDF_Document*>(doc);
}

FPDF_DOCUMENT FPDFDocumentFromCPDFDocument(CPDF_Document* doc) {
  return reinterpret_cast<FPDF_DOCUMENT>(doc);
}

CPDF_Page* CPDFPageFromFPDFPage(FPDF_PAGE page) {
  return page ? IPDFPageFromFPDFPage(page)->AsPDFPage() : nullptr;
}

CPDFSDK_InteractiveForm* FormHandleToInteractiveForm(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      CPDFSDKFormFillEnvironmentFromFPDFFormHandle(hHandle);
  return pFormFillEnv ? pFormFillEnv->GetInteractiveForm() : nullptr;
}

ByteString ByteStringFromFPDFWideString(FPDF_WIDESTRING wide_string) {
  return WideStringFromFPDFWideString(wide_string).ToUTF8();
}

WideString WideStringFromFPDFWideString(FPDF_WIDESTRING wide_string) {
  return WideString::FromUTF16LE(wide_string,
                                 WideString::WStringLength(wide_string));
}

#ifdef PDF_ENABLE_XFA
RetainPtr<IFX_SeekableStream> MakeSeekableStream(
    FPDF_FILEHANDLER* pFilehandler) {
  return pdfium::MakeRetain<FPDF_FileHandlerContext>(pFilehandler);
}
#endif  // PDF_ENABLE_XFA

const CPDF_Array* GetQuadPointsArrayFromDictionary(
    const CPDF_Dictionary* dict) {
  return dict->GetArrayFor("QuadPoints");
}

CPDF_Array* GetQuadPointsArrayFromDictionary(CPDF_Dictionary* dict) {
  return dict->GetArrayFor("QuadPoints");
}

CPDF_Array* AddQuadPointsArrayToDictionary(CPDF_Dictionary* dict) {
  return dict->SetNewFor<CPDF_Array>(kQuadPoints);
}

bool IsValidQuadPointsIndex(const CPDF_Array* array, size_t index) {
  return array && index < array->size() / 8;
}

bool GetQuadPointsAtIndex(const CPDF_Array* array,
                          size_t quad_index,
                          FS_QUADPOINTSF* quad_points) {
  ASSERT(quad_points);
  ASSERT(array);

  if (!IsValidQuadPointsIndex(array, quad_index))
    return false;

  quad_index *= 8;
  quad_points->x1 = array->GetNumberAt(quad_index);
  quad_points->y1 = array->GetNumberAt(quad_index + 1);
  quad_points->x2 = array->GetNumberAt(quad_index + 2);
  quad_points->y2 = array->GetNumberAt(quad_index + 3);
  quad_points->x3 = array->GetNumberAt(quad_index + 4);
  quad_points->y3 = array->GetNumberAt(quad_index + 5);
  quad_points->x4 = array->GetNumberAt(quad_index + 6);
  quad_points->y4 = array->GetNumberAt(quad_index + 7);
  return true;
}

CFX_FloatRect CFXFloatRectFromFSRECTF(const FS_RECTF& rect) {
  return CFX_FloatRect(rect.left, rect.bottom, rect.right, rect.top);
}

void FSRECTFFromCFXFloatRect(const CFX_FloatRect& rect, FS_RECTF* out_rect) {
  out_rect->left = rect.left;
  out_rect->top = rect.top;
  out_rect->right = rect.right;
  out_rect->bottom = rect.bottom;
}

CFX_Matrix CFXMatrixFromFSMatrix(const FS_MATRIX& matrix) {
  return CFX_Matrix(matrix.a, matrix.b, matrix.c, matrix.d, matrix.e, matrix.f);
}

unsigned long Utf16EncodeMaybeCopyAndReturnLength(const WideString& text,
                                                  void* buffer,
                                                  unsigned long buflen) {
  ByteString encoded_text = text.ToUTF16LE();
  unsigned long len = encoded_text.GetLength();
  if (buffer && len <= buflen)
    memcpy(buffer, encoded_text.c_str(), len);
  return len;
}

unsigned long GetRawStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen) {
  return GetStreamMaybeCopyAndReturnLengthImpl(stream, buffer, buflen,
                                               /*decode=*/false);
}

unsigned long DecodeStreamMaybeCopyAndReturnLength(const CPDF_Stream* stream,
                                                   void* buffer,
                                                   unsigned long buflen) {
  return GetStreamMaybeCopyAndReturnLengthImpl(stream, buffer, buflen,
                                               /*decode=*/true);
}

void SetPDFSandboxPolicy(FPDF_DWORD policy, FPDF_BOOL enable) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS: {
      if (enable)
        g_sandbox_policy |= 0x01;
      else
        g_sandbox_policy &= 0xFFFFFFFE;
    } break;
    default:
      break;
  }
}

FPDF_BOOL IsPDFSandboxPolicyEnabled(FPDF_DWORD policy) {
  switch (policy) {
    case FPDF_POLICY_MACHINETIME_ACCESS:
      return !!(g_sandbox_policy & 0x01);
    default:
      return false;
  }
}

void ReportUnsupportedFeatures(CPDF_Document* pDoc) {
  const CPDF_Dictionary* pRootDict = pDoc->GetRoot();
  if (pRootDict) {
    // Portfolios and Packages
    if (pRootDict->KeyExist("Collection")) {
      RaiseUnsupportedError(FPDF_UNSP_DOC_PORTABLECOLLECTION);
      return;
    }
    if (pRootDict->KeyExist("Names")) {
      const CPDF_Dictionary* pNameDict = pRootDict->GetDictFor("Names");
      if (pNameDict && pNameDict->KeyExist("EmbeddedFiles")) {
        RaiseUnsupportedError(FPDF_UNSP_DOC_ATTACHMENT);
        return;
      }
      if (pNameDict && pNameDict->KeyExist("JavaScript")) {
        const CPDF_Dictionary* pJSDict = pNameDict->GetDictFor("JavaScript");
        const CPDF_Array* pArray =
            pJSDict ? pJSDict->GetArrayFor("Names") : nullptr;
        if (pArray) {
          for (size_t i = 0; i < pArray->size(); i++) {
            ByteString cbStr = pArray->GetStringAt(i);
            if (cbStr.Compare("com.adobe.acrobat.SharedReview.Register") == 0) {
              RaiseUnsupportedError(FPDF_UNSP_DOC_SHAREDREVIEW);
              return;
            }
          }
        }
      }
    }

    // SharedForm
    const CPDF_Stream* pStream = pRootDict->GetStreamFor("Metadata");
    if (pStream) {
      CPDF_Metadata metaData(pStream);
      for (const auto& err : metaData.CheckForSharedForm())
        RaiseUnsupportedError(static_cast<int>(err));
    }
  }

  // XFA Forms
  if (!pDoc->GetExtension() && CPDF_InteractiveForm(pDoc).HasXFAForm())
    RaiseUnsupportedError(FPDF_UNSP_DOC_XFAFORM);
}

void CheckForUnsupportedAnnot(const CPDF_Annot* pAnnot) {
  switch (pAnnot->GetSubtype()) {
    case CPDF_Annot::Subtype::FILEATTACHMENT:
      RaiseUnsupportedError(FPDF_UNSP_ANNOT_ATTACHMENT);
      break;
    case CPDF_Annot::Subtype::MOVIE:
      RaiseUnsupportedError(FPDF_UNSP_ANNOT_MOVIE);
      break;
    case CPDF_Annot::Subtype::RICHMEDIA:
      RaiseUnsupportedError(FPDF_UNSP_ANNOT_SCREEN_RICHMEDIA);
      break;
    case CPDF_Annot::Subtype::SCREEN: {
      const CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
      ByteString cbString = pAnnotDict->GetStringFor("IT");
      if (cbString != "Img")
        RaiseUnsupportedError(FPDF_UNSP_ANNOT_SCREEN_MEDIA);
      break;
    }
    case CPDF_Annot::Subtype::SOUND:
      RaiseUnsupportedError(FPDF_UNSP_ANNOT_SOUND);
      break;
    case CPDF_Annot::Subtype::THREED:
      RaiseUnsupportedError(FPDF_UNSP_ANNOT_3DANNOT);
      break;
    case CPDF_Annot::Subtype::WIDGET: {
      const CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
      ByteString cbString = pAnnotDict->GetStringFor(pdfium::form_fields::kFT);
      if (cbString == pdfium::form_fields::kSig)
        RaiseUnsupportedError(FPDF_UNSP_ANNOT_SIG);
      break;
    }
    default:
      break;
  }
}

#if !defined(OS_WIN)
void SetLastError(int err) {
  g_last_error = err;
}

int GetLastError() {
  return g_last_error;
}
#endif

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
