#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef MIN_TIMEOUT
# define MIN_TIMEOUT (200000)
#endif


#ifndef MAX_TIMEOUT
# define MAX_TIMEOUT (5 * MIN_TIMEOUT)
#endif

#ifndef COUNT
#  define COUNT (5)
#endif

#ifndef PORT
#  define PORT (1234)
#endif

#ifndef PROCESS_EXP
#  define PROCESS_EXP (8)
#endif

int main(int argc, char* argv[])
{
	int sd;
	struct sockaddr_in addr;
	int timeout, i, buflen;
	char buffer[1024];
	if ( argc != 2 ) {
		fprintf(stderr,"Usage: %s <ip>\n", argv[0]);
		return __LINE__;
	}

	for ( i = 0 ; i < PROCESS_EXP ; i++ ) {
		fork();
	}
	
	if ( (-1) == (sd = socket(AF_INET, SOCK_STREAM, 0))) {	
		perror("socket");
		return __LINE__;
	}
	
	addr.sin_family =  AF_INET;
	addr.sin_port = htons(PORT); 
	if( (-1) == (addr.sin_addr.s_addr = inet_addr(argv[1])) ) {
		fprintf(stderr,"Incorrent ip address\n");
		close(sd);
		return __LINE__;
	}

	if ( (-1) == connect(sd, (struct sockaddr*)&addr, sizeof(addr)) ) {
		perror("connect");
		close(sd);
		return __LINE__;
	}

	srand(getpid() * time(NULL));

	for ( i = 0 ; i < COUNT ; i++ ) {
		timeout = (rand() % (MAX_TIMEOUT - MIN_TIMEOUT))  + MIN_TIMEOUT;
		printf("Timeount %d\n", timeout);
		usleep(timeout);
		buflen = snprintf(buffer, 1024, "[%d] PING #%d\n", getpid(), i+1);
		write(sd, buffer, buflen);
	}
	
	close(sd);
	printf("Done\n");
	return 0;
}
