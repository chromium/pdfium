// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_widgetacc.h"

#include <algorithm>
#include <tuple>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "third_party/base/stl_util.h"
#include "xfa/fde/cfde_textout.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fontmgr.h"
#include "xfa/fxfa/cxfa_textlayout.h"
#include "xfa/fxfa/cxfa_textprovider.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_comb.h"
#include "xfa/fxfa/parser/cxfa_decimal.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_format.h"
#include "xfa/fxfa/parser/cxfa_image.h"
#include "xfa/fxfa/parser/cxfa_items.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_picture.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/cxfa_stroke.h"
#include "xfa/fxfa/parser/cxfa_ui.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/xfa_utils.h"

class CXFA_WidgetLayoutData {
 public:
  CXFA_WidgetLayoutData() : m_fWidgetHeight(-1) {}
  virtual ~CXFA_WidgetLayoutData() {}

  float m_fWidgetHeight;
};

namespace {

constexpr uint8_t g_inv_base64[128] = {
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62,  255,
    255, 255, 63,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  255, 255,
    255, 255, 255, 255, 255, 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
    10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,
    25,  255, 255, 255, 255, 255, 255, 26,  27,  28,  29,  30,  31,  32,  33,
    34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51,  255, 255, 255, 255, 255,
};

uint8_t* XFA_RemoveBase64Whitespace(const uint8_t* pStr, int32_t iLen) {
  uint8_t* pCP;
  int32_t i = 0, j = 0;
  if (iLen == 0) {
    iLen = strlen((char*)pStr);
  }
  pCP = FX_Alloc(uint8_t, iLen + 1);
  for (; i < iLen; i++) {
    if ((pStr[i] & 128) == 0) {
      if (g_inv_base64[pStr[i]] != 0xFF || pStr[i] == '=') {
        pCP[j++] = pStr[i];
      }
    }
  }
  pCP[j] = '\0';
  return pCP;
}

int32_t XFA_Base64Decode(const char* pStr, uint8_t* pOutBuffer) {
  if (!pStr) {
    return 0;
  }
  uint8_t* pBuffer =
      XFA_RemoveBase64Whitespace((uint8_t*)pStr, strlen((char*)pStr));
  if (!pBuffer) {
    return 0;
  }
  int32_t iLen = strlen((char*)pBuffer);
  int32_t i = 0, j = 0;
  uint32_t dwLimb = 0;
  for (; i + 3 < iLen; i += 4) {
    if (pBuffer[i] == '=' || pBuffer[i + 1] == '=' || pBuffer[i + 2] == '=' ||
        pBuffer[i + 3] == '=') {
      if (pBuffer[i] == '=' || pBuffer[i + 1] == '=') {
        break;
      }
      if (pBuffer[i + 2] == '=') {
        dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 6) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 1]]);
        pOutBuffer[j] = (uint8_t)(dwLimb >> 4) & 0xFF;
        j++;
      } else {
        dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 12) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 1]] << 6) |
                 ((uint32_t)g_inv_base64[pBuffer[i + 2]]);
        pOutBuffer[j] = (uint8_t)(dwLimb >> 10) & 0xFF;
        pOutBuffer[j + 1] = (uint8_t)(dwLimb >> 2) & 0xFF;
        j += 2;
      }
    } else {
      dwLimb = ((uint32_t)g_inv_base64[pBuffer[i]] << 18) |
               ((uint32_t)g_inv_base64[pBuffer[i + 1]] << 12) |
               ((uint32_t)g_inv_base64[pBuffer[i + 2]] << 6) |
               ((uint32_t)g_inv_base64[pBuffer[i + 3]]);
      pOutBuffer[j] = (uint8_t)(dwLimb >> 16) & 0xff;
      pOutBuffer[j + 1] = (uint8_t)(dwLimb >> 8) & 0xff;
      pOutBuffer[j + 2] = (uint8_t)(dwLimb)&0xff;
      j += 3;
    }
  }
  FX_Free(pBuffer);
  return j;
}

FXCODEC_IMAGE_TYPE XFA_GetImageType(const WideString& wsType) {
  WideString wsContentType(wsType);
  wsContentType.MakeLower();
  if (wsContentType == L"image/jpg")
    return FXCODEC_IMAGE_JPG;
  if (wsContentType == L"image/png")
    return FXCODEC_IMAGE_PNG;
  if (wsContentType == L"image/gif")
    return FXCODEC_IMAGE_GIF;
  if (wsContentType == L"image/bmp")
    return FXCODEC_IMAGE_BMP;
  if (wsContentType == L"image/tif")
    return FXCODEC_IMAGE_TIF;
  return FXCODEC_IMAGE_UNKNOWN;
}

RetainPtr<CFX_DIBitmap> XFA_LoadImageData(CXFA_FFDoc* pDoc,
                                          CXFA_Image* pImage,
                                          bool& bNameImage,
                                          int32_t& iImageXDpi,
                                          int32_t& iImageYDpi) {
  WideString wsHref = pImage->GetHref();
  WideString wsImage = pImage->GetContent();
  if (wsHref.IsEmpty() && wsImage.IsEmpty())
    return nullptr;

  FXCODEC_IMAGE_TYPE type = XFA_GetImageType(pImage->GetContentType());
  ByteString bsContent;
  uint8_t* pImageBuffer = nullptr;
  RetainPtr<IFX_SeekableReadStream> pImageFileRead;
  if (wsImage.GetLength() > 0) {
    XFA_AttributeEnum iEncoding = pImage->GetTransferEncoding();
    if (iEncoding == XFA_AttributeEnum::Base64) {
      ByteString bsData = wsImage.UTF8Encode();
      int32_t iLength = bsData.GetLength();
      pImageBuffer = FX_Alloc(uint8_t, iLength);
      int32_t iRead = XFA_Base64Decode(bsData.c_str(), pImageBuffer);
      if (iRead > 0) {
        pImageFileRead =
            pdfium::MakeRetain<CFX_MemoryStream>(pImageBuffer, iRead, false);
      }
    } else {
      bsContent = ByteString::FromUnicode(wsImage);
      pImageFileRead = pdfium::MakeRetain<CFX_MemoryStream>(
          const_cast<uint8_t*>(bsContent.raw_str()), bsContent.GetLength(),
          false);
    }
  } else {
    WideString wsURL = wsHref;
    if (wsURL.Left(7) != L"http://" && wsURL.Left(6) != L"ftp://") {
      RetainPtr<CFX_DIBitmap> pBitmap =
          pDoc->GetPDFNamedImage(wsURL.AsStringView(), iImageXDpi, iImageYDpi);
      if (pBitmap) {
        bNameImage = true;
        return pBitmap;
      }
    }
    pImageFileRead = pDoc->GetDocEnvironment()->OpenLinkedFile(pDoc, wsURL);
  }
  if (!pImageFileRead) {
    FX_Free(pImageBuffer);
    return nullptr;
  }
  bNameImage = false;
  RetainPtr<CFX_DIBitmap> pBitmap =
      XFA_LoadImageFromBuffer(pImageFileRead, type, iImageXDpi, iImageYDpi);
  FX_Free(pImageBuffer);
  return pBitmap;
}

class CXFA_TextLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_TextLayoutData() {}
  ~CXFA_TextLayoutData() override {}

  CXFA_TextLayout* GetTextLayout() const { return m_pTextLayout.get(); }
  CXFA_TextProvider* GetTextProvider() const { return m_pTextProvider.get(); }

  void LoadText(CXFA_FFDoc* doc, CXFA_WidgetAcc* pAcc) {
    if (m_pTextLayout)
      return;

    m_pTextProvider =
        pdfium::MakeUnique<CXFA_TextProvider>(pAcc, XFA_TEXTPROVIDERTYPE_Text);
    m_pTextLayout =
        pdfium::MakeUnique<CXFA_TextLayout>(doc, m_pTextProvider.get());
  }

 private:
  std::unique_ptr<CXFA_TextLayout> m_pTextLayout;
  std::unique_ptr<CXFA_TextProvider> m_pTextProvider;
};

