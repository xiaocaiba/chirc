unix 网络编程课程设计
  1 设计目的 

本课题通过实现一个聊天中继服务器与多个客户端的通信，具体包括注册、登
录、私信、群发等功能，熟悉和掌握TCP套接字编程、UDP套接字编程、I/O复用、
线程、名字与地址转换等技术，加深对网络编程的理解。 
2 设计要求 
编写一个聊天客户端程序和一个中继服务器程序，实现一个服务器与多个客户端
的通信，具体需要满足以下要求。 
2.1 客户程序 
要求以服务器主机名或IP地址、以及服务名或端口号作为命令行参数，例如使用
udp_client、udp_connect 或 tcp_connect 函数实现。实际测试时可以使用环回IP地址
和端口号进行测试。 
使用select 或poll 等I/O 复用函数实现以下功能的并发： 
（1）使用UDP套接字，实现基于用户名和密码的注册功能。 
（2）使用TCP套接字，基于用户名和密码进行登录，登录成功后能够并发实现： 
（2.1）把键入消息发送给服务器，具体可以实现： 
a. 获取当前在线用户和已存在群组的列表； 
b. 私信聊天：向指定在线用户发送私信（通过服务器转发）； 
c. 
群组聊天：创建新的聊天群组、加入已存在的群组、在已经加入的
群组内发送消息、离开群组；  
d. 注销登录。 
（2.2）接收来自服务器的消息，并做相应提示。 
2.2 服务器程序 
对命令行参数没有特别要求。 
使用I/O复用、子进程、线程中至少两种设计范式实现以下功能的并发： 
（1）使用UDP套接字，处理来自客户的注册请求； 
（2）使用TCP套接字，能够并发实现： 
（2.1）处理来自新客户的登录请求，如果登录成功则通知所有在线客户； 
（2.2）接收来自在线客户的消息，具体可以实现： 
a. 提供当前在线客户和已存在群组的列表； 
b. 处理私信聊天：转发客户私信； 
c. 
处理群组聊天：处理新建群组、加入群组、群发消息、离开群组； 
d. 处理客户的注销请求并通知所有在线客户。 
（2.3）键入消息，具体可以实现： 
a. 发送给一个在线客户； 
b. 发送给已存在的群组； 
c. 
向所有在线客户广播。   
