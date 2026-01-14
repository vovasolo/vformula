#include "vformula.h"
#include <iostream>
#include <string>
#include <cmath>
#include <chrono>
#include <Eigen/Dense>

int main(int argc, char **argv)
{
    if (argc !=2) {
        std::cout << "Usage example: " << argv[0] << " \"2*sin(x/10*pi)\"\n";
        return -1;
    }

    VFormula <Eigen::ArrayXd> vf;
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

// timed run
    int vlen = 100;         // vector length, i.e number of evals in one go
    int nevals = 10000000;  // total number of evals tor run
    int nreps = nevals/vlen;// number of repetitions
    double sum =  0.;
    Eigen::ArrayXd x = Eigen::ArrayXd::Constant(vlen, 0.);

    std::cout << "Timed run: vector of " << vlen << " variables, repeated " << nreps << " times\n";
    auto start = std::chrono::high_resolution_clock::now();

    //  Code to be timed
    for (int i=0; i<nreps; i++) {
        x[0] = i*1e-6;
        sum += vf.Eval(x)[0];
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << sum << std::endl;
    auto diff = end - start;
    std::cout << std::chrono::duration <double, std::nano> (diff).count()/nevals << " ns/eval" << std::endl;
    return 0;
}
