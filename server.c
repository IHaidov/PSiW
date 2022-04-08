#include <fcntl.h>
#include <string.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <time.h>
#include <stdio.h>
#define USERS_NR 9	//stała liczba użytkowników

struct User {		//struktura uzytkownika
	char name[20];	
	char password[20];
	int active;	//0-niezalogowany 1-zalogowany
	int server[4];	//0-nienalezy 1-nalezy
};

struct msgbuf {		//struktura wiadomości
	long type;
	char text[200];
};
struct message {	//struktura przekierowywanych wiadomosci 
	long type;	//od kogo
	char text[400];	//wiadomość
	int address;	//do kogo <1,9> - pojedynczy użytkownik
	time_t msgtime; //{11, 22, 33} - grupa 1, 2 i 3.
};
//ŁADOWANIE UŻYTKOWNIKÓW Z PLIKU
void read_users(struct User *user) {
	FILE *fp;
	fp = fopen("list_of_users.txt", "r");
		
	int index;
	index = 1;
	while(index <= USERS_NR)
	{
		fscanf(fp, "%s", user[index].name);
		fscanf(fp, "%s", user[index].password);
		user[index].active = 0;
		user[index].server[1] = 0;
		user[index].server[2] = 0;
		user[index].server[3] = 0;
		index++;
	}
	//ŁADOWANIE PRZYNALEŻNOŚCI DO GRUP
	index = 1;
	int var;
	while(index <= USERS_NR)
	{
		fscanf(fp, "%d", &var);
		if(var > 0) {
			user[index].server[var] = 1;
		}		
		index++;
	}
	fclose(fp);
}
//SPRAWDZENIE ISTNIENIA UZYTKOWNIKA
int user_check(char* name, struct User *user, int id) {
	int I= 1;
	struct msgbuf message_check;
	message_check.type = 10;
	printf("próba logowania\n");
	//PRZEGLĄD PO WSZYSTKICH NAZWACH I POROWNANIE
	while(strcmp(name, user[I].name)) {
		I++;	
		//GDY NIE ZNALAZŁ TAKIEJ NAZWY PROSI O POPRAWE	
		if(I > USERS_NR) {		
			printf("błędny login\n");
			strcpy(message_check.text, "error");
			message_check.type = 10;
			msgsnd(id, &message_check, sizeof(message_check)-sizeof(long), 0);
			message_check.type = 100;
			msgrcv(id, &message_check, sizeof(message_check)-sizeof(long), 100, 0);
			strcpy(name, message_check.text);
			I = 1;
		}
	}
	//POPRAWNA NAZWA UZYTKOWNIKA
	message_check.type = 10;
	strcpy(message_check.text, "poprawny");
	msgsnd(id, &message_check, sizeof(message_check)-sizeof(long), 0);
	//ZWRACA NUMER UŻYTKOWNIKA
	return I;
}
//OBSŁUGA KLIENTÓW
void send_request(int id_s, struct User *user) {
	
	struct msgbuf message;	//odbieranie wiadomości	
	int clause;		//zmienna pomocnicza - odbiór komunikatu
// 1)	//LOGOWANIE
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 100, IPC_NOWAIT);
	if(clause != -1) {
		//SPRAWDZENIE NAZWY		
		int index;
		index = user_check(message.text, user, id_s);
		printf("uzytkownik %d chce sie zalogowac\n", index);
		
		//bufor pomocniczy do kontaku z userem
		struct msgbuf message_check;	
		message_check.type = 10;
		//warunek stopu
		int var;		
		var = 1;
		//SPRAWDZENIE HASLA
		do {		
			msgrcv(id_s, &message, sizeof(message_check)-sizeof(long), 100, 0);
			//BŁĘDNE HASŁO
			if(strcmp(user[index].password, message.text)) {
				message_check.type = 10;
				strcpy(message_check.text, "error");
				printf("uzytkownik %d błędne hasło\n", index);
				msgsnd(id_s, &message_check, sizeof(message_check)-sizeof(long), 0);
			}
			//POPRAWNE HASŁO
			else {	
				message_check.type = 10;
				strcpy(message_check.text, "poprawny");
				msgsnd(id_s, &message_check, sizeof(message_check)-sizeof(long), 0);
				var = 0;
			}
		} while(var);
		//UZYTKOWNIK ZALOGOWANY
		printf("uzytkownik %d zalogował się\n", index);
		sprintf(message_check.text, "%d", index);
		//WYSLANIE UZYTKOWNIKOWI SWOJEGO INDEKSU
		message_check.type = 10;		
		msgsnd(id_s, &message_check, sizeof(message_check)-sizeof(long), 0);
		user[index].active = 1;
	}
