// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_crypto_handler.h"

#include <time.h>

#include <algorithm>
#include <array>
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
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_memcpy_wrappers.h"
#include "core/fxcrt/stl_util.h"

namespace {

constexpr char kContentsKey[] = "Contents";
constexpr char kTypeKey[] = "Type";

struct AESCryptContext {
  bool iv_;
  uint32_t block_offset_;
  CRYPT_aes_context context_;
  std::array<uint8_t, 16> block_;
};

}  // namespace

// static
bool CPDF_CryptoHandler::IsSignatureDictionary(
    const CPDF_Dictionary* dictionary) {
  if (!dictionary) {
    return false;
  }
  RetainPtr<const CPDF_Object> type_obj =
      dictionary->GetDirectObjectFor(kTypeKey);
  if (!type_obj) {
    type_obj = dictionary->GetDirectObjectFor(pdfium::form_fields::kFT);
  }
  return type_obj && type_obj->GetString() == pdfium::form_fields::kSig;
}

DataVector<uint8_t> CPDF_CryptoHandler::EncryptContent(
    uint32_t objnum,
    uint32_t gennum,
    pdfium::span<const uint8_t> source) const {
  if (cipher_ == Cipher::kNone) {
    return DataVector<uint8_t>(source.begin(), source.end());
  }
  std::array<uint8_t, 16> realkey;
  size_t realkeylen = realkey.size();
  if (cipher_ != Cipher::kAES || key_len_ != 32) {
    std::array<uint8_t, 32> key1;
    PopulateKey(objnum, gennum, key1);
    if (cipher_ == Cipher::kAES) {
      fxcrt::Copy(ByteStringView("sAlT").unsigned_span(),
                  pdfium::span(key1).subspan(key_len_ + 5));
    }
    size_t len = cipher_ == Cipher::kAES ? key_len_ + 9 : key_len_ + 5;
    CRYPT_MD5Generate(pdfium::span(key1).first(len), realkey);
    realkeylen = std::min(key_len_ + 5, realkeylen);
  }
  if (cipher_ == Cipher::kAES) {
    if (key_len_ == 32) {
      CRYPT_AESSetKey(aes_context_.get(), encrypt_key_);
    } else {
      CRYPT_AESSetKey(aes_context_.get(),
                      pdfium::span(realkey).first(key_len_));
    }

    static constexpr size_t kIVSize = 16;
    static constexpr size_t kPaddingSize = 16;
    const size_t source_padding_size = source.size() % kPaddingSize;
    const size_t source_data_size = source.size() - source_padding_size;

    DataVector<uint8_t> dest(kIVSize + source_data_size + kPaddingSize);
    auto dest_span = pdfium::span(dest);
    auto dest_iv_span = dest_span.first<kIVSize>();
    auto dest_data_span = dest_span.subspan(kIVSize, source_data_size);
    auto dest_padding_span = dest_span.subspan(kIVSize + source_data_size);

    for (auto& v : dest_iv_span) {
      v = static_cast<uint8_t>(rand());
    }
    CRYPT_AESSetIV(aes_context_.get(), dest_iv_span);
    CRYPT_AESEncrypt(aes_context_.get(), dest_data_span,
                     source.first(source_data_size));

    std::array<uint8_t, kPaddingSize> padding;
    fxcrt::Copy(source.subspan(source_data_size, source_padding_size), padding);
    std::ranges::fill(pdfium::span(padding).subspan(source_padding_size),
                      16 - source_padding_size);
    CRYPT_AESEncrypt(aes_context_.get(), dest_padding_span, padding);
    return dest;
  }
  DataVector<uint8_t> dest(source.begin(), source.end());
  CRYPT_ArcFourCryptBlock(dest, pdfium::span(realkey).first(realkeylen));
  return dest;
}

void* CPDF_CryptoHandler::DecryptStart(uint32_t objnum, uint32_t gennum) {
  if (cipher_ == Cipher::kNone) {
    return this;
  }

  if (cipher_ == Cipher::kAES) {
    AESCryptContext* context = FX_Alloc(AESCryptContext, 1);
    context->iv_ = true;
    context->block_offset_ = 0;
    if (key_len_ == 32) {
      CRYPT_AESSetKey(&context->context_, encrypt_key_);
      return context;
    }
    std::array<uint8_t, 48> key1;
    PopulateKey(objnum, gennum, key1);
    fxcrt::Copy(ByteStringView("sAlT").unsigned_span(),
                pdfium::span(key1).subspan(key_len_ + 5));

    std::array<uint8_t, 16> realkey;
    CRYPT_MD5Generate(pdfium::span(key1).first(key_len_ + 9), realkey);
    CRYPT_AESSetKey(&context->context_, realkey);
    return context;
  }

  std::array<uint8_t, 48> key1;
  PopulateKey(objnum, gennum, key1);

  std::array<uint8_t, 16> realkey;
  CRYPT_MD5Generate(pdfium::span(key1).first(key_len_ + 5), realkey);
  size_t realkeylen = std::min(key_len_ + 5, realkey.size());

  CRYPT_rc4_context* context = FX_Alloc(CRYPT_rc4_context, 1);
  CRYPT_ArcFourSetup(context, pdfium::span(realkey).first(realkeylen));
  return context;
}