class CXFA_ImageLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_ImageLayoutData()
      : m_bNamedImage(false), m_iImageXDpi(0), m_iImageYDpi(0) {}

  ~CXFA_ImageLayoutData() override {}

  bool LoadImageData(CXFA_FFDoc* doc, CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pAcc->GetNode()->GetFormValueIfExists();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImageIfExists();
    if (!image)
      return false;

    pAcc->SetImageImage(XFA_LoadImageData(doc, image, m_bNamedImage,
                                          m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
  bool m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};

class CXFA_FieldLayoutData : public CXFA_WidgetLayoutData {
 public:
  CXFA_FieldLayoutData() {}
  ~CXFA_FieldLayoutData() override {}

  bool LoadCaption(CXFA_FFDoc* doc, CXFA_WidgetAcc* pAcc) {
    if (m_pCapTextLayout)
      return true;
    CXFA_Caption* caption = pAcc->GetNode()->GetCaptionIfExists();
    if (!caption || caption->IsHidden())
      return false;

    m_pCapTextProvider = pdfium::MakeUnique<CXFA_TextProvider>(
        pAcc, XFA_TEXTPROVIDERTYPE_Caption);
    m_pCapTextLayout =
        pdfium::MakeUnique<CXFA_TextLayout>(doc, m_pCapTextProvider.get());
    return true;
  }

  std::unique_ptr<CXFA_TextLayout> m_pCapTextLayout;
  std::unique_ptr<CXFA_TextProvider> m_pCapTextProvider;
  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  std::vector<float> m_FieldSplitArray;
};

class CXFA_TextEditData : public CXFA_FieldLayoutData {};

class CXFA_ImageEditData : public CXFA_FieldLayoutData {
 public:
  CXFA_ImageEditData()
      : m_bNamedImage(false), m_iImageXDpi(0), m_iImageYDpi(0) {}

  ~CXFA_ImageEditData() override {}

  bool LoadImageData(CXFA_FFDoc* doc, CXFA_WidgetAcc* pAcc) {
    if (m_pDIBitmap)
      return true;

    CXFA_Value* value = pAcc->GetNode()->GetFormValueIfExists();
    if (!value)
      return false;

    CXFA_Image* image = value->GetImageIfExists();
    if (!image)
      return false;

    pAcc->SetImageEditImage(XFA_LoadImageData(doc, image, m_bNamedImage,
                                              m_iImageXDpi, m_iImageYDpi));
    return !!m_pDIBitmap;
  }

  RetainPtr<CFX_DIBitmap> m_pDIBitmap;
  bool m_bNamedImage;
  int32_t m_iImageXDpi;
  int32_t m_iImageYDpi;
};

float GetEdgeThickness(const std::vector<CXFA_Stroke*>& strokes,
                       bool b3DStyle,
                       int32_t nIndex) {
  float fThickness = 0;

  CXFA_Stroke* stroke = strokes[nIndex * 2 + 1];
  if (stroke->IsVisible()) {
    if (nIndex == 0)
      fThickness += 2.5f;

    fThickness += stroke->GetThickness() * (b3DStyle ? 4 : 2);
  }
  return fThickness;
}

bool SplitDateTime(const WideString& wsDateTime,
                   WideString& wsDate,
                   WideString& wsTime) {
  wsDate = L"";
  wsTime = L"";
  if (wsDateTime.IsEmpty())
    return false;

  auto nSplitIndex = wsDateTime.Find('T');
  if (!nSplitIndex.has_value())
    nSplitIndex = wsDateTime.Find(' ');
  if (!nSplitIndex.has_value())
    return false;

  wsDate = wsDateTime.Left(nSplitIndex.value());
  if (!wsDate.IsEmpty()) {
    if (!std::any_of(wsDate.begin(), wsDate.end(), std::iswdigit))
      return false;
  }
  wsTime = wsDateTime.Right(wsDateTime.GetLength() - nSplitIndex.value() - 1);
  if (!wsTime.IsEmpty()) {
    if (!std::any_of(wsTime.begin(), wsTime.end(), std::iswdigit))
      return false;
  }
  return true;
}

std::pair<XFA_Element, CXFA_Node*> CreateUIChild(CXFA_Node* pNode) {
  XFA_Element eType = pNode->GetElementType();
  XFA_Element eWidgetType = eType;
  if (eType != XFA_Element::Field && eType != XFA_Element::Draw)
    return {eWidgetType, nullptr};

  eWidgetType = XFA_Element::Unknown;
  XFA_Element eUIType = XFA_Element::Unknown;
  auto* defValue =
      pNode->JSObject()->GetOrCreateProperty<CXFA_Value>(0, XFA_Element::Value);
  XFA_Element eValueType =
      defValue ? defValue->GetChildValueClassID() : XFA_Element::Unknown;
  switch (eValueType) {
    case XFA_Element::Boolean:
      eUIType = XFA_Element::CheckButton;
      break;
    case XFA_Element::Integer:
    case XFA_Element::Decimal:
    case XFA_Element::Float:
      eUIType = XFA_Element::NumericEdit;
      break;
    case XFA_Element::ExData:
    case XFA_Element::Text:
      eUIType = XFA_Element::TextEdit;
      eWidgetType = XFA_Element::Text;
      break;
    case XFA_Element::Date:
    case XFA_Element::Time:
    case XFA_Element::DateTime:
      eUIType = XFA_Element::DateTimeEdit;
      break;
    case XFA_Element::Image:
      eUIType = XFA_Element::ImageEdit;
      eWidgetType = XFA_Element::Image;
      break;
    case XFA_Element::Arc:
    case XFA_Element::Line:
    case XFA_Element::Rectangle:
      eUIType = XFA_Element::DefaultUi;
      eWidgetType = eValueType;
      break;
    default:
      break;
  }

  CXFA_Node* pUIChild = nullptr;
  CXFA_Ui* pUI =
      pNode->JSObject()->GetOrCreateProperty<CXFA_Ui>(0, XFA_Element::Ui);
  CXFA_Node* pChild = pUI ? pUI->GetFirstChild() : nullptr;
  for (; pChild; pChild = pChild->GetNextSibling()) {
    XFA_Element eChildType = pChild->GetElementType();
    if (eChildType == XFA_Element::Extras ||
        eChildType == XFA_Element::Picture) {
      continue;
    }

    auto node = CXFA_Node::Create(pChild->GetDocument(), XFA_Element::Ui,
                                  XFA_PacketType::Form);
    if (node && node->HasPropertyFlags(eChildType, XFA_PROPERTYFLAG_OneOf)) {
      pUIChild = pChild;
      break;
    }
  }

  if (eType == XFA_Element::Draw) {
    XFA_Element eDraw =
        pUIChild ? pUIChild->GetElementType() : XFA_Element::Unknown;
    switch (eDraw) {
      case XFA_Element::TextEdit:
        eWidgetType = XFA_Element::Text;
        break;
      case XFA_Element::ImageEdit:
        eWidgetType = XFA_Element::Image;
        break;
      default:
        eWidgetType = eWidgetType == XFA_Element::Unknown ? XFA_Element::Text
                                                          : eWidgetType;
        break;
    }
  } else {
    if (pUIChild && pUIChild->GetElementType() == XFA_Element::DefaultUi) {
      eWidgetType = XFA_Element::TextEdit;
    } else {
      eWidgetType =
          pUIChild ? pUIChild->GetElementType()
                   : (eUIType == XFA_Element::Unknown ? XFA_Element::TextEdit
                                                      : eUIType);
    }
  }

  if (!pUIChild) {
    if (eUIType == XFA_Element::Unknown) {
      eUIType = XFA_Element::TextEdit;
      if (defValue) {
        defValue->JSObject()->GetOrCreateProperty<CXFA_Text>(0,
                                                             XFA_Element::Text);
      }
    }
    return {eWidgetType,
            pUI ? pUI->JSObject()->GetOrCreateProperty<CXFA_Node>(0, eUIType)
                : nullptr};
  }

  if (eUIType != XFA_Element::Unknown)
    return {eWidgetType, pUIChild};

  switch (pUIChild->GetElementType()) {
    case XFA_Element::CheckButton: {
      eValueType = XFA_Element::Text;
      if (CXFA_Items* pItems =
              pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false)) {
        if (CXFA_Node* pItem =
                pItems->GetChild<CXFA_Node>(0, XFA_Element::Unknown, false)) {
          eValueType = pItem->GetElementType();
        }
      }
      break;
    }
    case XFA_Element::DateTimeEdit:
      eValueType = XFA_Element::DateTime;
      break;
    case XFA_Element::ImageEdit:
      eValueType = XFA_Element::Image;
      break;
    case XFA_Element::NumericEdit:
      eValueType = XFA_Element::Float;
      break;
    case XFA_Element::ChoiceList: {
      eValueType = (pUIChild->JSObject()->GetEnum(XFA_Attribute::Open) ==
                    XFA_AttributeEnum::MultiSelect)
                       ? XFA_Element::ExData
                       : XFA_Element::Text;
      break;
    }
    case XFA_Element::Barcode:
    case XFA_Element::Button:
    case XFA_Element::PasswordEdit:
    case XFA_Element::Signature:
    case XFA_Element::TextEdit:
    default:
      eValueType = XFA_Element::Text;
      break;
  }
  if (defValue)
    defValue->JSObject()->GetOrCreateProperty<CXFA_Node>(0, eValueType);

  return {eWidgetType, pUIChild};
}

}  // namespace

CXFA_WidgetAcc::CXFA_WidgetAcc(CXFA_Node* pNode)
    : m_bIsNull(true),
      m_bPreNull(true),
      m_pUiChildNode(nullptr),
      m_eUIType(XFA_Element::Unknown),
      m_pNode(pNode) {}

CXFA_WidgetAcc::~CXFA_WidgetAcc() = default;

void CXFA_WidgetAcc::ResetData() {
  WideString wsValue;
  XFA_Element eUIType = GetUIType();
  switch (eUIType) {
    case XFA_Element::ImageEdit: {
      CXFA_Value* imageValue = m_pNode->GetDefaultValueIfExists();
      CXFA_Image* image = imageValue ? imageValue->GetImageIfExists() : nullptr;
      WideString wsContentType, wsHref;
      if (image) {
        wsValue = image->GetContent();
        wsContentType = image->GetContentType();
        wsHref = image->GetHref();
      }
      SetImageEdit(wsContentType, wsHref, wsValue);
      break;
    }
    case XFA_Element::ExclGroup: {
      CXFA_Node* pNextChild = m_pNode->GetFirstContainerChild();
      while (pNextChild) {
        CXFA_Node* pChild = pNextChild;
        CXFA_WidgetAcc* pAcc = pChild->GetWidgetAcc();
        if (!pAcc)
          continue;

        bool done = false;
        if (wsValue.IsEmpty()) {
          CXFA_Value* defValue = pAcc->GetNode()->GetDefaultValueIfExists();
          if (defValue) {
            wsValue = defValue->GetChildValueContent();
            SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            pAcc->SetValue(XFA_VALUEPICTURE_Raw, wsValue);
            done = true;
          }
        }
        if (!done) {
          CXFA_Items* pItems =
              pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
          if (!pItems)
            continue;

          WideString itemText;
          if (pItems->CountChildren(XFA_Element::Unknown, false) > 1) {
            itemText =
                pItems->GetChild<CXFA_Node>(1, XFA_Element::Unknown, false)
                    ->JSObject()
                    ->GetContent(false);
          }
          pAcc->SetValue(XFA_VALUEPICTURE_Raw, itemText);
        }
        pNextChild = pChild->GetNextContainerSibling();
      }
      break;
    }
    case XFA_Element::ChoiceList:
      ClearAllSelections();
    default: {
      CXFA_Value* defValue = m_pNode->GetDefaultValueIfExists();
      if (defValue)
        wsValue = defValue->GetChildValueContent();

      SetValue(XFA_VALUEPICTURE_Raw, wsValue);
      break;
    }
  }
}

void CXFA_WidgetAcc::SetImageEdit(const WideString& wsContentType,
                                  const WideString& wsHref,
                                  const WideString& wsData) {
  CXFA_Value* formValue = m_pNode->GetFormValueIfExists();
  CXFA_Image* image = formValue ? formValue->GetImageIfExists() : nullptr;
  if (image) {
    image->SetContentType(WideString(wsContentType));
    image->SetHref(wsHref);
  }

  m_pNode->JSObject()->SetContent(wsData, GetFormatDataValue(wsData), true,
                                  false, true);

  CXFA_Node* pBind = m_pNode->GetBindData();
  if (!pBind) {
    if (image)
      image->SetTransferEncoding(XFA_AttributeEnum::Base64);
    return;
  }
  pBind->JSObject()->SetCData(XFA_Attribute::ContentType, wsContentType, false,
                              false);
  CXFA_Node* pHrefNode = pBind->GetFirstChild();
  if (pHrefNode) {
    pHrefNode->JSObject()->SetCData(XFA_Attribute::Value, wsHref, false, false);
  } else {
    CFX_XMLNode* pXMLNode = pBind->GetXMLMappingNode();
    ASSERT(pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element);
    static_cast<CFX_XMLElement*>(pXMLNode)->SetString(L"href", wsHref);
  }
}

CXFA_FFWidget* CXFA_WidgetAcc::GetNextWidget(CXFA_FFWidget* pWidget) {
  return static_cast<CXFA_FFWidget*>(pWidget->GetNext());
}

void CXFA_WidgetAcc::UpdateUIDisplay(CXFA_FFDocView* docView,
                                     CXFA_FFWidget* pExcept) {
  CXFA_FFWidget* pWidget = docView->GetWidgetForNode(m_pNode);
  for (; pWidget; pWidget = GetNextWidget(pWidget)) {
    if (pWidget == pExcept || !pWidget->IsLoaded() ||
        (GetUIType() != XFA_Element::CheckButton && pWidget->IsFocused())) {
      continue;
    }
    pWidget->UpdateFWLData();
    pWidget->AddInvalidateRect();
  }
}

