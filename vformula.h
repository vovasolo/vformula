#ifndef VFORMULA_H
#define VFORMULA_H

#include <cstddef>
#include <stack>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <stdexcept>

class VParser
{
public: 
/*
The provided expression is translated first into series of commands in postfix order for a simple stack-based
evaluator.

Possible values of CmdType and associated actions:
0  CmdNop: no operation (for debugging)
1  CmdOper: take two top elements from the stack, perform operation @addr on them, push the result
2  CmdFunc: take the top element from the stack, perform function @addr on it, push the result
3  CmdReadConst: push constant @addr to the stack
4  CmdReadVar: push variable @addr to the stack
5  CmdWriteVar: take element from the stack, store it into variable @addr
6  CmdReturn: stop execution, return top of the stack
*/
    enum CmdType {
        CmdNop = 0,
        CmdOper,
        CmdFunc,
        CmdReadConst,
        CmdReadVar,
        CmdWriteVar,
        CmdReturn
    };

    enum TokenType {
        TokNull = 0,
        TokNumber,
        TokConst,
        TokVar,
        TokWrVar,
        TokFunc,
        TokOper,
        TokUnary,
        TokOpen,
        TokClose,
        TokComma,
        TokEndSub,
        TokEnd,
        TokError
    };
    
    struct Token {
        TokenType type;
        std::string string;
        int addr;
        int args;
        Token(TokenType t, std::string s, int a=0) : type(t), string(s), addr(a) {;}
    };
    
    struct Cmdaddr{
        unsigned short cmd;
        unsigned short addr;
        Cmdaddr(int c, int a) : cmd(c), addr(a) {;} 
    }; 

// Evaluator memory
    std::vector <Cmdaddr> Command; // expression translated to commands in postfix order
    std::vector <double> Const;  // vector of constants

// Parser memory
    std::vector <std::string> ConstName; // names of constants: position corresponds to position in Const
    std::vector <std::string> VarName;   // names of variables: position corresponds to position in Var
    std::vector <std::string> FuncName;  // names of functions: position corresponds to position in Func
    std::vector <std::string> FuncMnem;  // function mnemonics: position corresponds to position in Func
    std::vector <int> FuncArgs; // number of arguments to take, position corresponds to position in Func
    std::vector <std::string> OperName;  // names of operations: position corresponds to position in Oper
    std::vector <std::string> OperMnem;  // operation mnemonics: position corresponds to position in Oper
    std::vector <int> OperRank;  // operation priorities (less is higher): position corresponds to position in Oper
    std::vector <int> OperArgs;  // number of arguments to take, position corresponds to position in Oper
    std::stack <Token> OpStack;  // parser stack
    std::string TargetVar;       // variable to which the result will be assigned

    VParser();
//    ~VParser() {;}

    bool FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr);

    size_t AddOperation(std::string name, std::string mnem, int rank, int args=2);
    size_t AddFunction(std::string name, std::string mnem, int args=1);
    bool AddConstant(std::string name, double val);
    bool AddVariable(std::string name);

    int GetConstCount() const {return ConstName.size();}
    double GetConstant(std::string name);
    double GetConstant(size_t addr);
    bool SetConstant(std::string name, double val);
    bool SetConstant(size_t addr, double val);

    int ParseExpr(std::string expr);
    bool CheckSyntax(Token token);
    Token GetNextToken();
    bool ShuntingYard();

    Cmdaddr MkCmd(int cmd, int addr) {return Cmdaddr(cmd, addr);}

    std::vector<std::string> GetPrg();
    std::vector<std::string> GetConstMap();
    std::vector<std::string> GetVarMap();
    std::vector<std::string> GetOperMap();
    std::vector<std::string> GetFuncMap();

    void VFail(int pos, std::string msg);
    bool Validate();

    std::string GetErrorString() {return ErrorString;}

private:
    size_t AddAutoConstant(double val);
    void PruneConstants();

    std::string Expr;
    size_t TokPos = 0; // current token position in Expr
    Token LastToken = Token(TokNull, "");
    size_t CmdPos = 0;
    std::string ErrorString;
    size_t pow2, pow3; // positions of the fast square and cube functions
    size_t neg, nop; // position of the sign inverse and nop functions 
    bool valid = true; // result of the code validity check
