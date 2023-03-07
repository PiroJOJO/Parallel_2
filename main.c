#include <iostream>
#include <vector>
#include <array>
#include <chrono>


#pragma acc routine seq
double& get_element(double* vec, size_t n, int i, int i2)
{
    return vec[i + n * i2];
}

void print_matrix(double* vec, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            std::cout<<get_element(vec, n, j, i)<<' ';
        }
        std::cout<<std::endl;
    }
}

#pragma acc routine seq
inline double steps(double* vec, size_t n, size_t step, double left, double right)
{
    double val = (right - left)/(n - 1);
    return left +  val * step;
}

int main(int argc, char *argv[])
{

    auto begin = std::chrono::steady_clock::now();
    size_t n = std::stoi(argv[1]);
    double* vec = new double[n*n];
    get_element(vec, n, 0, 0) = 10;
    get_element(vec, n, n - 1, 0) = 20;
    get_element(vec, n, n - 1, n - 1) = 30;
    get_element(vec, n, 0, n - 1) = 20;
    double err = 0;

#pragma acc data copy(vec[:n*n])
{
#pragma acc parallel loop
    for (size_t i = 1; i < n - 1; ++i)
    {
        get_element(vec, n, i, 0) = steps(vec, n, i, get_element(vec, n, 0, 0), get_element(vec, n, n - 1, 0));
        get_element(vec, n, i, n - 1) = steps(vec, n, i, get_element(vec, n, 0, n - 1), get_element(vec, n, n - 1, n - 1));
    }
#pragma acc data copy (err)
    {
#pragma acc parallel loop collapse(2) reduction(max:err)
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 1; j < n - 1; ++j)
        {
		double old_val = get_element(vec, n, j, i);
            get_element(vec, n, i, j) = steps(vec, n, j, get_element(vec, n, i, 0), get_element(vec, n, i, n - 1));
	    err = std::max(std::abs(get_element(vec, n, j, i) - old_val), err);
        }
    }
    }
}

    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
    std::cout<<"time: "<<elapsed_ms.count()<<" mcs\n";
    std::cout<<"error: "<<err<<std::endl;
   // print_matrix(vec, n);

    delete[] vec;
    return 0;
}

