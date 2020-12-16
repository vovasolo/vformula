#include <stack>
#include <vector>
#include <set>
#include <map>
#include <string>
#include <cmath>

/*
The provided expression is translated first into series of commands in postfix order for a simple stack-based
evaluator. The commands are integers composed as:
CmdType*1000 + addr

Possible values of CmdType and associated actions:
0  CmdNop: no operation (for debugging)
1  CmdOper: take two top elements from the stack, perform operation @addr on them, push the result
2  CmdFunc: take the top element from the stack, perform function @addr on it, push the result
3  CmdReadConst: push constant @addr to the stack
4  CmdReadVar: push variable @addr to the stack
5  CmdWriteVar: take element from the stack, store it into variable @addr
6  CmdReturn: stop execution, return top of the stack
*/

enum TokenType {
    TokNumber = 0,
    TokConst,
    TokVar,
    TokFunc,
    TokOper,
    TokOpen,
    TokClose,
    TokEnd,
    TokError
};

struct Token {
    TokenType type;
    std::string string;
    int addr;
    Token(TokenType t, std::string s, int a=0) : type(t), string(s), addr(a) {;}
};

class Parser
{
public: 
    typedef void (Parser::*FuncPtr)();

    enum CmdType {
        CmdNop = 0,
        CmdOper,
        CmdFunc,
        CmdReadConst,
        CmdReadVar,
        CmdWriteVar,
        CmdReturn
    };

// Evaluator memory
    std::vector <int> Command;   // expression translated to commands in postfix order
    std::vector <double> Const;  // vector of constants
    std::vector <double> Var;    // vector of variables
    std::vector <FuncPtr> Func;  // vector of function pointers 
    std::vector <FuncPtr> Oper;  // vector of operator pointers 
    std::stack <double> Stack;   // evaluator stack

    void Add() {double tmp = Stack.top(); Stack.pop(); Stack.top() += tmp;}
    void Sub() {double tmp = Stack.top(); Stack.pop(); Stack.top() -= tmp;}
    void Mul() {double tmp = Stack.top(); Stack.pop(); Stack.top() *= tmp;}
    void Div() {double tmp = Stack.top(); Stack.pop(); Stack.top() /= tmp;}
    void Neg() {Stack.top() = -Stack.top();}
    void Sqrt() {Stack.top() = sqrt(Stack.top());}
    void Exp() {Stack.top() = exp(Stack.top());}
    void Log() {Stack.top() = log(Stack.top());}
    void Sin() {Stack.top() = sin(Stack.top());}
    void Cos() {Stack.top() = cos(Stack.top());}
    void Tan() {Stack.top() = tan(Stack.top());}

    void Sinh() {Stack.top() = sinh(Stack.top());}
    void Cosh() {Stack.top() = cosh(Stack.top());}
    void Tanh() {Stack.top() = tanh(Stack.top());}

// Parser memory
    std::vector <std::string> ConstName; // names of constants: position corresponds to position in Const
    std::vector <std::string> VarName;   // names of variables: position corresponds to position in Var
    std::vector <std::string> FuncName;  // names of functions: position corresponds to position in Func
    std::vector <std::string> FuncMnem;  // function mnemonics: position corresponds to position in Func
    std::vector <std::string> OperName;  // names of operations: position corresponds to position in Oper
    std::vector <std::string> OperMnem;  // operation mnemonics: position corresponds to position in Oper
    std::vector <int> OperRank;  // operation priorities (less is higher): position corresponds to position in Oper
    std::stack <Token> OpStack;  // parser stack

    Parser();
    ~Parser() {;}

    int AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank);
    int AddFunction(std::string name, FuncPtr ptr, std::string mnem);
    int AddConstant(std::string name, double val);
    int AddVariable(std::string name, double val);

    bool ParseExpr(std::string expr);
    Token GetNextToken();
    bool ShuntingYard();
    void PrintMap();

    double Eval();
    double Eval(double x);

//    bool SetVar(std::string name, double val); // returns true if var with this name exists

private:
    std::string Expr;
    int TokPos = 0; // current token position in Expr
    int CmdPos = 0;
    std::string ErrorString;
};
