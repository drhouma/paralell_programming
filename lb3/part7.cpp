#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const std::string matrixPath = "data_p7/matrix/";

const std::string vectorPath = "data_p7/free_vector/";

struct Matrix {
    int rows;
    int cols;
    double **matrix;

    Matrix(int _rows, int _cols, double **_matrix)
        : rows(_rows), cols(_cols), matrix(_matrix) {}

    void DestroyMatrix() {
        for (int i = 0; i < cols; i++) {
            delete matrix[i];
        }
        delete matrix;
        matrix = nullptr;
    }
};

/// @brief Yakobi realisation
/// @param A матрица коэффицентов
/// @param B вектор свободных членов
/// @return x - вектор приближения
double *Yakobi(double **A, double *b, int size, int max_iter, double eps) {
    double *x = new double[size];
    double *x_new = new double[size];

    for (int k = 0; k < max_iter; k++) {
        // #pragma omp parallel for private(sum)
        for (int i = 0; i < size; ++i) {
            double sum = 0.0;
            for (int j = 0; j < size; ++j) {
                if (i != j) {
                    sum += A[i][j] * x[j];
                }
            }
            x_new[i] = (b[i] - sum) / A[i][i];
        }

        for (int z = 0; z < size; z++) {
            std::cout << x_new[z] << ' ';
        }
        std::cout << '\n';

        // Проверка на сходимость
        double max_diff = 0.0;
        for (int z = 0; z < size; ++z) {
            max_diff = std::max(max_diff, fabs(x_new[z] - x[z]));
        }

        // Обновление вектора x
        for (int z = 0; z < size; ++z) {
            x[z] = x_new[z];
        }

        // Если достигнута заданная точность, выходим из цикла
        if (max_diff < eps) {
            break;
        }
    }
    delete[] x_new;
    return x;
}

// file format: first two numbers - matrix rows and cols
// float numbers readable with . (not ,)
Matrix GetMatrixFromFile(const std::string &filename) {
    std::fstream input(filename, std::ios_base::in);
    int rows;
    int cols;
    input >> rows;
    input >> cols;
    std::cout << "Matrix size: " << rows << ' ' << cols << '\n';

    double **matrix = new double *[rows];
    for (int i = 0; i < rows; i++) {
        matrix[i] = new double[cols];
    }
    try {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                input >> matrix[i][j];
            }
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        Matrix m(rows, cols, matrix);
        m.DestroyMatrix();
    }
    return Matrix(rows, cols, matrix);
}

// file format: first number - size of a vector
// float numbers readable with . (not ,)
double *GetVectorFromFile(const std::string &filename, int *vsize) {
    std::fstream input(filename, std::ios_base::in);
    int size;
    input >> size;
    *vsize = size;
    std::cout << "Vector size: " << size << '\n';
    double *vector = new double[size];
    try {
        for (int i = 0; i < size; i++) {
            input >> vector[i];
        }
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        delete[] vector;
        vector = nullptr;
    }
    return vector;
}

int main() {
    bool useTimeCount = false;
    while (true) {
        std::cout << "User interface:\n";
        std::cout << "1. toggle time count\n";
        std::cout << "2. solve equation\n";
        std::cout << "0. exit\n";
        std::string input;
        std::cin >> input;
        int opt;
        try {
            opt = std::stoi(input.c_str());
        } catch (const std::exception &e) {
            std::cerr << "incorrect option!" << '\n';
        }

        switch (opt) {
            case 1: {
                /* code */
                useTimeCount = !useTimeCount;
                std::cout << (useTimeCount ? "Time view on\n" : "Time view off\n");
                break;
            }

            case 2: {
                /* code */
                std::cout << "enter filename with matrix (in matrix dir)" << std::endl;
                std::cin >> input;
                Matrix m = GetMatrixFromFile(matrixPath + input);
                if (m.matrix == nullptr) continue;
                if (m.cols != m.rows) {
                    std::cerr << "can process only square matricies, given non square\n";
                    m.DestroyMatrix();
                    continue;
                }

                int size;
                std::cout << "enter filename with vector (in vector dir)" << std::endl;
                std::cin >> input;
                double *vector = GetVectorFromFile(vectorPath + input, &size);
                if (vector == nullptr) continue;
                if (size != m.cols) {
                    std::cerr << "vector size is not equal to matrix size\n";
                    m.DestroyMatrix();
                    delete[] vector;
                    continue;
                }

                auto start = clock();
                double *res = Yakobi(m.matrix, vector, m.cols, 1000, 0.00001);
                auto end = clock();

                std::cout << "result vector: " << std::endl;
                for (int i = 0; i < m.cols; i++) {
                    std::cout << res[i] << ' ';
                }
                std::cout << std::endl;
                delete[] res;
                delete[] vector;
                m.DestroyMatrix();
                if (useTimeCount) {
                    std::cout << "evaluation time: " << (end - start) / (double)CLOCKS_PER_SEC;
                    std::cout << std::endl;
                }
                break;
            }
            case 0: {
                exit(0);
                break;
            }
            default: {
                std::cerr << "incorrect option!" << '\n';
                break;
            }
        }
    }
}