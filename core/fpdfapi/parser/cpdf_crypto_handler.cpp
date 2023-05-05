// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_crypto_handler.h"

#include <time.h>

#include <algorithm>
#include <stack>
#include <utility>

#include "constants/form_fields.h"
#include "core/fdrm/fx_crypt.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object_walker.h"
#include "core/fpdfapi/parser/cpdf_parser.h"
#include "core/fpdfapi/parser/cpdf_security_handler.h"
#include "core/fpdfapi/parser/cpdf_simple_parser.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

namespace {

constexpr char kContentsKey[] = "Contents";
constexpr char kTypeKey[] = "Type";

}  // namespace

// static
bool CPDF_CryptoHandler::IsSignatureDictionary(
    const CPDF_Dictionary* dictionary) {
  if (!dictionary)
    return false;
  RetainPtr<const CPDF_Object> type_obj =
      dictionary->GetDirectObjectFor(kTypeKey);
  if (!type_obj)
    type_obj = dictionary->GetDirectObjectFor(pdfium::form_fields::kFT);
  return type_obj && type_obj->GetString() == pdfium::form_fields::kSig;
}

void CPDF_CryptoHandler::EncryptContent(uint32_t objnum,
                                        uint32_t gennum,
                                        pdfium::span<const uint8_t> source,
                                        uint8_t* dest_buf,
                                        size_t& dest_size) const {
  if (m_Cipher == Cipher::kNone) {
    memcpy(dest_buf, source.data(), source.size());
    return;
  }
  uint8_t realkey[16];
  size_t realkeylen = sizeof(realkey);
  if (m_Cipher != Cipher::kAES || m_KeyLen != 32) {
    uint8_t key1[32];
    PopulateKey(objnum, gennum, key1);

    if (m_Cipher == Cipher::kAES)
      memcpy(key1 + m_KeyLen + 5, "sAlT", 4);
    size_t len = m_Cipher == Cipher::kAES ? m_KeyLen + 9 : m_KeyLen + 5;
    CRYPT_MD5Generate({key1, len}, realkey);
    realkeylen = std::min(m_KeyLen + 5, sizeof(realkey));
  }
  if (m_Cipher == Cipher::kAES) {
    CRYPT_AESSetKey(m_pAESContext.get(),
                    m_KeyLen == 32 ? m_EncryptKey : realkey, m_KeyLen);
    uint8_t iv[16];
    for (int i = 0; i < 16; i++) {
      iv[i] = (uint8_t)rand();
    }
    CRYPT_AESSetIV(m_pAESContext.get(), iv);
    memcpy(dest_buf, iv, 16);
    int nblocks = source.size() / 16;
    CRYPT_AESEncrypt(m_pAESContext.get(), dest_buf + 16, source.data(),
                     nblocks * 16);
    uint8_t padding[16];
    memcpy(padding, source.data() + nblocks * 16, source.size() % 16);
    memset(padding + source.size() % 16, 16 - source.size() % 16,
           16 - source.size() % 16);
    CRYPT_AESEncrypt(m_pAESContext.get(), dest_buf + nblocks * 16 + 16, padding,
                     16);
    dest_size = 32 + nblocks * 16;
  } else {
    DCHECK_EQ(dest_size, source.size());
    if (dest_buf != source.data())
      memcpy(dest_buf, source.data(), source.size());
    CRYPT_ArcFourCryptBlock({dest_buf, dest_size}, {realkey, realkeylen});
  }
}

struct AESCryptContext {
  bool m_bIV;
  uint32_t m_BlockOffset;
  CRYPT_aes_context m_Context;
  uint8_t m_Block[16];
};

void* CPDF_CryptoHandler::DecryptStart(uint32_t objnum, uint32_t gennum) {
  if (m_Cipher == Cipher::kNone)
    return this;

  if (m_Cipher == Cipher::kAES && m_KeyLen == 32) {
    AESCryptContext* pContext = FX_Alloc(AESCryptContext, 1);
    pContext->m_bIV = true;
    pContext->m_BlockOffset = 0;
    CRYPT_AESSetKey(&pContext->m_Context, m_EncryptKey, 32);
    return pContext;
  }
  uint8_t key1[48];
  PopulateKey(objnum, gennum, key1);

  if (m_Cipher == Cipher::kAES)
    memcpy(key1 + m_KeyLen + 5, "sAlT", 4);

  uint8_t realkey[16];
  size_t len = m_Cipher == Cipher::kAES ? m_KeyLen + 9 : m_KeyLen + 5;
  CRYPT_MD5Generate({key1, len}, realkey);
  size_t realkeylen = std::min(m_KeyLen + 5, sizeof(realkey));

  if (m_Cipher == Cipher::kAES) {
    AESCryptContext* pContext = FX_Alloc(AESCryptContext, 1);
    pContext->m_bIV = true;
    pContext->m_BlockOffset = 0;
    CRYPT_AESSetKey(&pContext->m_Context, realkey, 16);
    return pContext;
  }
  CRYPT_rc4_context* pContext = FX_Alloc(CRYPT_rc4_context, 1);
  CRYPT_ArcFourSetup(pContext, {realkey, realkeylen});
  return pContext;
}

