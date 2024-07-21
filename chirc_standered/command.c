
#include "server.h"

#include "command.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include<sys/socket.h>
#include <ctype.h>  // 用于 isspace 函数

extern int connfd[MAX_CLIENTS];
extern Client clients[MAX_CLIENTS];
extern Channel channels[MAX_CHANNELS];

extern int  channel_count;
extern int total_user_count;
extern char operator_password[50];


// 检查消息格式是否符合要求
int check_command_format(char* message) {

    int param_count = 0;

    // 找到命令后的第一个空格
    char* cmd_end = strchr(message, ' ');
    if (cmd_end == NULL) {
        return 1;  // 只有命令，没有参数，格式正确
    }

    // 确认第一个空格后的字符不是空格
    if (isspace(*(cmd_end + 1))) {
        return 0;  // 命令后有多个空格，格式不正确
    }

    // 开始检查参数
    char* param = cmd_end + 1;

    while (1) {

        if ((param = strchr(param, ' ')) != NULL)
        {
            param_count++;
            param += 1;
            if (isspace(*param)) {
                return 0;      // 命令后有多个空格，格式不正确
            }
            if (*param == ':')
            {
                break;
            }
        }
        else {
            param_count++;
            break;
        }
    }

    if (param_count > 15) {
        return 0;  // 参数数量超过15个，格式不正确
    }

    return 1;  // 所有参数之间只有一个空格，格式正确
}

void* process_command(void* arg) {

    thread_arg_t* args = (thread_arg_t*)arg;

    int sockfd_index = args->sockfd_index;
    char* client_ip = args->client_ip;

    char buffer[BUFFER_SIZE];

    ssize_t len;

    int i;
    int flag = 0;
    for (i = 0;i < MAX_CLIENTS;i++) {
        if (clients[i].sockfd == -1)
        {
            flag = 1;
            break;
        }

    }
    if (flag == 0) {
        printf("error: connections are full\n");
        exit(0);
    }

    clients[i].sockfd = sockfd_index;         //将connfd中的下标写入client

    strcpy(clients[i].hostname, client_ip);
    clients[i].is_registered = 0;


    int is_command;
    int is_registered;
    while (1) {
        // 清空缓冲区
        memset(buffer, 0, BUFFER_SIZE);

        if (len = read(connfd[sockfd_index], buffer, BUFFER_SIZE) > 0) {

            printf("read the command successed: %s", buffer);

            is_command = 0;
            is_registered = clients[i].is_registered;

            // 检查命令格式
            if (check_command_format(buffer) == 0) {
                send_error_message(sockfd_index, i, ERR_UNKNOWNCOMMAND, "Invalid command format");
                continue;
            }

            if (strncmp(buffer, "NICK", 4) == 0) {
                handle_nick_command(sockfd_index, i, buffer);
                continue;
            }
            else if (strncmp(buffer, "USER", 4) == 0) {
                handle_user_command(sockfd_index, i, buffer);
                continue;
            }
            else if (strncmp(buffer, "PRIVMSG", 7) == 0) {
                is_command = 1;
                if (is_registered == 1) {
                    handle_privmsg_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "NOTICE", 6) == 0) {
                is_command = 1;
                if (is_registered == 1) {
                    handle_notice_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "PING", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_ping_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "PONG", 4) == 0) {
                is_command = 1;
                if (is_registered == 1) {
                    handle_pong_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "MOTD", 4) == 0) {
                is_command = 1;
                if (is_registered == 1) {
                    handle_motd_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "LUSERS", 6) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_lusers_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "WHOIS", 5) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_whois_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "JOIN", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_join_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "PART", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_part_command(sockfd_index, i, buffer);
                    continue;
                }

            }
            else if (strncmp(buffer, "TOPIC", 5) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_topic_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "OPER", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_oper_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "QUIT", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_quit_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "LIST", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_list_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "AWAY", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_away_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "NAMES", 5) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_names_command(sockfd_index, i, buffer);
                    continue;
                }
            }
            else if (strncmp(buffer, "MODE", 4) == 0) {
                is_command = 1;
                if (is_registered == 1)
                {
                    handle_mode_command(sockfd_index, i, buffer);
                    continue;
                }
            }




            // 未注册前收到其他合法命令，发送错误消息
            if (is_registered == 0 && is_command == 1) {
                send_error_message(sockfd_index, i, ERR_NOTREGISTERED, "You have not registered");
                continue;
            }

            // 未注册前收到不认识的命令，忽略
            else if (is_registered == 0 && is_command == 0) {
                continue;
            }

            // 已注册时收到未知命令，发送错误消息
            else {
                send_error_message(sockfd_index, i, ERR_UNKNOWNCOMMAND, "Unknown command");
                continue;
            }

        }
    }
}

