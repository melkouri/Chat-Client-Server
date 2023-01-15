# Chat Client/Server

L'objectif du projet est de réaliser une application de chat client/server en C permettant des échanges de messages entre 2 clients, entre plusieurs clients, ou à tous les utilisateurs connectés au réseau (localhost). Cette application devra aussi pouvoir générer l'historique des messages d'un client à sa reconnexion même quand le serveur a été déconnecté à un moment donné.  

# Composition du binôme
Le projet a été réalisé par Malak El Kouri et Imadath YAYA

## PLAN
* [Execution du programme](#launch-the-program)
* [Savoir-acquis](#savoir-acquis)
* [Technologies](#technologies)
* [Description](#description)
* [Informations](#informations)

## Launch the program

Le programme contient 4 sous-dossiers : *Client* , *Serveur* , *Historique* et *Screens* :

* Seveur :  *server2.c* , *server2.h* , *client2.h*
* Client:   *client2.c* , *client2.h*
* Historique: initialement vide. Au lancement du serveur se dossier se remplit d'un fichier *clients.txt* et des fichiers d'historique des messages de chaque client.
* Screens : il contient des captures d'écran de cas d'utilisation.

Pour compiler le programme, suivre les instructions suivantes: 

-> Se mettre dans le répertoire *Client* (**cd Client**) puis lancer la commande
```  
gcc  client2.c -o client
```
-> Se mettre dans le répertoire *Serveur*(**cd Serveur**) puis lancer la commande 
```
gcc  server2.c -o server
```
Chaque fenêtre du terminal représentera un individu (client ou serveur). Pour établir une communication, il faudra au moins 2 terminals ouverts ( parce que 1 server + 1 client).

Dans le premier terminal( qui représentera le server), il faudra juste lancer la commande suivante pour lancer le serveur.
(N.B : Le numéro de port a été prédefini dans le code sous la constante *PORT* et prend la valeur 1977)
```
./server
```
Dans un second terminal ( qui représentera un client), il faut rentrer l'adresse IP de la machine locale sur laquelle est lancée le serveur. Comme pour le serveur, le numéro de port a déjà été prédéfini dans le code.
Le client devra rentrer également son pseudo. Ce pseudo sera le nom qui sera utilisé par les autres clients connectés sur le serveur pour communiquer avec lui. 

Ainsi, pour connecter le client au serveur, il faut lancer la commande suivante: 
```
./client <IP adress localhost> <pseudo>
```

Pour un second client, il faudra ouvrir un 3ème terminal et lancer la commande précédente correspondant au client :
```
./client <IP adress localhost> <pseudo>
```
et ainsi de suite si on souhaite ajouter d'autres clients.

A la connexion d'un client, on affiche sur son terminal, les différentes personnes connectées actuellement.

Pour se déconnecter, il suffit de lancer dans le terminal la commande *Ctrl+C*
Lorsqu'un client se déconnecte, les autres clients connectés au serveur sont avertis de la déconnexion de celui-ci. 

Dans le cas où le client destinataire existe mais est juste déconnecté, celui-ci verra tous les messages lors de sa reconnexion.

## Savoir-acquis
- Manipuler les primitives du réseau et les sockets POSIX en C : socket(), bind(), listen(), connect(), accept(), send(), recv()  
- Implémenter des protocoles de communication sous TCP/IP

## Technologies
Le projet a été developpé entièrement en **C**.

## Description


### Fonctionnalité 1 : Discussion bilatérale 

Une des fonctionnalité du programme est la discussion privée entre deux clients. Ainsi, un client X donné peut envoyer des messages en privé à un client Y donné. Pour ce faire, le client X doit suivre le format suivant :

```
nomDestinataire : message
```
*Exemple* : `ima : Coucou ima !`

Dans le cas où le client destinataire est inexistant c'est à dire qu'il n'y a aucun client associé à ce pseudo, le message suivant est envoyé à l'expéditeur afin de le lui notifier

```
Ce client n'existe pas
```
Dans le cas où le client destinataire existe mais est juste déconnecté, le message suivant est envoyé à l'expéditeur
```
nomDestinataire n'est pas connecte(e) actuellement.
```
L'expéditeur quand à lui, à sa reconnexion, voit également  dans son historique les messages qui lui ont été envoyés quand il n'était pas connecté. 


### Fonctionnalité 2: Envoi d'un message à tous les clients

Le programme permet à un client d'envoyer un message à tous les autres clients . Pour ce faire, il faudra mettre comme destinataire *toall* suivi du message. Le format à respecter est le suivant 

```
toall:message
```
*Exemple* : `toall : Hi everyone !`

### Fonctionnalité 3 : Création de groupe et discussion de groupe 

L'application chat client/serveur offre comme fonctionnalité la création d'un groupe de discussion. Lors de la création du groupe, le nom à attribuer au groupe doit être mentionné précédé de **@** ainsi que le nom des membres. Le format à adopter pour créer un groupe est le suivant: 

```
nomClient1 nomClient2 ... @nomGrp: message
```
*Exemple* : `ima malak @girls : Hi girls !`

Les membres du groupe reçoivent le message ci-dessous leur mentionnant qu'ils font partir d'un groupe qui vient d'être créé. 

Une fois le groupe créé, les membres peuvent discuter entre eux juste en mentionnant comme destinataire le nom du groupe toujrs précédé de **@**. Le format a respecté est le suivant : 

```
@nom du groupe : message
```
*Exemple* : `@girls: Hello mimi !How are you?`

### Fonctionnalité 4 : Historique du client 

L'une des fonctionnalité majeure du programme est qu'il stocke l'historique de discussion de chaque client et qu'il le restaure à la reconnexion de celui-ci. 

L'historique de discussion de chaque client est donc stocké dans un *fichier* portant son nom suivi de *.txt* dans le dossier *Historique*.

L'historique du client n'est donc pas éphémère mais plutôt persistant. Donc si le serveur s'arrête  puis redémarre plus tard, lorsque le client se reconnecte avec le même pseudo, toute son historique de discussion lui est restaurée sur le terminal.

Pour ce faire, on enregistre dans un fichier .txt portant le nom de *clients.txt*, le nom de chaque client lors de leur première connexion au serveur. Ainsi, même si le serveur s'arrete et redémarre, ce fichier permet de retrouver si un client a déjà été connecté à une session et donc dans ce cas, de lui afficher son historique enregistré dans le fichier portant son nom.  

Ainsi, un client, dès sa connexion, a un fichier correspondant à son historique de discussion et portant son nom soit: 

```
nomClient.txt
```
*Exemple* : `ima.txt`

**Format d'un fichier correspondant à l'historique du client**

Dans le fichier de l'historique des clients, les messages sont enregistrés sous le format suivant : 

* Si le client est l'expéditeur du message, le message est enregistré sous le format suivant : 
```
To destinataire : message   || Date: dateEnvoi
```
N.B: Dans ce format, **destinataire** peut prendre l'une de ces valeurs: *toall*, *nomUtilisateur*, *@nomGrp*

*Exemple* : `To toall : Hi everyone ! || Date: Sat Oct 29 22:17:02 2022`

* Si le client est le destinataire du message et :

-> le message est destiné à un groupe (nomGrp) ou à tout le monde(toall), le format d'enregistrement est le suivant:

```
From [nomExpediteur to destinataire] : message   || Date: dateEnvoi
```
NB: Dans ce cas, **destinataire** ne peut être que *nomGrp* ou *toall*

*Exemple* : `From [mimi to @girls] : Hi girls !   || Date: Sat Oct 29 22:30:02 2022`

-> le message est destiné à un client en particulier, le message est enregistré sous le format suivant:
```
 From [nomExpediteur] : message  || Date: dateEnvoi
```
*Exemple* : `From [mimi] : Coucou ima ! || Date: Sat Oct 29 22:35:02 2022`

**N.B:** 
* Une fois que le serveur s'arrête et redémarre, les groupes ne sont pas stockés et donc l'envoi d'un message dans un groupe préexistant est impossible. On a simplement pas eu le temps de faire persister les groupes dans un fichier.txt et de lui réattribuer ses membres.

* On a également pas eu le temps d'empêcher un client n'appartenant pas au groupe, d'envoyer un message dans ce groupe. Ce sont les différentes améliorations auxquelles on a pensé si il y avait plus de temps.

## Informations
Ce projet est un projet académique pour le cours de "Programmation Réseau" durant notre 4ème année d'école d'ingénieur à l'INSA LYON.

YAYA Imadath - EL KOURI Malak
