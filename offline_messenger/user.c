#include "user.h"

void setIdUser(struct User* user, int id) {
    user->id = id;
}

void setUsername(struct User* user, const char* username) {
    // Verificați dacă există deja un username și eliberați memoria alocată anterior, dacă este cazul
    if (user->username != NULL) {
        free(user->username);
    }

    // Alocare memorie pentru noul username
    user->username = strdup(username);
}

void setPassword(struct User* user, const char* password) {
    // Verificați dacă există deja un password și eliberați memoria alocată anterior, dacă este cazul
    if (user->password != NULL) {
        free(user->password);
    }

    // Alocare memorie pentru noul password
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                fprintf(stderr, "User not found.\n");
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                fprintf(stderr, "User not found.\n");
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                fprintf(stderr, "User nu exista.\n");
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                fprintf(stderr, "User nu exista.\n");
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
            mysql_close(conn);
            return 0;
        }

        MYSQL_RES *result = mysql_store_result(conn);
        if (result) {
            if (mysql_num_rows(result) > 0) {
                fprintf(stderr, "User already exists.\n");
                mysql_free_result(result);
                mysql_close(conn);
                return 0;
            } else {
                mysql_free_result(result);

                sprintf(query, "INSERT INTO users (username, password) VALUES ('%s', '%s')",
                        getUsername(user), getPassword(user));

                if (mysql_query(conn, query)) {
                    fprintf(stderr, "Error saving user to database: %s\n", mysql_error(conn));
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                    fprintf(stderr, "Error updating authentication status: %s\n", mysql_error(conn));
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                    fprintf(stderr, "Error updating authentication status: %s\n", mysql_error(conn));
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
            fprintf(stderr, "Error querying user in database: %s\n", mysql_error(conn));
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
                    fprintf(stderr, "Error updating authentication status: %s\n", mysql_error(conn));
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
