
#include "myBrowser.h"

size_t 		totalBytes;
DOC_TYPE 	docType;
char 		*filename;
char *		data;
int 		headerLength;
int			sockfd ;
char 		*buf;
bool        isCacheFound;
FILE 		*cacheFile;
char 		*GETURL;
bool 		refreshMode;
struct tm  *timeinfo;
time_t 		rawtime;
char 		*modifiedLMT;
bool 		cacheMode;

/*
* @def 			: Create the packet header with the respective command 
* @param host 	: Host name(server) to which the HTTP request to be sent
* @param cmd 	: The command given by the user.
* @return 		: Returns the pointer to the HTTP packet created.
*/


char * createHTTPpackage_GET(char *host,char *cmd){

	//general header: Cache Control, Connection, Host
	
	char *HTTP_packet =	(char *) malloc(1000);
	memset(HTTP_packet,0,1000);
	strcpy(HTTP_packet,"");
	strcat(HTTP_packet,cmd);
	strcat(HTTP_packet,"\r\n");
	if(!cacheMode)
		strcat(HTTP_packet,"Cache-Control:no-cache\r\n");
	strcat(HTTP_packet,"Connection:keep-alive\r\n");
	strcat(HTTP_packet,"Host:");
	strcat(HTTP_packet,host); 
	strcat(HTTP_packet,"\r\n"); 


	/*strcat(HTTP_packet,"Accept-Language: en-us,en;q=0.5\r\n");
	strcat(HTTP_packet,"Accept-Encoding: gzip,deflate\r\n");
	strcat(HTTP_packet,"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n");
	//Request Header:If-Modified-Since
	strcat(HTTP_packet,"Keep-Alive: 300\r\n");*/
	if(refreshMode){
		if(readCache(GETURL,modifiedLMT)){
			strcat(HTTP_packet,"If-Modified-Since:");
			strcat(HTTP_packet,modifiedLMT);
			strcat(HTTP_packet,"\r\n");
		}
		
	}

	strcat(HTTP_packet,"\r\n");

	return HTTP_packet;
}

/*
 * @def				: Take hostname and maps it to the ip address by using DNS.
 * @param hostname	: the hostname for which ip address to be found
 * @param ip		: the ip address holder
 * @return			: returns 0 on error, 1 on success
 * */

