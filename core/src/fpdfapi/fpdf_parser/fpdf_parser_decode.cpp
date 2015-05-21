// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_parser.h"
#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fxcodec/fx_codec.h"
#include <limits.h>
#define _STREAM_MAX_SIZE_		20 * 1024 * 1024
FX_DWORD _A85Decode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    dest_size = 0;
    dest_buf = NULL;
    if (src_size == 0) {
        return 0;
    }
    FX_DWORD zcount = 0;
    FX_DWORD pos = 0;
    while (pos < src_size) {
        FX_BYTE ch = src_buf[pos];
        if (ch < '!' && ch != '\n' && ch != '\r' && ch != ' ' && ch != '\t') {
            break;
        }
        if (ch == 'z') {
            zcount ++;
        } else if (ch > 'u') {
            break;
        }
        pos ++;
    }
    if (pos == 0) {
        return 0;
    }
    if (zcount > UINT_MAX / 4) {
        return (FX_DWORD) - 1;
    }
    if (zcount * 4 > UINT_MAX - (pos - zcount)) {
        return (FX_DWORD) - 1;
    }
    dest_buf = FX_Alloc(FX_BYTE, zcount * 4 + (pos - zcount));
    int state = 0;
    FX_UINT32 res = 0;
    pos = dest_size = 0;
    while (pos < src_size) {
        FX_BYTE ch = src_buf[pos++];
        if (ch == '\n' || ch == '\r' || ch == ' ' || ch == '\t') {
            continue;
        }
        if (ch == 'z') {
            FXSYS_memset32(dest_buf + dest_size, 0, 4);
            state = 0;
            res = 0;
            dest_size += 4;
        } else {
            if (ch < '!' || ch > 'u') {
                break;
            }
            res = res * 85 + ch - 33;
            state ++;
            if (state == 5) {
                for (int i = 0; i < 4; i ++) {
                    dest_buf[dest_size++] = (FX_BYTE)(res >> (3 - i) * 8);
                }
                state = 0;
                res = 0;
            }
        }
    }
    if (state) {
        int i;
        for (i = state; i < 5; i ++) {
            res = res * 85 + 84;
        }
        for (i = 0; i < state - 1; i ++) {
            dest_buf[dest_size++] = (FX_BYTE)(res >> (3 - i) * 8);
        }
    }
    if (pos < src_size && src_buf[pos] == '>') {
        pos ++;
    }
    return pos;
}
FX_DWORD _HexDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    FX_DWORD i;
    for (i = 0; i < src_size; i ++)
        if (src_buf[i] == '>') {
            break;
        }
    dest_buf = FX_Alloc( FX_BYTE, i / 2 + 1);
    dest_size = 0;
    FX_BOOL bFirstDigit = TRUE;
    for (i = 0; i < src_size; i ++) {
        FX_BYTE ch = src_buf[i];
        if (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r') {
            continue;
        }
        int digit;
        if (ch <= '9' && ch >= '0') {
            digit = ch - '0';
        } else if (ch <= 'f' && ch >= 'a') {
            digit = ch - 'a' + 10;
        } else if (ch <= 'F' && ch >= 'A') {
            digit = ch - 'A' + 10;
        } else if (ch == '>') {
            i ++;
            break;
        } else {
            continue;
        }
        if (bFirstDigit) {
            dest_buf[dest_size] = digit * 16;
        } else {
            dest_buf[dest_size ++] += digit;
        }
        bFirstDigit = !bFirstDigit;
    }
    if (!bFirstDigit) {
        dest_size ++;
    }
    return i;
}
FX_DWORD RunLengthDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    FX_DWORD i = 0;
    FX_DWORD old;
    dest_size = 0;
    while (i < src_size) {
        if (src_buf[i] < 128) {
            old = dest_size;
            dest_size += src_buf[i] + 1;
            if (dest_size < old) {
                return (FX_DWORD) - 1;
            }
            i += src_buf[i] + 2;
        } else if (src_buf[i] > 128) {
            old = dest_size;
            dest_size += 257 - src_buf[i];
            if (dest_size < old) {
                return (FX_DWORD) - 1;
            }
            i += 2;
        } else {
            break;
        }
    }
    if (dest_size >= _STREAM_MAX_SIZE_) {
        return -1;
    }
    dest_buf = FX_Alloc( FX_BYTE, dest_size);
    i = 0;
    int dest_count = 0;
    while (i < src_size) {
        if (src_buf[i] < 128) {
            FX_DWORD copy_len = src_buf[i] + 1;
            FX_DWORD buf_left = src_size - i - 1;
            if (buf_left < copy_len) {
                FX_DWORD delta = copy_len - buf_left;
                copy_len = buf_left;
                FXSYS_memset8(dest_buf + dest_count + copy_len, '\0', delta);
            }
            FXSYS_memcpy32(dest_buf + dest_count, src_buf + i + 1, copy_len);
            dest_count += src_buf[i] + 1;
            i += src_buf[i] + 2;
        } else if (src_buf[i] > 128) {
            int fill = 0;
            if (i < src_size - 1) {
                fill = src_buf[i + 1];
            }
            FXSYS_memset8(dest_buf + dest_count, fill, 257 - src_buf[i]);
            dest_count += 257 - src_buf[i];
            i += 2;
        } else {
            break;
        }
    }
    FX_DWORD ret = i + 1;
    if (ret > src_size) {
        ret = src_size;
    }
    return ret;
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFaxDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        const CPDF_Dictionary* pParams)
{
    int K = 0;
    FX_BOOL EndOfLine = FALSE;
    FX_BOOL ByteAlign = FALSE;
    FX_BOOL BlackIs1 = FALSE;
    int Columns = 1728;
    int Rows = 0;
    if (pParams) {
        K = pParams->GetInteger(FX_BSTRC("K"));
        EndOfLine = pParams->GetInteger(FX_BSTRC("EndOfLine"));
        ByteAlign = pParams->GetInteger(FX_BSTRC("EncodedByteAlign"));
        BlackIs1 = pParams->GetInteger(FX_BSTRC("BlackIs1"));
        Columns = pParams->GetInteger(FX_BSTRC("Columns"), 1728);
        Rows = pParams->GetInteger(FX_BSTRC("Rows"));
        if (Rows > USHRT_MAX) {
            Rows = 0;
        }
        if (Columns <= 0 || Rows < 0 || Columns > USHRT_MAX || Rows > USHRT_MAX) {
            return NULL;
        }
    }
    return CPDF_ModuleMgr::Get()->GetFaxModule()->CreateDecoder(src_buf, src_size, width, height,
            K, EndOfLine, ByteAlign, BlackIs1, Columns, Rows);
}
static FX_BOOL CheckFlateDecodeParams(int Colors, int BitsPerComponent, int Columns)
{
    if (Columns < 0) {
        return FALSE;
    }
    int check = Columns;
    if (Colors < 0 || (check > 0 && Colors > INT_MAX / check)) {
        return FALSE;
    }
    check *= Colors;
    if (BitsPerComponent < 0 ||
            (check > 0 && BitsPerComponent > INT_MAX / check)) {
        return FALSE;
    }
    check *= BitsPerComponent;
    if (check > INT_MAX - 7) {
        return FALSE;
    }
    return TRUE;
}
ICodec_ScanlineDecoder* FPDFAPI_CreateFlateDecoder(FX_LPCBYTE src_buf, FX_DWORD src_size, int width, int height,
        int nComps, int bpc, const CPDF_Dictionary* pParams)
{
    int predictor = 0;
    int Colors = 0, BitsPerComponent = 0, Columns = 0;
    if (pParams) {
        predictor = ((CPDF_Dictionary*)pParams)->GetInteger(FX_BSTRC("Predictor"));
        Colors = pParams->GetInteger(FX_BSTRC("Colors"), 1);
        BitsPerComponent = pParams->GetInteger(FX_BSTRC("BitsPerComponent"), 8);
        Columns = pParams->GetInteger(FX_BSTRC("Columns"), 1);
        if (!CheckFlateDecodeParams(Colors, BitsPerComponent, Columns)) {
            return NULL;
        }
    }
    return CPDF_ModuleMgr::Get()->GetFlateModule()->CreateDecoder(src_buf, src_size, width, height,
            nComps, bpc, predictor, Colors, BitsPerComponent, Columns);
}
FX_DWORD FPDFAPI_FlateOrLZWDecode(FX_BOOL bLZW, const FX_BYTE* src_buf, FX_DWORD src_size, CPDF_Dictionary* pParams,
                                  FX_DWORD estimated_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    int predictor = 0;
    FX_BOOL bEarlyChange = TRUE;
    int Colors = 0, BitsPerComponent = 0, Columns = 0;
    if (pParams) {
        predictor = ((CPDF_Dictionary*)pParams)->GetInteger(FX_BSTRC("Predictor"));
        bEarlyChange = ((CPDF_Dictionary*)pParams)->GetInteger(FX_BSTRC("EarlyChange"), 1);
        Colors = pParams->GetInteger(FX_BSTRC("Colors"), 1);
        BitsPerComponent = pParams->GetInteger(FX_BSTRC("BitsPerComponent"), 8);
        Columns = pParams->GetInteger(FX_BSTRC("Columns"), 1);
        if (!CheckFlateDecodeParams(Colors, BitsPerComponent, Columns)) {
            return (FX_DWORD) - 1;
        }
    }
    return CPDF_ModuleMgr::Get()->GetFlateModule()->FlateOrLZWDecode(bLZW, src_buf, src_size,
            bEarlyChange, predictor, Colors, BitsPerComponent, Columns, estimated_size,
            dest_buf, dest_size);
}
FX_BOOL PDF_DataDecode(FX_LPCBYTE src_buf, FX_DWORD src_size, const CPDF_Dictionary* pDict,
                       FX_LPBYTE& dest_buf, FX_DWORD& dest_size, CFX_ByteString& ImageEncoding,
                       CPDF_Dictionary*& pImageParms, FX_DWORD last_estimated_size, FX_BOOL bImageAcc)

