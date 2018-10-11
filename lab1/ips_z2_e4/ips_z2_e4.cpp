// ips_z2_e4.cpp: определ€ет точку входа дл€ консольного приложени€.
//

#include "stdafx.h"

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>

#include <chrono>
#include <vector>

using namespace std::chrono;

/// ‘ункци€ CompareForAndCilk_For
/// size - размер запол€емого массива
void CompareForAndCilk_For(size_t size)
{
	constexpr int max_value = 20000;
	printf("Size %d \t", size);

	// «аполн€ем обычный вектор
	std::vector<int> vec{};
	vec.reserve(size);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < size; ++i)
	{
		vec.push_back(rand() % max_value + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration1 = (t2 - t1);
	printf("For - %f  :", duration1.count());

	// «аполн€ем вектор параллельно
	cilk::reducer<cilk::op_vector<int>>red_vec;

	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(long i = 0; i < size; ++i)
	{
		red_vec->push_back(rand() % max_value + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();

	duration<double> duration2 = (t4 - t3);
	printf("  %f - Cilk_For\n", duration2.count());
}


int main()
{
	printf("Compare For and Cilk_For test.\n\n");
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	std::vector<size_t> sizes{ 1000000, 100000, 10000, 1000, 500, 100, 50, 10 };
	for (auto size : sizes)
	{
		CompareForAndCilk_For(size);
	}

	return 0;
}

