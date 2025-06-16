#include "risc.h"


reg REG;

// 访问 raw slice
void Visit(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    // 根据 slice 的 kind 决定将 ptr 视作何种元素
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION:
        // 访问函数
        Visit(reinterpret_cast<koopa_raw_function_t>(ptr));
        break;
      case KOOPA_RSIK_BASIC_BLOCK:
        // 访问基本块
        Visit(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
        break;
      case KOOPA_RSIK_VALUE:
        // 访问指令
        Visit(reinterpret_cast<koopa_raw_value_t>(ptr));
        break;
      default:
        // 我们暂时不会遇到其他内容, 于是不对其做任何处理
        assert(false);
    }
  }
}

// 访问 raw program
void Visit(const koopa_raw_program_t &program) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有全局变量
  Visit(program.values);
  std::cout << "\t.text" << std::endl;
  // 访问所有函数
  Visit(program.funcs);
}


// 访问函数
void Visit(const koopa_raw_function_t &func) {
    // 执行一些其他的必要操作
    // ...
    // 访问所有基本块
    std::cout << "\t.globl " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ":" << std::endl;
    int stack_num = 0;
    for (int i = 0; i < func->bbs.len; ++i)
    {
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        stack_num += bb->insts.len * 4;
    }
    /*
       ...
    |___|___|
    |   | 0 |
       ...
    */
    int stack_num_ = (stack_num + 15) / 16 * 16; 
    stack stk = REG.stk_set(func->name, stack_num_);//set a new stack with a length stack_num_
    //cout << "nnd : " << stk.curr_stack_offset << " " << stk.stack_len << " " << REG.curr_stk<< endl;
    //stk = REG.name2stk(REG.curr_stk);
    //cout << "nnd : " << stk.curr_stack_offset << " " << stk.stack_len << endl;
    Print_addi(stack_num_); //do addi sp, sp, !-stack_num_; 
    Visit(func->bbs);
}





// 访问基本块
void Visit(const koopa_raw_basic_block_t &bb) {
  // 执行一些其他的必要操作
  // ...
  // 访问所有指令
  Visit(bb->insts);
}

