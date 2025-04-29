// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_formfillenvironment.h"

#include <stdint.h>

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_annotcontext.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/numerics/safe_conversions.h"
#include "core/fxcrt/stl_util.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fxjs/ijs_event_context.h"
#include "fxjs/ijs_runtime.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#endif

static_assert(FXCT_ARROW ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kArrow),
              "kArrow value mismatch");
static_assert(FXCT_NESW ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kNESW),
              "kNEWS value mismatch");
static_assert(FXCT_NWSE ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kNWSE),
              "kNWSE value mismatch");
static_assert(FXCT_VBEAM ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kVBeam),
              "kVBeam value mismatch");
static_assert(FXCT_HBEAM ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kHBeam),
              "HBeam value mismatch");
static_assert(FXCT_HAND ==
                  static_cast<int>(IPWL_FillerNotify::CursorStyle::kHand),
              "kHand value mismatch");

FPDF_WIDESTRING AsFPDFWideString(ByteString* bsUTF16LE) {
  // Force a private version of the string, since we're about to hand it off
  // to the embedder. Should the embedder modify it by accident, it won't
  // corrupt other shares of the string beyond |bsUTF16LE|.
  return reinterpret_cast<FPDF_WIDESTRING>(
      bsUTF16LE->GetBuffer(bsUTF16LE->GetLength()).data());
}

CPDFSDK_FormFillEnvironment::CPDFSDK_FormFillEnvironment(
    CPDF_Document* pDoc,
    FPDF_FORMFILLINFO* pFFinfo)
    : info_(pFFinfo),
      cpdfdoc_(pDoc),
      interactive_form_filler_(
          std::make_unique<CFFL_InteractiveFormFiller>(this)) {
  DCHECK(cpdfdoc_);
}

CPDFSDK_FormFillEnvironment::~CPDFSDK_FormFillEnvironment() {
  being_destroyed_ = true;
  ClearAllFocusedAnnots();

  // |page_map_| will try to access |interactive_form_| when it cleans itself
  // up. Make sure it is deleted before |interactive_form_|.
  page_map_.clear();

  // Must destroy the |interactive_form_filler_| before the environment (|this|)
  // because any created form widgets hold a pointer to the environment.
  // Those widgets may call things like KillTimer() as they are shutdown.
  interactive_form_filler_.reset();

  if (info_ && info_->Release) {
    info_->Release(info_);
  }
}

void CPDFSDK_FormFillEnvironment::InvalidateRect(CPDFSDK_Widget* widget,
                                                 const CFX_FloatRect& rect) {
  IPDF_Page* pPage = widget->GetPage();
  if (!pPage) {
    return;
  }

  CFX_Matrix device2page =
      widget->GetPageView()->GetCurrentMatrix().GetInverse();
  CFX_PointF left_top = device2page.Transform(CFX_PointF(rect.left, rect.top));
  CFX_PointF right_bottom =
      device2page.Transform(CFX_PointF(rect.right, rect.bottom));

  CFX_FloatRect rcPDF(left_top.x, right_bottom.y, right_bottom.x, left_top.y);
  rcPDF.Normalize();
  Invalidate(pPage, rcPDF.GetOuterRect());
}

void CPDFSDK_FormFillEnvironment::OutputSelectedRect(
    CFFL_FormField* pFormField,
    const CFX_FloatRect& rect) {
  if (!info_ || !info_->FFI_OutputSelectedRect) {
    return;
  }

  auto* pPage = FPDFPageFromIPDFPage(pFormField->GetSDKWidget()->GetPage());
  DCHECK(pPage);

  CFX_PointF ptA = pFormField->PWLtoFFL(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptB = pFormField->PWLtoFFL(CFX_PointF(rect.right, rect.top));
  info_->FFI_OutputSelectedRect(info_, pPage, ptA.x, ptB.y, ptB.x, ptA.y);
}

bool CPDFSDK_FormFillEnvironment::IsSelectionImplemented() const {
  FPDF_FORMFILLINFO* pInfo = GetFormFillInfo();
  return pInfo && pInfo->FFI_OutputSelectedRect;
}

#ifdef PDF_ENABLE_V8
CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetCurrentView() {
  IPDF_Page* pPage = GetCurrentPage();
  return pPage ? GetOrCreatePageView(pPage) : nullptr;
}

IPDF_Page* CPDFSDK_FormFillEnvironment::GetCurrentPage() const {
  if (info_ && info_->FFI_GetCurrentPage) {
    return IPDFPageFromFPDFPage(info_->FFI_GetCurrentPage(
        info_, FPDFDocumentFromCPDFDocument(cpdfdoc_)));
  }
  return nullptr;
}

WideString CPDFSDK_FormFillEnvironment::GetLanguage() {
#ifdef PDF_ENABLE_XFA
  if (!info_ || info_->version < 2 || !info_->FFI_GetLanguage) {
    return WideString();
  }

  int nRequiredLen = info_->FFI_GetLanguage(info_, nullptr, 0);
  if (nRequiredLen <= 0) {
    return WideString();
  }

  DataVector<uint8_t> pBuff(nRequiredLen);
  int nActualLen = info_->FFI_GetLanguage(info_, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen) {
    return WideString();
  }

  return WideString::FromUTF16LE(
      pdfium::span(pBuff).first(static_cast<size_t>(nActualLen)));
#else   // PDF_ENABLE_XFA
  return WideString();
#endif  // PDF_ENABLE_XFA
}

WideString CPDFSDK_FormFillEnvironment::GetPlatform() {
#ifdef PDF_ENABLE_XFA
  if (!info_ || info_->version < 2 || !info_->FFI_GetPlatform) {
    return WideString();
  }

  int nRequiredLen = info_->FFI_GetPlatform(info_, nullptr, 0);
  if (nRequiredLen <= 0) {
    return WideString();
  }

  DataVector<uint8_t> pBuff(nRequiredLen);
  int nActualLen = info_->FFI_GetPlatform(info_, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen) {
    return WideString();
  }

  return WideString::FromUTF16LE(
      pdfium::span(pBuff).first(static_cast<size_t>(nActualLen)));
#else   // PDF_ENABLE_XFA
  return WideString();
#endif  // PDF_ENABLE_XFA
}

int CPDFSDK_FormFillEnvironment::JS_appAlert(const WideString& Msg,
                                             const WideString& Title,
                                             int Type,
                                             int Icon) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->app_alert) {
    return -1;
  }

  ByteString bsMsg = Msg.ToUTF16LE();
  ByteString bsTitle = Title.ToUTF16LE();
  return js_platform->app_alert(js_platform, AsFPDFWideString(&bsMsg),
                                AsFPDFWideString(&bsTitle), Type, Icon);
}

