#include "server_init.h"
#include "serve.h"
#include <unistd.h>
#include <pthread.h>

void* serviceTheRequest(void * serveParam);
struct server_data
{
	int clientfd;
	struct sockaddr_in client;	
	
};
char * filename;


int client_fd;
int main(int argc,char ** argv){ 	
	if(argc!=3){
		printf("%s","Invalid number of arguments");
		exit(0);	
	}	
	int port=atol(argv[1]);
	filename=argv[2];
	int daemon;
	/*child*/
	if((daemon=fork())==0){
		int sock=server_init(port);
		if(sock>=0){
			struct sockaddr_in client;		
			int number=0;		
			while(1){
				int length=sizeof(client);
				int client_fd=accept(sock,(struct sockaddr *)& client,&length);
				if(client_fd<0){
					perror("Couldn't accept connection\n");
					break;			
				}
				struct server_data p;
				p.clientfd=client_fd;
				p.client=client;
				/*initialize pthread*/				
				pthread_t tid; 
				pthread_attr_t attr; 
				pthread_attr_init(&attr);
				pthread_create(&tid, &attr, serviceTheRequest,(void *)&p);
				}	
			}
		else{
			fprintf(stderr,"%s\n","Server could not be initialized");	
		}
	}
	/*parent*/
	else if(daemon>0){
		/*orpahn the child*/		
		exit(0);	
	}
	return 0;
}

void * serviceTheRequest(void *serveParam){
	int status;
	/*Get the struct of data*/
	struct server_data* p=(struct server_data *)serveParam;
	struct sockaddr_in client=(*p).client;
	int clientfd=(*p).clientfd;
	/*Get ip address of client*/	
	char *ip=inet_ntoa(client.sin_addr);	
	char *loginfo=respond(&status,clientfd,ip);		
	/*Send to log file*/	
	sendTolog(status,loginfo,filename);
	free(loginfo);
	pthread_exit( (void *)status);		
}
