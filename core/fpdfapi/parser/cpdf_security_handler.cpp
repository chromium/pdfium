// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_security_handler.h"

#include <stdint.h>
#include <time.h>

#include <algorithm>
#include <array>
#include <optional>
#include <utility>

#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/byteorder.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/fx_random.h"
#include "core/fxcrt/notreached.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/span_util.h"
#include "core/fxcrt/stl_util.h"

namespace {

constexpr std::array<const uint8_t, 32> kDefaultPasscode = {
    0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41, 0x64, 0x00, 0x4e,
    0x56, 0xff, 0xfa, 0x01, 0x08, 0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68,
    0x3e, 0x80, 0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a};

void GetPassCode(const ByteString& password, pdfium::span<uint8_t> output) {
  DCHECK_EQ(sizeof(kDefaultPasscode), output.size());
  size_t len = std::min(password.GetLength(), output.size());
  auto remaining = fxcrt::spancpy(output, password.unsigned_span().first(len));
  if (!remaining.empty()) {
    auto default_span = pdfium::span(kDefaultPasscode);
    fxcrt::spancpy(remaining, default_span.first(remaining.size()));
  }
}

void CalcEncryptKey(const CPDF_Dictionary* pEncrypt,
                    const ByteString& password,
                    pdfium::span<uint8_t> key,
                    bool ignore_metadata,
                    const ByteString& file_id) {
  std::ranges::fill(key, 0);

  uint8_t passcode[32];
  GetPassCode(password, passcode);

  CRYPT_md5_context md5 = CRYPT_MD5Start();
  CRYPT_MD5Update(&md5, passcode);

  ByteString okey = pEncrypt->GetByteStringFor("O");
  CRYPT_MD5Update(&md5, okey.unsigned_span());

  uint32_t perm = pEncrypt->GetIntegerFor("P");
  CRYPT_MD5Update(&md5, pdfium::as_bytes(pdfium::span_from_ref(perm)));
  if (!file_id.IsEmpty()) {
    CRYPT_MD5Update(&md5, file_id.unsigned_span());
  }
  const bool is_revision_3_or_greater = pEncrypt->GetIntegerFor("R") >= 3;
  if (!ignore_metadata && is_revision_3_or_greater &&
      !pEncrypt->GetBooleanFor("EncryptMetadata", true)) {
    static constexpr uint32_t tag = 0xFFFFFFFF;
    CRYPT_MD5Update(&md5, pdfium::byte_span_from_ref(tag));
  }
  uint8_t digest[16];
  CRYPT_MD5Finish(&md5, digest);
  size_t copy_len = std::min(key.size(), sizeof(digest));
  auto digest_span = pdfium::span(digest).first(copy_len);
  if (is_revision_3_or_greater) {
    for (int i = 0; i < 50; i++) {
      CRYPT_MD5Generate(digest_span, digest);
    }
  }
  fxcrt::Copy(digest_span, key);
}

bool IsValidKeyLengthForCipher(CPDF_CryptoHandler::Cipher cipher,
                               size_t keylen) {
  switch (cipher) {
    case CPDF_CryptoHandler::Cipher::kAES:
      return keylen == 16 || keylen == 24 || keylen == 32;
    case CPDF_CryptoHandler::Cipher::kAES2:
      return keylen == 32;
    case CPDF_CryptoHandler::Cipher::kRC4:
      return keylen >= 5 && keylen <= 16;
    case CPDF_CryptoHandler::Cipher::kNone:
      return true;
  }
}

int BigOrder64BitsMod3(pdfium::span<const uint8_t> data) {
  uint64_t ret = 0;
  for (int i = 0; i < 4; ++i) {
    ret <<= 32;
    ret |= fxcrt::GetUInt32MSBFirst(data.first<4u>());
    ret %= 3;
    data = data.subspan<4u>();
  }
  return static_cast<int>(ret);
}

void Revision6_Hash(const ByteString& password,
                    pdfium::span<const uint8_t, 8> salt,
                    std::optional<pdfium::span<const uint8_t, 48>> vector,
                    pdfium::span<uint8_t, 32> hash) {
  CRYPT_sha2_context sha;
  CRYPT_SHA256Start(&sha);
  CRYPT_SHA256Update(&sha, password.unsigned_span());
  CRYPT_SHA256Update(&sha, salt);
  if (vector.has_value()) {
    CRYPT_SHA256Update(&sha, vector.value());
  }

  std::array<uint8_t, 32> digest;
  CRYPT_SHA256Finish(&sha, digest);

  DataVector<uint8_t> encrypted_output;
  DataVector<uint8_t> inter_digest;
  pdfium::span<uint8_t> input(digest);
  auto key = input.first<16u>();
  auto iv = input.subspan<16u, 16u>();
  int i = 0;
  size_t block_size = 32;
  CRYPT_aes_context aes = {};
  do {
    size_t round_size = password.GetLength() + block_size;
    if (vector.has_value()) {
      round_size += vector.value().size();
    }
    encrypted_output.resize(round_size * 64);
    auto encrypted_output_span = pdfium::span(encrypted_output);
    auto password_span = password.unsigned_span();
    auto input_span = pdfium::span(input).first(block_size);
    DataVector<uint8_t> content;
    for (int j = 0; j < 64; ++j) {
      content.insert(std::end(content), std::begin(password_span),
                     std::end(password_span));
      content.insert(std::end(content), std::begin(input_span),
                     std::end(input_span));
      if (vector.has_value()) {
        content.insert(std::end(content), std::begin(vector.value()),
                       std::end(vector.value()));
      }
    }
    CHECK_EQ(content.size(), encrypted_output.size());
    CRYPT_AESSetKey(&aes, key);
    CRYPT_AESSetIV(&aes, iv);
    CRYPT_AESEncrypt(&aes, encrypted_output_span, content);

    switch (BigOrder64BitsMod3(encrypted_output_span)) {
      case 0:
        block_size = 32;
        inter_digest = CRYPT_SHA256Generate(encrypted_output_span);
        break;
      case 1:
        block_size = 48;
        inter_digest = CRYPT_SHA384Generate(encrypted_output_span);
        break;
      default:
        block_size = 64;
        inter_digest = CRYPT_SHA512Generate(encrypted_output_span);
        break;
    }
    input = inter_digest;
    key = input.first<16u>();
    iv = input.subspan<16u, 16u>();
    ++i;
  } while (i < 64 || i - 32 < encrypted_output.back());

  fxcrt::Copy(input.first(32u), hash);
}

}  // namespace