int CPDFSDK_FormFillEnvironment::JS_appResponse(
    const WideString& Question,
    const WideString& Title,
    const WideString& Default,
    const WideString& Label,
    FPDF_BOOL bPassword,
    pdfium::span<uint8_t> response) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->app_response) {
    return -1;
  }

  ByteString bsQuestion = Question.ToUTF16LE();
  ByteString bsTitle = Title.ToUTF16LE();
  ByteString bsDefault = Default.ToUTF16LE();
  ByteString bsLabel = Label.ToUTF16LE();
  return js_platform->app_response(
      js_platform, AsFPDFWideString(&bsQuestion), AsFPDFWideString(&bsTitle),
      AsFPDFWideString(&bsDefault), AsFPDFWideString(&bsLabel), bPassword,
      response.data(), pdfium::checked_cast<int>(response.size()));
}

void CPDFSDK_FormFillEnvironment::JS_appBeep(int nType) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->app_beep) {
    return;
  }

  js_platform->app_beep(js_platform, nType);
}

WideString CPDFSDK_FormFillEnvironment::JS_fieldBrowse() {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Field_browse) {
    return WideString();
  }

  const int nRequiredLen = js_platform->Field_browse(js_platform, nullptr, 0);
  if (nRequiredLen <= 0) {
    return WideString();
  }

  DataVector<uint8_t> pBuff(nRequiredLen);
  const int nActualLen =
      js_platform->Field_browse(js_platform, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen) {
    return WideString();
  }

  // Don't include trailing NUL.
  pBuff.resize(nActualLen - 1);

  // Use FromDefANSI() per "local encoding" comment in fpdf_formfill.h.
  return WideString::FromDefANSI(ByteStringView(pBuff));
}

void CPDFSDK_FormFillEnvironment::JS_docmailForm(
    pdfium::span<const uint8_t> mailData,
    FPDF_BOOL bUI,
    const WideString& To,
    const WideString& Subject,
    const WideString& CC,
    const WideString& BCC,
    const WideString& Msg) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Doc_mail) {
    return;
  }

  ByteString bsTo = To.ToUTF16LE();
  ByteString bsSubject = Subject.ToUTF16LE();
  ByteString bsCC = CC.ToUTF16LE();
  ByteString bsBcc = BCC.ToUTF16LE();
  ByteString bsMsg = Msg.ToUTF16LE();
  js_platform->Doc_mail(js_platform, const_cast<uint8_t*>(mailData.data()),
                        pdfium::checked_cast<int>(mailData.size()), bUI,
                        AsFPDFWideString(&bsTo), AsFPDFWideString(&bsSubject),
                        AsFPDFWideString(&bsCC), AsFPDFWideString(&bsBcc),
                        AsFPDFWideString(&bsMsg));
}

void CPDFSDK_FormFillEnvironment::JS_docprint(FPDF_BOOL bUI,
                                              int nStart,
                                              int nEnd,
                                              FPDF_BOOL bSilent,
                                              FPDF_BOOL bShrinkToFit,
                                              FPDF_BOOL bPrintAsImage,
                                              FPDF_BOOL bReverse,
                                              FPDF_BOOL bAnnotations) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Doc_print) {
    return;
  }

  js_platform->Doc_print(js_platform, bUI, nStart, nEnd, bSilent, bShrinkToFit,
                         bPrintAsImage, bReverse, bAnnotations);
}

void CPDFSDK_FormFillEnvironment::JS_docgotoPage(int nPageNum) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Doc_gotoPage) {
    return;
  }

  js_platform->Doc_gotoPage(js_platform, nPageNum);
}

WideString CPDFSDK_FormFillEnvironment::JS_docGetFilePath() {
  return GetFilePath();
}
#endif  // PDF_ENABLE_V8

WideString CPDFSDK_FormFillEnvironment::GetFilePath() const {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Doc_getFilePath) {
    return WideString();
  }

  const int nRequiredLen =
      js_platform->Doc_getFilePath(js_platform, nullptr, 0);
  if (nRequiredLen <= 0) {
    return WideString();
  }

  DataVector<uint8_t> pBuff(nRequiredLen);
  const int nActualLen =
      js_platform->Doc_getFilePath(js_platform, pBuff.data(), nRequiredLen);
  if (nActualLen <= 0 || nActualLen > nRequiredLen) {
    return WideString();
  }

  // Don't include trailing NUL.
  pBuff.resize(nActualLen - 1);

  // Use FromDefANSI() per "local encoding" comment in fpdf_formfill.h.
  return WideString::FromDefANSI(ByteStringView(pBuff));
}

