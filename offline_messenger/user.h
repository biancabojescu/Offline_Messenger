#ifndef USER_H
#define USER_H

#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct User {
    int id;
    char* username;
    char* password;
    int authenticated;
};

void setIdUser(struct User* user, int id);
void setUsername(struct User* user, const char* username);
void setPassword(struct User* user, const char* password);
void setAuthenticated(struct User* user, int authenticated);
int getIdUser(struct User* user);
char* getUsername(struct User* user);
char* getPassword(struct User* user);
int getAuthenticated(struct User* user);
int isAuthenticated(struct User* user);
struct User* getUserByUsername(const char* username);
struct User* getUserById(int id);

int registerUser(struct User* user);
int loginUser(struct User* user);
int changePassword(struct User* user, char* newPassword);
int logoutUser(struct User* user);

#endif 
