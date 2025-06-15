#include "include/ast.h"

int record::curr_idx = -1;
int record::curr_par_idx = -1;
symboltablemap* psymbol_table_map = new symboltablemap();
globaltable* pglobal_table = new globaltable();
///symboltable  symbol_table = symboltable();
//static int returned = 0;
static int count_if_ = 0;
static int count_while = 0;
static std::vector<int> stack_while;
static int count_short = 0;
//static int count_temp = 0;
static int is_global = 1;

/////////////////
//lv8
/////////////////

record CompUnitAST :: Dump(std::stringstream  &cstr) const  {
    cstr 
    <<"decl @getint(): i32"<<"\n"
    <<"decl @getch(): i32"<<"\n"
    <<"decl @getarray(*i32): i32"<<"\n"
    <<"decl @putint(i32)"<<"\n"
    <<"decl @putch(i32)"<<"\n"
    <<"decl @putarray(i32, *i32)"<<"\n"
    <<"decl @starttime()"<<"\n"
    <<"decl @stoptime()"<<"\n"
    << endl;
    //vector<record>* prec = new vector<record>;
    string int_ = "int" ;
    string void_ = "void" ;
    symbol funcsymbol = symbol(_FUN_, int_);
    string ident = "getint";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, int_);
    ident = "getch";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, int_);
    ident = "getarray";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, void_);
    ident = "putint";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, void_);
    ident = "putch";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, void_);
    ident = "putarray";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, void_);
    ident = "starttime";
    pglobal_table -> Add(ident, funcsymbol);

    funcsymbol = symbol(_FUN_, void_);
    ident = "stoptime";
    pglobal_table -> Add(ident, funcsymbol);

    for (auto &dfunc_def : *vfunc_def)
    {
        dfunc_def->Dump(cstr);
    }
   return record();
}

record DFuncDefAST :: Dump(std::stringstream  &cstr) const {
    if(type == 0){
        //funcdef
        is_global = 0;
        //in the func
        record rec = func_def->Dump(cstr);
        is_global = 1;
        return rec;
    }
    else if(type == 1){
        //decl
        is_global = 1;
        record rec = decl->Dump(cstr);
        is_global = 1;
        return rec;
    }
    return record();
}

//
//
//fun @f() {
//%entry:
//  ret
//}
record FuncDefAST :: Dump(std::stringstream  &cstr) const  {
    cstr << "fun @" + ident + "(";
    //add a table map for func params with a type as _VAR_I, use same as var but @xxxx.
    psymbol_table_map->Add();
    
    //params!
    int size = vfunc_param->size();
    vector<record>* prec = new vector<record>;
    for (auto &func_param : *vfunc_param)
    {
        record rec = func_param->Dump(cstr);
        //cstr << "####";
        prec->push_back(rec);
        //add func params in outer symbol map
        //psymbol_table_map->Add_symb(rec.name_par, symbol(rec));
        if(size > 1){
            cstr << ", ";
        }
        size --;
    }

    cstr << ")";
    record typerec= func_type->Dump(cstr);
    if(typerec.type == _VOID_){
        cstr << " ";
    }
    else if(typerec.type == _IMM_){
        cstr << ": i32";
    }
    //store in globtable
    symbol funcsymbol = symbol(_FUN_, typerec.name_par);
    pglobal_table -> Add(ident, funcsymbol);

    cstr << " {\n";
    cstr << "%entry:\n";
    for (auto &rec1 : *prec)
    {
        if(rec1.type == _IMM_){
            string symbstr = psymbol_table_map->Add_symb( rec1.name_par, symbol(_VAR_, 0));
            record rec = record(_REG_, 0);
            cstr << "\t" << symbstr << " = alloc i32\n";
            cstr << "\tstore " << "@" << rec1.name_par << ", " << symbstr << "\n";
        }
        else if(rec1.type == _INIT_PTR_ARR_){
            string symbstr = psymbol_table_map->Add_symb( rec1.name_par, symbol(_PTR_ARR_VAR_, 0, rec1.pdim->size()));
            record rec = record(_REG_, 0);
            cstr << "\t" << symbstr << " = alloc *";
            int constexp_len = rec1.pdim->size();
            if(constexp_len){
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = rec1.pdim->size();
                for(auto& constexp: *rec1.pdim){//[[[1],2],3]
                    cstr << ", " << constexp << "]";
                    constexp_len --;
                }
                constexp_len = rec1.pdim->size();
            }
            else{
                cstr << "i32";
            }
            cstr << "\n\tstore " << "@" << rec1.name_par << ", " << symbstr << "\n";
        }
    }
    record rec2 = block->Dump(cstr);
    if(!rec2.returned){
        if(typerec.name_par == "int"){
            cstr << "\tret 0\n";
        }
        else if(typerec.name_par == "void"){
            cstr << "\tret\n";
        }
    }
    //returned = 0;
    cstr << "}\n";
    psymbol_table_map->Del();

    return rec2;
  }

record FuncFParamAST :: Dump(std::stringstream  &cstr) const  {
    // write down param
    //record rec1 = record(_PAR_, ident, "");
    if(type == 0){
        cstr << "@" << ident;
        record rec = btype -> Dump(cstr);
        if(rec.type == _IMM_){
            cstr << ":" << "i32";
        }
    }
    //int arr[] 和 int arr[][10] 对应的类型分别为 *i32 和 *[i32, 10].
    else if(type == 2){
        cstr << "@" << ident;
        record rec = btype -> Dump(cstr);
        if(rec.type == _IMM_){
            cstr << ": *" << "i32";
        }
    }
    else if(type == 1){
        cstr << "@" << ident;
        record rec = btype -> Dump(cstr);
        if(rec.type == _IMM_){
            cstr << ": *";
            vector<int>* dim_sizes = new vector<int>();
            int total_size = 1;
            reverse(vconst_def->begin(), vconst_def->end());
            int constexp_len = vconst_def->size();
            for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                cstr << '[';
                if(i == constexp_len - 1){
                    cstr << "i32";
                }
            }
            constexp_len = vconst_def->size();
            for(auto& constexp: *vconst_def){//[[[1],2],3]
                record rec = constexp-> Dump(cstr);
                cstr << ", " << rec.idx << "]";
                dim_sizes->push_back(rec.idx);
                //cstr<<"\n"<< rec.idx<< "\n";
                total_size *= rec.idx;
                constexp_len --;
            }
            constexp_len = vconst_def->size();
            return record(ident, type, dim_sizes);
        }
    }
    //rec1.type_par = rec.type_par; 
    return record(ident, type);
}


record FuncTypeAST :: Dump(std::stringstream  &cstr) const  {
    if (type == "int")
    {
        return record(type, 0);
    }
    else if (type == "void")
    {
        return record(type, -1);
    }
    return record();
  }


////////////////////////////////////////////
//Lv5/6/7
////////////////////////////////////////////

record StmtAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){//ret 
        record rec = exp->Dump(cstr);
        cstr << "\tret "+ rec.print() +"\n";
        rec.returned = 1;
        return rec;
    }
    else if(type == 1){//decl lval = exp
        //symbol table store!!!!!!!!!!!
        LValAST* plval;
        plval = dynamic_cast<LValAST*>(lval.get());
        //cstr << "Debug! " << (psymbol_table_map -> topidx) << " "<< (psymbol_table_map -> curr_map ->curr_idx) << endl;
        std::string identname = plval->ident; // + "_" + std::to_string(psymbol_table_map -> topidx);
        //cstr << "Debug name! " << identname << endl;
        //symboltable* symbol_table = psymbol_table_map->curr_map;
        symbol sym = psymbol_table_map->Find_symb(identname);
        if(sym.type == _ARR_VAR_ && plval -> type == 1){
            string curr_symstr = psymbol_table_map->Print_symb(identname);
            record rec_curr = record();
            for(auto& pvexp: *(plval->vexp)){
                record pvrec = pvexp->Dump(cstr); //1?
                rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                curr_symstr = rec_curr.print();
            }
            record rec = exp->Dump(cstr);
            cstr << "\tstore " << rec.print() << ", " << curr_symstr << "//stmtlocal\n";
            return rec;
        }
        else if(sym.type == _PTR_ARR_VAR_ && plval -> type == 1){
            string curr_symstr = psymbol_table_map->Print_symb(identname);
            record rec_curr = record();
            int getptr = 0;
            for(auto& pvexp: *(plval->vexp)){
                if(getptr == 0){
                    record pvrec = pvexp->Dump(cstr); //1?
                    rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = load " << curr_symstr << "\n";
                    curr_symstr = rec_curr.print();
                    rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                    curr_symstr = rec_curr.print();
                    getptr ++;
                    continue;
                }
                record pvrec = pvexp->Dump(cstr); //1?
                rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                curr_symstr = rec_curr.print();
                getptr ++;
            }
            record rec = exp->Dump(cstr);
            cstr << "\tstore " << rec.print() << ", " << curr_symstr << "//stmtlocal\n";
            return rec;
        }
        else if(sym.type != _NONE_){
            //exist!
            record rec = exp->Dump(cstr);
            cstr << "\tstore " << rec.print() << ", " << psymbol_table_map->Print_symb(identname) << "\n";
            return rec;
        }
        else{
            symbol sym = pglobal_table->Find(identname);
            if(sym.type == _ARR_VAR_ && plval -> type == 1){
                string curr_symstr = pglobal_table->Print(identname);
                record rec_curr = record();
                for(auto& pvexp: *(plval->vexp)){
                    record pvrec = pvexp->Dump(cstr); //1?
                    record rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                    curr_symstr = rec_curr.print();
                }
                record rec = exp->Dump(cstr);
                cstr << "\tstore " << rec.print() << ", " << curr_symstr << "//stmtglobal\n";
                return rec;
            }
            else if(sym.type != _NONE_){
                record rec = exp->Dump(cstr);
                cstr << "\tstore " << rec.print() << ", " << pglobal_table->Print(identname) << "\n";
                return rec;
            }
        }
    }
    else if(type == 2){//block
        record rec = block->Dump(cstr);
        return rec;
    }
    else if(type == 3){//exp nothing 
        exp->Dump(cstr);
        return record();
    }
    else if(type == 4){//ret nothing 
        cstr << "\tret \n";
        record rec = record();
        rec.returned = 1;
        return rec;
    }
    else if(type == 5){//nothing 
        return record();
    }
    else if(type == 6){ // if else
        //do exp:
        count_if_ ++;
        int curr_count_if_ = count_if_;
        record rec = exp->Dump(cstr);
        //br exp then else
        cstr << "\tbr " << rec.print() << ", %then" << curr_count_if_ << ", %else" << curr_count_if_ << endl; 
        //%then:
        cstr << "%then"<< curr_count_if_ << ": "<< endl;
        record recif = stmtif->Dump(cstr);
        if(!recif.returned){
            //cstr << recif.returned;
            cstr << "\tjump " << "%end" << curr_count_if_ << endl; 
        }
        //%else:
        cstr << "%else"<< curr_count_if_ << ": " <<endl;
        record recelse = stmtelse->Dump(cstr);
        if(!recelse. returned){
            cstr << "\tjump " << "%end" << curr_count_if_ << endl; 
        }
        record rec1 = record();
        //%end:
        /*  while_rec:

            while_body:
            if :
                returned;
            else:
                break;
            end:

            while_end:
        */
        if(!(recif.returned == 1 && recelse.returned == 1) && ! (recif.returned == -1 && recelse.returned == -1)){
            cstr << "%end"<< curr_count_if_ << ": "  <<endl;
        }
        else{
            rec1.returned = 1;
        }
        return rec1;
    }
    else if(type == 7){ // if
        //do exp:
        count_if_ ++;
        int curr_count_if_ = count_if_;
        record rec = exp->Dump(cstr);
        //br exp then else
        cstr << "\tbr " << rec.print() << ", %then" << curr_count_if_ << ", %end" << curr_count_if_ << endl; 
        //%then:
        cstr << "%then"<< curr_count_if_ << ": "<< endl;
        record recif = stmtif -> Dump(cstr);
        if(!recif.returned){
            cstr << "\tjump " << "%end" << curr_count_if_ << endl; 
        }
        //%else:
        //cstr << "%else"<< count_if_ << ": " <<endl;
        //record recelse = stmtelse->Dump(cstr);
        //if(!recelse. returned){
        //    cstr << "\tjump " << ", %end" << count_if_ << endl; 
        //}
        record rec1 = record();
        //%end:
        //if(! (recif.returned && recelse.returned)){
        cstr << "%end"<< curr_count_if_ << ": "  <<endl;
        //}
        //else{
        //    rec.returned = 1;
        //}
        return rec1;
    }
    else if(type == 8){ // while ( ) {};
        //do exp:
        count_while ++;
        stack_while.push_back(count_while);
        int curr_count_while = count_while;
        cstr << "\tjump " << "%while_rec" << curr_count_while << endl; 
        
        cstr << "%while_rec"<< curr_count_while << ": "<< endl;
        record rec = exp->Dump(cstr);
        //br exp while end
        cstr << "\tbr " << rec.print() << ", %while_body" << curr_count_while << ", %while_end" << curr_count_while << endl; 
        //% while body: stmt_if (actually)
        cstr << "%while_body"<< curr_count_while << ": "<< endl;
        record recif = stmtif->Dump(cstr);
        if(!recif.returned){
            //cstr << recif.returned;
            cstr << "\tjump " << "%while_rec" << curr_count_while << endl; 
        }
        //%while end:
        //no need to have if ret or not
        cstr << "%while_end"<< curr_count_while << ": " <<endl;
        stack_while.pop_back();
        recif.returned = 0;
        return recif;
    }
    else if(type == 9){ // break;
        //jump to while_end, and no ret 
        /* while{
                in = 1 count = 1
                while{
                    in = 2 count = 2
                }
                while{
                    in = 2 count = 3
                }
                in = 1 count = 2
            }
        */
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
    return record();
}


