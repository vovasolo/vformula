#include "vformula.h"
#include <iostream>
#include <string>
#include <cmath>
#include <cstdio>

int main(int argc, char **argv)
{
    if (argc !=2) {
        std::cout << "Usage example: " << argv[0] << " \"2*sin(x/10*pi)\"\n";
        return -1;
    }

    VFormula <double> vf;
    vf.AddConstant("pi", M_PI);
    vf.AddVariable("x");

    std::string f(argv[1]);
    std::cout << "Expression to evaluate: " << f << std::endl;

    int errpos = vf.ParseExpr(f);
    if (errpos != 1024) {
        std::cout << "Parsing error: " << vf.GetErrorString().c_str() << std::endl;
        std::cout << f << std::endl;
        std::cout << (std::string(errpos, ' ')+ "^").c_str() << std::endl;
        return -2;
    }
    if (!vf.Validate()) {
        std::cout << "Validation failed: " << vf.GetErrorString().c_str() << "\n\n";
    }

    std::cout << "Byte code:\n";
    std::vector <std::string> prg = vf.GetPrg();
    for (auto &cmd : prg)
        std::cout << cmd << std::endl;

    std::cout << "\nConstants:\n";
    std::vector <std::string> cmap = vf.GetConstMap();
    for (int i=0; i<cmap.size(); i++) {
        char buf[8];
        sprintf(buf, "%02d: ", i);
        std::cout << buf << cmap[i] << std::endl;
    }

    std::cout << "\nVariables:\n";
    std::vector <std::string> vmap = vf.GetVarMap();
    for (int i=0; i<vmap.size(); i++) {
        char buf[8];
        sprintf(buf, "%02d: ", i);
        std::cout << buf << vmap[i] << std::endl;
    }
/* Uncomment to also get a list of available functions
    std::cout << "\nFunctions:\n";
    std::vector <std::string> fmap = vf.GetFuncMap();
    for (int i=0; i<fmap.size(); i++) {
        char buf[8];
        sprintf(buf, "%02d: ", i);
        std::cout << buf << fmap[i] << std::endl;
    }
*/
    return 0;
}