CPDF_SecurityHandler::CPDF_SecurityHandler() = default;

CPDF_SecurityHandler::~CPDF_SecurityHandler() = default;

bool CPDF_SecurityHandler::OnInit(const CPDF_Dictionary* pEncryptDict,
                                  RetainPtr<const CPDF_Array> pIdArray,
                                  const ByteString& password) {
  if (pIdArray) {
    file_id_ = pIdArray->GetByteStringAt(0);
  } else {
    file_id_.clear();
  }
  if (!LoadDict(pEncryptDict)) {
    return false;
  }
  if (cipher_ == CPDF_CryptoHandler::Cipher::kNone) {
    return true;
  }
  if (!CheckSecurity(password)) {
    return false;
  }

  InitCryptoHandler();
  return true;
}

bool CPDF_SecurityHandler::CheckSecurity(const ByteString& password) {
  if (!password.IsEmpty() && CheckPassword(password, true)) {
    owner_unlocked_ = true;
    return true;
  }
  return CheckPassword(password, false);
}

uint32_t CPDF_SecurityHandler::GetPermissions(bool get_owner_perms) const {
  uint32_t dwPermission =
      owner_unlocked_ && get_owner_perms ? 0xFFFFFFFF : permissions_;
  if (encrypt_dict_ &&
      encrypt_dict_->GetByteStringFor("Filter") == "Standard") {
    // See PDF Reference 1.7, page 123, table 3.20.
    dwPermission &= 0xFFFFFFFC;
    dwPermission |= 0xFFFFF0C0;
  }
  return dwPermission;
}

