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
*/

#include "svs_tokenizer.h"

// enable tokenizer debug messages
static uint8_t tokenizerDebug = 0;
static uint8_t tokenizerInit;
static uint8_t tokenizerFerrFlag;

//exact debug helpers
static uint8_t tokenizer_exact_debug;
static uint16_t tokenizer_exact_token;
static uint16_t tokenizer_exact_char;
static uint16_t tokenizer_exact_line;

uint8_t debug_buffer[101];
uint8_t debug_buffer_pos;

void setTokenizerDebug(uint8_t level) {
	tokenizerDebug = level;
}

void setTokenizerFerrFlag() {
	tokenizerFerrFlag = 1;
}

uint8_t tokenGetch();

void tokenizerReset() {
	tokenizerInit = 0;
	tokenizer_exact_debug = 0;
}

void token_line_print() {
	uint16_t x;
	uint8_t c = 0;
	debug_buffer[100] = 0;
	printf("near line %u :\n %s", tokenizer_exact_line, debug_buffer);

	x = 0;
	c = tokenGetch();
	while ((c != '\n') && (c != 0) && (x < 100)) {
		printf("%c", c);
		c = tokenGetch();
		x++;
	}
}

// this will run tokenizer on svs file and print line with given token.
void tokenzer_print_token_line(uint16_t token, svsVM *s) {
	uint16_t x;

	for(x = 0; x < 100; x++) {
		debug_buffer[x] = 0;
	}
	tokenizer_exact_token = token;

	tokenizer_exact_char = 0;
	tokenizer_exact_line = 1;
	debug_buffer_pos = 0;

	printf("File: %s\n", s->fName);

	tokenGetchSetup(s->fName, 0);

	tokenizerReset();
	tokenizer_exact_debug = 1;

	// reset some tokenizer variables
	s->funcTableLen = 0;
	s->varTableLen = 0;
	s->stringFieldLen = 0;
	s->errToken = 0;
	s->err = 0;

	tokenParse(s);
	printf("Token %u, ", token);
	token_line_print();
}

void tokenizerErrorPrint(uint8_t *str) {
	printf("Tokenizer Error: %s\n", str);
	tokenizer_exact_line++;
	token_line_print();
	printf("\n");
}

void tokenizerErrorPrintNL(uint8_t *str) {
	printf("Tokenizer Error: %s\n", str);
}

void tokenDMSG(char *text, uint16_t tokenId, VARTYPE data, uint8_t type, uint16_t textPos) {
#ifndef DBG_DISABLED
	if(tokenizerDebug == 1) {
		printf("TokenDMSG: %s \ntokenId: %u\n", text, tokenId);
	}
	if(tokenizerDebug == 2) {
		printf("%u:%u:%i:%u (id:type:data:tP)\n", tokenId, type, data.val_s, textPos);
	}
#endif
}

uint8_t tokenGetchDbg() {
	uint8_t x;
	uint8_t c = 0;

	c = tokenGetch();

	if (c == '\n') {
		tokenizer_exact_line++;
		for(x = 0; x < 100; x++) {
			debug_buffer[x] = 0;
		}
		debug_buffer_pos = 0;
	} else {
		if (debug_buffer_pos < 99) {
			debug_buffer[debug_buffer_pos] = c;
			debug_buffer_pos++;
		} else {
			debug_buffer_pos = 0;
		}
	}

	//printf("token getch dbg: %c (%u)\n", c, c);
	return c;
}


uint8_t tokenPreprocessor() {
	uint8_t c = 0;

	c = tokenGetchDbg();

	if (c == '#') {
		c = tokenGetchDbg();
		if (c == '#') {
			return c;
		} else if (c == '*') {
			// multiline comment
			while (1) {
				c = tokenGetchDbg();
				if (c == '*') {
					c = tokenGetchDbg();
					if (c == '#') {
						return tokenGetchDbg();
					}
				}
			}
		} else {
			// single line comment
			while (c != '\n') {
				c = tokenGetchDbg();
			}
		}
		//printf("token prep dbg: %c\n", c);
		return c;
	} else {
		//printf("token prep dbg: %c\n", c);
		return c;
	}
}




