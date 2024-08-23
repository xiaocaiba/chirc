#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<pthread.h>

/*存储用户*/
struct user
{
    char username[20];
    char password[20];
};

/*存储用户及其用户套接字文件描述符*/
struct user_socket
{
    char username[20];
    int socketfd;             //存的时下标，真正的描述符都存在connfd数组里
    int status; //标识是否在线 0:在线 -1:下线
};

/*存储聊天室*/
struct chatroom
{
    char name[20];
    char passwd[20];
    int user[10]; //加入聊天室的人数
    int status;   //标识是否还存在 0:存在 -1:销毁
};

#define PORT 8888
#define UDP_PORT 8889

#define MAXMEM 50
#define MAXROOM 50
#define BUFFSIZE 256

int user_count;     //记录总的用户数
int chatroom_count; //记录聊天室个数

int udp_sockfd;

int listenfd, connfd[MAXMEM];   //connfd是记录每个用户tcp连接的数组，初始化为-1，每个sockfd唯一且大于3
struct user users[MAXMEM];               //记录所有用户
struct user_socket online_users[MAXMEM]; //记录在线用户
struct chatroom chatrooms[MAXROOM];      //记录聊天室

void* create_udp_connect(void* arg);

void create_tcp_connect();

void init();  //从文件读取用户信息，status全初始化为-1

void save_users();    //将内存中的所有用户信息写入文件中

void rcv_snd(int n);   //接受来自客户端的命令，并将命令解析，执行相应的函数，将结果返回给客户端


void register_user();

int user_login(int n);

void get_online_users(int sfd);

void send_private_msg(char* username, char* data, int sfd);

void send_all_msg(char* msg, int sfd);

void create_chatroom(char* name, char* passwd, int sfd);
void join_chatroom(char* name, char* passwd, int sfd);
void send_chatroom_msg(char* room, char* mesg, int sfd);
void get_inroom_users(char* name, int sfd);
void leave_chatroom(char* name, int sfd);
void get_online_chatrooms(int sfd);


void quit_client(int n);

//server派生的自身到自己的tcp连接，以实现管理功能，采用另一个线程实现
void* server_manage_client(void* arg);


void create_tcp_connect()
{
    struct sockaddr_in serv_addr, cli_addr;
    int i;
    time_t timenow;
    pthread_t thread;
    char buff[BUFFSIZE];

    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0)
    {
        perror("fail to socket");
        exit(-1);
    }

    if (bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("fail to bind");
        exit(-2);
    }

    listen(listenfd, MAXMEM);

    // 将套接字描述符数组初始化为-1，表示空闲
    for (i = 0; i < MAXMEM; i++)
        connfd[i] = -1;

    while (1)
    {
        socklen_t len = sizeof(cli_addr);
        for (i = 0; i < MAXMEM; i++)
        {
            if (connfd[i] == -1)
                break;
        }
        // accept 从listen接受的连接队列中取得一个连接
        connfd[i] = accept(listenfd, (struct sockaddr*)&cli_addr, &len);
        if (connfd[i] < 0)
        {
            perror("fail to accept.");
        }
        timenow = time(NULL);
        printf("%.24s\n\tconnect from: %s, port %d\n",
            ctime(&timenow), inet_ntop(AF_INET, &(cli_addr.sin_addr), buff, BUFFSIZE),
            ntohs(cli_addr.sin_port));

        // 针对当前套接字创建一个线程，对当前套接字的消息进行处理
        pthread_create(malloc(sizeof(pthread_t)), NULL, (void*)(&rcv_snd), (void*)i);
    }
}