int hostname_to_ip(char * hostname , char* ip)
{	
	fflush(stdout);
    struct hostent *he;
    struct in_addr addr_list;
    int i;         
    if ( (he = gethostbyname( hostname ) ) == NULL)
    {
        // get the host info
        herror("gethostbyname");
        return 0;
    }  

    addr_list = *(struct in_addr *) he->h_addr;    
    strcpy(ip , inet_ntoa(addr_list) );     
    return 1;
}
/*@def 		:
* @param url 	  : The URL to be written
* @param LMT      : Last modified time of the file
* @param filename : Name of the file to be stored
* @param 
* @return 
*/
void writeCache(char *url,char *LMT,char *filename){
	int size = strlen(url) + strlen(filename);
	char *record = malloc(size+3);
	memset(record,0,size+3);
	strcpy(record,url);
	strcat(record,"$");
	strcat(record,filename);
	cacheFile = fopen(CACHE_FILE,"a");
	fprintf(cacheFile, "%s\n", record);
	fclose(cacheFile);
	char c[200];
	if(LMT == NULL){
		getCurrTimeLMT();
		LMT = (char *)malloc(50);
		memset(LMT,0,50);
		strftime(LMT,50,"%Y%m%d%H%M.%S",timeinfo);		
		sprintf(c,"touch -t %s %s",LMT,filename);
		system(c);
	}
	else{
		LMT = (char *)malloc(50);
		memset(LMT,0,50);
		timeinfo = malloc(sizeof(struct tm));
		memset(timeinfo,0,sizeof(struct tm));
		strptime(modifiedLMT,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
		strftime(LMT,50,"%Y%m%d%H%M.%S",timeinfo);		
		sprintf(c,"touch -t %s %s",LMT,filename);
		system(c);

	}
	printf(".................... Done with writing in cache...........\n");

}

void datetimetostr( time_t time ,char *buffer)
{
    struct tm *timeinfo;
    timeinfo = gmtime(&time);
    strftime(buffer,80,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
}


void setFileType(){

	if(strstr(filename,".pdf") != NULL)
		docType = PDF;
	else if(strstr(filename,".html") !=NULL)
		docType = HTML;
	else if(checkImageExtension((strstr(filename,".")+1))){
		docType = IMAGE;
	}
	else if(strstr(filename,".text") != NULL)
		docType = TEXT;
	else
		docType = NONE;
}

int readCache(char *url,char *LMTR){
	char *record;
	size_t read;
	size_t len = 0;
	cacheFile = fopen(CACHE_FILE,"r");
    while ((read = getline(&record, &len, cacheFile)) != -1) {
            
            if(strstr(record,url) != NULL){
            	
            	char *first  = strstr(record,"$") +1;

            	strtok(first,"\n");
            	sprintf(first,"%s",first);
            	memcpy(filename,first,strlen(first));
            	struct stat st = {0};
				stat(filename, &st);
				LMTR = (char *)malloc(50);
				memset(LMTR,0,50);
				timeinfo = localtime(&st.st_ctime);
				strftime(LMTR,50,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);			
				fclose(cacheFile);
            	return 1;
            }   		       
    }

    fclose(cacheFile);
    return 0;
}

void createGETURL(char * hostDummy){

			int l = strlen(hostDummy);
			GETURL = (char*	) malloc(l);
			memset(GETURL,0,l);
			memcpy(GETURL,hostDummy,l);
			//set the file name of the file for GET
			filename = getfileName(hostDummy);
			sprintf(filename,"%s",filename);
			if(strstr(filename,"index.html")){
				GETURL = (char*	) realloc(GETURL,l+10);
				memcpy(GETURL+l,filename,10);
			}
}

char * getfileName(char * getcmd){


	if(getcmd[strlen(getcmd)-1] == '/' || strstr(getcmd,"/") == NULL)
		return "index.html";

	if(strstr(getcmd,"//") != NULL){
		getcmd = strstr(getcmd,"//")+2;

	}

	while(strstr(getcmd,"/") != NULL)
		getcmd = strstr(getcmd,"/")+1;

	getcmd = strtok(getcmd,":");

	return getcmd;


}
int main(int argc, char const *argv[])
{
	char *cmd = malloc(100);
	buf = (char*) malloc(HTTP_RESPONSE_LENGHT);
	cacheFile = fopen(CACHE_FILE,"w");
	fclose(cacheFile);
	struct sockaddr_in	serv_addr;
	char *choice =(char *) malloc(10);
	filename =(char *) malloc(200);
	timeinfo = (struct tm *)malloc(sizeof(struct tm));
	memset(timeinfo,0,sizeof(struct tm));

	struct stat st = {0};

	if (stat(CACHE_FOLDER, &st) == -1) {
	    mkdir(CACHE_FOLDER, 0777);
	}
	while(1){

		char ip[100];
		
		totalBytes 	= 0;
		docType 	= NONE;
		int 	port = 80;
		memset(choice,0,10);
		memset(filename,0,200);
		memset(cmd,0,100);
		refreshMode = false;
		cacheMode   = false;
		//get command input from user from terminal;
		printf("\nmyBrowser>");
		fgets (cmd,100,stdin);

		if(strlen(cmd)<4){
			printf("Invalid input Try again\n");
			continue;
		}

		cmd = strtok(cmd, "\n");

         
		if(strcmp(cmd,"exit") == 0 || strcmp(cmd,"EXIT") == 0)
			break;



		char *cmdForPut = (char *)malloc(strlen(cmd));
		memset(cmdForPut,0,strlen(cmd));
		memcpy(cmdForPut,cmd, strlen(cmd));
		//Find the port intended by user


		if(strstr(cmd,"HTTP/1.1") == NULL)
			strcat(cmd," HTTP/1.1");
		char *portPos;

		if( strstr(cmd,":") != NULL && (portPos = strstr(strstr(cmd,":")+1,":"))!=NULL){
			//printf("he%d\n",atoi(portPos+1) );
			port 	= atoi(portPos+1);
			*(strstr(strstr(cmd,":")+1,":")) = 0;
			strcat(cmd," HTTP/1.1");
		}
		else if((portPos = strstr(cmd,":")) != NULL && strstr(cmd,"http://") == NULL){
			//printf("here %d\n",atoi(portPos+1) );
			port 	= atoi(portPos+1);
			*(strstr(cmd,":")+1) = 0;
			strcat(cmd," HTTP/1.1");
		}
		
		char *cmdDummy = (char *)malloc(strlen(cmd));
		memset(cmdDummy,0,strlen(cmd));
		memcpy(cmdDummy,cmd,strlen(cmd));

       	// parse it to get command Type;
		char * cmdType 	= strtok(cmdDummy," ");
		sprintf(cmdType,"%s",cmdType);

		char *host 		=strtok(NULL," ");	
		sprintf(host,"%s",host);             // get the host name
		char *hostDummy  = (char *)malloc(strlen(host));
		memset(hostDummy,0,strlen(host));
		strcpy(hostDummy,host);
		if(port != 80){
					sprintf(hostDummy,"%s%d",hostDummy,port);
					sprintf(hostDummy,"%s",hostDummy);
		}
		
		if(strcmp(host,"http") == 0){
			printf("May be input command is wrong...\n");
			continue;
		}

		if(strcmp(cmdType,"REFRESH") == 0 || strcmp(cmdType,"refresh") == 0){
			refreshMode = true;
			cmdType = (char *)malloc(3);
			strcpy(cmdType,"GET");
		}

		if(strstr(host,"//") != NULL){
			strtok(host,"//");
			host = strtok(NULL,"//");
		}else{
			//append http:// to command
			host 	= strtok(host,":");
			strcpy(cmd,cmdType);
			strcat(cmd," http://");
			strcat(cmd,host);
			strcat(cmd," HTTP/1.1");
			host 	= strtok(host,"/");

		}

		if(!hostname_to_ip(host,ip)){
			printf("Please input valid command\n");
			continue;
		}

		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Unable to create socket\n");
			exit(0);
		}
		serv_addr.sin_family		= AF_INET;
		serv_addr.sin_addr.s_addr	= inet_addr(ip);
		serv_addr.sin_port		    = htons(port);

		
	
		if(strcmp(cmdType,"GET") == 0 || strcmp(cmdType,"get") == 0 || refreshMode){
			bool doCache = false;
			createGETURL(hostDummy);
			if(!refreshMode){
				do{
					
					printf("Is a cached copy ok (yes/no)?\n");
					scanf("%s" ,choice);
					//choice = strtok(choice,"\n");
					if(memcmp(choice,"yes",3) == 0 || memcmp(choice,"no",2) == 0)
						break;

				}while(1 );
				isCacheFound == false;			
				
				
				if( strcmp(choice,"yes") == 0){
					//serve the cached copy of the page requested by the user
					cacheMode = true;
					if(readCache(GETURL,NULL) == 1){
						//file is found in the cache read it from the location and show it to the user.
						setFileType(); 
						showData();
						isCacheFound = true;
					}
					else{
						//file is not found in the cache, you have to go to the server.
						isCacheFound = false;
					}
					doCache = true;
					
				}
			}

			if( strcmp(choice,"no") == 0 || isCacheFound == false || refreshMode ){			
				//open TCp connection
				if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {

					perror("Unable to connect to server\n");
					printf("May be server is not running.Please check your url again\n");
					continue;
				}
				httpGet(host,cmd,doCache);
				close(sockfd);
			}			
		}
		//handle Put command 
		else if(strcmp(cmdType,"put") == 0 || strcmp(cmdType,"PUT") == 0){
				//open TCp connection
			if ((connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr))) < 0) {

				perror("Unable to connect to server\n");
				printf("May be server is not running.Please check your url again\n");
				continue;
			}
			strtok(cmdForPut," ");
			strtok(NULL," ");
			char *filename = strtok(NULL," ");
			printf("Put has to be done for :%s\n",filename );
			*(strstr(cmd,"HTTP/1.1")-1) = 0;
			strcat(cmd,"/");
			strcat(cmd,filename);
			strcat(cmd," HTTP/1.1");
			printf("%s\n",cmd );
			httpPut(filename,host,cmd);
			close(sockfd);
		}
		else
			printf("Invalid input.Please try again\n");
		//exit(0);
	}
	return 0;
}

