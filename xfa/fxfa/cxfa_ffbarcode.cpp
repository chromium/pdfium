// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_ffbarcode.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "third_party/base/check.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_barcode.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fxfa/cxfa_fffield.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/cxfa_fwladapterwidgetmgr.h"
#include "xfa/fxfa/parser/cxfa_barcode.h"
#include "xfa/fxfa/parser/cxfa_border.h"

namespace {

struct BarCodeInfo {
  uint32_t uHash;        // `pName` hashed as if wide string.
  const char pName[20];  // Inline string data reduces size for small strings.
  BC_TYPE eBCType;
};

const BarCodeInfo kBarCodeData[] = {
    {0x7fb4a18, "ean13", BC_TYPE::kEAN13},
    {0x8d13a3d, "code11", BC_TYPE::kUnknown},
    {0x8d149a8, "code49", BC_TYPE::kUnknown},
    {0x8d16347, "code93", BC_TYPE::kUnknown},
    {0x91a92e2, "upsMaxicode", BC_TYPE::kUnknown},
    {0xa7d48dc, "fim", BC_TYPE::kUnknown},
    {0xb359fe9, "msi", BC_TYPE::kUnknown},
    {0x121f738c, "code2Of5Matrix", BC_TYPE::kUnknown},
    {0x15358616, "ucc128", BC_TYPE::kUnknown},
    {0x1f4bfa05, "rfid", BC_TYPE::kUnknown},
    {0x1fda71bc, "rss14Stacked", BC_TYPE::kUnknown},
    {0x22065087, "ean8add2", BC_TYPE::kUnknown},
    {0x2206508a, "ean8add5", BC_TYPE::kUnknown},
    {0x2278366c, "codabar", BC_TYPE::kCodabar},
    {0x2a039a8d, "telepen", BC_TYPE::kUnknown},
    {0x323ed337, "upcApwcd", BC_TYPE::kUnknown},
    {0x347a1846, "postUSIMB", BC_TYPE::kUnknown},
    {0x391bb836, "code128", BC_TYPE::kCode128},
    {0x398eddaf, "dataMatrix", BC_TYPE::kDataMatrix},
    {0x3cff60a8, "upcEadd2", BC_TYPE::kUnknown},
    {0x3cff60ab, "upcEadd5", BC_TYPE::kUnknown},
    {0x402cb188, "code2Of5Standard", BC_TYPE::kUnknown},
    {0x411764f7, "aztec", BC_TYPE::kUnknown},
    {0x44d4e84c, "ean8", BC_TYPE::kEAN8},
    {0x48468902, "ucc128sscc", BC_TYPE::kUnknown},
    {0x4880aea4, "upcAadd2", BC_TYPE::kUnknown},
    {0x4880aea7, "upcAadd5", BC_TYPE::kUnknown},
    {0x54f18256, "code2Of5Industrial", BC_TYPE::kUnknown},
    {0x58e15f25, "rss14Limited", BC_TYPE::kUnknown},
    {0x5c08d1b9, "postAUSReplyPaid", BC_TYPE::kUnknown},
    {0x5fa700bd, "rss14", BC_TYPE::kUnknown},
    {0x631a7e35, "logmars", BC_TYPE::kUnknown},
    {0x6a236236, "pdf417", BC_TYPE::kPDF417},
    {0x6d098ece, "upcean2", BC_TYPE::kUnknown},
    {0x6d098ed1, "upcean5", BC_TYPE::kUnknown},
    {0x76b04eed, "code3Of9extended", BC_TYPE::kUnknown},
    {0x7c7db84a, "maxicode", BC_TYPE::kUnknown},
    {0x8266f7f7, "ucc128random", BC_TYPE::kUnknown},
    {0x83eca147, "postUSDPBC", BC_TYPE::kUnknown},
    {0x8dd71de0, "postAUSStandard", BC_TYPE::kUnknown},
    {0x98adad85, "plessey", BC_TYPE::kUnknown},
    {0x9f84cce6, "ean13pwcd", BC_TYPE::kUnknown},
    {0xb514fbe9, "upcA", BC_TYPE::kUPCA},
    {0xb514fbed, "upcE", BC_TYPE::kUnknown},
    {0xb5c6a853, "ean13add2", BC_TYPE::kUnknown},
    {0xb5c6a856, "ean13add5", BC_TYPE::kUnknown},
    {0xb81fc512, "postUKRM4SCC", BC_TYPE::kUnknown},
    {0xbad34b22, "code128SSCC", BC_TYPE::kUnknown},
    {0xbfbe0cf6, "postUS5Zip", BC_TYPE::kUnknown},
    {0xc56618e8, "pdf417macro", BC_TYPE::kUnknown},
    {0xca730f8a, "code2Of5Interleaved", BC_TYPE::kUnknown},
    {0xd0097ac6, "rss14Expanded", BC_TYPE::kUnknown},
    {0xd25a0240, "postAUSCust2", BC_TYPE::kUnknown},
    {0xd25a0241, "postAUSCust3", BC_TYPE::kUnknown},
    {0xd53ed3e7, "rss14Truncated", BC_TYPE::kUnknown},
    {0xe72bcd57, "code128A", BC_TYPE::kUnknown},
    {0xe72bcd58, "code128B", BC_TYPE::kCode128B},
    {0xe72bcd59, "code128C", BC_TYPE::kCode128C},
    {0xee83c50f, "rss14StackedOmni", BC_TYPE::kUnknown},
    {0xf2a18f7e, "QRCode", BC_TYPE::kQRCode},
    {0xfaeaf37f, "postUSStandard", BC_TYPE::kUnknown},
    {0xfb48155c, "code3Of9", BC_TYPE::kCode39},
};

absl::optional<BC_CHAR_ENCODING> CharEncodingFromString(
    const WideString& value) {
  if (value.CompareNoCase(L"UTF-16"))
    return BC_CHAR_ENCODING::kUnicode;
  if (value.CompareNoCase(L"UTF-8"))
    return BC_CHAR_ENCODING::kUTF8;
  return absl::nullopt;
}

absl::optional<BC_TEXT_LOC> TextLocFromAttribute(XFA_AttributeValue value) {
  switch (value) {
    case XFA_AttributeValue::None:
      return BC_TEXT_LOC::kNone;
    case XFA_AttributeValue::Above:
      return BC_TEXT_LOC::kAbove;
    case XFA_AttributeValue::Below:
      return BC_TEXT_LOC::kBelow;
    case XFA_AttributeValue::AboveEmbedded:
      return BC_TEXT_LOC::kAboveEmbed;
    case XFA_AttributeValue::BelowEmbedded:
      return BC_TEXT_LOC::kBelowEmbed;
    default:
      return absl::nullopt;
  }
}

}  // namespace.

