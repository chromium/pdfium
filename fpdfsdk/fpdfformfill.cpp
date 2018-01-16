// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_formfill.h"

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfdoc/cpdf_formcontrol.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fpdfdoc/cpdf_interform.h"
#include "core/fpdfdoc/cpdf_occontext.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_interform.h"
#include "fpdfsdk/cpdfsdk_pageview.h"
#include "fpdfsdk/fsdk_actionhandler.h"
#include "fpdfsdk/fsdk_define.h"
#include "public/fpdfview.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_context.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"

static_assert(static_cast<int>(FormType::kNone) == FORMTYPE_NONE,
              "None form types must match");
static_assert(static_cast<int>(FormType::kAcroForm) == FORMTYPE_ACRO_FORM,
              "AcroForm form types must match");
static_assert(static_cast<int>(FormType::kXFAFull) == FORMTYPE_XFA_FULL,
              "XFA full form types must match");
static_assert(static_cast<int>(FormType::kXFAForeground) ==
                  FORMTYPE_XFA_FOREGROUND,
              "XFA foreground form types must match");
#endif  // PDF_ENABLE_XFA

static_assert(static_cast<int>(FormFieldType::kUnknown) ==
                  FPDF_FORMFIELD_UNKNOWN,
              "Unknown form field types must match");
static_assert(static_cast<int>(FormFieldType::kPushButton) ==
                  FPDF_FORMFIELD_PUSHBUTTON,
              "PushButton form field types must match");
static_assert(static_cast<int>(FormFieldType::kCheckBox) ==
                  FPDF_FORMFIELD_CHECKBOX,
              "CheckBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kRadioButton) ==
                  FPDF_FORMFIELD_RADIOBUTTON,
              "RadioButton form field types must match");
static_assert(static_cast<int>(FormFieldType::kComboBox) ==
                  FPDF_FORMFIELD_COMBOBOX,
              "ComboBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kListBox) ==
                  FPDF_FORMFIELD_LISTBOX,
              "ListBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kTextField) ==
                  FPDF_FORMFIELD_TEXTFIELD,
              "TextField form field types must match");
static_assert(static_cast<int>(FormFieldType::kSignature) ==
                  FPDF_FORMFIELD_SIGNATURE,
              "Signature form field types must match");
#ifdef PDF_ENABLE_XFA
static_assert(static_cast<int>(FormFieldType::kXFA) == FPDF_FORMFIELD_XFA,
              "XFA form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_CheckBox) ==
                  FPDF_FORMFIELD_XFA_CHECKBOX,
              "XFA CheckBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_ComboBox) ==
                  FPDF_FORMFIELD_XFA_COMBOBOX,
              "XFA ComboBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_ImageField) ==
                  FPDF_FORMFIELD_XFA_IMAGEFIELD,
              "XFA ImageField form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_ListBox) ==
                  FPDF_FORMFIELD_XFA_LISTBOX,
              "XFA ListBox form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_PushButton) ==
                  FPDF_FORMFIELD_XFA_PUSHBUTTON,
              "XFA PushButton form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_Signature) ==
                  FPDF_FORMFIELD_XFA_SIGNATURE,
              "XFA Signature form field types must match");
static_assert(static_cast<int>(FormFieldType::kXFA_TextField) ==
                  FPDF_FORMFIELD_XFA_TEXTFIELD,
              "XFA TextField form field types must match");
#endif  // PDF_ENABLE_XFA
static_assert(kFormFieldTypeCount == FPDF_FORMFIELD_COUNT,
              "Number of form field types must match");

