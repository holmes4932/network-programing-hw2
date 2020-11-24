#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include<arpa/inet.h>

#define BUFSIZE 8192
#define SERV_PORT 6667

char buf[BUFSIZE];
char msg[BUFSIZE];
char cmd[BUFSIZE];

void *thread_listen(void *args){
	int fd = *((int *)args);

	while(1){
		memset(buf, 0, sizeof(buf));
		read(fd, buf, BUFSIZE);

		if(strlen(buf) >= 1){
			if(strcmp(buf,">>")==0){
				printf("%s",buf);
				fflush(stdout);
			}
			else{
            	printf("******Server******\n%s", buf);
            	fflush(stdout);

			}
		}
		if(strncmp("end connect with server", buf, 23) == 0){
			break;
		}
	}

	pthread_exit(NULL);
}

void *thread_write(void *args){
	int fd = *((int *)args);

	while(1){
		fgets(cmd, BUFSIZE, stdin);

		write(fd,cmd,strlen(cmd));
		if(strncmp("quit", cmd, 4) == 0){
			break;
		}
	}

	pthread_exit(NULL);
}

int main(){
    int cliSocket;
	struct sockaddr_in svrAddr;
	socklen_t addr_size;

	cliSocket = socket(PF_INET, SOCK_STREAM, 0);
	svrAddr.sin_family = AF_INET;
	svrAddr.sin_port = htons(SERV_PORT);
	svrAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(svrAddr.sin_zero, 0, sizeof(svrAddr.sin_zero));

	addr_size = sizeof(svrAddr);
	connect(cliSocket, (struct sockaddr *) &svrAddr, addr_size);

	pthread_t tid_listen, tid_write;

	pthread_create(&tid_listen, NULL, thread_listen, &cliSocket);
	pthread_create(&tid_write, NULL, thread_write, &cliSocket);

	pthread_join(tid_listen, NULL);
	pthread_join(tid_write, NULL);

	close(cliSocket);

	return 0;
}
