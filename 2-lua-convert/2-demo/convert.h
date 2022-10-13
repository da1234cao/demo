/**
 * Copyright (C) 2008  Huang Guan
 * Copyright (C) 2011  iBoxpay.com inc.
 *
 * $Id: 509d9187fcedee642b722b528884dc8378b93ede $
 *
 * Description: GBK UTF-8 iconv functions header file
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
 */

// 修改自：https://github.com/lytsing/gbk-utf8

#ifndef CONVERT_H
#define CONVERT_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <iconv.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <memory.h>

#ifdef __cplusplus
extern "C" {
#endif

// 禁止输入空字符串；
// 返回值小于等于0，为出错；大于零，为转换后的字符个数

int utf8_to_gbk(const char* src, char* dst, int len);

int gbk_to_utf8(const char* src, char* dst, int len);

#ifdef __cplusplus
}
#endif

#endif  // end of CONVERT_H

