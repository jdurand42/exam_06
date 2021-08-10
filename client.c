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
	int ret = -1;
	SOCKADDR_IN sin;
	socklen_t sock_len;

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

	/*if ((csock = accept(sock, (SOCKADDR*)&sin, &sock_len)) == -1)
		return (exit_fatal("Error while accept\n"));
	*/
	close(sock);

	return (0);
}
