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

This is the main command execution logic.
*/

#include "svs_comm_exec.h"

uint8_t commExDebug;

void setCommExDebug (uint8_t level) {
	commExDebug = level;
}

void commExDMSG(char *text, uint16_t tokenId) {
	if (commExDebug == 1) {
		printf("commExDMSG: %s \ntokenId: %u\n", text, tokenId);
	}
}


// přeskakuje while smyčku, případně jednu z větví ifu
// used for skipping code after if/else/while, if the equation say so
uint16_t commSkip(uint16_t index, svsVM *s) {
	uint16_t x = 0;
	uint16_t count = 0;

	x = index;
	commExDMSG("commSkip start", index);

	while(1) { //začátek bloku
		if (getTokenType(x,s) == 7) {
			count += 1 ;
		}

		if (getTokenType(x ,s) == 8) {
			if (count > 0) {
				count -= 1;
			} else {
				errSoft((uint8_t *)"commSkip: Bracket sanity error. Maybe missing \";\"?" ,s);
				errSoftSetParam((uint8_t *)"TokenId", (varType)x ,s);
				errSoftSetToken(x, s);
				return 0;
			}
			if (count == 0) {
				commExDMSG("commSkip end (skipped BLOCK(S))", x);
				break;
			}
		}

		//řeší situaci kdy za příkazem není závorka, funguje ok, pokud je na konci řádku středník
		if ((getTokenType(x, s) == 9) && (count == 0)) {
			commExDMSG("commSkip end (skipped LINE)", x);
			break;
			//blok má ve zvyku obdržet token posledního vykonaného příkazu, inkrement provádí sám.
		}

		x++; //poskočí na další token
	}

	if ((x >= index) && (x <= s->tokenMax)) {
		return x;
	} else {
		errSoft("commSkip: Skip sanity error.", s);
		errSoftSetParam("after TokenId", (varType)index, s);
		errSoftSetToken(index, s);
		return 0;
	}
}


