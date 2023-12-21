#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include "user.h"
#include "mesaje.h"

/* Portul folosit */
#define PORT 2908

/* Codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData {
    int idThread; // ID-ul thread-ului tinut in evidenta de acest program
    int cl; // Descriptorul intors de accept
    struct User *user;
} thData;

static void *treat(void *); /* Functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void handleLogin(thData *);
void handleRegister(thData *);
void handleLogout(thData *);
void handleChangePassword(thData *);
void handleSentMessage(thData *);

int main() {
    struct sockaddr_in server; // Structura folosita de server
    struct sockaddr_in from;
    int sd; // Descriptorul de socket
    pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* Crearea unui socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    /* Utilizarea optiunii SO_REUSEADDR */
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* Pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* Umplem structura folosita de server */
    /* Stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* Acceptam orice adresa */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* Atasam socketul */
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    /* Punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen(sd, 2) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    /* Servim in mod concurent clientii... folosind thread-uri */
    while (1) {
        int client;
        thData *td; // Parametru functia executata de thread
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        /* Acceptam un client (stare blocanta pana la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        td->user = (struct User *)malloc(sizeof(struct User));
        setAuthenticated(td->user, 0);

        pthread_create(&th[i], NULL, &treat, td);
    }
}

static void *treat(void *arg) {
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[Thread] - %d - Asteptam comanda...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());

    while (1) {
        char command[31];
        if (read(tdL.cl, command, sizeof(command)) <= 0) {
            perror("Eroare la citirea comenzii de la client.\n");
            break;
        } else {
            if(!tdL.user->authenticated){
                if (strcmp(command, "login") == 0) {
                    handleLogin(&tdL);
                    tdL.user->authenticated = 1;
                    if (tdL.user->authenticated){
                        //printf("Aici intra? login dupa ce se face if\n");
                        int response = 1; // Cod de succes pentru a intra în meniu
                        if (write(tdL.cl, &response, sizeof(int)) <= 0) {
                            perror("Eroare la trimiterea raspunsului catre client.\n");
                        }
                    }
                    printf("%s", getPassword(tdL.user));
                } else if (strcmp(command, "register") == 0) {
                    handleRegister(&tdL);
                } else if (strcmp(command, "quit") == 0) {
                    tdL.user->authenticated = 0;
                    break;
                }
            } else {
                if (strcmp(command, "logout") == 0) {
                    //("%s", getPassword(tdL.user));
                    //printf("Macar aici ajunge");
                    //tdL.user->authenticated = 0;
                    handleLogout(&tdL);

                } else if(strcmp(command, "change_password") == 0){
                    handleChangePassword(&tdL);
                }else if(strcmp(command, "send_message") == 0){
                    handleSentMessage(&tdL);
                } else if (strcmp(command, "quit") == 0) {
                    handleLogout(&tdL);
                    break;
                } else {
                    printf("[Thread %d] Comanda necunoscuta: %s\n", tdL.idThread, command);
                    // Trimite mesaj de eroare către client
                }
            }
        }
    }

    /* Am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return NULL;
}

void handleLogin(thData *tdL) {
    char username[50];
    char password[50];

    if (read(tdL->cl, username, sizeof(username)) <= 0) {
        perror("Eroare la citirea username-ului de la client.\n");
        return;
    }

    if (read(tdL->cl, password, sizeof(password)) <= 0) {
        perror("Eroare la citirea parolei de la client.\n");
        return;
    }

    setUsername(tdL->user, username);
    setPassword(tdL->user, password);

    if (loginUser(tdL->user)) {
        //printf("Returneaza 1");
        int response = 1; // Autentificare cu succes
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        //printf("Returneaza 0");
        int response = 0; // Eroare la autentificare
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleRegister(thData *tdL) {
    char username[50];
    char password[50];
    if (read(tdL->cl, username, sizeof(username)) <= 0) {
        perror("Eroare la citirea username-ului de la client.\n");
        return;
    }

    if (read(tdL->cl, password, sizeof(password)) <= 0) {
        perror("Eroare la citirea parolei de la client.\n");
        return;
    }

    setUsername(tdL->user, username);
    setPassword(tdL->user, password);

    if (registerUser(tdL->user)) {
        //printf("Registerul merge?");
        int response = 1; // Inregistrare cu succes
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0; // Eroare la inregistrare
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleLogout(thData *tdL) {
    //printf("Intra aici handleLogout");
    if (logoutUser(tdL->user)) {
        //printf("se face logout");
        int response = 1; // Delogare cu succes
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0; // Eroare la delogare
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleChangePassword(thData *tdL){
    char new_password[50];

    if (read(tdL->cl, new_password, sizeof(new_password)) <= 0) {
        perror("Eroare la citirea noii parole de la client.\n");
        return;
    }

    setPassword(tdL->user, new_password);

    if(changePassword(tdL->user, new_password)){
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0; // Eroare la inregistrare
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleSentMessage(thData *tdL){
    char destinatar_username[50];
    char mesaj[500];

    // Citire destinatar
    if (read(tdL->cl, destinatar_username, sizeof(destinatar_username)) <= 0) {
        perror("Eroare la citirea destinatarului de la client.\n");
        return;
    }

    // Citire mesaj
    if (read(tdL->cl, mesaj, sizeof(mesaj)) <= 0) {
        perror("Eroare la citirea mesajului de la client.\n");
        return;
    }

    // Obține informații despre destinatar (User)
    struct User *destinatar = getUserByUsername(destinatar_username);

    if (destinatar == NULL) {
        // Destinatarul nu există în baza de date
        int response = 0; // Eroare la trimiterea mesajului
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
        return;
    }

    struct Mesaje *mesajNou = (struct Mesaje *)malloc(sizeof(struct Mesaje));
    setMesaj(mesajNou,mesaj);

    // Apelarea funcției sentMessage pentru a adăuga mesajul în baza de date
    if (sentMessage(tdL->user, destinatar, mesajNou)) {
        int response = 1; // Mesaj trimis cu succes
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0; // Eroare la trimiterea mesajului
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }

    free(mesajNou);
}
