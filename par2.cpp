#include <bits/stdc++.h>
#include "mpi/mpi.h"

using namespace std;

vector<int> fact;

vector<int> nth_permutation(vector<int> &arr, long long n) {
    int N = arr.size();

    sort(arr.begin(), arr.end());

    set<int> st;
    for (int x: arr) st.insert(x);

    vector<int> ans;

    for (int i = 0; i < N; i++) {
        int cn = 1;
        int cval = fact[N - 1 - i];

        while (cval < n) {
            cn++;
            cval *= cn;
            cval /= (cn - 1);
        }

        int pval = cval * (cn - 1) / cn;
        n -= pval;

        auto it = st.begin();
        for (int i = 0; i < cn - 1; i++)it++;
        ans.push_back(*it);
        st.erase(it);
    }

    return ans;
}

int findCost(int**& matrix, vector<int>& arr) {
    int cost = matrix[arr[0]][0];

    for (int i = 1; i < arr.size(); i++)
        cost += matrix[arr[i]][arr[i - 1]];

    cost += matrix[0][arr[arr.size() - 1]];

    return cost;
}

int main(int argc, char *argv[]) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1, 40);
    int n = stoi(argv[1]);

    int size;
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    fact.resize(n);
    fact[0] = 1;
    for (int i = 1; i < n; i++)
        fact[i] = i * fact[i - 1];

    int **matrix = new int *[n];
    matrix[0] = new int[n * n];
    for (int i = 1; i < n; i++)
        matrix[i] = matrix[i - 1] + n;

    if (rank == 0) {
        for (int i = 0; i < n; i++) {
            matrix[i][i] = 0;
            for (int j = i + 1; j < n; j++) {
                int randomValue = dis(gen);
                matrix[i][j] = randomValue;
                matrix[j][i] = randomValue;
            }
        }

        for (int j = 0; j < n; j++) {
            std::cout << "{";
            for (int i = 0; i < n; i++) {
                std::cout << matrix[j][i];
                if (i != n - 1)
                    std::cout << ", ";
                else
                    std::cout << "}";
            }
            std::cout << std::endl;
        }
    }

    MPI_Bcast(*matrix, n * n, MPI_INT, 0, MPI_COMM_WORLD);

    double start = MPI_Wtime();

    int optimal_value = INT32_MAX;

    int permPerProc = fact[n - 1] / size;
    int rem = fact[n - 1] % size;

    int startPermInd, endPermInd;

    vector<int> route(n - 1);
    for (int i = 1; i < n; i++)
        route[i - 1] = i;

    if (rem == 0) {
        startPermInd = (rank * permPerProc) + 1;
        endPermInd = (rank + 1) * permPerProc;
    } else {
        if (rank < rem) {
            startPermInd = (rank * (permPerProc + 1)) + 1;
            endPermInd = (rank + 1) * (permPerProc + 1);
        } else {
            startPermInd = rem * (permPerProc + 1) + (rank - rem) * permPerProc + 1;
            endPermInd = rem * (permPerProc + 1) + (rank + 1 - rem) * permPerProc;
        }
    }

    vector<int> minRoute(n + 1);

    vector<int> routeBegin = nth_permutation(route, startPermInd);
    vector<int> routeEnd = nth_permutation(route, endPermInd);

    do {
        int val = findCost(matrix, routeBegin);

        if (val < optimal_value) {
            optimal_value = val;
            minRoute = routeBegin;
        }

        if (routeBegin == routeEnd)
            break;

    } while (next_permutation(routeBegin.begin(), routeBegin.end()));

    MPI_Gather(&minRoute[0], n, MPI_INT, matrix[0], n, MPI_INT, 0, MPI_COMM_WORLD);
    vector<int> answers(size);
    MPI_Gather(&optimal_value, 1, MPI_INT, &answers[0], 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        int minIndex = 0;
        for (int i = 1; i < size; i++) {
            if (answers[i] < answers[minIndex])
                minIndex = i;
        }

        double final = MPI_Wtime();

        cout << "\nMin route = ";
        for (int i = 0; i < n; i++)
            cout << matrix[minIndex][i] << ' ';
        cout << "; min way length = " << answers[minIndex] << endl;
        cout << "\nTime spent = " << final - start << endl;
    }

    MPI_Finalize();

    return 0;
}