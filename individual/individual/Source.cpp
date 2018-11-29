#include <vector>
#include <stdio.h>
#include <cilk/cilk_api.h>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>

#include <chrono>

#define ITERATIONS 1000

namespace
{
    std::chrono::duration<double> duration_s{0.0};
    std::chrono::duration<double> duration_p{ 0.0 };
}


double CalcIntegral(double beg, double end, std::function<double(double)> func, int N = 10)
{
    double res = 0.0;
    double h = (end - beg) / N;
    double x = 0.;

    for (int i = 0; i <= N; ++i)
        res += func(beg + i*h);

    return res;
}


double SerialShell(double beg, double end, std::function<double(double)> func, int N = 10)
{
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    double res = 0;
    for (int i = 0; i < ITERATIONS; ++i)
        res = CalcIntegral(beg, end, func, N);

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    duration_s = (t2 - t1);

    return res;
}


double CalcIntegral_paralel(double beg, double end, std::function<double(double)> func, int N = 10)
{
    cilk::reducer_opadd<double> res(0.0);
    double h = (end - beg) / N;
    double x = 0.;

    cilk_for(int i = 0; i <= N; ++i)
    {
        res += func(beg + i*h);
    }

    return res.get_value();
}


double ParalelShell(double beg, double end, std::function<double(double)> func, int N = 10)
{
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    double res = 0;
    for (int i = 0; i < ITERATIONS; ++i)
        res = CalcIntegral_paralel(beg, end, func, N);

    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    duration_p = (t2 - t1);

    return res;
}


int main()
{
    // устанавливаем количество работающих потоков = 4
    __cilkrts_set_param("nworkers", "4");

    const double beg = -1.;
    const double end = 1.;

    auto fnFunction = [](double x) -> double
                      {
                          return 5. / std::sqrt(8 - 4*x*x);
                      };
    std::vector<int> vals = { 10, 100, 1000, 10000 };

    for (auto && val : vals)
    {
        double res = SerialShell(beg, end, fnFunction, val);
        double resP = ParalelShell(beg, end, fnFunction, val);

        printf("Number of breaks: %d.\t Result: %f. Serial time -\t %f, paralel time - \t %f\n", val, res, duration_s.count(), duration_p.count());
    }

    return 0;
}