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

This is the garbage collector for strings.
*/

#include "svs_garbage_collector.h"

uint8_t gcDebug=0;


//cgDMSG("text");
void cgDMSG(char *text) {
  if (gcDebug == 1) {
	  printf("gcDMSG: %s \n", text);
	}
}

void setGcDebug(uint8_t level) {
  gcDebug = level;
}

#ifndef CMDLINE
void pscg_garbage_walkaround(uint8_t *strId, uint32_t str_len, uint8_t *max);
uint8_t pscg_garbage_walkaround2(uint8_t *strId);
#endif


uint8_t gcGetValidString(uint16_t strId, svsVM *s) {
  uint16_t x = 0;

  //textové konstanty tam háže tokenizer, tudíž jsou ze začátku textového pole
  //a už se nemění, stačí tedy zjistit kde je poslední konstanta, menší id jsou pak
  //automaticky validní

  if (strId<=s->stringConstMax){
    return 1;
  }

  #ifndef CMDLINE
  if (pscg_garbage_walkaround2( (uint8_t*) ((uint32_t)strId + (uint32_t)(s->stringField)) ) ){
    //printf("dbg: GC: valid: %s\n",strId+s->stringField);
    return 1;
  }
  #endif

  for(x=1;x<=s->varTableLen;x++){
		if (varGetType((varType)x,s)==1){ //pokud je promněnná string
			if (varGetVal((varType)x,s).val_str==strId){
			  return 1; // a má tu správnou hodnotu, vracíme 1
			}
		}
	}

	//array
	for(x = 1;x <= s->varArrayLen; x++) {
		if (s->varArrayType[x] == 1) { //pokud je promněnná string
			if (s->varArray[x].val_str == strId) {
			  return 1; // a má tu správnou hodnotu, vracíme 1
			}
		}
	}

	//pokud jsme došli až sem a nic se nenašlo, vracíme 0

	return 0;
}

uint8_t gcRemoveString(uint16_t strId, uint8_t type, svsVM *s) {
  uint16_t x = 0;
  uint16_t str_len = 0;

  //spočtu délku toho stringu
  x = strId;
  while (s->stringField[x]!=0) {
    x++;
  }

  str_len = x - strId + 1;

  for(x = strId + str_len; x < s->stringFieldLen; x++) {
    s->stringField[x - str_len] = s->stringField[x];
  }

  s->stringFieldLen -= str_len;

  x = 0;

  for(x=1;x<=s->varTableLen;x++){
		if (varGetType((varType)x, s)==1){ //pokud je promněnná string
			if (varGetVal((varType)x, s).val_str>strId){
			  varSetVal((varType)x, (varType)((uint16_t)(varGetVal((varType)x, s).val_str - str_len)), s); //upravíme id
			}
		}
	}

	//array
	for(x = 1; x <= s->varArrayLen; x++) {
		if (s->varArrayType[x] == 1) { //pokud je promněnná string
			if (s->varArray[x].val_str > strId) {
				s->varArray[x].val_str -= str_len;
			}
		}
	}

	#ifndef CMDLINE
    pscg_garbage_walkaround((uint32_t)((uint32_t)strId + (uint32_t)(s->stringField)), str_len, (uint32_t)((uint32_t)s->stringField + (uint32_t)s->stringFieldLen));
  #endif

	return 0;
}

void garbageCollect(uint16_t count, svsVM *s){
  uint16_t x = 0;
  uint8_t valid = 0;
  uint16_t gc_start;

  if ((STRING_FIELD_L*10)/(s->stringFieldLen+1)>11){
    //pokud není volná už jen jedna desetina string fieldu, nebudeme provádět GC
    //cgDMSG("Garbage Collection not needed.");
    if (count==0){
      return;
    }
  }

  if (s->profilerEnabled){
    printf("Profiler: collecting garbage! ");
  }
  gc_start=s->stringFieldLen;

  if (STRING_FIELD_L<s->stringFieldLen){
    //tohle už je chybový stav
    errMsgS("garbageCollect: String field invalid! (STRING_FIELD_L<stringFieldLen)");
  }

  if (0==s->stringFieldLen){
    cgDMSG("Nothing in string field.");
    //v poli stringu nic není
    return;
  }

  //ověříme validitu nultého stringu, páč před ním není nula
  valid=gcGetValidString(0,s);
  if (valid==1){ //pokud byl validní, tak hledáme nějaký další invalidní
    for(x=0;x<s->stringFieldLen-1;x++){
      if (0==s->stringField[x]){ //ukončovací znak předchozího stringu
        valid=gcGetValidString(x+1,s);
        if (0==valid){
          //cgDMSG("Non-valid string removed.");
          //printf("string: %s\n",stringField+x+1 );
          gcRemoveString(x+1, valid,s);

          if (count!=0){
            if (s->stringFieldLen<(gc_start-count)){
              break;
            }
          }
          //printf("string field len:: %u\n",stringFieldLen );
        }
      }
    }
  }
  if (s->profilerEnabled){
    printf(" Collecting Done! %u/%u occupied %u freed.\n",s->stringFieldLen,  STRING_FIELD_L,gc_start-s->stringFieldLen );
  }
  if (valid==1){
    cgDMSG("All strings valid.");
    return; //všechny stringy jsou validní
  }
}