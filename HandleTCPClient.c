#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> /* for recv() and send() */
#include <unistd.h>     /* for close() */
#include <string.h>

#define RCVBUFSIZE 100   /* Size of receive buffer */
#define PASSWORD "password"

struct domainIpMap {
	char domainName[50];
	char IP[17];
	int count;
	struct domainIpMap* next;
};

struct IpList {
	char IP[17];
	clock_t startTime;
};

typedef struct domainIpMap* element;
typedef struct IpList* IpAddress;



void DieWithError(char *errorMessage);  /* Error handling function */


element findIP(int clntSocket, char* receivedMessage, element domain) {
	int tempType, foundinList = 0;
	char tempDomainBuffer[50];
	char *stringToClient = (char *)malloc(200);

	memset(stringToClient, 0, strlen(stringToClient));
	memset(tempDomainBuffer, 0, strlen(tempDomainBuffer));
	element node = domain;
	printf("%s\n", tempDomainBuffer);
	sscanf(receivedMessage, "%d %s", &tempType, tempDomainBuffer);

	while (node != NULL) {

		if (strcmp(tempDomainBuffer, node->domainName) == 0) {
			node->count++;
			foundinList++; /**/
			strcat(stringToClient, node->IP);
			strcat(stringToClient, "  ");
			
			//send(clntSocket, node->IP, strlen(node->IP) + 1, 0);
		}

		node = node->next;
	}
	if (foundinList == 0) {
		struct hostent *he;
		if ((he = gethostbyname(tempDomainBuffer)) != NULL) {

			struct in_addr **addr_list;
			addr_list = (struct in_addr **)he->h_addr_list;
			for (int i = 0; addr_list[i] != NULL; i++) {

				if (domain == NULL) {
					element new;
					do {
						new = malloc(sizeof(struct domainIpMap));
					} while (new == NULL);

					strcpy(new->domainName, tempDomainBuffer);
					strcpy(new->IP, inet_ntoa(*addr_list[i]));
					new->next = NULL;
					new->count = 0;

					domain = new;
				} else {

					element new, current, previous;
					do {
					new = malloc(sizeof(struct domainIpMap));
					} while (new == NULL);

					strcpy(new->domainName, tempDomainBuffer);
					strcpy(new->IP, inet_ntoa(*addr_list[i]));
					new->next = NULL;
					new->count = 0;
					current = domain;

					while (current != NULL) {
					previous = current;
					current = current->next;
					}

					previous->next = new;
				}
				node = domain;
				foundinList++; /*counts the number of ip adress in gethosbyname*/
				strcat(stringToClient, inet_ntoa(*addr_list[i]));
				strcat(stringToClient, "  ");
			}

		}
		else {
			//printf("The domain does not exist\n");
			strcat(stringToClient,"The domain does not exist\n");
		}
		
	}
	//Sending the string to client
	send(clntSocket, stringToClient, strlen(stringToClient) , 0);
	free(stringToClient);

	node = domain;
	printf("The list is:\n");
	while (node != NULL) {
		
		printf("%s %d %s\n", node->domainName, node->count, node->IP);
		node = node->next;
	}
	return domain;
}