void handle_nick_command(int fd_index, int client_index, char* message) {

    char* nick = strtok(message + 5, "\r\n");

    if (!nick) {
        send_error_message(fd_index, client_index, ERR_NONICKNAMEGIVEN, "No nickname given");
        return;
    }
    // 检查昵称是否已经存在
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (strcmp(clients[i].nickname, nick) == 0)
        {
            if (clients[client_index].nickname[0] != '\0') {
                send_error_message(fd_index, client_index, ERR_NICKNAMEINUSE, "Nickname is already in use");
            }
            else {
                char response[512];
                snprintf(response, sizeof(response), ":%s %d * %s :Nickname is already in use\r\n", SERVER_NAME, ERR_NICKNAMEINUSE, nick);
                send(connfd[fd_index], response, strlen(response), 0);
            }
            return;
        }
    }

    int flag = 0;
    // 如果用户已经注册并更改昵称，则通知客户端
    if (clients[client_index].is_registered) {
        flag = 1;
        char response[512];
        snprintf(response, sizeof(response), ":%s NICK %s\r\n", clients[client_index].nickname, nick);
        /* for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_registered) {
                send(connfd[i], response, strlen(response), 0);
            }
        } 如果更改成功，则发送一条 NICK 消息给所有在同一频道的用户和更改昵称的用户自身  */
        send(connfd[fd_index], response, strlen(response), 0);
    }

    strcpy(clients[client_index].nickname, nick);

    if (flag == 0)
        check_registration(client_index, fd_index);
}

void handle_user_command(int fd_index, int client_index, char* message) {

    if (clients[client_index].is_registered == 1) {
        send_error_message(fd_index, client_index, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
        return;
    }

    char* token = strtok(message + 5, "\r\n");

    if (!token) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    char* username = strtok(token, " ");
    printf("username is:%s\n", username);


    char* fullname = strtok(NULL, ":");
    printf("fullname is:%s\n", fullname);

    if (!fullname) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    strcpy(clients[client_index].username, username);

    strcpy(clients[client_index].fullname, fullname);

    check_registration(client_index, fd_index);
}

void check_registration(int client_index, int fd_index) {

    if (clients[client_index].nickname[0] != '\0' && clients[client_index].username[0] != '\0')
    {
        clients[client_index].is_registered = 1;
        clients[client_index].is_away = 1;

        //注册验证通过就构建完整标识符，方便消息格式的使用
        snprintf(complete_name[client_index], 150, "%s!%s@%s", clients[client_index].nickname, clients[client_index].username, clients[client_index].hostname);

        send_welcome_messages(fd_index, client_index);
    }

}

void send_welcome_messages(int fd_index, int client_index) {
    char response[512];
    snprintf(response, sizeof(response), ":%s 001 %s :Welcome to the Internet Relay Network %s\r\n", SERVER_NAME, clients[client_index].nickname, complete_name[client_index]);
    send(connfd[fd_index], response, strlen(response), 0);
    snprintf(response, sizeof(response), ":%s 002 %s :Your host is %s, running version 1.0\r\n", SERVER_NAME, clients[client_index].nickname, SERVER_NAME);
    send(connfd[fd_index], response, strlen(response), 0);
    snprintf(response, sizeof(response), ":%s 003 %s :This server was created 2024-07-01\r\n", SERVER_NAME, clients[client_index].nickname);
    send(connfd[fd_index], response, strlen(response), 0);
    snprintf(response, sizeof(response), ":%s 004 %s %s 1.0 ao mtov\r\n", SERVER_NAME, clients[client_index].nickname, SERVER_NAME);
    send(connfd[fd_index], response, strlen(response), 0);

}


Channel* find_channel(const char* channel_name) {

    printf("channel_count的值是%d\n", channel_count);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (strcmp(channels[i].name, channel_name) == 0) {
            return &channels[i];
        }
    }
    return NULL;
}

int is_client_in_channel(Client* client, Channel* channel)
{
    /* printf("先看看里面有没有东西\n");
    printf("channel_count:%d\n", channel_count);
    printf("channel的name\n%s\n", channel->name);
    printf("channel的user_count:  %d\n", channel->user_count);

    printf("发送者的sockfd是：%d\n", client->sockfd); */

    /* for (int i = 0;i < channel->user_count;i++) {
        printf("这个channel里的fd_index: %d\t", channel->users[i]);
    } */

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (channel->users[i] == client->sockfd)
        {
            return 1;
        }
    }
    return 0;
}

void handle_privmsg_command(int fd_index, int client_index, char* message) {

    char* target = strtok(message + 8, " ");
    printf("join命令收到的channel是\n%send\n", target);
    char* msg = strtok(NULL, "\r\n");

    if (!target) {
        send_error_message(fd_index, client_index, ERR_NORECIPIENT, "No recipient given (PRIVMSG)");
        return;
    }

    if (!msg) {
        send_error_message(fd_index, client_index, ERR_NOTEXTTOSEND, "No text to send");
        return;
    }

    if (target[0] == '#' || target[0] == '&') {

        Channel* channel = find_channel(target);

        if (channel == NULL) {
            send_error_message(fd_index, client_index, ERR_NOSUCHNICK, "No such nick/channel");
            return;
        }

        Client* sender = &clients[client_index];

        if (is_client_in_channel(sender, channel) == 0) {
            send_error_message(fd_index, client_index, ERR_CANNOTSENDTOCHAN, "Cannot send to channel");
            return;
        }

        if (channel->is_check_mode == 1)
        {
            //当频道处于审核模式时，只有频道管理员和voic权限的成员才能发送消息，其余将收到ERR_CANNOTSENDTOCHAN回复
            if (channel->user_status[client_index].is_channel_operator == 1 || channel->user_status[client_index].has_voice == 1 || clients[client_index].is_operator == 1)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    int recipient_index = channel->users[i];

                    if (recipient_index != -1 && recipient_index != sender->sockfd) {

                        char response[512];
                        snprintf(response, sizeof(response), ":%s PRIVMSG %s %s\r\n", complete_name[client_index], target, msg);
                        send(connfd[recipient_index], response, strlen(response), 0);
                    }
                }
            }
            else {
                send_error_message(fd_index, client_index, ERR_CANNOTSENDTOCHAN, "Cannot send to channel");
            }
            return;
        }
        else {
            // 向频道的所有用户发送消息
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                int recipient_index = channel->users[i];

                if (recipient_index != -1 && recipient_index != sender->sockfd) {
                    char response[512];
                    snprintf(response, sizeof(response), ":%s PRIVMSG %s %s\r\n", complete_name[client_index], target, msg);
                    send(connfd[recipient_index], response, strlen(response), 0);
                }
            }
        }

    }
    else {

        // 查找目标用户
        int target_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_registered && strcmp(clients[i].nickname, target) == 0) {
                target_index = i;
                break;
            }
        }

        if (target_index == -1) {
            send_error_message(fd_index, client_index, ERR_NOSUCHNICK, "No such nick/channel");
            return;
        }

        // 构建并发送消息
        char response[512];
        snprintf(response, sizeof(response), ":%s PRIVMSG %s %s\r\n", complete_name[client_index], target, msg);
        send(connfd[target_index], response, strlen(response), 0);
    }
}

