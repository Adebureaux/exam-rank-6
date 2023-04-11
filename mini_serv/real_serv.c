#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client
{
	int hasNl, id;
} client;
client clients[1024];
fd_set rset, wset, set = (const fd_set){0};
int sockfd, maxfd, id = 0, res = 0, newfd = 0;
char buffout[65536 + 42], buffin[65536], buff[65536];

void fatal()
{
	write(2, "Fatal error\n", 12);
	close(sockfd);
	exit(1);
}

void broadcast(int sender)
{
	for (int i = 0; i <= maxfd; i++)
		if (FD_ISSET(i, &wset) && i != sender)
			send(i, buffout, strlen(buffout), 0);
}

int main(int argc, char **argv)
{
	struct sockaddr_in servaddr = (const struct sockaddr_in){0};
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		return (1);
	}
	if ((maxfd = sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		fatal();
	FD_SET(sockfd, &set);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1]));
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0 || listen(sockfd, 10) != 0)
		fatal();
	while (1)
	{
		wset = rset = set;
		if (select(maxfd + 1, &rset, &wset, 0, 0) <= 0)
			continue;
		for (int fd = 0; fd <= maxfd; fd++)
		{
			if (FD_ISSET(fd, &rset) && fd == sockfd)
			{
				if ((newfd = accept(sockfd, 0, 0)) < 0)
					continue; // or fatal() dont know yet
				clients[newfd].hasNl = 0;
				clients[newfd].id = id++;
				maxfd = newfd > maxfd ? newfd : maxfd;
				FD_SET(newfd, &set);
				sprintf(buffout, "server: client %d just arrived\n", clients[newfd].id);
				broadcast(newfd);
				break;
			}
			else if (FD_ISSET(fd, &rset) && fd != sockfd)
			{
				if ((res = recv(fd, buffin, 65536, 0)) <= 0)
				{
					sprintf(buffout, "server: client %d just left\n", clients[fd].id);
					broadcast(fd);
					FD_CLR(fd, &set);
					close(fd);
					break;
				}
				else
				{
					for (int i = 0, j = 0; i < res; i++, j++)
					{
						buff[j] = buffin[i];
						buff[j + 1] = '\0';
						if (buff[j] == '\n')
						{
							if (clients[fd].hasNl)
								sprintf(buffout, "%s", buff);
							else
								sprintf(buffout, "client %d: %s", clients[fd].id, buff);
							clients[fd].hasNl = 0;
							broadcast(fd);
							j = -1;
						}
						else if (i == (res - 1))
						{
							if (clients[fd].hasNl)
								sprintf(buffout, "%s", buff);
							else
								sprintf(buffout, "client %d: %s", clients[fd].id, buff);
							clients[fd].hasNl = 1;
							broadcast(fd);
							break;
						}
					}
				}
			}
		}
	}
}