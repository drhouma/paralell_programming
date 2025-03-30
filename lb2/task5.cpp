#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <iostream>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int condition_met = 0;  // Shared variable

void* thread_function(void* arg) {
    pthread_mutex_lock(&mutex);

    // Wait until the condition is met
    while (!condition_met) {
        pthread_cond_wait(&cond, &mutex);
    }

    // Condition is met, proceed with the task
    printf("Condition met, thread proceeding.\n");

    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main() {
    pthread_t thread;

    // Create a thread
    pthread_create(&thread, NULL, thread_function, NULL);

    sleep(1);  // подождать секунду перед обновлением переменной

    pthread_mutex_lock(&mutex);
    std::cout << "set condition var\n";
    condition_met = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    // Wait for the thread to finish
    pthread_join(thread, NULL);

    // Clean up
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}