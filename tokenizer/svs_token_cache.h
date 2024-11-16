/*
Copyright (c) 2016 Stanislav Brtna

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef SVS_TOKEN_CACHE_H
#define SVS_TOKEN_CACHE_H

#include "../svs_basics.h"

uint8_t getTokenType(uint16_t tokenId, svsVM *s);
uint8_t setTokenType(uint16_t tokenId, uint8_t val, svsVM *s);
VARTYPE getTokenData(uint16_t tokenId, svsVM *s);
uint8_t setTokenData(uint16_t tokenId, VARTYPE val, svsVM *s);
uint16_t getTokenMax(svsVM *s);
uint8_t closeTokenCache(svsVM *s);
uint8_t cacheReload(uint16_t tokenId,  svsVM *s);
uint8_t cacheRead(svsVM *s);
void setCacheDebug(uint8_t level);

void SVScloseCache(svsVM *s);
void SVSopenCache(svsVM *s);

uint32_t svsGetCacheReloads(uint8_t reset);

void svsLoadCacheSegmentOffsets();

#endif
