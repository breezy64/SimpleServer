#include "serve.h"
#include<time.h>
#include<string.h>
#include <sys/types.h>
#include <regex.h>
#include<stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>

char *receiveFromClient(int clientfd);
char * parseRequest(char *httpRequest);
char * getInput(FILE * clientStream);
char *generateResponse(int status, char * fileContent);
void sendToClient(int fd,char *message);
char *generatelogText(char *Date,char * client_ip,char *httpRequest,int status);
int searchPattern(char *string, char * searchPattern,regmatch_t * match,int flag);
char *substringRegex(char *src, char *dest,regmatch_t *match);
void responseText(int status,char **requestResponse,char **fileContent);
char * getDate();

char *file=NULL;



void sendTolog(int status_code,char * responseInfo, char *logfilename){
	FILE *logfile=fopen(logfilename,"a");
	if(logfile==NULL){
		perror("Unable to access log file");
	}
	else{
		fprintf(logfile,"%s\n",responseInfo);
	}
	fclose(logfile);
}

char* respond(int * status,int clientfd,char *ip_addr){
	char *fileContent=NULL;
	char * httpRequest;
	httpRequest=receiveFromClient(clientfd);
	file=parseRequest(httpRequest);
	if(file==NULL){
			*status=400;		
	}
	else if(strlen(file)==0){
		*status=500;
	}
	else{
		FILE * requestFile=fopen(file+2,"r");
		if(requestFile==NULL){
			if(errno==EINVAL){
				*status=403;				
			}
			else if(errno==ENOENT){
				*status=404;
			}
			else{
				*status=500;				
			}			
		}
		else{
			*status=200;
			fileContent=getInput(requestFile);
			fclose(requestFile);
		}	
	}
	char *response=generateResponse(*status, fileContent);
	sendToClient(clientfd,response);
	close(clientfd);
	free(response);
	char *log=generatelogText(getDate(),ip_addr,httpRequest,*status);
	free(httpRequest);	
	return log;
}

