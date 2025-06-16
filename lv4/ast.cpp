#include "include/ast.h"

int record::curr_idx = -1;
symboltable  symbol_table = symboltable();
static int returned = 0;

record CompUnitAST :: Dump(std::stringstream  &cstr) const  {
   record rec = func_def->Dump(cstr);
   return rec;
}

record FuncDefAST :: Dump(std::stringstream  &cstr) const  {
    cstr << "fun @" + ident + "(): ";
    func_type->Dump(cstr);
    cstr << " {\n";
    record rec2 = block->Dump(cstr);
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
//Lv4
////////////////////////////////////////////


record BlockAST ::Dump(std::stringstream  &cstr) const  {
    cstr << "%entry:\n";
    for(auto& block_item : *vblock_item){
        if(!returned){
            block_item->Dump(cstr);
        }
    }
    return record();
  }

record BlockItemAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){
        stmt->Dump(cstr);
    }
    else if(type == 1){
        decl->Dump(cstr);
    }
    return record();
  }

record StmtAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){//ret 
        record rec = exp->Dump(cstr);
        cstr << "\tret "+ rec.print() +"\n";
        returned = 1;
        return rec;
    }
    else if(type == 1){//decl
        //symbol table store!!!!!!!!!!!
        //TODO!
        LValAST* plval;
        plval = dynamic_cast<LValAST*>(lval.get());
        std::string identname = plval->ident;
        symbol sym = symbol_table.Find(identname);
        if(sym.type != _NONE_){
            //exist!
            record rec = exp->Dump(cstr);
            cstr << "\tstore " << rec.print() << ", @" << identname << "\n";
            return rec;
        }
    }
    return record();
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
    if(symbol_table.Find(ident).type == _NONE_){
        //not exist!
        record rec = const_initval -> Dump(cstr);
        symbol_table.Add(ident, symbol(_VAL_, rec.idx));
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
    if(type == 0){//ident
        cstr << "\t@" << ident << " = alloc i32\n";
        symbol_table.Add(ident, symbol(_VAR_, 0));//initialize!
    }
    else if(type == 1){
        if(symbol_table.Find(ident).type == _NONE_){
            //not exist!
            record rec = initval -> Dump(cstr);
            cstr << "\t@" << ident << " = alloc i32\n";
            cstr << "\tstore " << rec.print() << ", @" << ident << "\n";
            symbol_table.Add(ident, symbol(_VAR_, rec.idx));
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

record LValAST ::Dump(std::stringstream  &cstr) const  {

    symbol identsym = symbol_table.Find(ident);
    int identtype = identsym.type;
    int identval = identsym.val;
    if(identtype ==  _NONE_){
        return record();
    }
    else if(identtype == _VAR_){
        record identrec = record(_REG_, 0); //some reg
        cstr << "\t" << identrec.print() << " = load @" << ident << "\n";
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
            if(loop == "||"){
                return record( _IMM_ , (lftrec.idx || rhtrec.idx));
            }
        }
        else{
            if(loop == "||"){
                record rec1 = record(_REG_, -1);
                record rec2 = record(_REG_, -1);
                cstr << "\t" << rec1.print() << " = or " << lftrec.print() << ", " << rhtrec.print() << "\n";
                cstr << "\t" << rec2.print() << " = ne " << rec1.print()   << ", 0\n";
                return  rec2;
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
            if(laop == "&&"){
                return record( _IMM_ , (lftrec.idx && rhtrec.idx));
            }
        }
        else{
            record rec1 = record(_REG_, -1);
            record rec2 = record(_REG_, -1);
            record rec3 = record(_REG_, -1);
            if(laop == "&&"){
                cstr << "\t" << rec1.print() << " = ne " << lftrec.print()   << ", 0\n";
                cstr << "\t" << rec2.print() << " = ne " << rhtrec.print()   << ", 0\n";
                cstr << "\t" << rec3.print() << " = and "<< rec1.print()     << ", " << rec2.print() << "\n";
            }
            return  rec3;
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


