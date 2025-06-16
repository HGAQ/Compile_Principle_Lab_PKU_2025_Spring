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
void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void rsc5(string str);

class reg
{
    const std::string reg_list[16] = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "x0"
    };
    bool reg_avail[16];
    koopa_raw_value_t reg_value[16];
public:
    reg();
    std::string reg_set(const koopa_raw_value_t &value, bool is_zero);
    std::string reg_reset(int index);

    std::string idx2list(int index);
    int list2idx(std::string index);
    //koopa_raw_value_t idx2val(int index);
    int val2idx(koopa_raw_value_t index);
};