////////////////////////////////////////////
//Lv4
////////////////////////////////////////////


record BlockAST ::Dump(std::stringstream  &cstr) const  {
    psymbol_table_map->Add();
    //startblock
    for(auto& block_item : *vblock_item){
        //the item ret!
        record rec = block_item->Dump(cstr);
        //item is returned no need to do anything more!
        if(rec.returned){
            psymbol_table_map->Del();
            return rec;
        }
    }
    psymbol_table_map->Del();
    return record();
  }

record BlockItemAST ::Dump(std::stringstream  &cstr) const  {
    record rec = record();
    if(type == 0){
        rec = stmt->Dump(cstr);
    }
    else if(type == 1){
        rec = decl->Dump(cstr);
    }
    return rec;
  }

record DeclAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){
        record crec = constdecl -> Dump(cstr);
        return crec;
    }
    else if(type == 1){
        record vrec = vardecl -> Dump(cstr);
        return vrec;
    }
    return record();
}

record ConstDeclAST ::Dump(std::stringstream  &cstr) const  {
    for(auto& constdef: *vconst_def){
        constdef -> Dump(cstr);
    }
    return record();
}

//record BTypeAST :: Dump(std::stringstream  &cstr) const  {
//    if (btype == "int")
//    {
//        cstr << "i32";
//    }
//    else if (btype == "void")
//    {
//        cstr << "void";
//    }
//    return record(_PAR_, "", btype);
//}

/////////////////////////////
//lv9
/////////////////////////////

record ConstDefAST ::Dump(std::stringstream  &cstr) const  {
    //symboltable* symbol_table = psymbol_table_map->curr_map;
    //cstr << "Debug name!" << ident_ <<endl;
    if(type == 0){
        if(is_global == 0){
            if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
                //not exist!
                record rec = const_initval -> Dump(cstr);
                psymbol_table_map -> Add_symb(ident, symbol(_VAL_, rec.idx));
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
            }
        }
        else{
            if(pglobal_table->Find(ident).type == _NONE_){
                //not exist!
                record rec = const_initval -> Dump(cstr);
                pglobal_table -> Add(ident, symbol(_VAL_, rec.idx));
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
            }
        }
    }
    ///array!!!!!!!!!!!!
    ///const int x[1][1][4][5][1][4] = {1,1,4,5,1,4} ~= int x[1][1][4][5][1][4] = {1,1,4,5,1,4}
    ///@arr = alloc [[i32, 3], 2] 
    else if (type == 1){
        if(is_global == 0){
            //local
            if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
                //not exist!?
                std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "\t" << symbstr << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                int total_size = 1;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    total_size *= rec.idx;
                    constexp_len --;
                }
                constexp_len = vconstexp->size();
                cstr << "\n";
                //store !
                record init_rec = const_initval->Dump(cstr);
                vector<pair<int, string>> my_map = translate(init_rec,total_size, dim_sizes);
                reverse(dim_sizes.begin(), dim_sizes.end());
                // 存储初始化值
                for (int i = 0; i < total_size; ++i) {
                    int curr_tot = i;
                    string dest_val = "0";
                    for (const auto& p : my_map) {
                        if (p.first == curr_tot) {
                            dest_val = p.second; // 找到匹配项，返回对应的string
                        }
                    }
                    int curr_size = total_size;
                    curr_tot = i;
                    string curr_str = symbstr;
                    for(int size: dim_sizes){
                        curr_size /= size;
                        int curr_print = curr_tot / curr_size;
                        record identrec = record(_REG_, 0); //some reg
                        cstr << "\t" << identrec.print() << " = getelemptr  " << curr_str << ", " << curr_print << "\n";
                        curr_str = identrec.print();
                        curr_tot = curr_tot % curr_size;
                    }
                    cstr << "\tstore " << dest_val << ", " << curr_str << "\n";
                }
                //
                return record();
                //record rec = const_initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
            }
        }
        else{//TODO!
            if(pglobal_table->Find(ident).type == _NONE_){
                //not exist!?
                pglobal_table -> Add(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "global " << pglobal_table->Print(ident) << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                int total_size = 1;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    total_size *= rec.idx;
                    constexp_len --;
                }
                constexp_len = vconstexp->size();
                cstr << ", ";
                //store !
                record init_rec = const_initval->Dump(cstr);
                vector<pair<int, string>> my_map = translate(init_rec, total_size, dim_sizes);
                reverse(dim_sizes.begin(), dim_sizes.end());
                // 存储初始化值
                for (int i = 0; i < total_size; ++i) {
                    int curr_tot = i;
                    string dest_val = "0";
                    for (const auto& p : my_map) {
                        if (p.first == curr_tot) {
                            dest_val = p.second; // 找到匹配项，返回对应的string
                        }
                    }
                    ////////////////////////////////////////////////
                    int curr_size = total_size;
                    if(i != 0){
                        cstr << ",";
                    }
                    for(int size : dim_sizes){
                        curr_tot = curr_tot % curr_size;
                        if(curr_tot == 0){
                            cstr << "{" ; 
                        }
                        curr_size /= size ;
                    }
                    cstr << dest_val;
                    /////////////////////////////////
                    curr_tot = i + 1;
                    curr_size = total_size;
                    for(int size : dim_sizes){
                        curr_tot = curr_tot % curr_size;
                        if(curr_tot == 0){
                            cstr << "}";
                        }
                        curr_size /= size ;
                    }
                }
                cstr << "\n";
                //
                // 添加到符号表
                return record();
                //record rec = const_initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
            }
        }
    }
    return record();
}

record VarDeclAST ::Dump(std::stringstream  &cstr) const  {
    for(auto& vardef:*vvar_def){
        vardef -> Dump(cstr);
    }
    return record();
}


