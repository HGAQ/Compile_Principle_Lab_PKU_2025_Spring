#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "include/ast.h"
#include "koopa.h"

void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb); 
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t ret);
void Visit(const koopa_raw_integer_t &integer);
void rsc5(string str);