static bool LoadCryptInfo(const CPDF_Dictionary* pEncryptDict,
                          const ByteString& name,
                          CPDF_CryptoHandler::Cipher* cipher,
                          size_t* keylen_out) {
  int Version = pEncryptDict->GetIntegerFor("V");
  *cipher = CPDF_CryptoHandler::Cipher::kRC4;
  *keylen_out = 0;
  int keylen = 0;
  if (Version >= 4) {
    RetainPtr<const CPDF_Dictionary> pCryptFilters =
        pEncryptDict->GetDictFor("CF");
    if (!pCryptFilters) {
      return false;
    }

    if (name == "Identity") {
      *cipher = CPDF_CryptoHandler::Cipher::kNone;
    } else {
      RetainPtr<const CPDF_Dictionary> pDefFilter =
          pCryptFilters->GetDictFor(name.AsStringView());
      if (!pDefFilter) {
        return false;
      }

      int nKeyBits = 0;
      if (Version == 4) {
        nKeyBits = pDefFilter->GetIntegerFor("Length", 0);
        if (nKeyBits == 0) {
          nKeyBits = pEncryptDict->GetIntegerFor("Length", 128);
        }
      } else {
        nKeyBits = pEncryptDict->GetIntegerFor("Length", 256);
      }
      if (nKeyBits < 0) {
        return false;
      }

      if (nKeyBits < 40) {
        nKeyBits *= 8;
      }
      keylen = nKeyBits / 8;
      ByteString cipher_name = pDefFilter->GetByteStringFor("CFM");
      if (cipher_name == "AESV2" || cipher_name == "AESV3") {
        *cipher = CPDF_CryptoHandler::Cipher::kAES;
      }
    }
  } else {
    keylen = Version > 1 ? pEncryptDict->GetIntegerFor("Length", 40) / 8 : 5;
  }

  if (keylen < 0 || keylen > 32) {
    return false;
  }
  if (!IsValidKeyLengthForCipher(*cipher, keylen)) {
    return false;
  }

  *keylen_out = keylen;
  return true;
}

bool CPDF_SecurityHandler::LoadDict(const CPDF_Dictionary* pEncryptDict) {
  encrypt_dict_.Reset(pEncryptDict);
  version_ = pEncryptDict->GetIntegerFor("V");
  revision_ = pEncryptDict->GetIntegerFor("R");
  permissions_ = pEncryptDict->GetIntegerFor("P", -1);
  if (version_ < 4) {
    return LoadCryptInfo(pEncryptDict, ByteString(), &cipher_, &key_len_);
  }

  ByteString stmf_name = pEncryptDict->GetByteStringFor("StmF");
  ByteString strf_name = pEncryptDict->GetByteStringFor("StrF");
  if (stmf_name != strf_name) {
    return false;
  }

  return LoadCryptInfo(pEncryptDict, strf_name, &cipher_, &key_len_);
}

bool CPDF_SecurityHandler::LoadDict(const CPDF_Dictionary* pEncryptDict,
                                    CPDF_CryptoHandler::Cipher* cipher,
                                    size_t* key_len) {
  encrypt_dict_.Reset(pEncryptDict);
  version_ = pEncryptDict->GetIntegerFor("V");
  revision_ = pEncryptDict->GetIntegerFor("R");
  permissions_ = pEncryptDict->GetIntegerFor("P", -1);

  ByteString strf_name;
  ByteString stmf_name;
  if (version_ >= 4) {
    stmf_name = pEncryptDict->GetByteStringFor("StmF");
    strf_name = pEncryptDict->GetByteStringFor("StrF");
    if (stmf_name != strf_name) {
      return false;
    }
  }
  if (!LoadCryptInfo(pEncryptDict, strf_name, cipher, key_len)) {
    return false;
  }

  cipher_ = *cipher;
  key_len_ = *key_len;
  return true;
}