uint8_t tokenInput(uint16_t index, uint8_t inc) {
	static uint8_t peek;
	static uint8_t strBuff[16];
	static uint16_t vTextPos;
	uint16_t x;

	if (tokenizerInit == 0) {
		tokenizerFerrFlag = 0;
		vTextPos = 0;
		tokenizerInit = 1;
		strBuff[0] = tokenPreprocessor();
	}

	if (inc == 1) {
			vTextPos++;
			for(x = 0; x < 15; x++) {
				strBuff[x] = strBuff[x + 1];
			}
			if (peek == 0) {
				strBuff[0] = tokenPreprocessor();
			}
			if (peek == 1) {
				peek = 0;
			}
			return 0;
	}

	if (index <= vTextPos) {
		return strBuff[index - vTextPos];
	} else {
		//peek
		strBuff[1] = tokenPreprocessor();
		peek = 1;
		return strBuff[index - vTextPos];
	}

	return 0;
}

/*
// stub used occasionaly for debug, TODO: use compiler flags
uint8_t tokenInput(uint16_t index, uint8_t inc){
	uint8_t c=0;
	c = tokenInputX(index, inc);

	printf("token input init:%u (index: %u, inc: %u): %c (%u)\n",tokenizerInit ,index, inc, c, c);
	return c;
}
*/

