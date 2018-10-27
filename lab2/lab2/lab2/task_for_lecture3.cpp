#include <stdio.h>
#include <ctime>
#include <cilk/cilk.h>
#include <cilk/reducer_opadd.h>
#include <chrono>


#include <cilk/cilk_api.h>
#include <cilk/reducer_vector.h>
#include <cilk/reducer_list.h>
#include <cilk/reducer.h>

using namespace std::chrono;

// количество строк в исходной квадратной матрице
constexpr int MATRIX_SIZE = 3000;
// переключение тестового и рабочего режимов
constexpr bool TEST_MODE = false;

namespace
{
    // затраченные на прямой проход метода Гаусса времена
    duration<double> serialDuration{};
    duration<double> parallelDuration{};
}

/// Функция InitMatrix() заполняет переданную в качестве 
/// параметра квадратную матрицу случайными значениями
/// matrix - исходная матрица СЛАУ
void InitMatrix( double** matrix )
{
	for ( int i = 0; i < MATRIX_SIZE; ++i )
	{
		matrix[i] = new double[MATRIX_SIZE + 1];
	}

	for ( int i = 0; i < MATRIX_SIZE; ++i )
	{
		for ( int j = 0; j <= MATRIX_SIZE; ++j )
		{
			matrix[i][j] = rand() % 2500 + 1;
		}
	}
}

void InitTestMatrix(double** test_matrix)
{
    // инициализация тестовой матрицы
    test_matrix[0][0] = 2; test_matrix[0][1] = 5;  test_matrix[0][2] = 4;  test_matrix[0][3] = 1;  test_matrix[0][4] = 20;
    test_matrix[1][0] = 1; test_matrix[1][1] = 3;  test_matrix[1][2] = 2;  test_matrix[1][3] = 1;  test_matrix[1][4] = 11;
    test_matrix[2][0] = 2; test_matrix[2][1] = 10; test_matrix[2][2] = 9;  test_matrix[2][3] = 7;  test_matrix[2][4] = 40;
    test_matrix[3][0] = 3; test_matrix[3][1] = 8;  test_matrix[3][2] = 9;  test_matrix[3][3] = 2;  test_matrix[3][4] = 37;
}

void InitMainMatrix(double** matrix)
{
    if (TEST_MODE)
    {
        InitTestMatrix(matrix);
    }
    else
    {
        InitMatrix(matrix);
    }
}

/// Функция SerialGaussMethod() решает СЛАУ методом Гаусса 
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
void SerialGaussMethod( double **matrix, const int rows, double* result )
{
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
	// прямой ход метода Гаусса
	for ( int k = 0; k < rows; ++k )
	{
		//
		for ( int i = k + 1; i < rows; ++i )
		{
            double koef = -matrix[i][k] / matrix[k][k];

			for ( int j = k; j <= rows; ++j )
			{
				matrix[i][j] += koef * matrix[k][j];
			}
		}
	}
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    serialDuration = (t2 - t1);
    printf("Serial forward Gauss time - %f \n", serialDuration.count());

	// обратный ход метода Гаусса
	result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

	for ( int k = rows - 2; k >= 0; --k )
	{
		result[k] = matrix[k][rows];

		//
		for ( int j = k + 1; j < rows; ++j )
		{
			result[k] -= matrix[k][j] * result[j];
		}

		result[k] /= matrix[k][k];
	}
}


/// Функция ParalelGaussMethod() решает СЛАУ методом Гаусса 
/// matrix - исходная матрица коэффиициентов уравнений, входящих в СЛАУ,
/// последний столбей матрицы - значения правых частей уравнений
/// rows - количество строк в исходной матрице
/// result - массив ответов СЛАУ
void ParallelGaussMethod(double **matrix, const int rows, double* result)
{
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    // прямой ход метода Гаусса
    for (int k = 0; k < rows; ++k)
    {
        //
        cilk_for (int i = k + 1; i < rows; ++i)
        {
            double koef = -matrix[i][k] / matrix[k][k];

            for (int j = k; j <= rows; ++j)
            {
                matrix[i][j] += koef * matrix[k][j];
            }
        }
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    parallelDuration = (t2 - t1);
    printf("Parallel forward Gauss time - %f \n", parallelDuration.count());

    // обратный ход метода Гаусса
    result[rows - 1] = matrix[rows - 1][rows] / matrix[rows - 1][rows - 1];

    for (int k = rows - 2; k >= 0; --k)
    {
        //result[k] = matrix[k][rows];
        cilk::reducer_opadd<double> result_k(matrix[k][rows]);

        //
        cilk_for (int j = k + 1; j < rows; ++j)
        {
            //result[k] -= matrix[k][j] * result[j];
            result_k -= matrix[k][j] * result[j];
        }

        //result[k] /= matrix[k][k];
        result[k] = result_k->get_value() / matrix[k][k];
    }
}


int main()
{
	srand( (unsigned) time( 0 ) );

    __cilkrts_set_param("nworkers", "4");

	// кол-во строк в матрице, приводимой в качестве примера
	const int matrix_lines = TEST_MODE ? 4 : MATRIX_SIZE;

	double **matrix = new double*[matrix_lines];

	// цикл по строкам
	for ( int i = 0; i < matrix_lines; ++i )
	{
		// (test_matrix_lines + 1)- количество столбцов в тестовой матрице,
		// последний столбец матрицы отведен под правые части уравнений, входящих в СЛАУ
		matrix[i] = new double[matrix_lines + 1];
	}

	// массив решений СЛАУ
	double *result  = new double[matrix_lines];

    // Инициализация матрицы системы уравнений
    

    // Решение
    InitMainMatrix(matrix);
	SerialGaussMethod( matrix, matrix_lines, result );

    InitMainMatrix(matrix);
    ParallelGaussMethod( matrix, matrix_lines, result );

    printf("Acceleration for %dx%d matrix - %f \n",
        matrix_lines, matrix_lines, serialDuration.count() / parallelDuration.count());
    if (TEST_MODE)
    {
        printf("Solution:\n");
        for (int i = 0; i < matrix_lines; ++i)
        {
            printf("x(%d) = %lf\n", i, result[i]);
        }
    }

    // очистка ресурсов
	for ( int i = 0; i < matrix_lines; ++i )
	{
		delete[] matrix[i];
	}

    delete[] matrix;
	delete[] result;

	return 0;
}