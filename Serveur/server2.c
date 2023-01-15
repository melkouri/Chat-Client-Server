#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"
#include "time.h"

static void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   int actual = 0;                           //number of clients
   int max = sock;
   Client clients[MAX_CLIENTS];              //array of all the clients
   Client destinataires[MAX_CLIENTS];        //array of the receivers
   Group * groups=malloc(sizeof(Group)*20);  //array of the chatting groups created
   int nbGrp = 0;                            //number of groups created
   fd_set rdfs;

   //maj_list_groupe (groups, &nbGrp);

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1)
         {
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);
         
         //Check if the client was disconnected
         int trouve=0;
         int i;
         for(i=0;i<actual;i++)
         {
            if(strcmp(buffer,clients[i].name)==0)
            {
               clients[i].connecte=1;
               clients[i].sock=csock;
               send_history(clients[i]);
               trouve=1;
               afficher_connectes(clients, actual , csock);
               break;
            }
         }
         if(trouve==0)
         {
            Client c = { csock };
            strncpy(c.name, buffer, BUF_SIZE - 1);
            c.groupCount=0;
            c.connecte=1;
            clients[actual] = c;
            if(check_clients_existants(clients[actual].name)==0)
            {
               save_clients(clients[actual]);
            }
            else{
               send_history(clients[actual]);
            }
            actual++;
            afficher_connectes(clients, actual , csock);
         }
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               char * bufferDest;
               char * bufferMsg;
               char * bufferGrp=NULL;
               char * nameDest=malloc(sizeof(char)*15);
               Client * liste;

               int m=0;
               bufferDest = strtok(buffer, ":");
               bufferMsg = strtok(NULL, ":");
               
               char * name=malloc(sizeof(char)*BUF_SIZE);
               char ** names=malloc(sizeof(name)*10);
               int k;
               for(k=0;k<10;k++){
                  names[k]=malloc(sizeof(char)*BUF_SIZE);
               }
  
               name=strtok(bufferDest," ");
               // loop through the string to extract all other tokens
               while( name != NULL ) {
                  strcpy(names[m],name);
                  m++;
                  name = strtok(NULL, " ");
               }
               
               if(m==1 && strcmp(bufferDest,"toall")!=0 && names[0][0]=='@')
               {
                  bufferGrp = names[0];
               }
               else if(m>1)
               {
                  bufferGrp = names[m-1];
               }

               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  clients[i].connecte=0;
                  clients[i].sock=-1;
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_clients(clients, clients[i], actual, buffer, 1,NULL);
               }
               else
               {
                  if(strcmp(bufferDest,"toall")==0)
                  {
                     //The receivers: all the clients
                     send_message_to_clients(clients, client, actual, bufferMsg, 0, bufferDest);
                     create_or_complete_history(client, clients,bufferMsg,actual,bufferDest);
                  }
                  else
                  {
                     if(bufferGrp!=NULL)
                     {
                        //The receivers: the clients forming the group
                        int index=-1;
                        groups = find_or_create_group(bufferGrp, m, names, groups, &nbGrp, client, clients, actual, &index);
                        strcpy(nameDest,bufferGrp);
                        if(index==-1)
                        {
                           //In case the group or the clients don't exist
                           liste=NULL;
                        }
                        else
                        {
                           liste=groups[index].client;         //Array of clients forming the group
                           m=groups[index].nbPersonne;   
                        }
                     }
                     else
                     {
                        //The receivers: only one client
                        liste=find_destinataire(names, m, actual,clients, destinataires);
                        strncpy(nameDest,names[0],BUF_SIZE - 1);
                     }

                     send_message_to_clients(liste, client, m, bufferMsg, 0,bufferGrp);
                     create_or_complete_history(client, liste,bufferMsg,m, nameDest);
                  }
                  free(nameDest); 
                  int r ;
                  for (r=0; r<m; r++)
                  {
                     free(names[r]);
                  }
                  free(name);
               }
               break;
            }
         }
      }
   }
   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients,int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   (*actual)--;
}

static void send_message_to_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server, char * bufferGrp)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   if (clients == NULL)
   {
      write_client(sender.sock,"Ce client n'existe pas");
   }
   else
   {
      if(from_server == 2)
      {
         strncpy(message,"Le groupe ",BUF_SIZE - 1);
         strncat(message, bufferGrp, sizeof message - strlen(message) - 1);
         strncat(message, " [ ", sizeof message - strlen(message) - 1);
         for(i = 0; i < actual; i++)
         {
            strncat(message,clients[i].name,sizeof message - strlen(message) - 1);
            strncat(message," ",sizeof message - strlen(message) - 1);
         }
         strncat(message," ] a été créé \r\n",sizeof message - strlen(message) - 1);
      }
      for(i = 0; i < actual; i++)
      {
         if(from_server == 2)
         {
            write_client(clients[i].sock, message);
         }
         else if(from_server != 2 && sender.sock != clients[i].sock){
            if(clients[i].connecte==1)
            {
               /* we don't send message to the sender */
               int nbDeco = 0;
               if(from_server == 0)
               {
                  strncpy(message, "From [", BUF_SIZE - 1);
                  strncat(message, sender.name, sizeof message - strlen(message) - 1);
                  if(bufferGrp!=NULL)
                  {
                     strncat(message, " to ", sizeof message - strlen(message) - 1);
                     strncat(message, bufferGrp, sizeof message - strlen(message) - 1);
                  }
                  strncat(message, "] : ", sizeof message - strlen(message) - 1);
               }
               strncat(message, buffer, sizeof message - strlen(message) - 1);
               write_client(clients[i].sock, message);
               message[0] = '\0';
            }
            else
            {
               strncpy(message, clients[i].name, BUF_SIZE - 1);
               strncat(message," n'est pas connecte(e) actuellement.", sizeof message - strlen(message) - 1);
               write_client(sender.sock,message);
            }
         }   
      }     
   }
}

