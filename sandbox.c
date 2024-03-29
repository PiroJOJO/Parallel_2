#include <iostream>
#include <algorithm>
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
inline double steps(size_t n, size_t step, double left, double right)
{
    double val = (right - left)/(n - 1);
    return left +  val * step;
}

int main(int argc, char *argv[]) {
    auto begin = std::chrono::steady_clock::now();
    if (argc != 7)
    {
        std::cout<<"Enter a string like this: Accuracy _ iterations _ size _"<<std::endl;
    }
    //Считывание значений с командной строки
    double error = std::stod(argv[2]);
    size_t iter = std::stoi(argv[4]);
    size_t n = std::stoi(argv[6]);
    double* vec = new double[n*n];
    double* new_vec = new double[n*n];
    size_t it = 0;
    //Заполнение угловых значений
    get_element(vec, n, 0, 0) = 10;
    get_element(vec, n, n - 1, 0) = 20;
    get_element(vec, n, n - 1, n - 1) = 30;
    get_element(vec, n, 0, n - 1) = 20;
    //Заполнение рамок матриц
#pragma acc data copy(vec[0:n*n], new_vec[0:n*n])
    {
#pragma acc parallel loop
        for (size_t i = 1; i < n - 1; ++i) {
            get_element(vec, n, i, 0) = steps(n, i, get_element(vec, n, 0, 0),
                                              get_element(vec, n, n - 1, 0));
            get_element(vec, n, i, n - 1) = steps(n, i, get_element(vec, n, 0, n - 1),
                                                  get_element(vec, n, n - 1, n - 1));
            get_element(vec, n, 0, i) = steps(n, i, get_element(vec, n, 0, 0),
                                              get_element(vec, n, 0, n - 1));
            get_element(vec, n, n - 1, i) = steps(n, i, get_element(vec, n, n - 1, 0),
                                                  get_element(vec, n, n - 1, n - 1));
        }
        std::copy_n(vec, n * n, new_vec);
// #pragma acc parallel loop
//         for (int i = 0; i < n; ++i) 
//         {
//                 get_element(new_vec, n, n - 1, i) = get_element(vec, n, n - 1, i);
//                 get_element(new_vec, n, i, n - 1) = get_element(vec, n, i, n - 1);
//                 get_element(new_vec, n, 0, i) = get_element(vec, n, 0, i);
//                 get_element(new_vec, n, i, 0) = get_element(vec, n, i, 0);
//         }
        double max_error = error + 1;             
        while(error < max_error && it < iter)
	    { 
                max_error = 0;
                it ++;
#pragma acc data copy(max_error)              
#pragma acc parallel loop collapse(2) independent reduction(max:max_error)
                for (int j = 1; j < n - 1; ++j)
                {
                    for (int k = 1; k < n - 1; ++k)
                    {
                        get_element(new_vec, n, k, j) =
                                0.25 * (get_element(vec, n, k - 1, j) + get_element(vec, n, k + 1, j)
                                        + get_element(vec, n, k, j - 1) + get_element(vec, n, k, j + 1));
                        max_error = std::max(max_error,
                                             std::abs(get_element(vec, n, k, j) - get_element(new_vec, n, k, j)));
                    }
                }
                #pragma acc parallel loop collapse(2)
	                for (int j = 1; j < n - 1; ++j) 
                        for (int k = 1; k < n - 1; ++k) 
                            get_element(vec, n, k, j) = get_element(new_vec, n, k, j);

            }
            std::cout<<"Error: "<<max_error<<std::endl;
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
    std::cout<<"time: "<<elapsed_ms.count()<<" mcs\n";
    std::cout<<"Iterations: "<<it<<std::endl;
    //print_matrix(vec, n);
    delete [] vec;
    delete [] new_vec;
    return 0;
}
