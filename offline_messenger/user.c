#include "user.h"

void setIdUser(struct User* user, int id) {
    user->id = id;
}

void setUsername(struct User* user, const char* username) {
    if (user->username != NULL) {
        free(user->username);
    }

    user->username = strdup(username);
}

void setPassword(struct User* user, const char* password) {
    if (user->password != NULL) {
        free(user->password);
    }

    user->password = strdup(password);
}

void setAuthenticated(struct User* user, int authenticated) {
    user->authenticated = authenticated;
}

int getIdUser(struct User* user) {
    MYSQL* conn = connDatabase();

    if (conn) {
        char query[500];
        sprintf(query, "SELECT id FROM users WHERE username = '%s'", getUsername(user));

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return -1;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);
                setIdUser(user, atoi(row[0]));
                mysql_free_result(result);
                mysql_close(conn);
                return user->id;
            } else {
                fprintf(stderr, "User-ul nu exista.\n");
                mysql_free_result(result);
                mysql_close(conn);
                return -1;
            }

            mysql_free_result(result);
        }

        mysql_close(conn);
        return user->id;
    } else {
        free(user);
        return -1;
    }
}

char* getUsername(struct User* user) {
    return user->username;
}

char* getPassword(struct User* user) {
    return user->password;
}
int getAuthenticated(struct User* user){
    return user->authenticated;
}

int isAuthenticated(struct User* user) {
    MYSQL* conn = connDatabase();

    if (conn) {
        char query[500];
        sprintf(query, "SELECT * FROM users WHERE username = '%s'", getUsername(user));

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return -1;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);
                setAuthenticated(user, atoi(row[3]));
                mysql_free_result(result);
                mysql_close(conn);
                return user->authenticated;
            } else {
                fprintf(stderr, "User-ul nu exista.\n");
                mysql_free_result(result);
                mysql_close(conn);
                return -1;
            }

            mysql_free_result(result);
        }

        mysql_close(conn);
        return user->authenticated;
    } else {
        free(user);
        return -1;
    }
}

struct User* getUserByUsername(const char* username) {
    MYSQL* conn = connDatabase();
    struct User* user = (struct User*)malloc(sizeof(struct User));

    if (conn) {
        char query[500];
        sprintf(query, "SELECT * FROM users WHERE username = '%s'", username);

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            free(user);
            return NULL;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);
                setIdUser(user, atoi(row[0]));
                setUsername(user, row[1]);
                setPassword(user, row[2]);
                setAuthenticated(user, atoi(row[3]));
            } else {
                fprintf(stderr, "User-ul nu exista.\n");
                mysql_free_result(result);
                mysql_close(conn);
                free(user);
                return NULL;
            }

            mysql_free_result(result);
        }

        mysql_close(conn);
        return user;
    } else {
        free(user);
        return NULL;
    }
}

struct User* getUserById(int id) {
    MYSQL* conn = connDatabase();
    struct User* user = (struct User*)malloc(sizeof(struct User));

    if (conn) {
        char query[500];
        sprintf(query, "SELECT * FROM users WHERE id = '%d'", id);

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            free(user);
            return NULL;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                MYSQL_ROW row = mysql_fetch_row(result);
                setIdUser(user, atoi(row[0]));
                setUsername(user, row[1]);
                setPassword(user, row[2]);
                setAuthenticated(user, atoi(row[3]));
            } else {
                fprintf(stderr, "User-ul nu exista.\n");
                mysql_free_result(result);
                mysql_close(conn);
                free(user);
                return NULL;
            }

            mysql_free_result(result);
        }

        mysql_close(conn);
        return user;
    } else {
        free(user);
        return NULL;
    }
}

int registerUser(struct User* user) {
   MYSQL* conn = connDatabase();

    if (conn) {
        char query[500];
        sprintf(query, "SELECT id FROM users WHERE username = '%s'", getUsername(user));

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return 0;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                fprintf(stderr, "Utilizatorul exista deja.\n");
                mysql_free_result(result);
                mysql_close(conn);
                return 0;
            } else {
                mysql_free_result(result);

                sprintf(query, "INSERT INTO users (username, password) VALUES ('%s', '%s')",
                        getUsername(user), getPassword(user));

                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Eroare la insert: %s\n", mysql_error(conn));
                    mysql_close(conn);
                    return 0;
                }

                mysql_close(conn);
                return 1;
            }
        }

        mysql_free_result(result);
        mysql_close(conn);
        return 0;
    } else {
        return 0;
    }
}