// static
BC_TYPE CXFA_FFBarcode::GetBarcodeTypeByName(const WideString& wsName) {
  if (wsName.IsEmpty())
    return BC_TYPE::kUnknown;

  auto* it = std::lower_bound(
      std::begin(kBarCodeData), std::end(kBarCodeData),
      FX_HashCode_GetLoweredW(wsName.AsStringView()),
      [](const BarCodeInfo& arg, uint32_t hash) { return arg.uHash < hash; });

  if (it != std::end(kBarCodeData) && wsName.EqualsASCII(it->pName))
    return it->eBCType;

  return BC_TYPE::kUnknown;
}

CXFA_FFBarcode::CXFA_FFBarcode(CXFA_Node* pNode, CXFA_Barcode* barcode)
    : CXFA_FFTextEdit(pNode), barcode_(barcode) {}

CXFA_FFBarcode::~CXFA_FFBarcode() = default;

void CXFA_FFBarcode::Trace(cppgc::Visitor* visitor) const {
  CXFA_FFTextEdit::Trace(visitor);
  visitor->Trace(barcode_);
}

bool CXFA_FFBarcode::LoadWidget() {
  DCHECK(!IsLoaded());

  CFWL_Barcode* pFWLBarcode = cppgc::MakeGarbageCollected<CFWL_Barcode>(
      GetFWLApp()->GetHeap()->GetAllocationHandle(), GetFWLApp());
  SetNormalWidget(pFWLBarcode);
  pFWLBarcode->SetAdapterIface(this);

  CFWL_NoteDriver* pNoteDriver = pFWLBarcode->GetFWLApp()->GetNoteDriver();
  pNoteDriver->RegisterEventTarget(pFWLBarcode, pFWLBarcode);
  m_pOldDelegate = pFWLBarcode->GetDelegate();
  pFWLBarcode->SetDelegate(this);

  {
    CFWL_Widget::ScopedUpdateLock update_lock(pFWLBarcode);
    pFWLBarcode->SetText(m_pNode->GetValue(XFA_ValuePicture::kDisplay));
    UpdateWidgetProperty();
  }

  return CXFA_FFField::LoadWidget();
}