void* create_udp_connect(void* arg)
{

    struct sockaddr_in serv_addr;
    int i;
    time_t timenow;
    pthread_t thread;

    char buf[BUFFSIZE];

    bzero(&serv_addr, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(UDP_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0)
    {
        perror("fail to create udp socket");
        exit(-1);
    }
    if (bind(udp_sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("fail to bind");
        exit(-2);
    }
    register_user();

}

/*用户注册*/
void register_user()
{
    while (1)
    {
        int  i;
        int recv_len;
        char buf[BUFFSIZE];
        char username[20], password[20];

        struct sockaddr_in cliaddr;
        socklen_t len;
        len = sizeof(cliaddr);

        char temp[BUFFSIZE];
        //会阻塞在这
        //client发送消息时，连带把地址发送过来，所以必须先client发送一个消息

        recv_len = recvfrom(udp_sockfd, username, 20, 0, (struct sockaddr*)&cliaddr, &len);
        printf("recived username is:%s", username);

        if (recv_len > 0)
        {
            username[recv_len - 1] = '\0'; // 去除换行符
        }

        //len = read(connfd[n], password, 20);
        recv_len = recvfrom(udp_sockfd, password, 20, 0, (struct sockaddr*)&cliaddr, &len);

        if (recv_len > 0)
        {
            password[recv_len - 1] = '\0'; // 去除换行符
        }
        for (i = 0; i < MAXMEM; i++)
        {
            if (strcmp(users[i].username, username) == 0)
            {
                strcpy(temp, "The username already exists.\n\n");

                //write(connfd[n], buf, strlen(buf) + 1);
                sendto(udp_sockfd, temp, strlen(temp), 0, (struct sockaddr*)&cliaddr, len);

                return;
            }
        }
        strcpy(users[user_count].username, username);
        strcpy(users[user_count].password, password);
        user_count++;
        sprintf(temp, "Account created successfully.\n \n");
        //write(connfd[n], buf, strlen(buf) + 1);
        sendto(udp_sockfd, temp, strlen(temp), 0, (struct sockaddr*)&cliaddr, len);

        save_users();
    }
}

/*服务器接收和发送函数*/
void rcv_snd(int n)
{
    int len;
    int i;
    char mytime[32], buf[BUFFSIZE];
    char temp[BUFFSIZE];
    char command[20], arg1[20], arg2[BUFFSIZE];
    time_t timenow;

    while (1) {
        len = read(connfd[n], buf, BUFFSIZE);
        //printf("can receive command from client is:\n");
        //printf("%s", buf);

        if (len > 0) {

            if (strcmp(buf, "login") == 0) {
                //printf("read command login successed\n");
                //登录成功时退出该循环
                if (user_login(n) == 0) {
                    //增加逻辑，登录成功通知所有在线用户，相当于send -all 一条登录消息
                    sprintf(temp, "I am online,welcome to chat with me!\n");
                    send_all_msg(temp, n);
                    break;
                }
            }
            else if (strcmp(buf, "quit") == 0) {
                quit_client(n);
            }
            else {
                char temp[BUFFSIZE];
                sprintf(temp, "invalid command,please login first or quit\n");
                write(connfd[n], temp, strlen(temp) + 1);
            }
        }
        else if (len == 0) { // 客户端关闭连接
            printf("Client closed connection\n");
            break; // 断开连接后退出循环
        }
    }

    while (1)
    {
        if ((len = read(connfd[n], temp, BUFFSIZE)) > 0)
        {
            temp[len] = '\n';
            sscanf(temp, "%s %s %[^\n]", command, arg1, arg2); //解析命令
            /* printf("command 1 :%s\t", command);
            printf("command 2: %s\t", arg1);
            printf("command 3: %s\n", arg2); */

            //根据解析出的命令执行不同的函数
            if (strcmp(command, "send") == 0 && strcmp(arg1, "-all") == 0)
            {
                send_all_msg(arg2, n);
            }
            else if (strcmp(command, "send") == 0 && strcmp(arg1, "-chatroom") == 0)
            {
                char room[20], mesg[BUFFSIZE];
                sscanf(arg2, "%s %[^\n]", room, mesg); //解析命令
                send_chatroom_msg(room, mesg, n);
            }
            else if (strcmp(command, "send") == 0)
            {
                send_private_msg(arg1, arg2, n);
            }
            else if (strcmp(command, "quit") == 0)
            {
                sprintf(temp, "the user has already offline\n");
                send_all_msg(temp, n);
                quit_client(n);
            }
            else if (strcmp(command, "create") == 0)
            {
                create_chatroom(arg1, arg2, n);
            }
            else if (strcmp(command, "join") == 0)
            {
                join_chatroom(arg1, arg2, n);
            }
            else if (strcmp(command, "ls") == 0 && strcmp(arg1, "-chatrooms") == 0)
            {
                get_online_chatrooms(n);
            }
            else if (strcmp(command, "ls") == 0 && strcmp(arg1, "-users") == 0)
            {
                get_online_users(n);
            }
            else if (strcmp(command, "ls") == 0 && strcmp(arg1, "-rmbs") == 0)
            {
                get_inroom_users(arg2, n);
            }
            else if (strcmp(command, "leave") == 0)
            {
                leave_chatroom(arg1, n);
            }
            else
            {
                char buff[BUFFSIZE];
                strcpy(buff, "Invalid command.\n");
                write(connfd[n], buff, strlen(buff) + 1);
            }
        }
    }
}

int main()
{
    init();

    printf("Running...\nEnter command \"quit\" to exit server.\n\n");

    pthread_t thread;

    pthread_t udp_pthread;

    //生成一个udp监听线程，会阻塞在第一个recvfrom函数，等待client发送username字符串
    pthread_create(&udp_pthread, NULL, create_udp_connect, NULL);


    //创建了manager_client线程，用于实现服务器向client发送消息
    pthread_t manager_client_pthread;
    pthread_create(&manager_client_pthread, NULL, server_manage_client, NULL);

    //会阻塞在Accept函数，等待tcp连接
    create_tcp_connect();

    return 0;
}

/*初始化,1,上线用户，和聊天室状态全更新为-1，*/
void init()
{
    int i, j;
    user_count = 0;
    chatroom_count = 0;
    for (i = 0; i < MAXMEM; i++)
    {
        online_users[i].status = -1;
    }
    for (i = 0; i < MAXROOM; i++)
    {
        chatrooms[i].status = -1;
        for (j = 0; j < 10; j++)
        {
            chatrooms[i].user[j] = -1;
        }
    }
    char buf[20];
    FILE* fp = NULL;
    fp = fopen("users.txt", "r");
    //从文件中读取用户
    while (fscanf(fp, "%s", buf) != EOF)
    {
        strcpy(users[user_count].username, buf);
        fscanf(fp, "%s", buf);
        strcpy(users[user_count].password, buf);
        user_count++;
    }
    fclose(fp);
}

/*将用户保存到文件*/
void save_users()
{
    int i;
    char buf[20];
    FILE* fp = NULL;
    fp = fopen("users.txt", "w+");
    for (i = 0; i < user_count; i++)
    {
        strcpy(buf, users[i].username);
        strcat(buf, "\n");
        fprintf(fp, buf);
        strcpy(buf, users[i].password);
        strcat(buf, "\n");
        fprintf(fp, buf);
    }
    fclose(fp);
}

/*服务器处理用户退出*/
void quit_client(int n)
{
    int ret, i;
    close(connfd[n]);   //quit就是客户退出聊天程序，关闭tcp连接
    connfd[n] = -1;
    for (i = 0; i < MAXMEM; i++)
    {
        if (n == online_users[i].socketfd)
        {
            online_users[i].status = -1;
        }
    }
    pthread_exit(&ret);
}

/*用户登录*/
int user_login(int n)
{
    int len, i, j;
    char buf[BUFFSIZE], username[20], password[20];
    sprintf(buf, "your username: ");
    write(connfd[n], buf, strlen(buf) + 1);
    //printf("login write to client successed\n");

    len = read(connfd[n], username, 20);

    sprintf(buf, "your password: ");
    write(connfd[n], buf, strlen(buf) + 1);
    len = read(connfd[n], password, 20);


    for (i = 0; i < MAXMEM; i++)
    {
        if (strcmp(username, users[i].username) == 0)
        {
            if (strcmp(password, users[i].password) == 0)
            {
                sprintf(buf, "Login successfully.\n\n");
                write(connfd[n], buf, strlen(buf) + 1);
                for (j = 0; j < MAXMEM; j++)    //添加到上线用户列表，先找个空的位置
                {
                    if (online_users[j].status == -1)
                        break;
                }
                strcpy(online_users[j].username, username);
                online_users[j].socketfd = n;
                online_users[j].status = 0;
                return 0;
            }
            else
            {
                sprintf(buf, "Wrong password.\n\n");
                write(connfd[n], buf, strlen(buf + 1));
                return -1;
            }
        }
    }
    sprintf(buf, "Account does not exist.\n\n");
    write(connfd[n], buf, strlen(buf + 1));
    return -1;
}


/*获取所有在线用户信息*/
void get_online_users(int sfd)
{
    int i;
    char buf[BUFFSIZE], nowtime[20];
    time_t now;
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);
    strcpy(buf, nowtime);
    strcat(buf, "\t");
    strcat(buf, "All online user(s):\n");
    for (i = 0; i < MAXMEM; i++)
    {
        if (online_users[i].status == 0)
        {
            strcat(buf, "\t");
            strcat(buf, online_users[i].username);
            strcat(buf, "\n");
        }
    }
    write(connfd[sfd], buf, strlen(buf) + 1);
}


/*用户发送私聊信息*/
void send_private_msg(char* username, char* data, int sfd)
{
    int i, j;
    time_t now;
    char send_man[20];
    char buf[BUFFSIZE], nowtime[20], temp[30];
    now = time(NULL);
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);

    for (j = 0; j < MAXMEM; j++)
    {
        if (sfd == online_users[j].socketfd)    //找到发送者的name
        {
            strcpy(send_man, online_users[j].username);
            break;
        }
    }
    int flag = -1;
    for (i = 0; i < MAXMEM; i++)
    {
        if (strcmp(username, online_users[i].username) == 0)
        {
            flag = 0;
            if (online_users[i].status == -1)
            {
                flag = 1;
            }
            break;
        }
    }
    if (flag == -1)
    {
        strcpy(buf, "User does not exit,please check\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else if (flag == 1)
    {
        strcpy(buf, "user is no online\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else {
        strcpy(buf, nowtime);
        strcat(buf, "\t");
        strcat(buf, "from ");
        strcat(buf, send_man);
        strcat(buf, ":\n");
        strcat(buf, data);
        strcat(buf, "\n");
        write(connfd[online_users[i].socketfd], buf, strlen(buf) + 1);
        strcpy(temp, "Send successfully.\n");
        write(connfd[sfd], temp, strlen(temp) + 1);
        return;
    }

}

/*用户群发信息给所有用户*/
void send_all_msg(char* msg, int sfd)
{
    int i;
    char buf[BUFFSIZE], nowtime[20], send_man[20], temp[30];
    time_t now;
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);
    for (i = 0; i < MAXMEM; i++)
    {
        if (sfd == online_users[i].socketfd)
        {
            strcpy(send_man, online_users[i].username);
            break;
        }
    }
    strcpy(buf, nowtime);
    strcat(buf, "\t");
    strcat(buf, "from ");
    strcat(buf, send_man);
    strcat(buf, "(brocast-sent):\n");
    strcat(buf, msg);
    strcat(buf, "\n");
    for (i = 0; i < MAXMEM; i++)
    {
        if (connfd[i] != -1 && i != sfd)
        {
            write(connfd[i], buf, strlen(buf) + 1);
        }
    }
    strcpy(temp, "Send successfully\n");
    write(connfd[sfd], temp, strlen(temp) + 1);
}

/*创建聊天室*/
void create_chatroom(char* name, char* passwd, int sfd)
{
    int i, j;
    char buf[BUFFSIZE];
    for (i = 0; i < MAXROOM; i++)   //找一个空闲的chatroom空间
    {
        if (chatrooms[i].status == -1)
            break;
    }
    strcpy(chatrooms[i].name, name);
    strcpy(chatrooms[i].passwd, passwd);

    chatrooms[i].status = 0;
    for (j = 0; j < 10; j++)     //将chatroom里第一个成员设置为创建者的fd
    {
        if (chatrooms[i].user[j] == -1)
            break;
    }
    chatrooms[i].user[j] = sfd;
    strcpy(buf, "Successfully created chat room.\n");
    write(connfd[sfd], buf, strlen(buf) + 1);
}

/*向聊天室发送信息*/
void send_chatroom_msg(char* room, char* mesg, int sfd)
{
    int i, j, k;
    int flag;
    flag = -1;
    char buf[BUFFSIZE], nowtime[20];
    time_t now;
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);

    char temp[BUFFSIZE];

    for (i = 0;i < MAXROOM;i++)
    {
        if (strcmp(chatrooms[i].name, room) == 0)   //先找找有没有这个chatroom
        {
            flag = 0;
            if (chatrooms[i].status == 0)
            {
                flag = 1;
                break;
            }
            break;
        }
    }
    if (flag == -1)
    {
        sprintf(buf, "error: dont have this chatroom,please check!!!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else if (flag == 0)
    {
        sprintf(buf, "error: this chatroom is offline!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }



    int p;
    for (p = 0;p < MAXMEM;p++)
    {
        if (online_users[p].socketfd == sfd)
        {
            break;
        }
    }
    if (strcmp(online_users[p].username, "manager") == 0)
    {
        strcpy(buf, nowtime);
        strcat(buf, "\tchatroom ");
        strcat(buf, chatrooms[i].name);
        strcat(buf, ":\nfrom ");
        strcat(buf, "manager");
        strcat(buf, ":\t");
        strcat(buf, mesg);
        strcat(buf, "\n");
        for (k = 0; k < 10; k++)
        {
            if (chatrooms[i].user[k] != -1)
            {
                if (connfd[chatrooms[i].user[k]] != -1)
                    write(connfd[chatrooms[i].user[k]], buf, strlen(buf) + 1);
            }
        }
        strcpy(temp, "Send successfully.\n");
        write(connfd[sfd], temp, strlen(temp) + 1);
        return;
    }

    flag = -1;
    for (j = 0; j < 10; j++)
    {
        if (chatrooms[i].user[j] == sfd)
        {
            flag = 0;
            break;
        }
    }
    if (flag == -1)
    {
        strcpy(buf, "error: You have not joined the chat room.\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
    else
    {

        for (k = 0; k < MAXMEM; k++)
        {
            if (online_users[k].status == 0 && online_users[k].socketfd == sfd)  //找到消息发送者
                break;
        }
        strcpy(buf, nowtime);
        strcat(buf, "\tchatroom ");
        strcat(buf, chatrooms[i].name);
        strcat(buf, ":\nfrom ");
        strcat(buf, online_users[k].username);
        strcat(buf, ":\t");
        strcat(buf, mesg);
        strcat(buf, "\n");
        for (k = 0; k < 10; k++)
        {
            if (chatrooms[i].user[k] != -1)
            {
                if (connfd[chatrooms[i].user[k]] != -1)
                    write(connfd[chatrooms[i].user[k]], buf, strlen(buf) + 1);
            }
        }
        strcpy(temp, "Send successfully.\n");
        write(connfd[sfd], temp, strlen(temp) + 1);
    }
}

/*加入聊天室*/
void join_chatroom(char* name, char* passwd, int sfd)
{
    int i, j;
    int room, flag;
    char buf[BUFFSIZE];
    flag = -1;

    for (i = 0;i < MAXROOM;i++)
    {
        if (strcmp(chatrooms[i].name, name) == 0)   //先找找有没有这个chatroom
        {
            flag = 0;
            if (chatrooms[i].status == 0)
            {
                flag = 1;
                break;
            }
            break;
        }
    }
    if (flag == -1)
    {
        sprintf(buf, "error: no this chatroom,please check!!!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else if (flag == 0)
    {
        sprintf(buf, "error: this chatroom is outline!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    flag = -1;

    for (j = 0; j < 10; j++)
    {
        if (chatrooms[i].user[j] == -1)   //在该聊天室找个空位，找不到就报错已满
        {
            flag = 0;
            break;
        }
        if (chatrooms[i].user[j] == sfd)
        {
            flag = 1;
            break;
        }
    }
    if (flag == -1)
    {
        sprintf(buf, "the member of this chatroom is full!!!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    if (flag == 1)
    {
        strcpy(buf, "You have joined the chat room ");
        strcat(buf, chatrooms[room].name);
        strcat(buf, ".\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
    else
    {

        if (strcmp(chatrooms[i].passwd, passwd) == 0)
        {

            chatrooms[i].user[j] = sfd;
            strcpy(buf, "Successfully joined the chat room.\n");
            write(connfd[sfd], buf, strlen(buf) + 1);
            return;
        }
        else
        {
            strcpy(buf, "Incorrect chat room password.\n");
            write(connfd[sfd], buf, strlen(buf) + 1);
            return;
        }
    }
}


/*获取所有已创建的聊天室的信息*/
void get_online_chatrooms(int sfd)
{
    int i;
    char buf[BUFFSIZE], nowtime[20];
    time_t now;
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);
    strcpy(buf, nowtime);
    strcat(buf, "\tAll online chat room(s):\n");
    for (i = 0; i < MAXROOM; i++)
    {
        if (chatrooms[i].status == 0)
        {
            strcat(buf, "\t");
            strcat(buf, chatrooms[i].name);
            strcat(buf, "\n");
        }
    }
    write(connfd[sfd], buf, strlen(buf) + 1);
}

/*查询所有加入某聊天室的用户*/
void get_inroom_users(char* name, int sfd)
{
    int i, j, k;
    int  flag;
    flag = -1;
    char buf[BUFFSIZE], nowtime[20];
    time_t now;
    time(&now);
    struct tm* tempTime = localtime(&now);
    strftime(nowtime, 20, "[%H:%M:%S]", tempTime);

    for (i = 0;i < MAXROOM;i++)
    {
        if (strcmp(chatrooms[i].name, name) == 0)   //先找找有没有这个chatroom
        {
            flag = 0;
            if (chatrooms[i].status == 0)
            {
                flag = 1;
                break;
            }
            break;
        }
    }
    if (flag == -1)
    {
        sprintf(buf, "error: no this chatroom,please check!!!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else if (flag == 0)
    {
        sprintf(buf, "error: this chatroom is outline!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    flag = -1;

    for (j = 0; j < 10; j++)
    {
        if (chatrooms[i].user[j] == sfd)
        {
            flag = 0;
            break;
        }
    }
    if (flag == -1)
    {
        strcpy(buf, "error: You have not joined the chat room.\n cant view members\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
    else
    {
        strcpy(buf, nowtime);
        strcat(buf, "\tAll users in the ");
        strcat(buf, chatrooms[i].name);
        strcat(buf, ":\n");
        for (j = 0; j < 10; j++)
        {
            if (chatrooms[i].user[j] >= 0)
                for (k = 0; k < MAXMEM; k++)
                {
                    if (online_users[k].status != -1 && (chatrooms[i].user[j] == online_users[k].socketfd))
                    {
                        strcat(buf, "\t");
                        strcat(buf, online_users[k].username);
                        strcat(buf, "\n");
                    }
                }
        }
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
}

/*退出聊天室*/
void leave_chatroom(char* name, int sfd)
{
    int i, j;
    int  flag;
    flag = -1;
    char buf[BUFFSIZE];

    for (i = 0;i < MAXROOM;i++)
    {
        if (strcmp(chatrooms[i].name, name) == 0)   //先找找有没有这个chatroom
        {
            flag = 0;
            if (chatrooms[i].status == 0)
            {
                flag = 1;
                break;
            }
            break;
        }
    }
    if (flag == -1)
    {
        sprintf(buf, "error: no this chatroom,please check!!!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }
    else if (flag == 0)
    {
        sprintf(buf, "error: this chatroom is outline!\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
        return;
    }

    flag = -1;

    for (j = 0; j < 10; j++)
    {
        if (chatrooms[i].user[j] == sfd)
        {
            chatrooms[i].user[j] = -1;
            flag = 0;
            break;
        }
    }
    if (flag == -1)
    {
        strcpy(buf, "error: You have not joined the chat room.\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
    else
    {
        strcpy(buf, "Successfully quit chat room ");
        strcat(buf, chatrooms[i].name);
        strcat(buf, ".\n");
        write(connfd[sfd], buf, strlen(buf) + 1);
    }
}

void* server_manage_client(void* arg)
{
    int manager_sockfd;
    struct sockaddr_in server_addr, client_addr;

    char* server_ip = "127.0.0.1";
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    manager_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //必须先暂停一段时间，等server建立好
    sleep(5);

    if (connect(manager_sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error connecting to server manager\n");
        exit(EXIT_FAILURE);
    }

    int i, maxfd;

    fd_set rset;

    char buf[BUFFSIZE];
    while (1)
    {
        FD_ZERO(&rset);
        FD_SET(fileno(stdin), &rset);
        FD_SET(manager_sockfd, &rset);

        maxfd = (fileno(stdin) > manager_sockfd ? fileno(stdin) : manager_sockfd) + 1;

        select(maxfd, &rset, NULL, NULL, NULL);

        if (FD_ISSET(manager_sockfd, &rset))
        {
            int len;
            if ((len = read(manager_sockfd, buf, BUFFSIZE)) > 0) // read 读取通信套接字
            {
                write(1, buf, len); //1：标准输出
                printf("\n");
            }
            else if (len < 0) // 读取错误
            {
                perror("Error reading from UDP socket");
                // 可以添加其他处理逻辑，如退出线程或重新尝试读取等
            }
            else // len == 0，表示连接已关闭
            {
                printf("manager: Connection closed by server(tcp)\n");
                sleep(2);
                // 可以添加其他处理逻辑，如退出线程或重新尝试连接等
            }
        }
        if (FD_ISSET(fileno(stdin), &rset))
        {
            if (fgets(buf, BUFFSIZE, stdin) != NULL)
            {
                if (strlen(buf) == 0 || strcmp(buf, "") == 0)
                {
                    printf("input is null or kong zifuchuan\n");
                    //continue;
                }
                if (strcmp(buf, "help\n") == 0)
                {
                    printf("enter exit to exit server\n");
                    printf("enter quit to quit manager client\n");
                    printf("manager client login as username: manager  password:123 \n");
                    printf("manager can have all client function\n");
                }
                else if (strcmp(buf, "exit\n") == 0)
                {
                    save_users();
                    printf("Server has aready exited  Byebye... \n");
                    close(listenfd);
                    exit(0);
                }
                else if (strcmp(buf, "\n") != 0)
                {
                    int len = strlen(buf);
                    buf[len] = '\0'; // 确保字符串以 null 结尾

                    if (buf[len - 1] == '\n') {
                        buf[len - 1] = '\0'; // 去除换行符
                    }
                    write(manager_sockfd, buf, strlen(buf));

                }
                else {
                    printf("invadid input , input the right commands!\n");
                }
            }
            //printf("写name成功能跳出这轮while\n");
        }
    }

}