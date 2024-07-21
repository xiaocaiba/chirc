#ifndef SERVER_H
#define SERVER_H

#define PORT 6667
#define MAX_CLIENTS 100
#define BUFFER_SIZE 512
#define NAME_SIZE 100
#define MAX_CHANNELS 100


typedef struct {
    int sockfd_index;
    char client_ip[16];
} thread_arg_t;       //传参数的结构体


typedef struct {

    int sockfd;
    char nickname[50];          //nick是临时的，动态的，相当于一个简称
    char username[50];        //一般是系统名，用户可以指定
    char fullname[50];       //展示信息的时候可能有点用
    char hostname[50];        //其实就是域名，为了方便直接在注册时指定ip
    int is_registered;        //0表示未注册，1表示已注册
    int is_operator;         //0表示普通用户，1表示为操作员
    int is_away;               //0表示离开  1表示在线
} Client;

typedef struct {
    int user_index;
    int is_channel_operator; // 0 表示普通用户，1 表示频道操作员
    int has_voice;   // 0 表示没有语音权限，1 表示有语音权限
} ChannelUserStatus;

typedef struct {
    int is_inuse;
    char name[100];
    char topic[256];
    int  users[MAX_CLIENTS];
    int user_count;
    int is_check_mode;            //审核模式初始化为0，1表示审核模式
    int is_topic_mode;             //主题模式初始化为0，1表示主题模式
    ChannelUserStatus user_status[MAX_CLIENTS]; // 用户状态数组
} Channel;

void init_server(int port, char* password);

#endif
