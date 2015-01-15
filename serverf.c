#include "server_init.h"
#include "serve.h"
#include <unistd.h>

void serviceTheRequest(int clientfd,struct sockaddr_in client);
char * filename;

int main(int argc,char ** argv){
	if(argc!=3){
		printf("%s","Invalid number of arguments");
		exit(0);	
	}	
	int port=atol(argv[1]);
	filename=argv[2];
	int daoemon;
	if((daoemon=fork())==0){
		int sock=server_init(port);
		if(sock>=0){
			struct sockaddr_in client;		
			int number=0;		
			while(1){
				int length=sizeof(client);
				int accept_fd=accept(sock,(struct sockaddr *)& client,&length);
				if(accept_fd<0){
					perror("Couldn't accept connection\n");
					break;			
				}
				int pid=fork();
				/*child process*/
				if(pid==0){
					serviceTheRequest(accept_fd,client);
					exit(0);			
				}
				else if(pid<0){
					printf("%s\n","Fork failed");			
				}
				/*parent process*/			
				else{
					close(accept_fd);				
					continue;
				}
			}	
		}
		else{
			fprintf(stderr,"%s\n","Server could not be initialized");	
		}
	}
	/*parent process*/
	else if(daoemon>0){
		/*orphan the child*/		
		exit(0);
	}
	return 0;
}

/*Handle the Request From the Client*/
void serviceTheRequest(int clientfd,struct sockaddr_in client){
	int status;
	/*Get IP address*/
	char *ip=inet_ntoa(client.sin_addr);	
	char *loginfo=respond(&status,clientfd,ip);	
	/*Write to log file*/	
	sendTolog(status,loginfo,filename);
	free(loginfo);
	exit(status);		
}
