#include <pthread.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

using namespace std;

/* Функция, которую будет исполнять созданный поток */
void *thread_job(void *arg) { cout << "Thread is running..." << endl; }

int main() {
  // Определяем переменные: идентификатор потока и код ошибки
  pthread_t thread;
  int err, n;
  size_t size = sizeof(int);

  // количество ядер процессора
  int numCPU = 16;

  cout << "Введите количество потоков" << endl;
  cin >> n;

  if (n > numCPU * 2) {
    cout << "количество потоков превышает количество ядер процессора в два ";
    cout << "раза -> уменьшаю его до 32" << endl;
    n = 32;
  }
  for (int i = 0; i < n; i++) {
    pthread_t thread;
    err = pthread_create(&thread, NULL, thread_job, NULL);
    // Если при создании потока произошла ошибка, выводим
    // сообщение об ошибке и прекращаем работу программы
    if (err != 0) {
      cout << "Cannot create a thread: " << strerror(err) << endl;
      exit(-1);
    }
  }
  // Ожидаем завершения созданного потока перед завершением
  // работы программы
  pthread_exit(NULL);
}