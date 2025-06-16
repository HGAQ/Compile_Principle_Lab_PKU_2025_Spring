#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>
using namespace std;

const std::string _TEST_ = "-koopa";
const std::string _RSC5_ = "-riscv";
#define _IMM_ 0
#define _REG_ 1
#define _VAR_ 1
#define _VAL_ 0
#define _NONE_ -1

class record
{
  public:
    static int curr_idx;
    int idx;
    int type; // == 0 : imm == 1 : Reg
    record(){
      idx = -1;
      type = _IMM_;
    }
    record(int type_, int idx_){
      if(type_ == _REG_){
        record::curr_idx += 1;
        idx = curr_idx;
        type = type_;
      }
      else if(type_ == _IMM_){
        type = type_ ;
        idx = idx_;
      }
    }
    std::string print(){
      if (type == _REG_){
        return "%" + std::to_string(idx);
      }
      else if(type == _IMM_){
        return std::to_string(idx);
      }
      return "";
    }
}
;

class symbol{
  public:
    int type; // == 0 : imm == 1 : Reg
    int val;
    symbol(){
      type = _NONE_;
      val = 0;
    }
    symbol(int type_, int val_){
      val = val_;
      type = type_;
    }
};

class symboltable{
  public:
    std::unordered_map<std::string, symbol> map;
    std::string Find(symbol sym){
        for(auto& k: map){
          if(k.second.val == sym.val && k.second.type == sym.type){
            return k.first;
          }
        }
        return "";
      }

    symbol Find(std::string sym){
        for(auto& k: map){
          if(k.first == sym){
            return k.second;
          }
        }
        return symbol();
      }

    void Add(std::string str, symbol sym){
      map.insert(pair<std::string, symbol>(str, sym));
    }

    symboltable(){
      map.clear();
    }
};




////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
//AST


// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual record Dump(std::stringstream &cstr) const  = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  record Dump(std::stringstream &cstr)const override ;
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  record Dump(std::stringstream &cstr)const override ;
};


class FuncTypeAST : public BaseAST {
 public:
  std::string type;//it is a type
  record Dump(std::stringstream &cstr)const override;
};


class BlockAST : public BaseAST {
 public:
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> vblock_item;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class BlockItemAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;
  std::unique_ptr<BaseAST> decl;
  int type;
  record Dump(std::stringstream &cstr)const override;
};

class DeclAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> constdecl;//it is a type
  std::unique_ptr<BaseAST> vardecl;//it is a type
  int type;
  record Dump(std::stringstream &cstr)const override;
};

class ConstDeclAST : public BaseAST {
 public:
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> vconst_def;//it is a type
  std::unique_ptr<BaseAST> btype;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class VarDeclAST : public BaseAST {
 public:  
  std::unique_ptr<std::vector<std::unique_ptr<BaseAST>>> vvar_def;//it is a type
  std::unique_ptr<BaseAST> btype;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class BTypeAST : public BaseAST {
 public:
  std::string btype;
  record Dump(std::stringstream &cstr)const override;
};

class ConstDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> const_initval;//it is a type
  std::string ident;
  record Dump(std::stringstream &cstr)const override;
};

class VarDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> initval;//it is a type
  std::string ident;
  int type;
  record Dump(std::stringstream &cstr)const override;
};

class ConstInitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> const_exp;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class InitValAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class ConstExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;//it is a type
  std::unique_ptr<BaseAST> lval;//it is a type
  int type;
  record Dump(std::stringstream &cstr)const override;
};

class LValAST : public BaseAST {
 public:
  std::string ident;
  record Dump(std::stringstream &cstr)const override;
};

class ExpAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> loexp;//it is a type
  record Dump(std::stringstream &cstr)const override;
};

class UExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> pexp;
  std::string uop;
  std::unique_ptr<BaseAST> uexp;
  record Dump(std::stringstream &cstr)const override;
};

class PExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> lval;
  int number;
  record Dump(std::stringstream &cstr)const override;
};

class MExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> uexp;
  std::string mop;
  std::unique_ptr<BaseAST> mexp;
  record Dump(std::stringstream &cstr)const override;
};

class AExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> mexp;
  std::string aop;
  std::unique_ptr<BaseAST> aexp;
  record Dump(std::stringstream &cstr)const override;
};

class LOExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> laexp;
  std::string loop;
  std::unique_ptr<BaseAST> loexp;
  record Dump(std::stringstream &cstr)const override;
};

class LAExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> eexp;
  std::string laop;
  std::unique_ptr<BaseAST> laexp;
  record Dump(std::stringstream &cstr)const override;
};

class EExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> rexp;
  std::string eop;
  std::unique_ptr<BaseAST> eexp;
  record Dump(std::stringstream &cstr)const override;
};

class RExpAST : public BaseAST {
 public:
  int type;
  std::unique_ptr<BaseAST> aexp;
  std::string rop;
  std::unique_ptr<BaseAST> rexp;
  record Dump(std::stringstream &cstr)const override;
};



//class NumberAST : public BaseAST {
// public:
//  std::string num;//it is a type
//  void Dump() const override {
//    std::cout << "Number = "<< num;
//  }
//};

// ...