/*transfer into real code index
here are some examples that useful, I tried to align the symbol with corresponding pos in full array!
1: x[3][3][2] = {  1,3                                                        }
              = {{{1,3},{0,0},{0,0}}, {{0,0},{0,0},{0,0}}, {{0,0},{0,0},{0,0}}}
2: x[3][3][2] = {1,{1,3}}
              = not allowed since aligned
3: x[3][3][2] = {  1,3, {1,3}                                                 }
              = {{{1,3},{1,3},{0,0}}, {{0,0},{0,0},{0,0}}, {{0,0},{0,0},{0,0}}}
4: x[3][3][2] = {  1,3, {1,3},{1,3}                                           }
              = {{{1,3},{1,3},{1,3}}, {{0,0},{0,0},{0,0}}, {{0,0},{0,0},{0,0}}}
5: x[3][3][2] = {{ 1,3             }, { 1,3             }, { 1,3             }}
              = {{{1,3},{0,0},{0,0}}, {{1,3},{0,0},{0,0}}, {{1,3},{0,0},{0,0}}}
6: x[3][3][2] = {  1,3, {1,3},{{1},{3}}                                           }
              = {{{1,3},{1,3},{ 1,  3 }}, {{0,0},{0,0},{0,0}}, {{0,0},{0,0},{0,0}}}
7: x[3][3][2] = {  1,3, {1,3},{1,3} , {{1  },{3  }      }                     }
              = {{{1,3},{1,3},{1,3}}, {{1,0},{3,0},{0,0}}, {{0,0},{0,0},{0,0}}}

 The rule:
 1. The {} represent the biggest aligned position, see 4,5.
 2. BUT when should we add 0?????
 If exist outer , we should add 0!?
 3. While there are case that not allowed add 0s, we trust the testcase are correct.
 
 Thus we have the index map for:
 { 1, 3,  {1, 3},  {1, 3}, {{1    },{3   },  4, 5}}
   0  1   20  21   30  31   400     410     42  43
{{{1, 3}, {1, 3}, {1, 3}}, {{1, 0}, {3, 0}, {4, 5}}, {{0,0},{0,0},{0,0}}}
 the real code index is:
 000 001 010 011  020 021  100 110 120 121
 so we could chose represent it in flaten way!
 dim size: [1][2][3]=>>>[n]*/
vector<pair<int, string>> translate(record init_rec, int total_size, vector<int> dim_sizes){
    vector<pair<int,string>> my_map;
    int curr_indexkv = -1;
    int curr_sizekv = total_size;
    vector<int> prev_first = {};
    stack<int> size_stack;//record dim in case the dim with size 1
    //size_stack.push(0); //first {}
    //int allow_add_0lvlkv = 1;
    
    for(auto it = init_rec.psr_map->begin(); it!=init_rec.psr_map->end(); it++){
        int DEBUG = 1;
        vector<int> indexkv = it->first;
        string recprintkv = it->second;
        if(DEBUG) cerr<< "DEBUG:::" << it->second << ";;";
        for(auto a: indexkv){
            if(DEBUG) cerr<<a;
        }
        cerr<< "\n";
        int sizekv = indexkv.size();
        int alignkv = total_size;
        if(!indexkv.empty() && indexkv.back() == 0){ //need align
            //previous {} ends,
            vector<int> curr_first = indexkv;
            //000       010     02        10     2
            //-3+3      0+1     1+0       0+1   1+0
            //{0}      {0,1}    {0,1}     {0}   {0} //here!
            //{0,1,2} {0,1,2'}  {0,1}   {0,1'}  {0}
            int count_exit = prev_first.size() - indexkv.size();
            if(DEBUG){
                cerr<< "DEBUG:delta size::" << count_exit << "\n";
            }
            while(curr_first.back() == 0 && curr_first.size()){//enter a {}
                count_exit += 1;
                curr_first.pop_back();
                if(DEBUG) cerr<< "DEBUG:pop back::" << curr_first.size() << "\n";
            }
            if(DEBUG) cerr<< "DEBUG:count exit::" << count_exit << "\n";
            if(!size_stack.empty() && count_exit){
                if(DEBUG) cerr<< "DEBUG:size stack::" << size_stack.size() <<": "<< size_stack.top() << "\n";
                curr_sizekv = 1;
                int min_exit = dim_sizes.size();
                for(int i = 0; i < count_exit; i++){
                    if(!size_stack.empty()){
                        min_exit = min(size_stack.top(),min_exit);
                        size_stack.pop();
                    }
                }
                if(DEBUG) cerr<< "DEBUG:size stack::" << size_stack.size() <<": "<< size_stack.top() << "\n";
                //top = 0 prod all 
                //2 3 3
                for(int i = 0; i < dim_sizes.size() - min_exit; i++){
                    curr_sizekv = curr_sizekv * dim_sizes[i];
                }
            }
            else{
                curr_sizekv = 0;//不exit 没必要加0
            }
            if(DEBUG) cerr<< "DEBUG:curr_sizekv::" <<curr_sizekv << "\n";
            if(DEBUG) cerr<< "DEBUG:prev_indexkv::" <<curr_indexkv << "\n";
            if(curr_sizekv){ 
                curr_indexkv = ((curr_indexkv + curr_sizekv) / curr_sizekv) * curr_sizekv; // add zeros
            }
            else {
                curr_indexkv = curr_indexkv + 1;
            }
            if(DEBUG) cerr<< "DEBUG:curr_indexkv::" <<curr_indexkv << "\n";
            
            my_map.push_back(make_pair(curr_indexkv,recprintkv));
            //so this is the next 
            //push the {}s
            //
            //enter a new  { { {} } }!
            //000       010     02        10     2
            //-3+3      0+1     1+0       0+1   1+0
            //{0}      {0,1}    {0,1}     {0}   {0}
            //{0,1,2} {0,1,2'}  {0,1}   {0,1'}  {0} //here!
            curr_first = indexkv;
            if(DEBUG) cerr<< "DEBUG:curr_first::" << curr_first.size() << "\n";
            deque<int> avail;
            int avail_sizekv = 1;
            for(int i = 0; i < dim_sizes.size(); i++){
                avail_sizekv *= dim_sizes[i];
                if(curr_indexkv % avail_sizekv == 0){
                    avail.push_back(dim_sizes.size() - 1 - i);
                }
                if(DEBUG) cerr<< "DEBUG:avail.push_back::" << curr_indexkv  << "%" << avail_sizekv << "?" << dim_sizes.size() - 1 - i << "\n";
            } 
            while(curr_first.back() == 0 && !curr_first.empty()){//enter a {}
                int to_push = 0;
                if(!size_stack.empty()){
                    to_push = size_stack.top();
                    while(!avail.empty()){
                        if(avail.back() > to_push){
                            to_push = avail.back();
                            break;
                        }
                        avail.pop_back();
                    }
                }
                size_stack.push(to_push);
                if(DEBUG) cerr<< "DEBUG:size_stack.push::" << to_push << "\n";
                curr_first.pop_back();
            }
        }
        else if(!indexkv.empty()){
            //previous {} ends,
            vector<int> curr_first = indexkv;
            //000       010     02        10     2
            //-3+3      0+1     1+0       0+1   1+0
            //{0}      {0,1}    {0,1}     {0}   {0} //here!
            //{0,1,2} {0,1,2'}  {0,1}   {0,1'}  {0}
            int count_exit = prev_first.size() - indexkv.size();
            curr_sizekv = 1;
            for(int i = 0; i<count_exit; i++){
                if(!size_stack.empty()){
                  size_stack.pop();
                }
            }
            //top = 0 prod all
            for(int i = dim_sizes.size() - 1; i >= size_stack.top(); i--){
                curr_sizekv *= dim_sizes[i];
            }
            //int increment =  indexkv[indexkv.size() - 1] - prev_first[indexkv.size()- 1];
            curr_indexkv = curr_indexkv += 1; // add zeros
            if(DEBUG) cerr<< "\n" << "DEBUG:curr_indexkv::" <<curr_indexkv << "\n\n";
            my_map.push_back(make_pair(curr_indexkv,recprintkv));
            //so this is the next 
            //no push the {}s
        }
        prev_first = indexkv;
    }
    return my_map;
}



