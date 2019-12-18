// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_security_handler.h"

#include <time.h>

#include <algorithm>
#include <utility>
#include <vector>

#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_crypto_handler.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/fx_random.h"
#include "third_party/base/ptr_util.h"

namespace {

const uint8_t kDefaultPasscode[32] = {
    0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41, 0x64, 0x00, 0x4e,
    0x56, 0xff, 0xfa, 0x01, 0x08, 0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68,
    0x3e, 0x80, 0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a};

void GetPassCode(const ByteString& password, pdfium::span<uint8_t> output) {
  DCHECK_EQ(sizeof(kDefaultPasscode), output.size());
  size_t len = std::min(password.GetLength(), output.size());
  size_t remaining = output.size() - len;
  memcpy(output.data(), password.raw_str(), len);
  if (remaining)
    memcpy(&output[len], kDefaultPasscode, remaining);
}

void CalcEncryptKey(const CPDF_Dictionary* pEncrypt,
                    const ByteString& password,
                    uint8_t* key,
                    size_t keylen,
                    bool ignore_metadata,
                    const ByteString& file_id) {
  uint8_t passcode[32];
  GetPassCode(password, passcode);
  CRYPT_md5_context md5 = CRYPT_MD5Start();
  CRYPT_MD5Update(&md5, passcode);
  ByteString okey = pEncrypt->GetStringFor("O");
  CRYPT_MD5Update(&md5, okey.raw_span());
  uint32_t perm = pEncrypt->GetIntegerFor("P");
  CRYPT_MD5Update(&md5, pdfium::as_bytes(pdfium::make_span(&perm, 1)));
  if (!file_id.IsEmpty())
    CRYPT_MD5Update(&md5, file_id.raw_span());
  const bool is_revision_3_or_greater = pEncrypt->GetIntegerFor("R") >= 3;
  if (!ignore_metadata && is_revision_3_or_greater &&
      !pEncrypt->GetBooleanFor("EncryptMetadata", true)) {
    constexpr uint32_t tag = 0xFFFFFFFF;
    CRYPT_MD5Update(&md5, pdfium::as_bytes(pdfium::make_span(&tag, 1)));
  }
  uint8_t digest[16];
  CRYPT_MD5Finish(&md5, digest);
  size_t copy_len = std::min(keylen, sizeof(digest));
  if (is_revision_3_or_greater) {
    for (int i = 0; i < 50; i++)
      CRYPT_MD5Generate({digest, copy_len}, digest);
  }
  memset(key, 0, keylen);
  memcpy(key, digest, copy_len);
}

bool IsValidKeyLengthForCipher(int cipher, size_t keylen) {
  switch (cipher) {
    case FXCIPHER_AES:
      return keylen == 16 || keylen == 24 || keylen == 32;
    case FXCIPHER_AES2:
      return keylen == 32;
    case FXCIPHER_RC4:
      return keylen >= 5 && keylen <= 16;
    case FXCIPHER_NONE:
      return true;
    default:
      NOTREACHED();
  }
  return false;
}

#define FX_GET_32WORD(n, b, i)                                        \
  {                                                                   \
    (n) = (uint32_t)(                                                 \
        ((uint64_t)(b)[(i)] << 24) | ((uint64_t)(b)[(i) + 1] << 16) | \
        ((uint64_t)(b)[(i) + 2] << 8) | ((uint64_t)(b)[(i) + 3]));    \
  }
int BigOrder64BitsMod3(uint8_t* data) {
  uint64_t ret = 0;
  for (int i = 0; i < 4; ++i) {
    uint32_t value;
    FX_GET_32WORD(value, data, 4 * i);
    ret <<= 32;
    ret |= value;
    ret %= 3;
  }
  return (int)ret;
}

void Revision6_Hash(const ByteString& password,
                    const uint8_t* salt,
                    const uint8_t* vector,
                    uint8_t* hash) {
  CRYPT_sha2_context sha;
  CRYPT_SHA256Start(&sha);
  CRYPT_SHA256Update(&sha, password.raw_str(), password.GetLength());
  CRYPT_SHA256Update(&sha, salt, 8);
  if (vector)
    CRYPT_SHA256Update(&sha, vector, 48);

  uint8_t digest[32];
  CRYPT_SHA256Finish(&sha, digest);

  std::vector<uint8_t> buf;
  uint8_t* input = digest;
  uint8_t* key = input;
  uint8_t* iv = input + 16;
  uint8_t* E = nullptr;
  int iBufLen = 0;
  std::vector<uint8_t> interDigest;
  int i = 0;
  int iBlockSize = 32;
  CRYPT_aes_context aes = {};
  while (i < 64 || i < E[iBufLen - 1] + 32) {
    int iRoundSize = password.GetLength() + iBlockSize;
    if (vector) {
      iRoundSize += 48;
    }
    iBufLen = iRoundSize * 64;
    buf.resize(iBufLen);
    E = buf.data();
    std::vector<uint8_t> content;
    for (int j = 0; j < 64; ++j) {
      content.insert(std::end(content), password.raw_str(),
                     password.raw_str() + password.GetLength());
      content.insert(std::end(content), input, input + iBlockSize);
      if (vector) {
        content.insert(std::end(content), vector, vector + 48);
      }
    }
    CRYPT_AESSetKey(&aes, key, 16, true);
    CRYPT_AESSetIV(&aes, iv);
    CRYPT_AESEncrypt(&aes, E, content.data(), iBufLen);
    int iHash = 0;
    switch (BigOrder64BitsMod3(E)) {
      case 0:
        iHash = 0;
        iBlockSize = 32;
        break;
      case 1:
        iHash = 1;
        iBlockSize = 48;
        break;
      default:
        iHash = 2;
        iBlockSize = 64;
        break;
    }
    interDigest.resize(iBlockSize);
    input = interDigest.data();
    if (iHash == 0) {
      CRYPT_SHA256Generate(E, iBufLen, input);
    } else if (iHash == 1) {
      CRYPT_SHA384Generate(E, iBufLen, input);
    } else if (iHash == 2) {
      CRYPT_SHA512Generate(E, iBufLen, input);
    }
    key = input;
    iv = input + 16;
    ++i;
  }
  if (hash) {
    memcpy(hash, input, 32);
  }
}

}  // namespace