// 2)	//WYLOGOWANIE
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 12, IPC_NOWAIT);
	if(clause != -1) {
		printf("użytkownik %s wylogował się\n", message.text);
		int pom;		
		sscanf(message.text, "%d", &pom);
		user[pom].active = 0;
	}
// 3)	//PODGLĄD LISTY UŻYTKOWNIKÓW
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 13, IPC_NOWAIT);
	if(clause != -1) {
		//POBRANIE NUMERU UZYTKOWNIKA		
		int number;
		sscanf(message.text, "%d", &number);
		printf("podgląd listy %d uzytkownika\n", number);
		//PRZYGOTOWANIE WIADOMOŚCI
		char help[3];
		//wklejam do message.text po kolei nazwy i active
		strcpy(message.text, user[1].name);
		strcat(message.text, " ");
		sprintf(help, "%d", user[1].active);
		strcat(message.text, help);
		strcat(message.text, "\n");
		message.type = 133;
		//SKLEJANIE WYSYŁKI
		int it;	
		for(it = 2; it <= USERS_NR; it++) {
			strcat(message.text, user[it].name);
			strcat(message.text, " ");
			sprintf(help, "%d", user[it].active);
			strcat(message.text, help);
			strcat(message.text, "\n");
		}
		//wysyłam caly podgląd w jednym stringu
		msgsnd(id_s, &message, sizeof(message)-sizeof(long), 0);
	}
// 4)	//ZAPISANIE SIĘ DO SERWERA
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 14, IPC_NOWAIT);
	if(clause != -1) {
		//POBRANIE NR UZYTKOWNIKA
		int number;
		sscanf(message.text, "%d", &number);
		printf("uzytkownik %d chce sie zapisac do serwera\n", number);	
		char help[6];
		//PRZYGOTOWANIE WIADOMOŚCI
		help[0] = user[number].server[1] + '0';
		help[1] = ' ';
		help[2] = user[number].server[2] + '0';
		help[3] = ' ';
		help[4] = user[number].server[3] + '0';
		help[5] = '\0';
		//WYSŁANIE PRZYNALEŻNOŚCI DO KTÓRYCH SERWEROW JUŻ NALEŻY
		strcpy(message.text, help);
		message.type = 144;
		msgsnd(id_s, &message, sizeof(message)-sizeof(long), 0);
		msgrcv(id_s, &message, sizeof(message)-sizeof(long), 14, 0);
		//ODEBRANIE NR SERWERA I PRZYPISANIE
		int server_nr;
		sscanf(message.text, "%d", &server_nr);
		printf("uzytkownik %d zapisał sie do serwera %d\n", number, server_nr);
		user[number].server[server_nr] = 1;		
	}	

// 5)	//WYPISANIE SIĘ Z SERWERA
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 15, IPC_NOWAIT);
	if(clause != -1) {
		//POBRANIE NR UZYTKOWNIKA		
		int number;
		sscanf(message.text, "%d", &number);
		printf("uzytkownik %d chce sie wypisac z serwera\n", number);
		//PRZYGOTOWANIE WIADOMOŚCI	
		char help[6];
		help[0] = user[number].server[1] + '0';
		help[1] = ' ';
		help[2] = user[number].server[2] + '0';
		help[3] = ' ';
		help[4] = user[number].server[3] + '0';
		help[5] = '\0';
		//WYSŁANIE PRZYNALEŻNOŚCI DO KTÓRYCH SERWEROW JUŻ NALEŻY
		strcpy(message.text, help);
		message.type = 155;
		msgsnd(id_s, &message, sizeof(message)-sizeof(long), 0);
		msgrcv(id_s, &message, sizeof(message)-sizeof(long), 15, 0);
		//ODEBRANIE NR SERWERA I WYPISANIE
		int server_nr;
		sscanf(message.text, "%d", &server_nr);
		printf("uzytkownik %d wypisał sie z serwera %d\n", number, server_nr);
		user[number].server[server_nr] = 0;		
	}	
