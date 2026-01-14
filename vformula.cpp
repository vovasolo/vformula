#include "vformula.h"
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <cstdio>
#include <string>
#include <vector>

//#include <chrono>

VParser::VParser()
{
    AddOperation("+", "ADD", 5);
    AddOperation("-", "SUB", 5);
    AddOperation("*", "MUL", 4);
    AddOperation("/", "DIV", 4);
    AddOperation("^", "POW", 3);
// unary minus and plus  
    neg = AddOperation("--", "NEG", 2, 1);
    nop = AddOperation("++", "NOP", 2, 1);
    
    pow2 = AddFunction("pow2", "POW2");
    pow3 = AddFunction("pow3", "POW3");

    AddFunction("pow", "POW", 2);
    AddFunction("abs", "ABS");
    AddFunction("sqrt", "SQRT");
    AddFunction("exp", "EXP");
    AddFunction("log", "LOG");

    AddFunction("sin", "SIN");
    AddFunction("cos", "COS");
    AddFunction("tan", "TAN");
    AddFunction("asin", "ASIN");
    AddFunction("acos", "ACOS");
    AddFunction("atan", "ATAN");

    AddFunction("sinh", "SINH");
    AddFunction("cosh", "COSH");
    AddFunction("tanh", "TANH");
    AddFunction("asinh", "ASINH");
    AddFunction("acosh", "ACOSH");
    AddFunction("atanh", "ATANH");

    AddFunction("max", "MAX", 2);
    AddFunction("min", "MIN", 2);

}

void VParser::VFail(int pos, std::string msg)
{
    valid = false;
    failpos = pos;
    ErrorString = msg;
}

bool VParser::Validate()
{
    valid = true;
    size_t codelen = Command.size();
    int stkptr = 0;
    bool finished = false;

    for (size_t i=0; i<codelen; i++) {
        unsigned short cmd = Command[i].cmd;
        unsigned short addr = Command[i].addr;
        switch (cmd) {
            case CmdOper:
                if (addr >= OperName.size())
                    VFail(i, "Operation out of range");
                stkptr = stkptr - OperArgs[addr] + 1; 
                break;
            case CmdFunc:
                if (addr >= FuncName.size())
                    VFail(i, "Function out of range");
                stkptr = stkptr - FuncArgs[addr] + 1;
                break;
            case CmdReadConst:
                if (addr >= Const.size())
                    VFail(i, "Constant out of range");
                stkptr = stkptr + 1;
                break;
            case CmdReadVar:
                if (addr >= VarName.size())
                    VFail(i, "Variable out of range");
                stkptr = stkptr + 1;
                break;
            case CmdWriteVar:
                if (addr >= VarName.size())
                    VFail(i, "Variable out of range");
                stkptr = stkptr - 1;
                if (stkptr != 0)
                    VFail(-1, std::string("Upon an assignment stack is out of balance by ") + std::to_string(stkptr));
                break;
            case CmdReturn:
                stkptr--;
                finished = true;
                break;                      
        }
        if (finished)
            break;
    }

    if (stkptr != 0)
        VFail(-1, std::string("Stack is out of balance by ") + std::to_string(stkptr) + " position(s)");

    return valid;
}

// parses provided expression
// returns 1024 on success or first error position on failure
int VParser::ParseExpr(std::string expr)
{
    Expr = expr;
    TokPos = 0;
    LastToken = Token(TokNull, "");
    Command.clear();
    while(!OpStack.empty()) // empty operation stack
        OpStack.pop();
    PruneConstants();
    return ShuntingYard() ? 1024 : TokPos;
}

