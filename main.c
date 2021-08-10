#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;

int exit_fatal(char *s)
{
	printf("%s", s);
	exit(EXIT_FAILURE);
	return (EXIT_FAILURE);
}

int main(int ac, char **av)
{
	int sock, csock;
	int ret;
	SOCKADDR_IN sin;
	socklen_t sock_len;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (!sock || sock == -1)
		return (exit_fatal("Socket creation failed\n"));

	sin.sin_addr.s_addr = htonl(2130706433);
	sin.sin_family = AF_INET;
	if (av[1])
		sin.sin_port = htons(atoi(av[1]));
	else
		sin.sin_port = htons(8080);
	if (bind(sock, (SOCKADDR*)&sin, sizeof(sin)))
		return (exit_fatal("Error while binding socket to sockaddr_in\n"));

	if (listen(sock, 5))
		return (exit_fatal("Error while listening\n"));

	sock_len = sizeof(sin);

	if ((csock = accept(sock, (SOCKADDR*)&sin, &sock_len)) == -1)
		return (exit_fatal("Error while accept\n"));

	closesocket(sock);

	return (0);
}