CPDF_SecurityHandler::CPDF_SecurityHandler() = default;

CPDF_SecurityHandler::~CPDF_SecurityHandler() = default;

bool CPDF_SecurityHandler::OnInit(const CPDF_Dictionary* pEncryptDict,
                                  const CPDF_Array* pIdArray,
                                  const ByteString& password) {
  if (pIdArray)
    m_FileId = pIdArray->GetStringAt(0);
  else
    m_FileId.clear();
  if (!LoadDict(pEncryptDict))
    return false;
  if (m_Cipher == FXCIPHER_NONE)
    return true;
  if (!CheckSecurity(password))
    return false;

  InitCryptoHandler();
  return true;
}

bool CPDF_SecurityHandler::CheckSecurity(const ByteString& password) {
  if (!password.IsEmpty() && CheckPassword(password, true)) {
    m_bOwnerUnlocked = true;
    return true;
  }
  return CheckPassword(password, false);
}

uint32_t CPDF_SecurityHandler::GetPermissions() const {
  uint32_t dwPermission = m_bOwnerUnlocked ? 0xFFFFFFFF : m_Permissions;
  if (m_pEncryptDict && m_pEncryptDict->GetStringFor("Filter") == "Standard") {
    // See PDF Reference 1.7, page 123, table 3.20.
    dwPermission &= 0xFFFFFFFC;
    dwPermission |= 0xFFFFF0C0;
  }
  return dwPermission;
}

