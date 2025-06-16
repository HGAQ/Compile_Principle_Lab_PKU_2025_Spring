#include "include/ast.h"

int record::curr_idx = -1;

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

record BlockAST ::Dump(std::stringstream  &cstr) const  {
    cstr << "%entry:\n";
    record rec = stmt->Dump(cstr);
    return rec;
  }

////////////////////////////////////////////
//Lv3
////////////////////////////////////////////

record StmtAST ::Dump(std::stringstream  &cstr) const  {
    record rec = exp->Dump(cstr);
    cstr << "\tret "+ rec.print() +"\n";
    return rec;
}

record ExpAST ::Dump(std::stringstream  &cstr) const  {
    return  loexp->Dump(cstr);
}

record UExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //M
       return  pexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record rhtrec = uexp->Dump(cstr);
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
    return record();
  }

record MExpAST ::Dump(std::stringstream &cstr) const  {
    if(type == 0){ //M
       return  uexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record lftrec = mexp->Dump(cstr);
        record rhtrec = uexp->Dump(cstr);
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
    return record();
}


record AExpAST ::Dump(std::stringstream  &cstr) const  {
    if(type == 0){ //M
       return  mexp->Dump(cstr);
    }
    else if(type == 1){//A A M
        record lftrec = aexp->Dump(cstr);
        record rhtrec = mexp->Dump(cstr);
        record rec1 = record(_REG_, -1);
        if(aop == "+"){
            cstr << "\t" << rec1.print() << " = add " << lftrec.print() << ", " << rhtrec.print() << "\n";
        }
        else if(aop == "-"){
            cstr << "\t" << rec1.print() << " = sub " << lftrec.print() << ", " << rhtrec.print() << "\n";
        }
        return  rec1;
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
        record rec1 = record(_REG_, -1);
        record rec2 = record(_REG_, -1);
        if(loop == "||"){
            cstr << "\t" << rec1.print() << " = or " << lftrec.print() << ", " << rhtrec.print() << "\n";
            cstr << "\t" << rec2.print() << " = ne " << rec1.print()   << ", 0\n";
        }
        return  rec2;
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
        record rec1 = record(_REG_, -1);
        if(eop == "=="){
            cstr << "\t" << rec1.print() << " = eq " << lftrec.print() << ", " << rhtrec.print() << "\n";
        }
        else if(eop == "!="){
            cstr << "\t" << rec1.print() << " = ne " << lftrec.print() << ", " << rhtrec.print() << "\n";
        }
        return  rec1;
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
    return record();
}


