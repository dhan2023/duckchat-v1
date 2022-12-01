#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <poll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include "duckchat.h"
  
#define PORT 5000
#define MAXLINE 1000

/*Buffer to store received packet info*/
char buffer[200];

/*Socket declaration*/
int sockfd;

/*Server socket address*/
struct sockaddr_in servaddr;

int term = 1;

int main(int argc, char *argv[]){   

    /*Argument Checking*/
    if(argc != 4){
        printf("Error: Invalid number of arguments\n");
        exit(0);
    }

    int pt = atoi(argv[2]);
    char* username = argv[3];

    /*Clear servaddr*/
    bzero(&servaddr, sizeof(servaddr)); 
    // servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");  //ip addess
    // servaddr.sin_port = htons(PORT);    //port
    // servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = inet_addr(argv[1]);  //ip addess
    servaddr.sin_port = htons(pt);    //port
    servaddr.sin_family = AF_INET;

    /*Create datagram socket*/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    // /*Connect to server*/
    // if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    // {
    //     printf("\n Error : Connect Failed \n");
    //     exit(0);
    // }
  
    char* activeChan = malloc(sizeof(CHANNEL_MAX));
    char msg[100];
    char *tok_msg;

/*PROTOCOL 0: LOGIN*/
    /*INITIAL LOGIN*/
    struct request_login lgin;
    memset(&lgin, 0, sizeof(lgin));
    lgin.req_type = REQ_LOGIN;
    strcpy(lgin.req_username, username);
    sendto(sockfd, (void *) &lgin, sizeof(lgin), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    /*INITIAL JOIN COMMON*/
    struct request_join joinChan;
    memset(&joinChan, 0, sizeof(joinChan));
    joinChan.req_type = REQ_JOIN;
    strcpy(joinChan.req_channel, "Common");
    strcpy(activeChan, "Common");
    sendto(sockfd, (void *) &joinChan, sizeof(joinChan), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
////////////////


/*OTHER STRUCTS/DECLARATIONS*/

    struct request_leave leaveChan;
    memset(&leaveChan, 0, sizeof(leaveChan));
    leaveChan.req_type = REQ_LEAVE;

    struct request_list chanList;
    memset(&chanList, 0, sizeof(chanList));
    chanList.req_type = REQ_LIST;

    struct request_who userList;
    memset(&userList, 0, sizeof(userList));
    userList.req_type = REQ_WHO;

    struct request_say text;
    memset(&text, 0, sizeof(text)); 
    text.req_type = REQ_SAY; 


    struct pollfd fds[2];
        fds[0].fd = sockfd;
        fds[0].events = POLLIN;
        fds[1].fd = 0;
        fds[1].events = POLLIN;

    struct text* received;

///////////////////////////////

/*TEXT PARSING LOOP*/
    setbuf(stdout, NULL);
    // printf(">");
    while(term){

        /*Poll for input*/
        poll(fds, 2, 0);

        if(fds[1].revents != 0){
            // printf(">");
            fgets(msg, sizeof(msg), stdin);
            tok_msg = strtok(msg, " ");
            // printf("%s", tok_msg); 


            /*PROTOCOL 1: LOGOUT*/
            if(strcmp(tok_msg, "/exit\n") == 0){
                struct request_logout dip;
                memset(&dip, 0, sizeof(dip));
                dip.req_type = REQ_LOGOUT;
                sendto(sockfd, (void *) &dip, sizeof(dip), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
                term = 0;
                // close(sockfd);        
            }


            /*PROTOCOL 2: JOIN NEW CHANNEL*/
            else if(strcmp(tok_msg, "/join") == 0){
                tok_msg = strtok(NULL, " ");
                tok_msg[strlen(tok_msg) - 1] = '\0';  
                strcpy(joinChan.req_channel, tok_msg);
                strcpy(activeChan, tok_msg);
                sendto(sockfd, (void *) &joinChan, sizeof(joinChan), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
            }


            /*PROTOCOL 3: LEAVE CHANNEL*/
            else if(strcmp(tok_msg, "/leave") == 0){
                tok_msg = strtok(NULL, " ");
                tok_msg[strlen(tok_msg) - 1] = '\0';  
                strcpy(leaveChan.req_channel, tok_msg);
                sendto(sockfd, (void *) &leaveChan, sizeof(leaveChan), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));


            }

            /*PROTOCOL 5: LIST OF CHANNELS*/
            else if(strcmp(tok_msg, "/list\n") == 0){
                sendto(sockfd, (void *) &chanList, sizeof(chanList), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
            }


            /*PROTOCOL 6: LIST OF CHANNEL USERS*/
            else if(strcmp(tok_msg, "/who") == 0){
                tok_msg = strtok(NULL, " ");
                tok_msg[strlen(tok_msg) - 1] = '\0';  
                strcpy(userList.req_channel, tok_msg);
                sendto(sockfd, (void *) &userList, sizeof(userList), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
            }


            /*SWITCH*/
            else if(strcmp(tok_msg, "/switch") == 0){
                tok_msg = strtok(NULL, " ");
                tok_msg[strlen(tok_msg) - 1] = '\0';  
                strcpy(activeChan, tok_msg);
            }


            /*PROTOCOL 4: SAY*/
            else if(tok_msg[0] == '/'){
                printf("*Unknown command\n");
            }

            else{
                // printf("oop\n");
                // msg[strlen(msg) - 1] = '\0';  
                strcpy(text.req_text, msg);
                strcpy(text.req_channel, activeChan);
        

                // sendto(sockfd, tok_msg, MAXLINE, 0, (struct sockaddr*)NULL, sizeof(servaddr));
                // sendto(sockfd, message, MAXLINE, 0, (struct sockaddr*)NULL, sizeof(servaddr));
                sendto(sockfd, (void *) &text, sizeof(text), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));

                // waiting for response
                // recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL);
                // puts(buffer);
            }
            // printf(">");
        }
        
        /*RECEIVE FROM SERVER*/
        if(fds[0].revents != 0){
            recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&servaddr, NULL);
            received = (struct text*) buffer;

            if(received->txt_type == TXT_SAY){
                struct text_say* receivedPrint = (struct text_say*) buffer;
                printf("[%s][%s] %s", receivedPrint->txt_channel, receivedPrint->txt_username, receivedPrint->txt_text);
            }

            if(received->txt_type == TXT_LIST){
                struct text_list* receivedPrint = (struct text_list*) buffer;
                printf("Existing Channels:\n");
                for(int i = 0; i < receivedPrint->txt_nchannels; i++){
                    printf(" %s\n", receivedPrint->txt_channels[i].ch_channel);
                }
            }

            if(received->txt_type == TXT_WHO){
                struct text_who* receivedPrint = (struct text_who*) buffer;
                printf("Users on channel %s:\n", receivedPrint->txt_channel);
                for(int i = 0; i < receivedPrint->txt_nusernames; i++){
                    printf(" %s\n", receivedPrint->txt_users[i].us_username);
                }
            }

            if(received->txt_type == TXT_ERROR){
                struct text_error* receivedPrint = (struct text_error*) buffer;
                printf("Error: %s\n", receivedPrint->txt_error);
            }

            // puts(buffer);
        }
    }
    free(activeChan);
    close(sockfd);
}
