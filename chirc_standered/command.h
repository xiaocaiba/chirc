#ifndef COMMAND_H
#define COMMAND_H

#define MOTD_FILE "motd.txt"
#define SERVER_NAME "local_server.com"

#define CHANNEL_MODE_AUDIT 'm'
#define CHANNEL_MODE_TOPIC 't'

#define ERR_NONICKNAMEGIVEN 431          //没有输入nickmame
#define ERR_NICKNAMEINUSE 433            //nickname占用
#define ERR_NEEDMOREPARAMS 461              //需要更多参数
#define ERR_ALREADYREGISTRED 462            //已注册
#define ERR_NOTREGISTERED 451               //未注册

#define ERR_NORECIPIENT 411
#define ERR_NOTEXTTOSEND 412            //无输入文本
#define ERR_NOSUCHNICK 401              //没有这个nick
#define ERR_NOSUCHSERVER 402            //没有此服务器
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NOORIGIN 409

#define ERR_UNKNOWNCOMMAND 421          //未知命令

#define ERR_NOTONCHANNEL 442


#define ERR_NOMOTD 422

#define RPL_LUSERCLIENT 251
#define RPL_LUSEROP 252
#define RPL_LUSERUNKNOWN 253
#define RPL_LUSERCHANNELS 254
#define RPL_LUSERME 255

#define RPL_WHOISUSER 311
#define RPL_WHOISSERVER 312
#define RPL_ENDOFWHOIS 318

#define RPL_NOTOPIC 331
#define RPL_TOPIC 332
#define RPL_NAMREPLY 353
#define RPL_ENDOFNAMES 366

#define RPL_MOTDSTART 375
#define RPL_MOTD 372
#define RPL_ENDOFMOTD 376

#define ERR_PASSWDMISMATCH 464
#define RPL_YOUREOPER 381

#define ERR_CHANOPRIVSNEEDED 482
#define ERR_UNKNOWNMODE 472

#define ERR_USERNOTINCHANNEL 441
#define ERR_USERSDONTMATCH 502
#define ERR_UMODEUNKNOWNFLAG 501
#define RPL_CHANNELMODEIS 324




//用户的完整标识符，直接申请为全局二维数据，防止线程冲突
char complete_name[100][150];

int check_command_format(char* message);     //检查命令格式是否正确

void* process_command(void* arg);

void handle_nick_command(int fd_index, int client_index, char* message);

void handle_user_command(int fd_index, int client_index, char* message);

void check_registration(int client_index, int fd_index);

void send_welcome_messages(int fd_index, int client_index);

Channel* find_channel(const char* channel_name);

int is_client_in_channel(Client* client, Channel* channel);

void handle_privmsg_command(int fd_index, int client_index, char* message);

void handle_notice_command(int fd_index, int client_index, char* message);


void handle_ping_command(int fd_index, int client_index, char* message);

void handle_pong_command(int fd_index, int client_index, char* message);

void handle_motd_command(int fd_index, int client_index, char* message);

void handle_lusers_command(int fd_index, int client_index, char* message);

void handle_whois_command(int fd_index, int client_index, char* message);


Channel* find_or_create_channel(const char* channel_name);

void add_user_to_channel(Channel* channel, Client* client);

void handle_join_command(int sockfd_index, int client_index, char* message);

void handle_part_command(int fd_index, int client_index, char* message);

void check_destroy_channel(Channel* channel);

void handle_topic_command(int fd_index, int client_index, char* message);

void handle_oper_command(int fd_index, int client_index, char* message);

void handle_list_command(int fd_index, int client_index, char* message);

void handle_names_command(int fd_index, int client_index, char* message);

void send_names_reply(int fd_index, int client_index, const char* channel_name);

int is_user_in_any_channel(int client_index);

void handle_away_command(int fd_index, int client_index, char* message);

void handle_quit_command(int fd_index, int client_index, char* message);

void handle_mode_command(int fd_index, int client_index, char* message);

void send_channel_mode(int fd_index, int client_index, Channel* channel);

void handle_channel_mode(int fd_index, int client_index, Channel* channel, const char* mode_string);

void handle_member_mode(int fd_index, int client_index, Channel* channel, const char* mode_string, const char* target_nick);

void set_channel_user_status(Channel* channel, int user_index, char mode, int value);

int find_client_by_nick(const char* nick);

int is_channel_operator(Channel* channel, int client_index);


void send_error_message(int fd_index, int client_index, int error_code, const char* message);
void send_reply(int fd_index, int reply_code, const char* msg);


#endif
