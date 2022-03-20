/* Simplified Sockets API based on linux sockets
 *
 *              Author   : Viki (a) Vignesh Natarajan
 *              Contact  : neoviki.com
 *              LICENCE  : MIT 
 */


/* Entire life cycle of client or server has to be handled by sigle thread for the process. This program is not threadsafe
 */

#ifndef NEOVIKI_SOCKET_T
#define NEOVIKI_SOCKET_T
#include <sys/socket.h>
#include <sys/un.h>

#define QUEUE_LENGTH 1
#define CLEAR_SCREEN()  printf ("\e[2J")
#define DEBUG

#define KEY_SIZE 256

typedef struct{
    /*client socket id*/
    int  id;
    char key[KEY_SIZE];
    int connected;
    int restart;
    /*one extra byte will be allocated for terminating character. This byte will not be tracked by buff_size*/
    char *buff;
    int  buff_size;
    /*thirdparty variables*/
    struct sockaddr_un  addr;
    int continuous_read_error;
    int continuous_write_error;
}client_t;

typedef struct{
    /*server socket id*/
    int  id;
    char key[KEY_SIZE];
    int connected;
    int restart;
    /*one extra byte will be allocated for terminating character. This byte will not be tracked by buff_size*/
    char *buff;
    int  buff_size;
 
    /*connection id for the incomming client, this api supports only one client*/
    int  conid;
   
    /*thirdparty variables*/
    struct sockaddr_un  addr;
    socklen_t           addr_len;
    int continuous_read_error;
    int continuous_write_error;
}server_t;

client_t *client_open(char *key, int buff_size);
server_t *server_open(char *key, int buff_size);

int client_connect(client_t *cli);
int server_connect(server_t *srv);

int client_read(client_t *cli);
int server_read(server_t *srv);

int client_write(client_t *cli, char *str);
int server_write(server_t *srv, char *str);

void client_close(client_t *cli);
void server_close(server_t *srv);

int is_client_connected();
int is_server_connected();

client_t *client_reopen(client_t *cli);
server_t *server_reopen(server_t *srv);

#endif
