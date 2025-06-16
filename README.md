# 编译原理课程实践


## 一、编译器概述

由于时间比较紧张, 所以到目前为止, 编译器只实现了`Lv1`-`Lv8`。
`lv9`正在进行中，但是进展缓慢。目前可以完成一些KoopaIR的测试点。

### 1.1 基本功能

本编译器基本具备如下功能：

- 词法分析：将源代码文本转换为符号流(Tokens)，识别关键字、标识符、运算符等基本语言元素。
- 语法分析：基于上下文无关文法构建抽象语法树(AST)，验证程序的语法正确性。
- 中间代码生成：将AST转换为机器无关的中间表示(IR)， 使用Koopa IR作为机器无关的中间表示。
- RISC-V代码生成：通过指令选择和寄存器分配，将Koopa IR换为可执行的RISC-V汇编代码。

## 二、编译器设计

### 2.1 主要模块组成

编译器一共分为4个模块，由不同文件来支持不同模块的功能。

- 词法分析器`sysy.l`：负责将源代码转换为token流
- 语法分析器`sysy.y`：负责构建AST
- 中间代码生成器`ast.cpp`：负责生成Koopa IR
- 目标代码生成器`risc.cpp`：负责生成RISC-V汇编

本lab的具体结构为：

```bash
src
├─ast.cpp
├─risc.cpp
├─sysy.y
├─sysy.l
└─include
  ├─ast.h
  └─risc.h
```

### 2.2 主要数据结构与设计

#### 2.2.1 中间代码生成


生成抽象语法树中我们建立了一系列节点来表示不同的语法结构，其形式与文档中类似，所有节点类都继承自基类

```c
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual record Dump(std::stringstream &cstr) const  = 0;
};
```

而其他子类中则包含各种在`sysy.y`翻译中需要的结构，我们以最复杂的`StmtAST`为例
```c
class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> stmtif;
  std::unique_ptr<BaseAST> stmtelse;
  int type;
  record Dump(std::stringstream &cstr)const override;
};
```

对于语法中含有`|`的语句，我们需要在`sysy.y`引入一个`type`来区分不同翻译的种类
，也方便我们后续操作。以`InitVal`为例

```c
InitVal
  : Exp{
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    ast -> type = 0;
    $$ = ast;
  }
  | '{' VInitVal '}'{
    auto ast = new InitValAST();
    ast -> vinitval = unique_ptr<vector<unique_ptr<BaseAST>>>($2);
    ast -> type = 1;
    $$ = ast;
  }
  | '{' '}'{
    auto ast = new InitValAST();
    ast -> type = 2;
    $$ = ast;
  }
  ;
```

此外对于语法中含有`{}`的语句，这表示可能含有多个相同的语句，我们在子类中加入类型为`unique_ptr<vector<unique_ptr<BaseAST>>>($2)`的变量，可以将任意多个语句包含其中并且可以逐一操作。以`VInitval`为例，

```c
VInitVal
  : InitVal{
    auto vect = new vector<unique_ptr<BaseAST>>();
    vect-> push_back(unique_ptr<BaseAST>($1));
    $$ = vect;
  }
  | VInitVal ',' InitVal{
    auto vect = $1;
    vect -> push_back(unique_ptr<BaseAST>($3));
    $$ = vect;
  }
;
```

对于语法中含有`[]`的语句，这表示可能含有一个语句，也可能没有。这相当于为子类中加入两种类型，比如`Stmt = [Exp] ";"`相当于`Stmt = Exp ";"| ";"`，因此不必引入新的数据结构来处理这种语法。

我们生成的语法与文档类似，但是在实现中会发现有二义性问题。

第一个问题是if...else...的二义性问题，对于

```c
Stmt = "if" "(" Exp ")" Stmt ["else" Stmt]
```

得到

```c
Stmt = "if" "(" Exp ")" Stmt | "if" "(" Exp ")" Stmt "else" Stmt
```

就会导致经典的Dangling-else问题。因此我们可以通过

```c
Stmt = "if" "(" Exp ")" Stmt | "if" "(" Exp ")" Stmt "else" StmtIf
```