void CXFA_WidgetAcc::CalcCaptionSize(CXFA_FFDoc* doc, CFX_SizeF& szCap) {
  CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
  if (!caption || !caption->IsVisible())
    return;

  LoadCaption(doc);

  XFA_Element eUIType = GetUIType();
  XFA_AttributeEnum iCapPlacement = caption->GetPlacementType();
  float fCapReserve = caption->GetReserve();
  const bool bVert = iCapPlacement == XFA_AttributeEnum::Top ||
                     iCapPlacement == XFA_AttributeEnum::Bottom;
  const bool bReserveExit = fCapReserve > 0.01;
  CXFA_TextLayout* pCapTextLayout =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
          ->m_pCapTextLayout.get();
  if (pCapTextLayout) {
    if (!bVert && eUIType != XFA_Element::Button)
      szCap.width = fCapReserve;

    CFX_SizeF minSize;
    szCap = pCapTextLayout->CalcSize(minSize, szCap);
    if (bReserveExit)
      bVert ? szCap.height = fCapReserve : szCap.width = fCapReserve;
  } else {
    float fFontSize = 10.0f;
    CXFA_Font* font = caption->GetFontIfExists();
    if (font) {
      fFontSize = font->GetFontSize();
    } else {
      CXFA_Font* widgetfont = m_pNode->GetFontIfExists();
      if (widgetfont)
        fFontSize = widgetfont->GetFontSize();
    }

    if (bVert) {
      szCap.height = fCapReserve > 0 ? fCapReserve : fFontSize;
    } else {
      szCap.width = fCapReserve > 0 ? fCapReserve : 0;
      szCap.height = fFontSize;
    }
  }

  CXFA_Margin* captionMargin = caption->GetMarginIfExists();
  if (!captionMargin)
    return;

  float fLeftInset = captionMargin->GetLeftInset();
  float fTopInset = captionMargin->GetTopInset();
  float fRightInset = captionMargin->GetRightInset();
  float fBottomInset = captionMargin->GetBottomInset();
  if (bReserveExit) {
    bVert ? (szCap.width += fLeftInset + fRightInset)
          : (szCap.height += fTopInset + fBottomInset);
  } else {
    szCap.width += fLeftInset + fRightInset;
    szCap.height += fTopInset + fBottomInset;
  }
}

bool CXFA_WidgetAcc::CalculateFieldAutoSize(CXFA_FFDoc* doc, CFX_SizeF& size) {
  CFX_SizeF szCap;
  CalcCaptionSize(doc, szCap);

  CFX_RectF rtUIMargin = GetUIMargin();
  size.width += rtUIMargin.left + rtUIMargin.width;
  size.height += rtUIMargin.top + rtUIMargin.height;
  if (szCap.width > 0 && szCap.height > 0) {
    CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
    XFA_AttributeEnum placement = caption ? caption->GetPlacementType()
                                          : CXFA_Caption::kDefaultPlacementType;
    switch (placement) {
      case XFA_AttributeEnum::Left:
      case XFA_AttributeEnum::Right:
      case XFA_AttributeEnum::Inline: {
        size.width += szCap.width;
        size.height = std::max(size.height, szCap.height);
      } break;
      case XFA_AttributeEnum::Top:
      case XFA_AttributeEnum::Bottom: {
        size.height += szCap.height;
        size.width = std::max(size.width, szCap.width);
      }
      default:
        break;
    }
  }
  return CalculateWidgetAutoSize(size);
}

bool CXFA_WidgetAcc::CalculateWidgetAutoSize(CFX_SizeF& size) {
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin) {
    size.width += margin->GetLeftInset() + margin->GetRightInset();
    size.height += margin->GetTopInset() + margin->GetBottomInset();
  }

  CXFA_Para* para = m_pNode->GetParaIfExists();
  if (para)
    size.width += para->GetMarginLeft() + para->GetTextIndent();

  Optional<float> width = m_pNode->TryWidth();
  if (width) {
    size.width = *width;
  } else {
    Optional<float> min = m_pNode->TryMinWidth();
    if (min)
      size.width = std::max(size.width, *min);

    Optional<float> max = m_pNode->TryMaxWidth();
    if (max && *max > 0)
      size.width = std::min(size.width, *max);
  }

  Optional<float> height = m_pNode->TryHeight();
  if (height) {
    size.height = *height;
  } else {
    Optional<float> min = m_pNode->TryMinHeight();
    if (min)
      size.height = std::max(size.height, *min);

    Optional<float> max = m_pNode->TryMaxHeight();
    if (max && *max > 0)
      size.height = std::min(size.height, *max);
  }
  return true;
}

void CXFA_WidgetAcc::CalculateTextContentSize(CXFA_FFDoc* doc,
                                              CFX_SizeF& size) {
  float fFontSize = m_pNode->GetFontSize();
  WideString wsText = GetValue(XFA_VALUEPICTURE_Display);
  if (wsText.IsEmpty()) {
    size.height += fFontSize;
    return;
  }

  wchar_t wcEnter = '\n';
  wchar_t wsLast = wsText[wsText.GetLength() - 1];
  if (wsLast == wcEnter)
    wsText = wsText + wcEnter;

  CXFA_FieldLayoutData* layoutData =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get());
  if (!layoutData->m_pTextOut) {
    layoutData->m_pTextOut = pdfium::MakeUnique<CFDE_TextOut>();
    CFDE_TextOut* pTextOut = layoutData->m_pTextOut.get();
    pTextOut->SetFont(GetFDEFont(doc));
    pTextOut->SetFontSize(fFontSize);
    pTextOut->SetLineBreakTolerance(fFontSize * 0.2f);
    pTextOut->SetLineSpace(m_pNode->GetLineHeight());

    FDE_TextStyle dwStyles;
    dwStyles.last_line_height_ = true;
    if (GetUIType() == XFA_Element::TextEdit && IsMultiLine())
      dwStyles.line_wrap_ = true;

    pTextOut->SetStyles(dwStyles);
  }
  layoutData->m_pTextOut->CalcLogicSize(wsText, size);
}

bool CXFA_WidgetAcc::CalculateTextEditAutoSize(CXFA_FFDoc* doc,
                                               CFX_SizeF& size) {
  if (size.width > 0) {
    CFX_SizeF szOrz = size;
    CFX_SizeF szCap;
    CalcCaptionSize(doc, szCap);
    bool bCapExit = szCap.width > 0.01 && szCap.height > 0.01;
    XFA_AttributeEnum iCapPlacement = XFA_AttributeEnum::Unknown;
    if (bCapExit) {
      CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
      iCapPlacement = caption ? caption->GetPlacementType()
                              : CXFA_Caption::kDefaultPlacementType;
      switch (iCapPlacement) {
        case XFA_AttributeEnum::Left:
        case XFA_AttributeEnum::Right:
        case XFA_AttributeEnum::Inline: {
          size.width -= szCap.width;
        }
        default:
          break;
      }
    }
    CFX_RectF rtUIMargin = GetUIMargin();
    size.width -= rtUIMargin.left + rtUIMargin.width;
    CXFA_Margin* margin = m_pNode->GetMarginIfExists();
    if (margin)
      size.width -= margin->GetLeftInset() + margin->GetRightInset();

    CalculateTextContentSize(doc, size);
    size.height += rtUIMargin.top + rtUIMargin.height;
    if (bCapExit) {
      switch (iCapPlacement) {
        case XFA_AttributeEnum::Left:
        case XFA_AttributeEnum::Right:
        case XFA_AttributeEnum::Inline: {
          size.height = std::max(size.height, szCap.height);
        } break;
        case XFA_AttributeEnum::Top:
        case XFA_AttributeEnum::Bottom: {
          size.height += szCap.height;
        }
        default:
          break;
      }
    }
    size.width = szOrz.width;
    return CalculateWidgetAutoSize(size);
  }
  CalculateTextContentSize(doc, size);
  return CalculateFieldAutoSize(doc, size);
}

bool CXFA_WidgetAcc::CalculateCheckButtonAutoSize(CXFA_FFDoc* doc,
                                                  CFX_SizeF& size) {
  float fCheckSize = GetCheckButtonSize();
  size = CFX_SizeF(fCheckSize, fCheckSize);
  return CalculateFieldAutoSize(doc, size);
}

bool CXFA_WidgetAcc::CalculatePushButtonAutoSize(CXFA_FFDoc* doc,
                                                 CFX_SizeF& size) {
  CalcCaptionSize(doc, size);
  return CalculateWidgetAutoSize(size);
}

CFX_SizeF CXFA_WidgetAcc::CalculateImageSize(float img_width,
                                             float img_height,
                                             float dpi_x,
                                             float dpi_y) {
  CFX_RectF rtImage(0, 0, XFA_UnitPx2Pt(img_width, dpi_x),
                    XFA_UnitPx2Pt(img_height, dpi_y));

  CFX_RectF rtFit;
  Optional<float> width = m_pNode->TryWidth();
  if (width) {
    rtFit.width = *width;
    GetWidthWithoutMargin(rtFit.width);
  } else {
    rtFit.width = rtImage.width;
  }

  Optional<float> height = m_pNode->TryHeight();
  if (height) {
    rtFit.height = *height;
    GetHeightWithoutMargin(rtFit.height);
  } else {
    rtFit.height = rtImage.height;
  }

  return rtFit.Size();
}

bool CXFA_WidgetAcc::CalculateImageAutoSize(CXFA_FFDoc* doc, CFX_SizeF& size) {
  if (!GetImageImage())
    LoadImageImage(doc);

  size.clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetImageImage();
  if (!pBitmap)
    return CalculateWidgetAutoSize(size);

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  GetImageDpi(iImageXDpi, iImageYDpi);

  size = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                            iImageXDpi, iImageYDpi);
  return CalculateWidgetAutoSize(size);
}

bool CXFA_WidgetAcc::CalculateImageEditAutoSize(CXFA_FFDoc* doc,
                                                CFX_SizeF& size) {
  if (!GetImageEditImage())
    LoadImageEditImage(doc);

  size.clear();
  RetainPtr<CFX_DIBitmap> pBitmap = GetImageEditImage();
  if (!pBitmap)
    return CalculateFieldAutoSize(doc, size);

  int32_t iImageXDpi = 0;
  int32_t iImageYDpi = 0;
  GetImageEditDpi(iImageXDpi, iImageYDpi);

  size = CalculateImageSize(pBitmap->GetWidth(), pBitmap->GetHeight(),
                            iImageXDpi, iImageYDpi);
  return CalculateFieldAutoSize(doc, size);
}

bool CXFA_WidgetAcc::LoadImageImage(CXFA_FFDoc* doc) {
  InitLayoutData();
  return static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get())
      ->LoadImageData(doc, this);
}

bool CXFA_WidgetAcc::LoadImageEditImage(CXFA_FFDoc* doc) {
  InitLayoutData();
  return static_cast<CXFA_ImageEditData*>(m_pLayoutData.get())
      ->LoadImageData(doc, this);
}

void CXFA_WidgetAcc::GetImageDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  CXFA_ImageLayoutData* pData =
      static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get());
  iImageXDpi = pData->m_iImageXDpi;
  iImageYDpi = pData->m_iImageYDpi;
}

void CXFA_WidgetAcc::GetImageEditDpi(int32_t& iImageXDpi, int32_t& iImageYDpi) {
  CXFA_ImageEditData* pData =
      static_cast<CXFA_ImageEditData*>(m_pLayoutData.get());
  iImageXDpi = pData->m_iImageXDpi;
  iImageYDpi = pData->m_iImageYDpi;
}

void CXFA_WidgetAcc::LoadText(CXFA_FFDoc* doc) {
  InitLayoutData();
  static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->LoadText(doc, this);
}

float CXFA_WidgetAcc::CalculateWidgetAutoWidth(float fWidthCalc) {
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin)
    fWidthCalc += margin->GetLeftInset() + margin->GetRightInset();

  Optional<float> min = m_pNode->TryMinWidth();
  if (min)
    fWidthCalc = std::max(fWidthCalc, *min);

  Optional<float> max = m_pNode->TryMaxWidth();
  if (max && *max > 0)
    fWidthCalc = std::min(fWidthCalc, *max);

  return fWidthCalc;
}

float CXFA_WidgetAcc::GetWidthWithoutMargin(float fWidthCalc) {
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin)
    fWidthCalc -= margin->GetLeftInset() + margin->GetRightInset();
  return fWidthCalc;
}

