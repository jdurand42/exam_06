#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

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

int exit_fatal(char *s)
{
	printf("%s", s);
	exit(EXIT_FAILURE);
	return (EXIT_FAILURE);
}

int main(int ac, char **av)
{
	int sock, csock;
	int ret = -1;
	int i = 0;
	SOCKADDR_IN sin;
	socklen_t sock_len;
	char b[256];
	char sb[256];

	while (ret)
	{
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (!sock || sock == -1)
			return (exit_fatal("Socket creation failed\n"));

		sin.sin_addr.s_addr = htonl(2130706433);
		sin.sin_family = AF_INET;
		if (av[1])
			sin.sin_port = htons(atoi(av[1]));
		else
			sin.sin_port = htons(8080);

		sock_len = sizeof(sin);
		ret = connect(sock, (SOCKADDR*)&sin, sizeof(sin));
		if (ret)
		{
			printf("Client could not connect to 127.0.0.1:8080\n");
			printf("Retrying in 5sec\n");
			close(sock);
			sleep(1);
		}
	}
	printf("Client connected to %s:%d\n", "127.0.0.1", 8080);

	if (recv(sock, b, 256, 0) == -1)
		return (exit_fatal("Failed to Received message\n"));
	printf("Received message: %s\n", b);

	while (strcmp("exit", b))
	{
		bzero(b, 256);
		i = 0;
		while (read(0, sb, 1))
		{
			if (sb[0] != '\n' && i < 255)
			{
				b[i] = sb[0];
				i++;
			}
			else
				break ;
		}
		if ((send(sock, b, 256, 0)) == -1)
			exit_fatal("Error while sending a message\n");
		printf("sent: %s\n", b);
		// shutdown(sock, 1);
		//send
	}

	/*if ((csock = accept(sock, (SOCKADDR*)&sin, &sock_len)) == -1)
		return (exit_fatal("Error while accept\n"));
	*/
	close(sock);

	return (0);
}
