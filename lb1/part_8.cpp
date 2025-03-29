#include <stdlib.h>

#include <iostream>
#include <string>
#include <vector>

using namespace std;
int func_code = 0;

struct params {
  int *arr_ptr;
  vector<int> *indicies;
};

int get_int(int diapason_s, int diapason_e, string str) {
  string input;
  int status = 0;
  int n;

  while (status != 1) {
    cout << str << " (" << diapason_s << " - " << diapason_e << ")" << endl;
    cin >> input;
    try {
      n = stoi(input);
      if (n < diapason_s || n > diapason_e) {
        throw exception();
      }
      status = 1;
    } catch (const std::exception &e) {
      cout << "Неккорректный ввод" << endl;
    }
  }
  return n;
}

int arr_func(int elem) {
  switch (func_code) {
    case 0:
      return elem + 2;
      break;
    case 1:
      return elem - 2;
      break;
    case 2:
      return elem / 2;
      break;
    case 3:
      return elem * 2;
      break;
    default:
      return elem + 2;
      break;
  }
}

void *thread_job(void *arg) {
  if (arg != NULL) {
    params *param = (params *)arg;
    if (arg == NULL) {
      throw std::out_of_range(
          "null ptr exception when reading argument in thread\n");
    }
    int *arr = param->arr_ptr;
    vector<int> *indicies = param->indicies;
    for (int elem : *indicies) {
      arr[elem] = arr_func(arr[elem]);
    }
    delete indicies;
    delete param;
  }
}

int main() {
  int threads_count, array_size;

  threads_count = get_int(1, 32, "Введите число потоков");
  array_size = get_int(1, 10000, "Введите размер массива");
  func_code = get_int(0, 3,
                      "Введите операцию, которая будет производится с "
                      "элементами массива 0: +, 1: -, 2: /, 3: *");

  int *arr = new int[array_size];
  for (int i = 0; i < array_size; i++) {
    *(arr + i) = i;
    // cout << *(arr + i) << endl;
  }

  int err;
  vector<pthread_t> threads;

  if (threads_count > array_size) {
    threads_count = array_size;
  }

  for (int i = 0; i < threads_count; i++) {
    vector<int> *indicies = new vector<int>;
    for (int j = 0; j + i < array_size; j = j + threads_count) {
      indicies->push_back(i + j);
    }
    pthread_t thread;
    params *param = new params;
    param->arr_ptr = arr;
    param->indicies = indicies;
    err = pthread_create(&thread, NULL, thread_job, param);
    threads.push_back(thread);
  }
  for (int i = 0; i < threads_count; i++) {
    pthread_join(threads[i], NULL);
  }
  cout << "result array:";
  for (int i = 0; i < array_size; i++) {
    cout << *(arr + i) << ' ';
  }
  cout << endl;
  delete[] arr;
}