static Client * find_destinataire(char ** name, int nbName, int actual,Client *clients,  Client *destinaires)
{
   int n=0;
   int i,j;
   for(j=0; j<nbName;j++)
   {
      for(i = 0; i < actual; i++)
      {
         if(strcmp(name[j],clients[i].name)==0)
         {
            destinaires[n]=clients[i];
            strcpy(destinaires[n].name,clients[i].name);
            destinaires[n].sock=clients[i].sock;
            n++;
            break;
         }
      }
   }
   if(n==0)
   {
      return NULL;
   }
   return destinaires;
}

static Group * find_or_create_group(char * grpname, int m, char ** names, Group * groups, int * nbGrp, Client sender, Client * clients,int actual, int * index)
{
   int trouve=0;
   if(m==1)
   {
      int i;
      for(i=0;i<(*nbGrp);i++)
      {
         if(strcmp(grpname,groups[i].name)==0)
         {
            trouve=1;
            *index=i;
            break;
         }
      }
      if(trouve==0)
      {
         *(index)=-1;
      }
   }
   else
   {
      Client * grpMembers=malloc(sizeof(Client)*m);
      grpMembers=find_destinataire(names,m, actual,clients, grpMembers);
      if(grpMembers==NULL)
      {
         *index=-1;
      }
      else
      {
         char * name=malloc(sizeof(char)*10);
         strcpy(name,grpname);
         grpMembers[m-1]=sender;
         Group room={name,grpMembers,m};
         groups[*nbGrp]=room;
         *index=*nbGrp;
         (*nbGrp)++;
         send_message_to_clients(grpMembers,sender,m,NULL,2,grpname);
      }
   }
   return groups;
}

static void create_or_complete_history(Client sender, Client * receiver,char * msg,int actual,char * nameDest)
{
   if(receiver!=NULL)
   {
      if(chdir("../Historique")==0){
         //Save the sender's history
         FILE * fileSender;
         char nameSender[10];

         time_t rawtime;
         struct tm* timeinfo;
         time(&rawtime);
         timeinfo=localtime(&rawtime);

         strncat(nameSender,sender.name,10);
         fileSender = fopen(strncat(nameSender,".txt",6), "a");
         fprintf(fileSender, "**** To %s : %s   || Date: %s \n", nameDest, msg,  asctime(timeinfo));
         fclose(fileSender);
         nameSender[0]='\0';

         //Save the receivers history
         int i=0;
         if(actual==1)                       
         {
            //The receiver is only one client
            FILE * fileReceiver;
            char nameReceiver[50];
            strncat(nameReceiver,receiver[i].name,10);
            fileReceiver = fopen(strncat(nameReceiver,".txt",6), "a");
            fprintf(fileReceiver, "**** From [%s] : %s   || Date: %s \n", sender.name, msg,asctime(timeinfo));
            fclose(fileReceiver);
            nameReceiver[0]='\0';
         }
         else
         {
            //The receivers are a more than one client
            char nameReceiver[50];
            for(i=0;i<actual;i++)
            {
               if(strcmp(sender.name,receiver[i].name)!=0)
               {
                  FILE * fileReceiver;
                  strncat(nameReceiver,receiver[i].name,10);
                  fileReceiver = fopen(strncat(nameReceiver,".txt",6), "a");
                  if(fileReceiver!=NULL)
                  {
                     fprintf(fileReceiver, "**** From [%s to %s] : %s   || Date: %s \n", sender.name, nameDest, msg,asctime(timeinfo));
                     nameReceiver[0]='\0';
                     fclose(fileReceiver);
                  }
               }
            }
         }
      }
   }
}

static void send_history(Client client){
   if(chdir("../Historique")==0){
      FILE * file;
      char * name=malloc(sizeof(char)*10);
      strncat(name,client.name,10);
      file = fopen(strncat(name,".txt",6), "r");
      if(file!=NULL)
      {
         char * buffer=malloc(sizeof(char)*BUF_SIZE);
         char * token;
         while(fgets(buffer, BUF_SIZE,file)!=NULL)
         {
            write_client(client.sock,buffer);
         }
         free(buffer);
         fclose(file);
      }
      free(name);
   }
}

static void afficher_connectes(Client * clients, int  actual , SOCKET sockClient)
{
   int i = 0;
   for (i =0; i<actual; i++){
      if(clients[i].connecte==1 && clients[i].sock!= sockClient)
      {
         char msginit [BUF_SIZE];
         strncpy(msginit,clients[i].name, BUF_SIZE -1);
         strncat(msginit, " est connecte actuellement\r\n", BUF_SIZE -strlen(msginit) - 1);
         write_client(sockClient,msginit);
      }
   }
}

static void save_clients(Client client)
{
   if(chdir("../Historique")==0){
      FILE * file;
      file = fopen("clients.txt", "a");
      fprintf(file, "%s\n",client.name);
      fclose(file);
   }
}

static int check_clients_existants(char * clientName)
{
   if(chdir("../Historique")==0){
      FILE * file;
      file = fopen("clients.txt", "r");
      if(file!=NULL)
      {
         char * buffer=malloc(sizeof(char)*10);
         while(fgets(buffer, BUF_SIZE,file)!=NULL)
         {
            if(strncmp(clientName,buffer,strlen(clientName)-1)==0)
            {
               fclose(file);
               return 1;
            }
         }
         free(buffer);
         fclose(file);
         return 0;
      }
   }
   return 1;
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;
   
   int opt = 1;
   if (setsockopt(sock, SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT, &opt,
				sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}