namespace {

CPDFSDK_FormFillEnvironment* HandleToCPDFSDKEnvironment(
    FPDF_FORMHANDLE handle) {
  return static_cast<CPDFSDK_FormFillEnvironment*>(handle);
}

CPDFSDK_InterForm* FormHandleToInterForm(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  return pFormFillEnv ? pFormFillEnv->GetInterForm() : nullptr;
}

CPDFSDK_PageView* FormHandleToPageView(FPDF_FORMHANDLE hHandle,
                                       FPDF_PAGE page) {
  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return nullptr;

  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  return pFormFillEnv ? pFormFillEnv->GetPageView(pPage, true) : nullptr;
}

#ifdef PDF_ENABLE_XFA
std::vector<ByteString>* FromFPDFStringHandle(FPDF_STRINGHANDLE handle) {
  return static_cast<std::vector<ByteString>*>(handle);
}

FPDF_STRINGHANDLE ToFPDFStringHandle(std::vector<ByteString>* strings) {
  return static_cast<FPDF_STRINGHANDLE>(strings);
}
#endif  // PDF_ENABLE_XFA

void FFLCommon(FPDF_FORMHANDLE hHandle,
               FPDF_BITMAP bitmap,
               FPDF_RECORDER recorder,
               FPDF_PAGE page,
               int start_x,
               int start_y,
               int size_x,
               int size_y,
               int rotate,
               int flags) {
  if (!hHandle)
    return;

  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return;

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Context* pContext = pPage->GetContext();
  if (!pContext)
    return;
  CPDF_Document* pPDFDoc = pContext->GetPDFDoc();
  if (!pPDFDoc)
    return;
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return;
#endif  // PDF_ENABLE_XFA

  CFX_Matrix matrix =
      pPage->GetDisplayMatrix(start_x, start_y, size_x, size_y, rotate);
  FX_RECT clip(start_x, start_y, start_x + size_x, start_y + size_y);

  auto pDevice = pdfium::MakeUnique<CFX_DefaultRenderDevice>();
#ifdef _SKIA_SUPPORT_
  pDevice->AttachRecorder(static_cast<SkPictureRecorder*>(recorder));
#endif
  RetainPtr<CFX_DIBitmap> holder(CFXBitmapFromFPDFBitmap(bitmap));
  pDevice->Attach(holder, false, nullptr, false);
  {
    CFX_RenderDevice::StateRestorer restorer(pDevice.get());
    pDevice->SetClip_Rect(clip);

    CPDF_RenderOptions options;
    uint32_t option_flags = options.GetFlags();
    if (flags & FPDF_LCD_TEXT)
      option_flags |= RENDER_CLEARTYPE;
    else
      option_flags &= ~RENDER_CLEARTYPE;
    options.SetFlags(option_flags);

    // Grayscale output
    if (flags & FPDF_GRAYSCALE)
      options.SetColorMode(CPDF_RenderOptions::kGray);

    options.SetDrawAnnots(flags & FPDF_ANNOT);

#ifdef PDF_ENABLE_XFA
    options.SetOCContext(
        pdfium::MakeRetain<CPDF_OCContext>(pPDFDoc, CPDF_OCContext::View));
    if (CPDFSDK_PageView* pPageView = pFormFillEnv->GetPageView(pPage, true))
      pPageView->PageView_OnDraw(pDevice.get(), &matrix, &options, clip);
#else   // PDF_ENABLE_XFA
    options.SetOCContext(pdfium::MakeRetain<CPDF_OCContext>(
        pPage->m_pDocument.Get(), CPDF_OCContext::View));
    if (CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, pPage))
      pPageView->PageView_OnDraw(pDevice.get(), &matrix, &options);
#endif  // PDF_ENABLE_XFA
  }
#ifdef _SKIA_SUPPORT_PATHS_
  pDevice->Flush(true);
  holder->UnPreMultiply();
#endif
}

}  // namespace