element addDomain(int clntSocket, char* receivedMessage, element domain) {
	int tempType, foundinList = 0;
	char tempIP[17];
	int flag;
	char tempDomainBuffer[50];
	char *stringToClient = (char *)malloc(100);

	memset(stringToClient, 0, strlen(stringToClient));
	memset(tempDomainBuffer, 0, strlen(tempDomainBuffer));

	for (int i = 0; i < sizeof(tempDomainBuffer); i++) {
		tempDomainBuffer[i] = '\0';
	}
	element node = domain;
	sscanf(receivedMessage, "%d %s %s", &tempType, tempDomainBuffer, tempIP);

	while (node != NULL) {

		if (strcmp(tempDomainBuffer, node->domainName) == 0) {
			if (strcmp(tempIP, node->IP) == 0) {
				foundinList++;
				break;
			}
		}

		node = node->next;
	}

	if (foundinList != 0) {
		char* message = "The IP already exists";
		//printf("%s\n", message);
		flag = 0;
		strcat(stringToClient, "IP address : ");
		strcat(stringToClient, tempIP);
		strcat(stringToClient, " is already present");
	}
	else {

		if (domain == NULL) {
			element new;
			do {
			new = malloc(sizeof(struct domainIpMap));
		} while (new == NULL);

		strcpy(new->domainName, tempDomainBuffer);
		strcpy(new->IP, tempIP);
		new->next = NULL;
		new->count = 0;
		domain = new;
		} else {

			element new, current, previous;
			do {
				new = malloc(sizeof(struct domainIpMap));
			} while (new == NULL);

			strcpy(new->domainName, tempDomainBuffer);
			strcpy(new->IP, tempIP);
			new->next = NULL;
			new->count = 0;
			current = domain;

			while (current != NULL) {
				previous = current;
				current = current->next;
			}
			previous->next = new;
		}

		
		char* message = "The IP was added";
		//printf("%s\n", message);
		strcat(stringToClient, "IP address : ");
		strcat(stringToClient, tempIP);
		strcat(stringToClient, " was added");
	}


	
	node = domain;
	printf("The list is:\n");
	while (node != NULL) {
		printf("%s %d %s\n", node->domainName, node->count, node->IP);
		node = node->next;
	}

	send(clntSocket, stringToClient, strlen(stringToClient), 0);
	free(stringToClient);
	return domain;
}

element deleteDomain(int clntSocket, char* receivedMessage, element domain) {
	int tempType, foundinList = 0;
	char tempIP[17];
	char tempDomainBuffer[50];
	char *stringToClient = (char *)malloc(100);

	memset(stringToClient, 0, strlen(stringToClient));
	memset(tempDomainBuffer, 0, strlen(tempDomainBuffer));

	for (int i = 0; i < sizeof(tempDomainBuffer); i++) {
		tempDomainBuffer[i] = '\0';
	}
	element node = domain, previous, temp;
	sscanf(receivedMessage, "%d %s %s", &tempType, tempDomainBuffer, tempIP);

	while (node != NULL && node->next != NULL) {

		if (strcmp(tempDomainBuffer, node->domainName) == 0) {
			if (node == domain) {
				domain = node->next;
				previous = node->next;
				temp = node;
				node = node->next;
				free(temp);
			}
			else {
				previous->next = node->next;
				temp = node;
				node = node->next;
				free(temp);
			}
			foundinList++;
		}
		else {
			previous = node;
			node = node->next;
		}
		if(node->next == NULL) {
			break;
		}
	}

	char* message;
	if (foundinList == 0) {
		message = "The domain to be deleted does not exist";
		//printf("%s\n", message);
		strcat(stringToClient, "The domain to be deleted : ");
		strcat(stringToClient, tempDomainBuffer);
		strcat(stringToClient, " does not exist");
		
	}
	else {
		message = "The domain was deleted";
		//printf("%s\n", message);
		strcat(stringToClient, "The domain:");
		strcat(stringToClient, tempDomainBuffer);
		strcat(stringToClient, " was deleted");
	}

	send(clntSocket, stringToClient, strlen(stringToClient), 0);
	free(stringToClient);
	return domain;
	
}

void mostRequestedDomain(int clntSocket, element domain) {
	int mostReq = 0, elemPre = 0;
	element node = domain;
	char tempDomainBuffer[50];
	char *stringToClient = (char *)malloc(100);

	memset(stringToClient, 0, strlen(stringToClient));
	memset(tempDomainBuffer, 0, strlen(tempDomainBuffer));

	while (node != NULL) {
		if (node == domain) {
			mostReq = node->count;
			node = node->next;
			strcpy(tempDomainBuffer, node->domainName);
			elemPre++;
		}
		else {
			if (node->count >= mostReq) {
				mostReq = node->count;
				strcpy(tempDomainBuffer, node->domainName);
				node = node->next;
				elemPre++;
			}
			else {
				node = node->next;
			}
		}
	}
	if (elemPre != 0) {
		char mostReqStr[3];
		sprintf(mostReqStr,"%d", mostReq);
		strcat(stringToClient, "The most requested domain is ");
		strcat(stringToClient, tempDomainBuffer);
		strcat(stringToClient, "\nNumber of times requested:");
		strcat(stringToClient,mostReqStr);
	} else {
		char mostReqStr[3];
		char* temp = "non-existent";
		sprintf(mostReqStr,"%d", mostReq);
		strcat(stringToClient, "The most requested domain is ");
		strcat(stringToClient, temp);
		strcat(stringToClient, "\nNumber of times requested:");
		strcat(stringToClient,mostReqStr);
	}
	
	send(clntSocket, stringToClient, strlen(stringToClient), 0);
	free(stringToClient);
}