static bool LoadCryptInfo(const CPDF_Dictionary* pEncryptDict,
                          const ByteString& name,
                          int* cipher,
                          size_t* keylen_out) {
  int Version = pEncryptDict->GetIntegerFor("V");
  *cipher = FXCIPHER_RC4;
  *keylen_out = 0;
  int keylen = 0;
  if (Version >= 4) {
    const CPDF_Dictionary* pCryptFilters = pEncryptDict->GetDictFor("CF");
    if (!pCryptFilters)
      return false;

    if (name == "Identity") {
      *cipher = FXCIPHER_NONE;
    } else {
      const CPDF_Dictionary* pDefFilter = pCryptFilters->GetDictFor(name);
      if (!pDefFilter)
        return false;

      int nKeyBits = 0;
      if (Version == 4) {
        nKeyBits = pDefFilter->GetIntegerFor("Length", 0);
        if (nKeyBits == 0) {
          nKeyBits = pEncryptDict->GetIntegerFor("Length", 128);
        }
      } else {
        nKeyBits = pEncryptDict->GetIntegerFor("Length", 256);
      }
      if (nKeyBits < 0)
        return false;

      if (nKeyBits < 40) {
        nKeyBits *= 8;
      }
      keylen = nKeyBits / 8;
      ByteString cipher_name = pDefFilter->GetStringFor("CFM");
      if (cipher_name == "AESV2" || cipher_name == "AESV3")
        *cipher = FXCIPHER_AES;
    }
  } else {
    keylen = Version > 1 ? pEncryptDict->GetIntegerFor("Length", 40) / 8 : 5;
  }

  if (keylen < 0 || keylen > 32)
    return false;
  if (!IsValidKeyLengthForCipher(*cipher, keylen))
    return false;

  *keylen_out = keylen;
  return true;
}

bool CPDF_SecurityHandler::LoadDict(const CPDF_Dictionary* pEncryptDict) {
  m_pEncryptDict.Reset(pEncryptDict);
  m_Version = pEncryptDict->GetIntegerFor("V");
  m_Revision = pEncryptDict->GetIntegerFor("R");
  m_Permissions = pEncryptDict->GetIntegerFor("P", -1);
  if (m_Version < 4)
    return LoadCryptInfo(pEncryptDict, ByteString(), &m_Cipher, &m_KeyLen);

  ByteString stmf_name = pEncryptDict->GetStringFor("StmF");
  ByteString strf_name = pEncryptDict->GetStringFor("StrF");
  if (stmf_name != strf_name)
    return false;

  return LoadCryptInfo(pEncryptDict, strf_name, &m_Cipher, &m_KeyLen);
}

bool CPDF_SecurityHandler::LoadDict(const CPDF_Dictionary* pEncryptDict,
                                    int* cipher,
                                    size_t* key_len) {
  m_pEncryptDict.Reset(pEncryptDict);
  m_Version = pEncryptDict->GetIntegerFor("V");
  m_Revision = pEncryptDict->GetIntegerFor("R");
  m_Permissions = pEncryptDict->GetIntegerFor("P", -1);

  ByteString strf_name;
  ByteString stmf_name;
  if (m_Version >= 4) {
    stmf_name = pEncryptDict->GetStringFor("StmF");
    strf_name = pEncryptDict->GetStringFor("StrF");
    if (stmf_name != strf_name)
      return false;
  }
  if (!LoadCryptInfo(pEncryptDict, strf_name, cipher, key_len))
    return false;

  m_Cipher = *cipher;
  m_KeyLen = *key_len;
  return true;
}

