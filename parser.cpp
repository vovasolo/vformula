#include "parser.h"
#include <algorithm>
#include <iostream>
#include <chrono>

int main()
{
    Parser p;
//    std::string e("1/(exp((x-3)/2)+exp((3-x)/2))");
    std::string e("(x-4)^2");

//    p.ParseExpr("sin(pi/4)-sqrt(2)/2");
//    p.ParseExpr("10*10+15");
//    p.ParseExpr("1");
//    p.ParseExpr("exp(0-(x-3)*(x-3))/2");
//    p.ParseExpr("1/(exp((x-3)/2)+exp((3-x)/2))");
//    p.ParseExpr("0.5/cosh((x-3)/2)");

    p.ParseExpr(e);
    std::cout << "\n--------Expression-------\n";
    std::cout << e << std::endl;  
    std::cout << "\n----------Map------------\n";
    p.PrintMap();
    std::cout << "\n---------Program---------\n";
    p.PrintPrg();
    std::cout << "\n-----------Eval----------\n";
    std::cout << p.Eval() << std::endl;
    std::cout << p.Stack.size() << " elements left in the stack\n\n"; 

// timed run
    std::cout << "Timed run\n";    
    auto start = std::chrono::high_resolution_clock::now();

//  Code to be timed
    for (int i=0; i<100000; i++)
        p.Eval();

    auto end = std::chrono::high_resolution_clock::now();
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::nano> (diff).count()/100000 << " ns/eval" << std::endl;
    
    return 0;
}

Parser::Parser()
{
    AddOperation("+", &Parser::Add, "ADD", 5);
    AddOperation("-", &Parser::Sub, "SUB", 5);
    AddOperation("*", &Parser::Mul, "MUL", 4);
    AddOperation("/", &Parser::Div, "DIV", 4);
    AddOperation("^", &Parser::Pow, "POW", 3);
// unary minus?

    pow2 = AddFunction("pow2", &Parser::Pow2, "POW2");
    pow3 = AddFunction("pow3", &Parser::Pow3, "POW3");


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
}

void Parser::PrintPrg()
{
    for (int cmd : Command) {
        int c = cmd/1000;
        int i = cmd%1000;

        if (c == CmdOper)
            std::cout << cmd << "\tOpr\t" << OperMnem[i] << std::endl;
        else if (c == CmdFunc)
            std::cout << cmd << "\tFun\t" << FuncMnem[i] << std::endl;
        else if (c == CmdReadConst) {
            if (ConstName[i].size() == 0)
                std::cout << cmd << "\tCon\t" << Const[i] << std::endl;
            else
                std::cout << cmd << "\tCon\t" << ConstName[i] << "=" << Const[i] << std::endl;
        }
        else if (c == CmdReadVar)
            std::cout << cmd << "\tVar\t" << VarName[i] << "=" << Var[i] << std::endl;                        
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
        int addr;
        if (FindSymbol(ConstName, symbol, &addr))
            return Token(TokConst, symbol, addr);

        if (FindSymbol(VarName, symbol, &addr))
            return Token(TokVar, symbol, addr);

        if (FindSymbol(FuncName, symbol, &addr)) {
            if (Expr[TokPos] != '(') {
                TokPos -= len;
                return Token(TokError, std::string("Known function ")+symbol+" without ()");
            }
            return Token(TokFunc, symbol, addr);
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

        if (token.type == TokNumber || token.type == TokConst) {
        // we have special treatment for the cases of ^2 and ^3
        // to make them process a bit faster
            if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 2) { // ^2
                Command.push_back(CmdFunc*1000 + pow2);
                OpStack.pop();
            } else if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 3) { // ^3
                Command.push_back(CmdFunc*1000 + pow3);
                OpStack.pop();
            } else { // in all other cases
                Command.push_back(CmdReadConst*1000 + token.addr); // move to command queue
            }
        }

        else if (token.type == TokVar) 
            Command.push_back(CmdReadVar*1000 + token.addr); // move to command queue

        else if (token.type == TokFunc) 
            OpStack.push(token); // push to Op stack

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

bool Parser::FindSymbol(std::vector <std::string> &namevec, std::string symbol, int *addr)
{
    std::vector <std::string> :: iterator itr;

    itr = std::find(namevec.begin(), namevec.end(), symbol);
    if (itr == namevec.end()) 
        return false;

    *addr = itr-namevec.begin();
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
    int addr;
    if (name.size() == 0) { // if automatically generated (empty name)
        std::vector <double> :: iterator itr = std::find(Const.begin(), Const.end(), val);
        if (itr != Const.end()) // if a constant with the same value already exists
            return itr-Const.begin(); // use it
    } else if (FindSymbol(ConstName, name, &addr)) { // if the constant with this name already exists - update it
        Var[addr] = val;
        return addr;
    }
// otherwise create a new one      
    ConstName.push_back(name);
    Const.push_back(val);
    return Const.size()-1;
}

int Parser::AddVariable(std::string name, double val) 
{
// if the variable with this name already exists - update it   
    int addr;
    if (FindSymbol(VarName, name, &addr)) { 
        Var[addr] = val;
        return addr;
    }
// otherwise create a new one
    VarName.push_back(name);
    Var.push_back(val);
    return Var.size()-1;
}

double Parser::GetConstant(std::string name)
{
    int addr;
    return FindSymbol(ConstName, name, &addr) ? Const[addr] : nan("");
}

double Parser::GetVariable(std::string name)
{
    int addr;
    return FindSymbol(VarName, name, &addr) ? Var[addr] : nan("");    
}

bool Parser::SetConstant(std::string name, double val)
{
    int addr;
    bool status = FindSymbol(ConstName, name, &addr);
    if (status)
        Const[addr] = val;
    return status;     
}

bool Parser::SetVariable(std::string name, double val)
{
    int addr;
    bool status = FindSymbol(VarName, name, &addr);
    if (status)
        Var[addr] = val;
    return status;    
}