FPDF_EXPORT int FPDF_CALLCONV
FPDFPage_HasFormFieldAtPoint(FPDF_FORMHANDLE hHandle,
                             FPDF_PAGE page,
                             double page_x,
                             double page_y) {
  if (!hHandle)
    return -1;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (pPage) {
    CPDF_InterForm interform(pPage->m_pDocument.Get());
    CPDF_FormControl* pFormCtrl = interform.GetControlAtPoint(
        pPage,
        CFX_PointF(static_cast<float>(page_x), static_cast<float>(page_y)),
        nullptr);
    if (!pFormCtrl)
      return -1;
    CPDF_FormField* pFormField = pFormCtrl->GetField();
    return pFormField ? static_cast<int>(pFormField->GetFieldType()) : -1;
  }

#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = UnderlyingFromFPDFPage(page);
  if (!pXFAPage)
    return -1;

  CXFA_FFPageView* pPageView = pXFAPage->GetXFAPageView();
  if (!pPageView)
    return -1;

  CXFA_FFDocView* pDocView = pPageView->GetDocView();
  if (!pDocView)
    return -1;

  CXFA_FFWidgetHandler* pWidgetHandler = pDocView->GetWidgetHandler();
  if (!pWidgetHandler)
    return -1;

  std::unique_ptr<IXFA_WidgetIterator> pWidgetIterator(
      pPageView->CreateWidgetIterator(XFA_TRAVERSEWAY_Form,
                                      XFA_WidgetStatus_Viewable));
  if (!pWidgetIterator)
    return -1;

  CXFA_FFWidget* pXFAAnnot;
  while ((pXFAAnnot = pWidgetIterator->MoveToNext()) != nullptr) {
    CFX_RectF rcBBox = pXFAAnnot->GetBBox(0);
    CFX_FloatRect rcWidget(rcBBox.left, rcBBox.top, rcBBox.left + rcBBox.width,
                           rcBBox.top + rcBBox.height);
    rcWidget.Inflate(1.0f, 1.0f);
    if (rcWidget.Contains(CFX_PointF(static_cast<float>(page_x),
                                     static_cast<float>(page_y)))) {
      return static_cast<int>(pXFAAnnot->GetFormFieldType());
    }
  }
#endif  // PDF_ENABLE_XFA
  return -1;
}

FPDF_EXPORT int FPDF_CALLCONV
FPDFPage_FormFieldZOrderAtPoint(FPDF_FORMHANDLE hHandle,
                                FPDF_PAGE page,
                                double page_x,
                                double page_y) {
  if (!hHandle)
    return -1;
  CPDF_Page* pPage = CPDFPageFromFPDFPage(page);
  if (!pPage)
    return -1;
  CPDF_InterForm interform(pPage->m_pDocument.Get());
  int z_order = -1;
  (void)interform.GetControlAtPoint(
      pPage, CFX_PointF(static_cast<float>(page_x), static_cast<float>(page_y)),
      &z_order);
  return z_order;
}

FPDF_EXPORT FPDF_FORMHANDLE FPDF_CALLCONV
FPDFDOC_InitFormFillEnvironment(FPDF_DOCUMENT document,
                                FPDF_FORMFILLINFO* formInfo) {
#ifdef PDF_ENABLE_XFA
  const int kRequiredVersion = 2;
#else   // PDF_ENABLE_XFA
  const int kRequiredVersion = 1;
#endif  // PDF_ENABLE_XFA
  if (!formInfo || formInfo->version != kRequiredVersion)
    return nullptr;

  UnderlyingDocumentType* pDocument = UnderlyingFromFPDFDocument(document);
  if (!pDocument)
    return nullptr;

#ifdef PDF_ENABLE_XFA
  // If the CPDFXFA_Context has a FormFillEnvironment already then we've done
  // this and can just return the old Env. Otherwise, we'll end up setting a new
  // environment into the XFADocument and, that could get weird.
  if (pDocument->GetFormFillEnv())
    return pDocument->GetFormFillEnv();
#endif

  auto pFormFillEnv =
      pdfium::MakeUnique<CPDFSDK_FormFillEnvironment>(pDocument, formInfo);

#ifdef PDF_ENABLE_XFA
  pDocument->SetFormFillEnv(pFormFillEnv.get());
#endif  // PDF_ENABLE_XFA

  return pFormFillEnv.release();  // Caller takes ownership.
}