void handle_notice_command(int fd_index, int client_index, char* message) {

    char* target = strtok(message + 7, " ");
    char* msg = strtok(NULL, "\r\n");

    if (!target || !msg) {
        return; // NOTICE 不会发送错误消息
    }
    // 检查目标是否是频道
    if (target[0] == '#' || target[0] == '&') {

        Channel* channel = find_channel(target);

        if (channel == NULL) {
            return; // 如果目标频道不存在，不发送任何消息
        }

        Client* sender = &clients[client_index];

        if (is_client_in_channel(sender, channel) == 0) {
            return; // 用户未加入频道，不发送任何消息
        }

        if (channel->is_check_mode == 1)
        {
            //当频道处于审核模式时，只有频道管理员和voic权限的成员才能发送消息，其余将收到ERR_CANNOTSENDTOCHAN回复
            if (channel->user_status[client_index].is_channel_operator == 1 || channel->user_status[client_index].has_voice == 1)
            {
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    int recipient_index = channel->users[i];

                    if (recipient_index != -1 && recipient_index != sender->sockfd) {

                        char response[512];
                        snprintf(response, sizeof(response), ":%s PRIVMSG %s %s\r\n", complete_name[client_index], target, msg);
                        send(connfd[recipient_index], response, strlen(response), 0);
                    }
                }
            }
            else {
                send_error_message(fd_index, client_index, ERR_CANNOTSENDTOCHAN, "Cannot send to channel");
            }
            return;
        }
        else {
            // 向频道的所有用户发送消息
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                int recipient_index = channel->users[i];

                if (recipient_index != -1 && recipient_index != sender->sockfd) {
                    char response[512];
                    snprintf(response, sizeof(response), ":%s PRIVMSG %s %s\r\n", complete_name[client_index], target, msg);
                    send(connfd[recipient_index], response, strlen(response), 0);
                }
            }
        }
    }
    else {
        // 查找目标用户
        int target_index = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_registered && strcmp(clients[i].nickname, target) == 0) {
                target_index = i;
                break;
            }
        }

        if (target_index == -1) {
            return; // 如果目标用户不存在，不发送任何消息
        }

        // 构建并发送消息
        char response[512];
        snprintf(response, sizeof(response), ":%s NOTICE %s %s\r\n", complete_name[client_index], target, msg);
        send(connfd[target_index], response, strlen(response), 0);
    }
}

void handle_ping_command(int fd_index, int client_index, char* message) {     //消息解析还是有问题

    char* origin = strtok(message + 5, " \r\n");

    char* dest = strtok(NULL, " \r\n");

    if (!origin) {
        send_error_message(fd_index, client_index, ERR_NOORIGIN, "No origin specified"); // ERR_NOORIGIN
        return;
    }

    char response[512];
    if (dest) {
        snprintf(response, sizeof(response), "PONG %s %s\r\n", origin, dest);
    }
    else {
        snprintf(response, sizeof(response), "PONG %s\r\n", origin);
    }

    send(connfd[fd_index], response, strlen(response), 0);
}

void handle_pong_command(int fd_index, int client_index, char* message) {

    char* origin = strtok(message + 5, " \r\n");
    char* dest = strtok(NULL, " \r\n");

    if (!origin) {
        send_error_message(fd_index, client_index, ERR_NOORIGIN, "No origin specified"); // ERR_NOORIGIN
        return;
    }

    // Typically no response is needed for PONG unless it's a forward
    if (dest) {
        char response[512];
        snprintf(response, sizeof(response), "PONG %s %s\r\n", origin, dest);
        send(connfd[fd_index], response, strlen(response), 0);
    }
}

void handle_motd_command(int fd_index, int client_index, char* message) {

    FILE* file = fopen(MOTD_FILE, "r");

    char response[512];

    if (file == NULL) {
        snprintf(response, sizeof(response), ":%s %d %s :MOTD File is missing\r\n", SERVER_NAME, ERR_NOMOTD, clients[client_index].nickname);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    snprintf(response, sizeof(response), ":%s %d %s :- %s Message of the day -\r\n", SERVER_NAME, RPL_MOTDSTART, clients[client_index].nickname, SERVER_NAME);
    send(connfd[fd_index], response, strlen(response), 0);

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        snprintf(response, sizeof(response), ":%s %d %s :- %s", SERVER_NAME, RPL_MOTD, clients[client_index].nickname, line);
        send(connfd[fd_index], response, strlen(response), 0);
    }

    fclose(file);

    snprintf(response, sizeof(response), ":%s %d %s :End of MOTD command\r\n", SERVER_NAME, RPL_ENDOFMOTD, clients[client_index].nickname);
    send(connfd[fd_index], response, strlen(response), 0);
}

