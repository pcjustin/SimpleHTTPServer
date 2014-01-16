/* vim: ts=4 sw=4
 */
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

int calculateHeaderSize(char *data)
{
	char *pch = NULL, *pHeader;
	int headerSize = 0;

	pch = strstr(data, "\r\n");
	while (1) {
		if (pch == NULL) break;
		else pHeader = pch;
		pch++;
		pch = strstr(pch, "\r\n");
	}
	headerSize = pHeader - data + 2;
	return headerSize;
}

void doprocessing (int sock)
{
	char *pData = NULL, *pTempData = NULL;
	char recvBuffer[1024];
	int recvSize = 0, dataSize = 0, DataLength = 0;

	memset(recvBuffer, '\0', sizeof(recvBuffer));
	while (1) {
		recvSize = recv(sock, recvBuffer, sizeof(recvBuffer), 0);
		if (recvSize > 0) {
			char sContentLength[16];
			char *pch = NULL;
			int headerSize = 0, contentLength = 0;

			memset(sContentLength, '\0', sizeof(sContentLength));
			pch = strstr(recvBuffer, "Content-Length:");
			/* Receive more data */
			if (pch == NULL) {
				pTempData = (char *)malloc(DataLength);
				if (pTempData == NULL) {
					perror("allocate memory to temp data buffer");
					break;
				}
				if (memcpy(pTempData, pData, DataLength) == NULL) {
					perror("copy data buffer to temp data buffer");
					break;
				}
				pData = (char *)realloc((char*)pData, DataLength + recvSize);
				if (memcpy(pData, pTempData, DataLength) == NULL) {
					perror("copy temp data buffer to data buffer");
					break;
				}
				if (memcpy(pData + DataLength, recvBuffer, recvSize) == NULL) {
					perror("copy receive buffer to data buffer");
					break;
				}
				if (pTempData != NULL) {
					free(pTempData);
					pTempData = NULL;
				}
				dataSize += recvSize;
				if (dataSize >= contentLength) {
					break;
				} else {
					/* Receive next data */
					continue;
				}
			}

			pData = (char *)malloc(recvSize);
			DataLength = recvSize;
			if (pData == NULL) {
				perror("allocate memory to data buffer");
				break;
			}
			if (memcpy(pData, recvBuffer, recvSize) == NULL) {
				perror("copy receive buffer to data buffer");
				break;
			}
			sscanf(pch, "%*s%[^\r]\r\n", sContentLength);
			if (sContentLength == NULL) {
				perror("Parsing HTTP header failed!");
				break;
			}
			contentLength = atoi(sContentLength);
			headerSize = calculateHeaderSize(recvBuffer);
			dataSize = recvSize - headerSize;
			if (dataSize >= contentLength) {
				break;
			}
		} else if (recvSize == 0) {
			fprintf(stderr, "Client is closing connection or the connection time out\n");
			close(sock);
			break;
		}
	}
	fprintf(stderr, "Data = \n%s\n", pData);
	
	if (pData != NULL) {
		free(pData);
		pData = NULL;
	}
	if (pTempData != NULL) {
		free(pTempData);
		pTempData = NULL;
	}
}

int setSocketTimeout(int sockfd)
{
	int ret=0;
	struct timeval timeout;
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;
	ret = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
			sizeof(timeout));

	return ret;
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
		ret = setSocketTimeout(newsockfd);
		if (ret < 0) {
			perror("setsockopt failed\n");
		}
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
