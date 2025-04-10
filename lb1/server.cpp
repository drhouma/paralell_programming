#include <arpa/inet.h>
#include <err.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#define Kb_512 1024 * 512
#define Mb_1 1024 * 1024
#define Mb_2 1024 * 1024 * 2

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
         "<body><h1>Request  number "
      << client_fd << " has been processed"
                      "</h1></body></html>\r\n";
    return s.str();
}

void *process_conn_php(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);  // Освобождаем память, выделенную для дескриптора

    // Буфер для чтения запроса
    char buffer[1024];
    read(client_fd, buffer, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Обеспечиваем нуль-терминатор
    FILE *fp;
    char result[1024];
    fp = popen("php -v", "r");
    if (fp == NULL) {
        perror("Failed to run command");
        close(client_fd);
        return NULL;
    }

    // Читаем результат выполнения скрипта
    std::string php_output;
    while (fgets(result, sizeof(result), fp) != NULL) {
        php_output += result;
    }
    pclose(fp);
    // Формируем HTTP-ответ
    std::string response_php = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: text/plain; charset=UTF-8\r\n\r\n" +
                              php_output;
    std::cout << response_php << std::endl;
    // Отправляем ответ клиенту
    write(client_fd, response_php.c_str(), response_php.size());
    close(client_fd);  // Закрываем соединение
    return NULL;       // Завершаем поток
}

void *process_conn(void *arg) {
    int client_fd = *(int *)arg;
    int req_n = ((int *)arg)[1];
    free(arg);  // Освобождаем память, выделенную для дескриптора

    // Отправляем ответ клиенту
    
    std::string resp_string = construct_ans(req_n);
    write(client_fd, resp_string.c_str(), resp_string.size()); /* -1: '\0' */
    close(client_fd);                                 // Закрываем соединение
    return NULL;                                      // Завершаем поток
}

int main() {
    int one = 1, client_fd;
    struct sockaddr_in svr_addr, cli_addr;
    socklen_t sin_len = sizeof(cli_addr);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) err(1, "can't open socket");

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));

    int port = 8080;
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = INADDR_ANY;
    svr_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&svr_addr, sizeof(svr_addr)) == -1) {
        close(sock);
        err(1, "Can't bind");
    }

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    // создаем поток с параметром detached
    int err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err != 0) {
        std::cout << "set_attr_ds err: " << err << std::endl;
    }
    // err = pthread_attr_setstacksize(&attr, Mb_2);
    if (err != 0) {
        std::cout << "set_attr_ds err: " << err << std::endl;
    }


    listen(sock, 5);
    int req_n = 1;
    while (1) {
        client_fd = accept(sock, (struct sockaddr *)&cli_addr, &sin_len);
        printf("got connection\n");

        if (client_fd == -1) {
            perror("Can't accept");
            continue;
        }

        // Создаем новый поток для обработки соединения
        pthread_t thread_id;
        int *arg = (int *)malloc(sizeof(int) * 2);  // Выделяем память для 2 аргументов
        arg[0] = client_fd;                         // Сохраняем дескриптор сокета
        arg[1] = req_n;                             // Сохраняем порядковый номер запроса
        req_n++;
        if (pthread_create(&thread_id, &attr, process_conn, arg) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(arg);  // Освобождаем память в случае ошибки
        }

        // pthread_detach(thread_id);  // Отсоединяем поток, чтобы не дожидаться его завершения
    }

    pthread_attr_destroy(&attr);
    close(sock);
    return 0;
}