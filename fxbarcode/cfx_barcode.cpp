// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxbarcode/cfx_barcode.h"

#include <memory>

#include "core/fxcrt/notreached.h"
#include "core/fxcrt/ptr_util.h"
#include "fxbarcode/cbc_codabar.h"
#include "fxbarcode/cbc_code128.h"
#include "fxbarcode/cbc_code39.h"
#include "fxbarcode/cbc_codebase.h"
#include "fxbarcode/cbc_datamatrix.h"
#include "fxbarcode/cbc_ean13.h"
#include "fxbarcode/cbc_ean8.h"
#include "fxbarcode/cbc_pdf417i.h"
#include "fxbarcode/cbc_qrcode.h"
#include "fxbarcode/cbc_upca.h"

namespace {

std::unique_ptr<CBC_CodeBase> CreateBarCodeEngineObject(BC_TYPE type) {
  switch (type) {
    case BC_TYPE::kCode39:
      return std::make_unique<CBC_Code39>();
    case BC_TYPE::kCodabar:
      return std::make_unique<CBC_Codabar>();
    case BC_TYPE::kCode128:
      return std::make_unique<CBC_Code128>(BC_TYPE::kCode128B);
    case BC_TYPE::kCode128B:
      return std::make_unique<CBC_Code128>(BC_TYPE::kCode128B);
    case BC_TYPE::kCode128C:
      return std::make_unique<CBC_Code128>(BC_TYPE::kCode128C);
    case BC_TYPE::kEAN8:
      return std::make_unique<CBC_EAN8>();
    case BC_TYPE::kUPCA:
      return std::make_unique<CBC_UPCA>();
    case BC_TYPE::kEAN13:
      return std::make_unique<CBC_EAN13>();
    case BC_TYPE::kQRCode:
      return std::make_unique<CBC_QRCode>();
    case BC_TYPE::kPDF417:
      return std::make_unique<CBC_PDF417I>();
    case BC_TYPE::kDataMatrix:
      return std::make_unique<CBC_DataMatrix>();
    case BC_TYPE::kUnknown:
      return nullptr;
  }
  NOTREACHED();
}

}  // namespace

CFX_Barcode::CFX_Barcode() = default;

CFX_Barcode::~CFX_Barcode() = default;

std::unique_ptr<CFX_Barcode> CFX_Barcode::Create(BC_TYPE type) {
  auto barcode = pdfium::WrapUnique(new CFX_Barcode());  // Private ctor.
  barcode->bc_engine_ = CreateBarCodeEngineObject(type);
  return barcode;
}

BC_TYPE CFX_Barcode::GetType() {
  return bc_engine_ ? bc_engine_->GetType() : BC_TYPE::kUnknown;
}

bool CFX_Barcode::SetModuleHeight(int32_t moduleHeight) {
  return bc_engine_ && bc_engine_->SetModuleHeight(moduleHeight);
}

bool CFX_Barcode::SetModuleWidth(int32_t moduleWidth) {
  return bc_engine_ && bc_engine_->SetModuleWidth(moduleWidth);
}

void CFX_Barcode::SetHeight(int32_t height) {
  if (bc_engine_) {
    bc_engine_->SetHeight(height);
  }
}

void CFX_Barcode::SetWidth(int32_t width) {
  if (bc_engine_) {
    bc_engine_->SetWidth(width);
  }
}

bool CFX_Barcode::SetPrintChecksum(bool checksum) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      static_cast<CBC_OneCode*>(bc_engine_.get())->SetPrintChecksum(checksum);
      return true;
    default:
      return false;
  }
}

bool CFX_Barcode::SetDataLength(int32_t length) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      static_cast<CBC_OneCode*>(bc_engine_.get())->SetDataLength(length);
      return true;
    default:
      return false;
  }
}

bool CFX_Barcode::SetCalChecksum(bool state) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      static_cast<CBC_OneCode*>(bc_engine_.get())->SetCalChecksum(state);
      return true;
    default:
      return false;
  }
}

bool CFX_Barcode::SetFont(CFX_Font* font) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      return static_cast<CBC_OneCode*>(bc_engine_.get())->SetFont(font);
    default:
      return false;
  }
}

bool CFX_Barcode::SetFontSize(float size) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      static_cast<CBC_OneCode*>(bc_engine_.get())->SetFontSize(size);
      return true;
    default:
      return false;
  }
}

bool CFX_Barcode::SetFontColor(FX_ARGB color) {
  if (!bc_engine_) {
    return false;
  }

  switch (GetType()) {
    case BC_TYPE::kCode39:
    case BC_TYPE::kCodabar:
    case BC_TYPE::kCode128:
    case BC_TYPE::kCode128B:
    case BC_TYPE::kCode128C:
    case BC_TYPE::kEAN8:
    case BC_TYPE::kEAN13:
    case BC_TYPE::kUPCA:
      static_cast<CBC_OneCode*>(bc_engine_.get())->SetFontColor(color);
      return true;
    default:
      return false;
  }
}

void CFX_Barcode::SetTextLocation(BC_TEXT_LOC location) {
  if (bc_engine_) {
    bc_engine_->SetTextLocation(location);
  }
}

bool CFX_Barcode::SetWideNarrowRatio(int8_t ratio) {
  return bc_engine_ && bc_engine_->SetWideNarrowRatio(ratio);
}

bool CFX_Barcode::SetStartChar(char start) {
  return bc_engine_ && bc_engine_->SetStartChar(start);
}

bool CFX_Barcode::SetEndChar(char end) {
  return bc_engine_ && bc_engine_->SetEndChar(end);
}

bool CFX_Barcode::SetErrorCorrectionLevel(int32_t level) {
  return bc_engine_ && bc_engine_->SetErrorCorrectionLevel(level);
}

bool CFX_Barcode::Encode(WideStringView contents) {
  return bc_engine_ && bc_engine_->Encode(contents);
}

bool CFX_Barcode::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix& matrix) {
  return bc_engine_ && bc_engine_->RenderDevice(device, matrix);
}
