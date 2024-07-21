#include "server.h"
#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>


char operator_password[50] = "123";

int connfd[MAX_CLIENTS];     //存储连接的socket描述符

Client clients[MAX_CLIENTS];


Channel channels[MAX_CHANNELS];

int total_user_count;     //记录总的用户数

int  channel_count; //记录聊天室个数


void init_server(int port, char* password) {

    strcpy(operator_password, password);

    int server_fd;
    struct sockaddr_in server_addr;

    // 创建套接字
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 绑定套接字
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 监听连接
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    //初始化
    channel_count = 0;
    total_user_count = 0;

    for (int i = 0;i < MAX_CLIENTS;i++) {

        connfd[i] = -1;

        clients[i].sockfd = -1;
        clients[i].is_registered = 0;
        clients[i].is_operator = 0;
        clients[i].is_away = 0;
        memset(clients[i].nickname, 0, NAME_SIZE);
        memset(clients[i].username, 0, NAME_SIZE);
        memset(clients[i].fullname, 0, NAME_SIZE);
        memset(clients[i].hostname, 0, NAME_SIZE);

    }
    for (int i = 0;i < MAX_CHANNELS;i++) {

        channels[i].is_inuse = -1;
        channels[i].user_count = 0;
        memset(channels[i].name, 0, NAME_SIZE);
        memset(channels[i].topic, 0, 256);
        for (int j = 0;j < MAX_CLIENTS;j++) {
            channels[i].users[j] = -1;
            channels[i].user_status[j].user_index = -1;
            channels[i].user_status[j].is_channel_operator = 0;
            channels[i].user_status[j].has_voice = 0;
        }
        channels[i].is_check_mode = 0;
        channels[i].is_topic_mode = 0;
    }

    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    int i = 0;
    // 接受连接
    while (1) {

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (connfd[i] == -1)
                break;
        }

        connfd[i] = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

        if (connfd[i] < 0) {
            perror("accept failed");
            continue;
        }

        // 获取对端IP地址
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        printf("Client IP: %s\n", client_ip);

        // 针对当前套接字创建一个线程，对当前套接字的消息进行处理
        // 创建线程并传递参数
        thread_arg_t* args = malloc(sizeof(thread_arg_t));
        args->sockfd_index = i;
        strcpy(args->client_ip, client_ip);

        pthread_t* thread = malloc(sizeof(pthread_t));

        pthread_create(thread, NULL, (void* (*)(void*))process_command, (void*)args);
    }
}