bool CPDF_SecurityHandler::AES256_CheckPassword(const ByteString& password,
                                                bool bOwner) {
  ASSERT(m_pEncryptDict);
  ASSERT(m_Revision >= 5);

  ByteString okey = m_pEncryptDict->GetStringFor("O");
  if (okey.GetLength() < 48)
    return false;

  ByteString ukey = m_pEncryptDict->GetStringFor("U");
  if (ukey.GetLength() < 48)
    return false;

  const uint8_t* pkey = bOwner ? okey.raw_str() : ukey.raw_str();
  CRYPT_sha2_context sha;
  uint8_t digest[32];
  if (m_Revision >= 6) {
    Revision6_Hash(password, (const uint8_t*)pkey + 32,
                   bOwner ? ukey.raw_str() : nullptr, digest);
  } else {
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, password.raw_str(), password.GetLength());
    CRYPT_SHA256Update(&sha, pkey + 32, 8);
    if (bOwner)
      CRYPT_SHA256Update(&sha, ukey.raw_str(), 48);
    CRYPT_SHA256Finish(&sha, digest);
  }
  if (memcmp(digest, pkey, 32) != 0)
    return false;

  if (m_Revision >= 6) {
    Revision6_Hash(password, (const uint8_t*)pkey + 40,
                   bOwner ? ukey.raw_str() : nullptr, digest);
  } else {
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, password.raw_str(), password.GetLength());
    CRYPT_SHA256Update(&sha, pkey + 40, 8);
    if (bOwner)
      CRYPT_SHA256Update(&sha, ukey.raw_str(), 48);
    CRYPT_SHA256Finish(&sha, digest);
  }
  ByteString ekey = m_pEncryptDict->GetStringFor(bOwner ? "OE" : "UE");
  if (ekey.GetLength() < 32)
    return false;

  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, digest, sizeof(digest), false);
  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);
  CRYPT_AESDecrypt(&aes, m_EncryptKey, ekey.raw_str(), 32);
  CRYPT_AESSetKey(&aes, m_EncryptKey, sizeof(m_EncryptKey), false);
  CRYPT_AESSetIV(&aes, iv);
  ByteString perms = m_pEncryptDict->GetStringFor("Perms");
  if (perms.IsEmpty())
    return false;

  uint8_t perms_buf[16] = {};
  size_t copy_len =
      std::min(sizeof(perms_buf), static_cast<size_t>(perms.GetLength()));
  memcpy(perms_buf, perms.raw_str(), copy_len);
  uint8_t buf[16];
  CRYPT_AESDecrypt(&aes, buf, perms_buf, 16);
  if (buf[9] != 'a' || buf[10] != 'd' || buf[11] != 'b')
    return false;

  if (FXDWORD_GET_LSBFIRST(buf) != m_Permissions)
    return false;

  // Relax this check as there appear to be some non-conforming documents
  // in the wild. The value in the buffer is the truth; if it requires us
  // to encrypt metadata, but the dictionary says otherwise, then we may
  // have a tampered doc.  Otherwise, give it a pass.
  return buf[8] == 'F' || IsMetadataEncrypted();
}

bool CPDF_SecurityHandler::CheckPassword(const ByteString& password,
                                         bool bOwner) {
  DCHECK_EQ(kUnknown, m_PasswordEncodingConversion);
  if (CheckPasswordImpl(password, bOwner)) {
    m_PasswordEncodingConversion = kNone;
    return true;
  }

  ByteStringView password_view = password.AsStringView();
  if (password_view.IsASCII())
    return false;

  if (m_Revision >= 5) {
    ByteString utf8_password = WideString::FromLatin1(password_view).ToUTF8();
    if (!CheckPasswordImpl(utf8_password, bOwner))
      return false;

    m_PasswordEncodingConversion = kLatin1ToUtf8;
    return true;
  }

  ByteString latin1_password = WideString::FromUTF8(password_view).ToLatin1();
  if (!CheckPasswordImpl(latin1_password, bOwner))
    return false;

  m_PasswordEncodingConversion = kUtf8toLatin1;
  return true;
}

bool CPDF_SecurityHandler::CheckPasswordImpl(const ByteString& password,
                                             bool bOwner) {
  if (m_Revision >= 5)
    return AES256_CheckPassword(password, bOwner);

  if (bOwner)
    return CheckOwnerPassword(password);

  return CheckUserPassword(password, false) ||
         CheckUserPassword(password, true);
}