void handle_lusers_command(int fd_index, int client_index, char* message) {

    int registered_users = 0;
    int operators = 0;
    int unknown_connections = 0;
    int channels_number = 0;            // Assuming channels count will be handled separately
    int total_connections = 0;

    // Calculate the number of registered users, operators, unknown connections, and total connections
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_registered == 1) {

            registered_users++;

            if (clients[i].is_operator == 1) {
                operators++;
            }
        }

        if (connfd[i] != -1) {
            total_connections++;
        }

    }

    unknown_connections = total_connections - registered_users;

    for (int i = 0;i < MAX_CHANNELS;i++) {
        if (channels[i].is_inuse == 1) {
            channels_number++;
        }
    }

    char response[512];

    // RPL_LUSERCLIENT: Show the number of registered users
    snprintf(response, sizeof(response), ":%s %d %s :There are %d users and 0 services on 1 server\r\n", SERVER_NAME, RPL_LUSERCLIENT, clients[client_index].nickname, registered_users);
    send(connfd[fd_index], response, strlen(response), 0);

    // RPL_LUSEROP: Show the number of operators
    snprintf(response, sizeof(response), ":%s %d %s %d :operator(s) online\r\n", SERVER_NAME, RPL_LUSEROP, clients[client_index].nickname, operators);
    send(connfd[fd_index], response, strlen(response), 0);

    // RPL_LUSERUNKNOWN: Show the number of unknown connections
    snprintf(response, sizeof(response), ":%s %d %s %d :unknown connection(s)\r\n", SERVER_NAME, RPL_LUSERUNKNOWN, clients[client_index].nickname, unknown_connections);
    send(connfd[fd_index], response, strlen(response), 0);

    // RPL_LUSERCHANNELS: Show the number of channels
    snprintf(response, sizeof(response), ":%s %d %s %d :channels formed\r\n", SERVER_NAME, RPL_LUSERCHANNELS, clients[client_index].nickname, channels_number);
    send(connfd[fd_index], response, strlen(response), 0);

    // RPL_LUSERME: Show the total number of clients (including unknown connections)
    snprintf(response, sizeof(response), ":%s %d %s :I have %d clients and 1 server\r\n", SERVER_NAME, RPL_LUSERME, clients[client_index].nickname, total_connections);
    send(connfd[fd_index], response, strlen(response), 0);

}

void handle_whois_command(int fd_index, int client_index, char* message) {

    char* nickname = strtok(message + 6, " \r\n");

    // 如果没有提供参数，返回ERR_NEEDMOREPARAMS错误
    if (!nickname) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    int target_index = -1;

    // 查找目标用户
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sockfd != -1 && clients[i].is_registered && strcmp(clients[i].nickname, nickname) == 0) {
            target_index = i;
            break;
        }
    }

    // 如果找不到用户，则发送 ERR_NOSUCHNICK
    if (target_index == -1) {
        char response[512];
        snprintf(response, sizeof(response), ":%s %d %s %s :No such nick/channel\r\n", SERVER_NAME, ERR_NOSUCHNICK, clients[client_index].nickname, nickname);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    // 发送 RPL_WHOISUSER
    char response[512];
    snprintf(response, sizeof(response), ":%s %d %s %s %s %s * :%s\r\n", SERVER_NAME, RPL_WHOISUSER, clients[client_index].nickname, clients[target_index].nickname, clients[target_index].username, clients[target_index].hostname, clients[target_index].fullname);
    send(connfd[fd_index], response, strlen(response), 0);

    // 发送 RPL_WHOISSERVER
    snprintf(response, sizeof(response), ":%s %d %s %s %s :%s\r\n", SERVER_NAME, RPL_WHOISSERVER, clients[client_index].nickname, clients[target_index].nickname, SERVER_NAME, "Server info");
    send(connfd[fd_index], response, strlen(response), 0);

    // 发送 RPL_ENDOFWHOIS
    snprintf(response, sizeof(response), ":%s %d %s %s :End of WHOIS list\r\n", SERVER_NAME, RPL_ENDOFWHOIS, clients[client_index].nickname, clients[target_index].nickname);
    send(connfd[fd_index], response, strlen(response), 0);

}


Channel* find_or_create_channel(const char* channel_name) {

    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (strcmp(channels[i].name, channel_name) == 0) {
            return &channels[i];
        }
    }

    int flag = 0;
    int i;
    for (i = 0;i < MAX_CHANNELS;i++) {
        if (channels[i].is_inuse == -1) {
            flag = 1;
            break;
        }
    }
    if (flag == 1) {
        strcpy(channels[i].name, channel_name);
        channels[i].user_count = 0;     //在频道中的用户数置为0
        channel_count++;
        channels[i].is_inuse = 1;
        return &channels[i];     //先返回，再加一
    }
    else
        return NULL; // No space for new channels
}

void add_user_to_channel(Channel* channel, Client* client) {

    int i = 0;
    int flag = 0;
    for (i;i < MAX_CLIENTS;i++) {
        if (channel->users[i] == -1)
        {
            flag = 1;
            break;
        }
    }
    //加入频道的第一个用户（频道创建者）自动设置为频道管理员
    if (i == 0) {
        channel->user_status[0].is_channel_operator = 1;
    }
    if (flag == 1) {
        channel->users[i] = client->sockfd;
        channel->user_count++;
    }

    for (int i = 0;i < channel->user_count;i++) {
        printf("存的user的sockfd是: %d\n", channel->users[i]);
    }

}

