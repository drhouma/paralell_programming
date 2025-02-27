#include <pthread.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>  // подключаем clock
#include <iostream>
#include <string>
#include <vector>

#define CLOCKS_PER_MSEC 1000

using namespace std;
int evals = 1;
bool print_attrs = false;
pthread_mutex_t mutex_;

void print_attr(pthread_attr_t attr) {
  int ds;
  size_t ssize, gsize;
  void *s_address;
  pthread_attr_getdetachstate(&attr, &ds);
  pthread_attr_getstacksize(&attr, &ssize);
  pthread_attr_getguardsize(&attr, &gsize);
  pthread_attr_getstackaddr(&attr, &s_address);

  cout << "stack size: " << ssize << endl;
  cout << "guard size: " << ssize << endl;
  cout << "detach state: " << ds << endl;
  printf("\nstackaddr is %x\n", s_address);
}

struct parameters {
  int index;
  clock_t time;
  pthread_attr_t attr;
};

void time_waste() {
  int divident = 44251243;
  int divisor = 12312312;
  for (int i = 0; i < evals; i++) {
    (divident * i) / (divisor + i) * i;
  }
}

int time_linear(int n) {
  int start1 = clock();
  for (int i = 0; i < n; i++) {
    time_waste();
  }
  int t1 = (clock() - start1);
  return t1;
}

/* Функция, которую будет исполнять созданный поток */
void *thread_job(void *arg) {
  pthread_mutex_lock(&mutex_);
  cout << "Thread is running...";
  if (arg != NULL) {
    parameters *params = (parameters *)arg;
    if (print_attrs) {
      print_attr(params->attr);
    }
    cout << " index: " << params->index << " time: " << params->time;
    delete params;
  }
  cout << endl;
  pthread_mutex_unlock(&mutex_);
  time_waste();
}

int time_parallel(int n, pthread_attr_t *attr = NULL,
                  vector<pthread_t> *v = NULL) {
  int start = clock(), err;
  for (int i = 0; i < n; i++) {
    pthread_t thread;
    parameters *param = new parameters;
    param->index = i;
    param->time = clock();
    param->attr = *attr;
    err = pthread_create(&thread, attr, thread_job, param);
    if (v) v->push_back(thread);
    // Если при создании потока произошла ошибка, выводим
    // сообщение об ошибке и прекращаем работу программы
    if (err != 0) {
      cout << "Cannot create a thread: " << strerror(err) << endl;
      exit(-1);
    }
  }
  int end = clock();
  return end - start;
}