// main execution function, takes token index, executes it and returns the next token
uint16_t commExecLoop(uint16_t index, svsVM *s) {
	uint16_t currToken = 0;
	uint16_t x = 0;
	uint16_t prac = 0;
	uint16_t backBr = 0;
	// zde je uložená pozice ve vartable, pro použití lokálních proměnných
	// varTableSP - variable table stack pointer, used with local variables
	uint16_t varTableSP = 0;
	uint8_t lock = 1;
	varRetVal varPrac;

	varRetValZero(&varPrac);

	currToken = index;

	errSoftSetToken(currToken ,s);

	if (s->handbrake == 1) {
		return 0;
	}

	if(getTokenType(currToken, s) != 255) {
		commExDMSG("commExecLoop", currToken);
		lock = 1;

		garbageCollect(0, s);

		// function call
		// volání funkce
		if ((getTokenType(currToken, s) == 17) && lock) {
			lock = 0;
			commExDMSG("commExecLoop: Function Call", currToken);
			commExDMSG(s->stringField + getTokenData(currToken, s).val_u, currToken);

			return commParseCall(currToken, s);
		}

		// local statement
		if ((getTokenType(currToken, s) == 32) && lock) {
			lock = 0;
			commExDMSG("commExecLoop: LOCAL statement", currToken);
			currToken++;

			if (getTokenType(currToken, s) != 10) { // expecting VAR
				errSoft("commEx: Syntax error next to LOCAL: Expected VAR after LOCAL.", s);
				errSoftSetParam("TokenId", (varType)currToken, s);
				errSoftSetToken(currToken, s);
				return 0;
			} else {
				varAddLocal(getTokenData(currToken, s), s); //přidáme lokální promněnnou / adds a local variable

				if (errCheck(s)) { //kontrola a obsluha možné chyby v předchozím volání / error check
					return 0;
				}
				currToken++;

				if (getTokenType(currToken, s) != 9) { //zkontrolujeme středník / semicolon check
					errSoft("commEx: Syntax error, missing ; .", s);
					errSoftSetParam("TokenId", (varType)currToken, s);
					errSoftSetToken(currToken, s);
					return 0;
				}
			}

			return currToken;
		}

		if ((getTokenType(currToken, s) == 28) && lock) { //sys statement
			lock = 0;
			commExDMSG("commExecLoop: SYS statement", currToken);
			sysExec(currToken, &varPrac, s);
			if (errCheck(s)) {
				return 0;
			}
			currToken = varPrac.tokenId;
			return currToken;
		}

		// equals statement
		if ((getTokenType(currToken, s) == 10) && lock) {
			//pokud narazíme na typ VAR / if we found a variable
			lock = 0;
			x = currToken; //uložíme token indexu promněnné / we store index of the variable
			commExDMSG("commExecLoop: = statement", currToken);
			currToken++;
			if (getTokenType(currToken,s) != 24) { //očekáváme "=" / expecting "="
				errSoft("commEx: Syntax error next to VAR (missing \"=\").", s);
				errSoftSetParam("TokenId", (varType)currToken, s);
				errSoftSetToken(currToken, s);
				return 0;
			}else{
				currToken++;
				//printf("var set val id: %u, value: %u\n", tokenData[x], exprExec(currToken));
				//printf("math exec id: %u\n", currToken);

				exprExec(currToken, &varPrac, s); //vykonáme výraz za = / executing expression after =
				if (errCheck(s)) {
					return 0;
				}
				varSetVal(getTokenData(x,s), varPrac.value, s); //nastavíme value / we set new value

				varSetType(getTokenData(x,s), varPrac.type, s); //nastavíme typ / we set new type
				if (varPrac.type == 0) {
					commExDMSG("commExecLoop: = statement: result is NUM", currToken);
				} else {
					commExDMSG("commExecLoop: = statement: result is STR", currToken);
				}

				currToken = varPrac.tokenId; //nastavíme token kde se má pokračovat / we set the token id we got from exprExec
				commExDMSG("commExecLoop: = statement: continue on token:", currToken);

				if (getTokenType(currToken,s) != 9) { //zkontrolujeme středník / semicolon check
					errSoft("commEx: Syntax error, missing ; .", s);
					errSoftSetParam("TokenId", (varType)currToken, s);
					errSoftSetToken(currToken, s);
					return 0;
				}
				return currToken;
			}
			return currToken;
		}

		if ((getTokenType(currToken,s) == 16) && lock) { //return
			lock = 0;
			if (s->commRetFlag == 0) {
				x = currToken;
				commExDMSG("commExecLoop: RETURN statement", currToken);
				currToken++;
				exprExec(currToken, &varPrac, s);
				if (errCheck(s)){
					return 0;
				}
				s->commRetVal = varPrac.value;
				s->commRetType = varPrac.type;
				s->commRetFlag = 1;
				return currToken - 1;
			}else{
				return currToken;
			}
		}

		if ((getTokenType(currToken, s) == 7) && lock) { // začátek bloku / start of block
			lock = 0;
			commExDMSG("commExecLoop: start of block ({)", currToken);
			currToken++; //odskočíme ze "{"
			varTableSP = s->varTableLen; //uložíme pozici var stacku
			while (getTokenType(currToken, s) != 8) { //pokud není "}"
				/*
				Break funguje tak, že když se na něj narazí, tak se snaží commExecLoop
				returnovat break token tak dlouho, až se narazí na while co ho zavolal
				a ten se breakne.
				*/

				if (getTokenType(currToken, s) != 15) { //break? ne
					//vykonáváme příkaz dokud nenarazíme na break a nebo end of block
					currToken = commExecLoop(currToken, s);
					if (errCheck(s)) {
						return 0;
					}
					if (s->handbrake == 1) {
						return 0;
					}

					if (getTokenType(currToken, s) == 15) { //break
						s->varTableLen = varTableSP; //při opuštění bloku vrátíme stack
						commExDMSG("commExecLoop: block: break occured inside block!", currToken);
						return currToken;
					}
					if (getTokenType(currToken, s) == 16) { //return
						s->varTableLen = varTableSP;
						commExDMSG("commExecLoop: block: end of block", currToken);
						return currToken;
					}
					currToken++;
				} else { //break
					s->varTableLen = varTableSP;
					commExDMSG("commExecLoop: start of block: break occured!", currToken);
					return currToken;
				}
			}
			s->varTableLen = varTableSP;
			return currToken;
		}

		if ((getTokenType(currToken, s) == 12) && lock) { //if / else
			lock = 0;
			commExDMSG("commExecLoop: if statement", currToken);
			currToken++;
			if (getTokenType(currToken, s) != 5) { // detekce závorky
				errSoft("commExecLoop: Unknown statement after if", s);
				errSoftSetParam("TokenId", (varType)currToken, s);
				errSoftSetToken(currToken, s);
				return 0;
			} else {
				currToken++;
				exprExec(currToken, &varPrac, s);
				if (errCheck(s)) {
					return 0;
				}
 				x = varPrac.value.val_s; //výsledek ifu
				currToken = varPrac.tokenId;
				if (getTokenType(currToken,s) != 6) {
					errSoft("commEx: Syntax error in if statement, missing \")\"", s);
					errSoftSetParam("TokenId", (varType)currToken, s);
					errSoftSetToken(currToken, s);
					return 0;
				}
				currToken++;
				if (x) { //podmínka splněna
					commExDMSG("commExecLoop: if statement: expression TRUE", currToken);
					//curr token teď míří přímo na novej příkaz
					if (getTokenType(currToken, s) != 15) {
						currToken = commExecLoop(currToken, s);
						if (errCheck(s)) {
							return 0;
						}
						if (getTokenType(currToken, s) == 16) {
							//printf("if: return occured token: %u \n", currToken);
							return currToken;
						}
					} else {
						//printf("if: break occured token: %u \n", currToken);
						return currToken;
					}
				} else {
					//currToken++; //curr token teď míří přímo na novej příkaz
					//printf("comSkip míří na token %u \n", currToken);
					currToken = commSkip(currToken, s);
					if (errCheck(s)) {
						return 0;
					}
					commExDMSG("commExecLoop: if statement: expression FALSE, code skipped", currToken);
				}
				//pokud existuje else větev
				if(getTokenType(currToken + 1, s) == 13) {
					commExDMSG("commExecLoop: if statement: else detected", currToken);
					currToken += 1;
					if (!x){ //podmínka nesplněna
						currToken++;
						commExDMSG("commExecLoop: if statement: else: executing", currToken);
						if (getTokenType(currToken,s) != 15) {
							currToken = commExecLoop(currToken, s);
							if (errCheck(s)) {
								return 0;
							}
							if (getTokenType(currToken, s) == 16) {
								//printf("if: return occured token: %u \n", currToken);
								return currToken;
							}
						} else {
							//printf("if: break occured token: %u \n", currToken);
							return currToken;
						}
					} else {
						currToken++; //curr token teď míří přímo na novej příkaz
						currToken = commSkip(currToken, s);
						if (errCheck(s)) {
							return 0;
						}
						commExDMSG("commExecLoop: if statement: else: skipped", currToken);
					}
				}
			}
			commExDMSG("commExecLoop: if statement: end of if", currToken);
			return currToken;
		}

		if ((getTokenType(currToken, s) == 34) && lock) { //for
			uint16_t exprPrac = 0;
			uint16_t endPrac = 0;
			uint16_t loopPrac = 0;
			uint32_t x = 0;
			lock = 0;
			commExDMSG("commExecLoop: for statement", currToken);
			currToken++;

			//struktura: for(init expr;check expr;endloop expr)

			if (getTokenType(currToken,s) != 5) { // detekce závorky
				errSoft((uint8_t *)"commEx: Unknown statement after while, missing \"(\"", s);
				errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
				errSoftSetToken(currToken, s);
				return 0;
			} else {
				currToken++;
				currToken = commExecLoop(currToken, s);
				if (errCheck(s)) {
					return 0;
				}
				if (getTokenType(currToken, s) != 9) { //zkontrolujeme středník
					errSoft((uint8_t *)"commExecLoop: for: Syntax error, missing ; .", s);
					errSoftSetParam((uint8_t *)"TokenId", (varType)currToken, s);
					errSoftSetToken(currToken, s);
					return 0;
				}

	 			currToken++;
	 			//kód podmínky
	 			exprPrac = currToken;
	 			while(1) {
		 			currToken = exprPrac;
		 			exprExec(currToken, &varPrac, s);
					if (errCheck(s)) {
						return 0;
					}
	 				x = varPrac.value.val_s; //výsledek podmínky
	 				currToken = varPrac.tokenId;
	 				endPrac = currToken + 1;

	 				//skip end prac
	 				currToken = commSkip(currToken + 1, s);
	 				if(x == 0) { //skip
	 					commExDMSG("commExecLoop: for: expression FALSE, skipping", currToken);
	 					//skip loop body
	 					currToken++;
	 					currToken = commSkip(currToken + 1, s);
	 					if (errCheck(s)) {
							return 0;
						}
	 					//brak out
	 					break;
	 				} else {
	 					commExDMSG("commExecLoop: for: expression TRUE", currToken);
	 				}
	 				if (getTokenType(currToken, s) == 9) {
	 					currToken++; //přeskočíme středník na konci výrazu
	 				}

	 				if (getTokenType(currToken, s) != 6) {
						errSoft("commEx: Syntax error in for statement, missing \")\" or maybe \";\"", s);
						errSoftSetParam("TokenId", (varType)exprPrac, s);
						//jako parametr stavíme exprPrac, kdyby current token mířil po skipu někam do háje
						errSoftSetToken(exprPrac, s);
						return 0;
					}
	 				currToken++;

	 				loopPrac = currToken;
	 				//tělo smyčky
	 				currToken = commExecLoop(loopPrac, s);
	 				if (errCheck(s)) {
						return 0;
					}

	 				//Detekce breaku
					if (getTokenType(currToken, s) == 15) {
						//v případě breaku nahrajeme do current tokenu start smyčky a pak jí přeskočíme comm skipem
						currToken = loopPrac;
						currToken = commSkip(currToken, s);
						if (errCheck(s)) {
							return 0;
						}
						commExDMSG("commExecLoop: for: break occured, skipping", currToken);
						break;
					}

					if (getTokenType(currToken, s) == 16) {
						commExDMSG("commExecLoop: while: return occured, returning", currToken);
						return currToken;
					}

	 				//inkrement/dekrement/whatever
	 				commExecLoop(endPrac, s);
	 				if (errCheck(s)) {
						return 0;
					}
 				}
			}

		}
		if ((getTokenType(currToken, s) == 14) && lock) { //while
			lock = 0;
			commExDMSG("commExecLoop: while statement", currToken);
			currToken++;
			if (getTokenType(currToken, s) != 5) { // detekce závorky
				errSoft("commEx: Unknown statement after while, missing \"(\"", s);
				errSoftSetParam("TokenId", (varType)currToken, s);
				errSoftSetToken(currToken, s);
				return 0;
			} else {
				currToken++;
				prac = currToken;
					while(1) {
					currToken = prac;
					exprExec(currToken, &varPrac, s);
					if (errCheck(s)) {
						return 0;
					}
	 				x = varPrac.value.val_s; //výsledek podmínky
					currToken = varPrac.tokenId;
					backBr = currToken;
					if (getTokenType(currToken, s) != 6) {
						errSoft("commEx: Syntax error in while statement, missing \")\"", s);
						errSoftSetParam("TokenId", (varType)currToken, s);
						errSoftSetToken(currToken, s);
						return 0;
					}
					if (x) { //podmínka splněna
						commExDMSG("commExecLoop: while: expression TRUE", currToken);
						currToken++; //curr token teď míří přímo na novej příkaz
						currToken = commExecLoop(currToken, s);
						if (errCheck(s)) {
							return 0;
						}
						//Detekce breaku
						if (getTokenType(currToken, s) == 15) {
							//v případě breaku nahrajeme do current tokenu start smyčky a pak jí přeskočíme comm skipem
							currToken = backBr + 1;
							currToken = commSkip(currToken, s);
							if (errCheck(s)) {
								return 0;
							}
							commExDMSG("commExecLoop: while: break occured, skipping", currToken);
							break;
						}

						if (getTokenType(currToken, s) == 16) {
							commExDMSG("commExecLoop: while: return occured, returning", currToken);
							return currToken;
						}

					} else {
						currToken++; //curr token teď míří přímo na novej příkaz
						currToken = commSkip(currToken, s);
						if (errCheck(s)) {
							return 0;
						}
						commExDMSG("commExecLoop: while: expression FALSE, skipping", currToken);
						break;
					}
				}
			}
			commExDMSG("commExecLoop: while: end of while", currToken);
		}

		if(lock == 1) {
			errSoft("commEx: Unexpected command!", s);
			errSoftSetParam("TokenType", (varType)(uint16_t)getTokenType(currToken, s), s);
			errSoftSetToken(currToken, s);
			return 0;
		}
		return currToken;
	}
	return 0;
}