bool CPDF_SecurityHandler::AES256_CheckPassword(const ByteString& password,
                                                bool bOwner) {
  DCHECK(encrypt_dict_);
  DCHECK(revision_ >= 5);

  ByteString okey = encrypt_dict_->GetByteStringFor("O");
  if (okey.GetLength() < 48) {
    return false;
  }
  ByteString ukey = encrypt_dict_->GetByteStringFor("U");
  if (ukey.GetLength() < 48) {
    return false;
  }
  pdfium::span<const uint8_t> pkey = ukey.unsigned_span();
  std::optional<pdfium::span<const uint8_t, 48>> maybe_skey;
  if (bOwner) {
    pkey = okey.unsigned_span();
    maybe_skey = ukey.unsigned_span().first<48u>();
  }

  uint8_t digest[32];
  if (revision_ >= 6) {
    Revision6_Hash(password, pkey.subspan<32u, 8u>(), maybe_skey, digest);
  } else {
    CRYPT_sha2_context sha;
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, password.unsigned_span());
    CRYPT_SHA256Update(&sha, pkey.subspan<32u, 8u>());
    if (maybe_skey.has_value()) {
      CRYPT_SHA256Update(&sha, maybe_skey.value());
    }
    CRYPT_SHA256Finish(&sha, digest);
  }
  if (!std::ranges::equal(digest, pkey.first<32u>())) {
    return false;
  }

  if (revision_ >= 6) {
    Revision6_Hash(password, pkey.subspan<40u, 8u>(), maybe_skey, digest);
  } else {
    CRYPT_sha2_context sha;
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, password.unsigned_span());
    CRYPT_SHA256Update(&sha, pkey.subspan<40u, 8u>());
    if (maybe_skey.has_value()) {
      CRYPT_SHA256Update(&sha, maybe_skey.value());
    }
    CRYPT_SHA256Finish(&sha, digest);
  }
  ByteString ekey = encrypt_dict_->GetByteStringFor(bOwner ? "OE" : "UE");
  if (ekey.GetLength() < 32) {
    return false;
  }

  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, digest);
  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);
  CRYPT_AESDecrypt(&aes, encrypt_key_, ekey.unsigned_span().first<32u>());
  CRYPT_AESSetKey(&aes, encrypt_key_);
  CRYPT_AESSetIV(&aes, iv);
  ByteString perms = encrypt_dict_->GetByteStringFor("Perms");
  if (perms.IsEmpty()) {
    return false;
  }

  uint8_t perms_buf[16] = {};
  size_t copy_len = std::min(sizeof(perms_buf), perms.GetLength());
  fxcrt::Copy(perms.unsigned_span().first(copy_len), perms_buf);

  uint8_t buf[16];
  CRYPT_AESDecrypt(&aes, buf, perms_buf);
  if (buf[9] != 'a' || buf[10] != 'd' || buf[11] != 'b') {
    return false;
  }

  if (fxcrt::GetUInt32LSBFirst(pdfium::span(buf).first<4u>()) != permissions_) {
    return false;
  }

  // Relax this check as there appear to be some non-conforming documents
  // in the wild. The value in the buffer is the truth; if it requires us
  // to encrypt metadata, but the dictionary says otherwise, then we may
  // have a tampered doc.  Otherwise, give it a pass.
  return buf[8] == 'F' || IsMetadataEncrypted();
}

bool CPDF_SecurityHandler::CheckPassword(const ByteString& password,
                                         bool bOwner) {
  DCHECK_EQ(kUnknown, password_encoding_conversion_);
  if (CheckPasswordImpl(password, bOwner)) {
    password_encoding_conversion_ = kNone;
    return true;
  }

  ByteStringView password_view = password.AsStringView();
  if (password_view.IsASCII()) {
    return false;
  }

  if (revision_ >= 5) {
    ByteString utf8_password = WideString::FromLatin1(password_view).ToUTF8();
    if (!CheckPasswordImpl(utf8_password, bOwner)) {
      return false;
    }

    password_encoding_conversion_ = kLatin1ToUtf8;
    return true;
  }

  ByteString latin1_password = WideString::FromUTF8(password_view).ToLatin1();
  if (!CheckPasswordImpl(latin1_password, bOwner)) {
    return false;
  }

  password_encoding_conversion_ = kUtf8toLatin1;
  return true;
}