bool CPDF_SecurityHandler::CheckUserPassword(const ByteString& password,
                                             bool bIgnoreEncryptMeta) {
  CalcEncryptKey(m_pEncryptDict.Get(), password, m_EncryptKey, m_KeyLen,
                 bIgnoreEncryptMeta, m_FileId);
  ByteString ukey =
      m_pEncryptDict ? m_pEncryptDict->GetStringFor("U") : ByteString();
  if (ukey.GetLength() < 16) {
    return false;
  }

  uint8_t ukeybuf[32];
  if (m_Revision == 2) {
    memcpy(ukeybuf, kDefaultPasscode, sizeof(kDefaultPasscode));
    CRYPT_ArcFourCryptBlock(ukeybuf, {m_EncryptKey, m_KeyLen});
    return memcmp(ukey.c_str(), ukeybuf, 16) == 0;
  }

  uint8_t test[32] = {};
  uint8_t tmpkey[32] = {};
  uint32_t copy_len = std::min(sizeof(test), ukey.GetLength());

  memcpy(test, ukey.c_str(), copy_len);
  for (int32_t i = 19; i >= 0; i--) {
    for (size_t j = 0; j < m_KeyLen; j++)
      tmpkey[j] = m_EncryptKey[j] ^ static_cast<uint8_t>(i);
    CRYPT_ArcFourCryptBlock(test, {tmpkey, m_KeyLen});
  }
  CRYPT_md5_context md5 = CRYPT_MD5Start();
  CRYPT_MD5Update(&md5, kDefaultPasscode);
  if (!m_FileId.IsEmpty())
    CRYPT_MD5Update(&md5, m_FileId.raw_span());
  CRYPT_MD5Finish(&md5, ukeybuf);
  return memcmp(test, ukeybuf, 16) == 0;
}

ByteString CPDF_SecurityHandler::GetUserPassword(
    const ByteString& owner_password) const {
  constexpr size_t kRequiredOkeyLength = 32;
  ByteString okey = m_pEncryptDict->GetStringFor("O");
  size_t okeylen = std::min<size_t>(okey.GetLength(), kRequiredOkeyLength);
  if (okeylen < kRequiredOkeyLength)
    return ByteString();

  DCHECK_EQ(kRequiredOkeyLength, okeylen);
  uint8_t passcode[32];
  GetPassCode(owner_password, passcode);
  uint8_t digest[16];
  CRYPT_MD5Generate(passcode, digest);
  if (m_Revision >= 3) {
    for (uint32_t i = 0; i < 50; i++)
      CRYPT_MD5Generate(digest, digest);
  }
  uint8_t enckey[32] = {};
  size_t copy_len = std::min(m_KeyLen, sizeof(digest));

  memcpy(enckey, digest, copy_len);
  uint8_t okeybuf[32] = {};
  memcpy(okeybuf, okey.c_str(), okeylen);
  pdfium::span<uint8_t> okey_span(okeybuf, okeylen);
  if (m_Revision == 2) {
    CRYPT_ArcFourCryptBlock(okey_span, {enckey, m_KeyLen});
  } else {
    for (int32_t i = 19; i >= 0; i--) {
      uint8_t tempkey[32] = {};
      for (size_t j = 0; j < m_KeyLen; j++)
        tempkey[j] = enckey[j] ^ static_cast<uint8_t>(i);
      CRYPT_ArcFourCryptBlock(okey_span, {tempkey, m_KeyLen});
    }
  }
  size_t len = kRequiredOkeyLength;
  while (len && kDefaultPasscode[len - 1] == okey_span[len - 1])
    len--;

  return ByteString(okeybuf, len);
}

bool CPDF_SecurityHandler::CheckOwnerPassword(const ByteString& password) {
  ByteString user_pass = GetUserPassword(password);
  return CheckUserPassword(user_pass, false) ||
         CheckUserPassword(user_pass, true);
}

bool CPDF_SecurityHandler::IsMetadataEncrypted() const {
  return m_pEncryptDict->GetBooleanFor("EncryptMetadata", true);
}

ByteString CPDF_SecurityHandler::GetEncodedPassword(
    ByteStringView password) const {
  switch (m_PasswordEncodingConversion) {
    case kNone:
      // Do nothing.
      return ByteString(password);
    case kLatin1ToUtf8:
      return WideString::FromLatin1(password).ToUTF8();
    case kUtf8toLatin1:
      return WideString::FromUTF8(password).ToLatin1();
    default:
      NOTREACHED();
      return ByteString(password);
  }
}

