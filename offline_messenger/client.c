#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

extern int errno;
int port;

int main(int argc, char *argv[]) {
    int sd;                     // descriptorul de socket
    struct sockaddr_in server;  // structura folosita pentru conectare
    char command[31];
    int response;
    int authenticated = 0;


    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    port = atoi(argv[2]);

    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    while (1) {
        if (!authenticated) {
            printf("Comenzi disponibile: login, register, quit\n");
        } else {
            printf("Comenzi disponibile: send_message, change_password, logout, quit\n");
        }
        printf("Introduceti comanda: ");
        fflush(stdout);
        scanf("%s", command);

        if (write(sd, command, sizeof(command)) <= 0) {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
        }
        if (!authenticated) {
            if (strcmp(command, "login") == 0) {
                // Handle login
                char username[50], password[50];
                printf("Username: ");
                fflush(stdout);
                scanf("%s", username);
                printf("Password: ");
                fflush(stdout);
                scanf("%s", password);

                if (write(sd, username, sizeof(username)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (write(sd, password, sizeof(password)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (read(sd, &response, sizeof(int)) <= 0) {
                    perror("[client]Eroare la citirea raspunsului de la server.\n");
                    return errno;
                }

                if (response == 1) {
                    printf("Autentificare cu succes!\n");
                    authenticated = 1;
                } else {
                    printf("Autentificare esuata!\n");
                }
            } else if (strcmp(command, "register") == 0) {
                // Handle register
                char username[50], password[50];
                printf("Username: ");
                fflush(stdout);
                scanf("%s", username);
                printf("Password: ");
                fflush(stdout);
                scanf("%s", password);

                if (write(sd, username, sizeof(username)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (write(sd, password, sizeof(password)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (read(sd, &response, sizeof(int)) <= 0) {
                    perror("[client]Eroare la citirea raspunsului de la server.\n");
                    return errno;
                }

                if (response == 1) {
                    printf("Inregistrare cu succes!\n");
                } else {
                    printf("Inregistrare esuata!\n");
                }
            } else if (strcmp(command, "quit") == 0) {
                authenticated = 0;

                break; // Iesire din program
            } else {
                printf("Comanda necunoscuta: %s\n", command);
            }
        } else {
            if (strcmp(command, "send_message") == 0) {
                char destinatar_username[50], mesaj[500];
                printf("Destinatar: ");
                fflush(stdout);
                scanf("%s", destinatar_username);
                printf("Mesaj: ");
                fflush(stdout);
                scanf(" %[^\n]", mesaj);

                if (write(sd, destinatar_username, sizeof(destinatar_username)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (write(sd, mesaj, sizeof(mesaj)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (read(sd, &response, sizeof(int)) <= 0) {
                    perror("[client]Eroare la citirea raspunsului de la server.\n");
                    return errno;
                }

                if (response == 1) {
                    printf("Mesaj trimis cu succes!\n");
                } else {
                    printf("Eroare la trimiterea mesajului!\n");
                }
            } else if (strcmp(command, "logout") == 0) {

                if (write(sd, command, sizeof(command)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (read(sd, &response, sizeof(int)) <= 0) {
                    perror("[client]Eroare la citirea raspunsului de la server.\n");
                    return errno;
                }

                if (response == 1) {
                    printf("Deconectare cu succes!\n");
                    authenticated = 0;
                } else {
                    printf("Deconectare esuata!\n");
                }
            } else if(strcmp(command, "change_password") == 0) {
                char new_password[50];
                printf("New_password: ");
                fflush(stdout);
                scanf("%s", new_password);
                if (write(sd, new_password, sizeof(new_password)) <= 0) {
                    perror("[client]Eroare la write() spre server.\n");
                    return errno;
                }

                if (read(sd, &response, sizeof(int)) <= 0) {
                    perror("[client]Eroare la citirea raspunsului de la server.\n");
                    return errno;
                }

                if (response == 1) {
                    printf("Parola a fost schimbata cu succes!\n");
                } else {
                    printf("A esuat schimbarea parolei!\n");
                }
            } else if (strcmp(command, "quit") == 0) {
                authenticated = 0;
                break; // Iesire din program
            } else {
                printf("Comanda necunoscuta: %s\n", command);
            }
        }
    }

    // Inchidem conexiunea cu serverul
    close(sd);
    return 0;
}