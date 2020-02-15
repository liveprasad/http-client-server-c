#include "myHTTP.h"

FILE *fpp;
time_t rawtime;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 0;


int main (int argc, char *argv[])
{

	int						sockfd, newsockfd ; /* Socket descriptors */
	int						clilen;
	struct 	sockaddr_in		serv_addr;
	struct sockaddr_storage cli_addr;

	/*The following system call opens a socket.
	On which TCP communication can be done.
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Cannot create socket\n");
		exit(0);
	}


	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= INADDR_ANY;
	serv_addr.sin_port		= htons(60000);

	/* With the information provided in serv_addr, we associate the server
	   with its port using the bind() system call. 
	*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
					sizeof(serv_addr)) < 0) {
		printf("Unable to bind local address\n");
		exit(0);
	}

	listen(sockfd, NUM_THREADS);

    fpp = fopen("log.txt","w");


    fclose(fpp);  	

	while (1) {

		

		/* The accept() system call accepts a client connection.
		   It blocks the server until a client request comes.

		   The accept() system call fills up the client's details
		   in a struct sockaddr which is passed as a parameter.
		   The length of the structure is noted in clilen. Note
		   that the new socket descriptor returned by the accept()
		   system call is stored in "newsockfd".
		*/
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr,
					&clilen) ;


		if (newsockfd < 0) {
			printf("Accept error\n");
			exit(0);
		}

		/* Having successfully accepted a client connection, the
		   server now forks. The parent closes the new socket
		   descriptor and loops back to accept the next connection.
		*/
		printf("newsockfd:::::::: %d\n",newsockfd);
		struct sock_data *socketData = (struct sock_data *)malloc(sizeof(struct sock_data));
		socketData->sockfd 		= sockfd;
		socketData->newsockfd 	= newsockfd;
		socketData->cli_addr     = cli_addr;
		int 		rc;
		pthread_t 	tID;
		if(count<NUM_THREADS){
			pthread_mutex_lock(&count_mutex);
	    		count = count + 1;
		    pthread_mutex_unlock(&count_mutex);
			
		}
		else{

			rc = pthread_create(&tID, NULL, busythread, socketData);
			if (rc){
		         printf("ERROR; return code from pthread_create() is %d\n", rc);
		         exit(-1);
	      	}
		    continue;
		}
		
		
		rc = pthread_create(&tID, NULL, handleRequest, socketData);
		if (rc){
	         printf("ERROR; return code from pthread_create() is %d\n", rc);
	         continue;
	         //exit(-1);
      	}
		//close(newsockfd);
	}
  

   /* Last thing that main() should do */
    
    pthread_exit(NULL);
}
// function to iplement 503
void *busythread( void *sockData){

	struct sock_data *socketData = (struct sock_data *)malloc(sizeof(struct sock_data));
	socketData 	= (struct sock_data *) sockData;
	int					sockfd, newsockfd ; /* Socket descriptors */
	int 				i;
	char 				buf[HTTP_REQUEST_LENGHT];
	
	sockfd 		= socketData->sockfd;
	newsockfd 	= socketData->newsockfd;
	printf("IN busy :::: %d \n",newsockfd );
	sendStatusCodesResponse(503,newsockfd);
	close(newsockfd);
}

/*@def 			  	: Handle request callback function for each request that arrives
* @param sockdata 	: neccessary data given by the main thread.
* @return 			: does not return anything 
*/