int loginUser(struct User* user) {
    MYSQL* conn = connDatabase();

     if (conn) {
        char query[500];
        sprintf(query, "SELECT authenticated FROM users WHERE username = '%s' AND password = '%s'",
                getUsername(user), getPassword(user));

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return 0;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                setAuthenticated(user, 1);
                mysql_free_result(result);

                sprintf(query, "UPDATE users SET authenticated = %d WHERE username = '%s' AND password = '%s'",
                        getAuthenticated(user), getUsername(user), getPassword(user));

                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Eroare la update: %s\n", mysql_error(conn));
                    mysql_close(conn);
                    return 0;
                }

                mysql_close(conn);
                return 1;
            }
            mysql_free_result(result);
        }

        mysql_close(conn);
        return 0;
    } else {
        return 0;
    }
}

int changePassword(struct User* user, char* newPassword) {
    MYSQL* conn = connDatabase(); 
    
     if (conn) {
        char query[500];
        sprintf(query, "SELECT id FROM users WHERE username = '%s'",
                getUsername(user));

        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return 0; 
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                setPassword(user, newPassword);
                mysql_free_result(result);

                sprintf(query, "UPDATE users SET password = '%s' WHERE username = '%s'",
                        getPassword(user), getUsername(user));

                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Eroare la update: %s\n", mysql_error(conn));
                    mysql_close(conn);
                    return 0; 
                }

                mysql_close(conn);
                return 1; 
            }
            mysql_free_result(result);
        }

        mysql_close(conn);
        return 0;
    } else {
        return 0;
    }
}

int logoutUser(struct User* user){
    MYSQL* conn = connDatabase();

     if (conn) {
        char query[500];
        sprintf(query, "SELECT authenticated FROM users WHERE username = '%s' AND password = '%s'",
                getUsername(user), getPassword(user));


        if (mysql_query(conn, query)) {
            fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
            mysql_close(conn);
            return 0;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                setAuthenticated(user, 0);
                mysql_free_result(result);

                sprintf(query, "UPDATE users SET authenticated = %d WHERE username = '%s' AND password = '%s'",
                        getAuthenticated(user), getUsername(user), getPassword(user));

                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Eroare la update: %s\n", mysql_error(conn));
                    mysql_close(conn);
                    return 0;
                }
                mysql_close(conn);
                return 1;
            }
            mysql_free_result(result);
        }

        mysql_close(conn);
        return 0;
    } else {
        return 0;
    }
}

char* online_users() {
    MYSQL* conn = connDatabase();

    if (!conn) {
        return NULL;
    }

    char query[500];
    sprintf(query, "SELECT username FROM users WHERE authenticated = 1");

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Eroare la SELECT din baza de date: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (!result) {
        printf("Nu există rezultate!\n");
        mysql_close(conn);
        return NULL;
    }

    int num_rows = mysql_num_rows(result);
    if (num_rows <= 0) {
        mysql_free_result(result);
        mysql_close(conn);
        return NULL;
    }

    size_t buffer_size = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        buffer_size += strlen(row[0]) + 2;
    }

    char* online_users = (char*)malloc(buffer_size + 1);
    if (!online_users) {
        mysql_free_result(result);
        mysql_close(conn);
        return NULL;
    }

    online_users[0] = '\0';


    mysql_data_seek(result, 0); // Resetarea cursorului pentru a itera din nou
    while ((row = mysql_fetch_row(result))) {
        strcat(online_users, row[0]);
        strcat(online_users, ", ");
    }

    size_t last_comma = strlen(online_users) - 2;
    online_users[last_comma] = '\0';

    mysql_free_result(result);
    mysql_close(conn);

    return online_users;
}
