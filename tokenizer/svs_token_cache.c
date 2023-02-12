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

uint8_t cacheDebug = 0;


//cgDMSG("text");
void cacheDMSG(char *text){
  if (cacheDebug==1){
    printf("cacheDMSG: %s \n", text);
  }
}

void setCacheDebug(uint8_t level){
  cacheDebug = level;
}


#if TOKEN_CACHE_DISABLED == 1
uint8_t getTokenType(uint16_t tokenId, svsVM *s){
  if(tokenId<TOKEN_LENGTH){
    return s->tokenCache[tokenId].Type;
  }else{
    errMsgS("getTokenType: Token field index invalid!");
  }
}

uint8_t setTokenType(uint16_t tokenId, uint8_t val, svsVM *s){
  if(tokenId<TOKEN_LENGTH){
    s->tokenCache[tokenId].Type=val;
  }else{
    errMsgS("setTokenType: Token field index invalid!");
  }
}

VARTYPE getTokenData(uint16_t tokenId, svsVM *s){
  if(tokenId<TOKEN_LENGTH){
    return s->tokenCache[tokenId].Data;
  }else{
    errMsgS("getTokenData: Token field index invalid!");
  }

}

uint8_t setTokenData(uint16_t tokenId, VARTYPE val, svsVM *s){
  if(tokenId<TOKEN_LENGTH){
    s->tokenCache[tokenId].Data=val;
  }else{
    errMsgS("setTokenData: Token field index invalid!");
  }
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

#else


uint8_t tokenInCache(uint16_t tokenId, svsVM *s){
  if((tokenId < (TOKEN_LENGTH + s->cacheStart)) && (tokenId >= (s->cacheStart))) {
    return 1;
  }else{
    return 0;
  }
}

uint8_t getTokenType(uint16_t tokenId, svsVM *s){
  if(tokenInCache(tokenId,s)){
    return s->tokenCache[tokenId - s->cacheStart ].Type;
  }else{
    cacheReload(tokenId,s);
    return s->tokenCache[tokenId - s->cacheStart ].Type;
  }
}


varType getTokenData(uint16_t tokenId, svsVM *s){
  if(tokenInCache(tokenId,s)){
    return s->tokenCache[tokenId - s->cacheStart ].Data;
  }else{
    cacheReload(tokenId,s);
    return s->tokenCache[tokenId - s->cacheStart ].Data;
  }
}



#ifdef PC

void SVScloseCache(svsVM *s) {
  fclose(s->vmCache);
}

void SVSopenCache(svsVM *s) {
  s->vmCache = fopen(s->vmName, "r+");
}

#else

void SVScloseCache(svsVM *s) {
  f_close(&(s->vmCache));
}

void SVSopenCache(svsVM *s) {
  f_open(&(s->vmCache), (char *)s->vmName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
}

#endif

#ifdef CACHE_SIMPLE
//simple cache reloader
uint8_t cacheReload(uint16_t tokenId,  svsVM *s) {
  uint32_t x, ret;
  tokenCacheStruct prac;
  prac.Data.val_s=666;
  prac.Type=6;
  if (cacheDebug==1){
    printf("cacheReload dbg: BEGIN: index: %u cache start: %u -> reloading cache\n", tokenId, s->cacheStart);
  }
  //
#ifdef PC
  if ((s->vmCache)==0){
    errMsgS("cacheReload: Error: File not valid!");
  }
#else
  if ((s->cacheFr)!=FR_OK){
    errMsgS("cacheReload: Error: File not valid!");
  }
#endif

  //fseek(s->vmCache,sizeof(tokenCacheStruct)*TOKEN_LENGTH*(tokenId%TOKEN_LENGTH),SEEK_SET);

  s->cacheStart=TOKEN_LENGTH*(tokenId/TOKEN_LENGTH);

  for(x=0;x<TOKEN_LENGTH;x++){
    #ifdef PC
    fseek(s->vmCache,sizeof(tokenCacheStruct)*(TOKEN_LENGTH*(tokenId/TOKEN_LENGTH)+x),SEEK_SET);
    ret= fread(&(s->tokenCache[x]), sizeof(tokenCacheStruct), 1, s->vmCache);
    #else
    f_lseek(&(s->vmCache),sizeof(tokenCacheStruct)*(TOKEN_LENGTH*(tokenId/TOKEN_LENGTH)+x));
    f_read(&(s->vmCache),&(s->tokenCache[x]),sizeof(tokenCacheStruct), (UINT*) &ret );
    #endif
  }

  //printf("cacheReload dbg: END: index: %u cache start: %u\n", tokenId,s->cacheStart );

  return 1;

}

#else

void fillCache(uint16_t cache_pos, uint16_t load_start, uint16_t load_end, svsVM *s);

// advanced cache reloader
uint8_t cacheReload(uint16_t tokenId,  svsVM *s){
  uint16_t x;
  uint16_t chacheStartPrac;

  if (cacheDebug==1){
    printf("cacheReload dbg: BEGIN: index: %u cache start: %u -> reloading cache\n(chache size: %u )\n", tokenId,s->cacheStart, TOKEN_LENGTH );
  }
  //
#ifdef PC
  if ((s->vmCache)==0){
    errMsgS("cacheReload: Error: File not valid!");
  }
#else
  if ((s->cacheFr)!=FR_OK){
    errMsgS((uint8_t *)"cacheReload: Error: File not valid!");
  }
#endif
  //TOKEN_CACHE_STEP

  chacheStartPrac = s->cacheStart;

  if (tokenId < s->cacheStart) {
    s->cacheStart = tokenId;

    // pokudjde něco znovupoužít
    // bacha, token lenght je sice x, ale indexuje se od 0 do x-1
    if (chacheStartPrac < (tokenId + TOKEN_LENGTH - 1)) {
      //printf("  debug: REV from %u to %u\n", TOKEN_LENGTH, TOKEN_LENGTH - 1 - (chacheStartPrac - tokenId));
      for(x = 0; x < ((tokenId + TOKEN_LENGTH) - chacheStartPrac); x++) {
        s->tokenCache[TOKEN_LENGTH-1 - x] = s->tokenCache[TOKEN_LENGTH-1 - (chacheStartPrac - tokenId )-x];
        //printf( "   %u = %u\n", TOKEN_LENGTH-1 - x, TOKEN_LENGTH - (chacheStartPrac - tokenId )-x);
      }
      //printf ("repeats %u\n", x-1);
    } else {
      //printf("debug: quick REV\n");
      chacheStartPrac = tokenId + TOKEN_LENGTH;
    }
    fillCache(0, tokenId, chacheStartPrac, s);

  } else {

    //pokud se něco z cache dá znovupoužít
    //printf("  debug: FWD: %u < %u\n",((tokenId + TOKEN_CACHE_STEP) - TOKEN_LENGTH), (chacheStartPrac + TOKEN_LENGTH));
    if (((tokenId + TOKEN_CACHE_STEP) - TOKEN_LENGTH) < (chacheStartPrac + TOKEN_LENGTH)) {
      //printf("   debug: FWD from %u to %u\n", TOKEN_LENGTH - ((chacheStartPrac + TOKEN_LENGTH) - (tokenId + TOKEN_CACHE_STEP - TOKEN_LENGTH) ), 0);
      s->cacheStart = (tokenId + TOKEN_CACHE_STEP) - TOKEN_LENGTH;
      for(x = 0; x < ( (chacheStartPrac + TOKEN_LENGTH) - (tokenId + TOKEN_CACHE_STEP - TOKEN_LENGTH)) ; x++) {
        s->tokenCache[x] = s->tokenCache[x + TOKEN_LENGTH - ((chacheStartPrac + TOKEN_LENGTH) - (tokenId + TOKEN_CACHE_STEP - TOKEN_LENGTH))];
        //printf( "   %u = %u\n", x, x + TOKEN_LENGTH - ((chacheStartPrac + TOKEN_LENGTH) - (tokenId + TOKEN_CACHE_STEP - TOKEN_LENGTH)));
      }
      //printf ("   repeatz %u, cache start:%u \n", x-1, s->cacheStart);
      fillCache(x, chacheStartPrac + TOKEN_LENGTH, tokenId + TOKEN_CACHE_STEP, s);
    } else {
      // jump is out of reach for chache, reloading all
      //printf("   debug: quick FWD\n");
      s->cacheStart = tokenId;
      fillCache(0, tokenId, tokenId + TOKEN_LENGTH, s);
    }
  }
  //printf("cacheReload dbg: END: index: %u cache start: %u\n", tokenId,s->cacheStart );

  return 1;
}

void fillCache(uint16_t cache_pos, uint16_t load_start, uint16_t load_end, svsVM *s) {
  uint32_t x, ret;

  //printf("debug: fill params: chache_pos:%u start:%u stop:%u\n", cache_pos, load_start, load_end);
  for(x = 0; (load_start + x) < load_end; x++) {
    #ifdef PC
    fseek(s->vmCache, sizeof(tokenCacheStruct) * (load_start + x), SEEK_SET);
    fread(&(s->tokenCache[cache_pos+x]), sizeof(tokenCacheStruct), 1, s->vmCache);
    #else
    f_lseek(&(s->vmCache),sizeof(tokenCacheStruct) * (load_start + x));
    f_read(&(s->vmCache), &(s->tokenCache[cache_pos+x]), sizeof(tokenCacheStruct), (UINT*)&ret );
    #endif
  }
}
#endif


uint8_t openTokenCache(svsVM *s) {
  if (s->vmCacheUsed == 0) {
#ifdef PC
    s->vmCache = fopen(s->vmName,"w+");

    if (!(s->vmCache)) {
      errMsgS("setTokenData: Error while opening cache file!");
    }
#else
    s->cacheFr = f_open(&(s->vmCache), (char *)s->vmName, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);

    if (s->cacheFr != FR_OK){
    errMsgS((uint8_t *)"setTokenData: Error while opening cache file!");
  }
#endif
    s->vmCacheUsed = 1;
  }
}

uint8_t setTokenType(uint16_t tokenId, uint8_t val,  svsVM *s){
#ifndef PC
  UINT ret;
#endif
  tokenCacheStruct prac;

  openTokenCache(s);

  if (tokenInCache(tokenId,s)) {
    s->tokenCache[tokenId - s->cacheStart].Type = val;
  }

#ifdef PC
  fseek(s->vmCache,sizeof(tokenCacheStruct)*tokenId,SEEK_SET);
  fread(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek(&(s->vmCache),sizeof(tokenCacheStruct)*tokenId);
  f_read(&(s->vmCache),&prac,sizeof(tokenCacheStruct), (UINT*) &ret );
#endif
  prac.Type=val;
#ifdef PC
  fseek(s->vmCache,sizeof(tokenCacheStruct)*tokenId,SEEK_SET);
  fwrite(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek( &(s->vmCache),sizeof(tokenCacheStruct)*tokenId);
  f_write( &(s->vmCache),&prac,sizeof(tokenCacheStruct), (UINT*) &ret );
#endif

  return 0;
}

uint8_t setTokenData(uint16_t tokenId, varType val, svsVM *s){
#ifndef PC
  UINT ret;
  //FRESULT fr;
#endif
  tokenCacheStruct prac;

  openTokenCache(s);

  if (tokenInCache(tokenId,s)){
    s->tokenCache[tokenId - s->cacheStart].Data=val;
  }
#ifdef PC
  fseek(s->vmCache,sizeof(tokenCacheStruct)*tokenId,SEEK_SET);
  fread(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek( &(s->vmCache),sizeof(tokenCacheStruct)*tokenId);
  f_read( &(s->vmCache),&prac,sizeof(tokenCacheStruct), (UINT*) &ret );
#endif
  prac.Data=val;
#ifdef PC
  fseek(s->vmCache,sizeof(tokenCacheStruct)*tokenId,SEEK_SET);
  fwrite(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek( &(s->vmCache),sizeof(tokenCacheStruct)*tokenId);
  f_write( &(s->vmCache),&prac,sizeof(tokenCacheStruct), (UINT*) &ret );
#endif
  return 0;
}

uint16_t getTokenMax(svsVM *s) {
	(void) (s);
  return 65000;
}

#endif
