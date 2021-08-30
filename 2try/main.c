
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

typedef struct s_client
{
	int fd;
	int id;
	struct s_client *next;
}							t_client;

int sockfd;
fd_set set, cpy_read, cpy_write;
int g_id = 0;
struct sockaddr_in servaddr;
t_client *cli = NULL;
char msg[42], str[42*4096];

void fatal()
{
	t_client *b;

	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sockfd);
	while (cli)
	{
		b = cli->next;
		close(cli->fd);
		free(cli);
		cli = b;
	}

	exit (1);
}

int get_id(int fd)
{
	t_client *b = cli;

	while (b)
	{
		if (b->fd == fd)
			return (b->id);
		b = b->next;
	}
	return (-1);
}

int get_max_fd()
{
	int max = sockfd;
	t_client *b = cli;

	while (b)
	{
		if (b->fd > max)
			max = b->fd;
		b = b->next;
	}
	return (max);
}

void send_all(int fd, char *s)
{
	t_client *b = cli;

	while (b)
	{
		if (b->fd != fd && FD_ISSET(b->fd, &cpy_write))
		{
			send(b->fd, s, strlen(s), 0);
		}
		b = b->next;
	}
}

int add_client_to_list(int fd)
{
	t_client *b = cli;
	t_client *new = NULL;

	if (!(new = malloc(sizeof(cli))))
		fatal();
	new->fd = fd;
	new->id = g_id;
	new->next = NULL;
	g_id++;
	if (!b)
	{
		cli = new;
	}
	else
	{
		while (b && b->next)
			b = b->next;
		b->next = new;
	}
	return (new->id);
}

void add_client()
{
	int connfd;
	connfd = accept(sockfd, NULL, NULL);
	if (connfd < 0) {
				fatal();
	}
	int id = add_client_to_list(connfd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d just arrived\n", id);
	send_all(connfd, msg);
	FD_SET(connfd, &set);
}

int rm_client_from_list(int fd)
{
	t_client *b = cli;
	t_client *del = NULL;
	int id = -1;

	if (b && b->fd == fd)
	{
		cli = b->next;
		// close(b->fd);
		id = b->id;
		free(b);
	}
	else
	{
		while (b && b->next)
		{
			if (b->next->fd == fd)
			{
				del = b->next;
				// close(del->fd);
				id = del->id;
				b->next = del->next;
				free(del);
			}
			b = b->next;
		}
	}
	return (id);
}

void rm_client(int fd)
{
	FD_CLR(fd, &set);
	close(fd);
	int id = rm_client_from_list(fd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d disconnected\n", id);
	send_all(fd, msg);

}

void ex_msg(int fd)
{
	int i = 0;

	char b[5000000] = {0};
	// char tmp[5000000] = {0};
	char announce[64];

	sprintf(announce, "client %d: ", get_id(fd));
	strcat(b, announce);
	int j = strlen(b);

	while (str[i])
	{
		b[j] = str[i];
		if  (str[i] == '\n')
		{
			if (str[i + 1])
			{
				strcat(b, announce);
				j += strlen(announce);
			}
		}
		i++;
		j++;
	}
	send_all(fd, b);
	bzero(&str, strlen(str));
}

int main(int ac, char **av) {
	int test = 0;
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		return (1);
	}
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		fatal();
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
			fatal();
	}
	if (listen(sockfd, 10) != 0) {
		fatal();
	}

	bzero(&msg, sizeof(msg));
	bzero(&str, sizeof(str));
	FD_ZERO(&set);
	FD_SET(sockfd, &set);

	while (1)
	{
		cpy_read = cpy_write = set;
		if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
			continue ;
		if (FD_ISSET(sockfd, &cpy_read))
		{
			add_client();
			continue ;
		}
		for (t_client *b = cli; b != NULL; b = b->next)
		{
			int ret;
			if (FD_ISSET(b->fd, &cpy_read))
			{
				ret = recv(b->fd, str, sizeof(str), 0);
				if (ret > 0)
				{
					ex_msg(b->fd);
					break ;
				}
				else if (ret == 0)
				{
					rm_client(b->fd);
					break ;
				}
				break ;
			}
		}
	}
	close(sockfd);
	return (0);
}
