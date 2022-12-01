#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "duckchat.h"
#include "channel.h"

#define PORT 5000
#define MAXLINE 1000

// Driver code
int main(int argc, char *argv[]){   

    /*Argument Checking*/
    if(argc != 3){
        printf("Error: Invalid number of arguments\n");
        exit(0);
    }
    
    int term = 1;
    char buffer[200];
    char *message = "matt\n";
    int listenfd;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr;
    bzero(&servaddr, sizeof(servaddr));
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s;
  
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
        Try each address until we successfully bind(2).
        If socket(2) (or bind(2)) fails, we (close the socket
        and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        listenfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (listenfd == -1)
            continue;

        if (bind(listenfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                  /* Success */

        close(listenfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }


    // // Create a UDP Socket
    // listenfd = socket(AF_INET, SOCK_DGRAM, 0);        
    // servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // // servaddr.sin_port = htons(PORT);
    // // servaddr.sin_family = AF_INET; 

    // // servaddr.sin_addr.s_addr = htonl(inet_addr(argv[1]));
    // servaddr.sin_port = htons(pt);
    // servaddr.sin_family = AF_INET; 
   
    // // bind server address to socket descriptor
    // bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    struct channelNodeHead *channelLL = (struct channelNodeHead*)malloc(sizeof(struct channelNodeHead));
    struct userNodeHead *userLL = (struct userNodeHead*)malloc(sizeof(struct userNodeHead));
    userLL->head = NULL;
    channelLL->head = NULL;

    printf("hehe\n");
    while(term){   
        //receive the datagram
        len = sizeof(cliaddr);
        recvfrom(listenfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr,&len); //receive message from server
        // printf("got login\n");

        //buffer currently of unknown type

        //struct sockaddr_in* userSock = (struct sockaddr_in *) cliaddr; //attempt to change cliaddr to correct type for addUser

        struct request* req = (struct request*) buffer;

        if(req->req_type == REQ_LOGIN){
            struct request_login* logreq = (struct request_login*) buffer;
            addUser(userLL, logreq->req_username, &cliaddr);
            printf("server: %s logs in\n", logreq->req_username);
            
        }//

        if(req->req_type == REQ_LOGOUT){
            // struct request_logout* logreq = (struct request_logout*) buffer;
            struct user* theUser = findUser(&cliaddr, userLL->head);
            
            struct channelNode* userChannels = (struct channelNode*) theUser->head;

            int term = 1;
            while(term){
                if(userChannels->next != NULL){
                    rmvUfromC(theUser->userIPsock, userChannels->chanInfo, channelLL);
                    rmvCfromU(theUser, userChannels->chanInfo->channelName);
                    userChannels = userChannels->next;
                }
                else{
                    rmvUfromC(theUser->userIPsock, userChannels->chanInfo, channelLL);
                    rmvCfromU(theUser, userChannels->chanInfo->channelName);
                    userChannels = userChannels->next;
                    term = 0;
                }
            }
            rmvUser(userLL, theUser->userIPsock);
            printf("server: %s logs out\n", theUser->userName);
        }

        if(req->req_type == REQ_JOIN){
            struct request_join* joinreq = (struct request_join*) buffer;
            struct user* theUser = findUser(&cliaddr, userLL->head);
            struct channel* theChannel;

            if(channelLL->head == NULL){
                addChan(channelLL, "Common");
            }
            else if(findChan(channelLL, joinreq->req_channel) == NULL){
                addChan(channelLL, joinreq->req_channel);
            }
            theChannel = findChan(channelLL, joinreq->req_channel);
            addUtoC(theUser, theChannel);
            addCtoU(theUser, theChannel);
            printf("server: %s joins channel %s\n", theUser->userName, theChannel->channelName);
            // printf("server: %s joins channel %s\n", joinreq->req_username);
        }

        if(req->req_type == REQ_LEAVE){
        
        }

        // if(req->req_type == REQ_SAY){
        
        // }

        // if(req->req_type == REQ_LIST){
        
        // }

        // if(req->req_type == REQ_WHO){
        
        // }


        // buffer[n] = '\0';
        puts(buffer);
                
        // send the response
        sendto(listenfd, message, MAXLINE, 0,
                (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }    
}





/*
RMV USER w LINKEDLIST

target = user you want to rmv
node = head
Loop:
    if node.data != target
        prev = node
        node = node.next
    else
        prev.next = node.next
        delete node

*/