float CXFA_WidgetAcc::CalculateWidgetAutoHeight(float fHeightCalc) {
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin)
    fHeightCalc += margin->GetTopInset() + margin->GetBottomInset();

  Optional<float> min = m_pNode->TryMinHeight();
  if (min)
    fHeightCalc = std::max(fHeightCalc, *min);

  Optional<float> max = m_pNode->TryMaxHeight();
  if (max && *max > 0)
    fHeightCalc = std::min(fHeightCalc, *max);

  return fHeightCalc;
}

float CXFA_WidgetAcc::GetHeightWithoutMargin(float fHeightCalc) {
  CXFA_Margin* margin = m_pNode->GetMarginIfExists();
  if (margin)
    fHeightCalc -= margin->GetTopInset() + margin->GetBottomInset();
  return fHeightCalc;
}

void CXFA_WidgetAcc::StartWidgetLayout(CXFA_FFDoc* doc,
                                       float& fCalcWidth,
                                       float& fCalcHeight) {
  InitLayoutData();

  XFA_Element eUIType = GetUIType();
  if (eUIType == XFA_Element::Text) {
    m_pLayoutData->m_fWidgetHeight = m_pNode->TryHeight().value_or(-1);
    StartTextLayout(doc, fCalcWidth, fCalcHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight > 0)
    return;

  m_pLayoutData->m_fWidgetHeight = -1;
  float fWidth = 0;
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    Optional<float> height = m_pNode->TryHeight();
    if (height)
      fCalcHeight = *height;
    else
      CalculateAccWidthAndHeight(doc, eUIType, fCalcWidth, fCalcHeight);

    m_pLayoutData->m_fWidgetHeight = fCalcHeight;
    return;
  }
  if (fCalcWidth < 0 && fCalcHeight < 0) {
    Optional<float> height;
    Optional<float> width = m_pNode->TryWidth();
    if (width) {
      fWidth = *width;

      height = m_pNode->TryHeight();
      if (height)
        fCalcHeight = *height;
    }
    if (!width || !height)
      CalculateAccWidthAndHeight(doc, eUIType, fWidth, fCalcHeight);

    fCalcWidth = fWidth;
  }
  m_pLayoutData->m_fWidgetHeight = fCalcHeight;
}

void CXFA_WidgetAcc::CalculateAccWidthAndHeight(CXFA_FFDoc* doc,
                                                XFA_Element eUIType,
                                                float& fWidth,
                                                float& fCalcHeight) {
  CFX_SizeF sz(fWidth, m_pLayoutData->m_fWidgetHeight);
  switch (eUIType) {
    case XFA_Element::Barcode:
    case XFA_Element::ChoiceList:
    case XFA_Element::Signature:
      CalculateFieldAutoSize(doc, sz);
      break;
    case XFA_Element::ImageEdit:
      CalculateImageEditAutoSize(doc, sz);
      break;
    case XFA_Element::Button:
      CalculatePushButtonAutoSize(doc, sz);
      break;
    case XFA_Element::CheckButton:
      CalculateCheckButtonAutoSize(doc, sz);
      break;
    case XFA_Element::DateTimeEdit:
    case XFA_Element::NumericEdit:
    case XFA_Element::PasswordEdit:
    case XFA_Element::TextEdit:
      CalculateTextEditAutoSize(doc, sz);
      break;
    case XFA_Element::Image:
      CalculateImageAutoSize(doc, sz);
      break;
    case XFA_Element::Arc:
    case XFA_Element::Line:
    case XFA_Element::Rectangle:
    case XFA_Element::Subform:
    case XFA_Element::ExclGroup:
      CalculateWidgetAutoSize(sz);
      break;
    default:
      break;
  }
  fWidth = sz.width;
  m_pLayoutData->m_fWidgetHeight = sz.height;
  fCalcHeight = sz.height;
}

bool CXFA_WidgetAcc::FindSplitPos(CXFA_FFDocView* docView,
                                  int32_t iBlockIndex,
                                  float& fCalcHeight) {
  XFA_Element eUIType = GetUIType();
  if (eUIType == XFA_Element::Subform)
    return false;

  if (eUIType != XFA_Element::Text && eUIType != XFA_Element::TextEdit &&
      eUIType != XFA_Element::NumericEdit &&
      eUIType != XFA_Element::PasswordEdit) {
    fCalcHeight = 0;
    return true;
  }

  float fTopInset = 0;
  float fBottomInset = 0;
  if (iBlockIndex == 0) {
    CXFA_Margin* margin = m_pNode->GetMarginIfExists();
    if (margin) {
      fTopInset = margin->GetTopInset();
      fBottomInset = margin->GetBottomInset();
    }

    CFX_RectF rtUIMargin = GetUIMargin();
    fTopInset += rtUIMargin.top;
    fBottomInset += rtUIMargin.width;
  }
  if (eUIType == XFA_Element::Text) {
    float fHeight = fCalcHeight;
    if (iBlockIndex == 0) {
      fCalcHeight = fCalcHeight - fTopInset;
      if (fCalcHeight < 0)
        fCalcHeight = 0;
    }

    CXFA_TextLayout* pTextLayout =
        static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->GetTextLayout();
    fCalcHeight =
        pTextLayout->DoLayout(iBlockIndex, fCalcHeight, fCalcHeight,
                              m_pLayoutData->m_fWidgetHeight - fTopInset);
    if (fCalcHeight != 0) {
      if (iBlockIndex == 0)
        fCalcHeight = fCalcHeight + fTopInset;
      if (fabs(fHeight - fCalcHeight) < XFA_FLOAT_PERCISION)
        return false;
    }
    return true;
  }
  XFA_AttributeEnum iCapPlacement = XFA_AttributeEnum::Unknown;
  float fCapReserve = 0;
  if (iBlockIndex == 0) {
    CXFA_Caption* caption = m_pNode->GetCaptionIfExists();
    if (caption && !caption->IsHidden()) {
      iCapPlacement = caption->GetPlacementType();
      fCapReserve = caption->GetReserve();
    }
    if (iCapPlacement == XFA_AttributeEnum::Top &&
        fCalcHeight < fCapReserve + fTopInset) {
      fCalcHeight = 0;
      return true;
    }
    if (iCapPlacement == XFA_AttributeEnum::Bottom &&
        m_pLayoutData->m_fWidgetHeight - fCapReserve - fBottomInset) {
      fCalcHeight = 0;
      return true;
    }
    if (iCapPlacement != XFA_AttributeEnum::Top)
      fCapReserve = 0;
  }
  CXFA_FieldLayoutData* pFieldData =
      static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get());
  int32_t iLinesCount = 0;
  float fHeight = m_pLayoutData->m_fWidgetHeight;
  if (GetValue(XFA_VALUEPICTURE_Display).IsEmpty()) {
    iLinesCount = 1;
  } else {
    if (!pFieldData->m_pTextOut) {
      // TODO(dsinclair): Inline fWidth when the 2nd param of
      // CalculateAccWidthAndHeight isn't a ref-param.
      float fWidth = m_pNode->TryWidth().value_or(0);
      CalculateAccWidthAndHeight(docView->GetDoc(), eUIType, fWidth, fHeight);
    }
    iLinesCount = pFieldData->m_pTextOut->GetTotalLines();
  }
  std::vector<float>* pFieldArray = &pFieldData->m_FieldSplitArray;
  int32_t iFieldSplitCount = pdfium::CollectionSize<int32_t>(*pFieldArray);
  for (int32_t i = 0; i < iBlockIndex * 3; i += 3) {
    iLinesCount -= (int32_t)(*pFieldArray)[i + 1];
    fHeight -= (*pFieldArray)[i + 2];
  }
  if (iLinesCount == 0)
    return false;

  float fLineHeight = m_pNode->GetLineHeight();
  float fFontSize = m_pNode->GetFontSize();
  float fTextHeight = iLinesCount * fLineHeight - fLineHeight + fFontSize;
  float fSpaceAbove = 0;
  float fStartOffset = 0;
  if (fHeight > 0.1f && iBlockIndex == 0) {
    fStartOffset = fTopInset;
    fHeight -= (fTopInset + fBottomInset);
    CXFA_Para* para = m_pNode->GetParaIfExists();
    if (para) {
      fSpaceAbove = para->GetSpaceAbove();
      float fSpaceBelow = para->GetSpaceBelow();
      fHeight -= (fSpaceAbove + fSpaceBelow);
      switch (para->GetVerticalAlign()) {
        case XFA_AttributeEnum::Top:
          fStartOffset += fSpaceAbove;
          break;
        case XFA_AttributeEnum::Middle:
          fStartOffset += ((fHeight - fTextHeight) / 2 + fSpaceAbove);
          break;
        case XFA_AttributeEnum::Bottom:
          fStartOffset += (fHeight - fTextHeight + fSpaceAbove);
          break;
        default:
          NOTREACHED();
          break;
      }
    }
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  for (int32_t i = iBlockIndex - 1; iBlockIndex > 0 && i < iBlockIndex; i++) {
    fStartOffset = (*pFieldArray)[i * 3] - (*pFieldArray)[i * 3 + 2];
    if (fStartOffset < 0.1f)
      fStartOffset = 0;
  }
  if (iFieldSplitCount / 3 == (iBlockIndex + 1))
    (*pFieldArray)[0] = fStartOffset;
  else
    pFieldArray->push_back(fStartOffset);

  XFA_VERSION version = docView->GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bCanSplitNoContent = false;
  XFA_AttributeEnum eLayoutMode = GetNode()
                                      ->GetParent()
                                      ->JSObject()
                                      ->TryEnum(XFA_Attribute::Layout, true)
                                      .value_or(XFA_AttributeEnum::Position);
  if ((eLayoutMode == XFA_AttributeEnum::Position ||
       eLayoutMode == XFA_AttributeEnum::Tb ||
       eLayoutMode == XFA_AttributeEnum::Row ||
       eLayoutMode == XFA_AttributeEnum::Table) &&
      version > XFA_VERSION_208) {
    bCanSplitNoContent = true;
  }
  if ((eLayoutMode == XFA_AttributeEnum::Tb ||
       eLayoutMode == XFA_AttributeEnum::Row ||
       eLayoutMode == XFA_AttributeEnum::Table) &&
      version <= XFA_VERSION_208) {
    if (fStartOffset < fCalcHeight) {
      bCanSplitNoContent = true;
    } else {
      fCalcHeight = 0;
      return true;
    }
  }
  if (bCanSplitNoContent) {
    if ((fCalcHeight - fTopInset - fSpaceAbove < fLineHeight)) {
      fCalcHeight = 0;
      return true;
    }
    if (fStartOffset + XFA_FLOAT_PERCISION >= fCalcHeight) {
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = 0;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
      } else {
        pFieldArray->push_back(0);
        pFieldArray->push_back(fCalcHeight);
      }
      return false;
    }
    if (fCalcHeight - fStartOffset < fLineHeight) {
      fCalcHeight = fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = 0;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
      } else {
        pFieldArray->push_back(0);
        pFieldArray->push_back(fCalcHeight);
      }
      return true;
    }
    float fTextNum =
        fCalcHeight + XFA_FLOAT_PERCISION - fCapReserve - fStartOffset;
    int32_t iLineNum =
        (int32_t)((fTextNum + (fLineHeight - fFontSize)) / fLineHeight);
    if (iLineNum >= iLinesCount) {
      if (fCalcHeight - fStartOffset - fTextHeight >= fFontSize) {
        if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
          (*pFieldArray)[iBlockIndex * 3 + 1] = (float)iLinesCount;
          (*pFieldArray)[iBlockIndex * 3 + 2] = fCalcHeight;
        } else {
          pFieldArray->push_back((float)iLinesCount);
          pFieldArray->push_back(fCalcHeight);
        }
        return false;
      }
      if (fHeight - fStartOffset - fTextHeight < fFontSize) {
        iLineNum -= 1;
        if (iLineNum == 0) {
          fCalcHeight = 0;
          return true;
        }
      } else {
        iLineNum = (int32_t)(fTextNum / fLineHeight);
      }
    }
    if (iLineNum > 0) {
      float fSplitHeight = iLineNum * fLineHeight + fCapReserve + fStartOffset;
      if (iFieldSplitCount / 3 == (iBlockIndex + 1)) {
        (*pFieldArray)[iBlockIndex * 3 + 1] = (float)iLineNum;
        (*pFieldArray)[iBlockIndex * 3 + 2] = fSplitHeight;
      } else {
        pFieldArray->push_back((float)iLineNum);
        pFieldArray->push_back(fSplitHeight);
      }
      if (fabs(fSplitHeight - fCalcHeight) < XFA_FLOAT_PERCISION)
        return false;

      fCalcHeight = fSplitHeight;
      return true;
    }
  }
  fCalcHeight = 0;
  return true;
}