/*
* @def 			: It takes data from user for GET command and creates the packet 
			  	  waits for server response and then accordingle takes the action
* @param host 	: hostname extracted from the user command
* @param cmd 	: user command taken
* @return 		: It does not returns anything
*/


void httpGet(char *host,char *cmd,bool doCache){
			// create the HTTP header
			char * HTTP_packet = createHTTPpackage_GET(host,cmd);
			printf("\n%s\n",HTTP_packet );
			fflush(stdout);
			
			//send a request to server over opened TCP connection by using Ip that has been extraxted;
			send(sockfd, HTTP_packet, strlen(HTTP_packet) + 1, 0);
				
			char * response = getResponse();
			//parse the response from the server and take appropriate action on it.			
			fflush(stdout);

			if(response == NULL){
				printf("No response from server. Please try again.\n May be connection error\n");
				return ;
			}
			int a = HTTP_parser(response);
			if( a != 0)
				doCache = false;

			if(a == 304){
				if(readCache(GETURL,NULL) == 1){
					setFileType();
				}
				showData();
				free(response);
				return;
			}
			fflush(stdout);
			//write data to file first.
			writeData(doCache);
			//Shows data with respect to data type
			showData();
			free(response);
			//free(data);
			
}
/*@def    : Receives the response from  the client 
* @return : Return the pointer of the resonse recived from server 
*/

