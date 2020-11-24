#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<pthread.h>
#include <arpa/inet.h>

#define BUFSIZE 8192
#define SERV_PORT 6667
#define LISTENQ 5
#define MAX_USER 10
#define MAX_LEN 100

struct data{
    int fd;
    int playwith;
	int flag;
    char name[MAX_LEN];
    char board[20];
	pthread_t tid;
}user[MAX_USER];

int user_cnt=0;
int listenfd;

void data_init(){
    for(int i=0;i<MAX_USER;i++){
        user[i].fd=-1;
        user[i].playwith=-1;
        strcpy(user[i].name,"\0");
        strcpy(user[i].board,"         ");
    }
}

long find_empty_fd(){
    for(long i=0;i<MAX_USER;i++){
        if(user[i].fd==-1){
            return i;
        }
    }
    return -1;
}

void draw_board(char *board,int fd){
    char buf[BUFSIZE],tmp[100];
    memset(buf,0,sizeof(buf));

    strcat(buf,"============================\n");
    strcat(buf,"   |   |   \t   |   |   \n");
    sprintf(tmp," %c | %c | %c \t 1 | 2 | 3 \n",board[0],board[1],board[2]);
    strcat(buf,tmp);
    strcat(buf,"---+---+---\t---+---+---\n");
    sprintf(tmp," %c | %c | %c \t 4 | 5 | 6 \n",board[3],board[4],board[5]);
    strcat(buf,tmp);
    strcat(buf,"---+---+---\t---+---+---\n");
    sprintf(tmp," %c | %c | %c \t 7 | 8 | 9 \n",board[6],board[7],board[8]);
    strcat(buf,tmp);
    strcat(buf,"   |   |   \t   |   |   \n");

    write(fd,buf,strlen(buf));
}

int win(char *board){

    if(board[0]==board[1]&&board[1]==board[2]&&board[0]!=' ')return 1;
    if(board[3]==board[4]&&board[4]==board[5]&&board[3]!=' ')return 1;
    if(board[6]==board[7]&&board[7]==board[8]&&board[6]!=' ')return 1;

    if(board[0]==board[3]&&board[3]==board[6]&&board[0]!=' ')return 1;
    if(board[1]==board[4]&&board[4]==board[7]&&board[1]!=' ')return 1;
    if(board[2]==board[5]&&board[5]==board[8]&&board[2]!=' ')return 1;

    if(board[0]==board[4]&&board[4]==board[8]&&board[0]!=' ')return 1;
    if(board[2]==board[4]&&board[4]==board[6]&&board[2]!=' ')return 1;

    return 0;
}

void play(int atk,int def){

    char buf[BUFSIZE];

    user[atk].playwith=def;
    user[def].playwith=atk;
    strcpy(user[atk].board,"         ");
    strcpy(user[def].board,"         ");
    for(int i=0;i<9;i++){
        if(i%2==0){
            sprintf(buf, "It's your turn\n");
            write(user[atk].fd, buf, strlen(buf));
			sprintf(buf, "Waiting for enemy...\n");
			write(user[def].fd, buf, strlen(buf));
            while(1){
                draw_board(user[atk].board, user[atk].fd);
                sprintf(buf,"choose a number (1~9)\nOOOOOOOOOO\n>>");
				write(user[atk].fd,buf,strlen(buf));
                recv(user[atk].fd, buf, BUFSIZE, 0);
                int num=atoi(buf)-1;
                if(num>=0&&num<=8&&user[atk].board[num]==' '){
                    user[atk].board[num]='O';
                    user[def].board[num]='O';
                    draw_board(user[atk].board, user[atk].fd);
                    break;
                }
                else{
                    sprintf(buf, "Input is not in rule\n");
                    write(user[atk].fd, buf, strlen(buf));
                }
            }
            if(win(user[atk].board)==1){
                sprintf(buf, "You win\n");
                write(user[atk].fd, buf, strlen(buf));
                draw_board(user[def].board, user[def].fd);
                sprintf(buf, "You lose\n");
                write(user[def].fd, buf, strlen(buf));
    			user[atk].playwith=-1;
    			user[def].playwith=-1;
                return;
            }
        }
        else{
            sprintf(buf, "It's your turn\n");
            write(user[def].fd, buf, strlen(buf));
			sprintf(buf, "Waiting for enemy...\n");
            write(user[atk].fd, buf, strlen(buf));
            while(1){
                draw_board(user[def].board, user[def].fd);
                sprintf(buf,"choose a number (1~9)\nXXXXXXXXXX\n>>");
				write(user[def].fd,buf,strlen(buf));
                recv(user[def].fd, buf, BUFSIZE, 0);
                int num=atoi(buf)-1;
                if(num>=0&&num<=8&&user[atk].board[num]==' '){
                    user[atk].board[num]='X';
                    user[def].board[num]='X';
                    draw_board(user[def].board, user[def].fd);
                    break;
                }
                else{
                    sprintf(buf, "Input is not in rule\n");
                    write(user[def].fd, buf, strlen(buf));
                }
            }
            if(win(user[def].board)==1){
                sprintf(buf, "You win\n");
                write(user[def].fd, buf, strlen(buf));
                draw_board(user[atk].board, user[atk].fd);
                sprintf(buf, "You lose\n");
                write(user[atk].fd, buf, strlen(buf));
    			user[atk].playwith=-1;
    			user[def].playwith=-1;
                return;
            }
        }
    }
    sprintf(buf, "Draw!!\n");
    write(user[atk].fd, buf, strlen(buf));
    write(user[def].fd, buf, strlen(buf));
    user[atk].playwith=-1;
    user[def].playwith=-1;
}

