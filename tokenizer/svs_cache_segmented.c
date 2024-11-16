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

extern uint8_t cacheDebug;

#ifdef SVS_TOKEN_CACHE_SEGMENTED

uint32_t cacheReloads;

uint32_t svsGetCacheReloads(uint8_t reset) {
  uint32_t res = cacheReloads;
  if (reset) {
    cacheReloads = 0;
  }
  return res;
}

void adjustHitRate(uint8_t hit, svsVM *s);

// returns segment + 1
uint8_t tokenInCache(uint16_t tokenId, svsVM *s) {
  for(uint8_t i = 0; i < TOKEN_SEGMENTS; i++) {
    if(tokenId >= s->tokenSegmentStart[i]
      && tokenId < s->tokenSegmentStart[i] + TOKEN_SEGMENT_SIZE
      && s->tokenSegmentValid[i]
    ) {
      return i + 1;
    }
  }
  return 0;
}


uint8_t getTokenType(uint16_t tokenId, svsVM *s){
  uint8_t segment = tokenInCache(tokenId, s);
  if (!segment) {
    segment = cacheReload(tokenId, s);
  }

  return s->tokenCache[
    TOKEN_SEGMENT_SIZE * (segment - 1)          // segment position
    + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
  ].Type;
}


varType getTokenData(uint16_t tokenId, svsVM *s){
  uint8_t segment = tokenInCache(tokenId, s);
  if (!segment) {
    segment = cacheReload(tokenId, s);
  }

  return s->tokenCache[
    TOKEN_SEGMENT_SIZE * (segment - 1)          // segment position
    + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
  ].Data;
}


void fillCacheSegment(uint8_t segment, uint16_t load_start, svsVM *s) {
  uint32_t x, ret;

#ifdef PC
  fseek(s->vmCache, sizeof(tokenCacheStruct) * (load_start), SEEK_SET);
  fread(&(s->tokenCache[TOKEN_SEGMENT_SIZE*segment]), sizeof(tokenCacheStruct), TOKEN_SEGMENT_SIZE, s->vmCache);
#else
  f_lseek(&(s->vmCache), sizeof(tokenCacheStruct) * load_start);
  f_read(&(s->vmCache), &(s->tokenCache[TOKEN_SEGMENT_SIZE*segment]), sizeof(tokenCacheStruct)*TOKEN_SEGMENT_SIZE, (UINT*)&ret );
#endif
}


// advanced cache reloader
uint8_t cacheReload(uint16_t tokenId, svsVM *s){
  uint16_t x;
  uint16_t chacheStartPrac;

  if (cacheDebug == 1) {
    printf("cacheReload dbg: BEGIN: index: %u cache start: %u -> reloading cache\n(chache size: %u )\n", tokenId, s->cacheStart, TOKEN_LENGTH );
  }

  // Error handling
#ifdef PC
  if ((s->vmCache)==0){
    errMsgS("cacheReload: Error: File not valid!");
  }
#else
  if ((s->cacheFr)!=FR_OK){
    errMsgS((uint8_t *)"cacheReload: Error: File not valid!");
  }
#endif

  // najít segment, kam nahrajem content
  uint8_t segment = 0;
  uint8_t found = 0;

  // non-valid segment
  for(uint8_t i = 0; i < TOKEN_SEGMENTS; i++) {
    if (s->tokenSegmentValid[i] == 0) {
      segment = i;
      found = 1;
      break;
    }
  }

  // something more funky
  if (!found) {
    
    // DBG
    if (cacheDebug == 1) {
      for(uint8_t i = 0; i < TOKEN_SEGMENTS; i++) {
        printf("CACHE: %u, start: %u, hits: %u\n", i, s->tokenSegmentStart[i], s->tokenSegmentHits[i]);  
      }
    }
    // TODO: implement more robust cache replacement alg...

    // get the least used segment
    found = 1;
    // random is the way
    segment = SVS_RND_FUNCTION % TOKEN_SEGMENTS;
  }

  if (!found) {
    errMsgS("Cache alg failed!");
  }

  tokenId = (tokenId/TOKEN_SEGMENT_SIZE) * TOKEN_SEGMENT_SIZE;
  if (cacheDebug == 1) {
    printf("Reloading! segment:%u tokId: %u\n", segment, tokenId);
  }
  fillCacheSegment(segment, tokenId, s);

  // update metadata
  s->tokenSegmentStart[segment] = tokenId;
  s->tokenSegmentValid[segment] = 1;
  s->tokenSegmentHits[segment]  = 1;

  cacheReloads++;

  return segment + 1;
}


void adjustHitRate(uint8_t hit, svsVM *s) {
  uint16_t max = 0;
  for(uint8_t i = 0; i < TOKEN_SEGMENTS; i++) {
    if (s->tokenSegmentHits[max] < s->tokenSegmentHits[i]) {
      max = i;
    } 
  }
  if (max != hit) {
    s->tokenSegmentHits[hit]++;
  }
}


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
    return 1;
  }
#endif
    s->vmCacheUsed = 1;
  }
  return 0;
}


// set token value, with write pass-through
void writeToken(uint16_t tokenId, tokenCacheStruct val, uint8_t ifType, svsVM *s) {
#ifndef PC
  UINT ret;
#endif
  tokenCacheStruct prac;

#ifdef PC
  fseek(s->vmCache,sizeof(tokenCacheStruct)*tokenId,SEEK_SET);
  fread(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek( &(s->vmCache),sizeof(tokenCacheStruct)*tokenId);
  f_read( &(s->vmCache),&prac,sizeof(tokenCacheStruct), (UINT*) &ret );
#endif

  if(ifType) {
    prac.Type = val.Type;
  } else {
    prac.Data = val.Data;
  }

#ifdef PC
  fseek(s->vmCache, sizeof(tokenCacheStruct) * tokenId, SEEK_SET);
  fwrite(&prac, sizeof(tokenCacheStruct), 1, s->vmCache);
#else
  f_lseek(&(s->vmCache), sizeof(tokenCacheStruct) * tokenId);
  f_write(&(s->vmCache), &prac, sizeof(tokenCacheStruct), (UINT*) &ret);
#endif
}


uint8_t setTokenType(uint16_t tokenId, uint8_t val,  svsVM *s){
  uint8_t segment = tokenInCache(tokenId, s);
  
  openTokenCache(s);

  if (!segment) {
    segment = cacheReload(tokenId, s);
  }
  
  if (segment) {
    s->tokenCache[
      TOKEN_SEGMENT_SIZE * (segment - 1)          // segment position
      + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
    ].Type = val;
  }

  writeToken(
    tokenId,
    s->tokenCache[
      TOKEN_SEGMENT_SIZE * (segment - 1)          // segment position
      + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
    ],
    1,
    s
  );
  return 0;
}

uint8_t setTokenData(uint16_t tokenId, varType val, svsVM *s){
  uint8_t segment = tokenInCache(tokenId, s);
  
  openTokenCache(s);

  if (!segment) {
    segment = cacheReload(tokenId, s);
  }
  
  if (segment) {
    s->tokenCache[
      TOKEN_SEGMENT_SIZE * (segment - 1)              // segment position
      + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
    ].Data = val;
  }

  writeToken(
    tokenId,
    s->tokenCache[
      TOKEN_SEGMENT_SIZE * (segment - 1)          // segment position
      + (tokenId - s->tokenSegmentStart[segment - 1]) // position in segment
    ],
    0,
    s
  );
  return 0;
}


uint16_t getTokenMax(svsVM *s) {
	(void) (s);
  return 65000;
}

#endif
