#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include "include/ast.h"
#include "koopa.h"
#include <unordered_map>

class reg;
class Stack;
class global;
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb); 
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t ret);
//void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value);
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value);
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value);
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit(const koopa_raw_jump_t &jump);
void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value);
void Visit(const koopa_raw_global_alloc_t  &global_alloc, const koopa_raw_value_t &value);
void Visit(const koopa_raw_call_t  &call, const koopa_raw_value_t &value);

void rsc5(string str);
void Print_addi(int stack_num_);
void Print_lw(int stack_offset, std::string curr_reg, std::string src_reg = "sp");
void Print_sw(int stack_offset, std::string curr_reg, std::string dest_reg = "sp");

class Stack{
    public:
        unordered_map<koopa_raw_value_t, int> stack_offset_table;
        int stack_len;//%8=0!
        int curr_stack_offset;
        //bad init!
        Stack(){stack_len = -1;}
        //good init!
        Stack(int stack_len_, int curr_stack_offset_){
            stack_len = stack_len_;
            curr_stack_offset = curr_stack_offset_;
            }

        int stk_val_set(const koopa_raw_value_t value){
            //auto it = stack_offset_table.find(value);
            //if (it != stack_offset_table.end()) {
            //    //cout << "repeat";
            //    return stack_offset_table[value];
            //}
            if(stack_offset_table.find(value) == stack_offset_table.end()){
                if(curr_stack_offset + 4 <= stack_len - 4)//del the ra save!
                {
                    stack_offset_table[value] = curr_stack_offset;
                    curr_stack_offset += 4;
                    return curr_stack_offset - 4;
                }
                return -1;
            }
            else{
                return stack_offset_table[value];
            }
        }

        int val2offset(koopa_raw_value_t val){
            return stack_offset_table[val];
        }
};


class global{
    public:
        unordered_map<koopa_raw_value_t, int> global_table;
        static int global_idx;//%8=0!
        //good init!
        global(){global_idx = -1;}

        int glb_val_set(const koopa_raw_value_t value){
            //auto it = stack_offset_table.find(value);
            //if (it != stack_offset_table.end()) {
            //    //cout << "repeat";
            //    return stack_offset_table[value];
            //}
            if(global_table.find(value) == global_table.end()){
                global_idx ++;
                global_table[value] = global_idx;
                return global_idx;
            }
            else{
                return global_table[value];
            }
        }

        int val2idx(koopa_raw_value_t val){
            return global_table[val];
        }
};



class reg
{
    const std::string reg_name[16] = {
        "t0", "t1", "t2", "t3", "t4", "t5", "t6",
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "x0"
    };
    bool reg_avail[16];
    koopa_raw_value_t reg_value[16];
public:
    unordered_map<std::string, Stack*> stack_table;
    global* pglobal_val;
    std::string curr_stk;

    reg();

    std::string reg_set(const koopa_raw_value_t &value, bool is_zero);
    std::string reg_set(bool is_zero);
    std::string reg_set(const koopa_raw_value_t &value, string str);
    std::string reg_reset(int index);
    Stack stk_set(string name, int stack_len, int curr_stack_offset_);

    Stack* name2stk(std::string sym);
    std::string idx2name(int index);
    int name2idx(std::string index);
    //koopa_raw_value_t idx2val(int index);
    int val2idx(koopa_raw_value_t index);
};



