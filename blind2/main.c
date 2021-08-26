#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct	s_client
{
	int fd;
	int id;
	struct s_client *next;
}								t_client;


int sock_fd;
struct sockaddr_in servaddr, test;
int g_id = 0;
t_client *cli = NULL;

fd_set cli_set, cpy_read, cpy_write;
char msg[42];
char str[42*4096], tmp[42*4096], buf[42*4096 + 42];

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
		if (b->fd == fd)
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
/*
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}
*/

void send_all(int fd, char *s)
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

	g_id++;
	int id = g_id;

	if (!(new = malloc(sizeof(t_client))))
		fatal();

	new->next = NULL;
	new->id = id;
	new->fd = fd;
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
	return (id);
}

void add_client()
{
	struct sockaddr_in cli_fd;
	socklen_t len = sizeof(cli_fd);
	int fd = accept(sock_fd, (struct sockaddr *)&cli_fd, &len);
	if (fd < 0)
		fatal();
	int id = add_client_to_list(fd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d just arrived\n", id);
	send_all(fd, msg);
	FD_SET(fd, &cli_set);
}

int rm_client_from_list(int fd)
{
	t_client *b = cli;
	t_client *del = NULL;
	int id = 0;

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
	int id = rm_client_from_list(fd);
	bzero(&msg, sizeof(msg));
	sprintf(msg, "server: client %d just disconnected\n", id);
	send_all(fd, msg);
	FD_CLR(fd, &cli_set);
	close(fd);
}

void ex_msg(int fd, char *s)
{
	int i = 0;
	int j = 0;

	while (str[i])
	{
		tmp[j] = str[i];
		j++;
		if (str[i] == '\n')
		{
			sprintf(buf, "client %d: %s", get_id(fd), tmp);
			send_all(fd, buf);
			j = 0;
			bzero(&tmp, strlen(tmp));
			bzero(&str, strlen(str));
		}
		i++;
	}
}

int main(int ac, char **av) {
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		return (1);
	}

	// socket create and verification
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd == -1) {
		fatal();
	}
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));

	// Binding newly created socket to given IP and verification
	if ((bind(sock_fd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
		fatal();
	}
	if (listen(sock_fd, 10) != 0) {
		fatal();
	}

	FD_ZERO(&cli_set);
	FD_SET(sock_fd, &cli_set);
	bzero(&msg, sizeof(msg));
	bzero(&str, sizeof(str));
	bzero(&tmp, sizeof(tmp));
	bzero(&buf, sizeof(buf));

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
					// printf("Bonjour\n");
					add_client();
					break ;
				}
				else
				{
					if (recv(fd, str, sizeof(str), 0) <= 0)
					{
						// printf("deco\n");
						rm_client(fd);
						break ;
					// disconnect
					}
					else
					{
					// receive msg;
						ex_msg(fd, str);
					}
				}
			}
		}
	}

	/*socklen_t len = sizeof(test);
	int connfd = accept(sock_fd, (struct sockaddr *)&test, &len);
	if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");*/
		close(sock_fd);
		return (0);
}