void CXFA_WidgetAcc::InitLayoutData() {
  if (m_pLayoutData)
    return;

  switch (GetUIType()) {
    case XFA_Element::Text:
      m_pLayoutData = pdfium::MakeUnique<CXFA_TextLayoutData>();
      return;
    case XFA_Element::TextEdit:
      m_pLayoutData = pdfium::MakeUnique<CXFA_TextEditData>();
      return;
    case XFA_Element::Image:
      m_pLayoutData = pdfium::MakeUnique<CXFA_ImageLayoutData>();
      return;
    case XFA_Element::ImageEdit:
      m_pLayoutData = pdfium::MakeUnique<CXFA_ImageEditData>();
      return;
    default:
      break;
  }
  if (m_pNode && m_pNode->GetElementType() == XFA_Element::Field) {
    m_pLayoutData = pdfium::MakeUnique<CXFA_FieldLayoutData>();
    return;
  }
  m_pLayoutData = pdfium::MakeUnique<CXFA_WidgetLayoutData>();
}

void CXFA_WidgetAcc::StartTextLayout(CXFA_FFDoc* doc,
                                     float& fCalcWidth,
                                     float& fCalcHeight) {
  LoadText(doc);

  CXFA_TextLayout* pTextLayout =
      static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())->GetTextLayout();
  float fTextHeight = 0;
  if (fCalcWidth > 0 && fCalcHeight > 0) {
    float fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
    fTextHeight = fCalcHeight;
    fTextHeight = GetHeightWithoutMargin(fTextHeight);
    pTextLayout->DoLayout(0, fTextHeight, -1, fTextHeight);
    return;
  }
  if (fCalcWidth > 0 && fCalcHeight < 0) {
    float fWidth = GetWidthWithoutMargin(fCalcWidth);
    pTextLayout->StartLayout(fWidth);
  }

  if (fCalcWidth < 0 && fCalcHeight < 0) {
    Optional<float> width = m_pNode->TryWidth();
    if (width) {
      pTextLayout->StartLayout(GetWidthWithoutMargin(*width));
      fCalcWidth = *width;
    } else {
      float fMaxWidth = CalculateWidgetAutoWidth(pTextLayout->StartLayout(-1));
      pTextLayout->StartLayout(GetWidthWithoutMargin(fMaxWidth));
      fCalcWidth = fMaxWidth;
    }
  }

  if (m_pLayoutData->m_fWidgetHeight < 0) {
    m_pLayoutData->m_fWidgetHeight = pTextLayout->GetLayoutHeight();
    m_pLayoutData->m_fWidgetHeight =
        CalculateWidgetAutoHeight(m_pLayoutData->m_fWidgetHeight);
  }
  fTextHeight = m_pLayoutData->m_fWidgetHeight;
  fTextHeight = GetHeightWithoutMargin(fTextHeight);
  pTextLayout->DoLayout(0, fTextHeight, -1, fTextHeight);
  fCalcHeight = m_pLayoutData->m_fWidgetHeight;
}

bool CXFA_WidgetAcc::LoadCaption(CXFA_FFDoc* doc) {
  InitLayoutData();
  return static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
      ->LoadCaption(doc, this);
}

CXFA_TextLayout* CXFA_WidgetAcc::GetCaptionTextLayout() {
  return m_pLayoutData
             ? static_cast<CXFA_FieldLayoutData*>(m_pLayoutData.get())
                   ->m_pCapTextLayout.get()
             : nullptr;
}

CXFA_TextLayout* CXFA_WidgetAcc::GetTextLayout() {
  return m_pLayoutData
             ? static_cast<CXFA_TextLayoutData*>(m_pLayoutData.get())
                   ->GetTextLayout()
             : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_WidgetAcc::GetImageImage() {
  return m_pLayoutData
             ? static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get())
                   ->m_pDIBitmap
             : nullptr;
}

RetainPtr<CFX_DIBitmap> CXFA_WidgetAcc::GetImageEditImage() {
  return m_pLayoutData
             ? static_cast<CXFA_ImageEditData*>(m_pLayoutData.get())
                   ->m_pDIBitmap
             : nullptr;
}

void CXFA_WidgetAcc::SetImageImage(const RetainPtr<CFX_DIBitmap>& newImage) {
  CXFA_ImageLayoutData* pData =
      static_cast<CXFA_ImageLayoutData*>(m_pLayoutData.get());
  if (pData->m_pDIBitmap != newImage)
    pData->m_pDIBitmap = newImage;
}

void CXFA_WidgetAcc::SetImageEditImage(
    const RetainPtr<CFX_DIBitmap>& newImage) {
  CXFA_ImageEditData* pData =
      static_cast<CXFA_ImageEditData*>(m_pLayoutData.get());
  if (pData->m_pDIBitmap != newImage)
    pData->m_pDIBitmap = newImage;
}

RetainPtr<CFGAS_GEFont> CXFA_WidgetAcc::GetFDEFont(CXFA_FFDoc* doc) {
  WideString wsFontName = L"Courier";
  uint32_t dwFontStyle = 0;
  CXFA_Font* font = m_pNode->GetFontIfExists();
  if (font) {
    if (font->IsBold())
      dwFontStyle |= FXFONT_BOLD;
    if (font->IsItalic())
      dwFontStyle |= FXFONT_ITALIC;

    wsFontName = font->GetTypeface();
  }
  return doc->GetApp()->GetXFAFontMgr()->GetFont(doc, wsFontName.AsStringView(),
                                                 dwFontStyle);
}

CXFA_Node* CXFA_WidgetAcc::GetUIChild() {
  if (m_eUIType == XFA_Element::Unknown)
    std::tie(m_eUIType, m_pUiChildNode) = CreateUIChild(m_pNode);
  return m_pUiChildNode;
}

XFA_Element CXFA_WidgetAcc::GetUIType() {
  GetUIChild();
  return m_eUIType;
}

bool CXFA_WidgetAcc::IsOpenAccess() const {
  return m_pNode && m_pNode->IsOpenAccess();
}

std::vector<CXFA_Event*> CXFA_WidgetAcc::GetEventByActivity(
    XFA_AttributeEnum iActivity,
    bool bIsFormReady) {
  std::vector<CXFA_Event*> events;
  for (CXFA_Node* node : m_pNode->GetNodeList(0, XFA_Element::Event)) {
    auto* event = static_cast<CXFA_Event*>(node);
    if (event->GetActivity() == iActivity) {
      if (iActivity == XFA_AttributeEnum::Ready) {
        WideString wsRef = event->GetRef();
        if (bIsFormReady) {
          if (wsRef == WideStringView(L"$form"))
            events.push_back(event);
        } else {
          if (wsRef == WideStringView(L"$layout"))
            events.push_back(event);
        }
      } else {
        events.push_back(event);
      }
    }
  }
  return events;
}

CXFA_Border* CXFA_WidgetAcc::GetUIBorder() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->JSObject()->GetProperty<CXFA_Border>(
                        0, XFA_Element::Border)
                  : nullptr;
}

CFX_RectF CXFA_WidgetAcc::GetUIMargin() {
  CXFA_Node* pUIChild = GetUIChild();
  CXFA_Margin* mgUI = nullptr;
  if (pUIChild) {
    mgUI =
        pUIChild->JSObject()->GetProperty<CXFA_Margin>(0, XFA_Element::Margin);
  }

  if (!mgUI)
    return CFX_RectF();

  CXFA_Border* border = GetUIBorder();
  if (border && border->GetPresence() != XFA_AttributeEnum::Visible)
    return CFX_RectF();

  Optional<float> left = mgUI->TryLeftInset();
  Optional<float> top = mgUI->TryTopInset();
  Optional<float> right = mgUI->TryRightInset();
  Optional<float> bottom = mgUI->TryBottomInset();
  if (border) {
    bool bVisible = false;
    float fThickness = 0;
    XFA_AttributeEnum iType = XFA_AttributeEnum::Unknown;
    std::tie(iType, bVisible, fThickness) = border->Get3DStyle();
    if (!left || !top || !right || !bottom) {
      std::vector<CXFA_Stroke*> strokes = border->GetStrokes();
      if (!top)
        top = GetEdgeThickness(strokes, bVisible, 0);
      if (!right)
        right = GetEdgeThickness(strokes, bVisible, 1);
      if (!bottom)
        bottom = GetEdgeThickness(strokes, bVisible, 2);
      if (!left)
        left = GetEdgeThickness(strokes, bVisible, 3);
    }
  }
  return CFX_RectF(left.value_or(0.0), top.value_or(0.0), right.value_or(0.0),
                   bottom.value_or(0.0));
}

XFA_AttributeEnum CXFA_WidgetAcc::GetButtonHighlight() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Highlight);
  return XFA_AttributeEnum::Inverted;
}

bool CXFA_WidgetAcc::HasButtonRollover() const {
  CXFA_Items* pItems =
      m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetFirstChild(); pText;
       pText = pText->GetNextSibling()) {
    if (pText->JSObject()->GetCData(XFA_Attribute::Name) == L"rollover")
      return !pText->JSObject()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetAcc::HasButtonDown() const {
  CXFA_Items* pItems =
      m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return false;

  for (CXFA_Node* pText = pItems->GetFirstChild(); pText;
       pText = pText->GetNextSibling()) {
    if (pText->JSObject()->GetCData(XFA_Attribute::Name) == L"down")
      return !pText->JSObject()->GetContent(false).IsEmpty();
  }
  return false;
}

bool CXFA_WidgetAcc::IsCheckButtonRound() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Shape) ==
           XFA_AttributeEnum::Round;
  return false;
}

XFA_AttributeEnum CXFA_WidgetAcc::GetCheckButtonMark() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild)
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Mark);
  return XFA_AttributeEnum::Default;
}

bool CXFA_WidgetAcc::IsRadioButton() {
  CXFA_Node* pParent = m_pNode->GetParent();
  return pParent && pParent->GetElementType() == XFA_Element::ExclGroup;
}

float CXFA_WidgetAcc::GetCheckButtonSize() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()
        ->GetMeasure(XFA_Attribute::Size)
        .ToUnit(XFA_Unit::Pt);
  }
  return CXFA_Measurement(10, XFA_Unit::Pt).ToUnit(XFA_Unit::Pt);
}

bool CXFA_WidgetAcc::IsAllowNeutral() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild &&
         pUIChild->JSObject()->GetBoolean(XFA_Attribute::AllowNeutral);
}

