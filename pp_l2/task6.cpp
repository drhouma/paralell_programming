#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct {
    int *data;
    int start;
    int end;
    int (*map_func)(int);
    int *results;
} MapArgs;

typedef struct {
    int *mapped_results;
    int size;
    int (*reduce_func)(int, int);
    int result;
} ReduceArgs;

int map_function(int x) {
    return x * x;  // Возвращаем квадрат числа
}

int reduce_function(int x, int y) {
    return x + y;  // Складываем результаты
}

void* map(void* args) {
    MapArgs* map_args = (MapArgs*)args;
    for (int i = map_args->start; i < map_args->end; i++) {
        map_args->results[i] = map_args->map_func(map_args->data[i]);
    }
    return NULL;
}

void* reduce(void* args) {
    ReduceArgs* reduce_args = (ReduceArgs*)args;
    reduce_args->result = reduce_args->mapped_results[0];
    for (int i = 1; i < reduce_args->size; i++) {
        reduce_args->result = reduce_args->reduce_func(reduce_args->result, reduce_args->mapped_results[i]);
    }
    return NULL;
}

int map_reduce(int *data, int data_size, int (*map_func)(int), int (*reduce_func)(int, int), int num_threads) {
    pthread_t *threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    int *mapped_results = (int* )malloc(data_size * sizeof(int));
    MapArgs *map_args = (MapArgs*)malloc(num_threads * sizeof(MapArgs));

    // Шаг 1: Применение функции map к данным
    int chunk_size = data_size / num_threads;
    for (int i = 0; i < num_threads; i++) {
        map_args[i].data = data;
        map_args[i].start = i * chunk_size;
        map_args[i].end = (i == num_threads - 1) ? data_size : (i + 1) * chunk_size;
        map_args[i].map_func = map_func;
        map_args[i].results = mapped_results;
        pthread_create(&threads[i], NULL, map, &map_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Шаг 2: Применение функции reduce к результатам
    ReduceArgs reduce_args;
    reduce_args.mapped_results = mapped_results;
    reduce_args.size = data_size;
    reduce_args.reduce_func = reduce_func;

    int final_result;
    reduce(&reduce_args);
    final_result = reduce_args.result;

    // Освобождение ресурсов
    free(threads);
    free(mapped_results);
    free(map_args);

    return final_result;
}

int main() {
    int data[] = {1, 2, 3, 4, 5};  // Исходные данные
    int data_size = sizeof(data) / sizeof(data[0]);
    int num_threads = 3;  // Количество потоков

    int result = map_reduce(data, data_size, map_function, reduce_function, num_threads);
    printf("Результат MapReduce: %d\n", result);  // Ожидаемый результат: 55 (1^2 + 2^2 + 3^2 + 4^2 + 5^2)

    return 0;
}