而StmtIf是一个与Stmt完全相同的类，只是没有语法为`"if" "(" Exp ")" Stmt "else" StmtIf`的语句。因此我们确保了所有else都能够与最近的未匹配的if来匹配。

另一个问题是类型的二义性问题，较为迷惑地，在文档中表示类型的语法中包含

```c
FuncType      ::= "void" | "int";
BType         ::= "int";
```

这两者混用会导致二义性问题的发生，我们将这两者保留其一，使用`FuncType`来代替`Btype`从而可以简单地解决。

#### 2.2.2 综合属性传递的设计考虑

对于生成Koopa IR的函数`record Dump(std::stringstream &cstr)`我们通过`record`类来传递综合属性至上一节点。比如`Stmt := "return" Exp ;`

```c
record StmtAST ::Dump(std::stringstream  &cstr) const {
    if(type == 0){//ret 
        record rec = exp->Dump(cstr);
        cstr << "\tret "+ rec.print() +"\n";
        rec.returned = 1;
        return rec;
    }
    ...
}
```

可以看出我们可以通过`Exp`返回的`rec`来传递子节点的综合属性。record不仅可以传递立即数类型的结果，还能传递临时符号、数组符号、函数名等结果，目前record结构为

```c
class record
{
  public:
    static int curr_idx;
    int idx;
    int type;
    int returned;
    string name_par; //i'm lazy too, it sometimes represent type as well
    unordered_map<string, int> init_map;
    unordered_map<string, string>* pstr_map;
}
;
```

我们省略了构造函数和一些未使用的变量，其中`curr_index`是静态的，来代表下一个临时符号的record应当为什么标号。`idx`在立即数类型下为立即数的数值，在临时符号中为record的标号。`type`为这个record的类型，而`name_par`、`init_map`、`pstr_map` ，分别用于函数名、常数组以及变量数组的初始化中。`returned`代表是否已经返回，以避免一个基本块中出现了两个`ret`语句。

#### 2.2.3 符号表的设计考虑

在Koopa IR还需要储存符号表，我们使用多个层级的字典来保存符号表与符号表的层级结构。不同符号表使用curridx来区分，符号表中使用，在局域符号表外还有一个全局符号表，记录着全局变量，常量，数组，函数等信息。在输出中，局域符号表中的名称可能会发生重复。因此我们加上了`_loc_`的前缀以及`_p + curridx`的后缀。而全局的变量与常量符号使用`_glb_`的前缀加以区分。

```c
class symbol{
  public:
    int type;
    int val;
    string functype;
};
class symboltable{
  public:
    std::unordered_map<std::string, symbol> map;
    int curr_idx ;
    symbol Find(std::string sym){
        //寻找symbol
      }
    void Add(std::string str, symbol sym){
        //增加symbol
    }
    std::string Print(std::string str){
        //输出symbol
    }
};
class globaltable{
  public:
    std::unordered_map<std::string, symbol> map;
    symbol Find(std::string sym){
        //寻找symbol
      }
    void Add(std::string str, symbol sym){
        //增加symbol
    }
    std::string Print(std::string str){
        //输出symbol
    }
};
class symboltablemap{
  public:
    std::vector<symboltable*> vect_map;
    symboltable* curr_map;
    int topidx;// curr_map's idx
    int nextidx;// next map idx - 1
    void Add(){
        //增加symbol表
    }
    void Del(){
        //删除符号表
    }
    symbol Find_curr_symb(std::string find_string){
        //在当前符号表中，寻找symbol
      }
    std::string Add_symb(std::string str, symbol add_symb){
        //在当前符号表中，增加symbol
    }
    symbol Find_symb(std::string find_string){
        //在全部符号表中寻找symbol
    }
    std::string Print_symb(std::string find_string){
      //输出符号表
    }
};
```

对于寻找符号，应当先由近至远地寻找局域符号，然后再寻找全局符号。在整个程序中我们使用唯一的全局符号表与局域符号表字典来，全部的符号操作都是在这两个表中进行。

#### 2.2.4 目标代码生成与寄存器

