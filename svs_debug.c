/*
Copyright (c) 2018 Stanislav Brtna

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
#include "svs_debug.h"

void tokenPrint(svsVM *s) {
  int x = 0;
  puts("TokenList:");

  for (x = 0; x < getTokenMax(s); x++) {
    if (getTokenType(x, s) == 255) {
      puts("EOF token");
      break;
    }
    if (getTokenType(x, s) != 31) {
      printf("index:%u type:%u data:%u \n", x, getTokenType(x, s), (unsigned int)getTokenData(x, s).val_u);
    } else {
      printf("index:%u type:%u data:%f \n", x, getTokenType(x, s), getTokenData(x, s).val_f);
    }
  }
}

void varTablePrint(svsVM *s) {
  uint16_t x = 1;
  puts("varTable:");
  while (x <= s->varTableLen) {
    if(varGetType((varType)x, s) == 0) {
      printf("var id: %u name: %s type: num value: %i \n", x, s->varTable[x].name, (unsigned int)s->varTable[x].value.val_s);
    } else if (varGetType((varType)x, s) == 1) {
      printf("var id: %u name: %s type: str value: %s\n", x, s->varTable[x].name, s->stringField + s->varTable[x].value.val_str);
    } else if(varGetType((varType)x, s) == 3) {
      printf("var id: %u name: %s type: float value: %f \n", x, s->varTable[x].name, s->varTable[x].value.val_f);
    }
    x++;
  }
}

void strTablePrint(svsVM *s) {
  int x = 0;
  puts("stringTable:");
  printf("%u: ", x);
  while (x < s->stringFieldLen) {
    if (s->stringField[x] != 0) {
      printf("%c", s->stringField[x]);
    } else {
      printf("\n%u: ", x);
    }
    x++;
  }
}
