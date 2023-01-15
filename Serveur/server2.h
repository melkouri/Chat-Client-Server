
#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#elif defined (linux)

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#else

#error not defined for this platform

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100

#define BUF_SIZE    1024

#include "client2.h"

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);

//Sends the msg to the clients 
static void send_message_to_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server, char * bufferGrp);

//Finds the receivers by their names from the array of clients, return NULL if the client doesn't exist
static Client * find_destinataire(char ** name, int nbName, int actual,Client *clients,  Client *destinaires);

//Creates a chat group or finds it in the array of groups, modifies index to the index of the group in the array.
static Group * find_or_create_group(char * grpname, int m, char ** names, Group * groups, int * nbGrp, Client sender, Client * clients,int actual, int * index);

//Shows the chat history of the connected client
static void send_history(Client client);

//Shows the list of the clients already connected
static void afficher_connectes(Client * clients, int  actual , SOCKET sockClient);

//Creates a file clientName.file for each clients and stores the messages exchanged
static void create_or_complete_history(Client sender, Client * receiver,char * msg,int actual,char * nameGrp);

//Saves the name of each new client in a file clients.txt
static void save_clients(Client client);

//Checks if the client exists in the file clients.txt : the file contains the names of all the clients 
//who were connected at least once since the server started
static int check_clients_existants(char * clientName);

static void remove_client(Client *clients,int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);

#endif /* guard */