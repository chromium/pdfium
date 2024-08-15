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

  BC_TYPE m_type = BC_TYPE::kUnknown;
  Status m_eStatus = Status::kNormal;
  std::optional<BC_TEXT_LOC> m_eTextLocation;
  std::optional<bool> m_bCalChecksum;
  std::optional<bool> m_bPrintChecksum;
  std::optional<char> m_cStartChar;
  std::optional<char> m_cEndChar;
  std::optional<int8_t> m_nWideNarrowRatio;
  std::optional<int32_t> m_nModuleHeight;
  std::optional<int32_t> m_nModuleWidth;
  std::optional<int32_t> m_nDataLength;
  std::optional<int32_t> m_nECLevel;
  std::unique_ptr<CFX_Barcode> m_pBarcodeEngine;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_Barcode;

#endif  // XFA_FWL_CFWL_BARCODE_H_