void CPDFSDK_FormFillEnvironment::SubmitForm(
    pdfium::span<const uint8_t> form_data,
    const WideString& URL) {
  IPDF_JSPLATFORM* js_platform = GetJSPlatform();
  if (!js_platform || !js_platform->Doc_submitForm) {
    return;
  }

  ByteString bsUrl = URL.ToUTF16LE();
  js_platform->Doc_submitForm(
      js_platform, const_cast<uint8_t*>(form_data.data()),
      fxcrt::CollectionSize<int>(form_data), AsFPDFWideString(&bsUrl));
}

IJS_Runtime* CPDFSDK_FormFillEnvironment::GetIJSRuntime() {
  if (!ijs_runtime_) {
    ijs_runtime_ = IJS_Runtime::Create(this);
  }
  return ijs_runtime_.get();
}

void CPDFSDK_FormFillEnvironment::Invalidate(IPDF_Page* page,
                                             const FX_RECT& rect) {
  if (info_ && info_->FFI_Invalidate) {
    info_->FFI_Invalidate(info_, FPDFPageFromIPDFPage(page), rect.left,
                          rect.top, rect.right, rect.bottom);
  }
}

void CPDFSDK_FormFillEnvironment::SetCursor(
    IPWL_FillerNotify::CursorStyle nCursorType) {
  if (info_ && info_->FFI_SetCursor) {
    info_->FFI_SetCursor(info_, static_cast<int>(nCursorType));
  }
}

int CPDFSDK_FormFillEnvironment::SetTimer(int uElapse,
                                          TimerCallback lpTimerFunc) {
  if (info_ && info_->FFI_SetTimer) {
    return info_->FFI_SetTimer(info_, uElapse, lpTimerFunc);
  }
  return CFX_Timer::HandlerIface::kInvalidTimerID;
}

void CPDFSDK_FormFillEnvironment::KillTimer(int nTimerID) {
  if (info_ && info_->FFI_KillTimer) {
    info_->FFI_KillTimer(info_, nTimerID);
  }
}

void CPDFSDK_FormFillEnvironment::OnChange() {
  if (info_ && info_->FFI_OnChange) {
    info_->FFI_OnChange(info_);
  }
}

void CPDFSDK_FormFillEnvironment::ExecuteNamedAction(
    const ByteString& namedAction) {
  if (info_ && info_->FFI_ExecuteNamedAction) {
    info_->FFI_ExecuteNamedAction(info_, namedAction.c_str());
  }
}

void CPDFSDK_FormFillEnvironment::OnSetFieldInputFocus(const WideString& text) {
  OnSetFieldInputFocusInternal(text, true);
}

void CPDFSDK_FormFillEnvironment::OnSetFieldInputFocusInternal(
    const WideString& text,
    bool bFocus) {
  if (info_ && info_->FFI_SetTextFieldFocus) {
    size_t nCharacters = text.GetLength();
    ByteString bsUTFText = text.ToUTF16LE();
    auto* pBuffer = reinterpret_cast<const unsigned short*>(bsUTFText.c_str());
    info_->FFI_SetTextFieldFocus(
        info_, pBuffer, pdfium::checked_cast<FPDF_DWORD>(nCharacters), bFocus);
  }
}

void CPDFSDK_FormFillEnvironment::OnCalculate(
    ObservedPtr<CPDFSDK_Annot>& pAnnot) {
  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  if (pWidget) {
    interactive_form_->OnCalculate(pWidget->GetFormField());
  }
}

void CPDFSDK_FormFillEnvironment::OnFormat(ObservedPtr<CPDFSDK_Annot>& pAnnot) {
  ObservedPtr<CPDFSDK_Widget> pWidget(ToCPDFSDKWidget(pAnnot.Get()));
  std::optional<WideString> sValue =
      interactive_form_->OnFormat(pWidget->GetFormField());
  if (!pWidget) {
    return;
  }
  if (sValue.has_value()) {
    interactive_form_->ResetFieldAppearance(pWidget->GetFormField(), sValue);
    interactive_form_->UpdateField(pWidget->GetFormField());
  }
}

void CPDFSDK_FormFillEnvironment::DoURIAction(const ByteString& bsURI,
                                              Mask<FWL_EVENTFLAG> modifiers) {
  if (!info_) {
    return;
  }

  if (info_->version >= 2 && info_->FFI_DoURIActionWithKeyboardModifier) {
    info_->FFI_DoURIActionWithKeyboardModifier(info_, bsURI.c_str(),
                                               modifiers.UncheckedValue());
    return;
  }

  if (info_->FFI_DoURIAction) {
    info_->FFI_DoURIAction(info_, bsURI.c_str());
  }
}

void CPDFSDK_FormFillEnvironment::DoGoToAction(int nPageIndex,
                                               int zoomMode,
                                               pdfium::span<float> fPosArray) {
  if (info_ && info_->FFI_DoGoToAction) {
    info_->FFI_DoGoToAction(info_, nPageIndex, zoomMode, fPosArray.data(),
                            fxcrt::CollectionSize<int>(fPosArray));
  }
}

#ifdef PDF_ENABLE_XFA
int CPDFSDK_FormFillEnvironment::GetPageViewCount() const {
  return fxcrt::CollectionSize<int>(page_map_);
}

