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

This is the error handling.
When embedding the SVS, you can write your own error printing functions to suit your needs.
*/

#include "svs_errors.h"

void svs_hardErrHandler () __attribute__ ((weak, alias ("hardErrHandler_default")));

void errSoft(uint8_t *str, svsVM *s) {
  if (errCheck(s)) {
    errSoftPrint(s);
    printf("Another error: %s\n", str);
    errMsgS((uint8_t *)"errSoft:Double soft error occured!");
  } else {
    s->err = 1;
    s->errString = (char *) str;
  }
}

void errSoftSetToken(uint16_t token, svsVM *s) {
    s->errToken = token;
}

void errSoftSetParam(uint8_t *str, varType val, svsVM *s) {
  if (s->errDbgUsed < 5) {
    s->errDbgVar[s->errDbgUsed] = val.val_s;
    s->errDbgStr[s->errDbgUsed] = str;
    s->errDbgUsed++;
  } else {
    errMsgS((uint8_t *)"errSoftSetParam: Too many parameters!");
  }
}

uint8_t errCheck(svsVM *s) {
  return s->err;
}


void errHalt() {
  puts("errHalt:Error!");
  #ifdef PC
  getchar();
  #else
  while(1);
  #endif
}

void errSoftPrint(svsVM *s) {
  uint8_t x;
  if (errCheck(s)) {
    printf("\nError occured: (soft)\n%s\n", s->errString);
    for (x = 0; x < s->errDbgUsed; x++) {
      printf("%s : %d\n", s->errDbgStr[x], s->errDbgVar[x]);
    }

    if (s->errToken != 0) {
      tokenzer_print_token_line(s->errToken, s);
    }
  }
}


void errMsg(errStruct err) {
  printf("\nToken: %u HARD ERR: %s\n", err.tokenId, err.errString );
  svs_hardErrHandler();
}

void errMsgS(uint8_t *str) {
  printf("\nHARD ERR: %s\n", str);
  svs_hardErrHandler();
}

#ifdef PC
void hardErrHandler_default() {
  getchar();
}
#else
void hardErrHandler_default() {
  while(1);
}
#endif

