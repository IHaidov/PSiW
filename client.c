#include <sys/stat.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include <sys/msg.h>

#include <time.h>
#include <fcntl.h>

size_t message_size = 400;
int activity; // 0-niezalogowany, 1-zalogowany
int indeks;	  // 0-niezalogowany, {1,2,..,9}-zalogowany

struct message_buffor
{
	long type; // 10-logowanie {1,2,..,9}-obsluga uzytkownika
	char text[200];
};

struct message
{					// struktura wysylanych wiadomosci
	long type;		// od kogo
	char text[400]; // wiadomosc
	int address;	// do kogo <1,9> - pojedynczy uzytkownik
	time_t msgtime; //{11, 22, 33} - grupa 1, 2 i 3.
};


// OBSLUGA KLIENTA
void client_options(char *input)
{
	int connect; // zmienna z kolejka serwera
	connect = msgget(99999, 0777 | IPC_CREAT);

	// 1)	//LOGOWANIE
	if (!strcmp(input, "login"))
	{
		if (!activity)
		{
			struct message_buffor message_login;
			// PODAWANIE LOGINU
			do
			{
				printf("podaj login:\n");
				scanf("%s", message_login.text);
				message_login.type = 100;
				msgsnd(connect, &message_login, sizeof(message_login) - sizeof(long), 0);
				message_login.type = 10;
				msgrcv(connect, &message_login, sizeof(message_login) - sizeof(long), 10, 0);
			} while (strcmp(message_login.text, "poprawny"));
			// PODAWANIE HASLA
			do
			{
				printf("podaj haslo:\n");
				scanf("%s", message_login.text);
				message_login.type = 100;
				msgsnd(connect, &message_login, sizeof(message_login) - sizeof(long), 0);
				message_login.type = 10;
				msgrcv(connect, &message_login, sizeof(message_login) - sizeof(long), 10, 0);
			} while (strcmp(message_login.text, "poprawny"));
			// ODEBRANIE NR UZYTKOWNIKA
			msgrcv(connect, &message_login, sizeof(message_login) - sizeof(long), 10, 0);
			sscanf(message_login.text, "%d", &indeks);
			activity = 1;
			printf("zalogowany pod indeksem %d\n", indeks);
		}
		else
		{
			printf("jestes juz zalogowany!\n");
		}
	}
	// 2)	//WYLOGOWYWANIE
	else if (!strcmp(input, "logout"))
	{
		// SPRAWDZENIE CZY JEST ZALOGOWANY
		if (activity)
		{
			struct message_buffor message_logout;
			message_logout.type = 12;
			sprintf(message_logout.text, "%d", indeks);
			msgsnd(connect, &message_logout, sizeof(message_logout) - sizeof(long), 0);
			activity = 0;
			indeks = 0;
			printf("wylogowano\n");
		}
		else
		{
			printf("nie jestes zalogowany!!!\n");
		}
	}
	// 3)	//PODGLAD LISTY ZALOGOWANYCH UZYTKOWNIKÓW
	else if (!strcmp(input, "check_users"))
	{
		struct message_buffor message_ch_us;
		message_ch_us.type = 13;
		// WYSLANIE NUMERU UZYTKOWNIKA
		sprintf(message_ch_us.text, "%d", indeks);
		msgsnd(connect, &message_ch_us, sizeof(message_ch_us) - sizeof(long), 0);
		msgrcv(connect, &message_ch_us, sizeof(message_ch_us) - sizeof(long), 133, 0);
		// ODBIÓR I PRINTOWANIE KOMUNIKATU
		printf("%s", message_ch_us.text);
	}
	// 4)	//ZAPISANIE SIE DO GRUPY
	else if (!strcmp(input, "join_server"))
	{
		if (activity)
		{
			struct message_buffor message_join_s;
			message_join_s.type = 14;
			strcpy(message_join_s.text, "");
			sprintf(message_join_s.text, "%d", indeks);
			// WYSYLANIE INDEKSU UZYTKOWNIKA
			msgsnd(connect, &message_join_s, sizeof(message_join_s) - sizeof(long), 0);
			msgrcv(connect, &message_join_s, sizeof(message_join_s) - sizeof(long), 144, 0);
			printf("serwery: 1 2 3\n");
			printf("-------%s-------\n", message_join_s.text);
			printf("0-nalezysz 1-nienalezysz\n");
			printf("podaj do ktorego serwera chesz sie dopisac: ");
			strcpy(message_join_s.text, "");
			int var;
			scanf("%d", &var);
			while (var < 1 || var > 3)
			{
				printf("bledny numer, podaj z zakresu {1,2,3}: ");
				scanf("%d", &var);
			}
			sprintf(message_join_s.text, "%d", var);
			message_join_s.type = 14;
			// WYSYLANIE NUMERU GRUPY DO KTÓREJ CHCE DOLACZYC
			msgsnd(connect, &message_join_s, sizeof(message_join_s) - sizeof(long), 0);
			printf("dodawanie zakonczone\n");
		}
		else
		{
			printf("Pierw sie zaloguj!\n");
		}
	}

	// 5)	//WYPISANIE SIE Z GRUPY
	else if (!strcmp(input, "gout_server"))
	{
		if (activity)
		{
			struct message_buffor message_left_server;
			message_left_server.type = 15;
			strcpy(message_left_server.text, "");
			sprintf(message_left_server.text, "%d", indeks);
			// WYSYLANIE INDEKSU UZYTKOWNIKA
			msgsnd(connect, &message_left_server, sizeof(message_left_server) - sizeof(long), 0);
			msgrcv(connect, &message_left_server, sizeof(message_left_server) - sizeof(long), 155, 0);
			printf("grupy: 1 2 3\n");
			printf("-------%s-------\n", message_left_server.text);
			printf("0-nienalezysz 1-nalezysz\n");
			printf("podaj z którego servera chcesz sie wypisac: ");
			strcpy(message_left_server.text, "");
			int var;
			scanf("%d", &var);
			while (var < 1 || var > 3)
			{
				printf("bledny numer, podaj z zakresu {1,2,3}: ");
				scanf("%d", &var);
			}
			sprintf(message_left_server.text, "%d", var);
			message_left_server.type = 15;
			// WYSYLANIE NUMERU GRUPY Z KTÓREJ CHCE WYJSC
			msgsnd(connect, &message_left_server, sizeof(message_left_server) - sizeof(long), 0);
			printf("wypisywanie zakonczone\n");
		}
		else
		{
			printf("Pierw sie zaloguj!\n");
		}
	}
	// 6)	//PODGLAD DANEJ GRUPY
	else if (!strcmp(input, "check_server"))
	{
		struct message_buffor message_ch_server;
		message_ch_server.type = 16;
		// POBRANIE NUMERU GRUPY
		printf("podaj numer serwera {1, 2, 3}: \n");
		int var;
		scanf("%d", &var);
		while (var < 1 || var > 3)
		{
			printf("bledny numer, podaj z zakresu powyzszego!: ");
			scanf("%d", &var);
		}
		sprintf(message_ch_server.text, "%d", var);
		// WYSLANIE NUMERU GRUPY
		msgsnd(connect, &message_ch_server, sizeof(message_ch_server) - sizeof(long), 0);
		msgrcv(connect, &message_ch_server, sizeof(message_ch_server) - sizeof(long), 166, 0);
		// ODBIÓR I PRINTOWANIE KOMUNIKATU
		printf("osoba|przynalezy\n");
		printf("%s", message_ch_server.text);
	}
	// 7)	//WYSLANIE WIADOMOSCI DO UZYTKOWNIKA
	else if (!strcmp(input, "send_user"))
	{
		if (activity)
		{
			struct message message_send_user;
			message_send_user.type = indeks;
			printf("Do kogo chcesz wyslac wiadomosc?: ");
			scanf("%d", &message_send_user.address);
			while (message_send_user.address > 9 || message_send_user.address < 1)
			{
				printf("Wprowadz nr uzytkownika z zakresu {1,2,..9}: ");
				scanf("%d", &message_send_user.address);
			}
			printf("Wprowadz wiadomosc(max 400 znaków):");
			// char b[400];
			fgets(message_send_user.text, 400, stdin);
			fgets(message_send_user.text, 400, stdin);
			message_send_user.msgtime = time(NULL);
			msgsnd(connect, &message_send_user, sizeof(message_send_user) - sizeof(long) - sizeof(int) - sizeof(time_t), 0);
			printf("Wiadomosc wyslana\n");
		}
		else
		{
			printf("Pierw sie zaloguj!\n");
		}
	}
	// 8)	//ODBIÓR WIADOMOSCI
	else if (!strcmp(input, "recieve"))
	{
		struct message all_messages[300];
        if (activity)
        {
            struct message message_send_user;
            int clause;
            int i=0;
            int counter=0;
            do
            {   
                clause = msgrcv(connect, &message_send_user, sizeof(message_send_user) - sizeof(long) - sizeof(int) - sizeof(time_t), indeks * 111, IPC_NOWAIT);
                if(clause == -1)
                {
                    printf("-----------------------\n");
                    break;
                }
                counter++;
                char *msgdata = ctime(&message_send_user.msgtime);
                //printf("%s", msgdata);
                if (message_send_user.address < 10)
                {
                    all_messages[i++] = message_send_user;
                    
                }
                else
                {
                    
                    all_messages[i++] = message_send_user;
                    
                    
                }
            }while (clause!=-1);
            
            if(counter>10){
            for(i=counter-10;i<counter;i++)
            {
                char *msgdata = ctime(&all_messages[i].msgtime);
                printf("%s", msgdata);
                if (all_messages[i].address < 10)
                {
                    
                    printf("Wiadomosc od %d uzytkownika\n", all_messages[i].address);
                    printf("%s", all_messages[i].text);
                }
                else
                {
                    
                    printf("Wiadomosc od %d grupy\n", (all_messages[i].address / 11));
                    printf("%s", all_messages[i].text);
                    
                }
            }
            }
            else {
            for(i=0;i<counter;i++)
            {
                char *msgdata = ctime(&all_messages[i].msgtime);
                printf("%s", msgdata);
                if (all_messages[i].address < 10)
                {
                    
                    printf("Wiadomosc od %d uzytkownika\n", all_messages[i].address);
                    printf("%s", all_messages[i].text);
                }
                else
                {
                    
                    printf("Wiadomosc od %d grupy\n", (all_messages[i].address / 11));
                    printf("%s", all_messages[i].text);
                    
                }
            }
             }
        }
        else
        {
            printf("Pierw sie zaloguj!\n");
        }
	}
	// 9)	//WYSLANIE WIADOMOSCI DO GRUPY
	else if (!strcmp(input, "send_server"))
	{
		if (activity)
		{
			struct message message_send_user;
			message_send_user.type = 19;
			printf("Do którego serwera chcesz wyslac wiadomosc?: ");
			scanf("%d", &message_send_user.address);
			while (message_send_user.address < 1 || message_send_user.address > 3)
			{
				printf("bledny numer, podaj z zakresu {1,2,3}!: ");
				scanf("%d", &message_send_user.address);
			}
			printf("Wprowadz wiadomosc(max 400 znaków):");
			fgets(message_send_user.text, 400, stdin);
			fgets(message_send_user.text, 400, stdin);
			message_send_user.msgtime = time(NULL);
			msgsnd(connect, &message_send_user, sizeof(message_send_user) - sizeof(long) - sizeof(int) - sizeof(time_t), 0);
			printf("Wiadomosc wyslana\n");
		}
		else
		{
			printf("Pierw sie zaloguj!\n");
		}
	}
	else
	{
		printf("zla komenda!\n");
	}
}

