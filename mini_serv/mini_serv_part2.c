#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client {
    int nl, id;
} client;
client clients[1024];
fd_set rset, wset, set = (const fd_set){0};
char buffin[66000], buffout[66000], buffer[66000];
int sockfd, maxfd, newfd, res, id = 0;

void exit_fatal() {
	write(2, "Fatal error\n", 12);
    close(sockfd);
    exit(1);
}

void broadcast(int sender) {
    for (int i = 0; i <= maxfd; i++)
		if (FD_ISSET(i, &wset) && i != sender)
			send(i, buffout, strlen(buffout), 0);
}

int main(int ac, char **av) {
    if (ac < 2) {
        write(2, "Wrong number of arguments\n", 26);
        return 1;
    }
	struct sockaddr_in servaddr;

	if ((maxfd = sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        exit_fatal();
	bzero(&servaddr, sizeof(servaddr));
	FD_SET(sockfd, &set);

	// assign IP, PORT 
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1]));
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
        exit_fatal();
	if (listen(sockfd, 10) != 0)
        exit_fatal();

    while (1) {
        wset = rset = set;
        if (select(maxfd + 1, &rset, &wset, 0, 0) <= 0)
            continue;
        for (int fd = 0; fd <= maxfd; fd++) {
            if (FD_ISSET(fd, &rset) && fd == sockfd) {
                if ((newfd = accept(sockfd, 0, 0)) < 0)
                    continue;
                clients[newfd].nl = 0;
                clients[newfd].id = id++;
                maxfd = newfd > maxfd ? newfd : maxfd;
                FD_SET(newfd, &set);
                sprintf(buffout, "server: client %d just arrived\n", clients[newfd].id);
                broadcast(newfd);
                break;
            }
            else if (FD_ISSET(fd, &rset) && fd != sockfd) {
                if ((res = recv(fd, buffin, 66000, 0)) <= 0) {
                    sprintf(buffout, "server: client %d just left\n", clients[fd].id);
                    broadcast(fd);
                    FD_CLR(fd, &set);
                    close(fd);
                    break;
                }
                else {
                    for (int i = 0, j = 0; i < res; i++, j++) {
                        buffer[j] = buffin[i];
                        buffer[j + 1] = '\0';
                        if (buffer[j] == '\n') {
                            if (clients[fd].nl)
                                sprintf(buffout, "%s", buffer);
                            else
                                sprintf(buffout, "client %d: %s", clients[fd].id, buffer);
                            clients[fd].nl = 0;
                            broadcast(fd);
                            j = -1;
                        }
                        else if (i == (res - 1)) {
                            if (clients[fd].nl)
                                sprintf(buffout, "%s", buffer);
                            else
                              sprintf(buffout, "client %d: %s", clients[fd].id, buffer);
                            clients[fd].nl = 1;
                            broadcast(fd);
                            break;
                        }
                    }
                }
            }
        }
    }
}