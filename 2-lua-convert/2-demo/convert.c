/**
 *  Copyright (C) 2008  Huang Guan
 *  Copyright (C) 2011  iBoxpay.com inc.
 *
 *  $Id: 691029ec2ac041372193855b2eb56db17bdac132 $
 *
 *  Description: This file mainly includes the functions about utf8
 *
 *  History:
 *  2008-7-10 13:31:57 Created.
 *  2011-12-28 Format the code style, and add comments by Lytsing
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "convert.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __WIN32__
int utf8_to_gbk(const char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
    if (i <= 0) {
        return i;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_UTF8, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_ACP, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len > i) {
        ret = WideCharToMultiByte(CP_ACP, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }
    
    free(strA);
    return ret;
}

int gbk_to_utf8(const char* src, char* dst, int len)
{
    int ret = 0;
    WCHAR* strA;
    int i= MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
    if (i <= 0) {
        return i;
    }
    strA = (WCHAR*)malloc(i * 2);
    MultiByteToWideChar(CP_ACP, 0, src, -1, strA, i);
    i = WideCharToMultiByte(CP_UTF8, 0, strA, -1, NULL, 0, NULL, NULL);
    if (len >= i) {
        ret = WideCharToMultiByte(CP_UTF8, 0, strA, -1, dst, i, NULL, NULL);
        dst[i] = 0;
    }
    free(strA);
    return ret;
}
#else   //Linux
// starkwong: In iconv implementations, inlen and outlen should be type of size_t not uint, which is different in length on Mac
int utf8_to_gbk(const char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char* inbuf = (char *)malloc(len);
    char* inbuf_hold = inbuf;   // iconv may change the address of inbuf
                                // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char* outbuf = dst;
    iconv_t cd;
    cd = iconv_open("GBK", "UTF-8");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
            free(inbuf_hold);
            return 0;
        }
        iconv_close(cd);
    }
    ret = outlen;
    free(inbuf_hold);
    return ret;
}

void gbk_to_utf8(const char* src, char* dst, int len)
{
    int ret = 0;
    size_t inlen = strlen(src) + 1;
    size_t outlen = len;

    // duanqn: The iconv function in Linux requires non-const char *
    // So we need to copy the source string
    char* inbuf = (char *)malloc(len);
    char* inbuf_hold = inbuf;   // iconv may change the address of inbuf
                                // so we use another pointer to keep the address
    memcpy(inbuf, src, len);

    char* outbuf2 = NULL;
    char* outbuf = dst;
    iconv_t cd;

    // starkwong: if src==dst, the string will become invalid during conversion since UTF-8 is 3 chars in Chinese but GBK is mostly 2 chars
    if (src == dst) {
        outbuf2 = (char*)malloc(len);
        memset(outbuf2, 0, len);
        outbuf = outbuf2;
    }

    cd = iconv_open("UTF-8", "GBK");
    if (cd != (iconv_t)-1) {
        if (iconv(cd, &inbuf, &inlen, &outbuf, &outlen) != 0) {
            free(inbuf_hold);
            if(src == dst) {
                free(outbuf2);
            }
            return 0;
        }else if (outbuf2 != NULL) { // 函数执行成功，且dst与src不相同，需要拷贝
            strcpy(dst, outbuf2);
            free(outbuf2);
        }
        iconv_close(cd);
    }
    ret = outlen;
    free(inbuf_hold);  
    return 0;
}
#endif

#ifdef __cplusplus
}
#endif
