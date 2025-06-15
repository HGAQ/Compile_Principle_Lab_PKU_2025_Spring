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
    case KOOPA_RVT_INTEGER:
      // 访问 integer 指令
      Visit(kind.data.integer, value);
      break;
    case KOOPA_RVT_BINARY:
      // 访问 binary 指令
      Visit(kind.data.binary, value);
      break;
    default:
      // 其他类型暂时遇不到
      assert(false);
  }
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
void Visit(const koopa_raw_return_t ret)
{
    //ret 114514
    if (ret.value && ret.value->kind.tag == KOOPA_RVT_INTEGER)
    {
        std::cout << "\tli a0, " << ret.value->kind.data.integer.value;
        //Visit(ret.value);
        std::cout << std::endl;
    }
    //ret %a0
    else if(ret.value){
        int idx = REG.val2idx(ret.value);
        if (idx<0){
           Visit(ret.value);
        }
        std::cout << "\tmv a0, " <<REG.idx2list(idx) << std::endl;
    }
    else
    {
        std::cout << "\tli a0, 0" << std::endl;
    }
    std::cout << "\tret" << std::endl;
}

void Visit(const koopa_raw_integer_t &integer, const koopa_raw_value_t &value)
{
    if (integer.value == 0)
    {
        REG.reg_set(value, true);
    }
    else
    {
        int idx = REG.list2idx(REG.reg_set(value, false));
        std::cout << "\tli " << REG.idx2list(idx) << ", " << integer.value << std::endl;
    }
}

void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
    //先 li, nothing = 0
    int lhsidx = REG.val2idx(binary.lhs);
    if (lhsidx<0){
      Visit(binary.lhs);
    }
    lhsidx = REG.val2idx(binary.lhs);

    int rhsidx = REG.val2idx(binary.rhs);
    if (rhsidx<0){
      Visit(binary.rhs);
    }
    rhsidx = REG.val2idx(binary.rhs);

    REG.reg_reset(lhsidx);
    REG.reg_reset(rhsidx);
    std::string vallist = REG.reg_set(value, false);
    int validx = REG.list2idx(vallist);

    std::string lhs = REG.idx2list(lhsidx);
    std::string rhs = REG.idx2list(rhsidx);
    std::string val = REG.idx2list(validx);

    // 根据二元运算符的类型进行处理
    switch (binary.op)
    {
    case KOOPA_RBO_EQ:
        std::cout << "\txor " << val << ", " << lhs << ", " << rhs << std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_NOT_EQ:
        std::cout << "\txor " << val << ", " << lhs << ", " << rhs << std::endl;
        std::cout << "\tsnez " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_GT:
        std::cout << "\tsgt " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_LT:
        std::cout << "\tslt " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_GE:
        std::cout << "\tslt " << val << ", " << lhs << ", " << rhs << std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_LE:
        std::cout << "\tsgt " << val << ", " << lhs << ", " << rhs << std::endl;
        std::cout << "\tseqz " << val << ", " << val << std::endl;
        break;
    case KOOPA_RBO_ADD:
        std::cout << "\tadd " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_SUB:
        std::cout << "\tsub " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_MUL:
        std::cout << "\tmul " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_DIV:
        std::cout << "\tdiv " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_MOD:
        std::cout << "\trem " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_AND:
        std::cout << "\tand " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    case KOOPA_RBO_OR:
        std::cout << "\tor " << val << ", " << lhs << ", " << rhs << std::endl;
        break;
    }
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










reg::reg(){
    for(int i = 0; i <= 15; i++){
        reg_avail[i] = false;
        reg_value[i] = 0;
    }
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
                return reg_list[i];
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

/*
    reset specified reg, return name!
*/
std::string reg::reg_reset(int index)
{
    if(index>=0 && index<=15){
        reg_value[index] = 0;
        reg_avail[index] = false;
        return reg_list[index];
    }
    else{
        return "";
    }
}


/*
    have idx find name， 16 = x0, UB = “”
*/
std::string reg::idx2list(int index){
    if(index>=0 && index<=15) {
        return reg_list[index];
      }
    std::cerr << "UB!!!" << std::endl;
    return "";
}

/*
    have name find indx， 16 = x0, -1 = not in
*/
int reg::list2idx(std::string index){
    for(int i = 0 ; i <= 15; i ++){
        if(reg_list[i] == index){
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

