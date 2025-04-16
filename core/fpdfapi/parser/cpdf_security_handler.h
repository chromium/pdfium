// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_SECURITY_HANDLER_H_
#define CORE_FPDFAPI_PARSER_CPDF_SECURITY_HANDLER_H_

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <memory>

#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Array;
class CPDF_Dictionary;

class CPDF_SecurityHandler final : public Retainable {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;

  bool OnInit(const CPDF_Dictionary* pEncryptDict,
              RetainPtr<const CPDF_Array> pIdArray,
              const ByteString& password);
  void OnCreate(CPDF_Dictionary* pEncryptDict,
                const CPDF_Array* pIdArray,
                const ByteString& password);

  // When `get_owner_perms` is true, returns full permissions if unlocked by
  // owner.
  uint32_t GetPermissions(bool get_owner_perms) const;
  bool IsMetadataEncrypted() const;

  CPDF_CryptoHandler* GetCryptoHandler() const { return crypto_handler_.get(); }

  // Take |password| and encode it, if necessary, based on the password encoding
  // conversion.
  ByteString GetEncodedPassword(ByteStringView password) const;

 private:
  enum PasswordEncodingConversion {
    kUnknown,
    kNone,
    kLatin1ToUtf8,
    kUtf8toLatin1,
  };

  CPDF_SecurityHandler();
  ~CPDF_SecurityHandler() override;

  bool LoadDict(const CPDF_Dictionary* pEncryptDict);
  bool LoadDict(const CPDF_Dictionary* pEncryptDict,
                CPDF_CryptoHandler::Cipher* cipher,
                size_t* key_len);

  ByteString GetUserPassword(const ByteString& owner_password) const;
  bool CheckPassword(const ByteString& user_password, bool bOwner);
  bool CheckPasswordImpl(const ByteString& password, bool bOwner);
  bool CheckUserPassword(const ByteString& password, bool bIgnoreEncryptMeta);
  bool CheckOwnerPassword(const ByteString& password);
  bool AES256_CheckPassword(const ByteString& password, bool bOwner);
  void AES256_SetPassword(CPDF_Dictionary* pEncryptDict,
                          const ByteString& password);
  void AES256_SetPerms(CPDF_Dictionary* pEncryptDict);
  bool CheckSecurity(const ByteString& password);

  void InitCryptoHandler();

  bool owner_unlocked_ = false;
  int version_ = 0;
  int revision_ = 0;
  uint32_t permissions_ = 0;
  size_t key_len_ = 0;
  CPDF_CryptoHandler::Cipher cipher_ = CPDF_CryptoHandler::Cipher::kNone;
  PasswordEncodingConversion password_encoding_conversion_ = kUnknown;
  ByteString file_id_;
  RetainPtr<const CPDF_Dictionary> encrypt_dict_;
  std::unique_ptr<CPDF_CryptoHandler> crypto_handler_;
  std::array<uint8_t, 32> encrypt_key_ = {};
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_SECURITY_HANDLER_H_