record VarDefAST ::Dump(std::stringstream  &cstr) const  {
    //std::string ident_ = ident;// + "_" + std::to_string(psymbol_table_map -> topidx);    
    //cstr << "Debug name!" << ident_ <<endl;
    if(is_global == 0){
        if(type == 0){//ident
            std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_VAR_, 0));//initialize!
            cstr << "\t" << symbstr << " = alloc i32\n";
        }
        else if(type == 1){
            if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
                //not exist!
                record rec = initval -> Dump(cstr);
                std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_VAR_, rec.idx));
                cstr << "\t" << symbstr << " = alloc i32\n";
                cstr << "\tstore " << rec.print() << ", " << symbstr << "\n";
            }
        }
        else if(type == 3){
            //local
            if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
                //not exist!?
                std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "\t" << symbstr << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                int total_size = 1;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    total_size *= rec.idx;
                    constexp_len --;
                }
                constexp_len = vconstexp->size();
                cstr << "\n";
                //store !
                record init_rec = initval->Dump(cstr);
                vector<pair<int, string>> my_map = translate(init_rec,total_size, dim_sizes);
                reverse(dim_sizes.begin(), dim_sizes.end());
                // 存储初始化值
                for (int i = 0; i < total_size; ++i) {
                    int curr_tot = i;
                    string dest_val = "0";
                    for (const auto& p : my_map) {
                        if (p.first == curr_tot) {
                            dest_val = p.second; // 找到匹配项，返回对应的string
                        }
                    }
                    int curr_size = total_size;
                    curr_tot = i;
                    string curr_str = symbstr;
                    for(int size: dim_sizes){
                        curr_size /= size;
                        int curr_print = curr_tot / curr_size;
                        record identrec = record(_REG_, 0); //some reg
                        cstr << "\t" << identrec.print() << " = getelemptr  " << curr_str << ", " << curr_print << "\n";
                        curr_str = identrec.print();
                        curr_tot = curr_tot % curr_size;
                    }
                    cstr << "\tstore " << dest_val << ", " << curr_str << "\n";
                }
                //
                return record();
                //record rec = initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
            }
        }
        else if(type == 2){
            if(psymbol_table_map->Find_symb(ident).type == _NONE_){
                //not exist!?
                psymbol_table_map -> Add_symb(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "\t" << psymbol_table_map->Print_symb(ident) << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    constexp_len --;
                }
                cstr << "\n";
                //
                // 添加到符号表
                return record();
                //record rec = const_initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
                }
            }
    }
    else{
        if(type == 0){//ident
            pglobal_table -> Add(ident, symbol(_VAR_, 0));//initialize!
            cstr << "global " << pglobal_table -> Print(ident) << " = alloc i32, zeroinit\n";
        }
        else if(type == 1){
            if(pglobal_table->Find(ident).type == _NONE_){
                //not exist!
                record rec = initval -> Dump(cstr);
                pglobal_table -> Add(ident, symbol(_VAR_, rec.idx));
                cstr << "global " << pglobal_table -> Print(ident) << " = alloc i32, " << rec.print() << "\n";
            }
        }
        else if(type == 3){
            if(pglobal_table->Find(ident).type == _NONE_){
                //not exist!?
                pglobal_table -> Add(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "global " << pglobal_table->Print(ident) << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                int total_size = 1;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    total_size *= rec.idx;
                    constexp_len --;
                }
                constexp_len = vconstexp->size();
                cstr << ", ";
                //store !
                record init_rec = initval->Dump(cstr);
                vector<pair<int, string>> my_map = translate(init_rec,total_size, dim_sizes);
                reverse(dim_sizes.begin(), dim_sizes.end());
                // 存储初始化值
                for (int i = 0; i < total_size; ++i) {
                    int curr_tot = i;
                    string dest_val = "0";
                    for (const auto& p : my_map) {
                        if (p.first == curr_tot) {
                            dest_val = p.second; // 找到匹配项，返回对应的string
                        }
                    }
                    ////////////////////////////////////////////////
                    int curr_size = total_size;
                    if(i != 0){
                        cstr << ",";
                    }
                    for(int size : dim_sizes){
                        curr_tot = curr_tot % curr_size;
                        if(curr_tot == 0){
                            cstr << "{" ; 
                        }
                        curr_size /= size ;
                    }
                    cstr << dest_val;
                    /////////////////////////////////
                    curr_tot = i + 1;
                    curr_size = total_size;
                    for(int size : dim_sizes){
                        curr_tot = curr_tot % curr_size;
                        if(curr_tot == 0){
                            cstr << "}";
                        }
                        curr_size /= size ;
                    }
                }
                cstr << "\n";
                //
                // 添加到符号表
                return record();
                //record rec = const_initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
                }
            }
        else if(type == 2){
            if(pglobal_table->Find(ident).type == _NONE_){
                //not exist!?
                pglobal_table -> Add(ident, symbol(_ARR_VAR_, 0, vconstexp->size()));
                cstr << "global " << pglobal_table->Print(ident) << " = alloc";
                //[1][2][3]
                //{3,2,1}
                vector<int> dim_sizes;
                reverse(vconstexp->begin(), vconstexp->end());
                int constexp_len = vconstexp->size();
                for(int i = 0; i < constexp_len; i++){//[[[1],2],3]
                    cstr << '[';
                    if(i == constexp_len - 1){
                        cstr << "i32";
                    }
                }
                constexp_len = vconstexp->size();
                for(auto& constexp: *vconstexp){//[[[1],2],3]
                    record rec = constexp-> Dump(cstr);
                    cstr << ", " << rec.idx << "]";
                    dim_sizes.push_back(rec.idx);
                    //cstr<<"\n"<< rec.idx<< "\n";
                    constexp_len --;
                }
                cstr << ", zeroinit\n";
                //
                // 添加到符号表
                return record();
                //record rec = const_initval -> Dump(cstr);
                //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
                }
            }
        }
    return record();
}


void proc(vector<pair<vector<int>,string>>& result_map, vector<unique_ptr<BaseAST>>* vconst_initval, vector<int> index_vect, std::stringstream  &cstr){
    int idx = 0;
    //item is a ConstInitValAST
    for(auto& item1 : *vconst_initval){
        auto* item = dynamic_cast<ConstInitValAST*>(item1.get());
        vector<int>* curr_vect = new vector<int>(index_vect);
        curr_vect->push_back(idx);
        if(item->type == 0){
            record rec = item -> const_exp -> Dump(cstr);
            result_map.push_back(pair<vector<int>, string>(*curr_vect, rec.print()));
        }
        else if(item->type == 1){
            proc(result_map, item -> vconst_initval.get(), *curr_vect, cstr);
        }
        idx ++;
    }
}