std::vector<std::string> VParser::GetPrg()
{
    char buf[32];
    std::vector<std::string> out;
    //std::string tab("\t");
    for (auto cmd : Command) {
        int c = cmd.cmd;
        int i = cmd.addr;
        sprintf(buf, "%02d:%02d ", c, i);

        if (c == CmdOper)
            //std::cout << buf << "\t" << OperMnem[i] << std::endl;
            out.push_back(std::string(buf) + "\t" + OperMnem[i]);
        else if (c == CmdFunc)
            //std::cout << buf << "\tCALL\t" << FuncMnem[i] << std::endl;
            out.push_back(std::string(buf) + "\tCALL\t" + FuncMnem[i]);
        else if (c == CmdReadConst) {
            if (i >= ConstName.size())
                //std::cout << buf << "\tPUSHC\t" << Const[i] << std::endl;
                out.push_back(std::string(buf) + "\tPUSHC\t" + std::to_string(Const[i]));
            else
                //std::cout << buf << "\tPUSHC\t" << ConstName[i] << "=" << Const[i] << std::endl;
                out.push_back(std::string(buf) + "\tPUSHC\t" + ConstName[i] + "=" + std::to_string(Const[i]));
        }
        else if (c == CmdReadVar)
            //std::cout << buf << "\tPUSHV\t" << VarName[i] << std::endl;
            out.push_back(std::string(buf) + "\tPUSHV\t" + VarName[i]);
        else if (c == CmdWriteVar)
            //std::cout << buf << "\tPOPV\t" << VarName[i] << std::endl;
            out.push_back(std::string(buf) + "\tPOPV\t" + VarName[i]);
    }
    return out;
}

std::vector<std::string> VParser::GetConstMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<Const.size(); i++) {
        std::string name( i<ConstName.size() ? ConstName[i] : "*");
        out.push_back(name + " : " + std::to_string(Const[i]));
    }
        
    return out;      
}

std::vector<std::string> VParser::GetVarMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<VarName.size(); i++)
        out.push_back(VarName[i]);
    return out;     
}

std::vector<std::string> VParser::GetOperMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<OperName.size(); i++)
        out.push_back(OperName[i] + " : " + OperMnem[i]);
    return out;
}

std::vector<std::string> VParser::GetFuncMap()
{
    std::vector<std::string> out;

    for (size_t i=0; i<FuncName.size(); i++)
        out.push_back(FuncName[i] + " : " + FuncMnem[i]);
    return out;
}