void CPDFSDK_FormFillEnvironment::DisplayCaret(IPDF_Page* page,
                                               FPDF_BOOL bVisible,
                                               double left,
                                               double top,
                                               double right,
                                               double bottom) {
  if (info_ && info_->version >= 2 && info_->FFI_DisplayCaret) {
    info_->FFI_DisplayCaret(info_, FPDFPageFromIPDFPage(page), bVisible, left,
                            top, right, bottom);
  }
}

int CPDFSDK_FormFillEnvironment::GetCurrentPageIndex() const {
  if (!info_ || info_->version < 2 || !info_->FFI_GetCurrentPageIndex) {
    return -1;
  }
  return info_->FFI_GetCurrentPageIndex(info_,
                                        FPDFDocumentFromCPDFDocument(cpdfdoc_));
}

void CPDFSDK_FormFillEnvironment::SetCurrentPage(int iCurPage) {
  if (!info_ || info_->version < 2 || !info_->FFI_SetCurrentPage) {
    return;
  }
  info_->FFI_SetCurrentPage(info_, FPDFDocumentFromCPDFDocument(cpdfdoc_),
                            iCurPage);
}

void CPDFSDK_FormFillEnvironment::GotoURL(const WideString& wsURL) {
  if (!info_ || info_->version < 2 || !info_->FFI_GotoURL) {
    return;
  }

  ByteString bsTo = wsURL.ToUTF16LE();
  info_->FFI_GotoURL(info_, FPDFDocumentFromCPDFDocument(cpdfdoc_),
                     AsFPDFWideString(&bsTo));
}

FS_RECTF CPDFSDK_FormFillEnvironment::GetPageViewRect(IPDF_Page* page) {
  FS_RECTF rect = {0.0f, 0.0f, 0.0f, 0.0f};
  if (!info_ || info_->version < 2 || !info_->FFI_GetPageViewRect) {
    return rect;
  }

  double left;
  double top;
  double right;
  double bottom;
  info_->FFI_GetPageViewRect(info_, FPDFPageFromIPDFPage(page), &left, &top,
                             &right, &bottom);

  rect.left = static_cast<float>(left);
  rect.top = static_cast<float>(top);
  rect.bottom = static_cast<float>(bottom);
  rect.right = static_cast<float>(right);
  return rect;
}

bool CPDFSDK_FormFillEnvironment::PopupMenu(IPDF_Page* page,
                                            int menuFlag,
                                            const CFX_PointF& pt) {
  return info_ && info_->version >= 2 && info_->FFI_PopupMenu &&
         info_->FFI_PopupMenu(info_, FPDFPageFromIPDFPage(page), nullptr,
                              menuFlag, pt.x, pt.y);
}

void CPDFSDK_FormFillEnvironment::EmailTo(FPDF_FILEHANDLER* fileHandler,
                                          FPDF_WIDESTRING pTo,
                                          FPDF_WIDESTRING pSubject,
                                          FPDF_WIDESTRING pCC,
                                          FPDF_WIDESTRING pBcc,
                                          FPDF_WIDESTRING pMsg) {
  if (info_ && info_->version >= 2 && info_->FFI_EmailTo) {
    info_->FFI_EmailTo(info_, fileHandler, pTo, pSubject, pCC, pBcc, pMsg);
  }
}

void CPDFSDK_FormFillEnvironment::UploadTo(FPDF_FILEHANDLER* fileHandler,
                                           int fileFlag,
                                           FPDF_WIDESTRING uploadTo) {
  if (info_ && info_->version >= 2 && info_->FFI_UploadTo) {
    info_->FFI_UploadTo(info_, fileHandler, fileFlag, uploadTo);
  }
}

FPDF_FILEHANDLER* CPDFSDK_FormFillEnvironment::OpenFile(int fileType,
                                                        FPDF_WIDESTRING wsURL,
                                                        const char* mode) {
  if (info_ && info_->version >= 2 && info_->FFI_OpenFile) {
    return info_->FFI_OpenFile(info_, fileType, wsURL, mode);
  }
  return nullptr;
}

RetainPtr<IFX_SeekableReadStream> CPDFSDK_FormFillEnvironment::DownloadFromURL(
    const WideString& url) {
  if (!info_ || info_->version < 2 || !info_->FFI_DownloadFromURL) {
    return nullptr;
  }

  ByteString bstrURL = url.ToUTF16LE();
  FPDF_FILEHANDLER* file_handler =
      info_->FFI_DownloadFromURL(info_, AsFPDFWideString(&bstrURL));
  if (!file_handler) {
    return nullptr;
  }

  return MakeSeekableStream(file_handler);
}

WideString CPDFSDK_FormFillEnvironment::PostRequestURL(
    const WideString& wsURL,
    const WideString& wsData,
    const WideString& wsContentType,
    const WideString& wsEncode,
    const WideString& wsHeader) {
  if (!info_ || info_->version < 2 || !info_->FFI_PostRequestURL) {
    return WideString();
  }

  ByteString bsURL = wsURL.ToUTF16LE();
  ByteString bsData = wsData.ToUTF16LE();
  ByteString bsContentType = wsContentType.ToUTF16LE();
  ByteString bsEncode = wsEncode.ToUTF16LE();
  ByteString bsHeader = wsHeader.ToUTF16LE();

  FPDF_BSTR response;
  FPDF_BStr_Init(&response);
  info_->FFI_PostRequestURL(
      info_, AsFPDFWideString(&bsURL), AsFPDFWideString(&bsData),
      AsFPDFWideString(&bsContentType), AsFPDFWideString(&bsEncode),
      AsFPDFWideString(&bsHeader), &response);

  // SAFETY: required from FFI callback.
  WideString wsRet = WideString::FromUTF16LE(UNSAFE_BUFFERS(
      pdfium::span(reinterpret_cast<const uint8_t*>(response.str),
                   static_cast<size_t>(response.len))));

  FPDF_BStr_Clear(&response);
  return wsRet;
}