uint16_t commExecById(uint16_t id, svsVM *s) {
	//třeba nastavit startovací token funkce
	//printf("Jumping into: %s starting at token %u \n",funcTable[id].fString, funcTable[id].tokenId );

	s->commRetVal.val_u = 0;
	s->commRetType = 0;
	s->commRetFlag = 0;

	commExecLoop(s->funcTable[id].tokenId, s);

	return 0;
}

uint16_t commExec(uint8_t * name, svsVM *s) {
	if (functionExists(name, s)) {
		return commExecById(functionGetId(name, s), s);
	} else {
		errSoft("commExecById: Function does not exist.", s);
		return 0;
	}
}

void commArgNull(comExArgs *target) {
	uint8_t x;

	for(x = 0; x < FUNCTION_ARGS_MAX; x++) {
		target->arg[x] = (varType)((uint16_t)0);
		target->argType[x] = 0;
	}

	target->usedup = 0;
}

void commArgCopy(comExArgs* source, comExArgs* target){
	uint8_t x;

	for(x = 0; x < FUNCTION_ARGS_MAX; x++) {
		target->arg[x] = source->arg[x];
		target->argType[x] = source->argType[x];
	}

	target->usedup = source->usedup;
}

uint16_t commParseCall(uint16_t index, svsVM *s) {
	 uint8_t *callName;
	 uint16_t x = 1;
	 varRetVal pracVar;
	 comExArgs pracArgs;
	 callName = s->stringField + getTokenData(index, s).val_u;

	 //tady bude třeba uložit staré argumenty
	 commArgCopy(&s->commArgs, &pracArgs);
	 commArgNull(&s->commArgs);

	 index++;

	 //tady načteme nové
	 if ((getTokenType(index, s) == 5)) {
		index++;
		if (getTokenType(index, s) != 6) { //pokud nemáme prázdnou závorku
			exprExec(index, &pracVar, s);
			s->commArgs.arg[x] = pracVar.value;
			s->commArgs.argType[x] = pracVar.type;
			index = pracVar.tokenId;
			x++;
		}

		while((getTokenType(index, s) == 33)) { //argumenty odděleny čárkou
			if (x == FUNCTION_ARGS_MAX + 1) {
				errSoft("commParseCall: too many arguments in function call!", s);
				errSoftSetParam("TokenId", (varType)index, s);
				errSoftSetToken(index, s);
				return 0;
			}
			index++;
			exprExec(index, &pracVar, s);
			s->commArgs.arg[x] = pracVar.value;
			s->commArgs.argType[x] = pracVar.type;
			index = pracVar.tokenId;
			x++;
		}

		if (getTokenType(index, s) != 6) {
			errSoft("commParseCall: Syntax error at end of function call. (missing \")\")", s);
			errSoftSetParam("TokenId", (varType)index, s);
			errSoftSetToken(index, s);
			return 0;
		}

		s->commArgs.usedup = x - 1;

		index++;
		varRetValZero(&pracVar);

		//commExec

		commExec(callName, s);
		if (errCheck(s)) {
			return 0;
		}

		// ret val:			 s->commRetVal;
		// ret val type:	s->commRetType;

		//shození return flagu
		s->commRetFlag = 0;

		//navrácení argumentů zpět
		commArgCopy(&pracArgs, &s->commArgs);

		return index;

	} else {
		errSoft("sysExec: Syntax error at the begin of function call. (missing \"(\")", s);
		errSoftSetParam("TokenId", (varType)index, s);
		errSoftSetToken(index, s);
		return 0;
	}

}
