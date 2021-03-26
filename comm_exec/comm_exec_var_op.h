#ifndef COMM_EXEC_NUMERIC_H
#define COMM_EXEC_NUMERIC_H

#include "svs_comm_exec.h"

uint8_t comm_exec_var_op(uint16_t *currToken, svsVM * s);
uint8_t comm_exec_arg_op(uint16_t *token, svsVM * s);

#endif