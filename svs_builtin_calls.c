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

#include "svs_builtin_calls.h"

uint16_t callToken; // used to precisely locate errors

svsBuiltInCallsTableType svsBuiltInCallsTable[] = {
	{"num", 1},
	{"float", 2},
	{"print", 3},
	{"isnum", 4},
	{"typeof", 5},
	{"getcp", 6},
	{"len", 7},
	{"end", 0}
};

uint16_t getBuiltInCallId(uint8_t * str) {
	uint16_t x = 0;
	while (strCmp("end", (char*)(svsBuiltInCallsTable[x].name)) != 1) {
		if (strCmp(str, (char*)(svsBuiltInCallsTable[x].name))) {
			return svsBuiltInCallsTable[x].id;
		}
		x++;
	}
	return 0;
}



uint16_t processBuiltInCall(uint16_t index, varRetVal *result, svsVM *s) {
	 uint16_t callId;
	 uint16_t x = 1;
	 varRetVal pracVar;
	 varType args[FUNCTION_ARGS_MAX];
	 uint8_t argType[FUNCTION_ARGS_MAX];

	 callId = getTokenData(index, s).val_u;
	 index++;

	 //tady načteme nové
	 if ((getTokenType(index, s) == 5)) {
		index++;
		if (getTokenType(index, s) != 6) { //pokud nemáme prázdnou závorku
			exprExec(index, &pracVar, s);
			args[x] = pracVar.value;
			argType[x] = pracVar.type;
			index = pracVar.tokenId;
			x++;
		}

		while((getTokenType(index, s) == 33)) { //argumenty odděleny čárkou
			if (x == FUNCTION_ARGS_MAX + 1) {
				errSoft("processBuiltInCall: too many arguments in function call!", s);
				errSoftSetParam("TokenId", (varType)index, s);
				errSoftSetToken(index, s);
				return 0;
			}
			index++;
			exprExec(index, &pracVar, s);
			args[x] = pracVar.value;
			argType[x] = pracVar.type;
			index = pracVar.tokenId;
			x++;
		}

		if (getTokenType(index, s) != 6) {
			errSoft("processBuiltInCall: Syntax error at end of function call. (missing \")\")", s);
			errSoftSetParam("TokenId", (varType)index, s);
			errSoftSetToken(index, s);
			return 0;
		}

		index++;
		varRetValZero(&pracVar);

		//Exec
		callToken = index;
		execBuiltInCall(callId, args, argType, x - 1, &pracVar, s);

		// Check
		if (errCheck(s)) {
			return 0;
		}

		// Return
		result->value = pracVar.value;
		result->type = pracVar.type;
		result->tokenId = index;
		return 1;

	} else {
		errSoft("processBuiltInCall: Syntax error at the begin of function call. (missing \"(\")", s);
		errSoftSetParam("TokenId", (varType)index, s);
		errSoftSetToken(index, s);
		return 0;
	}

}

static void simpleError(uint8_t * text, svsVM *s) {
	errSoft(text, s);
	errSoftSetParam("TokenId", (varType)callToken, s);
	errSoftSetToken(callToken, s);
}

