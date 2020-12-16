#include "parser.h"
#include <algorithm>
#include <iostream>
#include <chrono>

int main()
{
    Parser p;

//    p.ParseExpr("sin(pi/4)-sqrt(2)/2");
//    p.ParseExpr("10*10+15");
//    p.ParseExpr("1");
//    p.ParseExpr("exp(0-(x-3)*(x-3))/2");
//    p.ParseExpr("1/(exp((x-3)/2)+exp((3-x)/2))");
    p.ParseExpr("0.5/cosh((x-3)/2)");
    std::cout << "\n---------------------\n";
    p.PrintMap();
    std::cout << "\n---------Eval--------\n";
    std::cout << p.Eval(4) << std::endl;
    std::cout << p.Stack.size() << " elements left in the stack\n\n"; 

// timed run
    std::cout << "Timed run\n";    
    auto start = std::chrono::steady_clock::now();

//  Code to be timed
    for (int i=0; i<10000; i++)
        p.Eval();

    auto end = std::chrono::steady_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::nano> (diff).count()/10000 << " ns/eval" << std::endl;
    
    return 0;
}

Parser::Parser()
{
    AddOperation("+", &Parser::Add, "ADD", 5);
    AddOperation("-", &Parser::Sub, "SUB", 5);
    AddOperation("*", &Parser::Mul, "MUL", 4);
    AddOperation("/", &Parser::Div, "DIV", 4);
// unary minus?

    AddFunction("sqrt", &Parser::Sqrt, "SQRT");
    AddFunction("exp", &Parser::Exp, "EXP");
    AddFunction("log", &Parser::Log, "LOG");    
    AddFunction("sin", &Parser::Sin, "SIN");
    AddFunction("cos", &Parser::Cos, "COS");
    AddFunction("tan", &Parser::Tan, "TAN");

    AddFunction("sinh", &Parser::Sinh, "SINH");
    AddFunction("cosh", &Parser::Cosh, "COSH");
    AddFunction("tanh", &Parser::Tanh, "TANH");

    AddConstant("pi", M_PI);

    AddVariable("x", 0.);
    AddVariable("y", 0.);
    AddVariable("z", 0.);
}

double Parser::Eval(double x)
{
    Var[0] = x;
    return Eval();
}

double Parser::Eval()
{
    int codelen = Command.size();
    for (int i=0; i<codelen; i++) {
        int cmd = Command[i];
        int addr = cmd%1000;
        cmd /= 1000;

        switch (cmd) {
            case CmdOper:
                (this->*Oper[addr])();
                break;
            case CmdFunc:
                (this->*Func[addr])();
                break;
            case CmdReadConst:
                Stack.push(Const[addr]);
                break;
            case CmdReadVar:
                Stack.push(Var[addr]);
                break;
            case CmdReturn:
                double result = Stack.top();
                Stack.pop();
                return result;                         
        }
    }
}

bool Parser::ParseExpr(std::string expr)
{
    Expr = expr;
    TokPos = 0;
    Command.clear();
    bool status = ShuntingYard();
    if (!status) {
        std::cout << "Parsing failed at " << TokPos << std::endl;
        std::cout << ErrorString << std::endl;
        return false;
    }

    for (int cmd : Command) {
        int c = cmd/1000;
        int i = cmd%1000;

        if (c == CmdOper)
            std::cout << cmd << "\tOpr\t" << OperMnem[i] << std::endl;
        else if (c == CmdFunc)
            std::cout << cmd << "\tFun\t" << FuncMnem[i] << std::endl;
        else if (c == CmdReadConst)
            std::cout << cmd << "\tCon\t" << ConstName[i] << " : " << Const[i] << std::endl;
        else if (c == CmdReadVar)
            std::cout << cmd << "\tVar\t" << VarName[i] << " : " << Var[i] << std::endl;                        
    }
}

void Parser::PrintMap()
{
    std::cout << "Operators\n";
    for (int i=0; i<OperName.size(); i++)
        std::cout << OperName[i] << " : " << OperMnem[i] << std::endl;
    std::cout << "Functions\n";
    for (int i=0; i<FuncName.size(); i++)
        std::cout << FuncName[i] << " : " << FuncMnem[i] << std::endl;
    std::cout << "Constants\n";
    for (int i=0; i<ConstName.size(); i++)
        std::cout << ConstName[i] << " : " << Const[i] << std::endl;
    std::cout << "Variables\n";
    for (int i=0; i<VarName.size(); i++)
        std::cout << VarName[i] << " : " << Var[i] << std::endl;        
}

