#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

typedef struct	s_client
{
	int	fd;
	int	id;
	struct s_client *next;
}				t_client;

t_client *clients = NULL;
int sock_fd, id_len = 0;
fd_set socks, cpy_read, cpy_write;
char msg[42];
char str[42*4096], tmp[42*4096], buffer[42*4096 + 42];

int	get_id(int fd)
{
	t_client	*b = clients;

	while (b)
	{
		if (b->fd == fd)
			return (b->id);
		b = b->next;
	}
	return (-1);
}

int	get_max_fd()
{
	int max = sock_fd;
	t_client *b = clients;

	while (b)
	{
		if (b->fd > max)
			max = b->fd;
		b = b->next;
	}
	return (max);
}

size_t ft_strlen(char *s)
{
	int i = 0;

	while (s[i])
		i++;
	return (i);
}

int exit_fatal()
{
	write(2, "Fatal error\n", ft_strlen("Fatal error\n"));
	close(sock_fd);
	exit(EXIT_FAILURE);
	return (EXIT_FAILURE);
}

int exit_fatal2(char *s)
{
	write(2, s, ft_strlen(s));
	close(sock_fd);
	exit(EXIT_FAILURE);
	return (EXIT_FAILURE);
}

void send_all(int fd, char *s)
{
	t_client *b = clients;

	//printf("%s\n", s);

	while (b)
	{
		// if fd is different and fd is ready for writing
		if (b->fd != fd && FD_ISSET(b->fd, &cpy_write))
		{
			if (send(b->fd, s, ft_strlen(s), 0) == -1)
				exit_fatal2("Error while sending message\n");
		}
		b = b->next;
	}
}

void add_client_to_list(int fd)
{
	t_client *b;
	t_client *b2 = clients;

	if (!(b = calloc(1, sizeof(t_client))))
		exit_fatal();
	b->id = id_len++;
	b->fd = fd;
	b->next = NULL;
	if (!clients)
		clients = b;
	else
	{
		while (b2->next)
			b2 = b2->next;
		b2->next = b;
	}
}

void add_client()
{
	SOCKADDR_IN csin;
	socklen_t sock_len;
	int csock;

	// accept connection
	if ((csock = accept(sock_fd, (SOCKADDR*)&csin, &sock_len)) == -1)
		exit_fatal2("Error while accepting client connection\n");
	// add client to list
	add_client_to_list(csock);
	// send message to all
	sprintf(msg, "server: client %d just arrived\n", get_id(csock));
	//printf("client %d, %d\n", get_id(csock), csock);
	send_all(csock, msg);
	// set fd set
	FD_SET(csock, &socks);
}

void delete_client(int fd)
{
	t_client *b = clients;
	t_client *del;
	int id = get_id(fd);

	if (b && b->fd == fd)
	{
		clients = b->next;
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
				free(del);
				break ;
			}
			b = b->next;
		}
	}
	sprintf(msg, "server: client %d just left\n", id);
	send_all(fd, msg);
	// return (id);
}

void get_msg(int fd)
{
	// printf("J'aime les messages\n");
	int i = 0;
	int j = 0;

	while (str[i])
	{
		tmp[j] = str[i];
		j++;
		if (str[i] == '\n')
		{
			sprintf(buffer, "client %d: %s", get_id(fd), tmp);
			send_all(fd, buffer);
			j = 0;
			bzero(&tmp, sizeof(str));
			bzero(&buffer, sizeof(buffer));
		}
		i++;
	}
	bzero(&str, sizeof(str));
}

void print_set()
{
	t_client *b = clients;

	while (b)
	{
		printf("client: %d, %d\n", b->id, b->fd);
		b = b->next;
	}
}

int main(int ac, char **av)
{
	int port;
	SOCKADDR_IN sin;

	if (ac != 2)
		return (exit_fatal2("Wrong number of arguments\n"));
	port = atoi(av[1]);

	// open server socket
	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		return (exit_fatal2("Error wile creating server socket\n"));
	sin.sin_addr.s_addr = htonl(2130706433);
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);

	if (bind(sock_fd, (const SOCKADDR*)&sin, sizeof(sin)))
	{
		printf("errno: %d\n", errno);
		return (exit_fatal2("Error while binding socket\n"));
	}
	if (listen(sock_fd, 10))
		return (exit_fatal2("Error while listening\n"));

	// set FD_SET and Buffers
	FD_ZERO(&socks);
	FD_SET(sock_fd, &socks);
	bzero(&buffer, sizeof(buffer));
	bzero(&tmp, sizeof(tmp));
	bzero(&str, sizeof(str));
	bzero(&msg, sizeof(msg));

	// motor
	while (1)
	{
		// initiate cpy set with socks
		cpy_read = cpy_write = socks;

		// select !
		//printf("Here1\n");
		if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
			continue ;
		//print_set();
		for (int fd = 0; fd < get_max_fd() + 1; fd++)
		{
			// check cpy_read where ready fd where writen by select if fd is ready
			// if not iterate fd++
			//printf("Here1\n");
			if (FD_ISSET(fd, &cpy_read))
			{
				// printf("Here2\n");
				// if fd == sock_fd it means that server socket received a connection
				// demamd so we add the client
				if (fd == sock_fd)
				{
					// printf("Here\n");
					bzero(&msg, ft_strlen(msg));
					add_client();
					break ;
				}
				else
				{
					// triger recv
					if (recv(fd, str, sizeof(str), 0) <= 0)
					{
						printf("Here\n");
						// if message reception <= like 0 char message -> means disconection
						bzero(&msg, ft_strlen(msg));
						// delete client from clients list + send message to all
						delete_client(fd);
						// clear fd set
						FD_CLR(fd, &socks);
						// close fd
						close(fd);
						break ;
						//sprintf(msg, "server: Client %d disconnected\n", get_id())
					}
					else
					{
						printf("bonjour\n");
						// we get a msg
						get_msg(fd);
					}
				}
			}
		}

	}

	return (0);
}
