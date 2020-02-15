
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

#define HTTP_RESPONSE_LENGHT 					1000 			// number of bytes to be read at a time from the response received
#define HTTP_SERVICE_PORT 						80
#define MAX_HTTP_RESPONSE_HEADER_FILDS 			200; 		// Max number of fields in the HTTP response can be 
#define HTTP_LAST_MODIFIED_FIELD 				"Last-Modified"
#define PDF_FILENAME							"output.pdf" 
#define APPS_LOG_FILE							"appsLog.txt"
#define HTTP_CONTENT_TYPE_FIELD					"Content-Type"
#define CACHE_FILE 								"cache.txt"
#define CACHE_FOLDER                            "cache"
typedef enum {
				PDF,
				TEXT,
				HTML,
				IMAGE,
				NONE
			} DOC_TYPE ;
char* ImageExtension[]={"jpeg","jpg","png","bimp"};

/*Get the Ip address from the hostname by using DNS*/
int hostname_to_ip(char *  , char *);

/*implements Http Get protocol*/
void httpGet(char *host,char *cmd,bool doCache);
/*implements HTTP Put protocol*/
void httpPut();
/*Create HTTP Packet to send*/
char *createHTTPpackage_GET(char *host,char *cmd);
/*Parse the response from the user to get Header and data separation */
int HTTP_parser(char *httpResponse);
/*Shows the data in respective viewer according to its content Type*/
void showData();
/*Implements put method for the server*/
void httpPut(char * filename , char *host,char * cmd);
char * readFile(char *filename,long *size);
char *create_RequestPacket_Header_PUT(char *host,char * cmd,char * Content_length,char *Content_type,char *Content_language);
bool checkImageExtension(char *ext);
char *getResponse();
int readCache(char *url,char *LMTR);
char * getfileName(char * getcmd);
void createGETURL(char * hostDummy);
void writeCache(char *url,char *LMT,char *filename);
void writeData(bool doCache);
void getCurrTimeLMT();
void setFileType();