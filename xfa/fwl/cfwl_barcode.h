// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_BARCODE_H_
#define XFA_FWL_CFWL_BARCODE_H_

#include <stdint.h>

#include <memory>
#include <optional>

#include "fxbarcode/BC_Library.h"
#include "xfa/fwl/cfwl_edit.h"

class CFX_Barcode;

namespace pdfium {

class CFWL_Barcode final : public CFWL_Edit {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_Barcode() override;

  // CFWL_Widget
  FWL_Type GetClassID() const override;
  void Update() override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;

  // CFWL_Edit
  void SetText(const WideString& wsText) override;
  void SetTextSkipNotify(const WideString& wsText) override;

  void SetType(BC_TYPE type);
  bool IsProtectedType() const;

  void SetModuleHeight(int32_t height);
  void SetModuleWidth(int32_t width);
  void SetDataLength(int32_t dataLength);
  void SetCalChecksum(bool calChecksum);
  void SetPrintChecksum(bool printChecksum);
  void SetTextLocation(BC_TEXT_LOC location);
  void SetWideNarrowRatio(int8_t ratio);
  void SetStartChar(char startChar);
  void SetEndChar(char endChar);
  void SetErrorCorrectionLevel(int32_t ecLevel);

 private:
  enum class Status : uint8_t {
    kNormal,
    kNeedUpdate,
    kEncodeSuccess,
  };

  explicit CFWL_Barcode(CFWL_App* pApp);

  void GenerateBarcodeImageCache();
  void CreateBarcodeEngine();

  BC_TYPE type_ = BC_TYPE::kUnknown;
  Status status_ = Status::kNormal;
  std::optional<BC_TEXT_LOC> text_location_;
  std::optional<bool> cal_checksum_;
  std::optional<bool> print_checksum_;
  std::optional<char> start_char_;
  std::optional<char> end_char_;
  std::optional<int8_t> wide_narrow_ratio_;
  std::optional<int32_t> module_height_;
  std::optional<int32_t> module_width_;
  std::optional<int32_t> data_length_;
  std::optional<int32_t> eclevel_;
  std::unique_ptr<CFX_Barcode> barcode_engine_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_Barcode;

#endif  // XFA_FWL_CFWL_BARCODE_H_