void CXFA_FFBarcode::RenderWidget(CFGAS_GEGraphics* pGS,
                                  const CFX_Matrix& matrix,
                                  HighlightOption highlight) {
  if (!HasVisibleStatus())
    return;

  CFX_Matrix mtRotate = GetRotateMatrix();
  mtRotate.Concat(matrix);

  CXFA_FFWidget::RenderWidget(pGS, mtRotate, highlight);
  DrawBorder(pGS, m_pNode->GetUIBorder(), m_UIRect, mtRotate);
  RenderCaption(pGS, mtRotate);
  CFX_RectF rtWidget = GetNormalWidget()->GetWidgetRect();

  CFX_Matrix mt(1, 0, 0, 1, rtWidget.left, rtWidget.top);
  mt.Concat(mtRotate);
  GetNormalWidget()->DrawWidget(pGS, mt);
}

void CXFA_FFBarcode::UpdateWidgetProperty() {
  CXFA_FFTextEdit::UpdateWidgetProperty();

  BC_TYPE bc_type = GetBarcodeTypeByName(barcode_->GetBarcodeType());
  if (bc_type == BC_TYPE::kUnknown)
    return;

  auto* pBarCodeWidget = static_cast<CFWL_Barcode*>(GetNormalWidget());
  pBarCodeWidget->SetType(bc_type);

  absl::optional<WideString> encoding_string = barcode_->GetCharEncoding();
  if (encoding_string.has_value()) {
    absl::optional<BC_CHAR_ENCODING> encoding =
        CharEncodingFromString(encoding_string.value());
    if (encoding.has_value())
      pBarCodeWidget->SetCharEncoding(encoding.value());
  }

  absl::optional<bool> calcChecksum = barcode_->GetChecksum();
  if (calcChecksum.has_value())
    pBarCodeWidget->SetCalChecksum(calcChecksum.value());

  absl::optional<int32_t> dataLen = barcode_->GetDataLength();
  if (dataLen.has_value())
    pBarCodeWidget->SetDataLength(dataLen.value());

  absl::optional<char> startChar = barcode_->GetStartChar();
  if (startChar.has_value())
    pBarCodeWidget->SetStartChar(startChar.value());

  absl::optional<char> endChar = barcode_->GetEndChar();
  if (endChar.has_value())
    pBarCodeWidget->SetEndChar(endChar.value());

  absl::optional<int32_t> ecLevel = barcode_->GetECLevel();
  if (ecLevel.has_value())
    pBarCodeWidget->SetErrorCorrectionLevel(ecLevel.value());

  absl::optional<int32_t> width = barcode_->GetModuleWidth();
  if (width.has_value())
    pBarCodeWidget->SetModuleWidth(width.value());

  absl::optional<int32_t> height = barcode_->GetModuleHeight();
  if (height.has_value())
    pBarCodeWidget->SetModuleHeight(height.value());

  absl::optional<bool> printCheck = barcode_->GetPrintChecksum();
  if (printCheck.has_value())
    pBarCodeWidget->SetPrintChecksum(printCheck.value());

  absl::optional<XFA_AttributeValue> text_attr = barcode_->GetTextLocation();
  if (text_attr.has_value()) {
    absl::optional<BC_TEXT_LOC> textLoc =
        TextLocFromAttribute(text_attr.value());
    if (textLoc.has_value())
      pBarCodeWidget->SetTextLocation(textLoc.value());
  }

  // Truncated is currently not a supported flag.

  absl::optional<int8_t> ratio = barcode_->GetWideNarrowRatio();
  if (ratio.has_value())
    pBarCodeWidget->SetWideNarrowRatio(ratio.value());

  if (bc_type == BC_TYPE::kCode39 || bc_type == BC_TYPE::kEAN8 ||
      bc_type == BC_TYPE::kEAN13 || bc_type == BC_TYPE::kUPCA) {
    pBarCodeWidget->SetPrintChecksum(true);
  }
}

bool CXFA_FFBarcode::AcceptsFocusOnButtonDown(
    Mask<XFA_FWL_KeyFlag> dwFlags,
    const CFX_PointF& point,
    CFWL_MessageMouse::MouseCommand command) {
  auto* pBarCodeWidget = static_cast<CFWL_Barcode*>(GetNormalWidget());
  if (!pBarCodeWidget || pBarCodeWidget->IsProtectedType())
    return false;
  if (command == CFWL_MessageMouse::MouseCommand::kLeftButtonDown &&
      !m_pNode->IsOpenAccess()) {
    return false;
  }
  return CXFA_FFTextEdit::AcceptsFocusOnButtonDown(dwFlags, point, command);
}