char *getResponse(){
			int i,recvBytes;
			char *response = NULL;
			totalBytes = 0;
			// wait for receiving the HTTP response from server

			
			//Read bytes of size HTTP_RESPONSE_LENGHT at a time and store them to the buffer
			int isAllocated = 0;
			do{
				memset(buf,0,HTTP_RESPONSE_LENGHT);

				recvBytes = recv(sockfd, buf, HTTP_RESPONSE_LENGHT, 0);
				if(recvBytes<= 0){
					perror("Message from receive:");
					break;
				}
				totalBytes  += recvBytes;

				response = (char *)realloc(response,totalBytes *sizeof(char));
				memcpy((response+totalBytes-recvBytes),buf,recvBytes);
				
				fflush(stdout);
				isAllocated = 1;
			}while(1);
			return response;
}

/*@def 			: Parse the first line of response to get status code
* @param token	: first line of the response
* @return 		: 1 on error code ,0 on successful response
*/

int handleStatusCode(char *token){

    char *tok = strtok(token,"\r");

    strtok(tok," ");
    tok = strtok(NULL," ");

    if(strstr(tok,"200")!=NULL || strstr(tok,"201") != NULL)
    	return 0;

    if(strstr(tok,"304") != NULL ){

    	return 304;
    }
    sprintf(filename,"%s.html",tok);
    docType = HTML;
	return 1;
    
}


/*@def 			: It parses the response
* @return 		: It returns 1 for error response else returns 0;
*/

int HTTP_parser(char *httpResponse ){
	//printf("TotalBytes receive : %ld\n",totalBytes );		
	data 						= strstr(httpResponse,"\r\n\r\n");
	headerLength = data -httpResponse;
	char *httpResponseHeader    = malloc(headerLength);
	memset(httpResponseHeader,0,headerLength);
	memcpy(httpResponseHeader,httpResponse,headerLength);
	//printf("******************\n%s\n************************", httpResponseHeader);

	data = data +4;

	//printf("%s\n", data );
	modifiedLMT = NULL;

	char *token = strtok(httpResponseHeader,"\n");
	char httpHeadersFields[20][100];
	int fields = 0;
	do{
		strcpy(&httpHeadersFields[fields][0],token);
		token 	=  strtok(NULL,"\n");
		fields++;
	}while(token != NULL && fields < 20);
	//printf("woooooooooooooooowwwwwwwwwwww\n");
	token = (char *) malloc(strlen((&httpHeadersFields[0][0])));
	memset(token,0,strlen((&httpHeadersFields[0][0])));
	memcpy(token,&httpHeadersFields[0][0],strlen((&httpHeadersFields[0][0])));
	int a = 0;
	a = handleStatusCode(token);
	if(a != 0){
		return a;
	}
	
	int i;
	for (i = 0; i < fields; ++i)
	{
		char *field 		= &httpHeadersFields[i][0];
		if(strstr(field,":") == NULL)
			continue;
		char *fieldValue 	= strstr(field,":")+1;
		char *fieldName 	= strtok(field,":");

		sprintf(fieldName,"%s",fieldName);
		sprintf(fieldValue,"%s",fieldValue);
		if(fieldValue == NULL)
			continue;
		fflush(stdout);
		if(strcmp(fieldName,HTTP_CONTENT_TYPE_FIELD) == 0){

			if(strstr(fieldValue,"pdf") != NULL)
				docType = PDF;
			else if(strstr(fieldValue,"html") !=NULL)
				docType = HTML;
			else if(strstr(fieldValue,"image") != NULL){
				docType = IMAGE;
			}
			else if(strstr(fieldValue,"text") != NULL)
				docType = TEXT;
			else
				docType = NONE;
			
			//break;
		}

		
		if(strcmp(fieldName,HTTP_LAST_MODIFIED_FIELD) == 0){
			fieldValue = strtok(fieldValue,"\r");
			modifiedLMT = malloc(strlen(fieldValue));
			memset(modifiedLMT,0,strlen(fieldValue));
			memcpy(modifiedLMT,fieldValue,strlen(fieldValue));
		}

		
	}
	return 0;

}