XFA_CHECKSTATE CXFA_WidgetAcc::GetCheckState() {
  WideString wsValue = m_pNode->GetRawValue();
  if (wsValue.IsEmpty())
    return XFA_CHECKSTATE_Off;

  auto* pItems = m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
  if (!pItems)
    return XFA_CHECKSTATE_Off;

  CXFA_Node* pText = pItems->GetFirstChild();
  int32_t i = 0;
  while (pText) {
    Optional<WideString> wsContent = pText->JSObject()->TryContent(false, true);
    if (wsContent && *wsContent == wsValue)
      return static_cast<XFA_CHECKSTATE>(i);

    i++;
    pText = pText->GetNextSibling();
  }
  return XFA_CHECKSTATE_Off;
}

void CXFA_WidgetAcc::SetCheckState(XFA_CHECKSTATE eCheckState, bool bNotify) {
  CXFA_Node* node = m_pNode->GetExclGroupIfExists();
  if (!node) {
    CXFA_Items* pItems =
        m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItems)
      return;

    int32_t i = -1;
    CXFA_Node* pText = pItems->GetFirstChild();
    WideString wsContent;
    while (pText) {
      i++;
      if (i == eCheckState) {
        wsContent = pText->JSObject()->GetContent(false);
        break;
      }
      pText = pText->GetNextSibling();
    }
    if (m_pNode)
      m_pNode->SyncValue(wsContent, bNotify);

    return;
  }

  WideString wsValue;
  if (eCheckState != XFA_CHECKSTATE_Off) {
    if (CXFA_Items* pItems =
            m_pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false)) {
      CXFA_Node* pText = pItems->GetFirstChild();
      if (pText)
        wsValue = pText->JSObject()->GetContent(false);
    }
  }
  CXFA_Node* pChild = node->GetFirstChild();
  for (; pChild; pChild = pChild->GetNextSibling()) {
    if (pChild->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pChild->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetFirstChild();
    if (!pItemchild)
      continue;

    WideString text = pItemchild->JSObject()->GetContent(false);
    WideString wsChildValue = text;
    if (wsValue != text) {
      pItemchild = pItemchild->GetNextSibling();
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    }
    pChild->SyncValue(wsChildValue, bNotify);
  }
  node->SyncValue(wsValue, bNotify);
}

CXFA_Node* CXFA_WidgetAcc::GetSelectedMember() {
  CXFA_Node* pSelectedMember = nullptr;
  WideString wsState = m_pNode->GetRawValue();
  if (wsState.IsEmpty())
    return pSelectedMember;

  for (CXFA_Node* pNode = ToNode(m_pNode->GetFirstChild()); pNode;
       pNode = pNode->GetNextSibling()) {
    CXFA_WidgetAcc widgetData(pNode);
    if (widgetData.GetCheckState() == XFA_CHECKSTATE_On) {
      pSelectedMember = pNode;
      break;
    }
  }
  return pSelectedMember;
}

CXFA_Node* CXFA_WidgetAcc::SetSelectedMember(const WideStringView& wsName,
                                             bool bNotify) {
  uint32_t nameHash = FX_HashCode_GetW(wsName, false);
  for (CXFA_Node* pNode = ToNode(m_pNode->GetFirstChild()); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == nameHash) {
      CXFA_WidgetAcc widgetData(pNode);
      widgetData.SetCheckState(XFA_CHECKSTATE_On, bNotify);
      return pNode;
    }
  }
  return nullptr;
}

void CXFA_WidgetAcc::SetSelectedMemberByValue(const WideStringView& wsValue,
                                              bool bNotify,
                                              bool bScriptModify,
                                              bool bSyncData) {
  WideString wsExclGroup;
  for (CXFA_Node* pNode = m_pNode->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Field)
      continue;

    CXFA_Items* pItem =
        pNode->GetChild<CXFA_Items>(0, XFA_Element::Items, false);
    if (!pItem)
      continue;

    CXFA_Node* pItemchild = pItem->GetFirstChild();
    if (!pItemchild)
      continue;

    WideString wsChildValue = pItemchild->JSObject()->GetContent(false);
    if (wsValue != wsChildValue) {
      pItemchild = pItemchild->GetNextSibling();
      if (pItemchild)
        wsChildValue = pItemchild->JSObject()->GetContent(false);
      else
        wsChildValue.clear();
    } else {
      wsExclGroup = wsValue;
    }
    pNode->JSObject()->SetContent(wsChildValue, wsChildValue, bNotify,
                                  bScriptModify, false);
  }
  if (m_pNode) {
    m_pNode->JSObject()->SetContent(wsExclGroup, wsExclGroup, bNotify,
                                    bScriptModify, bSyncData);
  }
}

CXFA_Node* CXFA_WidgetAcc::GetExclGroupFirstMember() {
  CXFA_Node* pExcl = GetNode();
  if (!pExcl)
    return nullptr;

  CXFA_Node* pNode = pExcl->GetFirstChild();
  while (pNode) {
    if (pNode->GetElementType() == XFA_Element::Field)
      return pNode;

    pNode = pNode->GetNextSibling();
  }
  return nullptr;
}
CXFA_Node* CXFA_WidgetAcc::GetExclGroupNextMember(CXFA_Node* pNode) {
  if (!pNode)
    return nullptr;

  CXFA_Node* pNodeField = pNode->GetNextSibling();
  while (pNodeField) {
    if (pNodeField->GetElementType() == XFA_Element::Field)
      return pNodeField;

    pNodeField = pNodeField->GetNextSibling();
  }
  return nullptr;
}

bool CXFA_WidgetAcc::IsChoiceListCommitOnSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::CommitOn) ==
           XFA_AttributeEnum::Select;
  }
  return true;
}

bool CXFA_WidgetAcc::IsChoiceListAllowTextEntry() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::TextEntry);
}

bool CXFA_WidgetAcc::IsChoiceListMultiSelect() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::Open) ==
           XFA_AttributeEnum::MultiSelect;
  }
  return false;
}

bool CXFA_WidgetAcc::IsListBox() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return false;

  XFA_AttributeEnum attr = pUIChild->JSObject()->GetEnum(XFA_Attribute::Open);
  return attr == XFA_AttributeEnum::Always ||
         attr == XFA_AttributeEnum::MultiSelect;
}

int32_t CXFA_WidgetAcc::CountChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> pItems;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pNode->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    pItems.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return 0;

  CXFA_Node* pItem = pItems[0];
  if (iCount > 1) {
    bool bItemOneHasSave =
        pItems[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItems[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = pItems[1];
  }
  return pItem->CountChildren(XFA_Element::Unknown, false);
}

Optional<WideString> CXFA_WidgetAcc::GetChoiceListItem(int32_t nIndex,
                                                       bool bSaveValue) {
  std::vector<CXFA_Node*> pItemsArray;
  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pNode->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != XFA_Element::Items)
      continue;

    ++iCount;
    pItemsArray.push_back(pNode);
    if (iCount == 2)
      break;
  }
  if (iCount == 0)
    return {};

  CXFA_Node* pItems = pItemsArray[0];
  if (iCount > 1) {
    bool bItemOneHasSave =
        pItemsArray[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        pItemsArray[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItems = pItemsArray[1];
  }
  if (!pItems)
    return {};

  CXFA_Node* pItem =
      pItems->GetChild<CXFA_Node>(nIndex, XFA_Element::Unknown, false);
  if (pItem)
    return {pItem->JSObject()->GetContent(false)};
  return {};
}

std::vector<WideString> CXFA_WidgetAcc::GetChoiceListItems(bool bSaveValue) {
  std::vector<CXFA_Node*> items;
  for (CXFA_Node* pNode = m_pNode->GetFirstChild(); pNode && items.size() < 2;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == XFA_Element::Items)
      items.push_back(pNode);
  }
  if (items.empty())
    return std::vector<WideString>();

  CXFA_Node* pItem = items.front();
  if (items.size() > 1) {
    bool bItemOneHasSave =
        items[0]->JSObject()->GetBoolean(XFA_Attribute::Save);
    bool bItemTwoHasSave =
        items[1]->JSObject()->GetBoolean(XFA_Attribute::Save);
    if (bItemOneHasSave != bItemTwoHasSave && bSaveValue == bItemTwoHasSave)
      pItem = items[1];
  }

  std::vector<WideString> wsTextArray;
  for (CXFA_Node* pNode = pItem->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    wsTextArray.emplace_back(pNode->JSObject()->GetContent(false));
  }
  return wsTextArray;
}

int32_t CXFA_WidgetAcc::CountSelectedItems() {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (IsListBox() || !IsChoiceListAllowTextEntry())
    return pdfium::CollectionSize<int32_t>(wsValueArray);

  int32_t iSelected = 0;
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    if (pdfium::ContainsValue(wsSaveTextArray, value))
      iSelected++;
  }
  return iSelected;
}

int32_t CXFA_WidgetAcc::GetSelectedItem(int32_t nIndex) {
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  if (!pdfium::IndexInBounds(wsValueArray, nIndex))
    return -1;

  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(),
                      wsValueArray[nIndex]);
  return it != wsSaveTextArray.end() ? it - wsSaveTextArray.begin() : -1;
}

std::vector<int32_t> CXFA_WidgetAcc::GetSelectedItems() {
  std::vector<int32_t> iSelArray;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  for (const auto& value : wsValueArray) {
    auto it = std::find(wsSaveTextArray.begin(), wsSaveTextArray.end(), value);
    if (it != wsSaveTextArray.end())
      iSelArray.push_back(it - wsSaveTextArray.begin());
  }
  return iSelArray;
}

std::vector<WideString> CXFA_WidgetAcc::GetSelectedItemsValue() {
  std::vector<WideString> wsSelTextArray;
  WideString wsValue = m_pNode->GetRawValue();
  if (IsChoiceListMultiSelect()) {
    if (!wsValue.IsEmpty()) {
      size_t iStart = 0;
      size_t iLength = wsValue.GetLength();
      auto iEnd = wsValue.Find(L'\n', iStart);
      iEnd = (!iEnd.has_value()) ? iLength : iEnd;
      while (iEnd >= iStart) {
        wsSelTextArray.push_back(wsValue.Mid(iStart, iEnd.value() - iStart));
        iStart = iEnd.value() + 1;
        if (iStart >= iLength)
          break;
        iEnd = wsValue.Find(L'\n', iStart);
        if (!iEnd.has_value())
          wsSelTextArray.push_back(wsValue.Mid(iStart, iLength - iStart));
      }
    }
  } else {
    wsSelTextArray.push_back(wsValue);
  }
  return wsSelTextArray;
}

bool CXFA_WidgetAcc::GetItemState(int32_t nIndex) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  return pdfium::IndexInBounds(wsSaveTextArray, nIndex) &&
         pdfium::ContainsValue(GetSelectedItemsValue(),
                               wsSaveTextArray[nIndex]);
}

