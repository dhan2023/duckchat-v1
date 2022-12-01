#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include "duckchat.h"


/*Users*/
struct user {
    char userName[USERNAME_MAX];
    void* head;    //dependency loop w channelNodeHead; type cast to channelNode in code to solve
    struct sockaddr_in* userIPsock;
}; 


/*User Node*/
struct userNode{
    struct user* userInfo;
    struct userNode* next;
};


/*Channels*/
struct channel {
    char channelName[CHANNEL_MAX];
    struct userNode* userHead;
};

/*Channel Node*/
struct channelNode{
    struct channel* chanInfo;
    struct channelNode* next;
};

/*Channel linked list current head; declare in server but set head to NULL if empty*/
struct channelNodeHead{
    struct channelNode* head;
};

/*(Logged in) User linked list current head; declare in server but set head to NULL if empty*/
struct userNodeHead{
    struct userNode* head; 
};



/*Find Channel*/
struct channel* findChan(struct channelNodeHead* head, char* name){
    struct channelNode *currentChan = (struct channelNode*)malloc(sizeof(struct channelNode));
    currentChan = head->head;
    int term = 1;

    while(term){
        if(strcmp(currentChan->chanInfo->channelName, name) == 0){
            return currentChan->chanInfo;
        }
        else if(currentChan->next != NULL){
            currentChan = currentChan->next;
        }
        else{
            term = 0;
        }
    }
    return NULL;

}

/*Add new channel to channel list; append to front/make new channel the head*/
//Problem: can currently create duplicate channels
void addChan(struct channelNodeHead *head, char* name){                             //args are current linked list head and name of new channel
    
    struct channel *newChan = (struct channel*)malloc(sizeof(struct channel));      //create new channel
    strcpy(newChan->channelName, name);                                              //assign name to new channel
    newChan->userHead = NULL;                                                       //no users in new channel; add in addUser

    struct channelNode *chanNode = (struct channelNode*)malloc(sizeof(struct channelNode));         //create new channel's node in linked list
    chanNode->chanInfo = newChan;                                                                   //assign new channel to its new node
    if(head->head == NULL){                                                                         //if there is no channels at all in linkedd list
        chanNode->next = NULL;                                                                      //denote no next channel
        head->head = chanNode;                                                                      //assign new channel as head/only channel
    }
    else{                                                                                           //if there are existing channels already
        chanNode->next = head->head;                                                                //make the current head the new channel's next                                           
        head->head = chanNode;                                                                      //make the new channel the new head
    }
}

/*VOID DELETECHANNEL*/
void deleteChan(struct channel* channel, struct channelNodeHead* head){
    struct channelNode *prevChan = (struct channelNode*)malloc(sizeof(struct channelNode));
    struct channelNode *currentChan = (struct channelNode*)malloc(sizeof(struct channelNode));
    currentChan = head->head;

    int term = 1;

    while(term){
        if(currentChan->chanInfo->channelName == channel->channelName){
            prevChan->next = currentChan->next;
            // free(currentChan);
            term = 0;
        }
        else if(currentChan->next != NULL){
            prevChan = currentChan;
            currentChan = currentChan->next;
        }
        else{
            printf("Error: Channel does not exist");
            term = 0;
        }
    }
}


/*Function takes client-given user address and a linked list of users and returns the user's struct*/
struct user* findUser(struct sockaddr_in* userAddr, struct userNode* head){
    char s[100];
    inet_ntop(AF_INET, &(((struct sockaddr_in *)userAddr)->sin_addr), s, 1024);
    uint16_t port = htons(userAddr->sin_port);
    int term = 1;

    struct userNode *currentUser = (struct userNode*)malloc(sizeof(struct userNode));
    currentUser = head;
    if(head == NULL){
        return NULL;
    }

    while(term){
        char d[100];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)currentUser->userInfo->userIPsock)->sin_addr), d, 1024);
        uint16_t portNode = htons(currentUser->userInfo->userIPsock->sin_port);
        // printf("ip:%s, port:%d, ip:%s, port:%d \n", s, port, d, portNode);
        if(((port) == (portNode)) && (strcmp(s, d) == 0)){
            // printf("name: %s\n", currentUser->userInfo->userName);
            return currentUser->userInfo;
        }
        else if(currentUser->next != NULL){
            currentUser = currentUser->next;
        }
        else{
            printf("Error: User not found\n");
            term = 0;
        }
    }
    return NULL;
}