public:    
    size_t failpos; // position in the code at which validation failed
};

template <typename VarType> 
class VFormula : public VParser
{
    typedef void (VFormula::*FuncPtr)();

    std::vector <VarType> Var;    // vector of variables
    std::stack <VarType> Stack;   // evaluator stack
    std::vector <FuncPtr> Func;  // vector of function pointers 
    std::vector <FuncPtr> Oper;  // vector of operator pointers 

    int veclen = 0;         // length of vectors to operate

    void Add() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() += tmp;}
    void Sub() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() -= tmp;}
    void Mul() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() *= tmp;}
    void Div() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() /= tmp;}
    void Neg() {Stack.top() = -Stack.top();}
    void Nop() {;}
    void Pow() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = pow(Stack.top(), tmp);}
    void Pow2() {VarType tmp = Stack.top(); Stack.top() = tmp*tmp;}
    void Pow3() {VarType tmp = Stack.top(); Stack.top() = tmp*tmp*tmp;}

    void Sqrt() {Stack.top() = sqrt(Stack.top());}
    void Exp() {Stack.top() = exp(Stack.top());}
    void Log() {Stack.top() = log(Stack.top());}
    void Sin() {Stack.top() = sin(Stack.top());}
    void Cos() {Stack.top() = cos(Stack.top());}
    void Tan() {Stack.top() = tan(Stack.top());}
    void Asin() {Stack.top() = asin(Stack.top());}
    void Acos() {Stack.top() = acos(Stack.top());}
    void Atan() {Stack.top() = atan(Stack.top());}
//    void Atan2() {double x = Stack.top(); Stack.pop(); Stack.top() = atan2(Stack.top(), x);} //atan2(y,x)

    void Sinh() {Stack.top() = sinh(Stack.top());}
    void Cosh() {Stack.top() = cosh(Stack.top());}
    void Tanh() {Stack.top() = tanh(Stack.top());}
    void Asinh() {Stack.top() = asinh(Stack.top());}
    void Acosh() {Stack.top() = acosh(Stack.top());}
    void Atanh() {Stack.top() = atanh(Stack.top());}

//    void Int() {double t; modf(Stack.top(), &t); Stack.top() = t;}
//    void Frac() {double t; Stack.top() = modf(Stack.top(), &t);}

// scalar and vector versions of these functions have different implementation 
/*
    #ifdef VECTOR_VARS
    void Abs() {Stack.top() = abs(Stack.top());}
    void Max() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.max(Stack.top());}
    void Min() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.min(Stack.top());}
    #else
    void Abs() {Stack.top() = fabs(Stack.top());}
    void Max() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::max(tmp, Stack.top());}
    void Min() {VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::min(tmp, Stack.top());}
    #endif    
*/
    void Abs() 
    {    
        if constexpr(std::is_scalar<VarType>::value)
            Stack.top() = fabs(Stack.top());
        else
            Stack.top() = abs(Stack.top());
    }

    void Max()
    {
        if constexpr(std::is_scalar<VarType>::value) {
            VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::max(tmp, Stack.top());
        } else {
            VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.max(Stack.top());
        }        
    }

    void Min()
    {
        if constexpr(std::is_scalar<VarType>::value) {
            VarType tmp = Stack.top(); Stack.pop(); Stack.top() = std::min(tmp, Stack.top());
        } else {
            VarType tmp = Stack.top(); Stack.pop(); Stack.top() = tmp.min(Stack.top());
        }        
    }

    // void Gaus();
    // void Pol2();
    // void Pol3();

    void MkOper(FuncPtr op, std::string mnem)
    {
        size_t addr;
        if (FindSymbol(OperMnem, mnem, &addr))
            Oper[addr] = op;
        else
            throw std::runtime_error(std::string("VFormula: Unknown operation ") + mnem);
    }

    void MkFunc(FuncPtr func, std::string mnem)
    {
        size_t addr;
        if (FindSymbol(FuncMnem, mnem, &addr))
            Func[addr] = func;
        else
            throw std::runtime_error(std::string("VFormula: Unknown function ") + mnem);
    }

