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

int exit_fatal(char *s)
{
	printf("%s", s);
	exit(EXIT_FAILURE);
	return (EXIT_FAILURE);
}

void create_message(char *b, char *s)
{
	int i = 0;
	int len = strlen(s);

	bzero(b, 256);
	while (i < len || i >= 255)
	{
		b[i] = s[i];
		i++;
	}
}

int main(int ac, char **av)
{
	int sock, csock;
	int ret;
	SOCKADDR_IN sin, csin;
	socklen_t sock_len, csock_len;
	char b[256];

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!sock || sock == -1)
		return (exit_fatal("Socket creation failed\n"));

	sin.sin_addr.s_addr = htonl(2130706433);
	sin.sin_family = AF_INET;
	if (av[1])
		sin.sin_port = htons(atoi(av[1]));
	else
		sin.sin_port = htons(8080);
	if (bind(sock, (SOCKADDR*)&sin, sizeof(sin)) != 0)
	{
		close(sock);
		return (exit_fatal("Error while binding socket to sockaddr_in\n"));
	}

	if (listen(sock, 5))
		return (exit_fatal("Error while listening\n"));

	sock_len = sizeof(sin);
	csock_len = sock_len;

	printf("Socket is set-up, waiting for client to connect\n");
	if ((csock = accept(sock, (SOCKADDR*)&csin, &csock_len)) == -1)
		return (exit_fatal("Error while accept\n"));
	printf("Received connection from client\n");

	create_message(b, "Salut le client");
	printf("message to send: %s\n", b);
	if ((send(csock, b, strlen(b), 0)) == -1)
		printf("Error while sending message: %s\n", b);

	while (1)
	{
		ret = 0;
		bzero(b, 256);
		if ((ret = recv(csock, b, 256, 0)) == -1)
			return (exit_fatal("Received failed\n"));
		if (ret > 0)
			printf("Received: %s\n", b);
		// exit(1);
			//return (exit_fatal("error while receiving message\n"));
	}

	shutdown(csock, 2);
	close(sock);

	return (0);
}