{
    CPDF_Object* pDecoder = pDict ? pDict->GetElementValue(FX_BSTRC("Filter")) : NULL;
    if (pDecoder == NULL || (pDecoder->GetType() != PDFOBJ_ARRAY && pDecoder->GetType() != PDFOBJ_NAME)) {
        return FALSE;
    }
    CPDF_Object* pParams = pDict ? pDict->GetElementValue(FX_BSTRC("DecodeParms")) : NULL;
    CFX_ByteStringArray DecoderList;
    CFX_PtrArray ParamList;
    if (pDecoder->GetType() == PDFOBJ_ARRAY) {
        if (pParams && pParams->GetType() != PDFOBJ_ARRAY) {
            pParams = NULL;
        }
        CPDF_Array* pDecoders = (CPDF_Array*)pDecoder;
        for (FX_DWORD i = 0; i < pDecoders->GetCount(); i ++) {
            CFX_ByteStringC str = pDecoders->GetConstString(i);
            DecoderList.Add(str);
            if (pParams) {
                ParamList.Add(((CPDF_Array*)pParams)->GetDict(i));
            } else {
                ParamList.Add(NULL);
            }
        }
    } else {
        DecoderList.Add(pDecoder->GetConstString());
        ParamList.Add(pParams ? pParams->GetDict() : NULL);
    }
    FX_LPBYTE last_buf = (FX_LPBYTE)src_buf;
    FX_DWORD last_size = src_size;
    for (int i = 0; i < DecoderList.GetSize(); i ++) {
        int estimated_size = i == DecoderList.GetSize() - 1 ? last_estimated_size : 0;
        CFX_ByteString decoder = DecoderList[i];
        CPDF_Dictionary* pParam = (CPDF_Dictionary*)ParamList[i];
        FX_LPBYTE new_buf = NULL;
        FX_DWORD new_size = (FX_DWORD) - 1;
        int offset = -1;
        if (decoder == FX_BSTRC("FlateDecode") || decoder == FX_BSTRC("Fl")) {
            if (bImageAcc && i == DecoderList.GetSize() - 1) {
                ImageEncoding = FX_BSTRC("FlateDecode");
                dest_buf = (FX_LPBYTE)last_buf;
                dest_size = last_size;
                pImageParms = pParam;
                return TRUE;
            } else {
                offset = FPDFAPI_FlateOrLZWDecode(FALSE, last_buf, last_size, pParam, estimated_size, new_buf, new_size);
            }
        } else if (decoder == FX_BSTRC("LZWDecode") || decoder == FX_BSTRC("LZW")) {
            offset = FPDFAPI_FlateOrLZWDecode(TRUE, last_buf, last_size, pParam, estimated_size, new_buf, new_size);
        } else if (decoder == FX_BSTRC("ASCII85Decode") || decoder == FX_BSTRC("A85")) {
            offset = _A85Decode(last_buf, last_size, new_buf, new_size);
        } else if (decoder == FX_BSTRC("ASCIIHexDecode") || decoder == FX_BSTRC("AHx")) {
            offset = _HexDecode(last_buf, last_size, new_buf, new_size);
        } else if (decoder == FX_BSTRC("RunLengthDecode") || decoder == FX_BSTRC("RL")) {
            if (bImageAcc && i == DecoderList.GetSize() - 1) {
                ImageEncoding = FX_BSTRC("RunLengthDecode");
                dest_buf = (FX_LPBYTE)last_buf;
                dest_size = last_size;
                pImageParms = pParam;
                return TRUE;
            }
            offset = RunLengthDecode(last_buf, last_size, new_buf, new_size);
        } else {
            if (decoder == FX_BSTRC("DCT")) {
                decoder = "DCTDecode";
            } else if (decoder == FX_BSTRC("CCF")) {
                decoder = "CCITTFaxDecode";
            } else if (decoder == FX_BSTRC("Crypt")) {
                continue;
            }
            ImageEncoding = decoder;
            pImageParms = pParam;
            dest_buf = (FX_LPBYTE)last_buf;
            dest_size = last_size;
            return TRUE;
        }
        if (last_buf != src_buf) {
            FX_Free(last_buf);
        }
        if (offset == -1) {
            return FALSE;
        }
        last_buf = new_buf;
        last_size = new_size;
    }
    ImageEncoding = "";
    pImageParms = NULL;
    dest_buf = last_buf;
    dest_size = last_size;
    return TRUE;
}
extern const FX_WORD PDFDocEncoding[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009,
    0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012, 0x0013,
    0x0014, 0x0015, 0x0016, 0x0017, 0x02d8, 0x02c7, 0x02c6, 0x02d9, 0x02dd, 0x02db,
    0x02da, 0x02dc, 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031,
    0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b,
    0x003c, 0x003d, 0x003e, 0x003f, 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045,
    0x0046, 0x0047, 0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059,
    0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f, 0x0060, 0x0061, 0x0062, 0x0063,
    0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d,
    0x006e, 0x006f, 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x0000, 0x2022, 0x2020,
    0x2021, 0x2026, 0x2014, 0x2013, 0x0192, 0x2044, 0x2039, 0x203a, 0x2212, 0x2030,
    0x201e, 0x201c, 0x201d, 0x2018, 0x2019, 0x201a, 0x2122, 0xfb01, 0xfb02, 0x0141,
    0x0152, 0x0160, 0x0178, 0x017d, 0x0131, 0x0142, 0x0153, 0x0161, 0x017e, 0x0000,
    0x20ac, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7, 0x00a8, 0x00a9,
    0x00aa, 0x00ab, 0x00ac, 0x0000, 0x00ae, 0x00af, 0x00b0, 0x00b1, 0x00b2, 0x00b3,
    0x00b4, 0x00b5, 0x00b6, 0x00b7, 0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd,
    0x00be, 0x00bf, 0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf, 0x00d0, 0x00d1,
    0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7, 0x00d8, 0x00d9, 0x00da, 0x00db,
    0x00dc, 0x00dd, 0x00de, 0x00df, 0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5,
    0x00e6, 0x00e7, 0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7, 0x00f8, 0x00f9,
    0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};