void handle_join_command(int sockfd_index, int client_index, char* message) {

    char* cmd = strtok(message, " ");

    char* channel_name = strtok(NULL, " \r\n");
    printf("join时解析出的channelname\n");
    printf("%send\n", channel_name);

    char response[512];

    if (channel_name == NULL) {
        send_error_message(sockfd_index, client_index, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
        return;
    }
    if (channel_name[0] == '#' || channel_name[0] == '&')
    {

        Client* client = &clients[client_index];

        Channel* channel = find_or_create_channel(channel_name);

        if (channel == NULL) {
            // Handle error if channel could not be created
            printf("No space for new channels\n");
            return;
        }

        add_user_to_channel(channel, client);

        if (strlen(channel->topic) > 0) {
            char topic_reply[512];
            snprintf(topic_reply, sizeof(topic_reply), "%s :%s", channel->name, channel->topic);

            send_reply(sockfd_index, RPL_TOPIC, topic_reply);
        }

        //调用names的函数，发送该channnel的成员列表回复
        send_names_reply(sockfd_index, client_index, channel_name);

        snprintf(response, sizeof(response), "%s 366 %s * :End of NAMES list\r\n", SERVER_NAME, clients[client_index].nickname);
        send(connfd[sockfd_index], response, strlen(response), 0);

        for (int i = 0; i < MAX_CLIENTS; i++)       //转发给这一频道的所有用户，包括自己
        {

            int recipient_index = channel->users[i];

            if (recipient_index != -1) {
                snprintf(response, sizeof(response), ":%s JOIN %s \r\n", complete_name[client_index], channel_name);
                send(connfd[recipient_index], response, strlen(response), 0);
            }
        }

    }
    else {
        send_error_message(sockfd_index, client_index, ERR_NEEDMOREPARAMS, "JOIN :channel name must start with # or &");
        return;
    }

}

void handle_part_command(int fd_index, int client_index, char* message) {

    int flag = 0;
    char response[512];
    char* token = strtok(message + 5, "\r\n");

    if (!token) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "Not enough parameters");
        return;
    }

    char* target = strtok(token, " ");
    printf("target is:%send\n", target);

    Channel* channel = find_channel(target);

    Client* sender = &clients[client_index];

    if (target[0] == '#' || target[0] == '&') {

        if (channel == NULL) {
            snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, target);
            send(connfd[fd_index], response, strlen(response), 0);
            return;
        }

        if (is_client_in_channel(sender, channel) == 0) {
            snprintf(response, sizeof(response), ":%s %d %s %s :You're not on that channel\r\n", SERVER_NAME, ERR_NOTONCHANNEL, clients[client_index].nickname, target);
            send(connfd[fd_index], response, strlen(response), 0);
            return;
        }
    }
    else {

        snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, target);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    char* msg = strtok(NULL, ":");
    if (msg != NULL) {
        flag = 1;
    }
    printf("msg is:%send", msg);


    if (flag == 1)
    {        //flag==1说明有消息，则需要先转发消息
        printf("是进入了part发消息的\n");
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int recipient_index = channel->users[i];

            if (recipient_index != -1 && recipient_index != sender->sockfd) {
                snprintf(response, sizeof(response), ":%s PART %s %s\r\n", complete_name[client_index], target, msg);
                send(connfd[recipient_index], response, strlen(response), 0);
            }
        }
    }

    for (int i = 0;i < MAX_CLIENTS;i++) {
        if (channel->users[i] == fd_index) {
            channel->users[i] = -1;
            break;
        }
    }
    channel->user_count--;

    check_destroy_channel(channel);
}

void check_destroy_channel(Channel* channel) {
    int i;
    int flag = 0;
    for (i = 0;i < MAX_CLIENTS;i++) {
        if (channel->users[i] != -1) {
            flag = 1;
            break;
        }
    }
    if (flag == 0 && channel->user_count == 0) {
        channel->is_inuse = -1;
        channel->user_count = 0;
        memset(channel->name, 0, NAME_SIZE);
        memset(channel->topic, 0, 256);
        printf("%s 该channel无人使用，已删除\n", channel->name);
    }
}