FPDF_BOOL CPDFSDK_FormFillEnvironment::PutRequestURL(
    const WideString& wsURL,
    const WideString& wsData,
    const WideString& wsEncode) {
  if (!info_ || info_->version < 2 || !info_->FFI_PutRequestURL) {
    return false;
  }

  ByteString bsURL = wsURL.ToUTF16LE();
  ByteString bsData = wsData.ToUTF16LE();
  ByteString bsEncode = wsEncode.ToUTF16LE();

  return info_->FFI_PutRequestURL(info_, AsFPDFWideString(&bsURL),
                                  AsFPDFWideString(&bsData),
                                  AsFPDFWideString(&bsEncode));
}

void CPDFSDK_FormFillEnvironment::PageEvent(int iPageCount,
                                            uint32_t dwEventType) const {
  if (info_ && info_->version >= 2 && info_->FFI_PageEvent) {
    info_->FFI_PageEvent(info_, iPageCount, dwEventType);
  }
}
#endif  // PDF_ENABLE_XFA

void CPDFSDK_FormFillEnvironment::ClearAllFocusedAnnots() {
  for (auto& it : page_map_) {
    if (it.second->IsValidSDKAnnot(GetFocusAnnot())) {
      ObservedPtr<CPDFSDK_PageView> pObserved(it.second.get());
      KillFocusAnnot({});
      if (!pObserved) {
        break;
      }
    }
  }
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetOrCreatePageView(
    IPDF_Page* pUnderlyingPage) {
  CPDFSDK_PageView* pExisting = GetPageView(pUnderlyingPage);
  if (pExisting) {
    return pExisting;
  }

  auto pNew = std::make_unique<CPDFSDK_PageView>(this, pUnderlyingPage);
  CPDFSDK_PageView* pPageView = pNew.get();
  page_map_[pUnderlyingPage] = std::move(pNew);

  // Delay to load all the annotations, to avoid endless loop.
  pPageView->LoadFXAnnots();
  return pPageView;
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetPageView(
    IPDF_Page* pUnderlyingPage) {
  auto it = page_map_.find(pUnderlyingPage);
  return it != page_map_.end() ? it->second.get() : nullptr;
}

CFX_Timer::HandlerIface* CPDFSDK_FormFillEnvironment::GetTimerHandler() {
  return this;
}

CPDFSDK_PageView* CPDFSDK_FormFillEnvironment::GetPageViewAtIndex(int nIndex) {
  IPDF_Page* pTempPage = GetPage(nIndex);
  return pTempPage ? GetPageView(pTempPage) : nullptr;
}

void CPDFSDK_FormFillEnvironment::ProcJavascriptAction() {
  auto name_tree = CPDF_NameTree::Create(cpdfdoc_, "JavaScript");
  if (!name_tree) {
    return;
  }

  size_t count = name_tree->GetCount();
  for (size_t i = 0; i < count; ++i) {
    WideString name;
    CPDF_Action action(ToDictionary(name_tree->LookupValueAndName(i, &name)));
    DoActionJavaScript(action, name);
  }
}

bool CPDFSDK_FormFillEnvironment::ProcOpenAction() {
  const CPDF_Dictionary* pRoot = cpdfdoc_->GetRoot();
  if (!pRoot) {
    return false;
  }

  RetainPtr<const CPDF_Object> pOpenAction(pRoot->GetDictFor("OpenAction"));
  if (!pOpenAction) {
    pOpenAction = pRoot->GetArrayFor("OpenAction");
  }
  if (!pOpenAction) {
    return false;
  }

  if (pOpenAction->IsArray()) {
    return true;
  }

  RetainPtr<const CPDF_Dictionary> pDict = ToDictionary(pOpenAction);
  if (!pDict) {
    return false;
  }

  DoActionDocOpen(CPDF_Action(std::move(pDict)));
  return true;
}

void CPDFSDK_FormFillEnvironment::RemovePageView(IPDF_Page* pUnderlyingPage) {
  auto it = page_map_.find(pUnderlyingPage);
  if (it == page_map_.end()) {
    return;
  }

  CPDFSDK_PageView* pPageView = it->second.get();
  if (pPageView->IsLocked() || pPageView->IsBeingDestroyed()) {
    return;
  }

  // Mark the page view so we do not come into |RemovePageView| a second
  // time while we're in the process of removing.
  pPageView->SetBeingDestroyed();

  // This must happen before we remove |pPageView| from the map because
  // |KillFocusAnnot| can call into the |GetPage| method which will
  // look for this page view in the map, if it doesn't find it a new one will
  // be created. We then have two page views pointing to the same page and
  // bad things happen.
  if (pPageView->IsValidSDKAnnot(GetFocusAnnot())) {
    KillFocusAnnot({});
  }

  // Remove the page from the map to make sure we don't accidentally attempt
  // to use the |pPageView| while we're cleaning it up.
  page_map_.erase(it);
}

IPDF_Page* CPDFSDK_FormFillEnvironment::GetPage(int nIndex) const {
  if (!info_ || !info_->FFI_GetPage) {
    return nullptr;
  }
  return IPDFPageFromFPDFPage(info_->FFI_GetPage(
      info_, FPDFDocumentFromCPDFDocument(cpdfdoc_), nIndex));
}

CPDFSDK_InteractiveForm* CPDFSDK_FormFillEnvironment::GetInteractiveForm() {
  if (!interactive_form_) {
    interactive_form_ = std::make_unique<CPDFSDK_InteractiveForm>(this);
  }
  return interactive_form_.get();
}

void CPDFSDK_FormFillEnvironment::UpdateAllViews(CPDFSDK_Annot* pAnnot) {
  for (const auto& it : page_map_) {
    ObservedPtr<CPDFSDK_PageView> pObserved(it.second.get());
    if (pObserved) {
      pObserved->UpdateView(pAnnot);
      if (!pObserved) {
        break;
      }
    }
  }
}

CPDFSDK_Annot* CPDFSDK_FormFillEnvironment::GetFocusAnnot() const {
  return focus_annot_.Get();
}

bool CPDFSDK_FormFillEnvironment::SetFocusAnnot(
    ObservedPtr<CPDFSDK_Annot>& pAnnot) {
  if (being_destroyed_) {
    return false;
  }
  if (focus_annot_ == pAnnot) {
    return true;
  }
  if (focus_annot_ && !KillFocusAnnot({})) {
    return false;
  }
  if (!pAnnot) {
    return false;
  }
  if (!pAnnot->GetPageView()->IsValid()) {
    return false;
  }

  if (focus_annot_) {
    return false;
  }

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Widget* pXFAWidget = pAnnot->AsXFAWidget();
  if (pXFAWidget && pXFAWidget->OnChangedFocus()) {
    return false;
  }

  // `pAnnot` may be destroyed in `OnChangedFocus()`.
  if (!pAnnot) {
    return false;
  }
#endif  // PDF_ENABLE_XFA

  if (!CPDFSDK_Annot::OnSetFocus(pAnnot, {})) {
    return false;
  }
  if (focus_annot_) {
    return false;
  }
  focus_annot_ = pAnnot;

  // If we are not able to inform the client about the focus change, it
  // shouldn't be considered as failure.
  SendOnFocusChange(pAnnot);
  return true;
}

bool CPDFSDK_FormFillEnvironment::KillFocusAnnot(Mask<FWL_EVENTFLAG> nFlags) {
  if (!focus_annot_) {
    return false;
  }

  ObservedPtr<CPDFSDK_Annot> pFocusAnnot(focus_annot_.Get());
  focus_annot_.Reset();

  if (!CPDFSDK_Annot::OnKillFocus(pFocusAnnot, nFlags)) {
    focus_annot_ = pFocusAnnot;
    return false;
  }

  // Might have been destroyed by OnKillFocus().
  if (!pFocusAnnot) {
    return false;
  }

  if (pFocusAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET) {
    const FormFieldType field_type =
        ToCPDFSDKWidget(pFocusAnnot.Get())->GetFieldType();
    if (field_type == FormFieldType::kTextField ||
        field_type == FormFieldType::kComboBox) {
      OnSetFieldInputFocusInternal(WideString(), false);
    }
  }
  return !focus_annot_;
}

int CPDFSDK_FormFillEnvironment::GetPageCount() const {
  CPDF_Document::Extension* pExtension = cpdfdoc_->GetExtension();
  return pExtension ? pExtension->GetPageCount() : cpdfdoc_->GetPageCount();
}

bool CPDFSDK_FormFillEnvironment::HasPermissions(uint32_t flags) const {
  return !!(cpdfdoc_->GetUserPermissions(/*get_owner_perms=*/true) & flags);
}

void CPDFSDK_FormFillEnvironment::SendOnFocusChange(
    ObservedPtr<CPDFSDK_Annot>& pAnnot) {
  if (!info_ || info_->version < 2 || !info_->FFI_OnFocusChange) {
    return;
  }

  // TODO(crbug.com/pdfium/1482): Handle XFA case.
  if (pAnnot->AsXFAWidget()) {
    return;
  }

  CPDFSDK_PageView* pPageView = pAnnot->GetPageView();
  if (!pPageView->IsValid()) {
    return;
  }

  IPDF_Page* page = pAnnot->GetPage();
  if (!page) {
    return;
  }

  RetainPtr<CPDF_Dictionary> annot_dict =
      pAnnot->GetPDFAnnot()->GetMutableAnnotDict();
  auto focused_annot = std::make_unique<CPDF_AnnotContext>(annot_dict, page);
  FPDF_ANNOTATION fpdf_annot =
      FPDFAnnotationFromCPDFAnnotContext(focused_annot.get());

  info_->FFI_OnFocusChange(info_, fpdf_annot, pPageView->GetPageIndex());
}

bool CPDFSDK_FormFillEnvironment::DoActionDocOpen(const CPDF_Action& action) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentOpenAction(action, &visited);
}

bool CPDFSDK_FormFillEnvironment::DoActionJavaScript(
    const CPDF_Action& JsAction,
    WideString csJSName) {
  if (JsAction.GetType() == CPDF_Action::Type::kJavaScript) {
    WideString swJS = JsAction.GetJavaScript();
    if (!swJS.IsEmpty()) {
      RunDocumentOpenJavaScript(csJSName, swJS);
      return true;
    }
  }

  return false;
}

void CPDFSDK_FormFillEnvironment::DoActionFieldJavaScript(
    const CPDF_Action& JsAction,
    CPDF_AAction::AActionType type,
    CPDF_FormField* pFormField,
    CFFL_FieldAction* data) {
  if (IsJSPlatformPresent() &&
      JsAction.GetType() == CPDF_Action::Type::kJavaScript) {
    WideString swJS = JsAction.GetJavaScript();
    if (!swJS.IsEmpty()) {
      RunFieldJavaScript(pFormField, type, data, swJS);
    }
  }
}

bool CPDFSDK_FormFillEnvironment::DoActionLink(const CPDF_Action& action,
                                               CPDF_AAction::AActionType type,
                                               Mask<FWL_EVENTFLAG> modifiers) {
  if (!CPDF_AAction::IsUserInput(type)) {
    return false;
  }

  switch (action.GetType()) {
    case CPDF_Action::Type::kGoTo:
      DoActionGoTo(action);
      return true;
    case CPDF_Action::Type::kURI:
      DoActionURI(action, modifiers);
      return true;
    default:
      return false;
  }
}

bool CPDFSDK_FormFillEnvironment::DoActionDestination(const CPDF_Dest& dest) {
  CPDF_Document* document = GetPDFDocument();
  DCHECK(document);

  std::vector<float> positions = dest.GetScrollPositionArray();
  DoGoToAction(dest.GetDestPageIndex(document), dest.GetZoomMode(), positions);
  return true;
}

bool CPDFSDK_FormFillEnvironment::DoActionPage(
    const CPDF_Action& action,
    CPDF_AAction::AActionType eType) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentPageAction(action, eType, &visited);
}