void getCurrTimeLMT(){

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	modifiedLMT = malloc(strlen(asctime (timeinfo)));
	memset(modifiedLMT,0,strlen(asctime (timeinfo)));
	memcpy(modifiedLMT,asctime (timeinfo),strlen(asctime (timeinfo)));	

}

void setFilePath(){
		char *filepath = (char *) malloc(200);
		char *orgName = (char*)malloc(200);
		memset(orgName,0,200);
		memset(filepath,0,200);

		memcpy(orgName,filename,strlen(filename));
		sprintf(filepath,"%s/%s",CACHE_FOLDER,filename);

		struct stat st = {0};
		if (stat(filepath, &st) != -1){
			//update file name with URL in it.
			char *dummyGETURL = (char *)malloc(strlen(GETURL));
			memcpy(dummyGETURL,GETURL,strlen(GETURL));	
			if(strstr(dummyGETURL,":") != NULL )
				*(strstr(dummyGETURL,":")) = 0;

			char *prev = NULL, *cur = strstr(dummyGETURL,"/")+1;
			prev = cur;
			while(strstr(cur,"/")){
				cur = strstr(cur,"/") + 1;
				prev = cur;

			}
			*(--prev) = 0;

			while((cur = strstr(dummyGETURL,"/")) != NULL || (cur = strstr(dummyGETURL,".")) != NULL )
				*cur = ' ';
			strcpy(filename,"");
			char * tok = strtok(dummyGETURL," ");
			while(tok != NULL){
				strcat(filename,tok);
				tok = strtok(NULL," ");
			}
			strcat(filename,orgName);
		}

		sprintf(filepath,"%s/",CACHE_FOLDER);
		strcat(filepath,filename);
		sprintf(filename,"%s",filepath);
		sprintf(filename,"%s",filename);
}

writeFile(){
	FILE *fp;
	fp = fopen(filename,"w");
	fwrite(data,1,totalBytes-4- headerLength,fp);
	fclose(fp);
}

/*@ def  	: write the data to the file recived from the user
*
*/

void writeData(bool doCache){

	printf("Data writing\n");

	

    if(doCache){

    	if(modifiedLMT== NULL)
			getCurrTimeLMT();
	
		setFilePath();
		writeFile();
		writeCache(GETURL,modifiedLMT,filename);
		return;
	}

	if(refreshMode){
		if(readCache(GETURL,NULL) == 1){
			setFileType();
		}
		else{
			setFilePath();
		}

		writeFile();
		writeCache(GETURL,modifiedLMT,filename);
		return;
	}
	
	writeFile();

}
/*@def    : displays the response from sever with corresponding application
  @return : does not return anything
*/