// 访问指令
void Visit(const koopa_raw_value_t &value) {
  // 根据指令类型判断后续需要如何访问
  const auto &kind = value->kind;
  switch (kind.tag) {
    case KOOPA_RVT_RETURN:
      // 访问 return 指令
      Visit(kind.data.ret);
      break;
    //case KOOPA_RVT_INTEGER:
    //  // 访问 integer 指令
    //  Visit(kind.data.integer, value);
    //  break;
    case KOOPA_RVT_BINARY:
      // 访问 binary 指令
      Visit(kind.data.binary, value);
      break;
    case KOOPA_RVT_ALLOC:

        break;
    case KOOPA_RVT_LOAD:
        // 访问 load 指令
        Visit(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // 访问 store 指令
        Visit(kind.data.store, value);
        break;
    default:
      // 其他类型暂时遇不到
      cerr << kind.tag << endl;
      assert(false);
  }
}

// %0 = load @k
// load sth from stack to reg!! 
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{   //we have value of load
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    //cout << curr_stack->curr_stack_offset <<endl;
    std::string curr_reg = REG.reg_set(value, false); // allocate a reg for val of load
    int stack_offset = curr_stack->val2offset(load.src); // get offset of the value in stack
    Print_lw(stack_offset, curr_reg);//do: load 4B in offset to reg  
    stack_offset = curr_stack->stk_val_set(value);//store the value in stack!
    //curr_stack->val2offset(load.src); // get offset of the value in stack
    Print_sw(stack_offset, curr_reg);//do: store 4B in reg to offset   
    REG.reg_reset(REG.name2idx(curr_reg)); // deallocate a reg for val of load
}


//store %1, @pj
//store sth from reg to stack!!
void Visit(const koopa_raw_store_t &store, const koopa_raw_value_t &value)
{
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    std::string curr_reg = REG.reg_set(value, false); // allocate a reg for val of store
    if (store.value->kind.tag == KOOPA_RVT_INTEGER){   
        //keep it in reg
        std::cout << "\tli " << curr_reg << ", " << store.value->kind.data.integer.value << std::endl;
    }
    else{
        //save it from stack to reg
        int stack_offset = curr_stack->val2offset(store.value); // get offset of the value in stack
        Print_lw(stack_offset, curr_reg); //do: load 4B in offset to reg  
    }
    //store it from reg to stack
    int stack_offset_dest = curr_stack->stk_val_set(store.dest); //store the value in stack!
    // curr_stack->val2offset(store.dest); // get offset of the value in stack
    Print_sw(stack_offset_dest, curr_reg);//do: store 4B in reg to offset   
    REG.reg_reset(REG.name2idx(curr_reg)); // deallocate a reg for val of load
}




// 访问对应类型指令的函数定义略
// 视需求自行实现
void Visit(const koopa_raw_return_t ret)
{
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    //ret 114514
    if (ret.value && ret.value->kind.tag == KOOPA_RVT_INTEGER)
    {
        std::cout << "\tli a0, " << ret.value->kind.data.integer.value << std::endl;
    }
    //ret %a0
    else if(ret.value){
        int stack_offset = curr_stack->val2offset(ret.value);
        Print_lw(stack_offset, "a0");
    }
    else
    {
        std::cout << "\tli a0, 0" << std::endl;
    }
    //recover the stack
    Print_addi(-curr_stack->stack_len);
    std::cout << "\tret" << std::endl;
}

//void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
//{
//    if (integer.value == 0)
//    {
//        REG.reg_set(value, true);
//    }
//    else
//    {
//        int idx = REG.name2idx(REG.reg_set(value, false));
//        std::cout << "\tli " << REG.idx2name(idx) << ", " << integer.value << std::endl;
//    }
//}

void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
    //lhs, rhs now at stack instead of reg
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    std::string lhs_reg = REG.reg_set(binary.lhs, false);//nothing here now ...
    std::string rhs_reg = REG.reg_set(binary.rhs, false);//nothing here now ...
    if (binary.lhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        std::cout << "\tli " << lhs_reg <<", " <<  binary.lhs->kind.data.integer.value << std::endl;
    }
    else
    {
        int stack_offset = curr_stack->val2offset(binary.lhs);
        Print_lw(stack_offset, lhs_reg);
    }
    if (binary.rhs->kind.tag == KOOPA_RVT_INTEGER)
    {
        std::cout << "\tli " << rhs_reg <<", " <<  binary.rhs->kind.data.integer.value << std::endl;
    }
    else
    {
        int stack_offset = curr_stack->val2offset(binary.rhs);
        Print_lw(stack_offset, rhs_reg);
    }
    
    REG.reg_reset(REG.name2idx(lhs_reg));
    REG.reg_reset(REG.name2idx(rhs_reg));

    std::string val = REG.reg_set(value, false);
    //int validx = REG.name2idx(val);

    // 根据二元运算符的类型进行处理
    switch (binary.op)
    {
    case KOOPA_RBO_EQ:
        std::cout << "\txor " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_NOT_EQ:
        std::cout << "\txor " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        std::cout << "\tsnez " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_GT:
        std::cout << "\tsgt " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_LT:
        std::cout << "\tslt " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_GE:
        std::cout << "\tslt " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_LE:
        std::cout << "\tsgt " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_ADD:
        std::cout << "\tadd " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_SUB:
        std::cout << "\tsub " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_MUL:
        std::cout << "\tmul " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_DIV:
        std::cout << "\tdiv " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_MOD:
        std::cout << "\trem " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_AND:
        std::cout << "\tand " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    case KOOPA_RBO_OR:
        std::cout << "\tor " << val << ", " << lhs_reg <<", " << rhs_reg <<std::endl;
        break;
    }
    //store it from reg to stack
    int stack_offset_dest = curr_stack->stk_val_set(value); //store the value in stack!
    // curr_stack->val2offset(value); // get offset of the value in stack
    Print_sw(stack_offset_dest, val);//do: store 4B in reg to offset   
    REG.reg_reset(REG.name2idx(val)); // deallocate a reg for val of load
}



void rsc5(string str){
  // 解析字符串 str, 得到 Koopa IR 程序
  //cerr<<str<<endl;
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(str.c_str(), &program);
  assert(ret == KOOPA_EC_SUCCESS);  // 确保解析时没有出错
  // 创建一个 raw program builder, 用来构建 raw program
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // 将 Koopa IR 程序转换为 raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  
  // 使用 for 循环遍历函数列表
  Visit(raw);

  // 释放 Koopa IR 程序占用的内存
  koopa_delete_program(program);
  // 处理完成, 释放 raw program builder 占用的内存
  // 注意, raw program 中所有的指针指向的内存均为 raw program builder 的内存
  // 所以不要在 raw program 处理完毕之前释放 builder
  koopa_delete_raw_program_builder(builder);
}





void Print_addi(int stack_num_)
{
    if (stack_num_ >= -2048 && stack_num_ < 2048){
        std::cout << "\taddi" << " sp, sp, " << - stack_num_ << std::endl; // arrange stack
    }
    else{
        int idx = REG.name2idx(REG.reg_set( false));
        std::cout << "\tli " << REG.idx2name(idx) << ", " << - stack_num_ << std::endl;
        std::cout << "\taddi" << " sp, sp, " << REG.idx2name(idx) << std::endl;
        REG.reg_reset(idx);
    }
}

