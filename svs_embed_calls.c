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

#include "svs_embed_calls.h"

uint16_t callToken; // used to precisely locate errors

svsEmbedCallsTableType svsEmbedCallsTable[] = {
	{"real", 1},
	{"flt", 2},
	{"end", 0}
};

uint16_t getEmbedCallId(uint8_t * str) {
	uint16_t x = 0;
	while (strCmp("end", (char*)(svsEmbedCallsTable[x].name)) != 1) {
		if (strCmp(str, (char*)(svsEmbedCallsTable[x].name))) {
			return svsEmbedCallsTable[x].id;
		}
		x++;
	}
	return 0;
}



uint16_t processEmbedCall(uint16_t index, varRetVal *result, svsVM *s) {
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
				errSoft("processEmbedCall: too many arguments in function call!", s);
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
			errSoft("processEmbedCall: Syntax error at end of function call. (missing \")\")", s);
			errSoftSetParam("TokenId", (varType)index, s);
			errSoftSetToken(index, s);
			return 0;
		}

		index++;
		varRetValZero(&pracVar);

		//Exec
		callToken = index;
		execEmbedCall(callId, args, argType, x - 1, &pracVar, s);

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
		errSoft("processEmbedCall: Syntax error at the begin of function call. (missing \"(\")", s);
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

uint16_t execEmbedCall(uint16_t callId, varType *args,  uint8_t * argType, uint16_t count, varRetVal *result, svsVM *s) {
	if (callId == 1) { // real
		varType prac;
	  uint16_t x = 0;
	  prac.val_s = 0;

	  if ((argType[1] != 1)&&(argType[1] != 3)) {
	  	simpleError("real: wrong type of argument!", s);
	  	return 0;
	  }

	  if (count != 1) {
	  	simpleError("real: wrong argument count!", s);
	  	return 0;
	  }

		if (argType[1] == 1) { // from string
			prac.val_s = 0;
			if(s->stringField[args[1].val_str] == '-') {
				x = 1;
				while(s->stringField[args[1].val_str+x] != 0) {
					if((s->stringField[args[1].val_str+x] >= '0') && (s->stringField[args[1].val_str+x]<='9')) {
						prac.val_s *= 10;
						prac.val_s += s->stringField[args[1].val_str + x] - 48;
					}
					x++;
				}
				prac.val_s *= -1;
			} else {
				x = 0;
				while(s->stringField[args[1].val_str+x]!=0){
					if((s->stringField[args[1].val_str+x]>='0')&&(s->stringField[args[1].val_str+x]<='9')){
						prac.val_s*=10;
						prac.val_s+=s->stringField[(args[1].val_str)+x]-48;
					}
					x++;
				}
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

	}
}