bool CPDFSDK_FormFillEnvironment::DoActionDocument(
    const CPDF_Action& action,
    CPDF_AAction::AActionType eType) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteDocumentPageAction(action, eType, &visited);
}

bool CPDFSDK_FormFillEnvironment::DoActionField(const CPDF_Action& action,
                                                CPDF_AAction::AActionType type,
                                                CPDF_FormField* pFormField,
                                                CFFL_FieldAction* data) {
  std::set<const CPDF_Dictionary*> visited;
  return ExecuteFieldAction(action, type, pFormField, data, &visited);
}

bool CPDFSDK_FormFillEnvironment::ExecuteDocumentOpenAction(
    const CPDF_Action& action,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict)) {
    return false;
  }

  visited->insert(pDict);

  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty()) {
        RunDocumentOpenJavaScript(WideString(), swJS);
      }
    }
  } else {
    DoActionNoJs(action, CPDF_AAction::AActionType::kDocumentOpen);
  }

  for (size_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteDocumentOpenAction(subaction, visited)) {
      return false;
    }
  }

  return true;
}

bool CPDFSDK_FormFillEnvironment::ExecuteDocumentPageAction(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict)) {
    return false;
  }

  visited->insert(pDict);

  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty()) {
        RunDocumentPageJavaScript(type, swJS);
      }
    }
  } else {
    DoActionNoJs(action, type);
  }

  for (size_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteDocumentPageAction(subaction, type, visited)) {
      return false;
    }
  }

  return true;
}