void handle_topic_command(int fd_index, int client_index, char* message) {

    char response[512];

    char* token = strtok(message + 6, "\r\n");

    char* channel_name = strtok(token, " ");

    printf("channel is%send\n", channel_name);

    char* new_topic = strtok(NULL, ":");
    printf("new topic is%send\n", new_topic);

    if (token == NULL) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
        return;
    }

    Channel* channel = find_channel(channel_name);

    Client* sender = &clients[client_index];


    if (channel_name[0] == '#' || channel_name[0] == '&') {

        if (channel == NULL) {
            snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, channel_name);
            send(connfd[fd_index], response, strlen(response), 0);
            return;
        }

        if (is_client_in_channel(sender, channel) == 0) {
            snprintf(response, sizeof(response), ":%s %d %s %s :You're not on that channel\r\n", SERVER_NAME, ERR_NOTONCHANNEL, clients[client_index].nickname, channel_name);
            send(connfd[fd_index], response, strlen(response), 0);
            return;
        }

    }
    else {
        send_error_message(fd_index, client_index, ERR_NOSUCHCHANNEL, "No such channel");
        return;
    }

    if (new_topic == NULL) {
        // Get the current topic
        if (strlen(channel->topic) == 0) {
            snprintf(response, sizeof(response), ":%s %d %s %s :No topic is set\r\n", SERVER_NAME, RPL_NOTOPIC, sender->nickname, channel->name);
            send(connfd[fd_index], response, strlen(response), 0);
        }
        else {
            snprintf(response, sizeof(response), ":%s %d %s %s :%s\r\n", SERVER_NAME, RPL_TOPIC, clients[client_index].nickname, channel->name, channel->topic);
            send(connfd[fd_index], response, strlen(response), 0);
        }
    }
    else {
        int flag = 1;
        if (channel->is_topic_mode == 1) {
            if (channel->user_status[client_index].is_channel_operator == 1 || clients[client_index].is_operator == 1)
                flag = 1;
            else
                flag = 0;
        }
        else {
            flag = 1;
        }

        if (flag == 1)
        {
            // Set a new topic
            strncpy(channel->topic, new_topic, 100);
            channel->topic[99] = '\0'; // Ensure null-termination

            snprintf(response, sizeof(response), ":%s TOPIC %s :%s\r\n", complete_name[client_index], channel->name, channel->topic);
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                int member_fd_index = channel->users[i];
                if (member_fd_index != -1)
                    send(connfd[member_fd_index], response, strlen(response), 0);
            }

        }
        else {
            snprintf(response, sizeof(response), ":%s %d %s %s :You're not channel operator\r\n", SERVER_NAME, ERR_CHANOPRIVSNEEDED, clients[client_index].nickname, channel->name);
            send(connfd[fd_index], response, strlen(response), 0);
            return;
        }

    }
}


void handle_oper_command(int fd_index, int client_index, char* message) {

    char* user = strtok(message + 5, " ");

    char* password = strtok(NULL, "\r\n");

    if (!user || !password) {
        send_error_message(fd_index, client_index, ERR_NEEDMOREPARAMS, "OPER :Not enough parameters");
        return;
    }

    if (strcmp(password, operator_password) != 0) {
        send_error_message(fd_index, client_index, ERR_PASSWDMISMATCH, "OPER :Password incorrect");
        return;
    }

    clients[client_index].is_operator = 1;

    char response[512];
    snprintf(response, sizeof(response), ":%s %d %s :You are now an IRC operator\r\n", SERVER_NAME, RPL_YOUREOPER, clients[client_index].nickname);
    send(connfd[fd_index], response, strlen(response), 0);
}




