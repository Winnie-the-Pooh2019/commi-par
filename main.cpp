#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <bits/stdc++.h>
#include "mpi/mpi.h"

class TravellingSalesman {
public:
    static std::pair<int, std::vector<int>> findOptimalRoute(std::vector<std::vector<int>>& distanceMatrix) {
        int n = distanceMatrix.size() - 1;
        std::vector<int> route(n);
        for (int i = 0; i < n; i++) {
            route[i] = i + 1;
        }

        std::vector<int> minRoute(n);
        int minDistance = INT32_MAX;

        do {
            int currentDistance = distanceMatrix[0][route[0]];
            for (int i = 1; i < n; i++)
                currentDistance += distanceMatrix[route[i - 1]][route[i]];
            currentDistance += distanceMatrix[route[n - 1]][0];

            if (currentDistance < minDistance) {
                minDistance = currentDistance;
                minRoute = route;
            }

        } while (std::next_permutation(route.begin(), route.end()));

        return {minDistance, minRoute};
    }
};

int main(int argc, char *argv[]) {
    MPI_Init(nullptr, nullptr);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 40);

    int n = std::stoi(argv[1]);

    std::vector<std::vector<int>> distanceMatrix/*(n, std::vector<int>(n));*/ = {
            {0, 37, 32, 18, 22, 4, 3, 15, 23, 28, 27, 23},
            {37, 0, 5, 19, 13, 22, 13, 39, 36, 22, 17, 29},
            {32, 5, 0, 24, 40, 31, 26, 12, 30, 28, 31, 19},
            {18, 19, 24, 0, 19, 3, 4, 21, 36, 33, 15, 14},
            {22, 13, 40, 19, 0, 31, 37, 5, 23, 21, 29, 36},
            {4, 22, 31, 3, 31, 0, 4, 35, 21, 32, 34, 16},
            {3, 13, 26, 4, 37, 4, 0, 28, 31, 16, 40, 7},
            {15, 39, 12, 21, 5, 35, 28, 0, 37, 34, 9, 22},
            {23, 36, 30, 36, 23, 21, 31, 37, 0, 38, 34, 38},
            {28, 22, 28, 33, 21, 32, 16, 34, 38, 0, 24, 39},
            {27, 17, 31, 15, 29, 34, 40, 9, 34, 24, 0, 24},
            {23, 29, 19, 14, 36, 16, 7, 22, 38, 39, 24, 0}
    };

//    for (int i = 0; i < n; i++) {
//        distanceMatrix[i][i] = 0; // set diagonal to 0
//        for (int j = i + 1; j < n; j++) {
//            int randomValue = dis(gen);
//            distanceMatrix[i][j] = randomValue;
//            distanceMatrix[j][i] = randomValue; // set symmetric value
//        }
//    }

    // Print generated matrix
    for (const auto &row : distanceMatrix) {
        std::cout << "[";
        for (int i = 0; i < row.size(); i++) {
            std::cout << row[i];
            if (i != row.size() - 1)
                std::cout << ", ";
            else
                std::cout << ']';
        }
        std::cout << std::endl;
    }

    double start = MPI_Wtime();

    auto optimalRoute = TravellingSalesman::findOptimalRoute(distanceMatrix);

    double final = MPI_Wtime();

    std::cout << "Optimal route: ";
    for (int city : optimalRoute.second) {
        std::cout << city << " ";
    }
    std::cout << "\nmin dist = " << optimalRoute.first << std::endl;
    std::cout << "\ntime spent = " << final - start << std::endl;

    MPI_Finalize();
    return 0;
}