char *receiveFromClient(int clientfd){
	/*Assume a header is less than a kilobyte*/	
	int buffer_size=1024;
	char* out=(char *)malloc(buffer_size);	
	recv(clientfd,out,buffer_size-1,0);
	return out;
}
/*See if the HTTP request is valid.If so, get the filename*/
char* parseRequest(char * httpRequest){
	char *request=(char *)malloc(strlen(httpRequest)+1);	
	regmatch_t * match=(regmatch_t *)malloc(sizeof(regmatch_t));	
	/*See if the request is an HTTP/1.1 GET Request which ends in a description of
	of a User-Agent(ie. check the header)	
	*/	
	if(searchPattern(httpRequest,"GET.*HTTP\/1.1",match,REG_NOSUB)){
		/*Make sure it ends in a blank line*/		
		if(searchPattern(httpRequest,".*[r\n][$\r\n]",match,REG_NEWLINE|REG_NOSUB)<=0){
			request=NULL;
		}
		else{		
			/*Get the file name*/			
			if(searchPattern(httpRequest,"[^a-zA-Z0-9]\/[a-zA-Z0-9\/\.\-_]+",match,REG_EXTENDED)){
				request=substringRegex(httpRequest,request,match);
			}
			else{
				request="";
			}
			realloc(request,strlen(request)+1);
		}	
	}
	else{
		request=NULL;	
	}
	return request; 
}
/*Get data from a stream. Probably wiser to use scanf*/
char * getInput(FILE * clientStream){
	char * line=NULL;
	size_t outlen=sizeof(char[100]);
	size_t length;	
	char *output=malloc(outlen);
	output[0]='\0';
	while (getline(&line,&length,clientStream)!=-1){			
			/*increase the size of the output string
			if it cannot accomodate the size of the current
			line*/
			if(outlen<length+strlen(output)+1){			
				output=(char *)realloc(output,strlen(output)+1+length);
			}
			strcat(output,line);
	}
	free(line);
	return output;
}
char *generateResponse(int status, char * fileContent){
	char *requestResponse;
	responseText(status,&requestResponse,&fileContent);
    char *date=getDate();
	char *out=(char *)malloc(strlen(fileContent)+strlen(requestResponse)+strlen(date)+100);
	sprintf(out,"HTTP/1.1 %d %s\nDate: %s\nContent-Type: text/html\nContent-Length: %d\n\n%s",status,requestResponse,date,strlen(fileContent)+1,fileContent);
	realloc(out,strlen(out)+1);
	return out;
}
/*Send response to client*/
void sendToClient(int fd,char *message){
	 send(fd, message, strlen(message)+1,0);
}
/*Create a string to be later written to output*/
char *generatelogText(char *Date,char *client_ip,char *httpRequest,int status){
	char *response;
	responseText(status,&response,NULL);
	/*Grab the first line of the reponse*/	
	char *httpHeader=strtok(httpRequest,"\n");
	/*Make sure httpHeader is not NULL*/
	httpHeader=httpHeader==NULL?httpRequest:httpHeader;
	/*Allocate some space on the heap*/	
	char *logOutput=(char *)malloc(strlen(Date)+1+strlen(httpHeader)+1+strlen(response)+1+strlen(client_ip)+1);
	/*Create the string*/	
	sprintf(logOutput,"%s\t%s\t%s\t%s",Date,client_ip,httpHeader,response);
	free(Date);
	/*Don't use more space than we need*/	
	realloc(logOutput,strlen(logOutput)+1);
	return logOutput;
}
/*Match a pattern, and put the locations of the match in a regmatch_t struct. Powered by regex */
int searchPattern(char *string, char * searchPattern,regmatch_t * match,int flag){
	regex_t preg;
	int regStatus=regcomp(&preg,searchPattern,flag);
	if(regStatus!=0){
		return -1;
	}
	int Doesmatch=regexec(&preg,string,sizeof(string),match,0);
	if(Doesmatch!=0){
		return 0;	
	}
	return 1;
}
/*Create a substring using regmatch_t to find the locations*/
char *substringRegex(char *src, char *dest,regmatch_t *match){
	char *offsetString=src+match[0].rm_so;
	int length=match[0].rm_eo-match[0].rm_so;
	dest=strncpy(dest,offsetString,length);
	dest[length]='\0';
	return dest;
}
/*Generate server responses when an error occurs*/
void responseText(int status,char **requestResponse,char **content){
	if(content==NULL){
		/*Ensure that content doesn't become NULL*/		
		char *safety="";
		content=&safety;		
	}	
	if(status==400){
		*requestResponse="Bad Request";
		*content=content==NULL?NULL:"<html><body>\n<h2>Malformed Request</h2>\n\
		Your browser sent a request I could not understand.\n</body></html>\n";
	}
	else if(status==500){
		*requestResponse="Internal Server Error";
		*content=content==NULL?NULL:"<html><body>\n<h2>Oops. \
		That Didn't work</h2>\nI had some sort of problem dealing with your request. Sorry, I'm lame.\n</body></html>\n";
	}
	else if(status==403){
		*requestResponse="Forbidden";
		*content=content==NULL?NULL:"<html><body>\n<h2>Permission Denied</h2>\n\
		You asked for a document you are not permitted to see. It sucks to be you.\n</body></html>\n"; 
	}
	else if(status==404){
		*requestResponse="Not Found";
		*content=content==NULL?NULL:"<html><body>\n<h2>Document not found</h2>\n\
		You asked for a document that doesn't exist. That is so sad.\n</body></html>\n"; 	
	}
	else{
		*requestResponse="OK";	
	}
}
/*Create a string representation of the date*/
char * getDate(){
	time_t serverTime;
	time(&serverTime);
	char *date=asctime(gmtime(&serverTime));
	/*Replace newline, (produced in date) with null character*/
	*(date+strlen(date)-1)='\0';    
	char *out=(char *)malloc(strlen(date)+10);
	sprintf(out,"%s GMT",date);
	realloc(out,strlen(out)+1);
	return out;
}