void set_attributes(pthread_attr_t *attr) {
  string input;
  int ds;
  void *my_stack_address;
  int gsize = 0, ssize = 0;
  cout << "Введите тип потока (1 - joinable, 2 - detached, -1 - нет типа)"
       << endl;
  cin >> input;
  if (input == "1") {
    ds = PTHREAD_CREATE_JOINABLE;
  } else if (input == "2") {
    ds = PTHREAD_CREATE_DETACHED;
  } else {
    ds = -1;
  }
  cout << "целое число, во сколько раз больше будет размер защиты чем "
          "минимальный: "
       << PTHREAD_STACK_MIN
       << " (-1 для отсутствия "
          "параметра размер защиты)"
       << endl;
  cin >> input;
  try {
    gsize = stoi(input);
    if (gsize <= 0) {
      gsize = -1;
    } else {
      gsize = gsize * PTHREAD_STACK_MIN;
    }
  } catch (const std::exception &e) {
    std::cerr << "Введено некорректное число: string -> int error" << '\n';
    gsize = -1;
  }
  cout << "целое число, во сколько раз больше будет размер стека чем "
          "минимальный  "
       << PTHREAD_STACK_MIN
       << " (-1 для отсутствия "
          "параметра размер стека)"
       << endl;
  cin >> input;
  try {
    ssize = stoi(input);
    if (ssize <= 0) {
      ssize = -1;
    } else {
      ssize = ssize * PTHREAD_STACK_MIN;
    }
  } catch (const std::exception &e) {
    std::cerr << "Введено некорректное число: string -> int error" << '\n';
    ssize = -1;
  }

  cout << "Введите 1, если хотите указать свой адрес стека" << endl;
  cin >> input;
  if (input == "1") {
    int ssize_ = PTHREAD_STACK_MIN * 3;
    if (ssize != -1) {
      ssize_ = ssize;
    }
    my_stack_address = (void *)malloc(ssize_);

    // Using PTHREAD_STACK_MIN to align stackaddr
    my_stack_address =
        (void *)((((long)my_stack_address + (PTHREAD_STACK_MIN - 1)) /
                  PTHREAD_STACK_MIN) *
                 PTHREAD_STACK_MIN);
  } else {
    my_stack_address = NULL;
  }
  int err;
  // setting attributes
  if (ds != -1) {
    err = pthread_attr_setdetachstate(attr, ds);
    if (err != 0) {
      cout << "detach state err: " << err << endl;
    }
  }
  if (gsize != -1) {
    size_t gs = gsize;
    err = pthread_attr_setguardsize(attr, gs);
    if (err != 0) {
      cout << "guard size err: " << err << endl;
    }
  }
  if (ssize != -1) {
    size_t ss = ssize;
    err = pthread_attr_setstacksize(attr, ss);
    if (err != 0) {
      cout << "stack size err: " << err << endl;
    }
  }
  if (my_stack_address != NULL) {
    size_t ss = ssize;
    err = pthread_attr_setstack(attr, my_stack_address, ss);
    if (err != 0) {
      cout << "stack addr err: " << err << endl;
    }
  }
}

int main() {
  // Определяем переменные: идентификатор потока и код ошибки
  pthread_t thread;
  int err, n;
  size_t size;
  pthread_mutex_init(&mutex_, NULL);

  // количество ядер процессора
  int numCPU = 16;

  cout << "Введите количество потоков" << endl;
  cin >> n;
  cout << "Введите количество вычислений" << endl;
  cin >> evals;
  cout << "Выводить атрибуты потока? (y/n)" << endl;
  string input;
  cin >> input;
  if (input == "y" || input == "Y") print_attrs = true;

  if (n > numCPU * 2) {
    cout << "количество потоков превышает количество ядер процессора в два ";
    cout << "раза -> уменьшаю его до 32" << endl;
    n = 32;
  }

  // int t1 = time_linear(n);
  pthread_attr_t attr;
  err = pthread_attr_init(&attr);
  if (err != 0) {
    cout << "err: " << err << " init err attr_init" << endl;
    exit(0);
  }

  set_attributes(&attr);

  vector<pthread_t> threads;
  int t = time_parallel(n, &attr, &threads);

  for (int i = 0; i < n; i++) {
    pthread_join(threads[i], NULL);
  }
  // cout << "Для создания " << n << " потоков потребовалось " << t;
  // cout << " микросекунд, на один поток " << t / n << " микросекунд" << endl;

  // cout << "t1(" << t1 << ") < t(" << t
  //      << ") = " << ((t1 < t) == 1 ? "True" : "False") << endl;

  pthread_mutex_destroy(&mutex_);
  pthread_attr_destroy(&attr);
  pthread_exit(NULL);
  return 0;
}

// 5. Добавить в программу возможность запуска потоков с разными атрибутами (см.
// пример 2).
// 6. Добавить в программу возможность передавать в поток сразу несколько
// параметров (см. пример 3).
// 7. Добавить в функцию потока возможность вывода информации о всех параметрах
// потока, с которыми он был создан.
// 8. Разработать программу, которая обеспечивает параллельное применение
// заданной функции к каждому элементу массива. Размер массива, применяемая
// функция и количество потоков задаются динамически.
// 9. Выполнить задание для самостоятельной работы (вариант согласовывается с
// преподавателем), по результатам подготовить отчет.