void leastRequestedDomain(int clntSocket, element domain) {
	int leastReq = 0, elemPre = 0;
	element node = domain;
	char *stringToClient = (char *)malloc(100);
		char tempDomainBuffer[50];
	memset(stringToClient, 0, strlen(stringToClient));
	memset(tempDomainBuffer, 0, strlen(tempDomainBuffer));

	while (node != NULL) {
		if (node == domain) {
			leastReq = node->count;
			strcpy(tempDomainBuffer, node->domainName);
			node = node->next;
		}
		else {
			if (node->count <= leastReq) {
				leastReq = node->count;
				strcpy(tempDomainBuffer, node->domainName);
				node = node->next;
			}
			else {
				node = node->next;
			}
		}
	}

	if (elemPre != 0) {
		char leastReqStr[3];
		sprintf(leastReqStr,"%d", leastReq);
		strcat(stringToClient, "The least requested domain is ");
		strcat(stringToClient, tempDomainBuffer);
		strcat(stringToClient, "\nNumber of times requested:");
		strcat(stringToClient,leastReqStr);
	} else {
		char leastReqStr[3];
		char* temp = "non-existent";
		sprintf(leastReqStr,"%d", leastReq);
		strcat(stringToClient, "The leastReq requested domain is ");
		strcat(stringToClient, temp);
		strcat(stringToClient, "\nNumber of times requested:");
		strcat(stringToClient,leastReqStr);
	}

	send(clntSocket, stringToClient, strlen(stringToClient) + 1, 0);
	free(stringToClient);
}

void shutDown(int clntSocket, element domain, char* fileName, char* echoBuffer) {
	int temp;
	char password[20];

	sscanf(echoBuffer, "%d %s", &temp, password);
	char *stringToClient = (char *)malloc(100);

	memset(stringToClient, 0, strlen(stringToClient));

	//printf("%s\n", password);

	if (strcmp(password, PASSWORD) == 0) {

		FILE* fileWrite;
		fileWrite = fopen(fileName, "w");
		element node = domain;
		while (node != NULL) {

			element temp;
			fprintf(fileWrite, "%s %d %s\n", node->domainName, node->count, node->IP);
			temp = node;
			node = node->next;
			free(temp);
		}
		fclose(fileWrite);

		char* message = "The server is shutting down";
		//printf("%s\n", message);
		strcat(stringToClient, message);
		send(clntSocket, stringToClient, strlen(stringToClient) + 1, 0);
		free(stringToClient);
		exit(0);
	}
	else {
		char* message = "Incorrect password";
		//printf("%s\n", message);
		strcat(stringToClient, message);
		send(clntSocket, stringToClient, strlen(stringToClient) + 1, 0);
		free(stringToClient);
	}

	
}

element HandleTCPClient(int clntSocket, int timeGap, element domain, IpAddress ip, char* fileName)
{
	char echoBuffer[RCVBUFSIZE];        /* Buffer for echo string */
	int recvMsgSize;                    /* Size of received message */
	int messageType;
	FILE* fileWrite;
	char *returnString;

	/* Receive message from client */
	/*if ()*/
	if ((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
		DieWithError("recv() failed");
	}
	sscanf(echoBuffer, "%d ", &messageType);
	switch (messageType) {
	case 1:
		domain = findIP(clntSocket, echoBuffer, domain);
		break;
	case 2:
		domain = addDomain(clntSocket, echoBuffer, domain);
		break;
	case 3:
		domain = deleteDomain(clntSocket, echoBuffer, domain);
		break;
	case 4:
		mostRequestedDomain(clntSocket, domain);
		break;
	case 5:
		leastRequestedDomain(clntSocket, domain);
		break;
	case 6:
		shutDown(clntSocket, domain, fileName, echoBuffer);
		break;
	default:
		printf("Default Error ");
	}
  	memset(echoBuffer, 0, RCVBUFSIZE);
	close(clntSocket);
	return domain;
}