void *handleRequest(void *sockData){

	sleep(10);
	struct sock_data *socketData = (struct sock_data *)malloc(sizeof(struct sock_data));
	socketData 	= (struct sock_data *) sockData;

	struct tm * timeinfo = (struct tm *) malloc(sizeof(struct tm) );
	memset(timeinfo,0,sizeof(struct tm) );
    time ( &rawtime );
  	timeinfo = localtime ( &rawtime );

	struct sockaddr_storage client_addr = socketData->cli_addr;
	socklen_t client_len = sizeof(struct sockaddr_storage);

	// Accept client request	
	char hoststr[100];
	char portstr[100];
	int rc = getnameinfo((struct sockaddr *)&client_addr, client_len, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);


	printf("Request Has arrived\n");

	
	int					sockfd, newsockfd ; /* Socket descriptors */
	int 				i;
	char 				buf[HTTP_REQUEST_LENGHT];
	
	sockfd 		= socketData->sockfd;
	newsockfd 	= socketData->newsockfd;
	char *request,*requestCopy;
	int isAllocated = 0,recvBytes = 0,totalBytes = 0;
	request = NULL;
	requestCopy = NULL;

	printf("IN normal :::: %d \n",newsockfd );
	do{
		for(i=0; i < HTTP_REQUEST_LENGHT; i++) 
			buf[i] = '\0';
		recvBytes = recv(newsockfd, buf, HTTP_REQUEST_LENGHT, 0);
		if (request == NULL)
			printf("IN request NULL\n");
		if(recvBytes<= 0){
			perror("Message from receive:");
			break;
		}

		
		totalBytes  += recvBytes;
		if(isAllocated){
			request = (char *)realloc(request,totalBytes);
		}
		else{
			printf("receive :%d\n",recvBytes );
			request = (char*)malloc(recvBytes);
		}
		memcpy((request+totalBytes-recvBytes),buf,recvBytes);
		//printf(" Request is : %s\n", request);
		fflush(stdout);
		isAllocated = 1;
		char *cmdType = malloc(4);
		memcpy(cmdType,request,3);
		cmdType[3] = 0;
		if(strcmp(cmdType,"GET") == 0)
			break;
		
	}while(1);	

	if(request == NULL)
	{
		printf("No proper request\n");
		sendStatusCodesResponse(400,newsockfd);
		fpp = fopen("log.txt","a");
		fprintf(fpp, "%s: %s: %s :BAD Request  \n\n", asctime (timeinfo), hoststr, portstr);
		fclose(fpp);

		free(request);
		free(requestCopy);
		close(newsockfd);
		printf("Exit from the loop of mutex");
		pthread_mutex_lock(&count_mutex);
			printf("Exit from the loop of mutex");
		    count = count -1;
		pthread_mutex_unlock(&count_mutex);
		pthread_exit(NULL);

	}	

	requestCopy = (char *)malloc(totalBytes);
	memcpy(requestCopy,request,totalBytes);
	char *cmd 		= strtok(requestCopy,"\n");
	char * token = strtok(NULL,"\n");
	char *LMT_C = NULL;
	while( token != NULL){
		LMT_C = strstr(token,"If-Modified-Since:");
		if(LMT_C != NULL)
			break;
		token = strtok(NULL,"\n");
	}

	if(LMT_C !=NULL){
		LMT_C = strstr(LMT_C,":")+1;
		LMT_C = strtok(LMT_C,"\r");
		sprintf(LMT_C,"%s",LMT_C);
		if(strlen(LMT_C) == 0)
			LMT_C = NULL;

	}

	char *cmdType 	= strtok(cmd," ");  	

	// get filepath
	char *cmd_part = strtok(NULL," ");
		sprintf(cmd_part,"%s",cmd_part);
		/*if(strstr(cmd_part,"~") !=NULL){
			cmd_part = strstr(cmd_part,"~")+1;
			char *newPath = malloc(100);
			strcpy(newPath,"/home/");
			strcat(newPath,cmd_part);
			cmd_part = newPath;
			
		}*/
	if(strstr(cmd_part,"//") != NULL){

		cmd_part = strstr(cmd_part,"//")+2;
		printf("filePath is %s\n",cmd_part );
		cmd_part = strstr(cmd_part,"/")+1;
		printf("filePath is %s\n",cmd_part);
	}
	else if(cmd_part[0] == '/')
		cmd_part = cmd_part+1;

	/*MyHTTP maintains an Access Log (AccessLog.txt) which records every client accesses. Format of
		every record/line of the AccessLog.txt: <Date(ddmmyy)>:<Time(hhmmss)>:<Client IP>:<Client
	Port>:<GET/PUT>:<URL>*/
	fpp = fopen("log.txt","a");
	fprintf(fpp, "%s: %s: %s : %s: %s \n\n", asctime (timeinfo), hoststr, portstr,cmdType,cmd_part);
	fclose(fpp);
	
	if(strcmp(cmdType,"GET") == 0|| strcmp(cmdType,"get")== 0){		

		handleGetRequest(cmd_part,newsockfd,LMT_C);

	}
	else if(strcmp(cmdType,"PUT") == 0|| strcmp(cmdType,"put") == 0){
		printf("Total bytes in the PUT: %d\n",totalBytes);
		handelPutRequest(request,totalBytes,newsockfd,cmd_part);
	}
	free(request);
	free(requestCopy);
	close(newsockfd);
	printf("Exit from the loop of mutex");
	pthread_mutex_lock(&count_mutex);
		printf("Exit from the loop of mutex");
	    count = count -1;
	pthread_mutex_unlock(&count_mutex);
	pthread_exit(NULL);
}
/* @def 				: handles the HTTP GET request from the client
*  @param 	filePath 	: path of the file requested by the user
*  @param 	newsockfd	: the socket to which the response needs to be sent
*  @return  			: it does not return anything. 	
*/