void print_usage(int fd){
    char buf[BUFSIZE];
    memset(buf,0,sizeof(buf));

    strcat(buf,"Usage:\n");
    strcat(buf,"\t$list\n");
    strcat(buf,"\t\tlist all online user\n");
    strcat(buf,"\t$playwith [-name]\n");
    strcat(buf,"\t\tinvite a person to play with you\n");
	strcat(buf,"\t$help\n");
	strcat(buf,"\t\tprint usage\n");
    strcat(buf,"\t$quit\n");  
	strcat(buf,"\t\tend connect\n");

    write(fd,buf,strlen(buf));

}

void thread(int n){
    char buf[BUFSIZE];

    sprintf(buf,"Please Enter Your Name \n>>");
    write(user[n].fd, buf, strlen(buf));
    recv(user[n].fd, buf, BUFSIZE, 0);
   	char *tmp=strtok(buf," \n\r");
    strcpy(user[n].name,tmp);
    print_usage(user[n].fd);

    while(1){
        sprintf(buf,">>");
		write(user[n].fd,buf,strlen(buf));
        recv(user[n].fd, buf, BUFSIZE, 0);

        char *token=strtok(buf," \n\r");

        if(strcmp("list",token)==0){
            memset(buf,0,sizeof(buf));
            strcat(buf,"online user:\n");
            for(int i=0;i<MAX_USER;i++){
                if(user[i].fd!=-1 && i!=n){
                    strcat(buf,"\t");
                    strcat(buf,user[i].name);
                    strcat(buf,"\n");
                }
            }
			strcat(buf,"\n");
			write(user[n].fd,buf,strlen(buf));
        }
        else if(strcmp("quit",token)==0){
            sprintf(buf,"end connect with server\n\n");
            write(user[n].fd,buf,strlen(buf));
            break;
        }
		else if(strcmp("help",token)==0){
			print_usage(user[n].fd);
		}
		else if(strcmp("y",token)==0){
			user[n].flag=1;
			while(user[n].playwith!=-1){
				sleep(1);
			}
		}
		else if(strcmp("n",token)==0){
			user[n].flag=2;
			user[n].playwith=-1;
		}
        else if(strcmp("playwith",token)==0){
            char *token=strtok(NULL," \n\r");
            if(token==NULL){
                sprintf(buf,"input error:\n$playwith [-name]\n\n");
                write(user[n].fd,buf,strlen(buf));
                continue;
            }
			char name[MAX_LEN];
			strcpy(name,token);
			int tar=-1;
            for(int i=0;i<MAX_USER;i++){
                if(user[i].fd!=-1&&user[i].playwith==-1&&strcmp(name,user[i].name)==0){
                    tar=i;
                }
            }
            if(tar==-1){
                sprintf(buf,"Cannot find %s\n\n",name);
                write(user[n].fd,buf,strlen(buf));
            }
            else{
                sprintf(buf,"Waiting %s response\n",name);
                write(user[n].fd,buf,strlen(buf));

				user[tar].flag=0;
				user[tar].playwith=n;

                sprintf(buf,"\n%s invite you to play(y/n)\n>>",user[n].name);
                write(user[tar].fd,buf,strlen(buf));
				
				while(user[tar].flag==0){
					sleep(1);
				}

                if(user[tar].flag==1){
                    sprintf(buf,"%s accepted your invite\n",name);
                    write(user[n].fd,buf,strlen(buf));
                    play(n,tar);
                }
                else{
                    sprintf(buf,"%s refused your invite\n",name);                  
					write(user[n].fd,buf,strlen(buf));
                }
            }
        }
    }

    close(user[n].fd);
    user[n].fd=-1;
    user_cnt--;
	pthread_exit(NULL);
}

int main(){
   
	struct sockaddr_in serv_addr,cli_addr;
	socklen_t length;

	listenfd=socket(AF_INET, SOCK_STREAM, 0);
	if(listenfd<0) {
		perror("Socket created failed.\n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		perror("Bind failed:");
		return -1;
	}

	printf("listening...\n");
	listen(listenfd, LISTENQ);

	printf("initialize...\n");
    data_init();
	pthread_t tid[MAX_USER];

    while(1){
		length=sizeof(cli_addr);

		long i=find_empty_fd();
		if(i==-1){
            printf("Server is full\n");
            return -1;
		}

		user[i].fd=accept(listenfd, (struct sockaddr*)&cli_addr, &length);
		user_cnt++;
		if(pthread_create(&tid[i], NULL, (void*)(&thread), (void*)i)!=0){
			perror("pthread create failed:");
		}
	}

	return 0;

}
