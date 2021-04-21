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

#ifndef SVS_BUILTIN_CALLS
#define SVS_BUILTIN_CALLS

#include "svs_basics.h"

typedef enum {
	NUM = 1,
	FLOAT = 2,
	PRINT = 3,
	ISNUM = 4,
	TYPEOF = 5,
	GETCP = 6,
	LEN = 7,
	SUBSTR = 8,
	// advanced math
	SIN = 9,
	COS = 10,
	TAN = 11,
	ATAN = 12,
	LOG = 13,
	EXP = 14,
	POW = 15,
	PI = 16,
	SQRT = 17,
	VER = 18,
 	RND = 19,
	DBG = 20,
	GCP = 21,
	GC = 22,
	INSTR = 23,
	LOWER = 24,
	UPPER = 25
} builtinCallEnum;

typedef struct {
  const char* name;
  builtinCallEnum id;
} svsBuiltInCallsTableType;

uint16_t getBuiltInCallId(uint8_t * str);
uint16_t processBuiltInCall(uint16_t index, varRetVal *result, svsVM *s);
uint16_t execBuiltInCall(builtinCallEnum callId, varType *args, uint8_t * argType, uint16_t count, varRetVal *result, svsVM *s);

#endif
