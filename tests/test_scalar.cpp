#include "vformula.h"
#include <iostream>
#include <string>
#include <cmath>

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
        std::cout << "Validation failed: " << vf.GetErrorString().c_str() << std::endl;
        return -3;
    }

    for (double x=-10.; x<10.1; x+=1.0) {
        std::cout << x << " \t" << vf.Eval(x) << std::endl;
    }
    return 0;
}