void CPDF_SecurityHandler::OnCreateInternal(CPDF_Dictionary* pEncryptDict,
                                            const CPDF_Array* pIdArray,
                                            const ByteString& user_password,
                                            const ByteString& owner_password,
                                            bool bDefault) {
  ASSERT(pEncryptDict);

  int cipher = FXCIPHER_NONE;
  size_t key_len = 0;
  if (!LoadDict(pEncryptDict, &cipher, &key_len)) {
    return;
  }
  ByteString owner_password_copy = owner_password;
  if (bDefault && owner_password.IsEmpty())
    owner_password_copy = user_password;

  if (m_Revision >= 5) {
    uint32_t random[4];
    FX_Random_GenerateMT(random, FX_ArraySize(random));
    CRYPT_sha2_context sha;
    CRYPT_SHA256Start(&sha);
    CRYPT_SHA256Update(&sha, reinterpret_cast<uint8_t*>(random),
                       sizeof(random));
    CRYPT_SHA256Finish(&sha, m_EncryptKey);
    AES256_SetPassword(pEncryptDict, user_password, false);
    if (bDefault)
      AES256_SetPassword(pEncryptDict, owner_password_copy, true);
    AES256_SetPerms(pEncryptDict);
    return;
  }
  if (bDefault) {
    uint8_t passcode[32];
    GetPassCode(owner_password_copy, passcode);
    uint8_t digest[16];
    CRYPT_MD5Generate(passcode, digest);
    if (m_Revision >= 3) {
      for (uint32_t i = 0; i < 50; i++)
        CRYPT_MD5Generate(digest, digest);
    }
    uint8_t enckey[32];
    memcpy(enckey, digest, key_len);
    GetPassCode(user_password, passcode);
    CRYPT_ArcFourCryptBlock(passcode, {enckey, key_len});
    uint8_t tempkey[32];
    if (m_Revision >= 3) {
      for (uint8_t i = 1; i <= 19; i++) {
        for (size_t j = 0; j < key_len; j++)
          tempkey[j] = enckey[j] ^ i;
        CRYPT_ArcFourCryptBlock(passcode, {tempkey, key_len});
      }
    }
    pEncryptDict->SetNewFor<CPDF_String>("O", ByteString(passcode, 32), false);
  }

  ByteString file_id;
  if (pIdArray)
    file_id = pIdArray->GetStringAt(0);

  CalcEncryptKey(m_pEncryptDict.Get(), user_password, m_EncryptKey, key_len,
                 false, file_id);
  if (m_Revision < 3) {
    uint8_t tempbuf[32];
    memcpy(tempbuf, kDefaultPasscode, sizeof(kDefaultPasscode));
    CRYPT_ArcFourCryptBlock(tempbuf, {m_EncryptKey, key_len});
    pEncryptDict->SetNewFor<CPDF_String>("U", ByteString(tempbuf, 32), false);
  } else {
    CRYPT_md5_context md5 = CRYPT_MD5Start();
    CRYPT_MD5Update(&md5, kDefaultPasscode);
    if (!file_id.IsEmpty())
      CRYPT_MD5Update(&md5, file_id.raw_span());

    uint8_t digest[32];
    CRYPT_MD5Finish(&md5, digest);
    pdfium::span<uint8_t> partial_digest_span(digest, 16);
    CRYPT_ArcFourCryptBlock(partial_digest_span, {m_EncryptKey, key_len});
    uint8_t tempkey[32];
    for (uint8_t i = 1; i <= 19; i++) {
      for (size_t j = 0; j < key_len; j++)
        tempkey[j] = m_EncryptKey[j] ^ i;
      CRYPT_ArcFourCryptBlock(partial_digest_span, {tempkey, key_len});
    }
    CRYPT_MD5Generate({digest, 16}, digest + 16);
    pEncryptDict->SetNewFor<CPDF_String>("U", ByteString(digest, 32), false);
  }
}

