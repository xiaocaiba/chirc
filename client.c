#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAXLINE 128

#define BUFFSIZE 256

#define MAXSOCKADDR sizeof(struct sockaddr)

int udp_sockfd;
int sockfd;

void get_help();

int udp_client(const char* host, const char* serv, void** saptr, socklen_t* lenp)
{
    int n;
    struct addrinfo hints, * res, * ressave;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((n = getaddrinfo(host, serv, &hints, &res)) != 0) {
        printf("udp_client error for %s, %s:%s\n", host, serv, gai_strerror(n));
        return -1;
    }
    ressave = res;
    do {
        udp_sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (udp_sockfd >= 0) break;
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("udp_client error for %s, %s\n", host, serv);
        return -1;
    }

    *saptr = malloc(res->ai_addrlen);
    memcpy(*saptr, res->ai_addr, sizeof(res->ai_addrlen));
    *lenp = res->ai_addrlen;

    freeaddrinfo(ressave);
    return udp_sockfd;
}

char* sock_ntop(const struct sockaddr* sa, socklen_t len)
{
    char portstr[7];
    static char str[MAXLINE + 1];

    switch (sa->sa_family) {
    case AF_INET:
    {
        struct sockaddr_in* sin = (struct sockaddr_in*)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) return NULL;

        if (ntohs(sin->sin_port) != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
            strcat(str, portstr);
        }
        return str;
    }
    }

    return NULL;
}

int create_udp_socket_for_regist(char* host, char* server)
{
    int  n;
    socklen_t salen;
    char* udp_server = "8889";

    struct sockaddr* sa;

    udp_sockfd = udp_client(host, udp_server, (void**)&sa, &salen);

    if (udp_sockfd != -1)
        printf("connect successed to %s\n", sock_ntop(sa, salen));

    connect(udp_sockfd, sa, salen);

    return udp_sockfd;
}

int tcp_connect(const char* hostname, const char* serv)
{
    int  n;

    struct addrinfo hints, * res, * ressave;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((n = getaddrinfo(hostname, serv, &hints, &res)) != 0) {
        printf("tcp connect error for %s, %s:%s\n", hostname, serv, gai_strerror(n));
        return -1;
    }

    ressave = res;

    do {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd < 0) continue;

        if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) break;
        close(sockfd);
    } while ((res = res->ai_next) != NULL);

    if (res == NULL) {
        printf("tcp_connect error for %s   %s\n", hostname, serv);
        return -1;
    }

    freeaddrinfo(ressave);

    return sockfd;
}

int create_tcp_connect(char* host, char* server)
{
    socklen_t len;
    struct sockaddr* sa;

    sockfd = tcp_connect(host, server);
    if (sockfd < 0) {
        //printf("tcp_connect error\n");
        return -1;
    }
    sa = malloc(MAXSOCKADDR);
    len = MAXSOCKADDR;

    if (getpeername(sockfd, sa, &len) < 0) {
        printf("getpeername error:%s\n", strerror(errno));
        close(sockfd);
        return -1;
    }

    printf("connected by tcp to %s\n", sock_ntop(sa, len));

    return 0;

}

int main(int argc, char** argv)
{
    int i, maxfd;

    fd_set rset;

    udp_sockfd = -1;
    sockfd = -1;

    if (argc != 3) {
        printf("usage:./client <hostname/ipaddress> <sevice/port>\n");
        return -1;
    }

    printf("Enter \"login\" to login\n");
    printf("Enter \"register\" to create an account\n");
    printf("Enter \"quit\" to quit\n");
    printf("Enter \"help\" to get more help\n\n");

    //创建udp套接字
    create_udp_socket_for_regist(argv[1], argv[2]);

    //tcp 创建并连接套接字
    create_tcp_connect(argv[1], argv[2]);


    //从标准输入读取命令，匹配相应的函数，把匹配不上的字符串写入udp，发给server
    char buf[BUFFSIZE];
    while (1)
    {
        FD_ZERO(&rset);
        FD_SET(fileno(stdin), &rset);
        FD_SET(sockfd, &rset);
        FD_SET(udp_sockfd, &rset);

        int flag = (fileno(stdin) > sockfd ? fileno(stdin) : sockfd);
        maxfd = (flag > udp_sockfd ? flag : udp_sockfd) + 1;

        select(maxfd, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset))
        {
            int len;
            if ((len = read(sockfd, buf, BUFFSIZE)) > 0) // read 读取通信套接字
            {
                write(1, buf, len); //1：标准输出
                printf("\n");
            }
            else if (len < 0) // 读取错误
            {
                perror("Error reading from TCP socket");
                // 可以添加其他处理逻辑，如退出线程或重新尝试读取等
            }
            else // len == 0，表示连接已关闭
            {
                printf("Connection closed by server(tcp)\n");
                sleep(2);
                // 可以添加其他处理逻辑，如退出线程或重新尝试连接等
            }
        }

        if (FD_ISSET(udp_sockfd, &rset))
        {
            //printf("说明是能进入udp读取的\n");
            int len;
            if ((len = read(udp_sockfd, buf, BUFFSIZE)) > 0) // read 读取通信套接字
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
                printf("Connection closed by server(udp)\n");
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
                    printf("input is null or empty chat string\n");
                    //continue;
                }
                if (strcmp(buf, "help\n") == 0)
                {
                    get_help();
                    //continue;
                }
                else if (strcmp(buf, "register\n") == 0)  //输入的是register，余下的输入写入udp
                {
                    printf("welcome to register happy chat system\n");

                    char temp[BUFFSIZE];

                    printf("your username:  \n");
                    fgets(temp, BUFFSIZE, stdin);
                    write(udp_sockfd, temp, strlen(temp));

                    printf("your password:  \n");
                    fgets(temp, BUFFSIZE, stdin);
                    write(udp_sockfd, temp, strlen(temp));

                }
                else if (strcmp(buf, "\n") != 0)
                {
                    int len = strlen(buf);
                    buf[len] = '\0'; // 确保字符串以 null 结尾

                    if (buf[len - 1] == '\n') {
                        buf[len - 1] = '\0'; // 去除换行符
                    }
                    write(sockfd, buf, strlen(buf));

                }
                else {
                    printf("invalid input , input the right commands!\n");
                }
            }
            //printf("写name成功能跳出这轮while\n");
        }
    }

    return 0;
}

/*获取帮助信息*/
void get_help()
{
    printf("Commands introduction:\n");
    printf("\t'ls -users':\t\tShow all online users\n");
    printf("\t'ls -chatrooms':\tShow all chat rooms\n");
    printf("\t'ls -rmbs room_name '\t\tShow all online users in the chat room you joined\n");
    printf("\t'send username mesg':\tSend a message to the user named 'username' msg:the content of the message\n");
    printf("\t'join chatroom passwd':\tJoin in a chat room named 'chatroom' with password 'passwd'\n");
    printf("\t'create chatrname passwd':\tCreate a chat room named 'chatrname' with password 'passwd'\n");
    printf("\t'send -chatroom room_name msg':\tSend a message to the chat room\n");
    printf("\t'leave chatroom_name':\t\t\tExit the chat room you joined\n");
    printf("\t'send -all msg':\tSend a message to all online users\n");
    printf("\t'login':\t\tLogin chat system\n");
    printf("\t'register':\t\tCreate an account\n");
    printf("\t'quit':\t\t\tExit chat system\n");
    printf("\t'help':\t\t\tGet more help information\n\n");
}

