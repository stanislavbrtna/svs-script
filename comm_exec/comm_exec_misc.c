/*
Copyright (c) 2019 Stanislav Brtna

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

This is the main command execution logic.
*/

#include "svs_comm_exec.h"


uint16_t exprSkip(uint16_t index, svsVM *s) {
  uint16_t count = 0;

  while(1) {
    if (getTokenType(index, s) == SVS_TOKEN_LBR) {
      count += 1 ;
    }

    if (getTokenType(index, s) == SVS_TOKEN_RBR) {
      if (count > 0) {
      count -= 1;
      } else {
        errSoft((uint8_t *)"commSkip: Bracket sanity error.", s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)index, s);
        errSoftSetToken(index, s);
        return 0;
      }
      if (count == 0) {
        commExDMSG("commSkip expression skip end, (skipped)", index, s);
        break;
      }
    }
    index++;
  }

  return index; // ends on last bracket
}


// used for skipping code after if/else/while, if the equation say so
uint16_t commSkip(uint16_t index, svsVM *s) {
  uint16_t x = 0;
  uint16_t count = 0;

  x = index;
  commExDMSG("commSkip start", index, s);

  // skip if/else
  if (getTokenType(x, s) == SVS_TOKEN_IF) {
    x = exprSkip(x, s);
    x++;
    x = commSkip(x, s);
    if (getTokenType(x + 1, s) != SVS_TOKEN_ELSE) {
      return x;
    } else {
      x++;
      x = commSkip(x, s);
      return x;
    }
  }

  // skip for and while
  if (getTokenType(x, s) == SVS_TOKEN_FOR ||
      getTokenType(x, s) == SVS_TOKEN_WHILE
      ) {
    x = exprSkip(x, s); // skip expression
    x++;
    x = commSkip(x, s); // skip loop body
    return x;
  }

  while(1) { //začátek bloku
    if (getTokenType(x,s) == SVS_TOKEN_LCBR) {
      count += 1 ;
    }

    if (getTokenType(x ,s) == SVS_TOKEN_RCBR) {
      if (count > 0) {
        count -= 1;
      } else {
        errSoft((uint8_t *)"commSkip: Curly bracket sanity error. Maybe missing \";\"?" ,s);
        errSoftSetParam((uint8_t *)"TokenId", (varType)x ,s);
        errSoftSetToken(x, s);
        return 0;
      }
      if (count == 0) {
        commExDMSG("commSkip end (skipped BLOCK(S))", x, s);
        break;
      }
    }

    //řeší situaci kdy za příkazem není závorka, funguje ok, pokud je na konci řádku středník
    if ((getTokenType(x, s) == 9) && (count == 0)) {
      commExDMSG("commSkip end (skipped LINE)", x, s);
      break;
      //blok má ve zvyku obdržet token posledního vykonaného příkazu, inkrement provádí sám.
    }

    x++; //poskočí na další token
  }

  if ((x >= index) && (x <= s->tokenMax)) {
    return x;
  } else {
    errSoft((uint8_t *)"commSkip: Skip sanity error.", s);
    errSoftSetParam((uint8_t *)"after TokenId", (varType)index, s);
    errSoftSetToken(index, s);
    return 0;
  }
}