//FIX???????? TAKES NAME AND USER ADDRESS BUT UNSURE IF CLIENT SENDS THOSE
/*Add new user to list of logged in users*/
//PROBLEM: can currently add duplicate users to channel
void addUser(struct userNodeHead* head, char* name, struct sockaddr_in* userStuff){

    if(findUser(userStuff, head->head) == NULL){
        struct user *newUser = (struct user*)malloc(sizeof(struct user));
        strcpy(newUser->userName, name);
        newUser->head = malloc(sizeof(struct channelNode));
        newUser->userIPsock = userStuff;

        struct userNode *userNode = (struct userNode*)malloc(sizeof(struct userNode));
        userNode->userInfo = newUser;
        if(head->head == NULL){                                                                         //if there is no channels at all in linkedd list
            userNode->next = NULL;                                                                      //denote no next channel
            head->head = userNode;                                                                      //assign new channel as head/only channel
        }
        else{                
            userNode->next = head->head;                                                          
            head->head = userNode;                                                      
        }
    }
}

void rmvUser(struct userNodeHead* head, struct sockaddr_in* userStuff){
    struct userNode *prevUser = NULL;
    struct userNode *currentUser = head->head;
    int term = 1;
    char d[100];
    inet_ntop(AF_INET, &(((struct sockaddr_in *)userStuff)->sin_addr), d, 1024);
    uint16_t givenPort = htons(userStuff->sin_port);

    while(term){
        char s[100];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)currentUser->userInfo->userIPsock)->sin_addr), s, 1024);
        uint16_t port = htons(currentUser->userInfo->userIPsock->sin_port);
        if(((port) == (givenPort)) && (strcmp(s, d) == 0)){
            if(prevUser == NULL){
                head->head = currentUser->next;
                // free(currentUser);
            }
            else{
                prevUser->next = currentUser->next;
                // free(currentUser);
            }
            term = 0;
        }
        else if(currentUser->next != NULL){
            prevUser = currentUser;
            currentUser = currentUser->next;
        }
        else{
            printf("user not in");
            term = 0;
        }
    }

}

void addUtoC(struct user* user, struct channel* channel){
    struct userNode *newUser = (struct userNode*)malloc(sizeof(struct userNode));
    newUser->userInfo = user;
    
    if(findUser(user->userIPsock, channel->userHead) == NULL){
        if(channel->userHead == NULL){
            newUser->next = NULL;
            channel->userHead = newUser;
        }
        else{
            newUser->next = channel->userHead;
            channel->userHead = newUser;
        }
    }

}


/*FREE STRUCT STUFF FUNCTiONS HERE????*/

//add channel to user channel list
void addCtoU(struct user* user, struct channel* channel){

    struct channelNode *channelAdd = (struct channelNode*)malloc(sizeof(struct channelNode));
    channelAdd->chanInfo = channel;

    if(user->head == NULL){
        user->head = channelAdd;
    }
    else{
        channelAdd->next = user->head;
        user->head = channelAdd;
    }

}

//remove channel from user channel list
void rmvCfromU(struct user* user, char* name){
    struct channelNode *prev = NULL;
    struct channelNode *current = (struct channelNode*)user->head;
    int term = 1;
    while(term){
        if(strcmp(current->chanInfo->channelName, name) == 0){
            if(prev != NULL){
                prev->next = current->next;
                // free(current);
            }
            else{
                user->head = current->next;
                // free(current);
            }
            term = 0;
        }
        else if(current->next != NULL){
            prev = current;
            current = current->next;
        }
        else{
            term = 0;
        }
    }
}


//free prev and current below at end of func
void rmvUfromC(struct sockaddr_in* userAddr, struct channel* channel, struct channelNodeHead* channelHead){
    struct userNode *prevUser = NULL;
    struct userNode *currentUser = channel->userHead;

    char s[100];
    int term = 1;
    inet_ntop(AF_INET, &(((struct sockaddr_in *)userAddr)->sin_addr), s, 1024);
    uint16_t givenPort = htons(userAddr->sin_port);

    while(term){
        char d[100];
        inet_ntop(AF_INET, &(((struct sockaddr_in *)currentUser->userInfo->userIPsock)->sin_addr), d, 1024);
        uint16_t port = htons(currentUser->userInfo->userIPsock->sin_port);
        if(((port) == (givenPort)) && (strcmp(s, d) == 0)){
            if(currentUser == channel->userHead){
                if(channel->userHead->next != NULL){
                    channel->userHead = currentUser->next;
                }
                // free(currentUser);
            }
            else{
                prevUser->next = currentUser->next;
                // free(currentUser);
            }
            // free(currentUser);
            term = 0;
        }
        else if(currentUser->next != NULL){
            prevUser = currentUser;
            currentUser = currentUser->next;
        }
        /*else = end of linked list, print user not exist*/
        else{
            printf("Error: User does not exist in this channel\n");
            term = 0;
        }
    }
    if(channel->userHead == NULL){
        printf("No more users in channel; Deleting...\n");
        deleteChan(channel, channelHead);
    }
}

