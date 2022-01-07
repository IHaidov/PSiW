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

const enum userAnswer { UNDEFINED, //бред
                        REGISTER,
                        LOGIN,      
                        TO_SEE_REGISTERED_USERS,
                        TO_SEE_SAVED_USERS,
                        TO_SAVE_USER_TO_ROOM,
                        TO_REMOVE_USER_FROM_ROOM,
                        TO_SEE_GLOBAL_CHAT,
                        //TO_SEND_PUBLIC_MESSAGE,
                        TO_SEND_PRIVATE_MESSAGE,
                        //TO_GET_PUBLIC_MESSAGE,
                        TO_SEE_PRIVATE_MESSAGE,
                        LOGOUT,
                        HEARTBEAT
                        };

enum userAnswer invokeMenu()
{
    int answer;

    inputNum(">>> Select feature:\n    1. Register\n    2. Login\n    3. See registered users in the room\n    4. See saved users in the room\n    5. Save user in the room\n    6.Remove user from the room\n    7. Global chat\n    8. Send private message\n    9. See Your messages\n    10. Log out", &answer);

    if (answer < 0 || answer >= MAXANSWER)
    {
        return UNDEFINED;
    }
    else
    {
        return answer;
    }
}

int main()
{
    int msqid;
    if (msqid = msgget(SERVERKEY, PERMS | IPC_CREAT) < 0)
    {
        perror("msgget error: ");
        exit(1);
    }
    int authentificated;
    authentificated = false;
    while (1)
    {
        userWants = invokeMenu();
        switch (userWants)
        {
            case REGISTER:{}
            case LOGIN:{}
            case TO_SEE_REGISTERED_USERS:{}
            case TO_SEE_SAVED_USERS:{}
            case TO_SAVE_USER_TO_ROOM:{}
            case TO_REMOVE_USER_FROM_ROOM:{}
            case TO_SEE_GLOBAL_CHAT:{}
            case TO_SEND_PRIVATE_MESSAGE:{}
            case TO_SEE_PRIVATE_MESSAGE:{}
            case LOGOUT:{}
            default:
            printf("WRONG COMMAND\n");
            break;
        
        }
    }
}