void CXFA_WidgetAcc::SetItemState(int32_t nIndex,
                                  bool bSelected,
                                  bool bNotify,
                                  bool bScriptModify,
                                  bool bSyncData) {
  std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
  if (!pdfium::IndexInBounds(wsSaveTextArray, nIndex))
    return;

  int32_t iSel = -1;
  std::vector<WideString> wsValueArray = GetSelectedItemsValue();
  auto it = std::find(wsValueArray.begin(), wsValueArray.end(),
                      wsSaveTextArray[nIndex]);
  if (it != wsValueArray.end())
    iSel = it - wsValueArray.begin();

  if (IsChoiceListMultiSelect()) {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsValue = m_pNode->GetRawValue();
        if (!wsValue.IsEmpty()) {
          wsValue += L"\n";
        }
        wsValue += wsSaveTextArray[nIndex];
        m_pNode->JSObject()->SetContent(wsValue, wsValue, bNotify,
                                        bScriptModify, bSyncData);
      }
    } else if (iSel >= 0) {
      std::vector<int32_t> iSelArray = GetSelectedItems();
      auto it = std::find(iSelArray.begin(), iSelArray.end(), nIndex);
      if (it != iSelArray.end())
        iSelArray.erase(it);
      SetSelectedItems(iSelArray, bNotify, bScriptModify, bSyncData);
    }
  } else {
    if (bSelected) {
      if (iSel < 0) {
        WideString wsSaveText = wsSaveTextArray[nIndex];
        m_pNode->JSObject()->SetContent(wsSaveText,
                                        GetFormatDataValue(wsSaveText), bNotify,
                                        bScriptModify, bSyncData);
      }
    } else if (iSel >= 0) {
      m_pNode->JSObject()->SetContent(WideString(), WideString(), bNotify,
                                      bScriptModify, bSyncData);
    }
  }
}

void CXFA_WidgetAcc::SetSelectedItems(const std::vector<int32_t>& iSelArray,
                                      bool bNotify,
                                      bool bScriptModify,
                                      bool bSyncData) {
  WideString wsValue;
  int32_t iSize = pdfium::CollectionSize<int32_t>(iSelArray);
  if (iSize >= 1) {
    std::vector<WideString> wsSaveTextArray = GetChoiceListItems(true);
    WideString wsItemValue;
    for (int32_t i = 0; i < iSize; i++) {
      wsItemValue = (iSize == 1) ? wsSaveTextArray[iSelArray[i]]
                                 : wsSaveTextArray[iSelArray[i]] + L"\n";
      wsValue += wsItemValue;
    }
  }
  WideString wsFormat(wsValue);
  if (!IsChoiceListMultiSelect())
    wsFormat = GetFormatDataValue(wsValue);

  m_pNode->JSObject()->SetContent(wsValue, wsFormat, bNotify, bScriptModify,
                                  bSyncData);
}

void CXFA_WidgetAcc::ClearAllSelections() {
  CXFA_Node* pBind = m_pNode->GetBindData();
  if (!pBind || !IsChoiceListMultiSelect()) {
    m_pNode->SyncValue(WideString(), false);
    return;
  }

  while (CXFA_Node* pChildNode = pBind->GetFirstChild())
    pBind->RemoveChild(pChildNode, true);
}

void CXFA_WidgetAcc::InsertItem(const WideString& wsLabel,
                                const WideString& wsValue,
                                bool bNotify) {
  int32_t nIndex = -1;
  WideString wsNewValue(wsValue);
  if (wsNewValue.IsEmpty())
    wsNewValue = wsLabel;

  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItem = m_pNode->GetFirstChild(); pItem;
       pItem = pItem->GetNextSibling()) {
    if (pItem->GetElementType() == XFA_Element::Items)
      listitems.push_back(pItem);
  }
  if (listitems.empty()) {
    CXFA_Node* pItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pItems);
    InsertListTextItem(pItems, wsLabel, nIndex);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  } else if (listitems.size() > 1) {
    for (int32_t i = 0; i < 2; i++) {
      CXFA_Node* pNode = listitems[i];
      bool bHasSave = pNode->JSObject()->GetBoolean(XFA_Attribute::Save);
      if (bHasSave)
        InsertListTextItem(pNode, wsNewValue, nIndex);
      else
        InsertListTextItem(pNode, wsLabel, nIndex);
    }
  } else {
    CXFA_Node* pNode = listitems[0];
    pNode->JSObject()->SetBoolean(XFA_Attribute::Save, false, false);
    pNode->JSObject()->SetEnum(XFA_Attribute::Presence,
                               XFA_AttributeEnum::Visible, false);
    CXFA_Node* pSaveItems = m_pNode->CreateSamePacketNode(XFA_Element::Items);
    m_pNode->InsertChild(-1, pSaveItems);
    pSaveItems->JSObject()->SetBoolean(XFA_Attribute::Save, true, false);
    pSaveItems->JSObject()->SetEnum(XFA_Attribute::Presence,
                                    XFA_AttributeEnum::Hidden, false);
    CXFA_Node* pListNode = pNode->GetFirstChild();
    int32_t i = 0;
    while (pListNode) {
      InsertListTextItem(pSaveItems, pListNode->JSObject()->GetContent(false),
                         i);
      ++i;

      pListNode = pListNode->GetNextSibling();
    }
    InsertListTextItem(pNode, wsLabel, nIndex);
    InsertListTextItem(pSaveItems, wsNewValue, nIndex);
  }
  if (!bNotify)
    return;

  m_pNode->GetDocument()->GetNotify()->OnWidgetListItemAdded(
      this, wsLabel.c_str(), wsValue.c_str(), nIndex);
}

void CXFA_WidgetAcc::GetItemLabel(const WideStringView& wsValue,
                                  WideString& wsLabel) {
  int32_t iCount = 0;
  std::vector<CXFA_Node*> listitems;
  CXFA_Node* pItems = m_pNode->GetFirstChild();
  for (; pItems; pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    listitems.push_back(pItems);
  }

  if (iCount <= 1) {
    wsLabel = wsValue;
    return;
  }

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }
  iCount = 0;

  int32_t iSearch = -1;
  for (CXFA_Node* pChildItem = pSaveItems->GetFirstChild(); pChildItem;
       pChildItem = pChildItem->GetNextSibling()) {
    if (pChildItem->JSObject()->GetContent(false) == wsValue) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return;

  CXFA_Node* pText =
      pLabelItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  if (pText)
    wsLabel = pText->JSObject()->GetContent(false);
}

WideString CXFA_WidgetAcc::GetItemValue(const WideStringView& wsLabel) {
  int32_t iCount = 0;
  std::vector<CXFA_Node*> listitems;
  for (CXFA_Node* pItems = m_pNode->GetFirstChild(); pItems;
       pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;
    iCount++;
    listitems.push_back(pItems);
  }
  if (iCount <= 1)
    return WideString(wsLabel);

  CXFA_Node* pLabelItems = listitems[0];
  bool bSave = pLabelItems->JSObject()->GetBoolean(XFA_Attribute::Save);
  CXFA_Node* pSaveItems = nullptr;
  if (bSave) {
    pSaveItems = pLabelItems;
    pLabelItems = listitems[1];
  } else {
    pSaveItems = listitems[1];
  }
  iCount = 0;

  int32_t iSearch = -1;
  WideString wsContent;
  CXFA_Node* pChildItem = pLabelItems->GetFirstChild();
  for (; pChildItem; pChildItem = pChildItem->GetNextSibling()) {
    if (pChildItem->JSObject()->GetContent(false) == wsLabel) {
      iSearch = iCount;
      break;
    }
    iCount++;
  }
  if (iSearch < 0)
    return L"";

  CXFA_Node* pText =
      pSaveItems->GetChild<CXFA_Node>(iSearch, XFA_Element::Unknown, false);
  return pText ? pText->JSObject()->GetContent(false) : L"";
}

bool CXFA_WidgetAcc::DeleteItem(int32_t nIndex,
                                bool bNotify,
                                bool bScriptModify) {
  bool bSetValue = false;
  CXFA_Node* pItems = m_pNode->GetFirstChild();
  for (; pItems; pItems = pItems->GetNextSibling()) {
    if (pItems->GetElementType() != XFA_Element::Items)
      continue;

    if (nIndex < 0) {
      while (CXFA_Node* pNode = pItems->GetFirstChild()) {
        pItems->RemoveChild(pNode, true);
      }
    } else {
      if (!bSetValue && pItems->JSObject()->GetBoolean(XFA_Attribute::Save)) {
        SetItemState(nIndex, false, true, bScriptModify, true);
        bSetValue = true;
      }
      int32_t i = 0;
      CXFA_Node* pNode = pItems->GetFirstChild();
      while (pNode) {
        if (i == nIndex) {
          pItems->RemoveChild(pNode, true);
          break;
        }
        i++;
        pNode = pNode->GetNextSibling();
      }
    }
  }
  if (bNotify)
    m_pNode->GetDocument()->GetNotify()->OnWidgetListItemRemoved(this, nIndex);
  return true;
}

bool CXFA_WidgetAcc::IsHorizontalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::HScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

bool CXFA_WidgetAcc::IsVerticalScrollPolicyOff() {
  CXFA_Node* pUIChild = GetUIChild();
  if (pUIChild) {
    return pUIChild->JSObject()->GetEnum(XFA_Attribute::VScrollPolicy) ==
           XFA_AttributeEnum::Off;
  }
  return false;
}

Optional<int32_t> CXFA_WidgetAcc::GetNumberOfCells() {
  CXFA_Node* pUIChild = GetUIChild();
  if (!pUIChild)
    return {};
  if (CXFA_Comb* pNode =
          pUIChild->GetChild<CXFA_Comb>(0, XFA_Element::Comb, false))
    return {pNode->JSObject()->GetInteger(XFA_Attribute::NumberOfCells)};
  return {};
}

WideString CXFA_WidgetAcc::GetPasswordChar() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild ? pUIChild->JSObject()->GetCData(XFA_Attribute::PasswordChar)
                  : L"*";
}

bool CXFA_WidgetAcc::IsMultiLine() {
  CXFA_Node* pUIChild = GetUIChild();
  return pUIChild && pUIChild->JSObject()->GetBoolean(XFA_Attribute::MultiLine);
}

std::pair<XFA_Element, int32_t> CXFA_WidgetAcc::GetMaxChars() {
  if (CXFA_Value* pNode =
          m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false)) {
    if (CXFA_Node* pChild = pNode->GetFirstChild()) {
      switch (pChild->GetElementType()) {
        case XFA_Element::Text:
          return {XFA_Element::Text,
                  pChild->JSObject()->GetInteger(XFA_Attribute::MaxChars)};
        case XFA_Element::ExData: {
          int32_t iMax =
              pChild->JSObject()->GetInteger(XFA_Attribute::MaxLength);
          return {XFA_Element::ExData, iMax < 0 ? 0 : iMax};
        }
        default:
          break;
      }
    }
  }
  return {XFA_Element::Unknown, 0};
}

int32_t CXFA_WidgetAcc::GetFracDigits() {
  CXFA_Value* pNode =
      m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Decimal* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::FracDigits, true)
      .value_or(-1);
}

int32_t CXFA_WidgetAcc::GetLeadDigits() {
  CXFA_Value* pNode =
      m_pNode->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
  if (!pNode)
    return -1;

  CXFA_Decimal* pChild =
      pNode->GetChild<CXFA_Decimal>(0, XFA_Element::Decimal, false);
  if (!pChild)
    return -1;

  return pChild->JSObject()
      ->TryInteger(XFA_Attribute::LeadDigits, true)
      .value_or(-1);
}

