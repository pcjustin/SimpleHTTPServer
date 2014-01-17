#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* network */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define IP_ADDRESS "127.0.0.1"
#define HTTP_PORT 8080
#define PACKET 1000
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
	serv_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	serv_addr.sin_port = htons(HTTP_PORT);

	ret = connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr));
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on connecting");
		exit(1);
	}
	
	char dataBuffer[PACKET];
	memset(dataBuffer, '\0', sizeof(dataBuffer));
	memset(dataBuffer, 'a', sizeof(dataBuffer));
	dataBuffer[PACKET-1] = '\0';
	dataBuffer[PACKET-2] = '*';
	dataBuffer[PACKET-3] = 'X';
	long dataBufferLen = strlen(dataBuffer);
	#define PACKET_HEADER "POST /HNAP1/ HTTP/1.1\r\n" \
	"Host: %s\r\n" \
	"Content-Length: %ld\r\n" \
	"Content-Type: text/xml\r\n" \
	"%s"

	char sendBuffer[PACKET+200];
	sprintf(sendBuffer, PACKET_HEADER, IP_ADDRESS, dataBufferLen, dataBuffer);
	int sendBufferLen = strlen(sendBuffer);
	fprintf(stdout, "sendBufferLen = %d\ndata =\n%s\n", sendBufferLen, sendBuffer);
	ret = send(sockfd, sendBuffer, sendBufferLen, MSG_CONFIRM );
	if (ret < 0) {
		close(sockfd);
		perror("ERROR on sending");
		exit(1);
	}
	
	close(sockfd);

	return 0;
}