我们用`reg`类来记录生成riscv代码时的寄存器分配与全局变量储存情况。特别地，对于栈寄存器，我们还需要`stack`类来保存函数调用时的栈分配。全局变量还需要一个`global`类来储存。

```c
class stack{
    public:
        unordered_map<koopa_raw_value_t, int> stack_offset_table;
        int stack_len;//stack_len % 8=0!
        int curr_stack_offset;
};
class global{
    public:
        unordered_map<koopa_raw_value_t, int> global_table;
        static int global_idx;
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
    unordered_map<std::string, stack*> stack_table;
    global* pglobal_val;
    std::string curr_stk;
};
```

在`reg`类中我们可以通过reg_avail来判断栈的可用情况，set时可以分配一个任意的为存储的寄存器，也可以指定寄存器来分配（通常用于函数调用）。在`stack`类中我们使用`curr_stack_offset`来确定下一次存入栈的偏移量，用`stack_len`来记录当前栈的长度。对于多级的调用会产生不同的栈指针，在`reg`类里，使用`stack_table`来记录不同层级调用栈指针的情况。

## 三、编译器实现

### 3.1 各阶段编码细节

#### Lv1. main函数 和 Lv2. 初试目标代码生成

基于文档，我们可以较容易地完成前两个难度，最困难的地方在于下载docker时经常会出现网络问题。在`ast.cpp`里，我们使用cout来输出我们的Koopa IR结果，而在riscv中将同样的字符串保存。

#### Lv3. 表达式

若我们使用Lv2的做法会使得结构过于冗长，对于需要传递的综合属性我们使用字符串传递，并将输出函数分为两部分`Dump`与`Dump_prev`来分别输出与传递参数的符号。不仅debug难以进行，而且扩展性差。通过结合往届经验，并结合自己的代码，我最后采用stringstream来存储字符串，将所有输出函数合并，通过前文中record类来传递综合属性。

此外还要考虑优先级的关系，但是在文档中已经给出了正确优先级的语法，根据那个就可以构造。对于riscv的生成，此时还没有考虑栈上的变量，因此一定要及时reset已经使用完毕的寄存器，或者通过Lv4中的栈上储存寄存器的方法得到最终的结果。

```c
void Visit(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value)
{
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
    ...
}
```

#### Lv4. 常量和变量

由于添加了变量，我们需要构造前文的符号表`symbol`与`symboltable`，并且要注意常量的定义与使用。以一个加法为例，

```c
record AExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //M
       return  mexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record lftrec = aexp->Dump(cstr);
        record rhtrec = mexp->Dump(cstr);
        if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            if(aop == "+"){
                return record( _IMM_ , lftrec.idx+rhtrec.idx);
            }
            else if(aop == "-"){
                return record( _IMM_ , lftrec.idx-rhtrec.idx);
            }
        }
        else{
            record rec1 = record(_REG_, -1);
            if(aop == "+"){
                cstr << "\t" << rec1.print() << " = add " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(aop == "-"){
                cstr << "\t" << rec1.print() << " = sub " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            return rec1;
        }
    }
    return record();
}
```

对两个变量的类型进行分类讨论。此外对于在riscv中变量的储存问题，我们统一采用栈上储存。因此在将中间代码翻译为riscv指令中，integer的`Visit`变得没有必要，而是选择在各个指令的层面进行翻译。然后还需要增加变量运算的`load`与`sw`指令，对于`load`指令，我们有

```c
// %0 = load @k
// load sth from stack to reg!! 
void Visit(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{   //we have value of load
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    std::string curr_reg = REG.reg_set(value, false); // allocate a reg for val of load
    int stack_offset = curr_stack->val2offset(load.src); // get offset of the value in stack
    Print_lw(stack_offset, curr_reg);//do: load 4B in offset to reg  
    stack_offset = curr_stack->stk_val_set(value);//store the value in stack!
    Print_sw(stack_offset, curr_reg);//do: store 4B in reg to offset   
    REG.reg_reset(REG.name2idx(curr_reg)); // deallocate a reg for val of load
}
```

