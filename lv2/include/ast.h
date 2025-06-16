#pragma once

#include <memory>
#include <string>
#include <iostream>
using namespace std;

const std::string _TEST_ = "-koopa";
const std::string _RSC5_ = "-riscv";

// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual std::string Dump(const std::string &mode) const = 0;
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  std::string Dump(const std::string &mode) const override {
    if(mode == _TEST_){
        func_def->Dump(mode);
    }
    else if(mode == _RSC5_){
        return func_def->Dump(mode);
    }
    else{
        std::cout << "CompUnitAST { ";
        func_def->Dump(mode);
        std::cout << " }";
    }
    return "";
  }
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::string Dump(const std::string &mode) const override {
    if(mode == _TEST_){
        std::cout << "fun @" << ident << "(): ";
        func_type->Dump(mode);
        std::cout << " {\n";
        block->Dump(mode);
        std::cout << "}\n";
    }
    else if(mode == _RSC5_){
        std::string str = "fun @" + ident + "(): " + func_type->Dump(mode) + " {\n" + block->Dump(mode) +  "}\n";
        return str;
    }
    else{
        std::cout << "FuncDefAST { ";
        func_type->Dump(mode);
        std::cout << ", Ident = " << ident << ", ";
        block->Dump(mode);
        std::cout << " }";
        }
    return "";
  }
};


class FuncTypeAST : public BaseAST {
 public:
  std::string type;//it is a type
  std::string Dump(const std::string &mode) const override {
    if(mode == _TEST_){
        if (type == "int")
        {
            std::cout << "i32";
        }
        else if (type == "void")
        {
            std::cout << "void";
        }
    }
    else if(mode == _RSC5_){
        std::string str = "";
        if (type == "int")
        {
            str +=  "i32";
        }
        else if (type == "void")
        {
            str +=  "void";
        }
        return str;
    }
    else{
        std::cout << "FuncType = " << type ;
    }
    return "";
  }
};

class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> stmt;//it is a type
  std::string Dump(const std::string &mode) const override {
    if(mode == _TEST_){
        std::cout << "%entry:\n";
        stmt->Dump(mode);
    }
    else if(mode == _RSC5_){
        return "%entry:\n" + stmt->Dump(mode);
    }
    else{
        std::cout << "Block { ";
        stmt->Dump(mode);
        std::cout << " }";
    }
    return "";
  }
};


class StmtAST : public BaseAST {
 public:
  int number;//it is a type
  std::string Dump(const std::string &mode) const override {
    if(mode == _TEST_){
        std::cout << "  ret ";
        std::cout << number;
        std::cout << "\n";
    }
    else if(mode == _RSC5_){
        return  "  ret "+ std::to_string(number) +"\n";
    }
    else{
        std::cout << "Ret = " << number;
    }
    return "";
  }
};

//class NumberAST : public BaseAST {
// public:
//  std::string num;//it is a type
//  void Dump() const override {
//    std::cout << "Number = "<< num;
//  }
//};

// ...