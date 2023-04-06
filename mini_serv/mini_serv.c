#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client {
    int new, id;
} client;
client clients[1024];
fd_set set, rset, wset = (const fd_set){0};
int newfd, maxfd, sockfd, id = 0;
char buffer[64], c;

void exit_fatal() {
    write(2, "Fatal error\n", 12);
    exit(1);
}

void broadcast(char *buff, int len, int sender) {
    for (int i = 0; i <= maxfd; i++)
        if (FD_ISSET(i, &wset) && i != sender)
            send(i, buff, len, 0);
}

int main(int ac, char **av) {
    if (ac < 2) {
        write(2, "Wrong number of arguments\n", 27);
		return (1);
    }
	struct sockaddr_in servaddr; 

	// socket create and verification 
	if ((maxfd = sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        exit_fatal();
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(av[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
        exit_fatal();
	if (listen(sockfd, 10) != 0)
        exit_fatal();
    FD_SET(sockfd, &set);

    while (1) {
        rset = wset = set;
        if (select(maxfd + 1, &rset, &wset, 0, 0) < 0)
            continue;
        for (int fd = 0; i <= maxfd; fd++) {
            if (FD_ISSET(fd, &rset) && fd == sockfd) {
                if ((newfd = accept(sockfd, 0, 0)) < 0)
                    continue;
                clients[newfd].id = id++;
                clients[newfd].new = 1;
                if (newfd > maxfd)
                    maxfd = newfd
                FD_SET(newfd, &set);
                sprintf(buff, "server: client %d just arrived\n", clients[newfd].id);
                broadcast(buff, strlen(buff), newfd);
                break;
            }
            else if (FD_ISSET(fd, &rset) && fd != sockfd) {
                if (recv(fd, &c, 1, 0) <= 0) {
                    sprintf(buff, "server: client %d just left\n", clients[fd].id);
                    broadcast(buff, strlen(buff), fd);
                    FD_CLR(fd, &set);
                    close(fd);
                    break;
                }
                else {
                    if (client[fd].new) {
                        sprintf(buff, "client %d: ", clients[fd].id);
                        broadcast(buff, strlen(buff), fd);
                        clients[fd].new = 0;
                    }
                    if (c == '\n')
                        clients[fd].new = 1;
                    broadcast(&c, 1, fd);
                    break;
                }
            }
        }
    }
    return 0;
}