对于一个变量，可以先`lw`存在一个临时寄存器中，然后用`sw`再放在栈上。对store和运算都是同理的。但是这些操作就要涉及需要分配多少栈空间。

```c
    std::cout << "\t.globl " << func->name + 1 << std::endl;
    std::cout << func->name + 1 << ":" << std::endl;
    int stack_num = 0;
    for (int i = 0; i < func->bbs.len; ++i)
    {
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        stack_num += bb->insts.len * 4;
    }
    int stack_num_ = (stack_num + 15) / 16 * 16; 
    stack stk = REG.stk_set(func->name, stack_num_);
    Print_addi(stack_num_); //do addi sp, sp, !-stack_num_; 
    Visit(func->bbs);
```

我们在这里采用的策略是有多少指令就使用多少栈空间，虽然可能会浪费，但是只多不少。最后要注意栈空间要对16B对齐。

#### Lv5. 语句块和作用域

由于添加了作用域，`symboltablemap`的实现也是必要的。注意不同的`symbolmap`应当有不同的`curr_idx`，查找符号应当由内至外，离开符号表应当及时在`symboltablemap`中删除。因此维护好`symboltablemap`非常重要，比如下面的例子，

```bash
int main(){
    int a = 0;
    a = a + 1;
    { 
        a = a + 1;
        int a = 0;
        a = a + 1;
        {
            a = a + 1;
            int a = 0;
            a = a + 1;
        }
        a = a + 1;
        int a = 0;
        a = a + 1;
    }
    {
        a = a + 1;
        int a = 0;
        a = a + 1;
    }
    return a;
}
```

特别地，要在返回值中记录是否返回不再翻译后面的语句。对于类型为`int`的函数要`ret 0`，对于类型为`void`的函数要直接`ret`。

#### Lv6. if语句

关于二义性的讨论前面已经充分阐述，对于每一个if语句，使用一个全局的`count_if`来确定这个语句的index，避免重名。此外还要注意若所有可能的分支都已经return，那么我们就没必要再return。

if语句的计算还涉及短路求值的问题，以`||`为例，

```c
record LOExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //LA
        return  laexp->Dump(cstr);
    }
    else if(type == 1){//LO LO LA
    //loop = or
        record lftrec = loexp->Dump(cstr);
        record rhtrec = laexp->Dump(cstr);
        if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            //imm
            if(loop == "||"){
                return record( _IMM_ , (lftrec.idx || rhtrec.idx));
            }
        }
        if(lftrec.type == _IMM_ ){
            //imm
            if(loop == "||" && lftrec.idx){
                return record( _IMM_ , 1);
            }
            else if (loop == "||"){
                record rec1 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = ne " << rhtrec.print()   << ", 1\n";
                return rec1;
            }
        }
        else{
            if(loop == "||"){
                record rec1 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = ne " << lftrec.print()   << ", 0\n";
                //if && = 0 then no need to right
                std::string symbstr = psymbol_table_map -> Add_symb("srt" + std::to_string(count_short), symbol(_VAR_, 0));
                cstr << "\t@" << symbstr << " = alloc i32\n";
                cstr << "\tstore " << rec1.print() << ", @" << symbstr << "\n";
                cstr << "\tbr " << rec1.print() << ", %short_end" << count_short << ", %short_rht" << count_short << endl; 

                cstr << "%short_rht" << count_short  << ":" << endl;
                record rec2 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = or " << lftrec.print() << ", " << rhtrec.print() << "\n";
                cstr << "\t" << rec2.print() << " = ne " << rec1.print()   << ", 0\n";
                record rec3 = record(_REG_, -1);
                cstr << "\t" << rec2.print() << " = ne " << rhtrec.print()   << ", 0\n";
                cstr << "\t" << rec3.print() << " = or "<< rec1.print()     << ", " << rec2.print() << "\n";
                cstr << "\tstore " << rec3.print() << ", @" << symbstr << "\n";
                cstr << "\tjump " << "%short_end" << count_short << endl; 

                cstr << "%short_end" << count_short  << ":" << endl;            
                record rec4 = record(_REG_, -1);
                cstr << "\t" << rec4.print() << " = load @"<< symbstr<< "\n";
                count_short ++;
                return rec4;
            }
        }
    }
    return record();
}
```

