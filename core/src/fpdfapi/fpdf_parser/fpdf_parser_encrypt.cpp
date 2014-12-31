// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include <time.h>
#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fdrm/fx_crypt.h"
const FX_BYTE defpasscode[32] = {
    0x28, 0xbf, 0x4e, 0x5e, 0x4e, 0x75, 0x8a, 0x41,
    0x64, 0x00, 0x4e, 0x56, 0xff, 0xfa, 0x01, 0x08,
    0x2e, 0x2e, 0x00, 0xb6, 0xd0, 0x68, 0x3e, 0x80,
    0x2f, 0x0c, 0xa9, 0xfe, 0x64, 0x53, 0x69, 0x7a
};
void CalcEncryptKey(CPDF_Dictionary* pEncrypt, FX_LPCBYTE password, FX_DWORD pass_size,
                    FX_LPBYTE key, int keylen, FX_BOOL bIgnoreMeta, CPDF_Array* pIdArray)
{
    int revision = pEncrypt->GetInteger(FX_BSTRC("R"));
    FX_BYTE passcode[32];
    for (FX_DWORD i = 0; i < 32; i ++) {
        passcode[i] = i < pass_size ? password[i] : defpasscode[i - pass_size];
    }
    FX_BYTE md5[100];
    CRYPT_MD5Start(md5);
    CRYPT_MD5Update(md5, passcode, 32);
    CFX_ByteString okey = pEncrypt->GetString(FX_BSTRC("O"));
    CRYPT_MD5Update(md5, (FX_LPBYTE)okey.c_str(), okey.GetLength());
    FX_DWORD perm = pEncrypt->GetInteger(FX_BSTRC("P"));
    CRYPT_MD5Update(md5, (FX_LPBYTE)&perm, 4);
    if (pIdArray) {
        CFX_ByteString id = pIdArray->GetString(0);
        CRYPT_MD5Update(md5, (FX_LPBYTE)id.c_str(), id.GetLength());
    }
    if (!bIgnoreMeta && revision >= 3 && !pEncrypt->GetInteger(FX_BSTRC("EncryptMetadata"), 1)) {
        FX_DWORD tag = (FX_DWORD) - 1;
        CRYPT_MD5Update(md5, (FX_LPBYTE)&tag, 4);
    }
    FX_BYTE digest[16];
    CRYPT_MD5Finish(md5, digest);
    FX_DWORD copy_len = keylen;
    if (copy_len > sizeof(digest)) {
        copy_len = sizeof(digest);
    }
    if (revision >= 3) {
        for (int i = 0; i < 50; i ++) {
            CRYPT_MD5Generate(digest, copy_len, digest);
        }
    }
    FXSYS_memset32(key, 0, keylen);
    FXSYS_memcpy32(key, digest, copy_len);
}
CPDF_CryptoHandler* CPDF_StandardSecurityHandler::CreateCryptoHandler()
{
    return FX_NEW CPDF_StandardCryptoHandler;
}
typedef struct _PDF_CRYPTOITEM : public CFX_Object {
    FX_INT32	m_Cipher;
    FX_INT32	m_KeyLen;
    FX_BOOL		m_bChecked;
    CPDF_StandardCryptoHandler*	m_pCryptoHandler;
} PDF_CRYPTOITEM;
CPDF_StandardSecurityHandler::CPDF_StandardSecurityHandler()
{
    m_Version = 0;
    m_Revision = 0;
    m_pParser = NULL;
    m_pEncryptDict = NULL;
    m_bOwner = FALSE;
    m_Permissions = 0;
    m_Cipher = FXCIPHER_NONE;
    m_KeyLen = 0;
}
CPDF_StandardSecurityHandler::~CPDF_StandardSecurityHandler()
{
}
FX_BOOL CPDF_StandardSecurityHandler::OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict)
{
    m_pParser = pParser;
    if (!LoadDict(pEncryptDict)) {
        return FALSE;
    }
    if (m_Cipher == FXCIPHER_NONE) {
        return TRUE;
    }
    return CheckSecurity(m_KeyLen);
}
FX_BOOL CPDF_StandardSecurityHandler::CheckSecurity(FX_INT32 key_len)
{
    CFX_ByteString password = m_pParser->GetPassword();
    if (CheckPassword(password, password.GetLength(), TRUE, m_EncryptKey, key_len)) {
        if (password.IsEmpty()) {
            if (!CheckPassword(password, password.GetLength(), FALSE, m_EncryptKey, key_len)) {
                return FALSE;
            }
        }
        m_bOwner = TRUE;
        return TRUE;
    }
    return CheckPassword(password, password.GetLength(), FALSE, m_EncryptKey, key_len);
}
FX_DWORD CPDF_StandardSecurityHandler::GetPermissions()
{
    return m_Permissions;
}
static FX_BOOL _LoadCryptInfo(CPDF_Dictionary* pEncryptDict, FX_BSTR name, int& cipher, int& keylen)
{
    int Version = pEncryptDict->GetInteger(FX_BSTRC("V"));
    cipher = FXCIPHER_RC4;
    keylen = 0;
    if (Version >= 4) {
        CPDF_Dictionary* pCryptFilters = pEncryptDict->GetDict(FX_BSTRC("CF"));
        if (pCryptFilters == NULL) {
            return FALSE;
        }
        if (name == FX_BSTRC("Identity")) {
            cipher = FXCIPHER_NONE;
        } else {
            CPDF_Dictionary* pDefFilter = pCryptFilters->GetDict(name);
            if (pDefFilter == NULL) {
                return FALSE;
            }
            int nKeyBits = 0;
            if (Version == 4) {
                nKeyBits = pDefFilter->GetInteger(FX_BSTRC("Length"), 0);
                if (nKeyBits == 0) {
                    nKeyBits = pEncryptDict->GetInteger(FX_BSTRC("Length"), 128);
                }
            } else {
                nKeyBits = pEncryptDict->GetInteger(FX_BSTRC("Length"), 256);
            }
            if (nKeyBits < 40) {
                nKeyBits *= 8;
            }
            keylen = nKeyBits / 8;
            CFX_ByteString cipher_name = pDefFilter->GetString(FX_BSTRC("CFM"));
            if (cipher_name == FX_BSTRC("AESV2") || cipher_name == FX_BSTRC("AESV3")) {
                cipher = FXCIPHER_AES;
            }
        }
    } else {
        keylen = Version > 1 ? pEncryptDict->GetInteger(FX_BSTRC("Length"), 40) / 8 : 5;
    }
    if (keylen > 32 || keylen < 0) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_StandardSecurityHandler::LoadDict(CPDF_Dictionary* pEncryptDict)
{
    m_pEncryptDict = pEncryptDict;
    m_bOwner = FALSE;
    m_Version = pEncryptDict->GetInteger(FX_BSTRC("V"));
    m_Revision = pEncryptDict->GetInteger(FX_BSTRC("R"));
    m_Permissions = pEncryptDict->GetInteger(FX_BSTRC("P"), -1);
    if (m_Version < 4) {
        return _LoadCryptInfo(pEncryptDict, CFX_ByteString(), m_Cipher, m_KeyLen);
    }
    CFX_ByteString stmf_name = pEncryptDict->GetString(FX_BSTRC("StmF"));
    CFX_ByteString strf_name = pEncryptDict->GetString(FX_BSTRC("StrF"));
    if (stmf_name != strf_name) {
        return FALSE;
    }
    if (!_LoadCryptInfo(pEncryptDict, strf_name, m_Cipher, m_KeyLen)) {
        return FALSE;
    }
    return TRUE;
}
FX_BOOL CPDF_StandardSecurityHandler::LoadDict(CPDF_Dictionary* pEncryptDict, FX_DWORD type, int& cipher, int& key_len)
{
    m_pEncryptDict = pEncryptDict;
    m_bOwner = FALSE;
    m_Version = pEncryptDict->GetInteger(FX_BSTRC("V"));
    m_Revision = pEncryptDict->GetInteger(FX_BSTRC("R"));
    m_Permissions = pEncryptDict->GetInteger(FX_BSTRC("P"), -1);
    CFX_ByteString strf_name, stmf_name;
    if (m_Version >= 4) {
        stmf_name = pEncryptDict->GetString(FX_BSTRC("StmF"));
        strf_name = pEncryptDict->GetString(FX_BSTRC("StrF"));
        if (stmf_name != strf_name) {
            return FALSE;
        }
    }
    if (!_LoadCryptInfo(pEncryptDict, strf_name, cipher, key_len)) {
        return FALSE;
    }
    m_Cipher = cipher;
    m_KeyLen = key_len;
    return TRUE;
    return TRUE;
}
FX_BOOL CPDF_StandardSecurityHandler::GetCryptInfo(int& cipher, FX_LPCBYTE& buffer, int& keylen)
{
    cipher = m_Cipher;
    buffer = m_EncryptKey;
    keylen = m_KeyLen;
    return TRUE;
}
#define FX_GET_32WORD(n,b,i)								\
    {														\
        (n) = (FX_DWORD)(( (FX_UINT64) (b)[(i)] << 24 )			\
                         | ( (FX_UINT64) (b)[(i) + 1] << 16 )					\
                         | ( (FX_UINT64) (b)[(i) + 2] <<  8 )					\
                         | ( (FX_UINT64) (b)[(i) + 3]       ));					\
    }
int BigOrder64BitsMod3(FX_LPBYTE data)
{
    FX_UINT64 ret = 0;
    for (int i = 0; i < 4; ++i) {
        FX_DWORD value;
        FX_GET_32WORD(value, data, 4 * i);
        ret <<= 32;
        ret |= value;
        ret %= 3;
    }
    return (int)ret;
}
void Revision6_Hash(FX_LPCBYTE password, FX_DWORD size, FX_LPCBYTE salt, FX_LPCBYTE vector, FX_LPBYTE hash)
{
    int iBlockSize = 32;
    FX_BYTE sha[128];
    CRYPT_SHA256Start(sha);
    CRYPT_SHA256Update(sha, password, size);
    CRYPT_SHA256Update(sha, salt, 8);
    if (vector) {
        CRYPT_SHA256Update(sha, vector, 48);
    }
    FX_BYTE digest[32];
    CRYPT_SHA256Finish(sha, digest);
    CFX_ByteTextBuf buf;
    FX_LPBYTE input = digest;
    FX_LPBYTE key = input;
    FX_LPBYTE iv = input + 16;
    FX_LPBYTE E = buf.GetBuffer();
    int iBufLen = buf.GetLength();
    CFX_ByteTextBuf interDigest;
    int i = 0;
    FX_LPBYTE aes = FX_Alloc(FX_BYTE, 2048);
    while (i < 64 || i < E[iBufLen - 1]  + 32) {
        int iRoundSize = size + iBlockSize;
        if (vector) {
            iRoundSize += 48;
        }
        iBufLen = iRoundSize * 64;
        buf.EstimateSize(iBufLen);
        E = buf.GetBuffer();
        CFX_ByteTextBuf content;
        for (int j = 0; j < 64; ++j) {
            content.AppendBlock(password, size);
            content.AppendBlock(input, iBlockSize);
            if (vector) {
                content.AppendBlock(vector, 48);
            }
        }
        CRYPT_AESSetKey(aes, 16, key, 16, TRUE);
        CRYPT_AESSetIV(aes, iv);
        CRYPT_AESEncrypt(aes, E, content.GetBuffer(), iBufLen);
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
        interDigest.EstimateSize(iBlockSize);
        input = interDigest.GetBuffer();
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
    FX_Free(aes);
    if (hash) {
        FXSYS_memcpy32(hash, input, 32);
    }
}
FX_BOOL CPDF_StandardSecurityHandler::AES256_CheckPassword(FX_LPCBYTE password, FX_DWORD size,
        FX_BOOL bOwner, FX_LPBYTE key)
{
    CFX_ByteString okey = m_pEncryptDict ? m_pEncryptDict->GetString(FX_BSTRC("O")) : CFX_ByteString();
    if (okey.GetLength() < 48) {
        return FALSE;
    }
    CFX_ByteString ukey = m_pEncryptDict ? m_pEncryptDict->GetString(FX_BSTRC("U")) : CFX_ByteString();
    if (ukey.GetLength() < 48) {
        return FALSE;
    }
    FX_LPCBYTE pkey = bOwner ? (FX_LPCBYTE)okey : (FX_LPCBYTE)ukey;
    FX_BYTE sha[128];
    FX_BYTE digest[32];
    if (m_Revision >= 6) {
        Revision6_Hash(password, size, (FX_LPCBYTE)pkey + 32, (bOwner ? (FX_LPCBYTE)ukey : NULL), digest);
    } else {
        CRYPT_SHA256Start(sha);
        CRYPT_SHA256Update(sha, password, size);
        CRYPT_SHA256Update(sha, pkey + 32, 8);
        if (bOwner) {
            CRYPT_SHA256Update(sha, ukey, 48);
        }
        CRYPT_SHA256Finish(sha, digest);
    }
    if (FXSYS_memcmp32(digest, pkey, 32) != 0) {
        return FALSE;
    }
    if (key == NULL) {
        return TRUE;
    }
    if (m_Revision >= 6) {
        Revision6_Hash(password, size, (FX_LPCBYTE)pkey + 40, (bOwner ? (FX_LPCBYTE)ukey : NULL), digest);
    } else {
        CRYPT_SHA256Start(sha);
        CRYPT_SHA256Update(sha, password, size);
        CRYPT_SHA256Update(sha, pkey + 40, 8);
        if (bOwner) {
            CRYPT_SHA256Update(sha, ukey, 48);
        }
        CRYPT_SHA256Finish(sha, digest);
    }
    CFX_ByteString ekey = m_pEncryptDict ? m_pEncryptDict->GetString(bOwner ? FX_BSTRC("OE") : FX_BSTRC("UE")) : CFX_ByteString();
    if (ekey.GetLength() < 32) {
        return FALSE;
    }
    FX_BYTE* aes = FX_Alloc(FX_BYTE, 2048);
    CRYPT_AESSetKey(aes, 16, digest, 32, FALSE);
    FX_BYTE iv[16];
    FXSYS_memset32(iv, 0, 16);
    CRYPT_AESSetIV(aes, iv);
    CRYPT_AESDecrypt(aes, key, ekey, 32);
    CRYPT_AESSetKey(aes, 16, key, 32, FALSE);
    CRYPT_AESSetIV(aes, iv);
    CFX_ByteString perms = m_pEncryptDict->GetString(FX_BSTRC("Perms"));
    if (perms.IsEmpty()) {
        return FALSE;
    }
    FX_BYTE perms_buf[16];
    FXSYS_memset32(perms_buf, 0, sizeof(perms_buf));
    FX_DWORD copy_len = sizeof(perms_buf);
    if (copy_len > (FX_DWORD)perms.GetLength()) {
        copy_len = perms.GetLength();
    }
    FXSYS_memcpy32(perms_buf, (FX_LPCBYTE)perms, copy_len);
    FX_BYTE buf[16];
    CRYPT_AESDecrypt(aes, buf, perms_buf, 16);
    FX_Free(aes);
    if (buf[9] != 'a' || buf[10] != 'd' || buf[11] != 'b') {
        return FALSE;
    }
    if (FXDWORD_GET_LSBFIRST(buf) != m_Permissions) {
        return FALSE;
    }
    if ((buf[8] == 'T' && !IsMetadataEncrypted()) || (buf[8] == 'F' && IsMetadataEncrypted())) {
        return FALSE;
    }
    return TRUE;
}
int CPDF_StandardSecurityHandler::CheckPassword(FX_LPCBYTE password, FX_DWORD pass_size, FX_BOOL bOwner, FX_LPBYTE key)
{
    return CheckPassword(password, pass_size, bOwner, key, m_KeyLen);
}
int CPDF_StandardSecurityHandler::CheckPassword(FX_LPCBYTE password, FX_DWORD size, FX_BOOL bOwner, FX_LPBYTE key, FX_INT32 key_len)
{
    if (m_Revision >= 5) {
        return AES256_CheckPassword(password, size, bOwner, key);
    }
    FX_BYTE keybuf[32];
    if (key == NULL) {
        key = keybuf;
    }
    if (bOwner) {
        return CheckOwnerPassword(password, size, key, key_len);
    }
    return CheckUserPassword(password, size, FALSE, key, key_len) || CheckUserPassword(password, size, TRUE, key, key_len);
}
FX_BOOL CPDF_StandardSecurityHandler::CheckUserPassword(FX_LPCBYTE password, FX_DWORD pass_size,
        FX_BOOL bIgnoreEncryptMeta, FX_LPBYTE key, FX_INT32 key_len)
{
    CalcEncryptKey(m_pEncryptDict, password, pass_size, key, key_len, bIgnoreEncryptMeta,
                   m_pParser->GetIDArray());
    CFX_ByteString ukey = m_pEncryptDict ? m_pEncryptDict->GetString(FX_BSTRC("U")) : CFX_ByteString();
    if (ukey.GetLength() < 16) {
        return FALSE;
    }
    FX_BYTE ukeybuf[32];
    if (m_Revision == 2) {
        FXSYS_memcpy32(ukeybuf, defpasscode, 32);
        CRYPT_ArcFourCryptBlock(ukeybuf, 32, key, key_len);
    } else {
        FX_BYTE test[32], tmpkey[32];
        FX_DWORD copy_len = sizeof(test);
        if (copy_len > (FX_DWORD)ukey.GetLength()) {
            copy_len = ukey.GetLength();
        }
        FXSYS_memset32(test, 0, sizeof(test));
        FXSYS_memset32(tmpkey, 0, sizeof(tmpkey));
        FXSYS_memcpy32(test, ukey.c_str(), copy_len);
        for (int i = 19; i >= 0; i --) {
            for (int j = 0; j < key_len; j ++) {
                tmpkey[j] = key[j] ^ i;
            }
            CRYPT_ArcFourCryptBlock(test, 32, tmpkey, key_len);
        }
        FX_BYTE md5[100];
        CRYPT_MD5Start(md5);
        CRYPT_MD5Update(md5, defpasscode, 32);
        CPDF_Array* pIdArray = m_pParser->GetIDArray();
        if (pIdArray) {
            CFX_ByteString id = pIdArray->GetString(0);
            CRYPT_MD5Update(md5, (FX_LPBYTE)id.c_str(), id.GetLength());
        }
        CRYPT_MD5Finish(md5, ukeybuf);
        return FXSYS_memcmp32(test, ukeybuf, 16) == 0;
    }
    if (FXSYS_memcmp32((FX_LPVOID)ukey.c_str(), ukeybuf, 16) == 0) {
        return TRUE;
    }
    return FALSE;
}
CFX_ByteString CPDF_StandardSecurityHandler::GetUserPassword(FX_LPCBYTE owner_pass, FX_DWORD pass_size)
{
    return GetUserPassword(owner_pass, pass_size, m_KeyLen);
}
CFX_ByteString CPDF_StandardSecurityHandler::GetUserPassword(FX_LPCBYTE owner_pass, FX_DWORD pass_size, FX_INT32 key_len)
{
    CFX_ByteString okey = m_pEncryptDict->GetString(FX_BSTRC("O"));
    FX_BYTE passcode[32];
    FX_DWORD i;
    for (i = 0; i < 32; i ++) {
        passcode[i] = i < pass_size ? owner_pass[i] : defpasscode[i - pass_size];
    }
    FX_BYTE digest[16];
    CRYPT_MD5Generate(passcode, 32, digest);
    if (m_Revision >= 3) {
        for (int i = 0; i < 50; i ++) {
            CRYPT_MD5Generate(digest, 16, digest);
        }
    }
    FX_BYTE enckey[32];
    FXSYS_memset32(enckey, 0, sizeof(enckey));
    FX_DWORD copy_len = key_len;
    if (copy_len > sizeof(digest)) {
        copy_len = sizeof(digest);
    }
    FXSYS_memcpy32(enckey, digest, copy_len);
    int okeylen = okey.GetLength();
    if (okeylen > 32) {
        okeylen = 32;
    }
    FX_BYTE okeybuf[64];
    FXSYS_memset32(okeybuf, 0, sizeof(okeybuf));
    FXSYS_memcpy32(okeybuf, okey.c_str(), okeylen);
    if (m_Revision == 2) {
        CRYPT_ArcFourCryptBlock(okeybuf, okeylen, enckey, key_len);
    } else {
        for (int i = 19; i >= 0; i --) {
            FX_BYTE tempkey[32];
            FXSYS_memset32(tempkey, 0, sizeof(tempkey));
            for (int j = 0; j < m_KeyLen; j ++) {
                tempkey[j] = enckey[j] ^ i;
            }
            CRYPT_ArcFourCryptBlock(okeybuf, okeylen, tempkey, key_len);
        }
    }
    int len = 32;
    while (len && defpasscode[len - 1] == okeybuf[len - 1]) {
        len --;
    }
    return CFX_ByteString(okeybuf, len);
}
FX_BOOL CPDF_StandardSecurityHandler::CheckOwnerPassword(FX_LPCBYTE password, FX_DWORD pass_size,
        FX_LPBYTE key, FX_INT32 key_len)
{
    CFX_ByteString user_pass = GetUserPassword(password, pass_size, key_len);
    if (CheckUserPassword(user_pass, user_pass.GetLength(), FALSE, key, key_len)) {
        return TRUE;
    }
    return CheckUserPassword(user_pass, user_pass.GetLength(), TRUE, key, key_len);
}
FX_BOOL CPDF_StandardSecurityHandler::IsMetadataEncrypted()
{
    return m_pEncryptDict->GetBoolean(FX_BSTRC("EncryptMetadata"), TRUE);
}
CPDF_SecurityHandler* FPDF_CreateStandardSecurityHandler()
{
    return FX_NEW CPDF_StandardSecurityHandler;
}
void CPDF_StandardSecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
        FX_LPCBYTE user_pass, FX_DWORD user_size,
        FX_LPCBYTE owner_pass, FX_DWORD owner_size, FX_BOOL bDefault, FX_DWORD type)
{
    int cipher = 0, key_len = 0;
    if (!LoadDict(pEncryptDict, type, cipher, key_len)) {
        return;
    }
    if (bDefault && (owner_pass == NULL || owner_size == 0)) {
        owner_pass = user_pass;
        owner_size = user_size;
    }
    if (m_Revision >= 5) {
        int t = (int)time(NULL);
        FX_BYTE sha[128];
        CRYPT_SHA256Start(sha);
        CRYPT_SHA256Update(sha, (FX_BYTE*)&t, sizeof t);
        CRYPT_SHA256Update(sha, m_EncryptKey, 32);
        CRYPT_SHA256Update(sha, (FX_BYTE*)"there", 5);
        CRYPT_SHA256Finish(sha, m_EncryptKey);
        AES256_SetPassword(pEncryptDict, user_pass, user_size, FALSE, m_EncryptKey);
        if (bDefault) {
            AES256_SetPassword(pEncryptDict, owner_pass, owner_size, TRUE, m_EncryptKey);
            AES256_SetPerms(pEncryptDict, m_Permissions, pEncryptDict->GetBoolean(FX_BSTRC("EncryptMetadata"), TRUE), m_EncryptKey);
        }
        return;
    }
    if (bDefault) {
        FX_BYTE passcode[32];
        FX_DWORD i;
        for (i = 0; i < 32; i ++) {
            passcode[i] = i < owner_size ? owner_pass[i] : defpasscode[i - owner_size];
        }
        FX_BYTE digest[16];
        CRYPT_MD5Generate(passcode, 32, digest);
        if (m_Revision >= 3) {
            for (int i = 0; i < 50; i ++) {
                CRYPT_MD5Generate(digest, 16, digest);
            }
        }
        FX_BYTE enckey[32];
        FXSYS_memcpy32(enckey, digest, key_len);
        for (i = 0; i < 32; i ++) {
            passcode[i] = i < user_size ? user_pass[i] : defpasscode[i - user_size];
        }
        CRYPT_ArcFourCryptBlock(passcode, 32, enckey, key_len);
        FX_BYTE tempkey[32];
        if (m_Revision >= 3) {
            for (i = 1; i <= 19; i ++) {
                for (int j = 0; j < key_len; j ++) {
                    tempkey[j] = enckey[j] ^ (FX_BYTE)i;
                }
                CRYPT_ArcFourCryptBlock(passcode, 32, tempkey, key_len);
            }
        }
        pEncryptDict->SetAtString(FX_BSTRC("O"), CFX_ByteString(passcode, 32));
    }
    CalcEncryptKey(m_pEncryptDict, (FX_LPBYTE)user_pass, user_size, m_EncryptKey, key_len, FALSE, pIdArray);
    if (m_Revision < 3) {
        FX_BYTE tempbuf[32];
        FXSYS_memcpy32(tempbuf, defpasscode, 32);
        CRYPT_ArcFourCryptBlock(tempbuf, 32, m_EncryptKey, key_len);
        pEncryptDict->SetAtString(FX_BSTRC("U"), CFX_ByteString(tempbuf, 32));
    } else {
        FX_BYTE md5[100];
        CRYPT_MD5Start(md5);
        CRYPT_MD5Update(md5, defpasscode, 32);
        if (pIdArray) {
            CFX_ByteString id = pIdArray->GetString(0);
            CRYPT_MD5Update(md5, (FX_LPBYTE)id.c_str(), id.GetLength());
        }
        FX_BYTE digest[32];
        CRYPT_MD5Finish(md5, digest);
        CRYPT_ArcFourCryptBlock(digest, 16, m_EncryptKey, key_len);
        FX_BYTE tempkey[32];
        for (int i = 1; i <= 19; i ++) {
            for (int j = 0; j < key_len; j ++) {
                tempkey[j] = m_EncryptKey[j] ^ (FX_BYTE)i;
            }
            CRYPT_ArcFourCryptBlock(digest, 16, tempkey, key_len);
        }
        CRYPT_MD5Generate(digest, 16, digest + 16);
        pEncryptDict->SetAtString(FX_BSTRC("U"), CFX_ByteString(digest, 32));
    }
}
void CPDF_StandardSecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
        FX_LPCBYTE user_pass, FX_DWORD user_size,
        FX_LPCBYTE owner_pass, FX_DWORD owner_size, FX_DWORD type)
{
    OnCreate(pEncryptDict, pIdArray, user_pass, user_size, owner_pass, owner_size, TRUE, type);
}
void CPDF_StandardSecurityHandler::OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray, FX_LPCBYTE user_pass, FX_DWORD user_size, FX_DWORD type)
{
    OnCreate(pEncryptDict, pIdArray, user_pass, user_size, NULL, 0, FALSE, type);
}
void CPDF_StandardSecurityHandler::AES256_SetPassword(CPDF_Dictionary* pEncryptDict, FX_LPCBYTE password, FX_DWORD size, FX_BOOL bOwner, FX_LPCBYTE key)
{
    FX_BYTE sha[128];
    CRYPT_SHA1Start(sha);
    CRYPT_SHA1Update(sha, key, 32);
    CRYPT_SHA1Update(sha, (FX_BYTE*)"hello", 5);
    FX_BYTE digest[20];
    CRYPT_SHA1Finish(sha, digest);
    CFX_ByteString ukey = pEncryptDict->GetString(FX_BSTRC("U"));
    FX_BYTE digest1[48];
    if (m_Revision >= 6) {
        Revision6_Hash(password, size, digest, (bOwner ? (FX_LPCBYTE)ukey : NULL), digest1);
    } else {
        CRYPT_SHA256Start(sha);
        CRYPT_SHA256Update(sha, password, size);
        CRYPT_SHA256Update(sha, digest, 8);
        if (bOwner) {
            CRYPT_SHA256Update(sha, ukey, ukey.GetLength());
        }
        CRYPT_SHA256Finish(sha, digest1);
    }
    FXSYS_memcpy32(digest1 + 32, digest, 16);
    pEncryptDict->SetAtString(bOwner ? FX_BSTRC("O") : FX_BSTRC("U"), CFX_ByteString(digest1, 48));
    if (m_Revision >= 6) {
        Revision6_Hash(password, size, digest + 8, (bOwner ? (FX_LPCBYTE)ukey : NULL), digest1);
    } else {
        CRYPT_SHA256Start(sha);
        CRYPT_SHA256Update(sha, password, size);
        CRYPT_SHA256Update(sha, digest + 8, 8);
        if (bOwner) {
            CRYPT_SHA256Update(sha, ukey, ukey.GetLength());
        }
        CRYPT_SHA256Finish(sha, digest1);
    }
    FX_BYTE* aes = FX_Alloc(FX_BYTE, 2048);
    CRYPT_AESSetKey(aes, 16, digest1, 32, TRUE);
    FX_BYTE iv[16];
    FXSYS_memset32(iv, 0, 16);
    CRYPT_AESSetIV(aes, iv);
    CRYPT_AESEncrypt(aes, digest1, key, 32);
    FX_Free(aes);
    pEncryptDict->SetAtString(bOwner ? FX_BSTRC("OE") : FX_BSTRC("UE"), CFX_ByteString(digest1, 32));
}
void CPDF_StandardSecurityHandler::AES256_SetPerms(CPDF_Dictionary* pEncryptDict, FX_DWORD permissions,
        FX_BOOL bEncryptMetadata, FX_LPCBYTE key)
{
    FX_BYTE buf[16];
    buf[0] = (FX_BYTE)permissions;
    buf[1] = (FX_BYTE)(permissions >> 8);
    buf[2] = (FX_BYTE)(permissions >> 16);
    buf[3] = (FX_BYTE)(permissions >> 24);
    buf[4] = 0xff;
    buf[5] = 0xff;
    buf[6] = 0xff;
    buf[7] = 0xff;
    buf[8] = bEncryptMetadata ? 'T' : 'F';
    buf[9] = 'a';
    buf[10] = 'd';
    buf[11] = 'b';
    FX_BYTE* aes = FX_Alloc(FX_BYTE, 2048);
    CRYPT_AESSetKey(aes, 16, key, 32, TRUE);
    FX_BYTE iv[16], buf1[16];
    FXSYS_memset32(iv, 0, 16);
    CRYPT_AESSetIV(aes, iv);
    CRYPT_AESEncrypt(aes, buf1, buf, 16);
    FX_Free(aes);
    pEncryptDict->SetAtString(FX_BSTRC("Perms"), CFX_ByteString(buf1, 16));
}
void CPDF_StandardCryptoHandler::CryptBlock(FX_BOOL bEncrypt, FX_DWORD objnum, FX_DWORD gennum, FX_LPCBYTE src_buf, FX_DWORD src_size,
        FX_LPBYTE dest_buf, FX_DWORD& dest_size)
{
    if (m_Cipher == FXCIPHER_NONE) {
        FXSYS_memcpy32(dest_buf, src_buf, src_size);
        return;
    }
    FX_BYTE realkey[16];
    int realkeylen = 16;
    if (m_Cipher != FXCIPHER_AES || m_KeyLen != 32) {
        FX_BYTE key1[32];
        FXSYS_memcpy32(key1, m_EncryptKey, m_KeyLen);
        key1[m_KeyLen + 0] = (FX_BYTE)objnum;
        key1[m_KeyLen + 1] = (FX_BYTE)(objnum >> 8);
        key1[m_KeyLen + 2] = (FX_BYTE)(objnum >> 16);
        key1[m_KeyLen + 3] = (FX_BYTE)gennum;
        key1[m_KeyLen + 4] = (FX_BYTE)(gennum >> 8);
        FXSYS_memcpy32(key1 + m_KeyLen, &objnum, 3);
        FXSYS_memcpy32(key1 + m_KeyLen + 3, &gennum, 2);
        if (m_Cipher == FXCIPHER_AES) {
            FXSYS_memcpy32(key1 + m_KeyLen + 5, "sAlT", 4);
        }
        CRYPT_MD5Generate(key1, m_Cipher == FXCIPHER_AES ? m_KeyLen + 9 : m_KeyLen + 5, realkey);
        realkeylen = m_KeyLen + 5;
        if (realkeylen > 16) {
            realkeylen = 16;
        }
    }
    if (m_Cipher == FXCIPHER_AES) {
        CRYPT_AESSetKey(m_pAESContext, 16, m_KeyLen == 32 ? m_EncryptKey : realkey, m_KeyLen, bEncrypt);
        if (bEncrypt) {
            FX_BYTE iv[16];
            for (int i = 0; i < 16; i ++) {
                iv[i] = (FX_BYTE)rand();
            }
            CRYPT_AESSetIV(m_pAESContext, iv);
            FXSYS_memcpy32(dest_buf, iv, 16);
            int nblocks = src_size / 16;
            CRYPT_AESEncrypt(m_pAESContext, dest_buf + 16, src_buf, nblocks * 16);
            FX_BYTE padding[16];
            FXSYS_memcpy32(padding, src_buf + nblocks * 16, src_size % 16);
            FXSYS_memset8(padding + src_size % 16, 16 - src_size % 16, 16 - src_size % 16);
            CRYPT_AESEncrypt(m_pAESContext, dest_buf + nblocks * 16 + 16, padding, 16);
            dest_size = 32 + nblocks * 16;
        } else {
            CRYPT_AESSetIV(m_pAESContext, src_buf);
            CRYPT_AESDecrypt(m_pAESContext, dest_buf, src_buf + 16, src_size - 16);
            dest_size = src_size - 16;
            dest_size -= dest_buf[dest_size - 1];
        }
    } else {
        ASSERT(dest_size == src_size);
        if (dest_buf != src_buf) {
            FXSYS_memcpy32(dest_buf, src_buf, src_size);
        }
        CRYPT_ArcFourCryptBlock(dest_buf, dest_size, realkey, realkeylen);
    }
}
typedef struct _AESCryptContext {
    FX_BYTE		m_Context[2048];
    FX_BOOL		m_bIV;
    FX_BYTE		m_Block[16];
    FX_DWORD	m_BlockOffset;
} AESCryptContext;
FX_LPVOID CPDF_StandardCryptoHandler::CryptStart(FX_DWORD objnum, FX_DWORD gennum, FX_BOOL bEncrypt)
{
    if (m_Cipher == FXCIPHER_NONE) {
        return this;
    }
    if (m_Cipher == FXCIPHER_AES && m_KeyLen == 32) {
        AESCryptContext* pContext = FX_Alloc(AESCryptContext, 1);
        pContext->m_bIV = TRUE;
        pContext->m_BlockOffset = 0;
        CRYPT_AESSetKey(pContext->m_Context, 16, m_EncryptKey, 32, bEncrypt);
        if (bEncrypt) {
            for (int i = 0; i < 16; i ++) {
                pContext->m_Block[i] = (FX_BYTE)rand();
            }
            CRYPT_AESSetIV(pContext->m_Context, pContext->m_Block);
        }
        return pContext;
    }
    FX_BYTE key1[48];
    FXSYS_memcpy32(key1, m_EncryptKey, m_KeyLen);
    FXSYS_memcpy32(key1 + m_KeyLen, &objnum, 3);
    FXSYS_memcpy32(key1 + m_KeyLen + 3, &gennum, 2);
    if (m_Cipher == FXCIPHER_AES) {
        FXSYS_memcpy32(key1 + m_KeyLen + 5, "sAlT", 4);
    }
    FX_BYTE realkey[16];
    CRYPT_MD5Generate(key1, m_Cipher == FXCIPHER_AES ? m_KeyLen + 9 : m_KeyLen + 5, realkey);
    int realkeylen = m_KeyLen + 5;
    if (realkeylen > 16) {
        realkeylen = 16;
    }
    if (m_Cipher == FXCIPHER_AES) {
        AESCryptContext* pContext = FX_Alloc(AESCryptContext, 1);
        pContext->m_bIV = TRUE;
        pContext->m_BlockOffset = 0;
        CRYPT_AESSetKey(pContext->m_Context, 16, realkey, 16, bEncrypt);
        if (bEncrypt) {
            for (int i = 0; i < 16; i ++) {
                pContext->m_Block[i] = (FX_BYTE)rand();
            }
            CRYPT_AESSetIV(pContext->m_Context, pContext->m_Block);
        }
        return pContext;
    }
    void* pContext = FX_Alloc(FX_BYTE, 1040);
    CRYPT_ArcFourSetup(pContext, realkey, realkeylen);
    return pContext;
}
FX_BOOL CPDF_StandardCryptoHandler::CryptStream(FX_LPVOID context, FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf, FX_BOOL bEncrypt)
{
    if (!context) {
        return FALSE;
    }
    if (m_Cipher == FXCIPHER_NONE) {
        dest_buf.AppendBlock(src_buf, src_size);
        return TRUE;
    }
    if (m_Cipher == FXCIPHER_RC4) {
        int old_size = dest_buf.GetSize();
        dest_buf.AppendBlock(src_buf, src_size);
        CRYPT_ArcFourCrypt(context, dest_buf.GetBuffer() + old_size, src_size);
        return TRUE;
    }
    AESCryptContext* pContext = (AESCryptContext*)context;
    if (pContext->m_bIV && bEncrypt) {
        dest_buf.AppendBlock(pContext->m_Block, 16);
        pContext->m_bIV = FALSE;
    }
    FX_DWORD src_off = 0;
    FX_DWORD src_left = src_size;
    while (1) {
        FX_DWORD copy_size = 16 - pContext->m_BlockOffset;
        if (copy_size > src_left) {
            copy_size = src_left;
        }
        FXSYS_memcpy32(pContext->m_Block + pContext->m_BlockOffset, src_buf + src_off, copy_size);
        src_off += copy_size;
        src_left -= copy_size;
        pContext->m_BlockOffset += copy_size;
        if (pContext->m_BlockOffset == 16) {
            if (!bEncrypt && pContext->m_bIV) {
                CRYPT_AESSetIV(pContext->m_Context, pContext->m_Block);
                pContext->m_bIV = FALSE;
                pContext->m_BlockOffset = 0;
            } else if (src_off < src_size) {
                FX_BYTE block_buf[16];
                if (bEncrypt) {
                    CRYPT_AESEncrypt(pContext->m_Context, block_buf, pContext->m_Block, 16);
                } else {
                    CRYPT_AESDecrypt(pContext->m_Context, block_buf, pContext->m_Block, 16);
                }
                dest_buf.AppendBlock(block_buf, 16);
                pContext->m_BlockOffset = 0;
            }
        }
        if (!src_left) {
            break;
        }
    }
    return TRUE;
}
FX_BOOL CPDF_StandardCryptoHandler::CryptFinish(FX_LPVOID context, CFX_BinaryBuf& dest_buf, FX_BOOL bEncrypt)
{
    if (!context) {
        return FALSE;
    }
    if (m_Cipher == FXCIPHER_NONE) {
        return TRUE;
    }
    if (m_Cipher == FXCIPHER_RC4) {
        FX_Free(context);
        return TRUE;
    }
    AESCryptContext* pContext = (AESCryptContext*)context;
    if (bEncrypt) {
        FX_BYTE block_buf[16];
        if (pContext->m_BlockOffset == 16) {
            CRYPT_AESEncrypt(pContext->m_Context, block_buf, pContext->m_Block, 16);
            dest_buf.AppendBlock(block_buf, 16);
            pContext->m_BlockOffset = 0;
        }
        FXSYS_memset8(pContext->m_Block + pContext->m_BlockOffset, (FX_BYTE)(16 - pContext->m_BlockOffset), 16 - pContext->m_BlockOffset);
        CRYPT_AESEncrypt(pContext->m_Context, block_buf, pContext->m_Block, 16);
        dest_buf.AppendBlock(block_buf, 16);
    } else if (pContext->m_BlockOffset == 16) {
        FX_BYTE block_buf[16];
        CRYPT_AESDecrypt(pContext->m_Context, block_buf, pContext->m_Block, 16);
        if (block_buf[15] <= 16) {
            dest_buf.AppendBlock(block_buf, 16 - block_buf[15]);
        }
    }
    FX_Free(pContext);
    return TRUE;
}
FX_LPVOID CPDF_StandardCryptoHandler::DecryptStart(FX_DWORD objnum, FX_DWORD gennum)
{
    return CryptStart(objnum, gennum, FALSE);
}
FX_DWORD CPDF_StandardCryptoHandler::DecryptGetSize(FX_DWORD src_size)
{
    return m_Cipher == FXCIPHER_AES ? src_size - 16 : src_size;
}
FX_BOOL CPDF_StandardCryptoHandler::Init(CPDF_Dictionary* pEncryptDict, CPDF_SecurityHandler* pSecurityHandler)
{
    FX_LPCBYTE key;
    if (!pSecurityHandler->GetCryptInfo(m_Cipher, key, m_KeyLen)) {
        return FALSE;
    }
    if (m_KeyLen > 32 || m_KeyLen < 0) {
        return FALSE;
    }
    if (m_Cipher != FXCIPHER_NONE) {
        FXSYS_memcpy32(m_EncryptKey, key, m_KeyLen);
    }
    if (m_Cipher == FXCIPHER_AES) {
        m_pAESContext = FX_Alloc(FX_BYTE, 2048);
    }
    return TRUE;
}
FX_BOOL CPDF_StandardCryptoHandler::Init(int cipher, FX_LPCBYTE key, int keylen)
{
    if (cipher == FXCIPHER_AES) {
        switch(keylen) {
            case 16:
            case 24:
            case 32:
                break;
            default:
                return FALSE;
        }
    } else if (cipher == FXCIPHER_AES2) {
        if (keylen != 32) {
            return FALSE;
        }
    } else if (cipher == FXCIPHER_RC4) {
        if (keylen < 5 || keylen > 16) {
            return FALSE;
        }
    } else {
        if (keylen > 32) {
            keylen = 32;
        }
    }
    m_Cipher = cipher;
    m_KeyLen = keylen;
    FXSYS_memcpy32(m_EncryptKey, key, keylen);
    if (m_Cipher == FXCIPHER_AES) {
        m_pAESContext = FX_Alloc(FX_BYTE, 2048);
    }
    return TRUE;
}
FX_BOOL CPDF_StandardCryptoHandler::DecryptStream(FX_LPVOID context, FX_LPCBYTE src_buf, FX_DWORD src_size,
        CFX_BinaryBuf& dest_buf)
{
    return CryptStream(context, src_buf, src_size, dest_buf, FALSE);
}
FX_BOOL CPDF_StandardCryptoHandler::DecryptFinish(FX_LPVOID context, CFX_BinaryBuf& dest_buf)
{
    return CryptFinish(context, dest_buf, FALSE);
}
FX_DWORD CPDF_StandardCryptoHandler::EncryptGetSize(FX_DWORD objnum, FX_DWORD version, FX_LPCBYTE src_buf, FX_DWORD src_size)
{
    if (m_Cipher == FXCIPHER_AES) {
        return src_size + 32;
    }
    return src_size;
}
FX_BOOL CPDF_StandardCryptoHandler::EncryptContent(FX_DWORD objnum, FX_DWORD gennum, FX_LPCBYTE src_buf, FX_DWORD src_size,
        FX_LPBYTE dest_buf, FX_DWORD& dest_size)
{
    CryptBlock(TRUE, objnum, gennum, src_buf, src_size, dest_buf, dest_size);
    return TRUE;
}
void CPDF_CryptoHandler::Decrypt(FX_DWORD objnum, FX_DWORD gennum, CFX_ByteString& str)
{
    CFX_BinaryBuf dest_buf;
    FX_LPVOID context = DecryptStart(objnum, gennum);
    DecryptStream(context, (FX_LPCBYTE)str, str.GetLength(), dest_buf);
    DecryptFinish(context, dest_buf);
    str = dest_buf;
}
CPDF_StandardCryptoHandler::CPDF_StandardCryptoHandler()
{
    m_pAESContext = NULL;
    m_Cipher = FXCIPHER_NONE;
    m_KeyLen = 0;
}
CPDF_StandardCryptoHandler::~CPDF_StandardCryptoHandler()
{
    if (m_pAESContext) {
        FX_Free(m_pAESContext);
    }
}
