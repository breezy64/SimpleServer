#include <stdint.h>

void sendTolog(int status_code,char * responseInfo, char *logfilename);

char* respond(int * status,int clientfd,char* ip_addr);


