// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_PARSER_INCLUDE_IPDF_DATA_AVAIL_H_
#define CORE_FPDFAPI_FPDF_PARSER_INCLUDE_IPDF_DATA_AVAIL_H_

#include "core/fxcrt/include/fx_stream.h"
#include "core/fxcrt/include/fx_system.h"

class CPDF_Document;
class CPDF_Object;

class IPDF_DataAvail {
 public:
  // Must match PDF_DATA_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocAvailStatus {
    DataError = -1,        // PDF_DATA_ERROR
    DataNotAvailable = 0,  // PDF_DATA_NOTAVAIL
    DataAvailable = 1,     // PDF_DATA_AVAIL
  };

  // Must match PDF_*LINEAR* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocLinearizationStatus {
    LinearizationUnknown = -1,  // PDF_LINEARIZATION_UNKNOWN
    NotLinearized = 0,          // PDF_NOT_LINEARIZED
    Linearized = 1,             // PDF_LINEARIZED
  };

  // Must match PDF_FORM_* definitions in public/fpdf_dataavail.h, but cannot
  // #include that header. fpdfsdk/fpdf_dataavail.cpp has static_asserts
  // to make sure the two sets of values match.
  enum DocFormStatus {
    FormError = -1,        // PDF_FORM_ERROR
    FormNotAvailable = 0,  // PDF_FORM_NOTAVAIL
    FormAvailable = 1,     // PDF_FORM_AVAIL
    FormNotExist = 2,      // PDF_FORM_NOTEXIST
  };

  class FileAvail {
   public:
    virtual ~FileAvail();
    virtual FX_BOOL IsDataAvail(FX_FILESIZE offset, uint32_t size) = 0;
  };

  class DownloadHints {
   public:
    virtual ~DownloadHints();
    virtual void AddSegment(FX_FILESIZE offset, uint32_t size) = 0;
  };

  static IPDF_DataAvail* Create(FileAvail* pFileAvail, IFX_FileRead* pFileRead);
  virtual ~IPDF_DataAvail();

  FileAvail* GetFileAvail() const { return m_pFileAvail; }
  IFX_FileRead* GetFileRead() const { return m_pFileRead; }

  virtual DocAvailStatus IsDocAvail(DownloadHints* pHints) = 0;
  virtual void SetDocument(CPDF_Document* pDoc) = 0;
  virtual DocAvailStatus IsPageAvail(int iPage, DownloadHints* pHints) = 0;
  virtual FX_BOOL IsLinearized() = 0;
  virtual DocFormStatus IsFormAvail(DownloadHints* pHints) = 0;
  virtual DocLinearizationStatus IsLinearizedPDF() = 0;
  virtual void GetLinearizedMainXRefInfo(FX_FILESIZE* pPos,
                                         uint32_t* pSize) = 0;

 protected:
  IPDF_DataAvail(FileAvail* pFileAvail, IFX_FileRead* pFileRead);

  FileAvail* m_pFileAvail;
  IFX_FileRead* m_pFileRead;
};

#endif  // CORE_FPDFAPI_FPDF_PARSER_INCLUDE_IPDF_DATA_AVAIL_H_
