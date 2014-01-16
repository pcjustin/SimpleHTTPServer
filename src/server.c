#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
/* network */
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define HTTP_PORT 8080

void sigHandler(int signo)
{
	if (signo == SIGCHLD) {
		wait3(NULL, WNOHANG, NULL);
	} else if (signo == SIGTERM) {
	}
}

void doprocessing (int sock)
{
	char recvBuffer[1024];
	int recvSize = 0;
	while (1) {
		recvSize = recv(sock, recvBuffer, sizeof(recvBuffer), 0);

		if (recvSize > 0) {
			char ContentLength[16];
			char *pch = strstr(recvBuffer, "Content-Length:");
			if (pch == NULL) {
				perror("Receive completed");
				break;
			}
			sscanf(pch, "%*s%[^\r]", ContentLength);
			printf("Content-Length: %s\n", ContentLength);
		} else if (recvSize == 0) {
			perror("closing connection");
			close(sock);
			break;
		}
	}
}

int main(int argc, char *argv)
{
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd, newsockfd, clilen;
	int ret=0;

	signal(SIGCHLD, &sigHandler);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	memset((char *) &serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(HTTP_PORT);
	
	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on binding");
		exit(1);
	}
	ret = listen(sockfd,5);
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on listening");
		exit(1);
	}
	clilen = sizeof(cli_addr);

	pid_t pid;
	while (1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			close(sockfd);
			perror("ERROR on accept");
			exit(1);
		}
		pid = fork();
		if (pid < 0) {
			close(newsockfd);
			close(sockfd);
			perror("ERROR on fork");
			exit(1);
		} else if (pid == 0) {
			close(sockfd);
			doprocessing(newsockfd);
			exit(0);
		} else {
			close(newsockfd);
		}
	}

	return 0;
}