bool CPDFSDK_FormFillEnvironment::IsValidField(
    const CPDF_Dictionary* pFieldDict) {
  DCHECK(pFieldDict);

  CPDFSDK_InteractiveForm* pForm = GetInteractiveForm();
  CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
  return !!pPDFForm->GetFieldByDict(pFieldDict);
}

bool CPDFSDK_FormFillEnvironment::ExecuteFieldAction(
    const CPDF_Action& action,
    CPDF_AAction::AActionType type,
    CPDF_FormField* pFormField,
    CFFL_FieldAction* data,
    std::set<const CPDF_Dictionary*>* visited) {
  const CPDF_Dictionary* pDict = action.GetDict();
  if (pdfium::Contains(*visited, pDict)) {
    return false;
  }

  visited->insert(pDict);

  if (action.GetType() == CPDF_Action::Type::kJavaScript) {
    if (IsJSPlatformPresent()) {
      WideString swJS = action.GetJavaScript();
      if (!swJS.IsEmpty()) {
        RunFieldJavaScript(pFormField, type, data, swJS);
        if (!IsValidField(pFormField->GetFieldDict())) {
          return false;
        }
      }
    }
  } else {
    DoActionNoJs(action, type);
  }

  for (size_t i = 0, sz = action.GetSubActionsCount(); i < sz; i++) {
    CPDF_Action subaction = action.GetSubAction(i);
    if (!ExecuteFieldAction(subaction, type, pFormField, data, visited)) {
      return false;
    }
  }

  return true;
}

void CPDFSDK_FormFillEnvironment::DoActionNoJs(const CPDF_Action& action,
                                               CPDF_AAction::AActionType type) {
  switch (action.GetType()) {
    case CPDF_Action::Type::kGoTo:
      DoActionGoTo(action);
      break;
    case CPDF_Action::Type::kURI:
      if (CPDF_AAction::IsUserInput(type)) {
        DoActionURI(action, Mask<FWL_EVENTFLAG>{});
      }
      break;
    case CPDF_Action::Type::kHide:
      DoActionHide(action);
      break;
    case CPDF_Action::Type::kNamed:
      DoActionNamed(action);
      break;
    case CPDF_Action::Type::kSubmitForm:
      if (CPDF_AAction::IsUserInput(type)) {
        DoActionSubmitForm(action);
      }
      break;
    case CPDF_Action::Type::kResetForm:
      DoActionResetForm(action);
      break;
    case CPDF_Action::Type::kJavaScript:
      NOTREACHED();
    case CPDF_Action::Type::kSetOCGState:
    case CPDF_Action::Type::kThread:
    case CPDF_Action::Type::kSound:
    case CPDF_Action::Type::kMovie:
    case CPDF_Action::Type::kRendition:
    case CPDF_Action::Type::kTrans:
    case CPDF_Action::Type::kGoTo3DView:
    case CPDF_Action::Type::kGoToR:
    case CPDF_Action::Type::kGoToE:
    case CPDF_Action::Type::kLaunch:
    case CPDF_Action::Type::kImportData:
      // Unimplemented
      break;
    default:
      break;
  }
}

