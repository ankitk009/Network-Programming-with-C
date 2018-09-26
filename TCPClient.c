#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <ctype.h>       /* for isdigit() */

#define RCVBUFSIZE 100   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */
int isValidIpAddress(char *ipAddress);  /*Validating IP address*/

int main(int argc, char *argv[])
{
    int sock;                           /* Socket descriptor */
    struct sockaddr_in ServAddr;        /*  server address */
    unsigned short ServPort;            /*  server port */
    char *servIP;                       /* Server IP address (dotted quad) */
    char *requestCodeStr;               /* Request Code send to server */
    int requestCode;                    /* Type of request code as an input argument*/   
    char *WebAddress;                   /* Web address String to send to server */
    char *IPAddress;                    /* IP address String to send to server */
    char *SecurityCode;                 /* Input security code*/
    char *sendToServerString;           /*String to sent to Server */
    char Buffer[RCVBUFSIZE];            /* Buffer for  string */
    unsigned int sendToServerStringLen; /* Length of string to  */
    int bytesRcvd, totalBytesRcvd;      /* Bytes read in single recv() and total bytes read */

    if ((argc < 3) || (argc > 6))       /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port> <Request Code> <Extra:WebAddress> <Extra:IP address>\n",
               argv[0]);
       exit(1);
    }

    servIP = argv[1];                   /* First arg: server IP address (dotted quad) */
	ServPort = atoi(argv[2]);           /*Second arg: Server port number */
    requestCodeStr = argv[3];           /* Third arg: requestCode to send to server */
    requestCode = atoi(requestCodeStr); /*Request code in number for switch case*/
        
        switch(requestCode)
        {
            case 1:
            if (argc > 5) {
                DieWithError("Incorrect number of arguments provided");
            }

            WebAddress = argv[4];
            //For adding one \t character and one null termination character
            sendToServerString = (char*) malloc(strlen(requestCodeStr) + strlen(WebAddress) + 2);

	        if (sendToServerString == NULL) {
	           break;
            }
            //Concatenating string to send to server
	        strcpy(sendToServerString, requestCodeStr);
	        strcat(sendToServerString, "\t");                  /*Adding tab character (refer to the report)*/
	        strcat(sendToServerString, WebAddress);
            strcat(sendToServerString,"\0");                   /*Terminating the string*/
            break;

            
            case 2:
            if (argc > 6){
                DieWithError("Incorrect number of arguments provided");
            }

            if (isValidIpAddress(argv[5])!=1)
                DieWithError("Enter valid IP Address ");

            WebAddress = argv[4];
            IPAddress = argv[5];
	        sendToServerString= (char*) malloc(strlen(requestCodeStr) + strlen(WebAddress) + strlen(IPAddress) + 3);
	        strcpy(sendToServerString,requestCodeStr);
	        strcat(sendToServerString, "\t");
            strcat(sendToServerString, WebAddress);
	        strcat(sendToServerString, "\t");
            strcat(sendToServerString, IPAddress);
            strcat(sendToServerString, "\0");
            break;            

            case 3:
            if (argc > 5) {
                    DieWithError("Incorrect number of arguments provided");
                }

            WebAddress = argv[4];
	        sendToServerString = (char*) malloc(strlen(requestCodeStr) + strlen(WebAddress) + 2);
	        strcpy(sendToServerString, requestCodeStr);
            strcat(sendToServerString, "\t");
            strcat(sendToServerString, WebAddress);
            strcat(sendToServerString,"\0");
            break;
            

            case 4:
            if (argc>4) {
                DieWithError("Incorrect number of arguments provided");
            }
            
            sendToServerString=(char*) malloc(strlen(requestCodeStr) + 1);
	        strcpy(sendToServerString, requestCodeStr);
            strcat(sendToServerString,"\0");
            break;

            case 5:
            if (argc>4) {
                DieWithError("Incorrect number of arguments provided");
            }
            sendToServerString = (char*) malloc(strlen(requestCodeStr) + 1);
	        strcpy(sendToServerString,requestCodeStr);
            strcat(sendToServerString,"\0");
	        break;
            
            case 6:
            if (argc>5) {
                DieWithError("Incorrect number of arguments provided");
            }
            
            SecurityCode=argv[4];
	        sendToServerString = (char*) malloc(strlen(requestCodeStr) + strlen(SecurityCode) + 2);
	        strcpy(sendToServerString,requestCodeStr);
	        strcat(sendToServerString,"\t");
	        strcat(sendToServerString,SecurityCode);
            strcat(sendToServerString,"\0");
            break;
            
            Default :
            DieWithError("Invalid request Code provided");
        }
        
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {         /* Create a reliable, stream socket using TCP */
        DieWithError("Failed to create socket");
    }


    /* Construct the server address structure */
    memset(&ServAddr, 0, sizeof(ServAddr));         /* Zero out structure */
    ServAddr.sin_family      = AF_INET;             /* Internet address family */
    ServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    ServAddr.sin_port        = htons(ServPort);     /* Server port */


    /* Establish the connection to the  server */
    if (connect(sock, (struct sockaddr *) &ServAddr, sizeof(ServAddr)) < 0) {
        DieWithError("connect() failed");
    }
    sendToServerStringLen = strlen(sendToServerString);          /* Determine length of string to send */
	
    /* Send the string to the server */
    if (send(sock, sendToServerString, sendToServerStringLen + 1, 0) != sendToServerStringLen + 1) {
        DieWithError("send() sent a different number of bytes than expected");
    }

    /* Receive the same string back from the server */
    totalBytesRcvd = 0;
    printf("Response Received:");                /* Setup to print the  string */
    memset(Buffer,0,RCVBUFSIZE-1);
    while ((bytesRcvd = recv(sock, Buffer, RCVBUFSIZE - 1, 0)) != 0)    /*Keep on receiving until returns 0 */ 
    {
        totalBytesRcvd += bytesRcvd;   /* Keep tally of total bytes */
        Buffer[bytesRcvd] = '\0';  /* Terminate the string! */
        //printf("%s", Buffer);      /* Print the  buffer */
    }

    printf("\n");                      /* Print a final linefeed */
    free(sendToServerString);          /* Free the dynamically created string */
    close(sock);                       /* Close the socket */
    exit(0);
}

    int isValidIpAddress(char *ipAddress)
    {
        struct sockaddr_in sa;
        int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));     /*inet_pton returns 1 if successful else -1 or 0 */
        return result;
    }
