#include <mpi/mpi.h>
#include <iostream>
#include <vector>
#include <random>

using namespace std;

// Объявление функций
void Solve(int rank, int n, vector<int> &b, const vector<vector<int>> &a);

int main(int argc, char **argv) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 40);

    int n;
    vector<vector<int>> a;
    vector<int> b;

// Инициализация MPI
    MPI_Init(&argc, &argv);

// Получение количества процессов
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

// Получение ранга процесса
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

// Чтение данных из файла
    if (rank == 0) {
        a.resize(n);
        for (int i = 0; i < n; i++) {
            a[i].resize(n);
            a[i][i] = 0;
            for (int j = i + 1; j < n; j++) {
                int randomValue = dis(gen);
                a[j][i] = randomValue;
                a[i][j] = randomValue;
            }
        }

        for (int j = 0; j < n; j++) {
            std::cout << "{";
            for (int i = 0; i < n; i++) {
                std::cout << a[j][i];
                if (i != n)
                    std::cout << ", ";
                else
                    std::cout << "},";
            }
            std::cout << std::endl;
        }
    }

// Распределение данных между процессами
    int data_size = n * (n - 1) / 2;
    int data_per_process = data_size / world_size;
    int start_index = rank * data_per_process;
    int end_index = (rank + 1) * data_per_process;

// Обработка данных
    Solve(rank, n, b, a);

// Сбор результатов
    if (rank != 0) {
        MPI_Send(&b[0], n, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
        for (int i = 1; i < world_size; i++) {
            MPI_Recv(&b[start_index + i * data_per_process], data_per_process, MPI_INT, i, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
        }
    }

// Вывод результатов
    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            cout << b[i] << " ";
        }
        cout << endl;
    }

// Завершение MPI
    MPI_Finalize();

    return 0;
}

// Решение задачи
void Solve(int rank, int n, vector<int> &b, const vector<vector<int>> &a) {
// Создание массива, в котором будут храниться расстояния между соседними городами
    vector<int> d(n - 1);
    for (int i = 0; i < n - 1; i++) {
        d[i] = a[i][i + 1];
    }

// Обход массива d в лексикографическом порядке
    for (int i = 0; i < n - 2; i++) {
        for (int j = i + 1; j < n - 1; j++) {
            if (d[i] > d[j]) {
                swap(d[i], d[j]);
            }
        }
    }

// Формирование пути
    for (int i = 0; i < n; i++) {
        b[i] = i + 1;
    }

// Обход массива d в обратном лексикографическом порядке
    for (int i = n - 2; i >= 0; i--) {
        b[i] = d[i];
    }
}