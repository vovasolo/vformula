#include "vformula.h"
#include <iostream>
#include <string>
#include <cmath>
#include <Eigen/Dense>

Eigen::MatrixXd hstack(Eigen::MatrixXd A, Eigen::MatrixXd B)
{
    Eigen::MatrixXd C(A.rows(), A.cols()+B.cols());
    C << A, B;
    return C;
}

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

    int pts = 21;
    double xmin = -10.;
    double xmax = 10.;

    Eigen::ArrayXd x(pts), y(pts);
    x.setLinSpaced(pts, xmin, xmax);
    y = vf.Eval(x);

    std::cout << hstack(x, y) << std::endl;

    std::cout << "Done!\n";

    return 0;
}