bool CPDF_CryptoHandler::DecryptStream(void* context,
                                       pdfium::span<const uint8_t> source,
                                       BinaryBuffer& dest_buf) {
  if (!context)
    return false;

  if (m_Cipher == Cipher::kNone) {
    dest_buf.AppendSpan(source);
    return true;
  }
  if (m_Cipher == Cipher::kRC4) {
    size_t old_size = dest_buf.GetSize();
    dest_buf.AppendSpan(source);
    CRYPT_ArcFourCrypt(
        static_cast<CRYPT_rc4_context*>(context),
        dest_buf.GetMutableSpan().subspan(old_size, source.size()));
    return true;
  }
  AESCryptContext* pContext = static_cast<AESCryptContext*>(context);
  uint32_t src_off = 0;
  uint32_t src_left = source.size();
  while (true) {
    uint32_t copy_size = 16 - pContext->m_BlockOffset;
    if (copy_size > src_left) {
      copy_size = src_left;
    }
    memcpy(pContext->m_Block + pContext->m_BlockOffset, source.data() + src_off,
           copy_size);
    src_off += copy_size;
    src_left -= copy_size;
    pContext->m_BlockOffset += copy_size;
    if (pContext->m_BlockOffset == 16) {
      if (pContext->m_bIV) {
        CRYPT_AESSetIV(&pContext->m_Context, pContext->m_Block);
        pContext->m_bIV = false;
        pContext->m_BlockOffset = 0;
      } else if (src_off < source.size()) {
        uint8_t block_buf[16];
        CRYPT_AESDecrypt(&pContext->m_Context, block_buf, pContext->m_Block,
                         16);
        dest_buf.AppendSpan(block_buf);
        pContext->m_BlockOffset = 0;
      }
    }
    if (!src_left) {
      break;
    }
  }
  return true;
}

bool CPDF_CryptoHandler::DecryptFinish(void* context, BinaryBuffer& dest_buf) {
  if (!context)
    return false;

  if (m_Cipher == Cipher::kNone)
    return true;

  if (m_Cipher == Cipher::kRC4) {
    FX_Free(context);
    return true;
  }
  auto* pContext = static_cast<AESCryptContext*>(context);
  if (pContext->m_BlockOffset == 16) {
    uint8_t block_buf[16];
    CRYPT_AESDecrypt(&pContext->m_Context, block_buf, pContext->m_Block, 16);
    if (block_buf[15] < 16) {
      dest_buf.AppendSpan(
          pdfium::make_span(block_buf).first(16 - block_buf[15]));
    }
  }
  FX_Free(pContext);
  return true;
}

ByteString CPDF_CryptoHandler::Decrypt(uint32_t objnum,
                                       uint32_t gennum,
                                       const ByteString& str) {
  BinaryBuffer dest_buf;
  void* context = DecryptStart(objnum, gennum);
  DecryptStream(context, str.raw_span(), dest_buf);
  DecryptFinish(context, dest_buf);
  return ByteString(dest_buf.GetSpan());
}

size_t CPDF_CryptoHandler::DecryptGetSize(size_t src_size) {
  return m_Cipher == Cipher::kAES ? src_size - 16 : src_size;
}

bool CPDF_CryptoHandler::IsCipherAES() const {
  return m_Cipher == Cipher::kAES;
}