void handle_mode_command(int fd_index, int client_index, char* message) {

    char* token = strtok(message + 5, "\r\n");

    char* channel_name = strtok(token, " ");
    printf("channelname is:%send\n", channel_name);

    char* mode_string = strtok(NULL, " ");
    printf("mode_string is:%send\n", mode_string);

    char* target_nick = strtok(NULL, " ");
    printf("target nick is:%send\n", target_nick);

    char response[BUFFER_SIZE];

    if (!channel_name) {
        send_error_message(fd_index, client_index, ERR_NOSUCHCHANNEL, "No channel name provided");
        return;
    }

    Channel* channel = find_channel(channel_name);
    if (channel == NULL) {
        snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, channel_name);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    if (mode_string == NULL) {
        send_channel_mode(fd_index, client_index, channel);
        return;
    }

    if (mode_string[0] == '+' || mode_string[0] == '-') {
        // Handle user modes
        if (target_nick != NULL) {
            handle_member_mode(fd_index, client_index, channel, mode_string, target_nick);
        }
        else {
            handle_channel_mode(fd_index, client_index, channel, mode_string);
        }
    }
    else {
        snprintf(response, sizeof(response), ":%s %d %s %s :is unknown mode char to me for %s\r\n", SERVER_NAME, ERR_UNKNOWNMODE, clients[client_index].nickname, mode_string, channel_name);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

}

void send_channel_mode(int fd_index, int client_index, Channel* channel) {

    char response[BUFFER_SIZE];
    char modes[20] = "";
    if (channel->is_check_mode == 0) {
        strcat(modes, "-m");
    }
    else if (channel->is_check_mode == 1)
    {
        strcat(modes, "+m");
    }
    strcat(modes, " ");

    if (channel->is_topic_mode == 0) {
        strcat(modes, "-t");
    }
    else if (channel->is_topic_mode == 1) {
        strcat(modes, "+t");
    }
    snprintf(response, sizeof(response), ":%s 324 %s %s %s\r\n",
        SERVER_NAME, clients[client_index].nickname, channel->name,
        modes);
    send(connfd[fd_index], response, strlen(response), 0);
}


void handle_channel_mode(int fd_index, int client_index, Channel* channel, const char* mode_string) {

    char response[BUFFER_SIZE];

    if (is_channel_operator(channel, client_index) == 0) {
        snprintf(response, sizeof(response), ":%s %d %s %s :You're not channel operator\r\n", SERVER_NAME, ERR_CHANOPRIVSNEEDED, clients[client_index].nickname, channel->name);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    if (strcmp(mode_string, "+m") == 0) {
        printf("+m\n");
        channel->is_check_mode = 1;
    }
    else if (strcmp(mode_string, "-m") == 0) {
        printf("-m\n");
        channel->is_check_mode = 0;
    }
    else if (strcmp(mode_string, "+t") == 0) {
        channel->is_topic_mode = 1;
        printf("+t\n");
    }
    else if (strcmp(mode_string, "-t") == 0) {
        channel->is_topic_mode = 0;
        printf("-t\n");
    }
    else {
        printf("进入了else部分\n");

        snprintf(response, sizeof(response), "%s %d %s %s %s :Unknown MODE flag\r\n",
            SERVER_NAME, ERR_UMODEUNKNOWNFLAG, clients[client_index].nickname, channel->name,
            mode_string);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    snprintf(response, sizeof(response), ":%s MODE %s %s \r\n",
        complete_name[client_index], channel->name,
        mode_string);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int user_index = channel->users[i];
        if (user_index != -1)
            send(connfd[user_index], response, strlen(response), 0);
    }
}

void handle_member_mode(int fd_index, int client_index, Channel* channel, const char* mode_string, const char* target_nick)
{
    char response[BUFFER_SIZE];

    int target_index = find_client_by_nick(target_nick);

    if (target_index == -1) {
        send_error_message(fd_index, client_index, ERR_NOSUCHNICK, "No such nick!");
        return;
    }

    int flag = 0;
    for (int i = 0;i < MAX_CLIENTS;i++) {
        if (channel->users[i] == clients[target_index].sockfd)
        {
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        snprintf(response, sizeof(response), ":%s %d %s %s %s :This nick is not on that channel\r\n", SERVER_NAME, ERR_NOTONCHANNEL, clients[client_index].nickname, channel->name, target_nick);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }


    if (is_channel_operator(channel, client_index) == 0) {
        snprintf(response, sizeof(response), ":%s %d %s %s :You're not channel operator\r\n", SERVER_NAME, ERR_CHANOPRIVSNEEDED, clients[client_index].nickname, channel->name);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    if (strcmp(mode_string, "+o") == 0) {
        set_channel_user_status(channel, target_index, 'o', 1);
    }
    else if (strcmp(mode_string, "-o") == 0) {
        set_channel_user_status(channel, target_index, 'o', 0);
    }
    else if (strcmp(mode_string, "+v") == 0) {
        set_channel_user_status(channel, target_index, 'v', 1);
    }
    else if (strcmp(mode_string, "-v") == 0) {
        set_channel_user_status(channel, target_index, 'v', 0);
    }
    else {
        snprintf(response, sizeof(response), "%s %d %s %s %s :Unknown MODE flag\r\n",
            SERVER_NAME, ERR_UMODEUNKNOWNFLAG, clients[client_index].nickname, channel->name,
            mode_string);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    snprintf(response, sizeof(response), ":%s MODE %s %s %s\r\n",
        complete_name[client_index], channel->name,
        mode_string, target_nick);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        int user_index = channel->users[i];
        if (user_index != -1)
            send(connfd[user_index], response, strlen(response), 0);
    }
}

void set_channel_user_status(Channel* channel, int user_index, char mode, int value) {

    for (int i = 0; MAX_CLIENTS; i++) {
        if (i == user_index) {
            if (mode == 'o') {
                channel->user_status[i].is_channel_operator = value;
            }
            else if (mode == 'v') {
                channel->user_status[i].has_voice = value;
            }
            printf("是进入了改变模式函数的\n");
            return;
        }
    }
}

int find_client_by_nick(const char* nick)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {          //返回的是目标nick的client_index
        if (clients[i].is_registered == 1 && strcmp(clients[i].nickname, nick) == 0) {
            return i;
        }
    }
    return -1;
}

int is_channel_operator(Channel* channel, int client_index) {

    if (clients[client_index].is_operator == 1) {
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (i == client_index) {
            if (channel->user_status[i].is_channel_operator == 1) {
                return 1;
            }

        }
    }
    return 0;
}




void handle_list_command(int fd_index, int client_index, char* message) {

    char response[BUFFER_SIZE];
    char* channel_name = strtok(message + 5, "\r\n");

    if (!channel_name) {
        // 无参数，返回所有频道的列表
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (channels[i].is_inuse == 1) {
                snprintf(response, sizeof(response), ":%s 322 %s %s %d :%s\r\n", SERVER_NAME, clients[client_index].nickname, channels[i].name, channels[i].user_count, channels[i].topic);
                send(connfd[fd_index], response, strlen(response), 0);
            }
        }
    }
    else {
        // 单个参数，返回指定频道的列表
        Channel* channel = find_channel(channel_name);
        if (channel != NULL) {
            snprintf(response, sizeof(response), ":%s 322 %s %s %d :%s\r\n", SERVER_NAME, clients[client_index].nickname, channel->name, channel->user_count, channel->topic);
            send(connfd[fd_index], response, strlen(response), 0);
        }
        else {
            snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, channel_name);
            send(connfd[fd_index], response, strlen(response), 0);
        }
    }

    // 发送列表结束回复
    snprintf(response, sizeof(response), ":%s 323 %s :End of LIST\r\n", SERVER_NAME, clients[client_index].nickname);
    send(connfd[fd_index], response, strlen(response), 0);
}


void handle_names_command(int fd_index, int client_index, char* message) {

    char response[BUFFER_SIZE];
    char* channel_name = strtok(message + 6, "\r\n");

    if (!channel_name) {
        // 无参数，返回所有频道的成员
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (channels[i].is_inuse == 1) {
                send_names_reply(fd_index, client_index, channels[i].name);
            }
        }

        // 返回不在任何频道的用户
        char nicknames[BUFFER_SIZE] = "";
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].sockfd != -1 && is_user_in_any_channel(i) == 0) {
                strcat(nicknames, clients[i].nickname);
                strcat(nicknames, " ");
            }
        }

        if (strlen(nicknames) > 0) {

            snprintf(response, sizeof(response), "%s 353 %s = %s :%s\r\n", SERVER_NAME, clients[client_index].nickname, channel_name, nicknames);
            send(connfd[fd_index], response, strlen(response), 0);
        }

        snprintf(response, sizeof(response), "%s 366 %s * :End of NAMES list\r\n", SERVER_NAME, clients[client_index].nickname);
        send(connfd[fd_index], response, strlen(response), 0);
    }
    else {
        // 单个参数，返回指定频道的成员
        Channel* channel = find_channel(channel_name);
        if (channel != NULL) {
            send_names_reply(fd_index, client_index, channel->name);
        }

        snprintf(response, sizeof(response), "%s 366 %s * :End of NAMES list\r\n", SERVER_NAME, clients[client_index].nickname);
        send(connfd[fd_index], response, strlen(response), 0);
    }
}


