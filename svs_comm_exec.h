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

This is the main command execution logic header.
*/

#ifndef SVS_COMM_EXEC_H
#define SVS_COMM_EXEC_H

#include "svs_basics.h"

void commExDMSG(char *text, uint16_t tokenId, svsVM *s);
uint16_t commSkip(uint16_t index, svsVM *s);
uint16_t commExecLoop(uint16_t index, svsVM *s);
uint16_t commExecById(uint16_t id, svsVM *s);
uint16_t commExecFromStart(svsVM *s);
uint16_t commExec(uint8_t * name, svsVM *s);
void setCommExDebug (uint8_t level);

void exprExec(uint16_t index, varRetVal *result, svsVM *s);

void commArgNull(comExArgs *target);
void commArgCopy(comExArgs* source, comExArgs* target);
uint16_t commParseCall(uint16_t index, svsVM *s);

#endif