void CPDFSDK_FormFillEnvironment::DoActionGoTo(const CPDF_Action& action) {
  DCHECK(action.GetDict());

  CPDF_Document* pPDFDocument = GetPDFDocument();
  DCHECK(pPDFDocument);

  CPDF_Dest MyDest = action.GetDest(pPDFDocument);
  DoActionDestination(MyDest);
}

void CPDFSDK_FormFillEnvironment::DoActionURI(const CPDF_Action& action,
                                              Mask<FWL_EVENTFLAG> modifiers) {
  DCHECK(action.GetDict());
  DoURIAction(action.GetURI(GetPDFDocument()), modifiers);
}

void CPDFSDK_FormFillEnvironment::DoActionNamed(const CPDF_Action& action) {
  DCHECK(action.GetDict());
  ExecuteNamedAction(action.GetNamedAction());
}

void CPDFSDK_FormFillEnvironment::RunFieldJavaScript(
    CPDF_FormField* pFormField,
    CPDF_AAction::AActionType type,
    CFFL_FieldAction* data,
    const WideString& script) {
  DCHECK(type != CPDF_AAction::kCalculate);
  DCHECK(type != CPDF_AAction::kFormat);

  RunScript(script, [type, data, pFormField](IJS_EventContext* context) {
    switch (type) {
      case CPDF_AAction::kCursorEnter:
        context->OnField_MouseEnter(data->bModifier, data->bShift, pFormField);
        break;
      case CPDF_AAction::kCursorExit:
        context->OnField_MouseExit(data->bModifier, data->bShift, pFormField);
        break;
      case CPDF_AAction::kButtonDown:
        context->OnField_MouseDown(data->bModifier, data->bShift, pFormField);
        break;
      case CPDF_AAction::kButtonUp:
        context->OnField_MouseUp(data->bModifier, data->bShift, pFormField);
        break;
      case CPDF_AAction::kGetFocus:
        context->OnField_Focus(data->bModifier, data->bShift, pFormField,
                               &data->sValue);
        break;
      case CPDF_AAction::kLoseFocus:
        context->OnField_Blur(data->bModifier, data->bShift, pFormField,
                              &data->sValue);
        break;
      case CPDF_AAction::kKeyStroke:
        context->OnField_Keystroke(
            &data->sChange, data->sChangeEx, data->bKeyDown, data->bModifier,
            &data->nSelEnd, &data->nSelStart, data->bShift, pFormField,
            &data->sValue, data->bWillCommit, data->bFieldFull, &data->bRC);
        break;
      case CPDF_AAction::kValidate:
        context->OnField_Validate(&data->sChange, data->sChangeEx,
                                  data->bKeyDown, data->bModifier, data->bShift,
                                  pFormField, &data->sValue, &data->bRC);
        break;
      default:
        NOTREACHED();
    }
  });
}

void CPDFSDK_FormFillEnvironment::RunDocumentOpenJavaScript(
    const WideString& sScriptName,
    const WideString& script) {
  RunScript(script, [sScriptName](IJS_EventContext* context) {
    context->OnDoc_Open(sScriptName);
  });
}

void CPDFSDK_FormFillEnvironment::RunDocumentPageJavaScript(
    CPDF_AAction::AActionType type,
    const WideString& script) {
  RunScript(script, [type](IJS_EventContext* context) {
    switch (type) {
      case CPDF_AAction::kOpenPage:
        context->OnPage_Open();
        break;
      case CPDF_AAction::kClosePage:
        context->OnPage_Close();
        break;
      case CPDF_AAction::kCloseDocument:
        context->OnDoc_WillClose();
        break;
      case CPDF_AAction::kSaveDocument:
        context->OnDoc_WillSave();
        break;
      case CPDF_AAction::kDocumentSaved:
        context->OnDoc_DidSave();
        break;
      case CPDF_AAction::kPrintDocument:
        context->OnDoc_WillPrint();
        break;
      case CPDF_AAction::kDocumentPrinted:
        context->OnDoc_DidPrint();
        break;
      case CPDF_AAction::kPageVisible:
        context->OnPage_InView();
        break;
      case CPDF_AAction::kPageInvisible:
        context->OnPage_OutView();
        break;
      default:
        NOTREACHED();
    }
  });
}

bool CPDFSDK_FormFillEnvironment::DoActionHide(const CPDF_Action& action) {
  CPDFSDK_InteractiveForm* pForm = GetInteractiveForm();
  if (pForm->DoAction_Hide(action)) {
    SetChangeMark();
    return true;
  }
  return false;
}

bool CPDFSDK_FormFillEnvironment::DoActionSubmitForm(
    const CPDF_Action& action) {
  CPDFSDK_InteractiveForm* pForm = GetInteractiveForm();
  return pForm->DoAction_SubmitForm(action);
}

void CPDFSDK_FormFillEnvironment::DoActionResetForm(const CPDF_Action& action) {
  CPDFSDK_InteractiveForm* pForm = GetInteractiveForm();
  pForm->DoAction_ResetForm(action);
}

void CPDFSDK_FormFillEnvironment::RunScript(const WideString& script,
                                            const RunScriptCallback& cb) {
  IJS_Runtime::ScopedEventContext pContext(GetIJSRuntime());
  cb(pContext.Get());
  pContext->RunScript(script);
  // TODO(dsinclair): Return error if RunScript returns a IJS_Runtime::JS_Error.
}