uint8_t tokenParse(svsVM *s) {
	uint16_t posText = 0;
	uint16_t posToken = 0;
	uint8_t	pracName[NAME_LENGTH];
	uint8_t	pracName2[NAME_LENGTH];
	uint8_t	pracStrInd = 0;
	uint8_t	Lock = 1;
	uint16_t x;
	uint16_t currLine = 0;
	uint16_t tokenMax = 0;
	VARTYPE numPrac;
	VARTYPE numPracF;
	int16_t brCount1 = 0; //počítá jednoduché závorky ( => +1 a ) => -1
	int16_t brCount2 = 0;	//počítá složené závorky {}
	uint8_t floatFound;
	uint16_t float_dp = 0;
	uint8_t esc_c_prac = 0;
	uint16_t tokenizer_prev_text;

	tokenizer_prev_text = 1;

	numPrac.val_s = 0;
	numPracF.val_s = 0;

	tokenMax = getTokenMax(s);

	// token input error testing
	tokenInput(0, 0);
	if (tokenizerFerrFlag) {
		tokenizerErrorPrintNL("tokenParse:Error in opening file.");
		return 1;
	}

	while (posToken < tokenMax) {

		if (posText == tokenizer_prev_text) {
			tokenizerErrorPrint("Tokenizer got stuck!");
			return 1;
		} else {
			tokenizer_prev_text = posText;
		}

		// nulování pracovních textových proměnných
		// putting zero in temp variables
		for(x = 0; x < NAME_LENGTH; x++) {
			pracName[x] = 0;
			pracName2[x] = 0;
		}

		Lock = 1;

		// number a float, záporná čísla řeší až expr exec
		if (isNumber(tokenInput(posText,0))){
			numPrac.val_s=0;
			numPracF.val_s=0;
			floatFound=0;
			setTokenType(posToken,0,s);
			//tokenType[posToken]=0;
			//tokenData[posToken]=tokenInput(posText,0)-48;
			numPrac.val_s=tokenInput(posText,0)-48;
			posText++;
			tokenInput(0,1);
			if (tokenInput(posText,0)=='.'){
			#ifdef USE_FLOAT
				floatFound=1;
				float_dp=1;
				tokenInput(0,1);
				posText++;
				numPrac.val_f=(float) numPrac.val_s;
			#else
				tokenizerErrorPrint("tokenParse: Float not supported in this build!");
				return 1;

			#endif
			}

			if(tokenInput(posText,0)=='x'){
				numPrac.val_s=0;
				tokenInput(0,1);
				posText++;
				while (((tokenInput(posText,0)>='0')&&(tokenInput(posText,0)<='9')) || ((tokenInput(posText,0)>='a')&&(tokenInput(posText,0)<='f'))
									|| ((tokenInput(posText,0)>='A')&&(tokenInput(posText,0)<='F'))){

						if((tokenInput(posText,0)>=47)&&(tokenInput(posText,0)<=57)){
							numPrac.val_s*=16;
							numPrac.val_s+=tokenInput(posText,0)-48;
							tokenInput(0,1);
							posText++;
						}

						if((tokenInput(posText,0)>=65)&&(tokenInput(posText,0)<=70)){
							numPrac.val_s*=16;
							numPrac.val_s+=tokenInput(posText,0)-55;
							tokenInput(0,1);
							posText++;
						}

						if((tokenInput(posText,0)>=97)&&(tokenInput(posText,0)<=102)){
							numPrac.val_s*=16;
							numPrac.val_s+=tokenInput(posText,0)-87;
							tokenInput(0,1);
							posText++;
						}

				}

				setTokenData(posToken, numPrac, s);
				tokenDMSG("Token set, type NUM", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			}else{
				while (isNumber(tokenInput(posText,0))){
					if(floatFound==0){
						numPrac.val_s*=10;
						numPrac.val_s+=tokenInput(posText,0)-48;
						tokenInput(0,1);
						posText++;
						if (tokenInput(posText,0)=='.'){
						#ifdef USE_FLOAT
							floatFound=1;
							float_dp=1;
							tokenInput(0,1);
							posText++;
							numPrac.val_f=(float) numPrac.val_s;
						#else
							tokenizerErrorPrint("tokenParse: Float not supported in this build!");
							return 1;
						#endif
						}
					}else{
						#ifdef USE_FLOAT
						numPracF.val_f+=((float)(tokenInput(posText,0)-48)/(float)((exp_helper(10,float_dp))));
						//printf("flt: %f fdp: %u \n",numPracF.val_f, float_dp);
						float_dp++;
						tokenInput(0,1);
						posText++;
						#endif
					}
				}
				if (floatFound==1){
					#ifdef USE_FLOAT
					setTokenType(posToken,31,s);

					// nefunguje na např 2.05
					//while(numPracF.val_f>1){
					//	numPracF.val_f=numPracF.val_f/10;
					//}
					numPrac.val_f+=numPracF.val_f;
					setTokenData(posToken, numPrac, s);
					tokenDMSG("Token set, type FLT", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
					//printf("flt: %f \n",getTokenData(posToken,s).val_f);
					floatFound=0;
					#endif
				}else{
					setTokenData(posToken, numPrac, s);
					tokenDMSG("Token set, type NUM", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				}
			}
			posToken++;
		}

		// space (ignored)
		// mezeru ignorujeme
		if (tokenInput(posText,0) == ' ') {
			tokenInput(0, 1);
			posText++;
		}

		// tab  (ignored)
		// tab ignorujeme
		if (tokenInput(posText, 0) == 9) {
			tokenInput(0, 1);
			posText++;
		}

		// newline: current line incremented, otherwise ignored
		if (tokenInput(posText, 0) == '\n') {
			currLine++;
			tokenInput(0, 1);
			posText++;
		}

		// +
		if (tokenInput(posText,0) == '+') {
			setTokenType(posToken, 1, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type +", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
		}

		// -
		if (tokenInput(posText, 0) == '-') {
			setTokenType(posToken, 2, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type -", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
		}

		// *
		if (tokenInput(posText, 0) == '*') {
			setTokenType(posToken, 3, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type *", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
		}

		// /
		if (tokenInput(posText, 0) == '/') {
			setTokenType(posToken, 4, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type /", posToken, getTokenData(posToken, s), getTokenType(posToken,s), posText);
			posToken++;
		}

		// %
		if (tokenInput(posText,0) == '%') {
			setTokenType(posToken, 30, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type %", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
		}

		// (
		if (tokenInput(posText, 0) == '(') {
			setTokenType(posToken, 5, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type (", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
			brCount1++; // variable used for bracket check
		}

		// )
		if (tokenInput(posText, 0) == ')') {
			setTokenType(posToken, 6, s);
			tokenInput(0, 1);
			posText++;
			tokenDMSG("Token set, type )", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			posToken++;
			brCount1--; //kontrola počtu závorek
		}

		// {
		if (tokenInput(posText,0)=='{'){
			//tokenType[posToken]=7;
			setTokenType(posToken,7,s);
			tokenInput(0,1);
			posText++;
			tokenDMSG("Token set, type {", posToken,	getTokenData(posToken,s),getTokenType(posToken,s), posText);
			posToken++;
			brCount2++;
		}

		// }
		if (tokenInput(posText,0)=='}'){
			//tokenType[posToken]=8;
			setTokenType(posToken,8,s);
			tokenInput(0,1);
			posText++;
			tokenDMSG("Token set, type }", posToken,	getTokenData(posToken,s),getTokenType(posToken,s), posText);
			posToken++;
			brCount2--;
		}

		// ;
		if (tokenInput(posText,0)==';'){
			//tokenType[posToken]=9;
			setTokenType(posToken,9,s);
			tokenInput(0,1);
			posText++;
			tokenDMSG("Token set, type ;", posToken,	getTokenData(posToken,s),getTokenType(posToken,s), posText);
			posToken++;
		}

		// ;
		if (tokenInput(posText,0)==','){
			setTokenType(posToken,33,s);
			tokenInput(0,1);
			posText++;
			tokenDMSG("Token set, type ,", posToken,	getTokenData(posToken,s),getTokenType(posToken,s), posText);
			posToken++;
		}

		//logic
		/*
		Logic:
		18 - ==	- rovno
		19 - !=	- nerovno
		20 - <	 - menší než
		21 - >	 - větší než
		22 - <=	- menší rovno
		23 - >=	- větší rovno

		24 - = přiřazení
		*/

		if (tokenInput(posText, 0) == '=') { //== a =
			if (tokenInput(posText + 1,0) == '=') {
				tokenInput(0, 1);
				posText++;
				setTokenType(posToken, 18, s);
				tokenDMSG("Token set, type ==", posToken,	getTokenData(posToken, s), getTokenType(posToken, s), posText);
			} else {
				setTokenType(posToken, 24, s);
				tokenDMSG("Token set, type =", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			}
			tokenInput(0, 1);
			posText++;
			posToken++;
		}

		if (tokenInput(posText, 0) == '<') { //< a <=
			if (tokenInput(posText + 1, 0) == '='){
				tokenInput(0, 1);
				posText++;
				setTokenType(posToken, 22, s);
				tokenDMSG("Token set, type <=", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			} else {
				setTokenType(posToken, 20, s);
				tokenDMSG("Token set, type <", posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
			}
			tokenInput(0, 1);
			posText++;
			posToken++;
		}

		if (tokenInput(posText,0)=='>'){ //> a >=
			if (tokenInput(posText+1,0)=='='){ //pouze přiřazení
				posText++;
				tokenInput(0,1);
				setTokenType(posToken,23,s);
				tokenDMSG("Token set, type >=", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			}else{
				setTokenType(posToken,21,s);
				tokenDMSG("Token set, type >", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			}
			tokenInput(0,1);
			posText++;
			posToken++;
		}

		if (tokenInput(posText,0)=='!'){
			if (tokenInput(posText+1,0)=='='){ //nerovno
				tokenDMSG("Token set, type !=", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				posText++;
				tokenInput(0,1);
				setTokenType(posToken,19,s);
			}else{
				tokenizerErrorPrint("Tokenizer: unknown symbol after \"!\"");
				return 1;
			}
			tokenInput(0,1);
			posText++;
			posToken++;
		}

		//txt
		if (tokenInput(posText,0)=='\"'){
			posText++;
			tokenInput(0,1);
			setTokenType(posToken,25,s);
			setTokenData(posToken,(varType)s->stringFieldLen,s);

			//maximální velikost textových konstant, pro garbage collection
			s->stringConstMax=s->stringFieldLen;
			//není použito unifikované funkce, protože tady by stejnak garbage
			//collcector nepomohl

			while((tokenInput(posText,0)!='\"')&&(tokenInput(posText,0)!=0)){
				if (s->stringFieldLen>=(STRING_FIELD_L-1)){
					tokenizerErrorPrint("Tokenizer: String field full!");
					return 1;
					//err.errString=;
					//err.tokenId=posToken;
					//errMsg(err);
				}
				if(tokenInput(posText,0)!='\\'){
					s->stringField[s->stringFieldLen]=tokenInput(posText,0);
				}else{
					esc_c_prac=tokenInput(posText+1,0);
					if(esc_c_prac=='n'){
						s->stringField[s->stringFieldLen]='\n'; // \n
					}else if(esc_c_prac=='\\'){
						s->stringField[s->stringFieldLen]='\\'; //
					}else if(esc_c_prac=='\"'){
						s->stringField[s->stringFieldLen]='\"'; // "
					}else{
						//errSoft("tokenParse:Undefined character after \"\\\"",s);
						tokenizerErrorPrint("tokenParse:Undefined character after \"\\\"");
						return 1;
					}
					posText++;
					tokenInput(0,1);
				}
				posText++;
				tokenInput(0,1);
				s->stringFieldLen++;
			}
			s->stringField[s->stringFieldLen]=0;
			s->stringFieldLen++; //míří vždy na první volné místo
			posText++;
			tokenInput(0,1);
			tokenDMSG("Token set, type STR, value:", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			tokenDMSG(s->stringField+getTokenData(posToken,s).val_str,posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			posToken++;
		}

		// & - creates string constant of a following symbol
		if (tokenInput(posText,0) == '&') {
			posText++;
			tokenInput(0, 1);

			setTokenType(posToken,25,s);

			setTokenData(posToken, (varType)s->stringFieldLen, s);

			//maximální velikost textových konstant, pro garbage collection
			s->stringConstMax = s->stringFieldLen;

			while((isRegChar(tokenInput(posText,0)) || isNumber(tokenInput(posText,0))) && (tokenInput(posText,0)!=0)) {
				if (s->stringFieldLen >= (STRING_FIELD_L-1)) {
					tokenizerErrorPrint("Tokenizer: String field full!");
					return 1;
				}

				s->stringField[s->stringFieldLen] = tokenInput(posText,0);
				posText++;
				tokenInput(0,1);
				s->stringFieldLen++;
			}
			s->stringField[s->stringFieldLen] = 0;
			s->stringFieldLen++; // always points on the next free
			tokenDMSG(
					"Token set, type STR, value:",
					posToken,
					getTokenData(posToken,s),
					getTokenType(posToken,s),
					posText
			);
			tokenDMSG(
					s->stringField + getTokenData(posToken,s).val_str,
					posToken,
					getTokenData(posToken,s),
					getTokenType(posToken,s),
					posText
			);
			posToken++;
		}

		// analizace textu
		if (isRegChar(tokenInput(posText,0))){
			// A-Z || a-z || _
			pracStrInd=0;
			// načtení slova do prac name
			pracName[pracStrInd]=tokenInput(posText,0);
			posText++;
			tokenInput(0,1);
			pracStrInd++;
			while(1){
				if (pracStrInd<NAME_LENGTH){
					if (isRegChar(tokenInput(posText,0))||isNumber(tokenInput(posText,0))){
						pracName[pracStrInd]=tokenInput(posText,0);
					}else{
						break;
					}
					posText++;
					tokenInput(0,1);
					pracStrInd++;
					pracName[pracStrInd]=0;
				}else{
					pracName[NAME_LENGTH-1]=0;
					tokenizerErrorPrint("tokenParse: Symbol name too long!");
					return 1;
				}
			}
			// detekce klíčových slov v prac name
			/*
			11 - function - ??
			12 - if
			13 - else
			14 - while
			15 - break
			16 - return

			když nic, tak
			10 - var
			*/


			if (strCmp(pracName, "end")){
				//tokenType[posToken]=255;
				setTokenType(posToken, 255,s);
				tokenDMSG("Token set, type EOF/END", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				printf("Tokenizer Done, %u of %u tokens used.\n",s->tokenMax, getTokenMax(s) );

				//pozor, tuten kód je ještě jednou dole, když tokenizer narazí na eof namísto endu
				if((brCount2==0) && (brCount1==0)){
					return 0;
				}else{
					if (brCount1<0){
						tokenizerErrorPrintNL("Bracket chceck error! Missing \"(\"");
						return 1;
					}

					if (brCount1>0){
						tokenizerErrorPrintNL("Bracket chceck error! Missing \")\"");
						return 1;
					}

					if (brCount2<0){
						tokenizerErrorPrintNL("Bracket chceck error! Missing \"{\"");
						return 1;
					}

					if (brCount2>0){
						tokenizerErrorPrintNL("Bracket chceck error! Missing \"}\"");
						return 1;
					}
				}

			}
			if (strCmp(pracName, "if")){
				tokenDMSG("Token set, type IF", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=12;
				setTokenType(posToken, 12,s);
				Lock=0;
			}
			if (strCmp(pracName, "else")){
				tokenDMSG("Token set, type ELSE", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=13;
				setTokenType(posToken, 13,s);
				Lock=0;
			}
			if (strCmp(pracName, "while")){
				tokenDMSG("Token set, type WHILE", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=14;
				setTokenType(posToken, 14,s);
				Lock=0;
			}
			if (strCmp(pracName, "break")){
				tokenDMSG("Token set, type BREAK", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=15;
				setTokenType(posToken, 15,s);
				Lock=0;
			}
			if (strCmp(pracName, "return")){
				tokenDMSG("Token set, type RETURN", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=16;
				setTokenType(posToken, 16,s);
				//tokenData[posToken]=0;
				setTokenData(posToken, (varType)((uint16_t)0),s);
				Lock=0;
			}

			if (strCmp(pracName, "local")){
				tokenDMSG("Token set, type LOCAL", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				//tokenType[posToken]=16;
				setTokenType(posToken, 32,s);
				//tokenData[posToken]=0;
				setTokenData(posToken, (varType)((uint16_t)0),s);
				Lock=0;
			}

			if (strCmp(pracName, "for")){
				tokenDMSG("Token set, type FOR", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 34,s);
				setTokenData(posToken, (varType)((uint16_t)0),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg0")){
				tokenDMSG("Token set, type ARG - id=0", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)0),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg1")){
				tokenDMSG("Token set, type ARG - id=1", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)1),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg2")){
				tokenDMSG("Token set, type ARG - id=2", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)2),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg3")){
				tokenDMSG("Token set, type ARG - id=3", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)3),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg4")){
				tokenDMSG("Token set, type ARG - id=4", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)4),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg5")){
				tokenDMSG("Token set, type ARG - id=5", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)5),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg6")){
				tokenDMSG("Token set, type ARG - id=6", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)6),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg7")){
				tokenDMSG("Token set, type ARG - id=7", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)7),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg8")){
				tokenDMSG("Token set, type ARG - id=8", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)8),s);
				Lock=0;
			}

			if (strCmp(pracName, "arg9")){
				tokenDMSG("Token set, type ARG - id=9", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 35,s);
				setTokenData(posToken, (varType)((uint16_t)9),s);
				Lock=0;
			}

			if (strCmp(pracName, "and")){
				tokenDMSG("Token set, type AND", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 37,s);
				setTokenData(posToken, (varType)((uint16_t)9),s);
				Lock=0;
			}

			if (strCmp(pracName, "or")){
				tokenDMSG("Token set, type OR", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 38,s);
				setTokenData(posToken, (varType)((uint16_t)9),s);
				Lock=0;
			}

			if (strCmp(pracName, "not")){
				tokenDMSG("Token set, type NOT", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				setTokenType(posToken, 39,s);
				setTokenData(posToken, (varType)((uint16_t)9),s);
				Lock=0;
			}

			//check for constants
			for(x=0;(x<sysConstsNum)&&(Lock==1);x++){
				uint16_t y=0;
				while ((strCmp("end", (char*)(sysConsts[x])[y].name)!=1)&&(Lock==1)){
					if(strCmp(pracName, (char*)(sysConsts[x])[y].name)){
						tokenDMSG("Token set, type SYS CONSTANT", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
						setTokenType(posToken, 0,s);
						setTokenData(posToken, (varType)((sysConsts[x])[y].value),s);
						Lock=0;
					}
					y++;
				}
			}

			//end of constants check
			if (strCmp(pracName, "sys")){
				//tokenType[posToken]=28;
				setTokenType(posToken, 28,s);
				setTokenData(posToken, (varType)((uint16_t)0),s);

				Lock=0;

				//získání názvu interní funkce:
				//filtrace bordelu
				while(1){
					if (isRegChar(tokenInput(posText,0))){
						break;
					}else{
						if (tokenInput(posText,0)!=' '){
							tokenizerErrorPrint("tokenParse: Undefined symbol after sys statement!");
							return 1;
						}
					}
					posText++;
					tokenInput(0,1);
				}
				//získání názvu interní funkce
				pracStrInd=0;
				pracName2[pracStrInd]=tokenInput(posText,0);
				posText++;
				tokenInput(0,1);
				pracStrInd++;
				while(1){
					if (pracStrInd<NAME_LENGTH){
						if (isRegChar(tokenInput(posText,0))||isNumber(tokenInput(posText,0))){
							pracName2[pracStrInd]=tokenInput(posText,0);
						}else{
							break;
						}
						posText++;
						tokenInput(0,1);
						pracStrInd++;
					}else{
						pracName2[NAME_LENGTH-1]=0;
						tokenizerErrorPrint("tokenParse: internal function name too long!");
						return 1;
					}
				}
				// název interní funkce máme v pracName2
				// Zde se dosazují indexy interních funkcí z pole sys name list
				// končícího end

				if (syscallExists(pracName2, s)){ //pokud už máme syscall zarezervovaný
					setTokenData(posToken, (varType)((uint16_t)syscallGetId(pracName2, s)) ,s); //šoupnem ID do dat
					tokenDMSG("Token set, type existing SYSCALL", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				}else{
						//přidáme syscall
						if(s->syscallTableLen<SYSCALL_TABLE_L){

						s->syscallTableLen++;
						for (x=0;x<NAME_LENGTH;x++){
							s->syscallTable[s->syscallTableLen].sysCallName[x]=pracName2[x];
						}
						setTokenData(posToken,(varType) s->syscallTableLen ,s);
						//printf("syscall id: %u, name: %s \n",s->syscallTableLen,s->syscallTable[s->syscallTableLen].sysCallName );
						tokenDMSG("Token set, type new SYSCALL", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
					}else{
						tokenizerErrorPrint("tokenParse: Too many SYS calls in one file!");
						return 1;
					}
				}
				/*
				x=0;
				while((strCmp(sysNameList[x], "end")!=1)){
					if (strCmp(sysNameList[x], pracName2)){
						tokenData[posToken]=x;
						tokenDMSG("Token set, type SYS, function:", posToken, tokenData[posToken],tokenType[posToken], posText);
						tokenDMSG(sysNameList[x], posToken, tokenData[posToken],tokenType[posToken], posText);
					}
					x++;
				}*/


			}

			//definice funkce: kontrola funkční lookup table -> přidání názvu a tokenu do funkční lookup table
			if (strCmp(pracName, "function")){
				//tokenType[posToken]=11;
				setTokenType(posToken, 11,s);
				tokenDMSG("Token set, type FUNCTION, name:", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
				Lock=0;
				//získání názvu funkce:
				//filtrace bordelu
				while(1){
					if (isRegChar(tokenInput(posText,0))){
						break;
					}else{
						if (tokenInput(posText,0)!=' '){
							tokenizerErrorPrint("tokenParse: Undefined symbol after function statement!");
							return 1;
						}
					}
					posText++;
					tokenInput(0,1);
				}
				//získání názvu funkce
				pracStrInd=0;
				pracName2[pracStrInd]=tokenInput(posText,0);
				posText++;
				tokenInput(0,1);
				pracStrInd++;
				while(1){
					if (pracStrInd<NAME_LENGTH){
						if (isRegChar(tokenInput(posText,0))||isNumber(tokenInput(posText,0))){
							pracName2[pracStrInd]=tokenInput(posText,0);
						}else{
							break;
						}
						posText++;
						tokenInput(0,1);
						pracStrInd++;
					}else{
						pracName2[NAME_LENGTH-1]=0;
						tokenizerErrorPrint("tokenParse: function name too long!");
						return 1;
					}
				}
				//název funkce máme v pracName2, zkotrolujem function lookup table
				if (functionExists(pracName2, s)){
					tokenizerErrorPrint("Error: multiple definitions of function!");
					return 1;
				}else{
					//přidáme funkci
					if (s->funcTableLen<FUNCTION_TABLE_L){
						s->funcTableLen++;
						s->funcTable[s->funcTableLen].tokenId=posToken+1; //aby to mířilo rovnou na začátek fce
						for (x=0;x<NAME_LENGTH;x++){
							s->funcTable[s->funcTableLen].fString[x]=pracName2[x];
						}
					}else{
						tokenizerErrorPrint("Error: too many functions!");
						return 1;
					}
				}
				tokenDMSG(s->funcTable[s->funcTableLen].fString, posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
			}

			if (Lock){
			//v prac name máme potencionální volání nebo identifikátor promněné
			// nejprva filtrujeme bordel
			//filtrace bordelu
				while(1){
					if (isRegChar(tokenInput(posText,0))||(tokenInput(posText,0)==0)||(tokenInput(posText,0)=='(')||(tokenInput(posText,0)=='=')||(tokenInput(posText,0)=='<')||(tokenInput(posText,0)=='>')
					||(tokenInput(posText,0)==';')||(tokenInput(posText,0)==',')||(tokenInput(posText,0)=='+')||(tokenInput(posText,0)=='-')||(tokenInput(posText,0)=='*')||(tokenInput(posText,0)=='/')||(tokenInput(posText,0)=='!')||(tokenInput(posText,0)==')')||(tokenInput(posText,0)=='%')||(tokenInput(posText,0)=='\n')){
						break;
					}else{
						if (tokenInput(posText,0)!=' '){
							//printf("symbol: %u (%c):", tokenInput(posText,0), tokenInput(posText,0));
							tokenizerErrorPrint("tokenParse: Undefined symbol after variable statement!");
							return 1;
						}
					}
					posText++;
					tokenInput(0,1);
				}
				if (tokenInput(posText,0)!='('){ //pokud nejde o volání fce
					if (varExists(pracName, s)){ //jedná se o existující promněnou
						//tokenType[posToken]=10; //nastavíme typ
						setTokenType(posToken, 10,s);
						//tokenData[posToken]=varGetId(pracName); //nastavíme id
						setTokenData(posToken, varGetId(pracName, s),s);
						tokenDMSG("Token set, type VAR, name:", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
						tokenDMSG(pracName, posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
					}else{
						s->varTableLen++;
						setTokenType(posToken, 10,s); //nastavíme typ
						setTokenData(posToken,(varType) s->varTableLen,s);
						s->varTable[s->varTableLen].type=0;
						for (x=0;x<NAME_LENGTH;x++){
							s->varTable[s->varTableLen].name[x]=pracName[x];
						}
						tokenDMSG("New variable found!", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
						tokenDMSG("Token set, type VAR, name:", posToken, getTokenData(posToken,s),getTokenType(posToken,s), posText);
						tokenDMSG(pracName, posToken, getTokenData(posToken, s), getTokenType(posToken, s), posText);
					}
				} else {
						// if it is built-in function call and function with that name is not registered
						// we register it as a built-in call, otherwise it will be a normal call
						if (getBuiltInCallId(pracName) && !(functionExists(pracName, s))) {
							setTokenType(posToken, 36, s);
							setTokenData(posToken, (varType)getBuiltInCallId(pracName), s);
							tokenDMSG("Token set, type BUILT-IN CALL, function name:", \
													posToken, getTokenData(posToken, s), \
													getTokenType(posToken, s), \
													posText \
												);
							tokenDMSG(pracName, \
													posToken, \
													getTokenData(posToken,s), \
													getTokenType(posToken,s), \
													posText \
												);
						} else {
							setTokenType(posToken, 17, s);
							s->stringConstMax = s->stringFieldLen;
							setTokenData(posToken, (varType)strNew(pracName, s), s);
							tokenDMSG("Token set, type CALL, function name:",
													posToken, \
													getTokenData(posToken,s), \
													getTokenType(posToken,s), \
													posText \
												);
							tokenDMSG(s->stringField + getTokenData(posToken, s).val_str,
													posToken, \
													getTokenData(posToken, s), \
													getTokenType(posToken, s), \
													posText \
												);
						}

				}
			}
			posToken++;
		}

		//eof
		if (tokenInput(posText,0) == 0) {
			setTokenType(posToken, 255, s);
			tokenDMSG("Token set, type EOF/END", \
									posToken, \
									getTokenData(posToken,s), \
									getTokenType(posToken,s), \
									posText \
								);
			printf("Tokenizer Done, %u of %u tokens used.\n",s->tokenMax, getTokenMax(s) );
			if((brCount2 == 0) && (brCount1 == 0)) {
					return 0;
				} else {
					if (brCount1 < 0) {
						tokenizerErrorPrintNL("tokenParse: Bracket chceck error! Missing \"(\"");
						return 1;
					}

					if (brCount1 > 0) {
						tokenizerErrorPrintNL("tokenParse: Bracket chceck error! Missing \")\"");
						return 1;
					}

					if (brCount2 < 0) {
						tokenizerErrorPrintNL("tokenParse: Bracket chceck error! Missing \"{\"");
						return 1;
					}

					if (brCount2 > 0) {
						tokenizerErrorPrintNL("tokenParse: Bracket chceck error! Missing \"}\"");
						return 1;
					}
				}
		}

		//line level debug
		if (tokenizer_exact_debug==1){
			if(tokenizer_exact_token <= posToken - 1) {
				return 0;
			}
		}

		// update délky programu
		// update script lenght (in tokens)
		s->tokenMax = posToken;

	}

	errSoft("tokenParse: Tokenizer overflow!",s);
	return 1;
}