record ConstInitValAST ::Dump(std::stringstream  &cstr) const  {
    if (type == 0){
        return const_exp -> Dump(cstr);
    }
    else if(type == 1){
        vector<pair<vector<int>,string>>* presult_map = new vector<pair<vector<int>,string>>(); 
        vector<int>* index_vect = new vector<int>();
        //next_vconst_initval = make_unique<vector<unique_ptr<BaseAST>>>();
        proc(*presult_map, vconst_initval.get(), *index_vect, cstr);
        return record(presult_map);
    }
    else if(type == 2){
        vector<pair<vector<int>,string>>* presult_map = new vector<pair<vector<int>,string>>(); 
        return record(presult_map);
    }
    return record();
}



void proc1(vector<pair<vector<int>,string>>& result_map, vector<unique_ptr<BaseAST>>* vinitval, vector<int> index_vect, std::stringstream  &cstr){
    int idx = 0;
    //item is a InitValAST
    for(auto& item1 : *vinitval){
        auto* item = dynamic_cast<InitValAST*>(item1.get());
        vector<int>* curr_vect = new vector<int>(index_vect);
        curr_vect->push_back(idx);
        if(item->type == 0){
            record rec = item -> exp -> Dump(cstr);
            result_map.push_back(pair<vector<int>, string>(*curr_vect, rec.print()));
            //cerr << idx <<';'<< rec.print() << "\n";
        }
        else if(item->type == 1){
            proc1(result_map, item -> vinitval.get(), *curr_vect, cstr);
        }
        idx ++;
    }
}



record InitValAST ::Dump(std::stringstream  &cstr) const  {
    if (type == 0){
        return exp -> Dump(cstr);
    }
    else if(type == 1){
        //vconst_initval: x[3][3] = {1,3}
        //auto* result_list = new vec8tor<int>();
        vector<pair<vector<int>,string>>* presult_map = new vector<pair<vector<int>,string>>(); 
        //int max_depth = 0; // 记录最大嵌套深度
        vector<int>* index_vect = new vector<int>();
        //cstr << "I'm here!"<<endl; 
        //next_vconst_initval = make_unique<vector<unique_ptr<BaseAST>>>();
        proc1(*presult_map, vinitval.get(), *index_vect, cstr);
        return record(presult_map);
    }
    else if(type == 2){
        vector<pair<vector<int>,string>>* presult_map = new vector<pair<vector<int>,string>>(); 
        return record(presult_map);
    }
    return record();
}

record ConstExpAST ::Dump(std::stringstream  &cstr) const  {
    return exp -> Dump(cstr);
}

record LValAST ::Dump(std::stringstream &cstr) const  {
    //symboltable* symbol_table = psymbol_table_map->curr_map;
    if(type == 0){
        symbol identsym = psymbol_table_map->Find_symb(ident);
        int identtype = identsym.type;
        int identval = identsym.val;
        //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
        if(identtype == _NONE_){
            symbol globsym = pglobal_table->Find(ident);
            if(globsym.type == _VAR_){
                record identrec = record(_REG_, 0); //some reg
                cstr << "\t" << identrec.print() << " = load " << pglobal_table->Print(ident) << "\n";
                return identrec;
            }
            else if(globsym.type == _VAL_){
                record identrec = record(_IMM_, globsym.val); //some imm to li
                return identrec;
            }
            else if(globsym.type == _ARR_VAR_ ){
                record identrec = record(_PTR_, 0); //some imm to li
                cstr << "\t" << identrec.print() << " = getelemptr  " << pglobal_table->Print(ident) << ", 0//lvalglbnonptr" << "\n";
                //cerr<<psymbol_table_map->Print_symb(ident) << " is getted, return" << identrec.print() << endl;
                return identrec;
            }
            return record();
        }
        else if(identtype == _VAR_){
            record identrec = record(_REG_, 0); //some reg
            cstr << "\t" << identrec.print() << " = load " << psymbol_table_map->Print_symb(ident) << "\n";
            return identrec;
        }
        else if(identtype == _VAL_){
            record identrec = record(_IMM_, identval); //some imm to li
            return identrec;
        }
        else if(identtype == _ARR_VAR_ ){
            record identrec = record(_PTR_, 0); //some imm to li
            cstr << "\t" << identrec.print() << " = getelemptr " << psymbol_table_map->Print_symb(ident) << ", 0//lvallocnonptr" << "\n";
            //cerr<<psymbol_table_map->Print_symb(ident) << " is getted, return" << identrec.print() << endl;
            return identrec;
        } 
        else if (identtype == _PTR_ARR_VAR_){
            record identrec = record(_PTR_, 0); //some imm to li
            record rec_curr = record(_REG_, 0);
            string curr_symstr = psymbol_table_map->Print_symb(ident);
            cstr << "\t" << rec_curr.print() << " = load " << curr_symstr << "\n";
            curr_symstr = rec_curr.print();
            cstr << "\t" << identrec.print() << " = getptr " << curr_symstr << ", 0//lvallocnonptr" << "\n";
            cerr<<psymbol_table_map->Print_symb(ident) << " is getted, return" << identrec.print() << endl;
            return identrec;
        }
        return record();
    }
    else if(type == 1){//a[1][1][4][5][1][4];
        symbol identsym = psymbol_table_map->Find_symb(ident);
        int identtype = identsym.type;
        if(identtype == _NONE_){
            symbol globsym = pglobal_table->Find(ident);
            if(globsym.type == _ARR_VAR_ && globsym.dim == vexp->size()){
                string curr_symstr = pglobal_table->Print(ident);
                for(auto& exp: *vexp){
                    record rec = exp->Dump(cstr); //1?
                    record rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< rec.print() << "\n";
                    curr_symstr = rec_curr.print();
                }
                record identrec = record(_REG_, 0); //some reg
                cstr << "\t" << identrec.print() << " = load " << curr_symstr << "\n";
                return identrec;
            }
            else if(globsym.type == _ARR_VAR_ && globsym.dim > vexp->size()){
                //its a pointer
                string curr_symstr = pglobal_table->Print(ident);
                for(auto& exp: *vexp){
                    record rec = exp->Dump(cstr); //1?
                    record rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< rec.print() << "\n";
                    curr_symstr = rec_curr.print();
                }
                record identrec = record(_PTR_, 0); //some imm to li
                cstr << "\t" << identrec.print() << " = getelemptr  " << curr_symstr << ", 0//lvalglbptr" << "\n";
                return identrec;
            }
            return record();
        }
        else if(identtype == _ARR_VAR_ && identsym.dim == vexp->size()){
            string curr_symstr = psymbol_table_map->Print_symb(ident);
            for(auto& exp: *vexp){
                record rec = exp->Dump(cstr); //1?
                record rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< rec.print() << "\n";
                curr_symstr = rec_curr.print();
            }
            record identrec = record(_REG_, 0); //some reg
            cstr << "\t" << identrec.print() << " = load " << curr_symstr << "\n";
            return identrec;
        }
        else if(identtype == _ARR_VAR_ && identsym.dim > vexp->size()){
            string curr_symstr = psymbol_table_map->Print_symb(ident);
            for(auto& exp: *vexp){
                record rec = exp->Dump(cstr); //1?
                record rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< rec.print() << "\n";
                curr_symstr = rec_curr.print();
            }
            record identrec = record(_PTR_, 0); //some imm to li
            cstr << "\t" << identrec.print() << " = getelemptr  " << curr_symstr << ", 0//lvallocptr" << "\n";
            return identrec;
        }
        else if(identtype == _PTR_ARR_VAR_ && identsym.dim + 1 == vexp->size()){
            string curr_symstr = psymbol_table_map->Print_symb(ident);
            int getptr = 0;
            for(auto& pvexp: *vexp){
                if(getptr == 0){
                    record pvrec = pvexp->Dump(cstr); //1?
                    record rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = load " << curr_symstr << "\n";
                    curr_symstr = rec_curr.print();
                    rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                    curr_symstr = rec_curr.print();
                    getptr ++;
                    continue;
                }
                record pvrec = pvexp->Dump(cstr); //1?
                record rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                curr_symstr = rec_curr.print();
                getptr ++;
            }
            record identrec = record(_REG_, 0); //some reg
            cstr << "\t" << identrec.print() << " = load " << curr_symstr << "\n";
            return identrec;
        }
        else if(identtype == _PTR_ARR_VAR_ && identsym.dim + 1 > vexp->size()){
            string curr_symstr = psymbol_table_map->Print_symb(ident);
            int getptr = 0;
            for(auto& pvexp: *vexp){
                if(getptr == 0){
                    record pvrec = pvexp->Dump(cstr); //1?
                    record rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = load " << curr_symstr << "\n";
                    curr_symstr = rec_curr.print();
                    rec_curr = record(_REG_, 0);
                    cstr << "\t" << rec_curr.print() << " = getptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                    curr_symstr = rec_curr.print();
                    getptr ++;
                    continue;
                }
                record pvrec = pvexp->Dump(cstr); //1?
                record rec_curr = record(_REG_, 0);
                cstr << "\t" << rec_curr.print() << " = getelemptr " << curr_symstr << ", "<< pvrec.print() << "\n";
                curr_symstr = rec_curr.print();
                getptr ++;
            }
            record identrec = record(_PTR_, 0); //some imm to li
            cstr << "\t" << identrec.print() << " = getelemptr  " << curr_symstr << ", 0//lvallocptrptr" << "\n";
            return identrec;
        }
        return record();
    }
    return record();
}