VParser::Token VParser::GetNextToken()
{
// skip spaces    
    while (TokPos < Expr.size() && Expr[TokPos] == ' ')
        TokPos++;

    if (TokPos >= Expr.size())
        return Token(TokEnd, "");

    int ch0 = Expr[TokPos]; // fetch the character at the current token position

// parentheses, comma and semicolon
    if (ch0 == '(') {
        TokPos++;
        return Token(TokOpen, "(");
    }
    if (ch0 == ')') {
        TokPos++;
        return Token(TokClose, ")");
    }
    if (ch0 == ',') {
        TokPos++;
        return Token(TokComma, ",");
    }
    if (ch0 == ';') {
        TokPos++;
        return Token(TokEndSub, ";");
    }

// number
    if (std::isdigit(ch0)) { 
        std::size_t len;
        double val = std::stod(Expr.substr(TokPos, std::string::npos), &len);
        int addr = AddAutoConstant(val); // numbers are stored as nameless constants 
        TokPos += len;
        return Token(TokNumber, Expr.substr(TokPos-len, len), addr); 
    }

// symbol (variable, constant or function name)
    if (std::isalpha(ch0)) { 
        size_t len = 1;
        for (size_t i=TokPos+1; i<Expr.size(); i++) {
            int ch = Expr[i];
            if (!isalpha(ch) && !isdigit(ch) && ch!='_') 
                break;
            len++;
        }
        std::string symbol = Expr.substr(TokPos, len);
        TokPos += len;
    
    // check if it's assignment
        if (Expr[TokPos] == '=') {
            size_t addr;
            if (FindSymbol(ConstName, symbol, &addr))
                return Token(TokError, std::string("Can not assign to constant: ")+symbol);
            if (FindSymbol(ConstName, symbol, &addr))
                return Token(TokError, std::string("Can not assign to function: ")+symbol);

            if (!FindSymbol(VarName, symbol, &addr)) {
                VarName.push_back(symbol);
                addr = VarName.size()-1;
            } 
            TokPos += 1;
//            std::cout << "symbol " << symbol << ", size " << VarName.size() << ", addr " << addr << std::endl;
            return Token(TokWrVar, symbol, addr);
        }

    // now check if it is a known symbol       
        size_t addr;
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

// unary minus and plus
    if (ch0 == '-' || ch0 == '+') {
        TokenType t = LastToken.type;
        if ( t == TokNull || t == TokOpen || t == TokOper || t==TokComma) {
            TokPos++;
            return Token(TokUnary, ch0 == '-' ? "-" : "+", ch0 == '-' ? neg : nop);
        }
    }    

// operators
    for (size_t i=0; i<OperName.size(); i++) 
        if (Expr.substr(TokPos, std::string::npos).find(OperName[i]) == 0) {
            TokPos += OperName[i].size();
            return Token(TokOper, OperName[i], i);
        }

    return Token(TokError, "Unknown character or character combination");
}

bool VParser::ShuntingYard()
{
// we'll track parentheses level
// it must never get negative and return to zero in the end
    int par_level = 0;  

    while (1) {
        Token token = GetNextToken();
        if (!CheckSyntax(token)) {
            return false;
        }

        if (token.type == TokError) {
            ErrorString = token.string;
            return false;
        } 

        if (token.type == TokNumber || token.type == TokConst) {
        // we have special treatment for the cases of ^2 and ^3
        // to make them process a bit faster
            if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 2) { // ^2
                Command.push_back(MkCmd(CmdFunc, pow2));
                OpStack.pop();
            } else if (!OpStack.empty() && OpStack.top().string == "^" && Const[token.addr] == 3) { // ^3
                Command.push_back(MkCmd(CmdFunc, pow3));
                OpStack.pop();
            } else { // in all other cases
                Command.push_back(MkCmd(CmdReadConst, token.addr)); // move to command queue
            }
        }

        else if (token.type == TokVar) 
            Command.push_back(MkCmd(CmdReadVar, token.addr)); // move to command queue

        else if (token.type == TokWrVar) {
            if (TargetVar.empty())
                TargetVar = token.string;
            else {
                ErrorString = std::string("Assignment to '") + TargetVar + "' was not terminated with ';'";
                return false;
            }
        }

        else if (token.type == TokFunc) {
            token.args = FuncArgs[token.addr]; // fill correct number of args (should be done in tokenizer?)
            OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokUnary) {
            if (token.string == "-")
                OpStack.push(token); // push to Op stack
        }

        else if (token.type == TokOper) {
            int rank = OperRank[token.addr];
            while (!OpStack.empty()) {
                Token op2 = OpStack.top();
                // <=  assuming all operators are left-associative
                // unary minus has highest precedence except when followed by ^
                if ((op2.type == TokOper && OperRank[op2.addr] <= rank) 
                    || (op2.type == TokUnary && token.string.compare("^") != 0)) {
                    Command.push_back(MkCmd(CmdOper, op2.addr));
                    OpStack.pop();
                } else {
                    LastToken = token;
                    break;
                }
            }
            OpStack.push(token);
        }

        else if (token.type == TokOpen) {
            par_level++;
            OpStack.push(token);
        }

        else if (token.type == TokClose || token.type == TokComma) {
            if (token.type == TokClose) {
                par_level--;
                if (par_level < 0) {
                    ErrorString = "Extra )";
                    return false;                   
                }
            }

            while (!OpStack.empty() && OpStack.top().type != TokOpen) {
                Command.push_back(MkCmd(CmdOper, OpStack.top().addr));
                OpStack.pop();
            }
            if (OpStack.empty()) {
                ErrorString = "Mismatched parenthesis";
                return false;
            }

            if (OpStack.top().type == TokOpen) // at this point this should be always true
                OpStack.pop();
            else {
                ErrorString = "Parentheses canary: check the parsing code";
                return false;                
            }               

            if (!OpStack.empty() && OpStack.top().type == TokFunc)
                if (--(OpStack.top().args) == 0) {
                    Command.push_back(MkCmd(CmdFunc, OpStack.top().addr));
                    OpStack.pop();
            }

            if (token.type == TokComma) // "," is equivalent to ")("
                OpStack.push(Token(TokOpen, "("));    
        }

        else if (token.type == TokEndSub) { // end of a subroutine
            // empty operation stack
            while (!OpStack.empty()) {
                Command.push_back(MkCmd(CmdOper, OpStack.top().addr));
                OpStack.pop();
            }

            size_t addr;
            if (FindSymbol(VarName, TargetVar, &addr)) {
                Command.push_back(MkCmd(CmdWriteVar, addr));
                TargetVar.clear();
            } else {
                ErrorString = TargetVar.empty() ? std::string("Extra ';'") :
                                                  std::string("Can not write: unknown variable " + TargetVar);
                return false;                 
            }
        }

        else if (token.type == TokEnd) {
            if (par_level != 0) {
                ErrorString = std::string("Unbalanced ") + std::string(par_level, '(');
                return false;    
            }
            break;
        }
        LastToken = token;
    }

    while (!OpStack.empty()) {
        Command.push_back(MkCmd(CmdOper, OpStack.top().addr));
        OpStack.pop();
    }
    Command.push_back(MkCmd(CmdReturn, 0));
    return true;
}