bool CPDF_CryptoHandler::DecryptStream(void* context,
                                       pdfium::span<const uint8_t> source,
                                       BinaryBuffer& dest_buf) {
  if (!context) {
    return false;
  }

  if (cipher_ == Cipher::kNone) {
    dest_buf.AppendSpan(source);
    return true;
  }
  if (cipher_ == Cipher::kRC4) {
    size_t old_size = dest_buf.GetSize();
    dest_buf.AppendSpan(source);
    CRYPT_ArcFourCrypt(
        static_cast<CRYPT_rc4_context*>(context),
        dest_buf.GetMutableSpan().subspan(old_size, source.size()));
    return true;
  }
  AESCryptContext* ctx = static_cast<AESCryptContext*>(context);
  uint32_t src_off = 0;
  uint32_t src_left = source.size();
  while (true) {
    uint32_t copy_size = 16 - ctx->block_offset_;
    if (copy_size > src_left) {
      copy_size = src_left;
    }
    fxcrt::Copy(source.subspan(src_off, copy_size),
                pdfium::span(ctx->block_).subspan(ctx->block_offset_));

    src_off += copy_size;
    src_left -= copy_size;
    ctx->block_offset_ += copy_size;
    if (ctx->block_offset_ == 16) {
      if (ctx->iv_) {
        CRYPT_AESSetIV(&ctx->context_, ctx->block_);
        ctx->iv_ = false;
        ctx->block_offset_ = 0;
      } else if (src_off < source.size()) {
        std::array<uint8_t, 16> block_buf;
        CRYPT_AESDecrypt(&ctx->context_, block_buf, ctx->block_);
        dest_buf.AppendSpan(block_buf);
        ctx->block_offset_ = 0;
      }
    }
    if (!src_left) {
      break;
    }
  }
  return true;
}

bool CPDF_CryptoHandler::DecryptFinish(void* context, BinaryBuffer& dest_buf) {
  if (!context) {
    return false;
  }
  if (cipher_ == Cipher::kNone) {
    return true;
  }
  if (cipher_ == Cipher::kRC4) {
    FX_Free(context);
    return true;
  }
  auto* ctx = static_cast<AESCryptContext*>(context);
  if (ctx->block_offset_ == 16) {
    std::array<uint8_t, 16> block_buf;
    CRYPT_AESDecrypt(&ctx->context_, block_buf, ctx->block_);
    if (block_buf.back() < 16) {
      dest_buf.AppendSpan(pdfium::span(block_buf).first(
          static_cast<size_t>(16 - block_buf.back())));
    }
  }
  FX_Free(ctx);
  return true;
}

ByteString CPDF_CryptoHandler::Decrypt(uint32_t objnum,
                                       uint32_t gennum,
                                       const ByteString& str) {
  BinaryBuffer dest_buf;
  void* context = DecryptStart(objnum, gennum);
  DecryptStream(context, str.unsigned_span(), dest_buf);
  DecryptFinish(context, dest_buf);
  return ByteString(ByteStringView(dest_buf.GetSpan()));
}

size_t CPDF_CryptoHandler::DecryptGetSize(size_t src_size) {
  return cipher_ == Cipher::kAES ? src_size - 16 : src_size;
}

bool CPDF_CryptoHandler::IsCipherAES() const {
  return cipher_ == Cipher::kAES;
}

bool CPDF_CryptoHandler::DecryptObjectTree(RetainPtr<CPDF_Object> object) {
  if (!object) {
    return false;
  }

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

CPDF_CryptoHandler::CPDF_CryptoHandler(Cipher cipher,
                                       pdfium::span<const uint8_t> key)
    : key_len_(std::min<size_t>(key.size(), 32)), cipher_(cipher) {
  DCHECK(cipher != Cipher::kAES || key.size() == 16 || key.size() == 24 ||
         key.size() == 32);
  DCHECK(cipher != Cipher::kAES2 || key.size() == 32);
  DCHECK(cipher != Cipher::kRC4 || (key.size() >= 5 && key.size() <= 16));

  if (cipher_ != Cipher::kNone) {
    fxcrt::Copy(key.first(key_len_), encrypt_key_);
  }
  if (cipher_ == Cipher::kAES) {
    aes_context_.reset(FX_Alloc(CRYPT_aes_context, 1));
  }
}

CPDF_CryptoHandler::~CPDF_CryptoHandler() = default;

void CPDF_CryptoHandler::PopulateKey(uint32_t objnum,
                                     uint32_t gennum,
                                     pdfium::span<uint8_t> key) const {
  fxcrt::Copy(pdfium::span(encrypt_key_).first(key_len_), key);
  key[key_len_ + 0] = (uint8_t)objnum;
  key[key_len_ + 1] = (uint8_t)(objnum >> 8);
  key[key_len_ + 2] = (uint8_t)(objnum >> 16);
  key[key_len_ + 3] = (uint8_t)gennum;
  key[key_len_ + 4] = (uint8_t)(gennum >> 8);
}
