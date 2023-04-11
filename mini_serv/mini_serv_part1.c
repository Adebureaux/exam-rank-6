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
    int sockfd;
	struct sockaddr_in servaddr;

	if ((maxfd = sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
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
}