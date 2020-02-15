#include <pthread.h>
#include <stdio.h> //printf
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <sys/socket.h>
#include <errno.h> //For errno - the error number
#include <netdb.h> //hostent
#include <arpa/inet.h>
#include <unistd.h> // for excelp
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <math.h>
#define NUM_THREADS     						1
#define HTTP_REQUEST_LENGHT 					1000
#define HTTP_CONTENT_TYPE_FIELD 				"Content-Type"
#define UNKNOWN_STATUS_CODE 					5000


struct sock_data{
   int  sockfd;
   int  newsockfd;
   struct 	sockaddr_storage cli_addr;
};

char* ImageExtension[]={"jpeg","jpg","png","bimp"};

/*Routine that each thread executes to handle HTTP request*/
void *handleRequest(void *sockData);
void handleGetRequest(char *filePath,int newsockfd,char *LMT_C);
int writeData(char *filename,int totalBytes,int headerLength,char *data);
char * readFile(char *filename,long *size,int *status,char *LMTR,char *LMT_C);
char * create_HTTP_ResponsePacket_GET(char *location,char *Content_language,char* Content_length,char* Content_type,char* Content_encoding,char* Last_modified );
bool checkImageExtension(char *ext);
char * createHTTPpackage_PUT(char *host,char *cmd);
char *getResponse(int sockfd, int * totalBytes);
void handelPutRequest(char *request,int totalBytes,int newsockfd,char *filepath);
int HTTP_parser(char *httpResponse,char *filename );
void sendStatusCodesResponse(int code,int newsockfd);
void *busythread(void *sockdata);