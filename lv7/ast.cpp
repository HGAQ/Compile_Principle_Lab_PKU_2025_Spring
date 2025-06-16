#include "include/ast.h"

int record::curr_idx = -1;
symboltablemap* psymbol_table_map = new symboltablemap();
///symboltable  symbol_table = symboltable();
//static int returned = 0;
static int count_if = 0;
static int count_while = 0;
static std::vector<int> stack_while;
static int count_short = 0;

record CompUnitAST :: Dump(std::stringstream  &cstr) const  {
   record rec = func_def->Dump(cstr);
   return rec;
}

record FuncDefAST :: Dump(std::stringstream  &cstr) const  {
    cstr << "fun @" + ident + "(): ";
    func_type->Dump(cstr);
    cstr << " {\n";
    cstr << "%entry:\n";
    record rec2 = block->Dump(cstr);
    if(!rec2.returned){
        cstr << "\tret 0\n";
    }
    //returned = 0;
    cstr << "}\n";
    return rec2;
  }


record FuncTypeAST :: Dump(std::stringstream  &cstr) const  {
    if (type == "int")
    {
        cstr << "i32";
    }
    else if (type == "void")
    {
        cstr << "void";
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
    else if(type == 1){//decl
        //symbol table store!!!!!!!!!!!
        //TODO!
        LValAST* plval;
        plval = dynamic_cast<LValAST*>(lval.get());
        //cstr << "Debug! " << (psymbol_table_map -> topidx) << " "<< (psymbol_table_map -> curr_map ->curr_idx) << endl;
        std::string identname = plval->ident; // + "_" + std::to_string(psymbol_table_map -> topidx);
        //cstr << "Debug name! " << identname << endl;
        //symboltable* symbol_table = psymbol_table_map->curr_map;
        symbol sym = psymbol_table_map->Find_symb(identname);
        if(sym.type != _NONE_){
            //exist!
            record rec = exp->Dump(cstr);
            cstr << "\tstore " << rec.print() << ", @" << psymbol_table_map->Print_symb(identname) << "\n";
            return rec;
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
        count_if ++;
        int curr_count_if = count_if;
        record rec = exp->Dump(cstr);
        //br exp then else
        cstr << "\tbr " << rec.print() << ", %then" << curr_count_if << ", %else" << curr_count_if << endl; 
        //%then:
        cstr << "%then"<< curr_count_if << ": "<< endl;
        record recif = stmtif->Dump(cstr);
        if(!recif.returned){
            //cstr << recif.returned;
            cstr << "\tjump " << "%end" << curr_count_if << endl; 
        }
        //%else:
        cstr << "%else"<< curr_count_if << ": " <<endl;
        record recelse = stmtelse->Dump(cstr);
        if(!recelse. returned){
            cstr << "\tjump " << "%end" << curr_count_if << endl; 
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
            cstr << "%end"<< curr_count_if << ": "  <<endl;
        }
        else{
            rec1.returned = 1;
        }
        return rec1;
    }
    else if(type == 7){ // if
        //do exp:
        count_if ++;
        int curr_count_if = count_if;
        record rec = exp->Dump(cstr);
        //br exp then else
        cstr << "\tbr " << rec.print() << ", %then" << curr_count_if << ", %end" << curr_count_if << endl; 
        //%then:
        cstr << "%then"<< curr_count_if << ": "<< endl;
        record recif = stmtif -> Dump(cstr);
        if(!recif.returned){
            cstr << "\tjump " << "%end" << curr_count_if << endl; 
        }
        //%else:
        //cstr << "%else"<< count_if << ": " <<endl;
        //record recelse = stmtelse->Dump(cstr);
        //if(!recelse. returned){
        //    cstr << "\tjump " << ", %end" << count_if << endl; 
        //}
        record rec1 = record();
        //%end:
        //if(! (recif.returned && recelse.returned)){
        cstr << "%end"<< curr_count_if << ": "  <<endl;
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

record BTypeAST :: Dump(std::stringstream  &cstr) const  {
    if (btype == "int")
    {
        cstr << "i32";
    }
    else if (btype == "void")
    {
        cstr << "void";
    }
    return record();
}

record ConstDefAST ::Dump(std::stringstream  &cstr) const  {
    //symboltable* symbol_table = psymbol_table_map->curr_map;
    //cstr << "Debug name!" << ident_ <<endl;
    if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
        //not exist!
        record rec = const_initval -> Dump(cstr);
        psymbol_table_map -> Add_symb(ident, symbol(_VAL_, rec.idx));
        //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
    }
    return record();
}

record VarDeclAST ::Dump(std::stringstream  &cstr) const  {
    for(auto& vardef:*vvar_def){
        vardef -> Dump(cstr);
    }
    return record();
}

record VarDefAST ::Dump(std::stringstream  &cstr) const  {
    //std::string ident_ = ident;// + "_" + std::to_string(psymbol_table_map -> topidx);    
    //cstr << "Debug name!" << ident_ <<endl;
    if(type == 0){//ident
        std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_VAR_, 0));//initialize!
        cstr << "\t@" << symbstr << " = alloc i32\n";
    }
    else if(type == 1){
        if(psymbol_table_map->Find_curr_symb(ident).type == _NONE_){
            //not exist!
            record rec = initval -> Dump(cstr);
            std::string symbstr = psymbol_table_map -> Add_symb(ident, symbol(_VAR_, rec.idx));
            cstr << "\t@" << symbstr << " = alloc i32\n";
            cstr << "\tstore " << rec.print() << ", @" << symbstr << "\n";
        }
    }
    return record();
}

record ConstInitValAST ::Dump(std::stringstream  &cstr) const  {
    return const_exp -> Dump(cstr);
}

record InitValAST ::Dump(std::stringstream  &cstr) const  {
    return exp -> Dump(cstr);
}

record ConstExpAST ::Dump(std::stringstream  &cstr) const  {
    return exp -> Dump(cstr);
}

record LValAST ::Dump(std::stringstream &cstr) const  {
    //symboltable* symbol_table = psymbol_table_map->curr_map;
    symbol identsym = psymbol_table_map->Find_symb(ident);
    int identtype = identsym.type;
    int identval = identsym.val;
    //cstr<<"Debug name !"<< psymbol_table_map->Print_symb(ident) <<endl;
    if(identtype ==  _NONE_){
        return record();
    }
    else if(identtype == _VAR_){
        record identrec = record(_REG_, 0); //some reg
        cstr << "\t" << identrec.print() << " = load @" << psymbol_table_map->Print_symb(ident) << "\n";
        return identrec;
    }
    else if(identtype == _VAL_){
        record identrec = record(_IMM_, identval); //some imm to li
        return identrec;
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

record LAExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //LA
        return  eexp->Dump(cstr);
    }
    else if(type == 1){//LO LO LA
    //loop = or
        record lftrec = laexp->Dump(cstr);
        record rhtrec = eexp->Dump(cstr);
        if(lftrec.type == _IMM_ && rhtrec.type == _IMM_){
            //imm
            if(laop == "&&"){
                return record( _IMM_ , (lftrec.idx && rhtrec.idx));
            }
        }
        if(lftrec.type == _IMM_ ){
            //imm
            if(laop == "&&" && lftrec.idx == 0){
                return record( _IMM_ , 0);
            }
            else if (laop == "&&"){
                record rec1 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = ne " << rhtrec.print()   << ", 0\n";
                return rec1;
            }
        }
        else if(laop == "&&"){
            record rec1 = record(_REG_, -1);
            cstr << "\t" << rec1.print() << " = ne " << lftrec.print()   << ", 0\n";
            //if && = 0 then no need to right
            std::string symbstr = psymbol_table_map -> Add_symb("srt" + std::to_string(count_short), symbol(_VAR_, 0));
            cstr << "\t@" << symbstr << " = alloc i32\n";
            cstr << "\tstore " << rec1.print() << ", @" << symbstr << "\n";
            cstr << "\tbr " << rec1.print() << ", %short_rht" << count_short << ", %short_end" << count_short << endl; 

            cstr << "%short_rht" << count_short  << ":" << endl;
            record rec2 = record(_REG_, -1);
            record rec3 = record(_REG_, -1);
            cstr << "\t" << rec2.print() << " = ne " << rhtrec.print()   << ", 0\n";
            cstr << "\t" << rec3.print() << " = and "<< rec1.print()     << ", " << rec2.print() << "\n";
            cstr << "\tstore " << rec3.print() << ", @" << symbstr << "\n";
            cstr << "\tjump " << "%short_end" << count_short << endl; 
            
            cstr << "%short_end" << count_short  << ":" << endl;            
            record rec4 = record(_REG_, -1);
            cstr << "\t" << rec4.print() << " = load @"<< symbstr << "\n";
            count_short ++;
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


