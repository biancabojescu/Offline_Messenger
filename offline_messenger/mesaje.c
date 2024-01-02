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
    if (mesaj->mesaj != NULL) {
        free(mesaj->mesaj);
    }

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
    if (isAuthenticated(expeditor)) {
        setExpeditorId(mesaj, getIdUser(expeditor));
        setDestinatarId(mesaj, getIdUser(destinatar));

        MYSQL* conn = connDatabase();
        if (conn) {
            char query[500];
            sprintf(query, "INSERT INTO mesaje (expeditorId, destinatarId, mesaj) VALUES ('%d', '%d', '%s')",
                    getExpeditorId(mesaj), getDestinatarId(mesaj), getMesaj(mesaj));

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

struct Mesaje** see_all_new_messages(struct User* destinatar) {
    MYSQL* conn = connDatabase();

    if (!conn) {
        return NULL;
    }

    char query[500];
    sprintf(query, "SELECT mesaje.id, mesaje.expeditorId, mesaje.destinatarId, mesaje.mesaj "
                   "FROM mesaje "
                   "JOIN users ON mesaje.expeditorId = users.id "
                   "AND mesaje.destinatarId = %d "
                   "AND mesaje.seen = 0", getIdUser(destinatar));

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (!result) {
        printf("Error retrieving results!\n");
        mysql_free_result(result);
        mysql_close(conn);
        return NULL;
    }

    int num_rows = mysql_num_rows(result);

    struct Mesaje** conversatie = (struct Mesaje**)malloc((num_rows + 1) * sizeof(struct Mesaje*));

    if (num_rows > 0) {
        int i = 0;
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)) && (i < num_rows)) {
            conversatie[i] = (struct Mesaje*)malloc(sizeof(struct Mesaje));
            setIdMesaje(conversatie[i], atoi(row[0]));
            setExpeditorId(conversatie[i], atoi(row[1]));
            setDestinatarId(conversatie[i], atoi(row[2]));
            setMesaj(conversatie[i], row[3]);
            setSeen(conversatie[i], 0);
            i++;
        }

        conversatie[i] = NULL;

        sprintf(query, "UPDATE mesaje SET seen = 1 WHERE destinatarId = %d AND seen = 0", getIdUser(destinatar));
        if (mysql_query(conn, query)) {
            fprintf(stderr, "Error updating message status: %s\n", mysql_error(conn));
        }

        mysql_free_result(result);
        mysql_close(conn);
        return conversatie;
    } else {
        conversatie[0]=NULL;
        mysql_free_result(result);
        mysql_close(conn);
        return conversatie;
    }
}
/*
int replyToMessage(struct User* expeditor, struct User* destinatar, struct Mesaje* mesaj) {
    // Implementați aici logica pentru răspuns la un mesaj
    // Returnați 0 sau alt cod de eroare în caz de eșec
    return 1; // Exemplu de succes
}
 */
struct Mesaje** get_a_conversation(struct User* user1, struct User* user2) {
    MYSQL* conn = connDatabase();

    if (!conn) {
        return NULL;
    }

    char query[500];
    sprintf(query, "select mesaje.id, mesaje.expeditorId, mesaje.destinatarId, mesaje.mesaj from mesaje join users on mesaje.expeditorId = users.id and ((mesaje.destinatarId=%d and mesaje.expeditorId=%d) or (mesaje.destinatarId = %d and mesaje.expeditorId=%d)) order by mesaje.id"
            , getIdUser(user1), getIdUser(user2), getIdUser(user2), getIdUser(user1));

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES* result = mysql_store_result(conn);

    if (!result) {
        printf("Error retrieving results!\n");
        mysql_free_result(result);
        mysql_close(conn);
        return NULL;
    }

    int num_rows = mysql_num_rows(result);

    struct Mesaje** conversatie = (struct Mesaje**)malloc((num_rows + 1) * sizeof(struct Mesaje*));

    if (num_rows > 0) {
        int i = 0;
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)) && (i < num_rows)) {
            conversatie[i] = (struct Mesaje*)malloc(sizeof(struct Mesaje));
            setIdMesaje(conversatie[i], atoi(row[0]));
            setExpeditorId(conversatie[i], atoi(row[1]));
            setDestinatarId(conversatie[i], atoi(row[2]));
            setMesaj(conversatie[i], row[3]);
            setSeen(conversatie[i], 1);
            i++;
        }

        conversatie[i] = NULL;

        sprintf(query, "UPDATE mesaje SET seen = 1 WHERE destinatarId = %d AND seen = 0", getIdUser(user1));
        if (mysql_query(conn, query)) {
            fprintf(stderr, "Error updating message status: %s\n", mysql_error(conn));
        }

        mysql_free_result(result);
        mysql_close(conn);
        return conversatie;
    } else {
        conversatie[0]=NULL;
        mysql_free_result(result);
        mysql_close(conn);
        return conversatie;
    }
}
