#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
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

int main() {
    MPI_Init(nullptr, nullptr);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 40);

    int n = 13;

    std::vector<std::vector<int>> distanceMatrix(n, std::vector<int>(n)); /*= {
            {0, 16, 14, 26, 27},
            {16, 0, 17, 30, 26},
            {14, 17, 0, 1, 12},
            {26, 30, 1, 0, 26},
            {27, 26, 12, 26, 0},
    };*/

    for (int i = 0; i < n; i++) {
        distanceMatrix[i][i] = 0; // set diagonal to 0
        for (int j = i + 1; j < n; j++) {
            int randomValue = dis(gen);
            distanceMatrix[i][j] = randomValue;
            distanceMatrix[j][i] = randomValue; // set symmetric value
        }
    }

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