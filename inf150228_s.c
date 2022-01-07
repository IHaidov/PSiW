#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define SERVERKEY 55555
#define PERMS 0644
#define MSGTEXTSIZE 256

#define true 1
#define false 0

const enum messageType {
    UNIDENTIFIED,
    AUTH,
    OK,
    BAD,
    EXISTS,
    REGISTERED,
    BADPASSWORD,
    TOOMANYCLIENTS,
    NEWTEXTMSG
};

struct message
{
    long int mtype;          //Это используется для внутренних функций считывания. Этим числом мы не пользуемся.
    int author;
    int type;
    char text[MSGTEXTSIZE];
};

void cleanMessage(struct message *msg){
    msg->type = UNIDENTIFIED;
    msg->author = -1;
    memset(msg->text, 0, sizeof(msg->text));
}

const unsigned long msgSIZE = sizeof(int) * 2 + MSGTEXTSIZE;
pthread_mutex_t lock;


int main()
{
    
}