CFX_WideString PDF_DecodeText(FX_LPCBYTE src_data, FX_DWORD src_len, CFX_CharMap* pCharMap)
{
    CFX_WideString result;
    if (src_len >= 2 && ((src_data[0] == 0xfe && src_data[1] == 0xff) || (src_data[0] == 0xff && src_data[1] == 0xfe))) {
        FX_BOOL bBE = src_data[0] == 0xfe;
        FX_DWORD max_chars = (src_len - 2) / 2;
        if (!max_chars) {
            return result;
        }
        if (src_data[0] == 0xff) {
            bBE = !src_data[2];
        }
        FX_LPWSTR dest_buf = result.GetBuffer(max_chars);
        FX_LPCBYTE uni_str = src_data + 2;
        int dest_pos = 0;
        for (FX_DWORD i = 0; i < max_chars * 2; i += 2) {
            FX_WORD unicode = bBE ? (uni_str[i] << 8 | uni_str[i + 1]) : (uni_str[i + 1] << 8 | uni_str[i]);
            if (unicode == 0x1b) {
                i += 2;
                while (i < max_chars * 2) {
                    FX_WORD unicode = bBE ? (uni_str[i] << 8 | uni_str[i + 1]) : (uni_str[i + 1] << 8 | uni_str[i]);
                    i += 2;
                    if (unicode == 0x1b) {
                        break;
                    }
                }
            } else {
                dest_buf[dest_pos++] = unicode;
            }
        }
        result.ReleaseBuffer(dest_pos);
    } else if (pCharMap == NULL) {
        FX_LPWSTR dest_buf = result.GetBuffer(src_len);
        for (FX_DWORD i = 0; i < src_len; i ++) {
            dest_buf[i] = PDFDocEncoding[src_data[i]];
        }
        result.ReleaseBuffer(src_len);
    } else {
        return (*pCharMap->m_GetWideString)(pCharMap, CFX_ByteString((FX_LPCSTR)src_data, src_len));
    }
    return result;
}
CFX_ByteString PDF_EncodeText(FX_LPCWSTR pString, int len, CFX_CharMap* pCharMap)
{
    if (len == -1) {
        len = FXSYS_wcslen(pString);
    }
    CFX_ByteString result;
    if (pCharMap == NULL) {
        FX_LPSTR dest_buf1 = result.GetBuffer(len);
        int i;
        for (i = 0; i < len; i ++) {
            int code;
            for (code = 0; code < 256; code ++)
                if (PDFDocEncoding[code] == pString[i]) {
                    break;
                }
            if (code == 256) {
                break;
            }
            dest_buf1[i] = code;
        }
        result.ReleaseBuffer(i);
        if (i == len) {
            return result;
        }
    }

    if(len > INT_MAX/2-1)
    {
        result.ReleaseBuffer(0);
        return result;
    }

    int encLen = len * 2 + 2;

    FX_LPBYTE dest_buf2 = (FX_LPBYTE)result.GetBuffer(encLen);
    dest_buf2[0] = 0xfe;
    dest_buf2[1] = 0xff;
    dest_buf2 += 2;
    for (int i = 0; i < len; i ++) {
        *dest_buf2++ = pString[i] >> 8;
        *dest_buf2++ = (FX_BYTE)pString[i];
    }
    result.ReleaseBuffer(encLen);
    return result;
}
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src, FX_BOOL bHex)
{
    CFX_ByteTextBuf result;
    int srclen = src.GetLength();
    if (bHex) {
        result.AppendChar('<');
        for (int i = 0; i < srclen; i ++) {
            result.AppendChar("0123456789ABCDEF"[src[i] / 16]);
            result.AppendChar("0123456789ABCDEF"[src[i] % 16]);
        }
        result.AppendChar('>');
        return result.GetByteString();
    }
    result.AppendChar('(');
    for (int i = 0; i < srclen; i ++) {
        FX_BYTE ch = src[i];
        if (ch == ')' || ch == '\\' || ch == '(') {
            result.AppendChar('\\');
        } else if (ch == 0x0a) {
            result << FX_BSTRC("\\n");
            continue;
        } else if (ch == 0x0d) {
            result << FX_BSTRC("\\r");
            continue;
        }
        result.AppendChar(ch);
    }
    result.AppendChar(')');
    return result.GetByteString();
}
void FlateEncode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    CCodec_ModuleMgr* pEncoders = CPDF_ModuleMgr::Get()->GetCodecModule();
    if (pEncoders) {
        pEncoders->GetFlateModule()->Encode(src_buf, src_size, dest_buf, dest_size);
    }
}
void FlateEncode(FX_LPCBYTE src_buf, FX_DWORD src_size, int predictor, int Colors, int BitsPerComponent, int Columns,
                 FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    CCodec_ModuleMgr* pEncoders = CPDF_ModuleMgr::Get()->GetCodecModule();
    if (pEncoders) {
        pEncoders->GetFlateModule()->Encode(src_buf, src_size, predictor, Colors, BitsPerComponent, Columns, dest_buf, dest_size);
    }
}
FX_DWORD FlateDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size)
{
    CCodec_ModuleMgr* pEncoders = CPDF_ModuleMgr::Get()->GetCodecModule();
    if (pEncoders) {
        return pEncoders->GetFlateModule()->FlateOrLZWDecode(FALSE, src_buf, src_size, FALSE, 0, 0, 0, 0, 0, dest_buf, dest_size);
    }
    return 0;
}