bool CPDF_SecurityHandler::CheckPasswordImpl(const ByteString& password,
                                             bool bOwner) {
  if (revision_ >= 5) {
    return AES256_CheckPassword(password, bOwner);
  }

  if (bOwner) {
    return CheckOwnerPassword(password);
  }

  return CheckUserPassword(password, false) ||
         CheckUserPassword(password, true);
}

bool CPDF_SecurityHandler::CheckUserPassword(const ByteString& password,
                                             bool bIgnoreEncryptMeta) {
  CalcEncryptKey(encrypt_dict_.Get(), password,
                 pdfium::span(encrypt_key_).first(key_len_), bIgnoreEncryptMeta,
                 file_id_);
  ByteString ukey =
      encrypt_dict_ ? encrypt_dict_->GetByteStringFor("U") : ByteString();
  if (ukey.GetLength() < 16) {
    return false;
  }

  uint8_t ukeybuf[32];
  if (revision_ == 2) {
    fxcrt::Copy(kDefaultPasscode, ukeybuf);
    CRYPT_ArcFourCryptBlock(ukeybuf,
                            pdfium::span(encrypt_key_).first(key_len_));
    return std::ranges::equal(ukey.unsigned_span().first<16u>(),
                              pdfium::span(ukeybuf).first<16u>());
  }

  std::array<uint8_t, 32> test = {};
  std::array<uint8_t, 32> tmpkey = {};
  uint32_t copy_len = std::min(sizeof(test), ukey.GetLength());
  fxcrt::Copy(ukey.unsigned_span().first(copy_len), test);
  for (int32_t i = 19; i >= 0; i--) {
    for (size_t j = 0; j < key_len_; j++) {
      tmpkey[j] = encrypt_key_[j] ^ static_cast<uint8_t>(i);
    }
    CRYPT_ArcFourCryptBlock(test, pdfium::span(tmpkey).first(key_len_));
  }
  CRYPT_md5_context md5 = CRYPT_MD5Start();
  CRYPT_MD5Update(&md5, kDefaultPasscode);
  if (!file_id_.IsEmpty()) {
    CRYPT_MD5Update(&md5, file_id_.unsigned_span());
  }
  CRYPT_MD5Finish(&md5, pdfium::span(ukeybuf).first<16u>());
  return std::ranges::equal(pdfium::span(test).first<16u>(),
                            pdfium::span(ukeybuf).first<16u>());
}

ByteString CPDF_SecurityHandler::GetUserPassword(
    const ByteString& owner_password) const {
  static constexpr size_t kRequiredOkeyLength = 32;
  const ByteString okey = encrypt_dict_->GetByteStringFor("O");
  if (okey.GetLength() < kRequiredOkeyLength) {
    return ByteString();
  }

  std::array<uint8_t, 32> passcode;
  std::array<uint8_t, 16> digest;
  GetPassCode(owner_password, passcode);
  CRYPT_MD5Generate(passcode, digest);
  if (revision_ >= 3) {
    for (uint32_t i = 0; i < 50; i++) {
      CRYPT_MD5Generate(digest, digest);
    }
  }

  std::array<uint8_t, 32> enckey = {};
  std::array<uint8_t, kRequiredOkeyLength> okeybuf = {};
  size_t copy_len = std::min(key_len_, sizeof(digest));
  fxcrt::Copy(pdfium::span(digest).first(copy_len), enckey);
  fxcrt::Copy(okey.unsigned_span(), okeybuf);
  pdfium::span<uint8_t> okey_span = pdfium::span(okeybuf);
  if (revision_ == 2) {
    CRYPT_ArcFourCryptBlock(okey_span, pdfium::span(enckey).first(key_len_));
  } else {
    for (int32_t i = 19; i >= 0; i--) {
      std::array<uint8_t, 32> tempkey = {};
      for (size_t j = 0; j < key_len_; j++) {
        tempkey[j] = enckey[j] ^ static_cast<uint8_t>(i);
      }
      CRYPT_ArcFourCryptBlock(okey_span, pdfium::span(tempkey).first(key_len_));
    }
  }
  size_t len = kRequiredOkeyLength;
  while (len && kDefaultPasscode[len - 1] == okey_span[len - 1]) {
    len--;
  }
  return ByteString(ByteStringView(okey_span.first(len)));
}

