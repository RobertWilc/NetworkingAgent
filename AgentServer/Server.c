#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>


struct Agent {
	char* ip;
	int time;

};

int size = 500;
struct Agent agentsList[500];
int ind = 0;

void error(char *msg) {
  perror(msg);
  exit(1);
}


void writingError(int bytes){
	if(bytes < 0){
		perror("Error writing on socket.");
		exit(1);
	}
}
void writeLogFile(FILE * adapter, char* logMsg, char* clientIp){

	time_t clk = time(NULL);
	char* time = ctime(&clk);

	fputs(time, adapter);
	fputs(logMsg, adapter);
	fputs(clientIp, adapter);
	fputs("'\n\n", adapter);
}

int addAgentInList(char* ip){

	agentsList[ind].ip = ip;
	agentsList[ind].time = (int)time(NULL);
	ind += 1;
	return 1;
}

int removeAgentFromList(char* ip){
	for(int i=0; i<size; i++){
		if(agentsList[i].ip == ip ){
			size = size - 1;
			for(int j=i; j<size; j++){
				agentsList[j].ip = agentsList[j+1].ip;
				agentsList[j].time = agentsList[j+1].time;
			}
			ind -= 1;
			return 1;
		}
	}
	return 0;
}

void displayAllAgents(){
	for(int i=0; i<size ; i++){
		if(agentsList[i].ip == NULL){
			break;
		}
		printf("%s", agentsList[i].ip);
		printf("%s", ", ");
		printf("%d\n", agentsList[i].time);
	}
}
int checkIfAgentActive(char* ip){

	for(int i =0; i<size; i++){
		if(agentsList[i].ip == ip){
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{

	if(argc < 0){
		printf ("Usage Error: Server <port> required.\r\n");
      	return(0);
	}
	struct sockaddr_in seraddr, cliaddr;

	/**
	 * nsockfd : socket file descripter to accept client connection
	 * sockfd : socket file descripter on which server listen
	 * */
	int nsockfd, bt, lt, sockfd;
	int byteswritten, bytesRead, PORT;
	char buff[1024];
	socklen_t addr_size;

	FILE *adapter, *fileReadPointer; 

	PORT = atoi(argv[1]);
	char* IP = "127.0.0.1";

	sockfd = socket(AF_INET, SOCK_STREAM, 0); //create server socket to listen
	if (sockfd < 0)
	{
		printf("Socket not Created!!!\n");
		return 0;
	}

	seraddr.sin_family = AF_INET;					  //set Ip version family
	seraddr.sin_port = htons(PORT);					  //set server port
	seraddr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr(IP); //set server address
	memset(&(seraddr.sin_zero), '\0', 8);

	//bind to the socket which was created first. bind() retunr -1 if binding failed
	bt = bind(sockfd, (struct sockaddr *)&seraddr, sizeof(struct sockaddr));
	if (bt < 0)
	{
		printf("Binding Failed!!!\n");
		return 0;
	}

	//start listening on created socket.
	lt = listen(sockfd, 5);
	if (lt < 0)
	{
		printf("Listen Failed!!!\n");
		return 0;
	}

	addr_size = sizeof(struct sockaddr);

	//-----------------------------------------------------------------------------

	while(1){

		printf("Server is listening...\n");
		memset(&cliaddr, 0, sizeof(cliaddr));
		
		//accept requests from connected client
		nsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &addr_size);

		char* clientIpStr = inet_ntoa(cliaddr.sin_addr);
		char* logMsg2;

		adapter = fopen("log.txt", "a");
		fileReadPointer = fopen("log.txt", "r");

	//while(1){
		memset(buff, '\0', sizeof(buff));
		//recv(nsockfd, buff, sizeof(buff), 0); //recieve message from client in buff
		bytesRead = read(nsockfd, buff, sizeof(buff));
		printf("Agent Action: %s\n", buff);

		if(bytesRead < 0 ){
			error("ERROR reading from socket");
		}else{
			if(strcmp(buff, "JOIN") == 0){
				logMsg2 = "Recieved a '#JOIN' action from agent '";
				writeLogFile(adapter, logMsg2, clientIpStr);

				if(checkIfAgentActive(clientIpStr) == 1){
					char buf[1024] = "$Already Member";
					byteswritten = write (nsockfd, buf, strlen(buf));
					logMsg2 = "Responded with '$Already Member' to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}
				else{
					byteswritten = write (nsockfd, "$OK", strlen(buff));

					logMsg2 = "Responded with '$OK' to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
					//TODO: Add client in the List
					int chk = addAgentInList(clientIpStr);
				}

			}
			else if(strcmp(buff, "LEAVE") == 0){
				logMsg2 = "Recieved a '#LEAVE' action from agent '";
				writeLogFile(adapter, logMsg2, clientIpStr);

				//TODO: CHeck if agent in the list & Remove agent from the list
				if(checkIfAgentActive(clientIpStr) == 1){
					removeAgentFromList(clientIpStr);
					byteswritten = write (nsockfd, "$OK", strlen(buff));
					logMsg2 = "Responded with '$OK' to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}else{
					char* buf = "$NOT Member";
					byteswritten = write (nsockfd, buf, strlen(buf));
					logMsg2 = "Responded with '$Not Member' to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}

			}
			else if(strcmp(buff, "LIST") == 0){
				logMsg2 = "Recieved a '#LIST' action from agent '";
				writeLogFile(adapter, logMsg2, clientIpStr);

				//TODO: CHeck if agent in the list & and send list to the agent
				if(checkIfAgentActive(clientIpStr) == 1){
					//send list through loop
					for(int i=0; i<size; i++){
						if(agentsList[i].ip == NULL){
							break;
						}
						char* ip = agentsList[i].ip;
						int tim = agentsList[i].time;
						char* timm;
						sprintf(timm, "%d", tim);

						char *str1 = (char *) malloc(1 + strlen(ip)+ strlen(timm) + strlen(", ") );
						strcpy(str1,ip);
						strcat(str1, ", ");
						strcat(str1,timm);

						strncpy(buff, str1, sizeof(buff) - 1);
						buff[1024] = '\0';
						byteswritten = write (nsockfd, buff, strlen(buff));
					}
					logMsg2 = "Active agents List is sent to '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}else{
					logMsg2 = "(agent not active) No response is supplied to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}
				
			}
			else if(strcmp(buff, "LOG") == 0){
				logMsg2 = "Recieved a '#LOG' action from agent '";
				writeLogFile(adapter, logMsg2, clientIpStr);
				fclose(adapter);

				//TODO: CHeck if agent in the list & and send logfile to the agent
				if(checkIfAgentActive(clientIpStr) == 1){
					//sent log file through loop
					if(fileReadPointer == NULL){
						printf("%s\n", "Could not found file log.txt");
					}else{
						while( fgets ( buff, 50, fileReadPointer ) != NULL ) { 
				           // Print the dataToBeRead  
							printf( "%s\n" , buff ) ;
							byteswritten = write (nsockfd, buff, strlen(buff));
				        } 
				        logMsg2 = "Log file is sent to agent '";
						writeLogFile(adapter, logMsg2, clientIpStr);
					}

				}else{
					logMsg2 = "(agent not active) No response is supplied to agent '";
					writeLogFile(adapter, logMsg2, clientIpStr);
				}
				
			}else{
				//error("Invalid Action..");
				printf("%s\n", "Invalid Action..");
				char buf[1024] = "Invalid Action: Select one of #JOIN, #LEAVE, #LIST, #LOG";
				byteswritten = write (nsockfd, buf, strlen(buf));
				writingError(byteswritten);
			}
			close(nsockfd);
		}

		fclose(adapter);
		fclose(fileReadPointer);
		
	//}
	}

	close(sockfd);
	return 1;
}
