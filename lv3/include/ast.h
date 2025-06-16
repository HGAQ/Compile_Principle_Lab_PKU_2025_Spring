#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

const std::string _TEST_ = "-koopa";
const std::string _RSC5_ = "-riscv";
#define _IMM_ 0
#define _REG_ 1


class record
{
  public:
    static int curr_idx;
    int idx;// == 0 : imm == 1 : Reg
    int type;
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
  std::unique_ptr<BaseAST> stmt;//it is a type
  record Dump(std::stringstream &cstr)const override;
};


class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;//it is a type
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