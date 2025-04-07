#include <omp.h>

#include <ctime>
#include <iostream>

using namespace std;

int OneSecondFunction(int arg) {
    for (int i = 0; i < 100000000; i++) {
        float a = i;
        float b = (a + i * 2) * (i - 3) / (a + i) * i;
        b = (a + i * 2) * (i - 3) / (a + i) * i;
    }
    return arg;
}

void TestOneSecondFunc() {
    auto start = clock();
    OneSecondFunction(5);
    auto end = clock();
    auto time = end - start;
    cout << "One sec func time: " << time / (double)CLOCKS_PER_SEC << endl;
}

void Ex19Program() {
    int a[100], b[100];

    // Инициализация массива b
    for (int i = 0; i < 100; i++)
        b[i] = i;

    // Директива OpenMP для распараллеливания цикла
#pragma omp parallel for
    for (int i = 0; i < 100; i++) {
        a[i] = b[i];
        b[i] = 2 * a[i];
    }

    int result = 0;
    // Далее значения a[i] и b[i] используются, например, так:
#pragma omp parallel for reduction(+ : result)
    for (int i = 0; i < 100; i++)
        result += (a[i] + b[i]);
    cout << "Result = " << result << endl;
    //
}

void Ex19ProgramNoOMP() {
    int a[100], b[100];

    // Инициализация массива b
    for (int i = 0; i < 100; i++)
        b[i] = i;

    for (int i = 0; i < 100; i++) {
        a[i] = b[i];
        b[i] = 2 * a[i];
    }

    int result = 0;
    for (int i = 0; i < 100; i++)
        result += (a[i] + b[i]);
    cout << "Result = " << result << endl;
}

int main() {
    // TestOneSecondFunc();
    Ex19ProgramNoOMP();
    while (true) {
        cout << "User interface:\n";
        cout << "1. Ex19 with omp\n";
        cout << "2. Ex19 without omp\n";
        cout << "0. exit";
        string input;
        cin >> input;
        if (input == "1") {
            Ex19Program();
        } else if (input == "2") {
            Ex19ProgramNoOMP();
        } else if (input == "0") {
            exit(0);
        }
    }
}