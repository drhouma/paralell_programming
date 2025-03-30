#include <arpa/inet.h>
#include <err.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <queue>
#include <sstream>

const int MAX_THREADS = 10;

const int MAX_ONLINE_USERS = 15;

char response[] =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n\r\n"
    "<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
    "<style>body {}"
    "h1 { font-size:4cm; text-align: center; color: black;"
    " text-shadow: 0 0 2mm red}</style></head>"
    "<body><h1>Goodbye, world!</h1></body></html>\r\n";

std::string construct_ans(int client_fd) {
    std::stringstream s;
    s << "HTTP/1.1 200 OK\r\n"
         "Content-Type: text/html; charset=UTF-8\r\n\r\n"
         "<!DOCTYPE html><html><head><title>Bye-bye baby bye-bye</title>"
         "<style>body {}"
         "h1 { font-size: 35pt; text-align: center; color: black;"
         " text-shadow: 0 0 2mm red}</style></head>"
         "<body><h1>Request number "
      << client_fd << " has been processed"
                      "</h1></body></html>\r\n";
    return s.str();
}

struct Request {
    int client_fd;
    int req_n;
};

std::queue<Request> request_queue;
pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;

void *process_conn(void *arg) {
    while (true) {
        pthread_mutex_lock(&queue_mutex);
        while (request_queue.empty()) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }

        // Извлекаем запрос из очереди
        Request req = request_queue.front();
        request_queue.pop();
        pthread_mutex_unlock(&queue_mutex);

        // Обработка запроса
        std::string resp_string = construct_ans(req.req_n);
        write(req.client_fd, resp_string.c_str(), resp_string.size());
        close(req.client_fd);  // Закрываем соединение
    }
    return NULL;  // Завершаем поток
}

int main() {
    int one = 1, client_fd;
    struct sockaddr_in svr_addr, cli_addr;
    socklen_t sin_len = sizeof(cli_addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) err(1, "can't open socket");

    int port = 8080;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) == -1) {
        close(sock);
        err(1, "Can't bind");
    }

    listen(sock, 15);

    // Создаем фиксированное количество потоков
    pthread_t threads[MAX_THREADS];
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_create(&threads[i], NULL, process_conn, NULL);
    }

    int req_n = 1;
    while (1) {
        client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
        printf("got connection\n");

        if (client_fd == -1) {
            perror("Can't accept");
            continue;
        }

        // Создаем новый запрос и добавляем его в очередь
        Request req;
        req.client_fd = client_fd;
        req.req_n = req_n++;

        pthread_mutex_lock(&queue_mutex);
        request_queue.push(req);
        pthread_cond_signal(&queue_cond);  // Уведомляем один из потоков
        pthread_mutex_unlock(&queue_mutex);
    }

    // Закрываем сокет и завершаем потоки (в этом примере не достигается)
    close(sock);
    return 0;
}