#include <pthread.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

#define err_exit(code, str) \
    {                       \
        cerr << str << ": " << strerror(code) \ 
    << endl;                \
        exit(EXIT_FAILURE); \
    }

const int TASKS_COUNT = 100;
const int THREAD_COUNT = 4;

int task_list[TASKS_COUNT];  // Массив заданий
int current_task = 0;        // Указатель на текущее задание
bool mutex_usage = true;
std::string mutex_str = mutex_usage ? "mutex" : "spin";

pthread_mutex_t mutex;  // Мьютекс
pthread_spinlock_t spin;

using std::cout;

void time_waste() {
    int evals = 1000;
    int divident = 44251243;
    int divisor = 12312312;
    for (int i = 0; i < evals; i++) {
        (divident * i) / (divisor + i) * i;
    }
}

void do_task(int task_no) {
    time_waste();
}

void *thread_job(void *arg) {
    int task_no;
    int err;
    int thread_no = *((int *)arg);
    // Перебираем в цикле доступные задания
    while (true) {
        // Захватываем мьютекс для исключительного доступа
        // к указателю текущего задания (переменная
        // current_task)
        if (mutex_usage)
            err = pthread_mutex_lock(&mutex);
        else
            err = pthread_spin_lock(&spin);

        if (err != 0)
            err_exit(err, "Cannot lock");
        // Запоминаем номер текущего задания, которое будем исполнять
        task_no = current_task;
        // Сдвигаем указатель текущего задания на следующее
        current_task++;
        // std::cout << "Выполняется " << current_task << " потоком "
        //   << thread_no << std::endl;
        // Освобождаем мьютекс
        if (mutex_usage)
            err = pthread_mutex_unlock(&mutex);
        else
            err = pthread_spin_unlock(&spin);
        if (err != 0)
            err_exit(err, "Cannot unlock");
        // Если запомненный номер задания не превышает
        // количества заданий, вызываем функцию, которая
        // выполнит задание.
        // В противном случае завершаем работу потока
        if (task_no < TASKS_COUNT)
            do_task(task_no);
        else
            return NULL;
    }
}

std::vector<pthread_t> create_threads(int *arr) {
    std::vector<pthread_t> v;
    for (int i = 0; i < THREAD_COUNT; i++) {
        *(arr + i) = i + 1;
        pthread_t thread;
        int err = pthread_create(&thread, NULL, thread_job, (arr + i));
        v.push_back(thread);
        // Если при создании потока произошла ошибка, выводим
        // сообщение об ошибке и прекращаем работу программы
        if (err != 0) {
            cout << "Cannot create a thread: " << strerror(err) << endl;
            exit(-1);
        }
    }
    return v;
}

int main() {
    pthread_t thread1, thread2;  // Идентификаторы потоков
    int err;                     // Код ошибки
              // Инициализируем массив заданий случайными числами
    for (int i = 0; i < TASKS_COUNT; ++i)
        task_list[i] = rand() % TASKS_COUNT;
    // Инициализируем мьютекс
    err = pthread_mutex_init(&mutex, NULL);

    if (err != 0)
        err_exit(err, "Cannot initialize mutex");
    err = pthread_spin_init(&spin, NULL);

    if (err != 0)
        err_exit(err, "Cannot initialize spin");

    // Создаём потоки
    int *arr = (int *)malloc(sizeof(int) * THREAD_COUNT);
    int start = clock();
    auto threads = create_threads(arr);
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    int end = clock();
    cout << "время на выполнение (" << mutex_str << "): "
         << end - start << "ms\n";
    free(arr);
    // Освобождаем ресурсы, связанные с мьютексом
    // for (size_t i = 0; i < TASKS_COUNT; i++)
    // {
    //     cout << "task_index: " << i << " value " << task_list[i] << std::endl;
    // }

    pthread_mutex_destroy(&mutex);
    pthread_spin_destroy(&spin);
}