////////////////////////////////////////////
//Lv3
////////////////////////////////////////////



record ExpAST ::Dump(std::stringstream  &cstr) const  {
    return  loexp->Dump(cstr);
}

record UExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //P
       return  pexp->Dump(cstr);
    }
    else if(type == 1){//UOP U
        record rhtrec = uexp->Dump(cstr);
        if(rhtrec.type == _IMM_){
            if(uop == "+"){
                return rhtrec;
            }
            else if(uop == "-"){
                rhtrec.idx = -rhtrec.idx;
                return rhtrec;
            }
            else if(uop == "!"){
                rhtrec.idx = !(rhtrec.idx);
                return rhtrec;
            }
        }
        else{
            record rec1 = record(_REG_, -1);
            if(uop == "+"){
                cstr << "\t" << rec1.print() << " = add 0," << rhtrec.print() << "\n";
            }
            else if(uop == "-"){
                cstr << "\t" << rec1.print() << " = sub 0," << rhtrec.print() << "\n";
            }
            else if(uop == "!"){
                cstr << "\t" << rec1.print() << " = eq 0, " << rhtrec.print() << "\n";
            }
            return rec1;
        }
    }
    else if(type == 2 || type == 3){
        //find the func, must in global;
        symbol sym = pglobal_table -> Find(ident);
        deque<record>* prec = new deque<record>();
        for (auto &func_param : *vfunc_param)
            {
                record rec = func_param->Dump(cstr);
                prec -> push_back(rec);
                //add func params in outer symbol map
            }
        cerr << ident << " is called"<<endl;
        if(sym.functype == "int"){
            //xx = f(a, b, c); => %xx =  call @f(%a, %b, %c)
            record rec1 = record(_REG_, -1);
            cstr << "\t" << rec1.print() << " = call @" << ident << "(";
            //no ub, so lets do not check the legth of params 
            //lets input!
            int size = prec -> size();
            //cstr << size ;
            for (auto &rec : *prec)
            {
                //add func params in outer symbol map
                cstr << rec.print();
                if(size > 1){
                    cstr << ",";
                }
                size --;
            }
            cstr << ")\n";
            return rec1;
        }
        else if(sym.functype == "void"){
            //xx = f(a, b, c); => %xx =  call @f(%a, %b, %c)
            cstr << "\t" << "call @" << ident << "(";
            //no ub, so lets do not check the legth of params 
            //but the sym have no use here
            //lets input!
            int size = prec -> size();
            //cstr << size ;
            for (auto &rec : *prec)
            {
                //add func params in outer symbol map
                cstr << rec.print();
                if(size > 1){
                    cstr << ",";
                }
                size --;
            }
            cstr << ")\n";
            return record();
        }
    }
    return record();
}

record PExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //(exp)
        //dump_prev
        //ret dump
        return exp->Dump(cstr);
    }
    else if(type == 1){//number
        //UExp->Dump_prev
        //%1 = Op 0, UnitaryExp->dump
        //ret dump
        return record(_IMM_, number);
    }
    else if(type == 2){
        return lval -> Dump(cstr);
    }
    return record();
  }

