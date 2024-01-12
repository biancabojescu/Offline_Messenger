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
void handleNewMessage(thData *);
void handleSeeConversations(thData *);
void  handleOnlineUsers(thData *);

int main() {
    struct sockaddr_in server; // Structura folosita de server
    struct sockaddr_in from;
    int sd; // Descriptorul de socket
    pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
    int i = 0;

    /* Crearea unui socket
     * AF_INET protocolul IPv4
     * SOCK_STREAM protocolul TCP*/
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    /* Utilizarea optiunii SO_REUSEADDR - reutilizarea adresei locale*/
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    /* Pregatirea structurilor de date */
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    /* Umplem structura folosita de server */
    /* Stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* Acceptam orice adresa disponibila*/
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Utilizam un port utilizator */
    server.sin_port = htons(PORT);

    /* Atasam socketul pt a specifica la ce adresa IP si port se vor asculta clientii */
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
    pthread_detach(pthread_self()); //se evita crearea unui zombie thread

    while (1) {
        char command[31];
        if (read(tdL.cl, command, sizeof(command)) <= 0) {
            perror("Eroare la citirea comenzii de la client.\n");
            break;
        } else {
            command[strlen(command)]='\0';
            if(!tdL.user->authenticated){
                if (strcmp(command, "login") == 0) {
                    handleLogin(&tdL);
                    tdL.user->authenticated = 1;
                } else if (strcmp(command, "register") == 0) {
                    handleRegister(&tdL);
                } else if (strcmp(command, "quit") == 0) {
                    tdL.user->authenticated = 0;
                    break;
                }
            } else {
                if (strcmp(command, "logout") == 0) {
                    handleLogout(&tdL);
                } else if(strcmp(command, "change_password") == 0){
                    handleChangePassword(&tdL);
                } else if(strcmp(command, "online_users") == 0){
                    handleOnlineUsers(&tdL);
                } else if(strcmp(command, "send_message") == 0){
                    handleSentMessage(&tdL);
                } else if(strcmp(command,"see_new_messages") == 0){
                    handleNewMessage(&tdL);
                } else if(strcmp(command,"see_a_conversation") == 0){
                    handleSeeConversations(&tdL);
                } else if (strcmp(command, "quit") == 0) {
                    handleLogout(&tdL);
                    break;
                } else {
                    printf("[Thread %d] Comanda necunoscuta: %s\n", tdL.idThread, command);
                }
            }
        }
    }

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

    username[strlen(username)]='\0';

    if (read(tdL->cl, password, sizeof(password)) <= 0) {
        perror("Eroare la citirea parolei de la client.\n");
        return;
    }

    password[strlen(password)]='\0';

    setUsername(tdL->user, username);
    setPassword(tdL->user, password);

    if (loginUser(tdL->user)) {
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0;
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
    username[strlen(username)]='\0';

    if (read(tdL->cl, password, sizeof(password)) <= 0) {
        perror("Eroare la citirea parolei de la client.\n");
        return;
    }
    password[strlen(password)]='\0';

    setUsername(tdL->user, username);
    setPassword(tdL->user, password);

    if (registerUser(tdL->user)) {
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleLogout(thData *tdL) {
    if (logoutUser(tdL->user)) {
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0;
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
    new_password[strlen(new_password)]='\0';

    setPassword(tdL->user, new_password);

    if(changePassword(tdL->user, new_password)){
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }
}

void handleSentMessage(thData *tdL){
    char destinatar_username[50];
    char mesaj[500];

    if (read(tdL->cl, destinatar_username, sizeof(destinatar_username)) <= 0) {
        perror("Eroare la citirea destinatarului de la client.\n");
        return;
    }
    destinatar_username[strlen(destinatar_username)] = '\0';

    if (read(tdL->cl, mesaj, sizeof(mesaj)) <= 0) {
        perror("Eroare la citirea mesajului de la client.\n");
        return;
    }
    mesaj[strlen(mesaj)]='\0';

    struct User *destinatar = getUserByUsername(destinatar_username);

    if (destinatar == NULL) {
        int response = 0;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
        return;
    }

    struct Mesaje *mesajNou = (struct Mesaje *)malloc(sizeof(struct Mesaje));
    setMesaj(mesajNou,mesaj);

    if (sentMessage(tdL->user, destinatar, mesajNou)) {
        int response = 1;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    } else {
        int response = 0;
        if (write(tdL->cl, &response, sizeof(int)) <= 0) {
            perror("Eroare la trimiterea raspunsului catre client.\n");
        }
    }

    free(mesajNou);
}

void handleNewMessage(thData* tdL) {
    char** all_new_messages = see_all_new_messages(tdL->user);

    size_t conversatie_size = 3000;
    char* conversatie = (char *)malloc(conversatie_size);
    conversatie[0] = '\0';

    if (all_new_messages == NULL || all_new_messages[0] == NULL) {
        printf("Nu exista mesaje noi pentru %s\n", tdL->user->username);
    }

    int numMessages = 0;
    while (all_new_messages[numMessages] != NULL) {
        numMessages++;
    }

    if (write(tdL->cl, &numMessages, sizeof(int)) <= 0) {
        perror("Error sending message ID to client.\n");
    }


    for (int i = 0; i < numMessages; i++) {
        char temp[30000];
        sprintf(temp, "%s\n", all_new_messages[i]);

        if (strlen(conversatie) + strlen(temp) < conversatie_size) {
            strcat(conversatie, temp);
        } else {
            printf("Eroare: Spațiu insuficient pentru adăugarea unui mesaj!\n");
        }
    }

    if (write(tdL->cl, conversatie, strlen(conversatie)) <= 0) {
        perror("Error sending message to client.\n");
    }

    for (int i = 0; i < numMessages; i++) {
        free(all_new_messages[i]);
    }
    free(all_new_messages);
    free(conversatie);
}

void handleSeeConversations(thData *tdL){
    char user2[50];
    memset(user2, 0, sizeof(user2));
    if (read(tdL->cl, user2, sizeof(user2)) <= 0) {
        perror("Eroare la citirea comenzii de la client.\n");
    }
    user2[strlen(user2)]='\0';

    char** all_new_messages = get_a_conversation(tdL->user, getUserByUsername(user2));

    size_t conversatie_size = 3000;
    char* conversatie = (char *)malloc(conversatie_size);
    memset(conversatie, 0, conversatie_size);
    conversatie[0] = '\0';

    if (all_new_messages == NULL || all_new_messages[0] == NULL) {
        sprintf(conversatie, "Nu există o conversație cu %s\n", user2);
        if (write(tdL->cl, conversatie, strlen(conversatie)) <= 0) {
            perror("Error sending message to client.\n");
        }
        //free(conversatie);
        return;
    }

    int numMessages = 0;
    while (all_new_messages[numMessages] != NULL) {
        numMessages++;
    }

    for (int i = 0; i < numMessages; i++) {
        char temp[30000];
        memset(temp,0, sizeof(temp));
        sprintf(temp, "%s\n", all_new_messages[i]);

        if (strlen(conversatie) + strlen(temp) < conversatie_size) {
            strcat(conversatie, temp);
        } else {
            printf("Eroare: Spațiu insuficient pentru adăugarea unui mesaj!\n");
        }
    }

    if (write(tdL->cl, conversatie, strlen(conversatie)) <= 0) {
        perror("Error sending message to client.\n");
    }


    for (int i = 0; i < numMessages; i++) {
        free(all_new_messages[i]);
    }
    free(all_new_messages);
    free(conversatie);
}

void handleOnlineUsers(thData *tdL) {
    char* onlineUsers = online_users();

    if (onlineUsers != NULL) {
        if (write(tdL->cl, onlineUsers, strlen(onlineUsers)) <= 0) {
            perror("Eroare la trimiterea utilizatorilor online către client.\n");
        }
        free(onlineUsers);
    } else {
        const char* errorMessage = "Eroare la obținerea utilizatorilor online.";
        if (write(tdL->cl, errorMessage, strlen(errorMessage)) <= 0) {
            perror("Eroare la trimiterea mesajului de eroare către client.\n");
        }
    }
}