逻辑运算的短路求值依旧是分类讨论运算符前后值的类型，并从左向右计算，若一定为某值就直接返回。若两侧都是变量，那么我们应当使用分支跳转，若结果确定就跳转到语句的结尾，否则再计算另一侧。使用一个全局的`count_short`来确定这个短路求值的index，避免重名。此外这样操作的结果值一定要储存在内存中，因为后文无法知道前文如何将操作值储存的位置。在riscv中，需要增加的是分支语句的翻译，

```c
void Visit(const koopa_raw_jump_t &jump)
{
    std::cout << "\tj " << jump.target->name + 1 << std::endl;
}


void Visit(const koopa_raw_branch_t &branch, const koopa_raw_value_t &value)
{
    stack* curr_stack = REG.name2stk(REG.curr_stk); // get curr stack
    std::string curr_reg = REG.reg_set(value, false); // allocate a reg for val of br
    if (branch.cond->kind.tag == KOOPA_RVT_INTEGER){   
        //keep it in reg
        std::cout << "\tli " << curr_reg << ", " << branch.cond->kind.data.integer.value << std::endl;
    }
    else{
        //save it from stack to reg
        int stack_offset = curr_stack->val2offset(branch.cond); // get offset of the value in stack
        Print_lw(stack_offset, curr_reg); //do: load 4B in offset to reg  
    }
    std::cout << "\tbnez " << curr_reg  << ", " <<  branch.true_bb->name + 1 << std::endl;
    REG.reg_reset(REG.name2idx(curr_reg)); // deallocate a reg for val of load
    std::cout << "\tj " << branch.false_bb->name + 1 << std::endl;
}
```
#### Lv7. while语句

while 语句与 if 语句类似，主要难点在于如何正确处理break和continue语句的跳转目标，我们在这里除了加入一个`count_while`，确定index问题，还要维护一个stack来记录上层`count_while`，可以通过这种方法确定break和continue跳转的对象。

```c
 else if(type == 9){ // break;
        int curr_count_while = stack_while.back();
        if (!stack_while.empty()){
            cstr << "\tjump " << "%while_end" << curr_count_while << endl; 
            record rec = record();
            rec.returned = -1;
            return rec;
        }
    }
    else if(type == 10){ // continue;
        //jump to while_end, and no ret 
        int curr_count_while = stack_while.back();
        if (!stack_while.empty()){
            cstr << "\tjump " << "%while_rec" << curr_count_while << endl; 
            record rec = record();
            rec.returned = -1;
            return rec;
        }
    }
```

#### Lv8. 函数和全局变量

在这里我们要加入`globaltable`相关的数据结构，同时对`FuncDef`相关的语法进行修改，使其能够包含多个函数，并相互调用。

在riscv实现时需要注意，进入函数时在栈顶保存ra寄存器，退出函数时恢复ra寄存器。在计算栈帧大小时，需要额外加1用于存储ra。栈帧布局时要在栈顶预留空间给函数参数，避免被局部变量覆盖。若参数超过8个，我们采用栈帧顶部作为函数参数，避免被局部变量覆盖。因此生成函数的栈帧也发生了变化，

```c
int stack_num = 0;
    int add_len_arg = 0;
    for (int i = 0; i < func->bbs.len; ++i)
    {
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[i]);
        for (size_t j = 0; j < bb->insts.len; ++j)
        {
            auto inst = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            if (inst->kind.tag == KOOPA_RVT_CALL)
            {
                int args = inst->kind.data.call.args.len;
                add_len_arg = std::max(add_len_arg, args - 8);
            }
        }
        stack_num += (bb->insts.len )* 4;
    }
    stack_num += 4 + add_len_arg * 4;
    int stack_num_ = (stack_num + 15) / 16 * 16; 
    stack stk = REG.stk_set(func->name, stack_num_, add_len_arg * 4);//
    Print_addi(stack_num_); //do addi sp, sp, !-stack_num_; 
    Print_sw(stack_num_ - 4 , "ra");
    Visit(func->bbs);
```

