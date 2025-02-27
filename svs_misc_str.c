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

#include "svs_misc_str.h"

uint16_t streamID;

uint16_t strNew(uint8_t *index, svsVM *s) {
  uint16_t x = 0;
  uint16_t retval;
  retval = s->stringFieldLen;

  while(index[x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strNew: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = index[x];
    x++;
    s->stringFieldLen++;
  }

  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++;
  return retval;
}

// returns c pointer to the new string
// Given pointer will not be valid after garbage collection
uint8_t *strNewP(uint8_t *index, svsVM *s) {
  uint16_t x = 0;
  uint8_t *retval;
  retval = &(s->stringField[s->stringFieldLen]);

  while(index[x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strNew: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = index[x];
    x++;
    s->stringFieldLen++;
  }

  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++;
  return retval;
}

// allocates new string with given lenght, returns c pointer to the new string
// sets the str_id pointer to the svs string index of the new string
// Given pointer will not be valid after garbage collection
uint8_t *strNewPLen(uint32_t len, uint16_t * str_id, svsVM *s) {
  uint16_t x = 0;
  uint8_t *retval;
  retval = &(s->stringField[s->stringFieldLen]);

  if (s->stringFieldLen + len >= (s->stringFieldMax - 1)) {
    errSoft((uint8_t *)"strNew: String field full!", s);
    return 0;
  }

  *str_id = s->stringFieldLen;

  s->stringFieldLen += len;
  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++;
  return retval;
}


uint16_t strAdd(uint16_t index1, uint16_t index2, svsVM *s) {
  uint16_t x = 0;
  uint16_t retval;
  retval = s->stringFieldLen;

  while (s->stringField[index1+x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strAdd: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = s->stringField[index1 + x];
    x++;
    s->stringFieldLen++;
  }

  x = 0;

  while (s->stringField[index2 + x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strAdd: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = s->stringField[index2 + x];
    x++;
    s->stringFieldLen++;
  }
  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++; // always points to the next free char

  return retval;
}

uint16_t strInsert(uint16_t index1, uint16_t index2, uint16_t pos, svsVM *s) {
  uint16_t x = 0;
  uint16_t cont;
  uint16_t retval;
  retval = s->stringFieldLen;

  // fill buffer with source string upto pos
  while ((s->stringField[index1 + x] != 0) && (x != pos)) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strInsert: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = s->stringField[index1 + x];
    x++;
    s->stringFieldLen++;
  }

  cont = x;

  x = 0;

  // instert string index 2
  while (s->stringField[index2 + x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strInsert: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = s->stringField[index2 + x];
    x++;
    s->stringFieldLen++;
  }

  // finish first string
  x = cont;

  while(s->stringField[index1 + x] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      errSoft((uint8_t *)"strInsert: String field full!", s);
      return 0;
    }
    s->stringField[s->stringFieldLen] = s->stringField[index1 + x];
    x++;
    s->stringFieldLen++;
  }

  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++;

  return retval;
}

void strNewStreamInit(svsVM *s) {
  streamID = s->stringFieldLen;
}

uint8_t strNewStreamPush(uint8_t c, svsVM *s) {
  if (s->stringFieldLen >= (s->stringFieldMax - 10)) {
    return 1;
  }
  s->stringField[s->stringFieldLen] = c;
  s->stringFieldLen++;

  return 0;
}

uint16_t strNewStreamEnd(svsVM *s) {
  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++;
  return streamID;
}

uint8_t strCmp(uint8_t *a, uint8_t *b) {
  uint16_t x = 0;

  while (1) {
    if ((a[x] == 0) || (b[x] == 0)) {
      if (a[x] != b[x]) {
        return 0;
      }
      return 1;
    } else {
      if (a[x] != b[x]) {
        return 0;
      }
      x++;
    }
  }
  return 0;
}


uint16_t strLenId(uint16_t id, svsVM *s) {
  uint16_t i = 0;
  while(s->stringField[i + id] == 0 && i < s->stringFieldLen) {
    i++;
  }

  return i;
}


VARTYPE i16toString(VARTYPE num, svsVM *s) {
  errStruct err;
  err.errString = "";
  err.tokenId = 0;
  VARTYPE retval;

  uint8_t i[16] = "              0";
  uint8_t a = 0;

  retval.val_u = s->stringFieldLen;

  if (num.val_s < 0) {
    num.val_s *= -1;
    while (0 != num.val_s) {
      i[14 - a] = (num.val_s % 10 + 48);
      num.val_s = num.val_s / 10;
      a++;
    }
    i[14-a] = '-';
  } else {
    while (0 != num.val_s) {
      i[14 - a] = (num.val_s%10 + 48);
      num.val_s = num.val_s / 10;
      a++;
    }
  }

  while(i[0] == ' ') {
    for(a = 0; a < 15; a++) {
      i[a] = i[a + 1];
    }
  }

  a = 0;
  while(i[a] != 0) {
    if (s->stringFieldLen >= (s->stringFieldMax - 1)) {
      err.errString = "svsMisc: i16toString: String field full!";
      err.tokenId = 0;
      errMsg(err);
    }

    s->stringField[s->stringFieldLen] = i[a];
    s->stringFieldLen++;
    a++;
  }

  s->stringField[s->stringFieldLen] = 0;
  s->stringFieldLen++; //míří vždy na první volné místo
  return retval;
}

#ifdef USE_FLOAT
varType floatToString(varType num, svsVM *s) {
  VARTYPE retval;
  uint8_t neg = 0;
  uint8_t a,b;

  uint64_t base     = 0;
  uint64_t fraction = 0;
  uint16_t decnum   = 0;

  uint8_t outBuff[20] = "0000000000000.00000";
  //                     -999999999999.99999

  if (errCheck(s)) {
    return (varType) (int32_t)0;
  }

  if (num.val_f == 0) {
    retval.val_str = strNew((uint8_t *)"0", s);
    return retval;
  }

  if (num.val_f > 999999999999.99999) {
    retval.val_str = strNew((uint8_t *)"Inf", s);
    return retval;
  }
  if (((num.val_f < 0.00001) && (num.val_f > 0.0)) || ((num.val_f > -0.00001) && (num.val_f < 0))) {
    retval.val_str = strNew((uint8_t *)"NaN", s);
    return retval;
  }

  if (num.val_f > 999999999999.99999) {
    retval.val_str = strNew((uint8_t *)"+Inf", s);
    return retval;
  }

  if (num.val_f < -999999999999.99999) {
    retval.val_str = strNew((uint8_t *)"-Inf", s);
    return retval;
  }

  if (num.val_f < 0.0) {
    neg = 1;
    num.val_f *= -1;
  }

  base = (uint64_t)num.val_f;
  num.val_f -= (float)base;

  // move fraction above zero
  while (num.val_f != (float)((uint64_t)num.val_f)) {
    num.val_f *= 10;
    decnum++;
  }

  fraction = (uint64_t)num.val_f;

  while (decnum > 5) {
    if ((fraction % 10) >= 5) {
      fraction /= 10;
      fraction += 1;
    } else {
      fraction /= 10;
    }
    decnum--;
  }

  a = 0;

  while (0 != fraction) {
    outBuff[18 - a - 5 + decnum] = (fraction % 10 + 48);
    fraction = fraction / 10;
    a++;

    if (18 - a - 5 + decnum == 13 && fraction != 0) {
      outBuff[13] = '.';
      base++;
      break;
    }
  }

  a = 0;

  while (0 != base) {
    outBuff[12 - a] = (base % 10 + 48);
    base = base / 10;
    a++;
  }

  a = 18;
  while(outBuff[a] == '0') {
    outBuff[a] = 0;
    a--;
  }

  if (outBuff[a] == '.') {
    outBuff[a] = 0;
  }

  a = 0;

  if (neg == 0) {
    while ((outBuff[0] == '0') && (outBuff[1] != '.')) {
      for (b = 0; b < 19; b++) {
        outBuff[b] = outBuff[b + 1];
      }
      a++;
    }
  } else {
    outBuff[0] = '-';
    while((outBuff[1] == '0') && (outBuff[2] != '.')) {
      for (b = 1; b < 19; b++) {
        outBuff[b] = outBuff[b + 1];
      }
      a++;
    }
  }

  if (errCheck(s)) {
    return retval;
  }

  retval.val_str = strNew(outBuff,s);
  return retval;
}

#else

varType floatToString(varType num, svsVM *s) {
  VARTYPE retval;
  retval.val_str = 0;
  return retval;
}
#endif