public:
    VFormula() {
        Oper.resize(OperName.size());
        Func.resize(FuncName.size());
        Var.resize(VarName.size());

        MkOper(&VFormula::Add, "ADD");
        MkOper(&VFormula::Sub, "SUB");
        MkOper(&VFormula::Mul, "MUL");
        MkOper(&VFormula::Div, "DIV");
        MkOper(&VFormula::Pow, "POW");
        MkOper(&VFormula::Neg, "NEG");
        MkOper(&VFormula::Nop, "NOP");

        MkFunc(&VFormula::Pow2, "POW2");
        MkFunc(&VFormula::Pow3, "POW3");
        MkFunc(&VFormula::Pow, "POW");
        MkFunc(&VFormula::Abs, "ABS");
        MkFunc(&VFormula::Sqrt, "SQRT");
        MkFunc(&VFormula::Exp, "EXP");
        MkFunc(&VFormula::Log, "LOG");

        MkFunc(&VFormula::Sin, "SIN");
        MkFunc(&VFormula::Cos, "COS");
        MkFunc(&VFormula::Tan, "TAN");
        MkFunc(&VFormula::Asin, "ASIN");        
        MkFunc(&VFormula::Acos, "ACOS");
        MkFunc(&VFormula::Atan, "ATAN");

        MkFunc(&VFormula::Sinh, "SINH");
        MkFunc(&VFormula::Cosh, "COSH");
        MkFunc(&VFormula::Tanh, "TANH");
        MkFunc(&VFormula::Asinh, "ASINH");        
        MkFunc(&VFormula::Acosh, "ACOSH");
        MkFunc(&VFormula::Atanh, "ATANH");

        MkFunc(&VFormula::Max, "MAX");
        MkFunc(&VFormula::Min, "MIN");

    }

    int ParseExpr(std::string expr)
    {
        int errpos = VParser::ParseExpr(expr);
        Var.resize(VarName.size());
        return errpos;
    }

    VarType GetVariable(std::string name)
    {
        size_t addr;
        return FindSymbol(VarName, name, &addr) ? Var[addr] : VarType(0);
    }

    bool SetVariable(std::string name, VarType val)
    {
        size_t addr;
        bool status = FindSymbol(VarName, name, &addr);
        if (status)
            Var[addr] = val;
        return status;    
    }

    VarType Eval()
    {
        size_t codelen = Command.size();
        for (size_t i=0; i<codelen; i++) {
            unsigned short cmd = Command[i].cmd;
            unsigned short addr = Command[i].addr;
            switch (cmd) {
                case CmdOper:
                    (this->*Oper[addr])();
                    break;
                case CmdFunc:
                    (this->*Func[addr])();
                    break;
                case CmdReadConst:
                    if constexpr(std::is_scalar<VarType>::value)
                        Stack.push(Const[addr]);
                    else
                        Stack.push(VarType::Constant(veclen, Const[addr]));
                    break;
                case CmdReadVar:
                    Stack.push(Var[addr]);
                    break;
                case CmdWriteVar:
                    Var[addr] = Stack.top();
                    Stack.pop();
                    break;
                case CmdReturn: {
                    VarType result = Stack.top();
                    Stack.pop();
                    //std::cout << "Stack size: " << Stack.size() << std::endl;
                    return result;
                }
                default: // unknown command means a bug in the parser
                    throw std::runtime_error(std::string("Eval: Unknown command ") + std::to_string(cmd));
            }
        }
        // empty program - return 0
        if constexpr(std::is_scalar<VarType>::value)
            return 0.;
        else
            return VarType::Constant(veclen, 0.);
    }

    VarType Eval(VarType x)
    {
        Var[0] = x;
        if constexpr(!std::is_scalar<VarType>::value)
            veclen = x.size();
        return Eval();
    }

    VarType Eval(VarType x, VarType y)
    {
        Var[0] = x;
        Var[1] = y;
        if constexpr(!std::is_scalar<VarType>::value)
            veclen = x.size();
        return Eval();
    }

};

#endif // VFORMULA_H
