
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct s_client
{
	int fd;
	int id;
	struct s_client *next;
}	t_client;

int sock_fd;
struct sockaddr_in servaddr;
int g_id = 0;
t_client *cli = NULL;
fd_set cli_set, cpy_read, cpy_write;
char str[42*4096], tmp[42*4096], buf[42*4096+42];
char msg[42];

void fatal()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	close(sock_fd);
	exit(1);
}

int get_id(int fd)
{
	t_client *b = cli;

	while (b)
	{
		if (fd == b->fd)
			return (b->id);
		b = b->next;
	}
	return (-1);
}

int get_max_fd()
{
	int max = sock_fd;
	t_client *b = cli;

	while (b)
	{
		if (max <= b->fd)
			max = b->fd;
		b = b->next;
	}
	return (max);
}

void send_all(char *s, int fd)
{
	t_client *b = cli;

	while (b)
	{
		if (b->fd != fd && FD_ISSET(b->fd, &cpy_write))
		{
			if (send(b->fd, s, strlen(s), 0) < 0)
				fatal();
		}
		b = b->next;
	}
}

int add_client_to_list(int fd)
{
	t_client *b = cli;
	t_client *new = NULL;

	if (!(new = malloc(sizeof(t_client))))
		fatal();
	g_id++;
	new->id = g_id;
	new->fd = fd;
	new->next = NULL;
	if (!b)
	{
		cli = new;
	}
	else
	{
		while (b->next)
			b = b->next;
		b->next = new;
	}
	return (new->id);
}

void add_client()
{
	int fd;
	int id;

	struct sockaddr_in client_sock;
	socklen_t len = sizeof(client_sock);

	fd = accept(sock_fd, (struct sockaddr *)&client_sock, &len);
	if (fd < 0)
		fatal();
	id = add_client_to_list(fd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d just arrived\n", id);
	send_all(msg, fd);
	FD_SET(fd, &cli_set);
}

int rm_client_from_list(int fd)
{
	int id;
	t_client *b = cli;
	t_client *del = NULL;

	if (b && b->fd == fd)
	{
		id = b->id;
		cli = b->next;
		free(b);
	}
	else
	{
		while (b && b->next)
		{
			if (b->next->fd == fd)
			{
				del = b->next;
				b->next = b->next->next;
				id = del->id;
				free(del);
			}
			b = b->next;
		}
	}
	return (id);
}

void rm_client(int fd)
{
	int id;

	id = rm_client_from_list(fd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d disconnected\n", id);
	send_all(msg, fd);
	FD_CLR(fd, &cli_set);
}

void ex_msg(int fd)
{
	int i = 0;
	int j = 0;

	while (str[i])
	{
		tmp[j] = str[i];
		j++;
		if (str[i] == '\n')
		{
			j = 0;
			sprintf(buf, "client %d: %s", get_id(fd), tmp);
			send_all(buf, fd);
			bzero(&tmp, sizeof(tmp));
			bzero(&buf, sizeof(buf));
		}
		i++;
	}
	bzero(&str, sizeof(str));
}

int main(int ac, char **av) {
	if (ac != 2)
	{
		write(2, "Wrong number of argument\n", strlen("Wrong number of argument\n"));
		return (1);
	}

	// socket create and verification
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		fatal();
	}
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

	if ((bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		fatal();
	}
	if (listen(sock_fd, 10) != 0) {
		fatal();
	}

	bzero(&str, sizeof(str));
	bzero(&tmp, sizeof(tmp));
	bzero(&msg, sizeof(msg));
	bzero(&buf, sizeof(buf));
	FD_ZERO(&cli_set);
	FD_SET(sock_fd, &cli_set);

	while (1)
	{
		cpy_read = cpy_write = cli_set;
		if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
			continue ;
		for (int fd = 0; fd < get_max_fd() + 1; fd++)
		{
			if (FD_ISSET(fd, &cpy_read))
			{
				if (fd == sock_fd)
				{
					// add client
					add_client();
					break ;
				}
				else
				{
					if (recv(fd, str, sizeof(str), 0) <= 0)
					{
					// client disconnected
						//printf("client disconnected\n");
						rm_client(fd);
						break ;
					}
					else
					{
					// printf("client sent msg\n");
						ex_msg(fd);
					// message received
					}
				}
			}
		}

	}
	close(sock_fd);
	return (0);
}