FPDF_EXPORT void FPDF_CALLCONV
FPDFDOC_ExitFormFillEnvironment(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return;

#ifdef PDF_ENABLE_XFA
  // Reset the focused annotations and remove the SDK document from the
  // XFA document.
  pFormFillEnv->ClearAllFocusedAnnots();
  // If the document was closed first, it's possible the XFA document
  // is now a nullptr.
  if (pFormFillEnv->GetXFAContext())
    pFormFillEnv->GetXFAContext()->SetFormFillEnv(nullptr);
#endif  // PDF_ENABLE_XFA
  delete pFormFillEnv;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnMouseMove(FPDF_FORMHANDLE hHandle,
                                                     FPDF_PAGE page,
                                                     int modifier,
                                                     double page_x,
                                                     double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnMouseMove(CFX_PointF(page_x, page_y), modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnFocus(FPDF_FORMHANDLE hHandle,
                                                 FPDF_PAGE page,
                                                 int modifier,
                                                 double page_x,
                                                 double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnFocus(CFX_PointF(page_x, page_y), modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnLButtonDown(FPDF_FORMHANDLE hHandle,
                                                       FPDF_PAGE page,
                                                       int modifier,
                                                       double page_x,
                                                       double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnLButtonDown(CFX_PointF(page_x, page_y), modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnLButtonUp(FPDF_FORMHANDLE hHandle,
                                                     FPDF_PAGE page,
                                                     int modifier,
                                                     double page_x,
                                                     double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnLButtonUp(CFX_PointF(page_x, page_y), modifier);
}

#ifdef PDF_ENABLE_XFA
FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnRButtonDown(FPDF_FORMHANDLE hHandle,
                                                       FPDF_PAGE page,
                                                       int modifier,
                                                       double page_x,
                                                       double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnRButtonDown(CFX_PointF(page_x, page_y), modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnRButtonUp(FPDF_FORMHANDLE hHandle,
                                                     FPDF_PAGE page,
                                                     int modifier,
                                                     double page_x,
                                                     double page_y) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnRButtonUp(CFX_PointF(page_x, page_y), modifier);
}
#endif  // PDF_ENABLE_XFA

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnKeyDown(FPDF_FORMHANDLE hHandle,
                                                   FPDF_PAGE page,
                                                   int nKeyCode,
                                                   int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnKeyDown(nKeyCode, modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnKeyUp(FPDF_FORMHANDLE hHandle,
                                                 FPDF_PAGE page,
                                                 int nKeyCode,
                                                 int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnKeyUp(nKeyCode, modifier);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FORM_OnChar(FPDF_FORMHANDLE hHandle,
                                                FPDF_PAGE page,
                                                int nChar,
                                                int modifier) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return false;
  return pPageView->OnChar(nChar, modifier);
}

FPDF_EXPORT unsigned long FPDF_CALLCONV
FORM_GetSelectedText(FPDF_FORMHANDLE hHandle,
                     FPDF_PAGE page,
                     void* buffer,
                     unsigned long buflen) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return 0;

  WideString wide_str_form_text = pPageView->GetSelectedText();
  ByteString encoded_form_text = wide_str_form_text.UTF16LE_Encode();
  unsigned long form_text_len = encoded_form_text.GetLength();

  if (buffer && buflen >= form_text_len)
    memcpy(buffer, encoded_form_text.c_str(), form_text_len);

  return form_text_len;
}

FPDF_EXPORT void FPDF_CALLCONV FORM_ReplaceSelection(FPDF_FORMHANDLE hHandle,
                                                     FPDF_PAGE page,
                                                     FPDF_WIDESTRING wsText) {
  CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page);
  if (!pPageView)
    return;

  size_t len = WideString::WStringLength(wsText);
  WideString wide_str_text = WideString::FromUTF16LE(wsText, len);

  pPageView->ReplaceSelection(wide_str_text);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FORM_ForceToKillFocus(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return false;
  return pFormFillEnv->KillFocusAnnot(0);
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_FFLDraw(FPDF_FORMHANDLE hHandle,
                                            FPDF_BITMAP bitmap,
                                            FPDF_PAGE page,
                                            int start_x,
                                            int start_y,
                                            int size_x,
                                            int size_y,
                                            int rotate,
                                            int flags) {
  FFLCommon(hHandle, bitmap, nullptr, page, start_x, start_y, size_x, size_y,
            rotate, flags);
}

#ifdef _SKIA_SUPPORT_
FPDF_EXPORT void FPDF_CALLCONV FPDF_FFLRecord(FPDF_FORMHANDLE hHandle,
                                              FPDF_RECORDER recorder,
                                              FPDF_PAGE page,
                                              int start_x,
                                              int start_y,
                                              int size_x,
                                              int size_y,
                                              int rotate,
                                              int flags) {
  FFLCommon(hHandle, nullptr, recorder, page, start_x, start_y, size_x, size_y,
            rotate, flags);
}
#endif

#ifdef PDF_ENABLE_XFA
FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_Undo(FPDF_DOCUMENT document,
                                                FPDF_WIDGET hWidget) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  static_cast<CXFA_FFWidget*>(hWidget)->Undo();
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_Redo(FPDF_DOCUMENT document,
                                                FPDF_WIDGET hWidget) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  static_cast<CXFA_FFWidget*>(hWidget)->Redo();
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_SelectAll(FPDF_DOCUMENT document,
                                                     FPDF_WIDGET hWidget) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  static_cast<CXFA_FFWidget*>(hWidget)->SelectAll();
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_Copy(FPDF_DOCUMENT document,
                                                FPDF_WIDGET hWidget,
                                                FPDF_WIDESTRING wsText,
                                                FPDF_DWORD* size) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  WideString wsCpText =
      static_cast<CXFA_FFWidget*>(hWidget)->Copy().value_or(WideString());

  ByteString bsCpText = wsCpText.UTF16LE_Encode();
  uint32_t len = bsCpText.GetLength() / sizeof(unsigned short);
  if (!wsText) {
    *size = len;
    return;
  }

  uint32_t real_size = len < *size ? len : *size;
  if (real_size > 0) {
    memcpy((void*)wsText,
           bsCpText.GetBuffer(real_size * sizeof(unsigned short)),
           real_size * sizeof(unsigned short));
    bsCpText.ReleaseBuffer(real_size * sizeof(unsigned short));
  }
  *size = real_size;
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_Cut(FPDF_DOCUMENT document,
                                               FPDF_WIDGET hWidget,
                                               FPDF_WIDESTRING wsText,
                                               FPDF_DWORD* size) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  WideString wsCpText =
      static_cast<CXFA_FFWidget*>(hWidget)->Cut().value_or(WideString());

  ByteString bsCpText = wsCpText.UTF16LE_Encode();
  uint32_t len = bsCpText.GetLength() / sizeof(unsigned short);
  if (!wsText) {
    *size = len;
    return;
  }

  uint32_t real_size = len < *size ? len : *size;
  if (real_size > 0) {
    memcpy((void*)wsText,
           bsCpText.GetBuffer(real_size * sizeof(unsigned short)),
           real_size * sizeof(unsigned short));
    bsCpText.ReleaseBuffer(real_size * sizeof(unsigned short));
  }
  *size = real_size;
}

FPDF_EXPORT void FPDF_CALLCONV FPDF_Widget_Paste(FPDF_DOCUMENT document,
                                                 FPDF_WIDGET hWidget,
                                                 FPDF_WIDESTRING wsText,
                                                 FPDF_DWORD size) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  WideString wstr = WideString::FromUTF16LE(wsText, size);
  static_cast<CXFA_FFWidget*>(hWidget)->Paste(wstr);
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_Widget_ReplaceSpellCheckWord(FPDF_DOCUMENT document,
                                  FPDF_WIDGET hWidget,
                                  float x,
                                  float y,
                                  FPDF_BYTESTRING bsText) {
  if (!hWidget || !document)
    return;

  CPDFXFA_Context* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  CFX_PointF ptPopup;
  ptPopup.x = x;
  ptPopup.y = y;
  ByteStringView bs(bsText);
  static_cast<CXFA_FFWidget*>(hWidget)->ReplaceSpellCheckWord(ptPopup, bs);
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_Widget_GetSpellCheckWords(FPDF_DOCUMENT document,
                               FPDF_WIDGET hWidget,
                               float x,
                               float y,
                               FPDF_STRINGHANDLE* stringHandle) {
  if (!hWidget || !document)
    return;

  auto* pContext = static_cast<CPDFXFA_Context*>(document);
  if (!pContext->ContainsXFAForm())
    return;

  CFX_PointF ptPopup;
  ptPopup.x = x;
  ptPopup.y = y;
  auto sSuggestWords = pdfium::MakeUnique<std::vector<ByteString>>();
  static_cast<CXFA_FFWidget*>(hWidget)->GetSuggestWords(ptPopup,
                                                        sSuggestWords.get());

  // Caller takes ownership.
  *stringHandle = ToFPDFStringHandle(sSuggestWords.release());
}

FPDF_EXPORT int FPDF_CALLCONV
FPDF_StringHandleCounts(FPDF_STRINGHANDLE sHandle) {
  std::vector<ByteString>* sSuggestWords = FromFPDFStringHandle(sHandle);
  return sSuggestWords ? pdfium::CollectionSize<int>(*sSuggestWords) : -1;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StringHandleGetStringByIndex(FPDF_STRINGHANDLE sHandle,
                                  int index,
                                  FPDF_BYTESTRING bsText,
                                  FPDF_DWORD* size) {
  if (!sHandle || !size)
    return false;

  int count = FPDF_StringHandleCounts(sHandle);
  if (index < 0 || index >= count)
    return false;

  std::vector<ByteString>* sSuggestWords = FromFPDFStringHandle(sHandle);
  uint32_t len = (*sSuggestWords)[index].GetLength();
  if (!bsText) {
    *size = len;
    return true;
  }

  uint32_t real_size = len < *size ? len : *size;
  if (real_size > 0)
    memcpy((void*)bsText, (*sSuggestWords)[index].c_str(), real_size);
  *size = real_size;
  return true;
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_StringHandleRelease(FPDF_STRINGHANDLE stringHandle) {
  delete FromFPDFStringHandle(stringHandle);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_StringHandleAddString(FPDF_STRINGHANDLE stringHandle,
                           FPDF_BYTESTRING bsText,
                           FPDF_DWORD size) {
  if (!stringHandle || !bsText || size == 0)
    return false;

  FromFPDFStringHandle(stringHandle)->push_back(ByteString(bsText, size));
  return true;
}
#endif  // PDF_ENABLE_XFA

FPDF_EXPORT void FPDF_CALLCONV
FPDF_SetFormFieldHighlightColor(FPDF_FORMHANDLE hHandle,
                                int fieldType,
                                unsigned long color) {
  CPDFSDK_InterForm* interForm = FormHandleToInterForm(hHandle);
  if (!interForm)
    return;

  Optional<FormFieldType> cast_input = IntToFormFieldType(fieldType);
  if (!cast_input)
    return;

  if (cast_input.value() == FormFieldType::kUnknown) {
    interForm->SetAllHighlightColors(color);
  } else {
    interForm->SetHighlightColor(color, cast_input.value());
  }
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_SetFormFieldHighlightAlpha(FPDF_FORMHANDLE hHandle, unsigned char alpha) {
  if (CPDFSDK_InterForm* pInterForm = FormHandleToInterForm(hHandle))
    pInterForm->SetHighlightAlpha(alpha);
}

FPDF_EXPORT void FPDF_CALLCONV
FPDF_RemoveFormFieldHighlight(FPDF_FORMHANDLE hHandle) {
  if (CPDFSDK_InterForm* pInterForm = FormHandleToInterForm(hHandle))
    pInterForm->RemoveAllHighLights();
}

FPDF_EXPORT void FPDF_CALLCONV FORM_OnAfterLoadPage(FPDF_PAGE page,
                                                    FPDF_FORMHANDLE hHandle) {
  if (CPDFSDK_PageView* pPageView = FormHandleToPageView(hHandle, page))
    pPageView->SetValid(true);
}

FPDF_EXPORT void FPDF_CALLCONV FORM_OnBeforeClosePage(FPDF_PAGE page,
                                                      FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return;

  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  if (!pPage)
    return;

  CPDFSDK_PageView* pPageView = pFormFillEnv->GetPageView(pPage, false);
  if (pPageView) {
    pPageView->SetValid(false);
    // RemovePageView() takes care of the delete for us.
    pFormFillEnv->RemovePageView(pPage);
  }
}

FPDF_EXPORT void FPDF_CALLCONV
FORM_DoDocumentJSAction(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (pFormFillEnv && pFormFillEnv->IsJSInitiated())
    pFormFillEnv->ProcJavascriptFun();
}

FPDF_EXPORT void FPDF_CALLCONV
FORM_DoDocumentOpenAction(FPDF_FORMHANDLE hHandle) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (pFormFillEnv && pFormFillEnv->IsJSInitiated())
    pFormFillEnv->ProcOpenAction();
}

FPDF_EXPORT void FPDF_CALLCONV FORM_DoDocumentAAction(FPDF_FORMHANDLE hHandle,
                                                      int aaType) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return;

  CPDF_Document* pDoc = pFormFillEnv->GetPDFDocument();
  const CPDF_Dictionary* pDict = pDoc->GetRoot();
  if (!pDict)
    return;

  CPDF_AAction aa(pDict->GetDictFor("AA"));
  auto type = static_cast<CPDF_AAction::AActionType>(aaType);
  if (aa.ActionExist(type)) {
    CPDF_Action action = aa.GetAction(type);
    CPDFSDK_ActionHandler* pActionHandler =
        HandleToCPDFSDKEnvironment(hHandle)->GetActionHandler();
    pActionHandler->DoAction_Document(action, type, pFormFillEnv);
  }
}

FPDF_EXPORT void FPDF_CALLCONV FORM_DoPageAAction(FPDF_PAGE page,
                                                  FPDF_FORMHANDLE hHandle,
                                                  int aaType) {
  CPDFSDK_FormFillEnvironment* pFormFillEnv =
      HandleToCPDFSDKEnvironment(hHandle);
  if (!pFormFillEnv)
    return;

  UnderlyingPageType* pPage = UnderlyingFromFPDFPage(page);
  CPDF_Page* pPDFPage = CPDFPageFromFPDFPage(page);
  if (!pPDFPage)
    return;

  if (!pFormFillEnv->GetPageView(pPage, false))
    return;

  CPDFSDK_ActionHandler* pActionHandler = pFormFillEnv->GetActionHandler();
  CPDF_Dictionary* pPageDict = pPDFPage->m_pFormDict.Get();
  CPDF_AAction aa(pPageDict->GetDictFor("AA"));
  CPDF_AAction::AActionType type = aaType == FPDFPAGE_AACTION_OPEN
                                       ? CPDF_AAction::OpenPage
                                       : CPDF_AAction::ClosePage;
  if (aa.ActionExist(type)) {
    CPDF_Action action = aa.GetAction(type);
    pActionHandler->DoAction_Page(action, type, pFormFillEnv);
  }
}