void send_names_reply(int fd_index, int client_index, const char* channel_name) {

    char response[512];
    Channel* channel = find_channel(channel_name);
    if (channel == NULL)
    {
        snprintf(response, sizeof(response), ":%s %d %s %s :No such channel\r\n", SERVER_NAME, ERR_NOSUCHCHANNEL, clients[client_index].nickname, channel_name);
        send(connfd[fd_index], response, strlen(response), 0);
        return;
    }

    char nicknames[BUFFER_SIZE] = "";

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (channel->users[i] != -1) {
            if (channel->user_status[i].is_channel_operator == 1) {
                strcat(nicknames, "@");
            }
            else if (channel->user_status[i].has_voice == 1) {
                strcat(nicknames, "+");
            }

            for (int j = 0;j < MAX_CLIENTS;j++) {          //找在channel里每个用户在clients里存的nickname
                if (clients[j].sockfd != -1 && clients[j].sockfd == channel->users[i]) {
                    strcat(nicknames, clients[i].nickname);
                    strcat(nicknames, " ");
                }
            }

        }
    }
    snprintf(response, sizeof(response), "%s 353 %s = %s :%s\r\n", SERVER_NAME, clients[client_index].nickname, channel_name, nicknames);
    send(connfd[fd_index], response, strlen(response), 0);
}

int is_user_in_any_channel(int client_index) {
    for (int i = 0; i < MAX_CHANNELS; i++) {
        for (int j = 0;j < MAX_CLIENTS;j++) {
            if (channels[i].is_inuse == 1 && channels[i].users[j] == clients[client_index].sockfd)
            {
                return 1;
            }
        }
    }
    return 0;
}


void handle_away_command(int fd_index, int client_index, char* message) {

    char* away_message = strtok(message + 5, "\r\n");

    if (away_message == NULL) {
        clients[client_index].is_away = 1; // 取消离开模式
        char response[512];
        snprintf(response, sizeof(response), ":%s 305 %s :You are no longer marked as being away\r\n", SERVER_NAME, clients[client_index].nickname);
        send(connfd[fd_index], response, strlen(response), 0);
    }
    else {
        clients[client_index].is_away = 0; // 设置离开模式
        char response[512];
        snprintf(response, sizeof(response), ":%s 306 %s :You have been marked as being away\r\n", SERVER_NAME, clients[client_index].nickname);
        send(connfd[fd_index], response, strlen(response), 0);
    }
}


void send_error_message(int fd_index, int client_index, int error_code, const char* message) {

    char response[512];
    snprintf(response, sizeof(response), ":%s %d %s :%s\r\n", SERVER_NAME, error_code, clients[client_index].nickname, message);
    send(connfd[fd_index], response, strlen(response), 0);
}

void send_reply(int fd_index, int reply_code, const char* msg) {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), ":%s %d %s\r\n", SERVER_NAME, reply_code, msg);
    send(connfd[fd_index], buffer, strlen(buffer), 0);
}

void handle_quit_command(int fd_index, int client_index, char* message) {

    char response[BUFFER_SIZE];
    int flag = 0;

    char* quit_message = strtok(message + 5, "\r\n");

    if (!quit_message) {
        snprintf(response, sizeof(response), "ERROR :Closing Link: %s (Client exited)\r\n", clients[client_index].hostname);
        send(connfd[fd_index], response, strlen(response), 0);
    }

    // 向用户所在的所有频道发送离开通知
    if (quit_message != NULL) {
        snprintf(response, sizeof(response), ":%s QUIT :%s\r\n", complete_name[client_index], quit_message);
        flag = 1;
    }
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].is_inuse == 1) {
            for (int j = 0; j < MAX_CLIENTS; j++) {
                if (channels[i].users[j] == fd_index) {
                    // 向频道中的所有用户发送QUIT消息
                    for (int k = 0; k < MAX_CLIENTS; k++) {
                        if (channels[i].users[k] != -1 && channels[i].users[k] != fd_index) {
                            if (flag == 1)
                                send(connfd[channels[i].users[k]], response, strlen(response), 0);
                        }
                    }
                    // 将用户从频道中移除
                    channels[i].users[j] = -1;
                    channels[i].user_count--;
                }
            }
            // 如果频道已空，销毁频道
            if (channels[i].user_count == 0) {
                channels[i].is_inuse = 0;
            }
        }
    }


    // 关闭连接
    close(connfd[fd_index]);

    connfd[fd_index] = -1;
    clients[client_index].is_registered = 0;
    clients[client_index].sockfd = -1;

    int ret;
    pthread_exit(&ret);
}