void showData(){

	//printf("In show data %s\n",filename);
	switch(docType){

		case PDF :
			if(!fork()){
    			execlp("evince", "evince", filename, NULL);
			}
    		break;
    		
    	case IMAGE :
    		if(!fork())
    			execlp("gimp", "gimp", filename, NULL);
    		break;

		case HTML:
			if(!fork())
    			execlp("firefox", "firefox", filename, NULL);
			break;

		case TEXT:
		case NONE:
			if(!fork())
    			execlp("gedit", "gedit", filename, NULL);
    		break;
	}
}
/* @def   					: Create Packet header for HTTP PUT 
*  @param host 				: name of the server to which request is being send
*  @param cmd 	 			: path of the file along with command type
*  @param Content_length	: length of the file being put on the server
*  @param Content_type  	: type of file being uploaded
*  @param Content_language	: languagae of the file being uploaded
*  @return 					: return the starting pointer of the header created.
*/
char *create_RequestPacket_Header_PUT(char *host,char * cmd,char * Content_length,char *Content_type,char *Content_language){

	char *putRequestHeader = (char *)malloc(1000);
	memset(putRequestHeader,0,1000);
	strcpy(putRequestHeader,cmd);
	strcat(putRequestHeader,"\r\n");
	strcat(putRequestHeader,"Host:");
	strcat(putRequestHeader,host);
	strcat(putRequestHeader,"\r\n");

	strcat(putRequestHeader,"Cache-Control: no-cache\r\n");
	/*
	strcat(putRequestHeader,"Location:");
	//strcat(putRequestHeader,location);
	strcat(putRequestHeader,"\r\n"); */
									//X-Pingback: http://net.tutsplus.com/xmlrpc.php
	strcat(putRequestHeader,"Content-Language:");//: daa
	strcat(putRequestHeader,Content_language);
	strcat(putRequestHeader,"\r\n");

	strcat(putRequestHeader,"Content-Type:");//: text/html;
	strcat(putRequestHeader,Content_type);
	strcat(putRequestHeader,"\r\n"); 

	strcat(putRequestHeader,"Content-Length:");//: 
	strcat(putRequestHeader,Content_length);
	strcat(putRequestHeader,"\r\n");

	/*strcat(putRequestHeader,"Content-Encoding:");//: gzip;
	strcat(putRequestHeader,Content_encoding);
	strcat(putRequestHeader,"\r\n");*/

	/*strcat(putRequestHeader,"Last-Modified:"); //Sat, 28 Nov 2009 03:50:37 GMT
	strcat(putRequestHeader,"Sat, 28 Nov 2009 03:50:37 GMT");
	strcat(putRequestHeader,"\r\n");*/

	strcat(putRequestHeader,"\r\n");

	/*PUT /hello.htm HTTP/1.1
	User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)
	Host: www.tutorialspoint.com
	Accept-Language: en-us
	Connection: Keep-Alive
	Content-type: text/html
	Content-Length: 182
*/

	return putRequestHeader;
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

/* @def 				: handles the HTTP PUT request 
*  @param 	filePath 	: path of the file requested by the user
*  @param 	host 		: host to which data to be sent
*  @param   cmd 		: cmd from the user
*  @return  			: it does not return anything. 	
*/

void httpPut(char * filename , char *host,char * cmd){

	long 	size = 0;
    char 	*data 		=  readFile(filename,&size);
    if(data == NULL){
    	return ;
    }
    char length[10];
    sprintf(length,"%ld",size);	

    char *Content_type =(char *) malloc(100);
    memset(Content_type,0,100);
    if(strstr(filename,".pdf"))
    	strcpy(Content_type,"application/pdf");
    else if(strstr(filename,".txt"))
    	strcpy(Content_type,"text/plain");
    else if(strstr(filename,".html"))
    	strcpy(Content_type,"text/html");
    else if(checkImageExtension((strstr(filename,".")+1))){
    	char *ext = strtok(filename,".");
    	ext = ext+1;
    	sprintf(ext,"%s",ext);

    	strcpy(Content_type,"image/");
    	strcat(Content_type,ext);
    }

    char *header 	= create_RequestPacket_Header_PUT(host,cmd,length,Content_type,"en");

    //printf("Header created for the put is %s\n",header );
    int   headerLength  = strlen(header);
    int   totalBytes 	= (headerLength+size);
    header = realloc(header,totalBytes);
    memcpy(header+headerLength,data,size);
    send(sockfd,header,totalBytes,0);
    //Half close the connection of TCP so that server comes out of the loop

    //printf("Total bytes send are : %d\n", totalBytes );
    shutdown(sockfd,SHUT_WR);
    char *response = getResponse();
    if(response == NULL){
    	printf("No response from server. Please try again\n");
    	return ;
    }
    HTTP_parser(response);
    writeData(false);
    showData();	
}

/* @def   			: Read the file 
*  @param filename  : name of the file to read
*  @param *size 	: size of the file being read
*  @param *status 	: status code of the file being created.
*  @return 			: return the poiter of the data read from file
*/

char * readFile(char *filename,long *size){

	//printf("****************Actual Path is : %s\n",filename);
	FILE *file = fopen(filename, "r");

	char *path = realpath(filename,NULL);
	//printf("path %s\n",path );
    char *data;
    size_t n = 0;
    char c;

    struct stat st;
	stat(filename, &st);
	*size = st.st_size;

	//printf("Total bytes of : %ld\n",*size);

    if (file == NULL){
       

        perror("File open:");
         return NULL; //could not open file
    }

    data =(char*) malloc(*size);
    memset(data,0,*size);

    fread(data,1, *size,file);
   
    fclose(file);
    return data;
}



