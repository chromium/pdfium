// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_FILESPEC_H_
#define CORE_FPDFDOC_CPDF_FILESPEC_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/string_pool_template.h"
#include "core/fxcrt/weak_ptr.h"
#include "core/fxcrt/widestring.h"

class CPDF_Dictionary;
class CPDF_Object;
class CPDF_Stream;

class CPDF_FileSpec {
 public:
  explicit CPDF_FileSpec(const CPDF_Object* pObj);
  ~CPDF_FileSpec();

  // Convert a platform dependent file name into pdf format.
  static WideString EncodeFileName(const WideString& filepath);

  // Convert a pdf file name into platform dependent format.
  static WideString DecodeFileName(const WideString& filepath);

  WideString GetFileName() const;
  const CPDF_Stream* GetFileStream() const;
  CPDF_Stream* GetFileStream();
  const CPDF_Dictionary* GetParamsDict() const;
  CPDF_Dictionary* GetParamsDict();

 private:
  RetainPtr<const CPDF_Object> const m_pObj;
};

#endif  // CORE_FPDFDOC_CPDF_FILESPEC_H_
