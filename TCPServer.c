#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <time.h>

#define MAXPENDING 5    /* Maximum outstanding connection requests */
#define FILEBUFFERLENGTH 150 /*Buffer for file*/

struct domainIpMap{
    char domainName[50];
    char IP[17];
    int count;
    struct domainIpMap* next;
};

struct IpList {
    char IP[17];
    time_t lastRequest;
    struct IpList* next;
};

typedef struct domainIpMap* element;
typedef struct IpList* IpAddress;

void DieWithError(char *errorMessage);  /* Error handling function */
element setupDomainInfo(char *fileName);  /* Setup the linked list from datafile*/
element HandleTCPClient(int clntSocket, int timegap, element head, IpAddress ip, char* fileName);   /* TCP client handling function */
char *stringToServer;


int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoServAddr; /* Local address */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned short echoServPort;     /* Server port */
    unsigned int clntLen;            /* Length of client address data structure */
    char *fileName;
    int timeGap;
    if (argc !=4 )     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port> <Filename> <Time Gap>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */
    fileName = argv[2];
    timeGap = atoi(argv[3]);

    /* Create socket for incoming connections */
    if ((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                /* Internet address family */
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    echoServAddr.sin_port = htons(echoServPort);      /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    element listHead = setupDomainInfo(fileName);
    IpAddress starter = NULL;
    int timeLimitError = 0, existsInList = 0; 

    for (;;) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(echoClntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr, 
                               &clntLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */
        IpAddress node = starter;

        while(node != NULL) {
            if(strcmp(inet_ntoa(echoClntAddr.sin_addr), node->IP) == 0) {
                if ((time(NULL) - node->lastRequest) <= timeGap) {
                    existsInList++;
                    timeLimitError++;
                    break;
                } else {
                    node->lastRequest = time(NULL);
                    existsInList++;
                    break;
                }
            }
            node = node->next;
        }

        if(existsInList == 0) {
            IpAddress new = malloc(sizeof(struct IpList));
            strcpy(new->IP, inet_ntoa(echoClntAddr.sin_addr));
            new->lastRequest = time(NULL);
            if (starter == NULL) {
                new->next = NULL;
                starter = new;
            } else {
                new->next = starter;
                starter = new;
            }
        } else {
            existsInList = 0;
        }

        if (timeLimitError == 0) {
            printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));        
            listHead = HandleTCPClient(clntSock, timeGap, listHead, starter, fileName);
        } else {
            stringToServer=(char *)malloc(40);
            memset(stringToServer,'\0',strlen(stringToServer));
            strcat(stringToServer,"Please retry after ");
            strcat(stringToServer,argv[3]);
            strcat(stringToServer,"seconds");
            send(clntSock, stringToServer, strlen(stringToServer), 0);
            free(stringToServer);
            close(clntSock);
            timeLimitError = 0;
        }
    }
    /* NOT REACHED */
}


element setupDomainInfo(char* fileName) {
    element current, head;
    char fileBuffer[FILEBUFFERLENGTH];

    head = current = NULL;
    FILE* fileRead;
    if(!fopen(fileName, "r")) {
        fileRead = fopen(fileName, "w");
        fclose(fileRead);
    }
    
    fileRead = fopen(fileName, "r");

    while(fgets(fileBuffer, FILEBUFFERLENGTH, fileRead)) {
        element node = malloc(sizeof(struct domainIpMap));
        char tempDomain[50], tempIp[16];
        int tempCount;
        sscanf(fileBuffer, "%s %d %s", node->domainName, &node->count, node->IP);
        if (head == NULL) {
            current = head = node;
        } else {
            current = current->next = node;
        }
    }
    fclose(fileRead);
    return head;
}