bool CXFA_WidgetAcc::SetValue(XFA_VALUEPICTURE eValueType,
                              const WideString& wsValue) {
  if (wsValue.IsEmpty()) {
    if (m_pNode)
      m_pNode->SyncValue(wsValue, true);
    return true;
  }

  m_bPreNull = m_bIsNull;
  m_bIsNull = false;
  WideString wsNewText(wsValue);
  WideString wsPicture = GetPictureContent(eValueType);
  bool bValidate = true;
  bool bSyncData = false;
  CXFA_Node* pNode = GetUIChild();
  if (!pNode)
    return true;

  XFA_Element eType = pNode->GetElementType();
  if (!wsPicture.IsEmpty()) {
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    IFX_Locale* pLocale = GetLocale();
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(GetNode());
    bValidate =
        widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture);
    if (bValidate) {
      widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsNewText,
                                     wsPicture, pLocale, pLocalMgr);
      wsNewText = widgetValue.GetValue();
      if (eType == XFA_Element::NumericEdit)
        wsNewText = NumericLimit(wsNewText, GetLeadDigits(), GetFracDigits());

      bSyncData = true;
    }
  } else {
    if (eType == XFA_Element::NumericEdit) {
      if (wsNewText != L"0")
        wsNewText = NumericLimit(wsNewText, GetLeadDigits(), GetFracDigits());

      bSyncData = true;
    }
  }
  if (eType != XFA_Element::NumericEdit || bSyncData) {
    if (m_pNode)
      m_pNode->SyncValue(wsNewText, true);
  }

  return bValidate;
}

WideString CXFA_WidgetAcc::GetPictureContent(XFA_VALUEPICTURE ePicture) {
  if (ePicture == XFA_VALUEPICTURE_Raw)
    return L"";

  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(GetNode());
  switch (ePicture) {
    case XFA_VALUEPICTURE_Display: {
      if (CXFA_Format* pFormat =
              m_pNode->GetChild<CXFA_Format>(0, XFA_Element::Format, false)) {
        if (CXFA_Picture* pPicture = pFormat->GetChild<CXFA_Picture>(
                0, XFA_Element::Picture, false)) {
          Optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
          if (picture)
            return *picture;
        }
      }

      IFX_Locale* pLocale = GetLocale();
      if (!pLocale)
        return L"";

      uint32_t dwType = widgetValue.GetType();
      switch (dwType) {
        case XFA_VT_DATE:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_TIME:
          return pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_DATETIME:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium) +
                 L"T" +
                 pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Medium);
        case XFA_VT_DECIMAL:
        case XFA_VT_FLOAT:
        default:
          return L"";
      }
    }
    case XFA_VALUEPICTURE_Edit: {
      CXFA_Ui* pUI = m_pNode->GetChild<CXFA_Ui>(0, XFA_Element::Ui, false);
      if (pUI) {
        if (CXFA_Picture* pPicture =
                pUI->GetChild<CXFA_Picture>(0, XFA_Element::Picture, false)) {
          Optional<WideString> picture =
              pPicture->JSObject()->TryContent(false, true);
          if (picture)
            return *picture;
        }
      }

      IFX_Locale* pLocale = GetLocale();
      if (!pLocale)
        return L"";

      uint32_t dwType = widgetValue.GetType();
      switch (dwType) {
        case XFA_VT_DATE:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        case XFA_VT_TIME:
          return pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        case XFA_VT_DATETIME:
          return pLocale->GetDatePattern(FX_LOCALEDATETIMESUBCATEGORY_Short) +
                 L"T" +
                 pLocale->GetTimePattern(FX_LOCALEDATETIMESUBCATEGORY_Short);
        default:
          return L"";
      }
    }
    case XFA_VALUEPICTURE_DataBind: {
      CXFA_Bind* bind = m_pNode->GetBindIfExists();
      if (bind)
        return bind->GetPicture();
      break;
    }
    default:
      break;
  }
  return L"";
}

IFX_Locale* CXFA_WidgetAcc::GetLocale() {
  return m_pNode ? m_pNode->GetLocale() : nullptr;
}

WideString CXFA_WidgetAcc::GetValue(XFA_VALUEPICTURE eValueType) {
  WideString wsValue = m_pNode->JSObject()->GetContent(false);

  if (eValueType == XFA_VALUEPICTURE_Display)
    GetItemLabel(wsValue.AsStringView(), wsValue);

  WideString wsPicture = GetPictureContent(eValueType);
  CXFA_Node* pNode = GetUIChild();
  if (!pNode)
    return wsValue;

  switch (GetUIChild()->GetElementType()) {
    case XFA_Element::ChoiceList: {
      if (eValueType == XFA_VALUEPICTURE_Display) {
        int32_t iSelItemIndex = GetSelectedItem(0);
        if (iSelItemIndex >= 0) {
          wsValue = GetChoiceListItem(iSelItemIndex, false).value_or(L"");
          wsPicture.clear();
        }
      }
    } break;
    case XFA_Element::NumericEdit:
      if (eValueType != XFA_VALUEPICTURE_Raw && wsPicture.IsEmpty()) {
        IFX_Locale* pLocale = GetLocale();
        if (eValueType == XFA_VALUEPICTURE_Display && pLocale)
          wsValue = FormatNumStr(NormalizeNumStr(wsValue), pLocale);
      }
      break;
    default:
      break;
  }
  if (wsPicture.IsEmpty())
    return wsValue;

  if (IFX_Locale* pLocale = GetLocale()) {
    CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(GetNode());
    CXFA_LocaleMgr* pLocalMgr = m_pNode->GetDocument()->GetLocalMgr();
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      case XFA_VT_TIME: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsValue, wsPicture, pLocale, eValueType))
            return wsValue;
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsValue, wsPicture, pLocale, eValueType);
  }
  return wsValue;
}

WideString CXFA_WidgetAcc::GetNormalizeDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsPicture = GetPictureContent(XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  ASSERT(GetNode());
  CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
  IFX_Locale* pLocale = GetLocale();
  CXFA_LocaleValue widgetValue = XFA_GetLocaleValue(GetNode());
  if (widgetValue.ValidateValue(wsValue, wsPicture, pLocale, &wsPicture)) {
    widgetValue = CXFA_LocaleValue(widgetValue.GetType(), wsValue, wsPicture,
                                   pLocale, pLocalMgr);
    return widgetValue.GetValue();
  }
  return wsValue;
}

WideString CXFA_WidgetAcc::GetFormatDataValue(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsPicture = GetPictureContent(XFA_VALUEPICTURE_DataBind);
  if (wsPicture.IsEmpty())
    return wsValue;

  WideString wsFormattedValue = wsValue;
  if (IFX_Locale* pLocale = GetLocale()) {
    ASSERT(GetNode());
    CXFA_Value* pNodeValue =
        GetNode()->GetChild<CXFA_Value>(0, XFA_Element::Value, false);
    if (!pNodeValue)
      return wsValue;

    CXFA_Node* pValueChild = pNodeValue->GetFirstChild();
    if (!pValueChild)
      return wsValue;

    int32_t iVTType = XFA_VT_NULL;
    switch (pValueChild->GetElementType()) {
      case XFA_Element::Decimal:
        iVTType = XFA_VT_DECIMAL;
        break;
      case XFA_Element::Float:
        iVTType = XFA_VT_FLOAT;
        break;
      case XFA_Element::Date:
        iVTType = XFA_VT_DATE;
        break;
      case XFA_Element::Time:
        iVTType = XFA_VT_TIME;
        break;
      case XFA_Element::DateTime:
        iVTType = XFA_VT_DATETIME;
        break;
      case XFA_Element::Boolean:
        iVTType = XFA_VT_BOOLEAN;
        break;
      case XFA_Element::Integer:
        iVTType = XFA_VT_INTEGER;
        break;
      case XFA_Element::Text:
        iVTType = XFA_VT_TEXT;
        break;
      default:
        iVTType = XFA_VT_NULL;
        break;
    }
    CXFA_LocaleMgr* pLocalMgr = GetNode()->GetDocument()->GetLocalMgr();
    CXFA_LocaleValue widgetValue(iVTType, wsValue, pLocalMgr);
    switch (widgetValue.GetType()) {
      case XFA_VT_DATE: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue date(XFA_VT_DATE, wsDate, pLocalMgr);
          if (date.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      case XFA_VT_TIME: {
        WideString wsDate, wsTime;
        if (SplitDateTime(wsValue, wsDate, wsTime)) {
          CXFA_LocaleValue time(XFA_VT_TIME, wsTime, pLocalMgr);
          if (time.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                                  XFA_VALUEPICTURE_DataBind)) {
            return wsFormattedValue;
          }
        }
        break;
      }
      default:
        break;
    }
    widgetValue.FormatPatterns(wsFormattedValue, wsPicture, pLocale,
                               XFA_VALUEPICTURE_DataBind);
  }
  return wsFormattedValue;
}

WideString CXFA_WidgetAcc::NormalizeNumStr(const WideString& wsValue) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsOutput = wsValue;
  wsOutput.TrimLeft('0');

  if (!wsOutput.IsEmpty() && wsOutput.Contains('.') && GetFracDigits() != -1) {
    wsOutput.TrimRight(L"0");
    wsOutput.TrimRight(L".");
  }
  if (wsOutput.IsEmpty() || wsOutput[0] == '.')
    wsOutput.InsertAtFront('0');

  return wsOutput;
}

WideString CXFA_WidgetAcc::FormatNumStr(const WideString& wsValue,
                                        IFX_Locale* pLocale) {
  if (wsValue.IsEmpty())
    return L"";

  WideString wsSrcNum = wsValue;
  WideString wsGroupSymbol =
      pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Grouping);
  bool bNeg = false;
  if (wsSrcNum[0] == '-') {
    bNeg = true;
    wsSrcNum.Delete(0, 1);
  }

  auto dot_index = wsSrcNum.Find('.');
  dot_index = !dot_index.has_value() ? wsSrcNum.GetLength() : dot_index;

  if (dot_index.value() < 1)
    return L"";

  size_t nPos = dot_index.value() % 3;
  WideString wsOutput;
  for (size_t i = 0; i < dot_index.value(); i++) {
    if (i % 3 == nPos && i != 0)
      wsOutput += wsGroupSymbol;

    wsOutput += wsSrcNum[i];
  }
  if (dot_index.value() < wsSrcNum.GetLength()) {
    wsOutput += pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Decimal);
    wsOutput += wsSrcNum.Right(wsSrcNum.GetLength() - dot_index.value() - 1);
  }
  if (bNeg)
    return pLocale->GetNumbericSymbol(FX_LOCALENUMSYMBOL_Minus) + wsOutput;

  return wsOutput;
}

void CXFA_WidgetAcc::InsertListTextItem(CXFA_Node* pItems,
                                        const WideString& wsText,
                                        int32_t nIndex) {
  CXFA_Node* pText = pItems->CreateSamePacketNode(XFA_Element::Text);
  pItems->InsertChild(nIndex, pText);
  pText->JSObject()->SetContent(wsText, wsText, false, false, false);
}

WideString CXFA_WidgetAcc::NumericLimit(const WideString& wsValue,
                                        int32_t iLead,
                                        int32_t iTread) const {
  if ((iLead == -1) && (iTread == -1))
    return wsValue;

  WideString wsRet;
  int32_t iLead_ = 0, iTread_ = -1;
  int32_t iCount = wsValue.GetLength();
  if (iCount == 0)
    return wsValue;

  int32_t i = 0;
  if (wsValue[i] == L'-') {
    wsRet += L'-';
    i++;
  }
  for (; i < iCount; i++) {
    wchar_t wc = wsValue[i];
    if (FXSYS_isDecimalDigit(wc)) {
      if (iLead >= 0) {
        iLead_++;
        if (iLead_ > iLead)
          return L"0";
      } else if (iTread_ >= 0) {
        iTread_++;
        if (iTread_ > iTread) {
          if (iTread != -1) {
            CFX_Decimal wsDeci = CFX_Decimal(wsValue.AsStringView());
            wsDeci.SetScale(iTread);
            wsRet = wsDeci;
          }
          return wsRet;
        }
      }
    } else if (wc == L'.') {
      iTread_ = 0;
      iLead = -1;
    }
    wsRet += wc;
  }
  return wsRet;
}
