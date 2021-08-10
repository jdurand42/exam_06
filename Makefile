SERVER_SRCS = ./serv.c

CLIENT_SRCS = ./client.c

all:
	gcc -o server $(SERVER_SRCS)
	gcc -o client $(CLIENT_SRCS)
