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

This is the garbage collector for strings.
*/

#include "svs_garbage_collector.h"

#ifndef CMDLINE
void pscg_garbage_walkaround(uint8_t *strId, uint32_t str_len, uint8_t *max);
uint8_t pscg_garbage_walkaround2(uint8_t *strId);
#endif

uint8_t gcDebug = 0;

void cgDMSG(char *text) {
  if (gcDebug == 1) {
    printf("gcDMSG: %s \n", text);
  }
}

void setGcDebug(uint8_t level) {
  gcDebug = level;
}

uint8_t gcGetValidString(uint16_t strId, svsVM *s) {
  uint16_t x = 0;

  if (strId <= s->stringConstMax) { // if string is a constant, it is valid
    return 1;
  }

  #ifndef CMDLINE
  if (pscg_garbage_walkaround2( (uint8_t*) ((uint32_t)strId + (uint32_t)(s->stringField)))) {
    return 1;
  }
  #endif

  for(x = 1; x <= s->varTableLen; x++) {
  // We need to access the var array without varGetType, because of how local variables work
    if (s->varTable[x].type == 1) {
      if (s->varTable[x].value.val_str == strId) {
        return 1;
      }
    }
  }

  //array
  for(x = 1; x <= s->varArrayLen; x++) {
    if (s->varArrayType[x] == 1) {
      if (s->varArray[x].val_str == strId) {
        return 1;
      }
    }
  }

  // not valid, returning zero
  return 0;
}

uint32_t gcGetLen(uint32_t strId, svsVM *s) {
  uint32_t x = 0;

  x = strId;
  while (s->stringField[x] != 0) {
    x++;
  }

  return x - strId + 1;
}

uint8_t gcRemoveStrIdLen(uint16_t strId, uint32_t str_len, svsVM *s) {
  uint32_t x = 0;

  for(x = strId + str_len; x < s->stringFieldLen; x++) {
    s->stringField[x - str_len] = s->stringField[x];
  }

  s->stringFieldLen -= str_len;

  for(x = 1; x <= s->varTableLen; x++) {
    if (s->varTable[x].type == 1) {
      if (s->varTable[x].value.val_str > strId) {
        // modify reference
        s->varTable[x].value.val_str = ((uint16_t)(varGetVal((varType)x, s).val_str - str_len));
      }
    }
  }

  //array
  for(x = 1; x <= s->varArrayLen; x++) {
    if (s->varArrayType[x] == 1) {
      if (s->varArray[x].val_str > strId) {
        s->varArray[x].val_str -= str_len;
      }
    }
  }

  #ifndef CMDLINE
    pscg_garbage_walkaround(
                       (void *)((uint32_t)strId + (uint32_t)(s->stringField)),
                       str_len,
                       (void *)((uint32_t)s->stringField + (uint32_t)s->stringFieldLen)
    );
  #endif

  return 0;
}


uint8_t gcRemoveString(uint16_t strId, svsVM *s) {
  uint16_t x = 0;
  uint16_t str_len = 0;

  str_len = gcGetLen((uint32_t)strId, s);

  gcRemoveStrIdLen(strId, str_len, s);
}

void garbageCollect(uint16_t count, svsVM *s) {
  uint16_t x = 0;
  uint8_t valid = 0;
  uint8_t all_valid = 1;
  uint16_t gc_start;

  if (s->stringFieldLen + 1 < GC_THRESHOLD) {
    if (count == 0) {
      return;
    }
  }

  if (s->profilerEnabled) {
    printf("Profiler: collecting garbage! ");
  }
  gc_start = s->stringFieldLen;

  if (s->stringFieldMax < s->stringFieldLen) {
    errMsgS((uint8_t *)"garbageCollect: String field invalid! (s->stringFieldMax<stringFieldLen)");
  }

  if (0 == s->stringFieldLen) {
    cgDMSG("Nothing in string field.");
    return;
  }

  if (s->stringConstMax > s->gcSafePoint) {
    x = s->stringConstMax;
  } else {
    x = s->gcSafePoint;
  }

  uint16_t remove_start = 0;
  uint8_t chain_remove = 0;

  for(; x < s->stringFieldLen - 1; x++) {
    if (0 == s->stringField[x]) { // on the end of previous string
      valid = gcGetValidString(x + 1, s); // check validity of the next one    
      if (0 == valid && chain_remove == 0) {
        all_valid = 0;
        cgDMSG("Non-valid string removed.");
        //printf("stringRM:(%u) %s\n", x + 1, s->stringField+x+1 );
        remove_start = x + 1;
        chain_remove = 1;
      }

      if (valid == 1 && chain_remove == 1) {
        gcRemoveStrIdLen(remove_start, x - remove_start + 1, s);
        chain_remove = 0;

        if (count != 0) {
          if (s->stringFieldLen < (gc_start - count)) {
            break;
          }
        }
      }
    }
  }

  if (s->profilerEnabled) {
    printf(" Collecting Done! %u/%u occupied %u freed.\n", s->stringFieldLen, s->stringFieldMax, gc_start-s->stringFieldLen);
  }

  if (all_valid == 1) {
    cgDMSG("All strings valid.");
    return;
  }

}


void gcCheckField(svsVM *s) {
  uint16_t x = 0;
  uint8_t valid = 0;
  uint8_t all_valid = 1;
  uint16_t gc_start;
  uint8_t safepoint_shown = 0;

  gc_start = s->stringFieldLen;
  printf("%s: starting\n", __FUNCTION__);
  if (0 == s->stringFieldLen) {
    printf("%s:Nothing in string field.\n", __FUNCTION__);
    return;
  }

  x = s->stringConstMax;

  for(; x < s->stringFieldLen - 1; x++) {
    if (0 == s->stringField[x]) { // on the end of previous string
      valid = gcGetValidString(x + 1, s); // check validity of the next one
      if (valid == 0) {
        all_valid = 0;
      }
      printf("id: %u, valid: %u\n", x + 1, valid);
      if(x >= s->gcSafePoint && safepoint_shown == 0) {
        printf("=== GC safepoint! ==\n");
        printf("Everything below can be safely removed.\n");
        safepoint_shown = 1;
      }
    }
  }

  if (all_valid == 1) {
    printf("All strings valid.\n");
    return;
  }
}