record MExpAST ::Dump(std::stringstream &cstr) const  {
    if(type == 0){ //M
       return  uexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record lftrec = mexp->Dump(cstr);
        record rhtrec = uexp->Dump(cstr);
        if (lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            if(mop == "*"){
                return record( _IMM_ , lftrec.idx*rhtrec.idx);
            }
            else if(mop == "/"){
                return record( _IMM_ , lftrec.idx/rhtrec.idx);
            }
            else if(mop == "%"){
                return record( _IMM_ , lftrec.idx%rhtrec.idx);
            }
        }
        else {
            record rec1 = record(_REG_, -1);
            if(mop == "*"){
                cstr << "\t" << rec1.print() << " = mul " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(mop == "/"){
                cstr << "\t" << rec1.print() << " = div " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(mop == "%"){
                cstr << "\t" << rec1.print() << " = mod " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            return  rec1;
        }
    }
    return record();
}


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

record LOExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //LA
        return  laexp->Dump(cstr);
    }
    else if(type == 1){//LO LO LA
    //loop = or
        record lftrec = loexp->Dump(cstr);
        //if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
        //    //imm
        //    if(loop == "||"){
        //        return record( _IMM_ , (lftrec.idx || rhtrec.idx));
        //    }
        //}
        if(lftrec.type == _IMM_ ){
            //imm
            if(loop == "||" && lftrec.idx){
                return record( _IMM_ , 1);
            }
            else if (loop == "||"){
                record rhtrec = laexp->Dump(cstr);
                if(rhtrec.type != _IMM_){
                    record rec1 = record(_REG_, -1);
                    cstr << "\t" << rec1.print() << " = ne " << rhtrec.print()   << ", 0\n";
                    return rec1;
                }
                else{
                    return record( _IMM_ , (lftrec.idx || rhtrec.idx));
                }
            }
        }
        else{
            if(loop == "||"){
                count_short ++;
                int curr_count_short = count_short;
                record rec1 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = ne " << lftrec.print()   << ", 0\n";
                //if && = 0 then no need to right
                std::string symbstr = psymbol_table_map -> Add_symb("srt" + std::to_string(curr_count_short), symbol(_VAR_, 0));
                cstr << "\t" << symbstr << " = alloc i32\n";
                cstr << "\tstore " << rec1.print() << ", " << symbstr << "\n";
                //rec1 == 1 then just short_end as 0 else = 0 short_rht
                cstr << "\tbr " << rec1.print() << ", %short_end" << curr_count_short << ", %short_rht" << curr_count_short << endl; 

                cstr << "%short_rht" << curr_count_short  << ":" << endl;
                record rhtrec = laexp->Dump(cstr);
                record rec2 = record(_REG_, -1);
                record rec3 = record(_REG_, -1);
                cstr << "\t" << rec2.print() << " = ne " << rhtrec.print()   << ", 0\n";
                cstr << "\t" << rec3.print() << " = or "<< rec1.print()     << ", " << rec2.print() << "\n";
                cstr << "\tstore " << rec3.print() << ", " << symbstr << "\n";
                cstr << "\tjump " << "%short_end" << curr_count_short << endl; 

                cstr << "%short_end" << curr_count_short  << ":" << endl;            
                record rec4 = record(_REG_, -1);
                cstr << "\t" << rec4.print() << " = load "<< symbstr<< "\n";
                return rec4;
            }
        }
    }
    return record();
}

record LAExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //LA
        return  eexp->Dump(cstr);
    }
    else if(type == 1){//LA LA E
    //loop = or
        record lftrec = laexp->Dump(cstr);
        ///no need to calc rht!!!!!!!!
        //if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
        //    //imm
        //    if(laop == "&&"){
        //        return record( _IMM_ , (lftrec.idx && rhtrec.idx));
        //    }
        //}
        if(lftrec.type == _IMM_ ){
            //imm
            if(laop == "&&" && lftrec.idx == 0){
                return record( _IMM_ , 0);
            }
            else if (laop == "&&"){
                record rhtrec = eexp->Dump(cstr);
                if(rhtrec.type == _IMM_){
                    return record( _IMM_ , (lftrec.idx && rhtrec.idx));
                }
                else{
                    record rec1 = record(_REG_, -1);
                    cstr << "\t" << rec1.print() << " = ne " << rhtrec.print()   << ", 0\n";
                    return rec1;
                }
            }
        }
        else if(laop == "&&"){
            count_short ++;
            int curr_count_short = count_short;
            record rec1 = record(_REG_, -1);
            cstr << "\t" << rec1.print() << " = ne " << lftrec.print()   << ", 0\n";
            //if && = 0 then no need to right
            std::string symbstr = psymbol_table_map -> Add_symb("srt" + std::to_string(curr_count_short), symbol(_VAR_, 0));
            cstr << "\t" << symbstr << " = alloc i32\n";
            cstr << "\tstore " << rec1.print() << ", " << symbstr << "\n";
            //rec1 == 0 then just short_end as 0 else = 1 short_rht
            cstr << "\tbr " << rec1.print() << ", %short_rht" << curr_count_short << ", %short_end" << curr_count_short << endl; 

            cstr << "%short_rht" << curr_count_short  << ":" << endl;
            record rhtrec = eexp->Dump(cstr);
            record rec2 = record(_REG_, -1);
            record rec3 = record(_REG_, -1);
            cstr << "\t" << rec2.print() << " = ne " << rhtrec.print()   << ", 0\n";
            cstr << "\t" << rec3.print() << " = and "<< rec1.print()     << ", " << rec2.print() << "\n";
            cstr << "\tstore " << rec3.print() << ", " << symbstr << "\n";
            cstr << "\tjump " << "%short_end" << curr_count_short << endl; 
            
            cstr << "%short_end" << curr_count_short  << ":" << endl;            
            record rec4 = record(_REG_, -1);
            cstr << "\t" << rec4.print() << " = load "<< symbstr << "\n";
            return  rec4;
        }
    }
    return record();
}

record EExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //LA
        return  rexp->Dump(cstr);
    }
    else if(type == 1){//LO LO LA
    //loop = or
        record lftrec = eexp->Dump(cstr);
        record rhtrec = rexp->Dump(cstr);
        if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            if(eop == "=="){
                return record( _IMM_ , (lftrec.idx == rhtrec.idx));
            }
            if(eop == "!="){
                return record( _IMM_ , (lftrec.idx != rhtrec.idx));
            }
        }
        else{
            record rec1 = record(_REG_, -1);
            if(eop == "=="){
                cstr << "\t" << rec1.print() << " = eq " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(eop == "!="){
                cstr << "\t" << rec1.print() << " = ne " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            return  rec1;
        }
    }
    return record();
}

record RExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //M
       return  aexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record lftrec = rexp->Dump(cstr);
        record rhtrec = aexp->Dump(cstr);
        if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            if(rop == ">="){
                return record( _IMM_ , (lftrec.idx >= rhtrec.idx));
            }
            if(rop == "<="){
                return record( _IMM_ , (lftrec.idx <= rhtrec.idx));
            }
            if(rop == ">"){
                return record( _IMM_ , (lftrec.idx > rhtrec.idx));
            }
            if(rop == "<"){
                return record( _IMM_ , (lftrec.idx < rhtrec.idx));
            }
        }
        else{
            record rec1 = record(_REG_, -1);
            if(rop == ">="){
                cstr << "\t" << rec1.print() << " = ge " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(rop == "<="){
                cstr << "\t" << rec1.print() << " = le " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(rop == "<"){
                cstr << "\t" << rec1.print() << " = lt " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            else if(rop == ">"){
                cstr << "\t" << rec1.print() << " = gt " << lftrec.print() << ", " << rhtrec.print() << "\n";
            }
            return  rec1;
        }
    }
    return record();
}
