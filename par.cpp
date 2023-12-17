#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include "mpi/mpi.h"

using namespace std;

int fuck(int n) {
    return (n == 1) ? 1 : n * fuck(n - 1);
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 40);
    int size, rank;

    MPI_Init(nullptr, nullptr);
    MPI_Status stat;
    MPI_Request request;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int n = 10;

    int** dm = new int*[n + 1];
    dm[0] = new int[(n + 1) * (n + 1)];
    for (int i = 1; i < n + 1; i++)
        dm[i] = dm[i - 1] + n + 1;

    if (rank == 0) {
        for (int i = 0; i < n + 1; i++) {
            dm[i][i] = 0; // set diagonal to 0
            for (int j = i + 1; j < n + 1; j++) {
                int randomValue = dis(gen);
                dm[i][j] = randomValue;
                dm[j][i] = randomValue; // set symmetric value
            }
        }

        for (int j = 0; j < n + 1; j++) {
            std::cout << "{";
            for (int i = 0; i < n + 1; i++) {
                std::cout << dm[j][i];
                if (i != n)
                    std::cout << ", ";
                else
                    std::cout << "},";
            }
            std::cout << std::endl;
        }
    }

    MPI_Bcast(*dm, (n + 1) * (n + 1), MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> route(n);
    vector<int> end(n);
    for (int i = 0; i < n; i++) {
        route[i] = i + 1;
        end[i] = -1;
    }
    vector<int> minRoute = route;
    int minDist = INT32_MAX;

    double start = MPI_Wtime();

    if (rank == 0) {

        int k = 0;
        do {
            int r = k % (size - 1) + 1;

            MPI_Send(&route[0], n, MPI_INT, r, 777, MPI_COMM_WORLD);
            k++;
        } while (next_permutation(route.begin(), route.end()));

        for (int i = 1; i < size; i++)
            MPI_Send(&end[0], n, MPI_INT, i, 777, MPI_COMM_WORLD);
    } else {

        int sum;
        while (true) {
            int *pos = new int[n];

            MPI_Recv(pos, n, MPI_INT, 0, 777, MPI_COMM_WORLD, &stat);

            if (pos[0] == -1)
                break;

            sum = dm[0][pos[0]];
            for (int i = 1; i < n; i++)
                sum += dm[pos[i - 1]][pos[i]];
            sum += dm[pos[n - 1]][0];

            if (sum < minDist) {
                minDist = sum;

                for (int i = 0; i < n; i++)
                    minRoute[i] = pos[i];
            }
        }

        minRoute.insert(minRoute.begin(), 0);
    }

    MPI_Gather(&minRoute[0], n + 1, MPI_INT, dm[0], n + 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gather(&minDist, 1, MPI_INT, &end[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int minIndex = 0;
        for (int i = 1; i < size; i++) {
            if (end[i] < end[minIndex])
                minIndex = i;
        }

        double final = MPI_Wtime();

        cout << "\nMin route = ";
        for (int i = 0; i < n + 1; i++)
            cout << dm[minIndex][i] << ' ';
        cout << "; min way length = " << end[minIndex] << endl;
        cout << "\nTime spent = " << final - start << endl;
    }

    MPI_Finalize();
    return 0;
}