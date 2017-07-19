// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PDFWINDOW_CPWL_APPSTREAM_H_
#define FPDFSDK_PDFWINDOW_CPWL_APPSTREAM_H_

#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_string.h"

class CPDFSDK_Widget;
class CPDF_Dictionary;
class CPDF_Stream;

class CPWL_AppStream {
 public:
  CPWL_AppStream(CPDFSDK_Widget* widget, CPDF_Dictionary* dict);
  ~CPWL_AppStream();

  void SetAsPushButton();
  void SetAsCheckBox();
  void SetAsRadioButton();
  void SetAsComboBox(const CFX_WideString* sValue);
  void SetAsListBox();
  void SetAsTextField(const CFX_WideString* sValue);

 private:
  void AddImage(const CFX_ByteString& sAPType, CPDF_Stream* pImage);
  void Write(const CFX_ByteString& sAPType,
             const CFX_ByteString& sContents,
             const CFX_ByteString& sAPState);
  void Remove(const CFX_ByteString& sAPType);

  CFX_ByteString GetBackgroundAppStream() const;
  CFX_ByteString GetBorderAppStream() const;

  CFX_UnownedPtr<CPDFSDK_Widget> widget_;
  CFX_UnownedPtr<CPDF_Dictionary> dict_;
};

#endif  // FPDFSDK_PDFWINDOW_CPWL_APPSTREAM_H_