bool CPDF_SecurityHandler::CheckOwnerPassword(const ByteString& password) {
  ByteString user_pass = GetUserPassword(password);
  return CheckUserPassword(user_pass, false) ||
         CheckUserPassword(user_pass, true);
}

bool CPDF_SecurityHandler::IsMetadataEncrypted() const {
  return encrypt_dict_->GetBooleanFor("EncryptMetadata", true);
}

ByteString CPDF_SecurityHandler::GetEncodedPassword(
    ByteStringView password) const {
  switch (password_encoding_conversion_) {
    case kNone:
      // Do nothing.
      return ByteString(password);
    case kLatin1ToUtf8:
      return WideString::FromLatin1(password).ToUTF8();
    case kUtf8toLatin1:
      return WideString::FromUTF8(password).ToLatin1();
    default:
      NOTREACHED();
  }
}

void CPDF_SecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict,
                                    const CPDF_Array* pIdArray,
                                    const ByteString& password) {
  DCHECK(pEncryptDict);

  CPDF_CryptoHandler::Cipher cipher = CPDF_CryptoHandler::Cipher::kNone;
  size_t key_len = 0;
  if (!LoadDict(pEncryptDict, &cipher, &key_len)) {
    return;
  }

  if (revision_ >= 5) {
    uint32_t random[4];
    FX_Random_GenerateMT(random);
    CRYPT_sha2_context sha;
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, pdfium::as_byte_span(random));
    CRYPT_SHA256Finish(&sha, encrypt_key_);
    AES256_SetPassword(pEncryptDict, password);
    AES256_SetPerms(pEncryptDict);
    return;
  }

  ByteString file_id;
  if (pIdArray) {
    file_id = pIdArray->GetByteStringAt(0);
  }

  CalcEncryptKey(encrypt_dict_.Get(), password,
                 pdfium::span(encrypt_key_).first(key_len), false, file_id);
  if (revision_ < 3) {
    uint8_t tempbuf[32];
    fxcrt::Copy(kDefaultPasscode, tempbuf);
    CRYPT_ArcFourCryptBlock(tempbuf, pdfium::span(encrypt_key_).first(key_len));
    pEncryptDict->SetNewFor<CPDF_String>(
        "U", ByteString(ByteStringView(pdfium::span(tempbuf))));
  } else {
    CRYPT_md5_context md5 = CRYPT_MD5Start();
    CRYPT_MD5Update(&md5, kDefaultPasscode);
    if (!file_id.IsEmpty()) {
      CRYPT_MD5Update(&md5, file_id.unsigned_span());
    }

    uint8_t digest[32];
    auto partial_digest_span = pdfium::span(digest).first<16u>();
    auto remaining_digest_span = pdfium::span(digest).subspan<16u>();
    CRYPT_MD5Finish(&md5, partial_digest_span);
    CRYPT_ArcFourCryptBlock(partial_digest_span,
                            pdfium::span(encrypt_key_).first(key_len));
    std::array<uint8_t, 32> tempkey;
    for (uint8_t i = 1; i <= 19; i++) {
      for (size_t j = 0; j < key_len; j++) {
        tempkey[j] = encrypt_key_[j] ^ i;
      }
      CRYPT_ArcFourCryptBlock(partial_digest_span,
                              pdfium::span(tempkey).first(key_len));
    }
    CRYPT_MD5Generate(partial_digest_span, remaining_digest_span.first<16u>());
    pEncryptDict->SetNewFor<CPDF_String>(
        "U", ByteString(ByteStringView(pdfium::span(digest))));
  }

  InitCryptoHandler();
}