bool CPDF_CryptoHandler::DecryptObjectTree(RetainPtr<CPDF_Object> object) {
  if (!object)
    return false;

  struct MayBeSignature {
    RetainPtr<const CPDF_Dictionary> parent;
    RetainPtr<CPDF_Object> contents;
  };

  std::stack<MayBeSignature> may_be_sign_dictionaries;
  const uint32_t obj_num = object->GetObjNum();
  const uint32_t gen_num = object->GetGenNum();

  RetainPtr<CPDF_Object> object_to_decrypt = object;
  while (object_to_decrypt) {
    CPDF_NonConstObjectWalker walker(std::move(object_to_decrypt));
    while (RetainPtr<CPDF_Object> child = walker.GetNext()) {
      RetainPtr<const CPDF_Dictionary> parent_dict =
          walker.GetParent() ? walker.GetParent()->GetDict() : nullptr;
      if (walker.dictionary_key() == kContentsKey &&
          (parent_dict->KeyExist(kTypeKey) ||
           parent_dict->KeyExist(pdfium::form_fields::kFT))) {
        // This object may be contents of signature dictionary.
        // But now values of 'Type' and 'FT' of dictionary keys are encrypted,
        // and we can not check this.
        // Temporary skip it, to prevent signature corruption.
        // It will be decrypted on next interations, if this is not contents of
        // signature dictionary.
        may_be_sign_dictionaries.push(
            {std::move(parent_dict), std::move(child)});
        walker.SkipWalkIntoCurrentObject();
        continue;
      }
      // Strings decryption.
      if (child->IsString()) {
        // TODO(art-snake): Move decryption into the CPDF_String class.
        CPDF_String* str = child->AsMutableString();
        str->SetString(Decrypt(obj_num, gen_num, str->GetString()));
      }
      // Stream decryption.
      if (child->IsStream()) {
        // TODO(art-snake): Move decryption into the CPDF_Stream class.
        CPDF_Stream* stream = child->AsMutableStream();
        auto stream_access =
            pdfium::MakeRetain<CPDF_StreamAcc>(pdfium::WrapRetain(stream));
        stream_access->LoadAllDataRaw();

        if (IsCipherAES() && stream_access->GetSize() < 16) {
          stream->SetData({});
          continue;
        }

        BinaryBuffer decrypted_buf;
        decrypted_buf.EstimateSize(DecryptGetSize(stream_access->GetSize()));

        void* context = DecryptStart(obj_num, gen_num);
        bool decrypt_result =
            DecryptStream(context, stream_access->GetSpan(), decrypted_buf);
        decrypt_result &= DecryptFinish(context, decrypted_buf);
        if (decrypt_result) {
          stream->TakeData(decrypted_buf.DetachBuffer());
        } else {
          // Decryption failed, set the stream to empty
          stream->SetData({});
        }
      }
    }
    // Signature dictionaries check.
    while (!may_be_sign_dictionaries.empty()) {
      auto dict_and_contents = may_be_sign_dictionaries.top();
      may_be_sign_dictionaries.pop();
      if (!IsSignatureDictionary(dict_and_contents.parent)) {
        // This is not signature dictionary. Do decrypt its contents.
        object_to_decrypt = dict_and_contents.contents;
        break;
      }
    }
  }
  return true;
}

size_t CPDF_CryptoHandler::EncryptGetSize(
    pdfium::span<const uint8_t> source) const {
  return m_Cipher == Cipher::kAES ? source.size() + 32 : source.size();
}

CPDF_CryptoHandler::CPDF_CryptoHandler(Cipher cipher,
                                       const uint8_t* key,
                                       size_t keylen)
    : m_KeyLen(std::min<size_t>(keylen, 32)), m_Cipher(cipher) {
  DCHECK(cipher != Cipher::kAES || keylen == 16 || keylen == 24 ||
         keylen == 32);
  DCHECK(cipher != Cipher::kAES2 || keylen == 32);
  DCHECK(cipher != Cipher::kRC4 || (keylen >= 5 && keylen <= 16));

  if (m_Cipher != Cipher::kNone)
    memcpy(m_EncryptKey, key, m_KeyLen);

  if (m_Cipher == Cipher::kAES)
    m_pAESContext.reset(FX_Alloc(CRYPT_aes_context, 1));
}

CPDF_CryptoHandler::~CPDF_CryptoHandler() = default;

void CPDF_CryptoHandler::PopulateKey(uint32_t objnum,
                                     uint32_t gennum,
                                     uint8_t* key) const {
  memcpy(key, m_EncryptKey, m_KeyLen);
  key[m_KeyLen + 0] = (uint8_t)objnum;
  key[m_KeyLen + 1] = (uint8_t)(objnum >> 8);
  key[m_KeyLen + 2] = (uint8_t)(objnum >> 16);
  key[m_KeyLen + 3] = (uint8_t)gennum;
  key[m_KeyLen + 4] = (uint8_t)(gennum >> 8);
}