uint16_t execBuiltInCall(uint16_t callId, varType *args,  uint8_t * argType, uint16_t count, varRetVal *result, svsVM *s) {
	// real
	if (callId == 1) {
		varType prac;
		uint8_t negative = 0;
	  uint16_t x = 0;
	  prac.val_s = 0;

	  if (count != 1) {
	  	simpleError("real(): wrong argument count!", s);
	  	return 0;
	  }

		if (argType[1] == 1) { // from string
			prac.val_s = 0;

			while((!isNumber(s->stringField[args[1].val_str + x]))&&(s->stringField[args[1].val_str + x] != 0)) {
				x++;
			}

			if(s->stringField[args[1].val_str + x] == '-') {
				negative = 1;
				x++;
			}

			while((s->stringField[args[1].val_str+x] != 0)&&(s->stringField[args[1].val_str+x] != '.')) {
				if((s->stringField[args[1].val_str+x] >= '0') && (s->stringField[args[1].val_str+x]<='9')) {
					prac.val_s *= 10;
					prac.val_s += s->stringField[args[1].val_str + x] - 48;
				}
				x++;
			}

			if (negative) {
				prac.val_s *= -1;
			}

			result->value = prac;
			result->type = 0;
			return 1;
		}

		if (argType[1] == 3) { // from float
			result->value = (varType)((int32_t) args[1].val_f);
			result->type = 0;
			return 1;
		}

		if (argType[1] == 0) { // from num
			result->value = args[1];
			result->type = 0;
			return 1;
		}

		simpleError("real(): wrong type of argument!", s);
		return 0;
	}

	// flt(num) # takes num or str returns float
	if (callId == 2) {

		if (count != 1) {
	  	simpleError("flt(): wrong argument count!", s);
	  	return 0;
	  }

	  if (argType[1] == 0) { // from num
			result->value = (varType)((float)args[1].val_s);
			result->type = 3;
			return 1;
		}

		if (argType[1] == 1) { // from str
			varType prac;
	  	float fltPrac = 0;
	  	uint8_t negative = 0;
	  	uint16_t x = 0;
	  	prac.val_s = 0;
	  	uint16_t float_dp = 0;

			fltPrac = 0;

			while((!isNumber(s->stringField[args[1].val_str + x])) \
						&& (s->stringField[args[1].val_str + x] != 0) \
						&& (s->stringField[args[1].val_str + x] != '-')) {
				x++;
			}

			if(s->stringField[args[1].val_str + x] == '-') {
				negative = 1;
				x++;
			}

			while((s->stringField[args[1].val_str + x] != 0) && isNumber(s->stringField[args[1].val_str + x])) {
				fltPrac *= 10;
				fltPrac += (float)(s->stringField[args[1].val_str + x] - 48);
				x++;
			}

			if ((s->stringField[args[1].val_str + x] == '.')){
				x++;
				while((s->stringField[args[1].val_str + x] != 0) && isNumber(s->stringField[args[1].val_str + x])) {
						fltPrac += ((float)(s->stringField[args[1].val_str + x] - 48)/(float)((exp_helper(10, float_dp))));
						float_dp++;
						x++;
				}
			}

			if (negative == 1) {
				fltPrac *= -1;
			}

			result->value = (varType)(fltPrac);
			result->type = 3;
			return 1;
		}

		if (argType[1] == 3) { // from float
			result->value = args[1];
			result->type = 3;
			return 1;
		}

		simpleError("flt(): wrong type of argument!", s);
	  return 0;
	}

	// print
	if (callId == 3) {
		if (count != 1) {
	  	simpleError("print(): wrong argument count!", s);
	  	return 0;
	  }

		if (argType[1] == 1) {
			printf("%s\n", (uint8_t *)(s->stringField+args[1].val_str));
			result->value = (varType)0;
			result->type = 0;
			return 1;
		}

		simpleError("print(): wrong type of argument!", s);
	  return 0;
	}

	// isnum(str|num)
	if (callId == 4) {

		if (count != 1) {
	  	simpleError("isnum(): wrong argument count!", s);
	  	return 0;
	  }

	  if ((argType[1] == 1)) {
			int32_t isNum = 1;
			int32_t isFlt = 0;
			uint8_t str;
			uint16_t x = 0;

			while (s->stringField[args[1].val_str + x] != 0) {
				str = s->stringField[args[1].val_str + x];
				if ((!isNumber(str))&&(str != ' ')&&(str != '.')&&(str != '-')) {
					isNum = 0;
				}
				if (str == '.'){
					isFlt = 1;
				}

				x++;
			}

			// when num return is 1, when float return is 2, otherwise zero
			result->value = (varType)(isNum + (isFlt * isNum));
			result->type = 0;
			return 1;
		}

		if (argType[1] == 0){
			result->value = (varType)1;
			result->type = 0;
			return 1;
		}

		if (argType[1] == 3){
			result->value = (varType)2;
			result->type = 0;
			return 1;
		}

		simpleError("isnum(): wrong type of argument!", s);
	  return 0;
	}

	// typeof(var)
	if (callId == 5) {

		if (count != 1) {
	  	simpleError("typeof(): wrong argument count!", s);
	  	return 0;
	  }

		result->value = (varType)((int32_t)argType[1]);
		result->type = 0;
		return 1;
	}

	// getcp
	if (callId == 6) {
		uint16_t len = 1; // we start indexing strings from 1
		uint16_t x = 0;
		uint8_t tmpChar[4];

		tmpChar[0] = 0;

		if (count != 2) {
	  	simpleError("getcp(): wrong argument count!", s);
	  	return 0;
	  }

		if (argType[1] != 1){
			simpleError("getcp(): wrong type of argument 1!", s);
			return 0;
		}

		if (argType[2] != 0){
			simpleError("getcp(): wrong type of argument 2!", s);
			return 0;
		}

		while (s->stringField[args[1].val_str + x] != 0) {
			if (len == args[2].val_s) {
				if ((s->stringField[args[1].val_str + x] >= 0xC3) \
							&& (s->stringField[args[1].val_str + x] <= 0xC5)) {
					tmpChar[0] = s->stringField[args[1].val_str + x];
					tmpChar[1] = s->stringField[args[1].val_str + x + 1];
					tmpChar[2] = 0;
				} else {
					tmpChar[0] = s->stringField[args[1].val_str + x];
					tmpChar[1] = 0;
				}
				break;
			}

			if ((s->stringField[args[1].val_str + x] >= 0xC3) \
					&&(s->stringField[args[1].val_str + x] <= 0xC5)) {
				x++;
			}
			len++;
			x++;
		}

		result->value = (varType)(strNew(tmpChar, s));
		result->type = 1;
		return 1;
	}

	// len
	if (callId == 7) {
		uint16_t len = 0;
		uint16_t x = 0;

		if (count != 1) {
	  	simpleError("len(): wrong argument count!", s);
	  	return 0;
	  }

		if (argType[1] != 1){
			simpleError("len(): wrong type of argument!", s);
			return 0;
		}

		while (s->stringField[args[1].val_str + x] != 0) {
			if ((s->stringField[args[1].val_str + x] >= 0xC3) \
					&& (s->stringField[args[1].val_str + x] <= 0xC5)) {
				x++;
			}
			len++;
			x++;
		}
		// to get just the characters, without end of string
		result->value = (varType)((int32_t)len);
		result->type = 0;
		return 1;
	}
}
