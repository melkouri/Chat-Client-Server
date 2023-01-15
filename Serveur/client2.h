#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   int groupCount;
   int connecte;
}Client;


typedef struct{
   char * name;
   Client * client;
   int nbPersonne;
} Group;

#endif /* guard */