void CPDF_SecurityHandler::AES256_SetPassword(CPDF_Dictionary* pEncryptDict,
                                              const ByteString& password) {
  CRYPT_sha1_context sha;
  CRYPT_SHA1Start(&sha);
  CRYPT_SHA1Update(&sha, encrypt_key_);
  CRYPT_SHA1Update(&sha, pdfium::as_byte_span("hello").first<5u>());

  uint8_t digest[20];
  CRYPT_SHA1Finish(&sha, digest);

  CRYPT_sha2_context sha2;
  uint8_t digest1[48];
  if (revision_ >= 6) {
    Revision6_Hash(password, pdfium::span(digest).first<8u>(), std::nullopt,
                   pdfium::span(digest1).first<32u>());
  } else {
    CRYPT_SHA256Start(&sha2);
    CRYPT_SHA256Update(&sha2, password.unsigned_span());
    CRYPT_SHA256Update(&sha2, pdfium::span(digest).first<8u>());
    CRYPT_SHA256Finish(&sha2, pdfium::span(digest1).first<32u>());
  }
  fxcrt::Copy(pdfium::span(digest).first<16u>(),
              pdfium::span(digest1).subspan<32u>());
  pEncryptDict->SetNewFor<CPDF_String>(
      "U", ByteString(ByteStringView(pdfium::span(digest1))));
  if (revision_ >= 6) {
    Revision6_Hash(password, pdfium::span(digest).subspan<8u, 8u>(),
                   std::nullopt, pdfium::span(digest1).first<32u>());
  } else {
    CRYPT_SHA256Start(&sha2);
    CRYPT_SHA256Update(&sha2, password.unsigned_span());
    CRYPT_SHA256Update(&sha2, pdfium::span(digest).subspan<8, 8>());
    CRYPT_SHA256Finish(&sha2, pdfium::span(digest1).first<32>());
  }
  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, pdfium::span(digest1).first<32>());
  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);
  CRYPT_AESEncrypt(&aes, digest1, encrypt_key_);
  pEncryptDict->SetNewFor<CPDF_String>(
      "UE", ByteString(ByteStringView(pdfium::span(digest1).first<32>())));
}

void CPDF_SecurityHandler::AES256_SetPerms(CPDF_Dictionary* pEncryptDict) {
  uint8_t buf[16];
  buf[0] = static_cast<uint8_t>(permissions_);
  buf[1] = static_cast<uint8_t>(permissions_ >> 8);
  buf[2] = static_cast<uint8_t>(permissions_ >> 16);
  buf[3] = static_cast<uint8_t>(permissions_ >> 24);
  buf[4] = 0xff;
  buf[5] = 0xff;
  buf[6] = 0xff;
  buf[7] = 0xff;
  buf[8] = pEncryptDict->GetBooleanFor("EncryptMetadata", true) ? 'T' : 'F';
  buf[9] = 'a';
  buf[10] = 'd';
  buf[11] = 'b';

  // In ISO 32000 Supplement for ExtensionLevel 3, Algorithm 3.10 says bytes 12
  // to 15 should be random data.
  uint32_t random_value;
  FX_Random_GenerateMT(pdfium::span_from_ref(random_value));
  fxcrt::Copy(pdfium::byte_span_from_ref(random_value),
              pdfium::span(buf).subspan<12, 4>());

  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, encrypt_key_);

  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);

  uint8_t dest[16];
  CRYPT_AESEncrypt(&aes, dest, buf);
  pEncryptDict->SetNewFor<CPDF_String>(
      "Perms", ByteString(ByteStringView(pdfium::span(dest))));
}

void CPDF_SecurityHandler::InitCryptoHandler() {
  crypto_handler_ = std::make_unique<CPDF_CryptoHandler>(
      cipher_, pdfium::span(encrypt_key_).first(key_len_));
}