// 6) 	//PODGLĄD DANEGO SERWERA
	clause = msgrcv(id_s, &message, sizeof(message)-sizeof(long), 16, IPC_NOWAIT);
	if(clause != -1) {
		//POBRANIE NUMERU SERWERA		
		int gnumber;
		sscanf(message.text, "%d", &gnumber);
		printf("podgląd %d serwera\n", gnumber);
		//PRZYGOTOWANIE WIADOMOŚCI
		char help[3];
		//wklejamy do message.text po kolei nazwy i active
		strcpy(message.text, user[1].name);
		strcat(message.text, "   ");
		sprintf(help, "%d", user[1].server[gnumber]);
		strcat(message.text, help);
		strcat(message.text, "\n");
		//SKLEJANIE WYSYŁKI
		int it;	
		for(it = 2; it <= USERS_NR; it++) {
			strcat(message.text, user[it].name);
			strcat(message.text, "   ");
			sprintf(help, "%d", user[it].server[gnumber]);
			strcat(message.text, help);
			strcat(message.text, "\n");
		}
		//wysyłamy caly podgląd w jednym stringu
		message.type = 166;
		msgsnd(id_s, &message, sizeof(message)-sizeof(long), 0);
	}
//PRZEKAZANIE WIADOMOŚCI UZYTKOWNIKA
	struct message mssg;
	clause = msgrcv(id_s, &mssg, sizeof(mssg)-sizeof(long)-sizeof(int)-sizeof(time_t), -9, IPC_NOWAIT);
	if(clause != -1) {
		printf("Wiadomosc od %ld do %d\n", mssg.type, mssg.address);
		int pom;
		pom = mssg.type;		
		mssg.type = mssg.address * 111;
		mssg.address = pom;
		mssg.msgtime = time(NULL);		
		msgsnd(id_s, &mssg, sizeof(mssg)-sizeof(long)-sizeof(int)-sizeof(time_t), 0);
	}
// 9)	//PRZEKAZANIE WIADOMOŚCI SERWERA
	clause = msgrcv(id_s, &mssg, sizeof(mssg)-sizeof(long)-sizeof(int)-sizeof(time_t), 19, IPC_NOWAIT);
	if(clause != -1) {
		printf("Wiadomosc do grupy%d\n", mssg.address);
		int it;		
		for(it = 1; it <= USERS_NR; it++) {
			if(user[it].server[mssg.address] == 1) {
				mssg.type = it * 111;
				mssg.msgtime = time(NULL);
				mssg.address = mssg.address * 11;		
				msgsnd(id_s, &mssg, sizeof(mssg)-sizeof(long)-sizeof(int)-sizeof(time_t), 0);
				mssg.address = mssg.address / 11;
			}
		}	
		
	}
	
}

int main(void) {
	printf(">>> Autorzy:\n");
    printf(">>> Andrei Alesik\n");
    printf(">>> Ivan Haidov\n\n");
    printf(">>> Serwer\n");
	printf("Żeby wyjść, wciśnij: CTRL+C\n");
	//tablica struktur użytkowników
	struct User test[USERS_NR + 1];	
	//wczytanie nazw i haseł użytkowników
	read_users(test);		
	//zmienna z kolejką serwera
	int ipc_s;		
	ipc_s = msgget(99999, 0777 | IPC_CREAT);	
		
	while(1) {
		send_request(ipc_s, test);		
	}
	return 0;
}
