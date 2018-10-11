// paralel_test.cpp: ���������� ����� ����� ��� ����������� ����������.
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

/// ������� ReducerMaxTest() ���������� ������������ ������� �������,
/// ����������� �� � �������� ���������, � ��� �������
/// mass_pointer - ��������� �������� ������ ����� �����
/// size - ���������� ��������� � �������
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n",
		maximum->get_reference(), maximum->get_index_reference());
}


/// ������� ReducerMinTest() ���������� ����������� ������� �������,
/// ����������� �� � �������� ���������, � ��� �������
/// mass_pointer - ��������� �������� ������ ����� �����
/// size - ���������� ��������� � �������
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n",
		minimum->get_reference(), minimum->get_index_reference());
}


/// ������� ParallelSort() ��������� ������ � ������� �����������
/// begin - ��������� �� ������ ������� ��������� �������
/// end - ��������� �� ��������� ������� ��������� �������
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}


/// ������� CompareForAndCilk_For 
void CompareForAndCilk_For(size_t size)
{
	constexpr int max_value = 20000;

	// ��������� ������� ������
	std::vector<int> vec{};
	vec.reserve(size);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (long i = 0; i < size; ++i)
	{
		vec.push_back(rand() % max_value + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration1 = (t2 - t1);
	printf("Duration is: %f seconds\n", duration1.count());

	// ��������� ������ �����������
	cilk::reducer<cilk::op_vector<int>>red_vec;

	high_resolution_clock::time_point t3 = high_resolution_clock::now();
	cilk_for(long i = 0; i < size; ++i)
	{
		red_vec->push_back(rand() % max_value + 1);
	}
	high_resolution_clock::time_point t4 = high_resolution_clock::now();

	duration<double> duration2 = (t3 - t4);
	printf("Duration is: %f seconds\n", duration2.count());
}


int main()
{
	srand((unsigned)time(0));

	// ������������� ���������� ���������� ������� = 4
	__cilkrts_set_param("nworkers", "4");

	constexpr long mass_size = 1000000;

	printf("\nNumber of elements in array = %d\n\n", mass_size);
	int *mass = new int[mass_size];
	for (long i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	int *mass_begin = mass;
	int *mass_end = mass_begin + mass_size;

	// ����� �� ���������������� �������
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	// ���������� � ��������� �������
	printf("\nSome sorting is happening!\n");

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	printf("Duration is: %f seconds\n\n", duration.count());

	// ����� �� �������������� �������
	ReducerMaxTest(mass, mass_size);
	ReducerMinTest(mass, mass_size);

	delete[] mass;
	return 0;
}