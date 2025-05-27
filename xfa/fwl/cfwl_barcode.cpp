// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_barcode.h"

#include "fxbarcode/cfx_barcode.h"
#include "xfa/fgas/font/cfgas_gefont.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_themepart.h"
#include "xfa/fwl/ifwl_themeprovider.h"
#include "xfa/fwl/theme/cfwl_utils.h"

namespace pdfium {

CFWL_Barcode::CFWL_Barcode(CFWL_App* app)
    : CFWL_Edit(app, Properties(), nullptr) {}

CFWL_Barcode::~CFWL_Barcode() = default;

FWL_Type CFWL_Barcode::GetClassID() const {
  return FWL_Type::Barcode;
}

void CFWL_Barcode::Update() {
  if (IsLocked()) {
    return;
  }

  CFWL_Edit::Update();
  GenerateBarcodeImageCache();
}

void CFWL_Barcode::DrawWidget(CFGAS_GEGraphics* pGraphics,
                              const CFX_Matrix& matrix) {
  if (!pGraphics) {
    return;
  }

  if ((properties_.states_ & FWL_STATE_WGT_Focused) == 0) {
    GenerateBarcodeImageCache();
    if (!barcode_engine_ || status_ != Status::kEncodeSuccess) {
      return;
    }

    CFX_Matrix mt;
    mt.e = GetRTClient().left;
    mt.f = GetRTClient().top;
    mt.Concat(matrix);

    // TODO(tsepez): Curious as to why |mt| is unused?
    barcode_engine_->RenderDevice(pGraphics->GetRenderDevice(), matrix);
    return;
  }
  CFWL_Edit::DrawWidget(pGraphics, matrix);
}

void CFWL_Barcode::SetType(BC_TYPE type) {
  if (type_ == type) {
    return;
  }

  barcode_engine_.reset();
  type_ = type;
  status_ = Status::kNeedUpdate;
}

void CFWL_Barcode::SetText(const WideString& wsText) {
  barcode_engine_.reset();
  status_ = Status::kNeedUpdate;
  CFWL_Edit::SetText(wsText);
}

void CFWL_Barcode::SetTextSkipNotify(const WideString& wsText) {
  barcode_engine_.reset();
  status_ = Status::kNeedUpdate;
  CFWL_Edit::SetTextSkipNotify(wsText);
}

bool CFWL_Barcode::IsProtectedType() const {
  if (!barcode_engine_) {
    return true;
  }

  BC_TYPE tEngineType = barcode_engine_->GetType();
  return tEngineType == BC_TYPE::kQRCode || tEngineType == BC_TYPE::kPDF417 ||
         tEngineType == BC_TYPE::kDataMatrix;
}

void CFWL_Barcode::OnProcessEvent(CFWL_Event* pEvent) {
  if (pEvent->GetType() == CFWL_Event::Type::TextWillChange) {
    barcode_engine_.reset();
    status_ = Status::kNeedUpdate;
  }
  CFWL_Edit::OnProcessEvent(pEvent);
}

void CFWL_Barcode::SetModuleHeight(int32_t height) {
  module_height_ = height;
}

void CFWL_Barcode::SetModuleWidth(int32_t width) {
  module_width_ = width;
}

void CFWL_Barcode::SetDataLength(int32_t dataLength) {
  data_length_ = dataLength;
  SetLimit(dataLength);
}

void CFWL_Barcode::SetCalChecksum(bool calChecksum) {
  cal_checksum_ = calChecksum;
}

void CFWL_Barcode::SetPrintChecksum(bool printChecksum) {
  print_checksum_ = printChecksum;
}

void CFWL_Barcode::SetTextLocation(BC_TEXT_LOC location) {
  text_location_ = location;
}

void CFWL_Barcode::SetWideNarrowRatio(int8_t ratio) {
  wide_narrow_ratio_ = ratio;
}

void CFWL_Barcode::SetStartChar(char startChar) {
  start_char_ = startChar;
}

void CFWL_Barcode::SetEndChar(char endChar) {
  end_char_ = endChar;
}

void CFWL_Barcode::SetErrorCorrectionLevel(int32_t ecLevel) {
  eclevel_ = ecLevel;
}

void CFWL_Barcode::GenerateBarcodeImageCache() {
  if (status_ != Status::kNeedUpdate) {
    return;
  }

  status_ = Status::kNormal;
  CreateBarcodeEngine();
  if (!barcode_engine_) {
    return;
  }

  IFWL_ThemeProvider* pTheme = GetThemeProvider();
  CFWL_ThemePart part(CFWL_ThemePart::Part::kNone, this);
  if (RetainPtr<CFGAS_GEFont> font = pTheme->GetFont(part)) {
    if (CFX_Font* pCXFont = font->GetDevFont()) {
      barcode_engine_->SetFont(pCXFont);
    }
  }
  barcode_engine_->SetFontSize(pTheme->GetFontSize(part));
  barcode_engine_->SetFontColor(pTheme->GetTextColor(part));
  barcode_engine_->SetHeight(int32_t(GetRTClient().height));
  barcode_engine_->SetWidth(int32_t(GetRTClient().width));
  if (module_height_.has_value()) {
    barcode_engine_->SetModuleHeight(module_height_.value());
  }
  if (module_width_.has_value()) {
    barcode_engine_->SetModuleWidth(module_width_.value());
  }
  if (data_length_.has_value()) {
    barcode_engine_->SetDataLength(data_length_.value());
  }
  if (cal_checksum_.has_value()) {
    barcode_engine_->SetCalChecksum(cal_checksum_.value());
  }
  if (print_checksum_.has_value()) {
    barcode_engine_->SetPrintChecksum(print_checksum_.value());
  }
  if (text_location_.has_value()) {
    barcode_engine_->SetTextLocation(text_location_.value());
  }
  if (wide_narrow_ratio_.has_value()) {
    barcode_engine_->SetWideNarrowRatio(wide_narrow_ratio_.value());
  }
  if (start_char_.has_value()) {
    barcode_engine_->SetStartChar(start_char_.value());
  }
  if (end_char_.has_value()) {
    barcode_engine_->SetEndChar(end_char_.value());
  }
  if (eclevel_.has_value()) {
    barcode_engine_->SetErrorCorrectionLevel(eclevel_.value());
  }

  status_ = barcode_engine_->Encode(GetText().AsStringView())
                ? Status::kEncodeSuccess
                : Status::kNormal;
}

void CFWL_Barcode::CreateBarcodeEngine() {
  if (barcode_engine_ || type_ == BC_TYPE::kUnknown) {
    return;
  }

  barcode_engine_ = CFX_Barcode::Create(type_);
}

}  // namespace pdfium
