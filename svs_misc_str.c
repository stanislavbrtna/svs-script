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
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strNew: String field full!");
			errHalt();
		}
		s->stringField[s->stringFieldLen] = index[x];
		x++;
		s->stringFieldLen++;
	}

	s->stringField[s->stringFieldLen] = 0;
	s->stringFieldLen++;
	return retval;
}

uint16_t strAdd(uint16_t index1, uint16_t index2, svsVM *s) {
	uint16_t x = 0;
	uint16_t retval;
	retval = s->stringFieldLen;

	while (s->stringField[index1+x] != 0) {
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strAdd: String field full!");
			errHalt();
		}
		s->stringField[s->stringFieldLen] = s->stringField[index1 + x];
		x++;
		s->stringFieldLen++;
	}

	x = 0;

	while (s->stringField[index2 + x] != 0) {
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strAdd: String field full!");
			errHalt();
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
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strAdd: String field full!");
			errHalt();
		}
		s->stringField[s->stringFieldLen] = s->stringField[index1 + x];
		x++;
		s->stringFieldLen++;
	}

	cont = x;

	x = 0;

	// instert string index 2
	while (s->stringField[index2 + x] != 0) {
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strAdd: String field full!");
			errHalt();
		}
		s->stringField[s->stringFieldLen] = s->stringField[index2 + x];
		x++;
		s->stringFieldLen++;
	}

	// finish first string
	x = cont;

	while(s->stringField[index1 + x] != 0) {
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
			errMsgS("strAdd: String field full!");
			errHalt();
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
  if (s->stringFieldLen >= (STRING_FIELD_L - 100)) {
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
	uint8_t retval = 1;

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

VARTYPE i16toString(VARTYPE num, svsVM *s) {
  errStruct err;
	err.errString = "";
	err.tokenId = 0;
	VARTYPE retval;

  uint8_t i[16] = "              0";
	uint8_t a = 0;
	uint8_t b = 0;

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
		if (s->stringFieldLen >= (STRING_FIELD_L - 1)) {
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

varType floatToString(varType num, svsVM *s) {
#ifdef USE_FLOAT
  errStruct err;
	err.errString = "";
	err.tokenId = 0;
	VARTYPE retval;
	uint8_t neg = 0;
	uint8_t a,b;

	uint64_t prac1 = 0;
	uint64_t prac2 = 0;
	uint16_t decnum = 0;

  uint8_t i[20] = "0000000000000.00000";
  //               -999999999999.99999

	if (num.val_f == 0) {
	  retval.val_str = strNew("0", s);
	  return retval;
	}

	if (num.val_f > 999999999999.99999) {
	  retval.val_str = strNew("Inf", s);
	  return retval;
	}
	if ((num.val_f < 0.00001) && (num.val_f > 0.0) || (num.val_f > 0.00001) && (num.val_f < 0.0)) {
	  retval.val_str = strNew("NaN", s);
	  return retval;
	}

	if (num.val_f > 999999999999.99999) {
	  retval.val_str = strNew("+Inf", s);
	  return retval;
	}

	if (num.val_f < -999999999999.99999) {
	  retval.val_str = strNew("-Inf", s);
	  return retval;
	}

	if (num.val_f < 0.0) {
	  neg = 1;
	  num.val_f *= -1;
	}

	prac1 = (uint64_t)num.val_f;
	num.val_f -= (float)prac1;

	while (num.val_f != (float)((uint64_t)num.val_f)) {
	  num.val_f *= 10;
	  decnum++;
	}

	prac2 = (uint64_t)num.val_f;

	a = 0;

	while (0 != prac1) {
	  i[12 - a] = (prac1 % 10 + 48);
	  prac1 = prac1 / 10;
	  a++;
  }

  a = 0;

  while (decnum > 5) {
    if ((prac2 % 10) >= 5) {
      prac2 /= 10;
      prac2 += 1;
    } else {
      prac2 /= 10;
    }
    decnum--;
  }

  a = 0;

  while (0 != prac2) {
	  i[18 - a - 5 + decnum] = (prac2 % 10 + 48);
	  prac2 = prac2 / 10;
	  a++;
  }

	a = 18;
	while(i[a] == '0') {
	  i[a] = 0;
	  a--;
	}

	if (i[a] == '.') {
	  i[a] = 0;
	}

	a = 0;

	if (neg == 0) {
	  while ((i[0] == '0') && (i[1] != '.')) {
	    for (b = 0; b < 20; b++) {
	      i[b] = i[b + 1];
	    }
	    a++;
	  }
	} else {
	  i[0] = '-';
	  while((i[1] == '0') && (i[2] != '.')) {
	    for (b = 1; b < 20; b++) {
	      i[b] = i[b + 1];
	    }
	    a++;
	  }
	}

	retval.val_str = strNew(i,s);
	return retval;

	#else
	VARTYPE retval;
	retval.val_str = 0;
	return retval;
	#endif
}
