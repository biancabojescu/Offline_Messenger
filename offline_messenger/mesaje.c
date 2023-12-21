#include "mesaje.h"

void setIdMesaje(struct Mesaje* mesaj, int id) {
    mesaj->id = id;
}

void setExpeditorId(struct Mesaje* mesaj, int expeditorId) {
    mesaj->expeditorId = expeditorId;
}

void setDestinatarId(struct Mesaje* mesaj, int destinatarId) {
    mesaj->destinatarId = destinatarId;
}

void setMesaj(struct Mesaje* mesaj, const char* mesajText) {
    // Verificați dacă există deja un mesaj și eliberați memoria alocată anterior, dacă este cazul
    if (mesaj->mesaj != NULL) {
        free(mesaj->mesaj);
    }

    // Alocare memorie pentru noul mesaj
    mesaj->mesaj = strdup(mesajText);
}

void setSeen(struct Mesaje* mesaj, int seen){
    mesaj->seen = seen;
}

int getIdMesaje(struct Mesaje* mesaj) {
    return mesaj->id;
}

int getExpeditorId(struct Mesaje* mesaj) {
    return mesaj->expeditorId;
}

int getDestinatarId(struct Mesaje* mesaj) {
    return mesaj->destinatarId;
}

char* getMesaj(struct Mesaje* mesaj) {
    return mesaj->mesaj;
}

int isSeen(struct Mesaje* mesaj){
    return mesaj->seen;
}

int sentMessage(struct User* expeditor, struct User* destinatar, struct Mesaje* mesaj) {
    if (isAuthenticated(expeditor) && isAuthenticated(destinatar)) {
        //printf("ID-ul expeditorului: %d\n", getIdUser(expeditor));
        setExpeditorId(mesaj, getIdUser(expeditor));
        setDestinatarId(mesaj, getIdUser(destinatar));

        // Implementează logica pentru trimiterea mesajului în baza de date
        MYSQL* conn = connDatabase();
        if (conn) {
            char query[500];
            sprintf(query, "INSERT INTO mesaje (expeditorId, destinatarId, mesaj) VALUES ('%d', '%d', '%s')",
                    getExpeditorId(mesaj), getDestinatarId(mesaj), getMesaj(mesaj));
            printf("%s", getUsername(expeditor));

            if (mysql_query(conn, query)) {
                fprintf(stderr, "Error sending message to database: %s\n", mysql_error(conn));
                mysql_close(conn);
                return 0;
            }

            mysql_close(conn);
            return 1;
        } else {
            return 0;
        }
    } else {
        fprintf(stderr, "Error: Expeditorul sau destinatarul nu este autentificat.\n");
        return 0;
    }
}

int replyToMessage(struct User* expeditor, struct User* destinatar, struct Mesaje* mesaj) {
    // Implementați aici logica pentru răspuns la un mesaj
    // Returnați 0 sau alt cod de eroare în caz de eșec
    return 1; // Exemplu de succes
}

struct Mesaje* getConversationHistory(struct User* user1, struct User* user2) {
    // Implementați aici logica pentru a obține istoricul conversației
    // Alocati memoria necesara pentru structurile Mesaje
    // Returnați un pointer la un array de Mesaje sau NULL în caz de eșec
    return NULL; // Exemplu pentru lipsa de date
}

int searchMessage(struct User* user, char* cuvant) {
    // Implementați aici logica pentru căutarea unui mesaj în funcție de cuvântul cheie
    // Returnați 1 dacă găsiți mesajul sau 0 dacă nu
    return 0; // Exemplu pentru niciun mesaj găsit
}

void markMessageAsSeen(struct Mesaje* mesaj) {
    // Actualizează câmpul "seen" în obiectul Mesaje
    mesaj->seen = 1; // Presupunând că 1 reprezintă mesajul văzut, poți ajusta după nevoie
    // În acest punct, ar trebui să actualizezi baza de date pentru a reflecta schimbările
    // Implementează logica specifică bazei de date aici
    // Poți utiliza o interogare SQL UPDATE pentru a actualiza câmpul "seen" în baza de date MySQL.
}