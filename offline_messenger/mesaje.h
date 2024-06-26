#ifndef MESAJE_H
#define MESAJE_H

#include "database.h"
#include "user.h"
#include <stdio.h>

struct Mesaje {
    int id;
    int expeditorId;
    int destinatarId;
    char* mesaj;
    int seen;
};

void setIdMesaje(struct Mesaje* mesaj, int id);
void setExpeditor(struct Mesaje* mesaj, int expeditorId);
void setDestinatar(struct Mesaje* mesaj, int destinatarId);
void setMesaj(struct Mesaje* mesaj, const char* mesajText);
void setSeen(struct Mesaje* mesaj, int seen);

int getIdMesaje(struct Mesaje* mesaj);
int getExpeditorId(struct Mesaje* mesaj);
int getDestinatarId(struct Mesaje* mesaj);
char* getMesaj(struct Mesaje* mesaj);
int isSeen(struct Mesaje* mesaj);

int sentMessage(struct User* expeditor, struct User* destinatar, struct Mesaje* mesaj);
char** see_all_new_messages(struct User* destinatar);
char** get_a_conversation(struct User* user1, struct User* user2);


#endif