void CPDF_SecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict,
                                    const CPDF_Array* pIdArray,
                                    const ByteString& user_password,
                                    const ByteString& owner_password) {
  OnCreateInternal(pEncryptDict, pIdArray, user_password, owner_password, true);
  InitCryptoHandler();
}

void CPDF_SecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict,
                                    const CPDF_Array* pIdArray,
                                    const ByteString& user_password) {
  OnCreateInternal(pEncryptDict, pIdArray, user_password, ByteString(), false);
  InitCryptoHandler();
}

void CPDF_SecurityHandler::AES256_SetPassword(CPDF_Dictionary* pEncryptDict,
                                              const ByteString& password,
                                              bool bOwner) {
  CRYPT_sha1_context sha;
  CRYPT_SHA1Start(&sha);
  CRYPT_SHA1Update(&sha, m_EncryptKey, sizeof(m_EncryptKey));
  CRYPT_SHA1Update(&sha, (uint8_t*)"hello", 5);

  uint8_t digest[20];
  CRYPT_SHA1Finish(&sha, digest);

  ByteString ukey = pEncryptDict->GetStringFor("U");
  CRYPT_sha2_context sha2;
  uint8_t digest1[48];
  if (m_Revision >= 6) {
    Revision6_Hash(password, digest, bOwner ? ukey.raw_str() : nullptr,
                   digest1);
  } else {
    CRYPT_SHA256Start(&sha2);
    CRYPT_SHA256Update(&sha2, password.raw_str(), password.GetLength());
    CRYPT_SHA256Update(&sha2, digest, 8);
    if (bOwner) {
      CRYPT_SHA256Update(&sha2, ukey.raw_str(), ukey.GetLength());
    }
    CRYPT_SHA256Finish(&sha2, digest1);
  }
  memcpy(digest1 + 32, digest, 16);
  pEncryptDict->SetNewFor<CPDF_String>(bOwner ? "O" : "U",
                                       ByteString(digest1, 48), false);
  if (m_Revision >= 6) {
    Revision6_Hash(password, digest + 8, bOwner ? ukey.raw_str() : nullptr,
                   digest1);
  } else {
    CRYPT_SHA256Start(&sha2);
    CRYPT_SHA256Update(&sha2, password.raw_str(), password.GetLength());
    CRYPT_SHA256Update(&sha2, digest + 8, 8);
    if (bOwner) {
      CRYPT_SHA256Update(&sha2, ukey.raw_str(), ukey.GetLength());
    }
    CRYPT_SHA256Finish(&sha2, digest1);
  }
  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, digest1, 32, true);
  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);
  CRYPT_AESEncrypt(&aes, digest1, m_EncryptKey, sizeof(m_EncryptKey));
  pEncryptDict->SetNewFor<CPDF_String>(bOwner ? "OE" : "UE",
                                       ByteString(digest1, 32), false);
}

void CPDF_SecurityHandler::AES256_SetPerms(CPDF_Dictionary* pEncryptDict) {
  uint8_t buf[16];
  buf[0] = static_cast<uint8_t>(m_Permissions);
  buf[1] = static_cast<uint8_t>(m_Permissions >> 8);
  buf[2] = static_cast<uint8_t>(m_Permissions >> 16);
  buf[3] = static_cast<uint8_t>(m_Permissions >> 24);
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
  uint32_t* buf_random = reinterpret_cast<uint32_t*>(&buf[12]);
  FX_Random_GenerateMT(buf_random, 1);

  CRYPT_aes_context aes = {};
  CRYPT_AESSetKey(&aes, m_EncryptKey, sizeof(m_EncryptKey), true);

  uint8_t iv[16] = {};
  CRYPT_AESSetIV(&aes, iv);

  uint8_t buf1[16];
  CRYPT_AESEncrypt(&aes, buf1, buf, 16);
  pEncryptDict->SetNewFor<CPDF_String>("Perms", ByteString(buf1, 16), false);
}

void CPDF_SecurityHandler::InitCryptoHandler() {
  m_pCryptoHandler =
      pdfium::MakeUnique<CPDF_CryptoHandler>(m_Cipher, m_EncryptKey, m_KeyLen);
}
