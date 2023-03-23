#include <iostream>
#include <algorithm>
#include <chrono>
#pragma acc routine seq
double steps(size_t n, size_t step, double left, double right)
{
    double val = (right - left)/(n - 1);
    return left +  val * step;
}

double** create_matrix(size_t n)
{
    double** arr = new double*[n];
    for (size_t i =0; i < n; ++i)
    {
        arr[i] = new double[n];
    }
    return arr;
}

void print_matrix(double** vec, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = 0; j < n; ++j)
        {
            std::cout<<vec[i][j]<<' ';
        }
        std::cout<<std::endl;
    }
}

int main(int argc, char *argv[])
{
    auto begin = std::chrono::steady_clock::now();
    if(argc != 7)
    {
        std::cout<<"Enter a string like this: Accuracy _ iterations _ size _"<<std::endl;
    }
    //Считывание значений с командной строки
    double error = std::stod(argv[2]);
    size_t iter = std::stoi(argv[4]);
    size_t n = std::stoi(argv[6]);
    double** vec = create_matrix(n);
    double** new_vec = create_matrix(n);
    vec[0][0] = 10;
    vec[0][n - 1] = 20;
    vec[n - 1][n - 1] = 30;
    vec[n - 1][0] = 20;
    size_t it = 0;
#pragma acc data copy(vec[0:n][0:n], new_vec[0:n][0:n])
    {
#pragma acc parallel loop independent
        for (size_t i = 0; i < n; ++i) 
        {
            vec[0][i] = steps(n, i, vec[0][0], vec[0][n - 1]);
            vec[i][n - 1] = steps(n, i, vec[0][n - 1], vec[n - 1][n - 1]);
            vec[n - 1][i] = steps(n, i, vec[n - 1][0], vec[n - 1][n - 1]);
            vec[i][0] = steps(n, i, vec[0][0], vec[n - 1][0]);
        }
#pragma acc parallel loop independent
        for (size_t i = 0; i < n; ++i) 
        {
            new_vec[0][i] = vec[0][i];
            new_vec[n - 1][i] = vec[n - 1][i];
            new_vec[i][0] = vec[i][0];
            new_vec[i][n - 1] = vec[i][n - 1];
        }
	double max_error = error + 1;
	while(error < max_error && it < iter)
	{  
		max_error = 0;
		it++;

#pragma acc data copy(max_error)
#pragma acc parallel loop collapse(2) independent reduction(max:max_error)
            for (int j = 1; j < n - 1; ++j) 
            {
                for (int k = 1; k < n - 1; ++k) 
                {
                    new_vec[j][k] = 0.25 * (vec[j - 1][k] + vec[j + 1][k] + vec[j][k - 1] + vec[j][k + 1]);
                    max_error = std::max(max_error,
                                         std::abs(vec[j][k] - new_vec[j][k]));
                }
                //std::copy_n(new_vec[j],n, vec[j]);
            }
 
#pragma acc parallel loop independent
	        for (size_t j = 0; j < n; ++j) 
            {
                    double* tmp = vec[j];
                    vec[j] = new_vec[j];
                    new_vec[j] = tmp;
            }
    }
   
    std::cout<<"Error: "<<max_error<<std::endl;
    }

    auto end = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::microseconds>(end-begin);
    std::cout<<"time: "<<elapsed_ms.count()<<" mcs\n";
    std::cout<<"Iterations: "<<it<<std::endl;
    //print_matrix(vec, n);
    for (size_t i = 0; i < n; i++)
    {
        delete [] vec[i];
        delete [] new_vec[i];
    }
    delete [] vec;
    delete [] new_vec;
    return 0;
}