Token Parser::GetNextToken()
{
// skip spaces    
    while (TokPos < Expr.size() && Expr[TokPos] == ' ')
        TokPos++;

    if (TokPos >= Expr.size())
        return Token(TokEnd, "");

    int ch0 = Expr[TokPos]; // fetch the character at the current token position

// parenthses
    if (ch0 == '(') {
        TokPos++;
        return Token(TokOpen, "(");
    }
    if (ch0 == ')') {
        TokPos++;
        return Token(TokClose, ")");
    }

// number
    if (std::isdigit(ch0)) { 
        std::size_t len;
        double val = std::stod(Expr.substr(TokPos, std::string::npos), &len);
        int addr = AddConstant("", val); // numbers are stored as nameless constants 
        TokPos += len;
        return Token(TokNumber, Expr.substr(TokPos-len, len), addr); 
    }

// symbol (variable, constant or function name)
    if (std::isalpha(ch0)) { 
        int len = 1;
        for (int i=TokPos+1; i<Expr.size(); i++) {
            int ch = Expr[i];
            if (!isalpha(ch) && !isdigit(ch) && ch!='_') 
                break;
            len++;
        }
        std::string symbol = Expr.substr(TokPos, len);
        TokPos += len;

    // now check if it is a known symbol       
        std::vector <std::string> :: iterator itr;

        itr = std::find(ConstName.begin(), ConstName.end(), symbol);
        if (itr != ConstName.end())
            return Token(TokConst, symbol, itr-ConstName.begin());

        itr = std::find(VarName.begin(), VarName.end(), symbol);
        if (itr != VarName.end())
            return Token(TokVar, symbol, itr-VarName.begin());

        itr = std::find(FuncName.begin(), FuncName.end(), symbol);
        if (itr != FuncName.end()) {
            if (Expr[TokPos] != '(') {
                TokPos -= len;
                return Token(TokError, std::string("Known function ")+symbol+" without ()");
            }
            return Token(TokFunc, symbol, itr-FuncName.begin());
        }

        TokPos -= len;
        return Token(TokError, std::string("Unknown symbol: ")+symbol);
    }

// operators
    for (int i=0; i<OperName.size(); i++) 
        if (Expr.substr(TokPos, std::string::npos).find(OperName[i]) == 0) {
            TokPos += OperName[i].size();
            return Token(TokOper, OperName[i], i);
        }

    return Token(TokError, "Unknown character or character combination");
}

bool Parser::ShuntingYard()
{
    while (1) {
        Token token = GetNextToken();

        if (token.type == TokError) {
            ErrorString = token.string;
            return false;
        } 

        if (token.type == TokNumber || token.type == TokConst) // move to command queue
            Command.push_back(CmdReadConst*1000 + token.addr);

        else if (token.type == TokVar) // move to command queue
            Command.push_back(CmdReadVar*1000 + token.addr);

        else if (token.type == TokFunc) // push to Op stack
            OpStack.push(token);

        else if (token.type == TokOper) {
            int rank = OperRank[token.addr];
            while (!OpStack.empty()) {
                Token op2 = OpStack.top();
                if (op2.type == TokOper && OperRank[op2.addr] < rank) {
                    Command.push_back(CmdOper*1000 + op2.addr);
                    OpStack.pop();
                } else {
                    break;
                }
            }
            OpStack.push(token);
        }

        else if (token.type == TokOpen)
            OpStack.push(token);

        else if (token.type == TokClose) {
            while (!OpStack.empty() && OpStack.top().type != TokOpen) {
                Command.push_back(CmdOper*1000 + OpStack.top().addr);
                OpStack.pop();
            }
            if (!OpStack.empty() && OpStack.top().type == TokOpen)
                OpStack.pop();
            if (!OpStack.empty() && OpStack.top().type == TokFunc) {
                Command.push_back(CmdFunc*1000 + OpStack.top().addr);
                OpStack.pop();
            }
        }

        else if (token.type == TokEnd)
            break;
    }

    while (!OpStack.empty()) {
        Command.push_back(CmdOper*1000 + OpStack.top().addr);
        OpStack.pop();
    }
    Command.push_back(CmdReturn*1000);
    return true;
}


int Parser::AddOperation(std::string name, FuncPtr ptr, std::string mnem, int rank)
{
    OperName.push_back(name);
    OperMnem.push_back(mnem);
    OperRank.push_back(rank);
    Oper.push_back(ptr);
    return Oper.size()-1;
}

int Parser::AddFunction(std::string name, FuncPtr ptr, std::string mnem) 
{
    FuncName.push_back(name);
    FuncMnem.push_back(mnem);
    Func.push_back(ptr);
    return Func.size()-1;
}

int Parser::AddConstant(std::string name, double val)
{
    ConstName.push_back(name);
    Const.push_back(val);
    return Const.size()-1;
}

int Parser::AddVariable(std::string name, double val) 
{
    VarName.push_back(name);
    Var.push_back(val);
    return Var.size()-1;
}