void Print_lw(int stack_offset, std::string curr_reg) {
    if (stack_offset >= -2048 && stack_offset < 2048){
        std::cout << "\tlw " << curr_reg << ", " <<  stack_offset  <<"(sp)"  << std::endl;
    }
    else{
        int idx = REG.name2idx(REG.reg_set(false));
        std::cout << "\tli " << REG.idx2name(idx) << ", " << stack_offset << std::endl;
        std::cout << "\tadd " << REG.idx2name(idx) << ", " << REG.idx2name(idx) << ", " << "sp" << std::endl;
        std::cout << "\tlw " << curr_reg << ", " <<  REG.idx2name(idx)  <<"(sp)"  << std::endl;
        REG.reg_reset(idx);
    }
}

void Print_sw(int stack_offset, std::string curr_reg) {
    if (stack_offset >= -2048 && stack_offset < 2048){
        std::cout << "\tsw " << curr_reg << ", " <<  stack_offset  <<"(sp)" << std::endl;
    }
    else{
        int idx = REG.name2idx(REG.reg_set(false));
        std::cout << "\tli " << REG.idx2name(idx) << ", " << stack_offset << std::endl;
        std::cout << "\tadd " << REG.idx2name(idx) << ", " << REG.idx2name(idx) << ", " << "sp" << std::endl;
        std::cout << "\tsw " << curr_reg << ", " <<  REG.idx2name(idx)  <<"(sp)"  << std::endl;
        REG.reg_reset(idx);
    }
}






reg::reg(){
    for(int i = 0; i <= 15; i++){
        reg_avail[i] = false;
        reg_value[i] = 0;
    }
    curr_stk = "";
}
/*
  have value set reg, return name
*/
std::string reg::reg_set(const koopa_raw_value_t &value, bool is_zero)
{
    if(!is_zero){
        for(int i = 0; i < 15; i++){
            if(!reg_avail[i]){
                reg_value[i] = value;
                reg_avail[i] = true;
                return reg_name[i];
            }
        }
    }
    else{
        reg_value[15] = value;
        return "x0";
    }
    std::cerr << "not enough reg!!!!!"<<std::endl;
    return "";
}

std::string reg::reg_set(bool is_zero)
{
    if(!is_zero){
        for(int i = 0; i < 15; i++){
            if(!reg_avail[i]){
                reg_avail[i] = true;
                return reg_name[i];
            }
        }
    }
    else{
        return "x0";
    }
    return "";
}

/*
    reset specified reg, return name!
*/
std::string reg::reg_reset(int index)
{
    if(index>=0 && index<=15){
        reg_value[index] = 0;
        reg_avail[index] = false;
        return reg_name[index];
    }
    else{
        return "";
    }
}


/*
    have idx find name， 16 = x0, UB = “”
*/
std::string reg::idx2name(int index){
    if(index>=0 && index<=15) {
        return reg_name[index];
      }
    std::cerr << "UB!!!" << std::endl;
    return "";
}

/*
    have name find indx， 16 = x0, -1 = not in
*/
int reg::name2idx(std::string index){
    for(int i = 0 ; i <= 15; i ++){
        if(reg_name[i] == index){
            return i;
        }
    }
    //notin
    return -1;
}


/*
    have idx find val, 16 = x0, UB = “”
*/
//koopa_raw_value_t reg::idx2val(int index){
//    if(index>=0 && index<=15) {
//        return reg_value[index];
//      }
//    std::cerr << "UB!!!" << std::endl;
//}


/*
    have val find idx UB = -1
*/
int reg::val2idx(koopa_raw_value_t index){
    for(int i = 0 ; i <= 15; i ++){
        if(reg_value[i] == index){
            return i;
        }
    }
    //notin
    return -1;
}

stack reg::stk_set(std::string name, int stack_len)
{   
    //if (stk.stack_len == -1){
    //    stack stk = stack(stack_len);
    //    stack_table.insert(pair<std::string,stack>(name, stack_len));
    //    return stk;
    //}
    stack stk = stack(stack_len);
    stack_table.insert(pair<std::string, stack*>(name, &stk));
    curr_stk = name;
    return stk;
}


stack* reg::name2stk(std::string sym){
    return stack_table[sym];
  }

//stack stk_val_set(stack stack, const koopa_raw_value_t value){
//    if(stack.curr_stack_offset + 4 <= stack.stack_len)
//    {
//        stack.stack_offset_table.insert(pair<koopa_raw_value_t,int>(value, stack.curr_stack_offset));
//        stack.curr_stack_offset += 4;
//    }
//    return stack;
//}