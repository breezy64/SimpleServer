#include "server_init.h"

int server_init(int myport){
	struct sockaddr_in server;		
	server.sin_family=AF_INET;
	server.sin_port=htons(myport);
	server.sin_addr.s_addr = INADDR_ANY;
	
	int sock=socket(AF_INET,SOCK_STREAM,0);	
	if(sock<0){
		fprintf(stderr,"%s\n","couldn't create socket");
		return -1;	
	}
	int bind_status=bind(sock,(struct sockaddr*)&server,sizeof(server));
	if(bind_status<0){
		fprintf(stderr,"%s\n","Couldn't bind socket to address");
		return -1;
	}		
	int listen_status=listen(sock,5);
	if(listen_status<0){
		fprintf(stderr,"%s\n","Unable to mark socket as passive");
		return -1;
	}
	return sock;
}