bool VParser::CheckSyntax(Token token)
{
    TokenType cur = token.type;
    TokenType last = LastToken.type;
    if(cur == TokOper && last == TokOper) {
        ErrorString = "Missing Operand";
        return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokOpen || cur == TokFunc) && 
        (last == TokConst || last == TokVar || last == TokNumber)) {
            ErrorString = "Missing Operator";
            return false;
    }
    if ((cur == TokConst || cur == TokVar || cur == TokNumber || cur == TokFunc) && 
        (last == TokClose)) {
            ErrorString = "Missing Operator";
            return false;
    }
    return true;
}

bool VParser::FindSymbol(std::vector <std::string> &namevec, std::string symbol, size_t *addr)
{
    std::vector <std::string> :: iterator itr;

    itr = std::find(namevec.begin(), namevec.end(), symbol);
    if (itr == namevec.end()) 
        return false;

    *addr = itr-namevec.begin();
    return true;
}

size_t VParser::AddOperation(std::string name, std::string mnem, int rank, int args)
{
    OperName.push_back(name);
    OperMnem.push_back(mnem);
    OperRank.push_back(rank);
    OperArgs.push_back(args);
    return OperName.size()-1;
}

size_t VParser::AddFunction(std::string name, std::string mnem, int args) 
{
    FuncName.push_back(name);
    FuncMnem.push_back(mnem);
    FuncArgs.push_back(args);
    return FuncName.size()-1;
}

// two types of constants:
//  * named - these are reusable; they either come preset like pi or created by user via AddConstant()
//  * auto - reset each time the parser runs; used to store the numbers from the formula
//  ConstName.size() gives the address of the first auto constant
bool VParser::AddConstant(std::string name, double val)
{
    size_t addr;
    // make sure there is no name clash with a variable or a function
    if (FindSymbol(VarName, name, &addr)) {
        ErrorString = "Can not add constant '" + name + "': variable with this name already exists";
        return false; 
    } else if (FindSymbol(FuncName, name, &addr)) {
        ErrorString = "Can not add constant '" + name + "': function with this name already exists";
        return false;
    }

    if (FindSymbol(ConstName, name, &addr)) { // if the constant with this name already exists - update it
        Const[addr] = val;
        return true;
    }
// otherwise create a new one      
    ConstName.push_back(name);
    Const.push_back(val);
    return true;
}

size_t VParser::AddAutoConstant(double val)
{

    std::vector <double> :: iterator itr = std::find(Const.begin() + ConstName.size(), Const.end(), val);
    if (itr != Const.end()) // if an auto constant with the same value already exists
        return itr-Const.begin(); // use it

// otherwise create a new one      
    Const.push_back(val);
    return Const.size()-1;
}

// remove all automatically generated (i.e. nameless) constants
void VParser::PruneConstants()
{
    Const.resize(ConstName.size());
}

bool VParser::AddVariable(std::string name) 
{
    size_t addr;
    // make sure there is no name clash with a constant or a function
    if (FindSymbol(ConstName, name, &addr)) {
        ErrorString = "Can not add variable '" + name + "': variable with this name already exists";
        return false; 
    } else if (FindSymbol(FuncName, name, &addr)) {
        ErrorString = "Can not add constant '" + name + "': function with this name already exists";
        return false;
    }

// add if it's not there already 
    if (!FindSymbol(VarName, name, &addr))
        VarName.push_back(name);

    return true;
}

double VParser::GetConstant(std::string name)
{
    size_t addr;
    return FindSymbol(ConstName, name, &addr) ? Const[addr] : nan("");
}

double VParser::GetConstant(size_t addr)
{
    if (addr<0 || addr>=Const.size())
        return nan("");
    return Const[addr];    
}

bool VParser::SetConstant(std::string name, double val)
{
    size_t addr;
    bool status = FindSymbol(ConstName, name, &addr);
    if (status)
        Const[addr] = val;
    return status;     
}

bool VParser::SetConstant(size_t addr, double val)
{
    if (addr<0 || addr>=Const.size())
        return false;
    Const[addr] = val;
    return true;     
}