#### Lv9. 数组

（实现中）

### 3.3 测试情况说明（如果进行过额外的测试，可增加此部分内容）

首先尝试的是已有的测试样例，对照github仓库进行比对，模拟运行。
若是不在测试样例仓库中的错误，可以在一些大学和组织的编译原理竞赛github与gitlab仓库中寻找一些测试样例，将docker中的测试样例替换，

```bash
cp ~/c/hello.c /opt/bin/testcases/lv8/00_int_func.c 
```

查看结果是否会与示例一致。最后对于像数组这样的抽象语法，会参考[Compiler Explore](https://godbolt.org/)得到各种样例与结果。

其中我利用以上方法总结了一些比文档更为清晰的数组语法规则：

```
1: x[3][3][2] = {1, 3}
              = {{{1,3},{0,0},{0,0}},
                 {{0,0},{0,0},{0,0}},
                 {{0,0},{0,0},{0,0}}}
2: x[3][3][2] = {1, {1,3}}
              = not allowed since aligned
3: x[3][3][2] = {1, 3, {1,3}}
              = {{{1,3},{1,3},{0,0}},
                 {{0,0},{0,0},{0,0}}, 
                 {{0,0},{0,0},{0,0}}}
4: x[3][3][2] = {1, 3, {1,3},{1,3}}
              = {{{1,3},{1,3},{1,3}},
                 {{0,0},{0,0},{0,0}},
                 {{0,0},{0,0},{0,0}}}
5: x[3][3][2] = {{1,3}, {1,3}, {1,3}}
              = {{{1,3},{0,0},{0,0}},
                 {{1,3},{0,0},{0,0}},
                 {{1,3},{0,0},{0,0}}}
6: x[3][3][2] = {1,3, {1,3},{{1},{3}}}
              = {{{1,3},{1,3},{1,3}},
                 {{0,0},{0,0},{0,0}},
7: x[3][3][2] = {1, 3, {1,3}, {1,3}, {{1}, {3}}}
              = {{{1,3},{1,3},{1,3}},
                 {{1,0},{3,0},{0,0}},
                 {{0,0},{0,0},{0,0}}}
```

- 在外层{}代表的数组中，新{}与尽可能多的（小于外层的）可对齐的维度对齐。
  - 例子1：{1, 3, {1,3}, {1,3}}
  - 原式子代表整个数组
  - {1,3}与尽可能多的的维度对齐， $2 \% 2 = 0$ 可以得到 [2\]
  - 例子2：{{1,3}, {1,3}, {1,3}}
  - 原式子代表整个数组
  - {1,3}与尽可能多的的维度对齐， $0 \% (2\times3) = 0$ 可以得到 [2\][3\]
- 0仅可以在一个{}的末尾补充。
  - 例子：{1, {1,3}}
  - 1,后不能补充0因为不在{}末尾，有语法错误
- 相信测试样例中没有语法错误。

## 四、实习总结

### 4.1 收获和体会

本次编译器实现过程是对编译原理理论的深度实践，主要收获体现在三方面：

- 系统化工程能力：从词法分析到目标代码生成的全流程实现，深刻理解了编译器各阶段的衔接逻辑。特别是符号表的多层级设计（全局/局部作用域）和综合属性的传递机制，让我掌握了大型系统设计中状态管理的核心方法。

- 编译优化意识：在寄存器分配（Lv4）的实现中，通过对比朴素实现与优化方案的性能差异，认识到对编译器的优化会对最终代码效率产生关键影响。

- 问题定位能力：调试Dangling-else二义性时，通过手动构造最小冲突案例并比对语法规则，最终采用StmtIf分离策略解决。此类经历极大提升了复杂问题的分解能力。

### 4.2 学习难点与建议

- 数组实现复杂度（Lv9）：高维数组初始化对齐规则和内存布局设计远超预期，需同时处理类型系统、存储计算和边界检查
- debug中没有完整的测试样例，定位困难。

改进建议：

- 提供更多中间表示的测试工具（如Koopa IR），加速调试过程。
- 删除lv9，细化文档。
