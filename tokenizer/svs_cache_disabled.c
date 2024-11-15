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

#include "svs_token_cache.h"


#if TOKEN_CACHE_DISABLED == 1

uint8_t getTokenType(uint16_t tokenId, svsVM *s) {
  if(tokenId < TOKEN_LENGTH) {
    return s->tokenCache[tokenId].Type;
  }else{
    errMsgS("getTokenType: Token field index invalid!");
  }
  return 0;
}

uint8_t setTokenType(uint16_t tokenId, uint8_t val, svsVM *s) {
  if(tokenId<TOKEN_LENGTH) {
    s->tokenCache[tokenId].Type = val;
  } else {
    errMsgS("setTokenType: Token field index invalid!");
  }
  return 0;
}

VARTYPE getTokenData(uint16_t tokenId, svsVM *s) {
  if(tokenId<TOKEN_LENGTH) {
    return s->tokenCache[tokenId].Data;
  } else {
    errMsgS("getTokenData: Token field index invalid!");
  }
  return (varType) 0;
}

uint8_t setTokenData(uint16_t tokenId, VARTYPE val, svsVM *s){
  if(tokenId<TOKEN_LENGTH){
    s->tokenCache[tokenId].Data=val;
  }else{
    errMsgS("setTokenData: Token field index invalid!");
  }
  return 0;
}

uint16_t getTokenMax(svsVM *s){
  return TOKEN_LENGTH;
}

uint8_t closeTokenCache(svsVM *s){
  return 0;
}

void SVScloseCache(svsVM *s) {
  return;
}

void SVSopenCache(svsVM *s) {
  return;
}

#endif