int main(void)
{
	printf(">>> Autorzy:\n");
	printf(">>> Andrei Alesik\n");
	printf(">>> Ivan Haidov\n\n");
	printf(">>> PSiW Projekt 'Czat'\n");
	printf(">>> Prosimy wpisac nastepujace komendy:\n");
	printf("--- login        - logowanie\n");
	printf("--- logout       - wylogowanie\n");
	printf("--- check_users  - podglad listy zalogowanych uzytkownikow\n");
	printf("--- join_server  - rejestracja do serweru\n");
	printf("--- gout_server  - wyjscie z serweru\n");
	printf("--- check_server - sprawdzenie uzytkowników, nalezacych do grupy\n");
	printf("--- send_user    - wyslanie wiadomosci do uzytkownika\n");
	printf("--- send_server  - wyslanie wiadomosci do grupy\n");
	printf("--- recieve      - odebranie wiadomosci\n");
	printf("--- exit         - aby zakonczyc\n");
	printf(">>> ");

	char string_in[20];
	scanf("%s", string_in);

	activity = 0; // niezalogowany
	while (strcmp(string_in, "exit"))
	{
		client_options(string_in);
		
		printf(">>> Prosimy wpisac nastepujace komendy:\n");
		printf("--- login        - logowanie\n");
		printf("--- logout       - wylogowanie\n");
		printf("--- check_users  - podglad listy zalogowanych uzytkownikow\n");
		printf("--- join_server  - rejestracja do serweru\n");
		printf("--- gout_server  - wyjscie z serweru\n");
		printf("--- check_server - sprawdzenie uzytkowników, nalezacych do grupy\n");
		printf("--- send_user    - wyslanie wiadomosci do uzytkownika\n");
		printf("--- send_server  - wyslanie wiadomosci do grupy\n");
		printf("--- recieve      - odebranie wiadomosci\n");
		printf("--- exit         - aby zakonczyc\n");
		printf(">>> ");
		scanf("%s", string_in);
	}
}
