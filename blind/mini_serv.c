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
	int	fd;
	int	id;
	struct s_client *next;
}								t_client;

t_client *clients = NULL;
int sock_fd, g_id;

fd_set cpy_read, cpy_write, cli_set;
char msg[42];
char str[42*4096], tmp[42*4096], buf[42*4096 + 42];


void fatal()
{
	write(2, "Fatal error\n", 12);
	close(sock_fd);
	exit(1);
}

int get_id(fd)
{
	t_client *b_clients = clients;

	while (b_clients)
	{
		if (b_clients->fd == fd)
			return (b_clients->id);
		b_clients = b_clients->next;
	}
	return (-1);
}

int get_max_fd()
{
	t_client *b_clients = clients;
	int max_fd = sock_fd;

	while (b_clients)
	{
		if (b_clients->fd >= max_fd)
			max_fd = b_clients->fd;
		b_clients = b_clients->next;
	}
	return (max_fd);
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
	t_client *b = clients;

	while (b)
	{
		// we check if fd != and
		// IF FD is READY for writing
		if (b->fd != fd && FD_ISSET(b->fd, &cpy_write))
		{
			// we send message
			// strlen here for not sending whole buffer i guess
			if (send(b->fd, s, strlen(s), 0) <= 0)
				fatal();
		}
		b = b->next;
	}
}

int add_client_to_list(fd)
{
	// add client_to list
	t_client *b_clients = clients;
	t_client *new_client = NULL;
	if (!(new_client = malloc(sizeof(t_client))))
		fatal();

	new_client->fd = fd;
	g_id++;
	new_client->id = g_id;
	new_client->next = NULL;

	if (!b_clients)
		clients = new_client;
	else
	{
		while (b_clients->next)
			b_clients = b_clients->next;
		b_clients->next = new_client;
	}
	return (new_client->id);
}

void add_client()
{
	// add client does -> accept
	// -> add client to list
	// -> send msg to all
	// -> add client to set
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int id = 0;
	int fd = accept(sock_fd, (struct sockaddr *)&cli, &len);
	if (fd < 0) {
				printf("server acccept failed...\n");
				fatal();
	}
	id = add_client_to_list(fd);
	sprintf(msg, "server: client %d just arrived\n", id);
	printf("%s", msg);
	send_all(fd, msg);
	FD_SET(fd, &cli_set);
}

int rm_client_from_list(fd)
{
	int id = 0;
	t_client *b = clients;
	t_client *del = NULL;

	if (b && b->fd == fd)
	{
		clients = b->next;
		id = b->id;
		free (b);
	}
	else
	{
		while (b && b->next)
		{
			if (fd == b->next->fd)
			{
				id = b->next->id;
				del = b->next;
				b->next = del->next;
				free(del);
				break ;
			}
			b = b->next;
		}
	}
	return (id);
}

void rm_client(fd)
{
	// we get id,
	// we send message to all
	// we detelete client from list
	// we delete client from set
	// we close fd for leaks
	int id = rm_client_from_list(fd);
	sprintf(msg, "server: client %d just disconnected\n", id);
	send_all(fd, msg);
	rm_client_from_list(fd);
	FD_CLR(fd, &cli_set);
	close(fd);
}

void ex_msg(int fd)
{
	int i = 0;
	int j = 0;

	while (str[i])
	{
		// we use tmp a pre buffer to write until \n in buf
		tmp[j] = str[i];
		j++;
		if (str[i] == '\n')
		{
			sprintf(buf, "client %d: %s", get_id(fd), tmp);
			send_all(fd, buf);
			// restart tmp
			j = 0;
			// bzero buf and tmp
			bzero(&tmp, strlen(tmp));
			bzero(&buf, strlen(buf));
		}
		i++;
	}
	// don't forget to bero str as well lol
	bzero(&str, strlen(str));
}

int main(int ac, char **av) {
	struct sockaddr_in servaddr, cli;
	int port = 0;
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
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
		printf("fatal in bind\n");
		fatal();
	}
	if (listen(sock_fd, 10) != 0) {
		fatal();
	}

	FD_ZERO(&cli_set);
	FD_SET(sock_fd, &cli_set);
	bzero(&tmp, sizeof(tmp));
	bzero(&str, sizeof(str));
	bzero(&msg, sizeof(msg));
	bzero(&buf, sizeof(buf));

	while (1)
	{
		cpy_write = cpy_read = cli_set;
		// select, if < 0 means no fd is ready
		// select will copy all ready fd in cpy_read
		if (select(get_max_fd() + 1, &cpy_read, &cpy_write, NULL, NULL) < 0)
			continue ;
		// iterate on all fds
		for (int fd = 0; fd < get_max_fd() + 1; fd++)
		{
			// if iterated fd is a member of cpy_read
			if (FD_ISSET(fd, &cpy_read))
			{
				// if sock_fd == fd -> means client is trying to connect
				// else means client sent a message or disconnected
				if (fd == sock_fd)
				{
					// we add_client
					// we break the loop
					printf("client tried to connect\n");
					bzero(&msg, sizeof(msg));
					add_client();
					break ;
				}
				else
				{
					// we recv with str as buffer
					if (recv(fd, str, sizeof(str), 0) <= 0)
					{
						// client disconnected
						// we rm client, message if sent there
						// we clear fd from the set
						// we close fd and we break
						printf("client disconnected\n");
						bzero(&msg, sizeof(msg));
						rm_client(fd);
						break ;
					}
					// else means message received is longer than 0 char -> not a disconnect
					else
					{
						printf("message received\n");
						ex_msg(fd);
					}
				}
			}
		}
	}


	// while (read(0, str, 1));

	/*socklen_t len = sizeof(cli);
	int connfd = accept(sock_fd, (struct sockaddr *)&cli, &len);
	if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");
	*/
	close(sock_fd);
	return (0);
}