void handleGetRequest(char *filePath,int newsockfd,char *LMT_C){    
    
    long 	size = 0;
    int status = 0;
    char *LMT =(char *) malloc(50);
    memset(LMT,0,50);
    char 	*data = readFile(filePath,&size,&status,LMT, LMT_C);

    if (status != 0){
    	sendStatusCodesResponse(status,newsockfd);
    	return ;
    }
  	
    char length[10];
    sprintf(length,"%ld",size);	
    char *Content_type =(char *) malloc(100);
    memset(Content_type,0,100);
    if(strstr(filePath,".pdf")){
    	Content_type =(char *) malloc(strlen("application/pdf"));
    	strcpy(Content_type,"application/pdf");
    }
    else if(checkImageExtension((strstr(filePath,".")+1))){
    	char *ext = strstr(filePath,".");
    	ext = ext+1;
    	//sprintf(ext,"%s",ext);

    	strcpy(Content_type,"image/");
    	strcat(Content_type,ext);
    }
    else if(strstr(filePath,".txt")){
    	Content_type =(char *) malloc(strlen("text/plain"));
    	strcpy(Content_type,"text/plain");
    }
    else if(strstr(filePath,".")){
    	Content_type =(char *) malloc(strlen("text/html"));
    	strcpy(Content_type,"text/html");
    } 

    
    char *header 	= create_HTTP_ResponsePacket_GET("local","en",length,Content_type,"",LMT);
    int   headerLength  = strlen(header);
    int   totalBytes 	= (headerLength+size);
    header = realloc(header,totalBytes);
    memcpy(header+headerLength,data,size);

    send(newsockfd,header,totalBytes,0);
    free(header);
    free(data);

}
/* @def   			: Read the file 
*  @param filename  : name of the file to read
*  @param *size 	: size of the file being read
*  @param *status 	: status code of the file being created.
*  @return 			: return the poiter of the data read from file
*/
char * readFile(char *filename,long *size,int *status,char *LMTR,char *LMT_C){

	printf("****************Actual Path is : %s\n",filename);
	

	char *path = realpath(filename,NULL);
	printf("path %s\n",path );
    char *data;
    size_t n = 0;
    char c;

    struct stat st;
	int a = stat(filename, &st);
	*size = st.st_size;

	struct tm * timeinfo = (struct tm *) malloc(sizeof(struct tm) );
	memset(timeinfo,0,sizeof(struct tm) );
	time_t c_time = 0;
	time_t this_time = 0;
	if(LMT_C != NULL){

			strptime(LMT_C,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
			c_time = mktime(timeinfo);
	}

	memset(timeinfo,0,sizeof(struct tm) );
    if(a != -1){
		
		timeinfo = localtime(&st.st_ctime);
		struct tm  *timeinfo1 = (struct tm *) malloc(sizeof(struct tm) );
		memset(timeinfo1,0,sizeof(struct tm) );
		memcpy(timeinfo1,timeinfo,sizeof(struct tm));
		this_time = mktime(timeinfo1);
		strftime(LMTR,50,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
		printf("LMTR ::::: %s\n",LMTR );
	}

	if(LMT_C != NULL){

		double d= difftime(c_time,this_time);
		if(d < 0.0 || fabs(d-0) <= 0.000001)  
			*status = 304;
		return NULL;
	}

	FILE *file = fopen(filename, "r");

	printf("Total bytes of : %ld\n",*size);

    if (file == NULL){
       
        perror("File open:");
        *status = 404;
        return NULL; //could not open file
    }

    data =(char*) malloc(*size);

    fread(data,1, *size,file);
   
    fclose(file);
    return data;
}

/* @def   					: Create Packet header for HTTP GET Response
*  @param location 			: location of the file
*  @param Content_length	: length of the file being sent to the client
*  @param Content_type  	: type of file being sent
*  @param Content_language	: languagae of the file being sent
*  @Content_encoding 		: Encoding of the file being sent
*  @Last_modified 			: Last modification status of the file being sent
*  @return 					: return the starting pointer of the header created.
*/


char * create_HTTP_ResponsePacket_GET(char *location,char *Content_language,char* Content_length,char* Content_type,char* Content_encoding,char* Last_modified ){

	//Response Header : Location, Retry-after
	//Entity Header: Content-language, Content-length, Content-type, Content-encoding,  Last modified
	char *responseHeader = (char *)malloc(1000);
	strcpy(responseHeader,"HTTP/1.x 200 OK");
	strcat(responseHeader,"\r\n");

	strcat(responseHeader,"Cache-Control: no-cache\r\n");

	strcat(responseHeader,"Connection:Close\r\n");
	strcat(responseHeader,"Location:");
	strcat(responseHeader,location);
	strcat(responseHeader,"\r\n"); 
									//X-Pingback: http://net.tutsplus.com/xmlrpc.php
	strcat(responseHeader,"Content-Language:");//: daa
	strcat(responseHeader,Content_language);
	strcat(responseHeader,"\r\n");

	strcat(responseHeader,"Content-Length:");//: 
	strcat(responseHeader,Content_length);
	strcat(responseHeader,"\r\n");

	strcat(responseHeader,"Content-Type:");//: text/html;
	strcat(responseHeader,Content_type);
	strcat(responseHeader,"\r\n"); 

	strcat(responseHeader,"Content-Encoding:");//: gzip;
	strcat(responseHeader,Content_encoding);
	strcat(responseHeader,"\r\n");

	strcat(responseHeader,"Last-Modified:"); //Sat, 28 Nov 2009 03:50:37 GMT
	strcat(responseHeader,Last_modified);
	strcat(responseHeader,"\r\n");

	strcat(responseHeader,"\r\n");

	/*PUT /hello.htm HTTP/1.1
	User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
	Host: www.tutorialspoint.com
	Accept-Language: en-us
	Connection: Keep-Alive
	Content-type: text/html
	Content-Length: 182
*/
	return responseHeader;
}

/* @def   			: Finds out the extension of the file in case of image 
*  @param ext       : actual extension of the image file in message
*  @return 			: return true if ext specified is there in available extensions
*/
bool checkImageExtension(char *ext){
	int i;
	for (i = 0; i < 4; ++i)
	{
		if(strcmp(ImageExtension[i],ext) == 0)
			return true;
	}
	return false;
}


////////////////////////////////////////Handle PUT Request...
//////////////////////////////////////// Create Response for the PUT

/* @def 				: send the response to the different request with specific status code
*  @param 	code 	  	: define the status code of the response
*  @param 	newsockfd 	: the socketFd to send the response to
*  @return  			: does not returns anything
*/

void sendStatusCodesResponse(int code,int newsockfd){
	char *header = (char*)malloc(1000);
	long size;
	char filename[100];
	sprintf(filename,"errorpages/%d.html",code);
	int status;
	char *data = readFile(filename,&size,&status,NULL,NULL);
	switch(code){
		case 201:
			sprintf(header,"HTTP/1.1 201 Created\r\nDate:%s\r\nContent-Type:text/html\r\nContent-length:%ld\r\nConnection:Closed\r\n\r\n"," Mon, 27 Jul 2009 12:28:53 GMT",size );
			break;
		case 404:
			sprintf(header,"HTTP/1.1 404\r\nContent-Type:text/html\r\nContent-length:%ld\r\nConnection:Closed\r\n\r\n",size);
			break;
		case 403:
			sprintf(header,"HTTP/1.1 403\r\nContent-Type:text/html\r\nContent-length:%ld\r\nConnection:Closed\r\n\r\n",size);
			break;
		case 503:
			sprintf(header,"HTTP/1.1 403\r\nContent-Type:text/html\r\nContent-length:%ld\r\nRetry-After:10\r\nConnection:Closed\r\n\r\n",size);
			break;
		case 304:
			sprintf(header,"HTTP/1.1 304\r\nContent-Type:text/html\r\nContent-length:%ld\r\nConnection:Closed\r\n\r\n",size);
			break;
	    case UNKNOWN_STATUS_CODE:
	    	sprintf(header,"HTTP/1.1 5000\r\nContent-Type:text/html\r\nContent-length:%ld\r\nConnection:Closed\r\n\r\n",size);
	    	break;
	}

	int   headerLength  = strlen(header);
    long   totalBytes 	= (headerLength+size);
    header = realloc(header,totalBytes);
    memcpy(header+headerLength,data,size);
    send(newsockfd,header,totalBytes,0);
    //printf("%s\n",header );
    free(header);
    free(data);
}

/* @def 				: send the response to the different request with specific status code
*  @param 	request	  	: actual put request send by the user
*  @param 	totalBytes  : total bytes of the request
*  @param 	newsockfd 	: the socketFd to send the response to
*  @param 	filepath    : path at which the file being stored
*  @return  			: does not returns anything
*/

void handelPutRequest(char *request,int totalBytes,int newsockfd,char *filepath){
	int sockfd;
	char   	*data 				= strstr(request,"\r\n\r\n");
	int 	headerLength 		= data -request;
   	printf("Data length is %d\n",totalBytes-4- headerLength );
	data = data +4;
	if(HTTP_parser(request,filepath)){
		//handle error..
		sendStatusCodesResponse(UNKNOWN_STATUS_CODE,newsockfd); // send the UNKNON ERROr response as there is some mismatch in file format
		printf("error in format\n");
	}
	else{
		if(writeData(filepath,totalBytes,headerLength,data))
			sendStatusCodesResponse(201,newsockfd); 			// send the 201 response as file is successfully created
		else
			sendStatusCodesResponse(UNKNOWN_STATUS_CODE,newsockfd); 
	}

}

/* @def 				: send the response to the different request with specific status code
*  @param 	filename  	: actual put request send by the user
*  @param 	totalBytes  : total bytes of the request
*  @param 	headerLength: length of the header of the request
*  @param 	data    	: path at which the file being stored
*  @return  			: 1 on success , otherwise 0;
*/

int writeData(char *filename,int totalBytes,int headerLength,char *data){
	FILE *fp;
	fp = fopen(filename,"w");
	if(fp == NULL){
		perror("File not written");
		return 0;
	}
	fwrite(data,1,totalBytes-4- headerLength,fp);
	fclose(fp);
	return 1;
}

/* @def 				: check if there is mismatch between content type send by user and filename
*  @param 	httpResponse: actual request send by the user 
*  @param 	filename    : actual file name send by the uer
*  @return  			: return 0 if no mismatch and return 1 for file mismatch
*/
int HTTP_parser(char *httpResponse,char *filename ){
			
	char *data 						= strstr(httpResponse,"\r\n\r\n");
	int headerLength = data -httpResponse;
	char *httpResponseHeader    = (char *) malloc(headerLength);
	memcpy(httpResponseHeader,httpResponse,headerLength);


	char *token = strtok(httpResponseHeader,"\n");
	char httpHeadersFields[20][100];
	int fields = 0;
	do{
		memcpy(&httpHeadersFields[fields][0],token,strlen(token));
		token 	=  strtok(NULL,"\n");
		fields++;
	}while(token != NULL); 

	int i;
	for (i = 0; i < fields; ++i)
	{
		char *field 		= &httpHeadersFields[i][0];
		if(strstr(field,":") == NULL)
			continue;
		char *fieldName 	= strtok(field,":");
		char *fieldValue 	= strtok(NULL,":");
		sprintf(fieldName,"%s",fieldName);
		sprintf(fieldValue,"%s",fieldValue);
		printf("Fieldname :%s\n", fieldName);
		printf("%s\n",fieldValue );
		if(strcmp(fieldName,HTTP_CONTENT_TYPE_FIELD) == 0){

			if(strstr(fieldValue,"pdf") != NULL && strstr(filename,"pdf")){
				return 0;	
			}
			else if(strstr(fieldValue,"html") !=NULL && strstr(filename,"html")){
				return 0;
			}
			
			else if(strstr(fieldValue,"image") != NULL){
			
				char *ext = strtok(fieldValue,"/");
				sprintf(ext,"%s",ext);
				
				return 0;

			}
			else if(strstr(fieldValue,"text") != NULL && strstr(filename,".txt")){
				return 0;
			}
			else{
				//handle error mismatch
				return 1;
			}
			break;
		}
		
	}

}
