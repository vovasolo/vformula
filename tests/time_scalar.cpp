#include "vformula.h"
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>

int main(int argc, char **argv)
{
//    if (argc !=2) {
//        std::cout << "Usage example: " << argv[0] << " \"2*sin(x/10*pi)\"\n";
//        return -1;
//    }

    VFormula <double> vf;
    vf.AddConstant("pi", M_PI);
    vf.AddVariable("x");

//    std::string f(argv[1]);
    std::string f("t=x^2;2*sin(x/10*pi)+t/10");

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

// timed run
    int nevals = 10000000;  // total number of evals tor run
    std::cout << "Timed run: " << nevals << " evaluations\n";
    double sum =  0.;

    auto start = std::chrono::high_resolution_clock::now();

//  Code to be timed
    for (int i=0; i<nevals; i++) {
        sum += vf.Eval(i*1e-6);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << sum << std::endl;
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::nano> (diff).count()/nevals << " ns/eval" << std::endl;
    return 0;
}
