#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

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

void *read_loop(void *sock_fd)
{
	printf("yo\n");
	char sb[256];
	char rb[1];
	bzero(&sb, sizeof(sb));
	bzero(&rb, sizeof(rb));
	int *fd = (int*)sock_fd;
	while (1)
	{

	}
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
	char rb[1];
	bzero(&sb, sizeof(sb));
	bzero(&rb, sizeof(rb));
	pthread_t thread;


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
	int sock_b = sock;

	//pthread_create(&thread, NULL, read_loop, (void*)&sock);

	while (1)
	{
		bzero(&b, sizeof(b));
		ret = recv(sock, b, 256, 0);
			// return (exit_fatal("Failed to Received message\n"));
		/*if (ret == 0)
		{
			//pthread_cancel(thread);
			close(sock);
			printf("Server offline\n");
			return (0);
		}*/
		printf("Received message: %s----\n", b);

		i = 0;
		while (read(0, rb, 1) > 0)
		{
			sb[i] = rb[0];
			i++;
			if (rb[0] == '\n')
				break ;
		}
		sb[i] = 0;
		printf("message: %s\n", sb);
		if (send(sock, sb, strlen(sb), 0) < 0)
		{
			printf("Error while sending\n");
		}
		//shutdown(*fd, 1);
	}

	/*while (strcmp("exit", b))
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
		b[i] = '\n';
		if ((send(sock, b, 256, 0)) == -1)
			exit_fatal("Error while sending a message\n");
		printf("sent: %s", b);
		// shutdown(sock, 1);
		//send
	}*/

	close(sock);
	return (0);
}
