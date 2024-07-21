# 定义编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -g
LDFLAGS = -lpthread

# 目标文件
TARGET = chirc

# 源文件
SRCS = main.c server.c command.c

# 生成的对象文件
OBJS = $(SRCS:.c=.o)

# 头文件
HEADERS = server.h command.h

# 生成最终可执行文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# 编译源文件生成对象文件
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# 清理编译生成的文件
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean

