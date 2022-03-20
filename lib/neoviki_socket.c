/* Simplified Sockets API based on linux sockets
 *
 *              Author   : Viki (a) Vignesh Natarajan
 *              Contact  : neoviki.com
 *              LICENCE  : MIT 
 */


#include "neoviki_socket.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#include "neoviki_printf.h"

static char *create_string_buffer(int buff_size)
{
    char *buff = NULL;
    
    if(buff_size < 0){
        neoviki_printf(debug, "buff_size: should be greater than 0\n");
        return NULL;
    }

    /*allocate one extra size for terminating character*/
    buff = (char *) calloc(buff_size+1, sizeof(char));
    return buff;
}

/*Fixed Parameters*/
static server_t *server_init(char *key)
{
  int ret;

  if(!key){neoviki_printf(error, "key = null\n"); return NULL;}

  server_t *srv = (server_t*) calloc(1, sizeof(server_t));
  
  if(!srv) { 
      neoviki_printf(debug, "OOM\n"); 
      return NULL;
  }

  srv->addr.sun_family = AF_LOCAL;
  strcpy(srv->addr.sun_path, key);
  strncpy(srv->key, key, KEY_SIZE);
  srv->addr_len = sizeof (struct sockaddr_in);
  
  srv->id = socket (AF_LOCAL, SOCK_STREAM, 0);

  if(srv->id < 0) {
    neoviki_printf (error, "server ->  socket()\n");
    free(srv);
    return NULL;
  }

  /*delete the key file ( if exist ): this is important*/
  unlink(key);

  /*assign address*/
  ret = bind(srv->id, (struct sockaddr *) &srv->addr, sizeof (srv->addr));
    
  if(ret  != 0) {
    neoviki_printf(error, "server -> bind()\n");
    close(srv->id);
    free(srv);
    return NULL;
  }

  /*prepare the sockets to accept*/
  ret = listen(srv->id, QUEUE_LENGTH);
 
  if(ret  != 0) {
    neoviki_printf(error, "server -> listen()\n");
    close(srv->id);
    free(srv);
    return NULL;
  }
 
  return srv;
}


static int validate_server(server_t *srv)
{
    if(!srv)  { 
        neoviki_printf(error, "srv = null (%s:%d)\n", __func__, __LINE__);  
        return -1;
    }

    if(srv->id < 0)  { 
        neoviki_printf(error, "srv->id < 0 (%s:%d)\n", __func__, __LINE__);
        return -1;
    }

    if(!srv->buff){
        neoviki_printf(error, "srv->buff = null (%s:%d)\n", __func__, __LINE__); 
        return -1;
    } 

    if(srv->buff_size < 0){
        neoviki_printf(error, "srv->buff_size < 0 (%s:%d)\n", __func__, __LINE__); 
        return -1;
    } 
    
    return 0;
}


static int validate_client(client_t *cli)
{
    if(!cli)  { 
        neoviki_printf(error, "cli = null (%s:%d)\n", __func__, __LINE__);  
        return -1;
    }

    if(cli->id < 0)  { 
        neoviki_printf(error, "cli->id < 0 (%s:%d)\n", __func__, __LINE__);
        return -1;
    }

    if(!cli->buff){
        neoviki_printf(error, "cli->buff = null (%s:%d)\n", __func__, __LINE__); 
        return -1;
    } 

    if(cli->buff_size < 0){
        neoviki_printf(error, "cli->buff_size < 0 (%s:%d)\n", __func__, __LINE__); 
        return -1;
    } 
    
    return 0;
}

client_t *client_open(char *key, int buff_size)
{
    client_t *cli = (client_t *) calloc(1, sizeof(client_t));

    if( !cli ){
        neoviki_printf(error, "OOM\n");
        return NULL;
    }

    if(buff_size <= 0) return NULL;

    cli->buff       = create_string_buffer(buff_size);
    cli->buff_size  = buff_size;

    if(!cli->buff){
        neoviki_printf(error, "OOM\n");
        free(cli);
        return NULL;
    }

    cli->connected              = 0;
    cli->continuous_write_error = 0;
    cli->continuous_read_error  = 0;

    /*TCP*/
    cli->id = socket (AF_LOCAL, SOCK_STREAM, 0);

    if( cli->id < 0){
        neoviki_printf(error, "client -> socket()\n");
        free(cli->buff);
        free(cli);
        return NULL;
    }else{
        neoviki_printf(success, "client -> socket()\n");
    }

    cli->addr.sun_family = AF_LOCAL;
    strcpy(cli->addr.sun_path, key);
    strncpy(cli->key, key, KEY_SIZE);

    return cli;
}

server_t *server_open(char *key, int buff_size)
{
    if(!key){neoviki_printf(error, "key = null\n"); return NULL;}
   
    server_t *srv = server_init(key);
    
    if(!srv) return NULL;
    if(buff_size <= 0) return NULL;

    srv->buff = create_string_buffer(buff_size);
    srv->buff_size = buff_size;

    if(!srv->buff){
        neoviki_printf(error, "OOM\n");
        free(srv);
        return NULL;
    }
    
    srv->connected = 0;
    srv->continuous_write_error = 0;
    srv->continuous_read_error = 0;


    return srv;
}

client_t *client_reopen(client_t *cli)
{
    if(!cli) return NULL;

    int buff_size = cli->buff_size;
    char key[KEY_SIZE];
    strncpy(key, cli->key, KEY_SIZE);
    client_close(cli);
    return client_open(key, buff_size);
}

server_t *server_reopen(server_t *srv)
{
    if(!srv)return NULL;

    int buff_size = srv->buff_size;
    char key[KEY_SIZE];
    strncpy(key, srv->key, KEY_SIZE);

    server_close(srv);
    return server_open(key, buff_size);
}


/*blocking function*/
int client_connect(client_t *cli)
{
    int ret=0;
    neoviki_printf(debug, "client_connect()\n");
  
    if(validate_client(cli)<0) return -1;
   
    neoviki_printf(debug, "client -> trying to connect to server\n");
    ret = connect( cli->id, (struct sockaddr *) &cli->addr, sizeof(cli->addr));

    if(ret == 0){
        neoviki_printf(success, "client -> connect()\n");
        cli->connected = 1;
    }else{
        neoviki_printf(debug, "client -> connect()\n");
        ret = -1;
    }

    return ret;
}

/*blocking function*/
int server_connect(server_t *srv)
{
    neoviki_printf(debug, "server_connect()\n");
    if(validate_server(srv) < 0) return -1;

    neoviki_printf(debug, "waiting for client ...\n");
    /*blocking call*/
    srv->conid = accept(srv->id, (struct sockaddr *) &srv->addr, &srv->addr_len);
    
    if (srv->conid < 0){
        neoviki_printf(debug, "server -> connect()\n");
        srv->conid = -1;
        srv->connected = 0;
    }else{
        neoviki_printf(success, "server -> connect()\n");
        srv->connected = 1;
    }

    return srv->conid;
}

int client_read(client_t *cli)
{
    int read_size = 0;

    if(validate_client(cli)<0) return -1;

    cli->buff[0] = '\0';
    
    read_size = recv(cli->id, cli->buff, cli->buff_size, 0);

    if( read_size <= 0) {
        //perror("client -> recv error : ");
        cli->continuous_read_error += 1; 
        /*make client connected to false if continuous read failure*/
        if(cli->continuous_read_error >= 3) {
            cli->connected = 0;
            cli->restart = 1;
        }
        return -1;
    }
    
    cli->continuous_read_error = 0;

    /* Don't worry about overflow of cli->buff, we preallocated one extra byte for terminating character.
     * The extra byte is not tracked by cli->buff_size.
     */
    //read_size is always <= cli->buff_size
    cli->buff[read_size+1] = '\0';

    return 0;
}

int server_read(server_t *srv)
{
    int read_size = 0;

    if(validate_server(srv) < 0) return -1;

    srv->buff[0] = '\0';

    read_size = recv (srv->conid, srv->buff, srv->buff_size, 0);

    if( read_size <= 0){
        //perror("server -> recv error ");
        srv->continuous_read_error += 1;
        /*make server connected to false if continuous read failure*/
        if(srv->continuous_read_error >= 3) {
            srv->connected = 0;
            srv->restart = 1;
        }
        return -1;
    }
    
    srv->continuous_read_error = 0;

    /* Don't worry about overflow of srv->buff, we preallocated one extra byte for terminating character.
     * The extra byte is not tracked by srv->buff_size.
     */
    //read_size is always <= srv->buff_size
    srv->buff[read_size+1] = '\0';

    return 0;
}


int client_write(client_t *cli, char *str)
{
    int nbytes_written = 0, strlen_excluding_terminating_character, data_len;

    if(validate_client(cli)<0) return -1;

    if(!str){ 
        neoviki_printf(error, "str = null (%s:%d)\n", __func__, __LINE__);
        return -1;
    }
    
    strlen_excluding_terminating_character = strlen(str); 
    data_len =  strlen_excluding_terminating_character + 1;

    if(strlen_excluding_terminating_character > cli->buff_size){
        neoviki_printf(error, "strlen(str) greater than cli->buff_size\n");
        return -1;
    }

    /* Don't worry about overflow of cli->buff, we preallocated one extra byte for terminating character.
     * The extra byte is not tracked by cli->buff_size.
     */
    strcpy(cli->buff, str);    /*copy characters including terminating character */

    nbytes_written = send(cli->id, cli->buff, data_len, 0);

    if(nbytes_written > 0){
        cli->continuous_write_error = 0;
        return 0;
    }else{
        cli->continuous_write_error += 1;
        /*make client connected to false if continuous write failure*/
        if(cli->continuous_write_error >= 3) {
            cli->connected = 0;
            cli->restart = 1;
        }
        return -1;
    }
}

int server_write(server_t *srv, char *str)
{
    int nbytes_written, strlen_excluding_terminating_character, data_len;
    
    if(validate_server(srv) < 0) return -1;
    
    if(!str){ 
        neoviki_printf(error, "str = null (%s:%d)\n", __func__, __LINE__);
        return -1;
    }

    strlen_excluding_terminating_character = strlen(str); 
    data_len = strlen_excluding_terminating_character + 1; 
   
    if(strlen_excluding_terminating_character > srv->buff_size){
        neoviki_printf(error, "[ strlen(str) ] greater than srv->buff_size\n");
        return -1;
    }

    /* Don't worry about overflow of srv->buff, we preallocated one extra byte for terminating character.
     * The extra byte is not tracked by srv->buff_size.
     */
    strcpy(srv->buff, str);     /*copy characters including terminating character */
    
    nbytes_written = send(srv->conid, srv->buff, data_len, 0);
  
    if(nbytes_written > 0){
        srv->continuous_write_error = 0; 
        return 0;
    }else{
        srv->continuous_write_error += 1;
        /*make server connected to false if continuous write failure*/
        if(srv->continuous_write_error >=3) {
            srv->connected = 0;
            srv->restart = 1;
        }
        return -1;
    }
}

void client_close(client_t *cli)
{
    if(validate_client(cli)<0) return;

    cli->connected = 0;

    close(cli->id);

    cli->id         = 0;
    cli->buff_size  = 0;

    free(cli->buff);
    free(cli);
}

void server_close(server_t *srv)
{
    if(validate_server(srv) < 0) return;

    srv->connected = 0;

    close(srv->conid);
    close(srv->id);
    
    srv->conid      = 0;
    srv->buff_size  = 0;
    srv->id         = 0;
    
    free(srv->buff);
    free(srv);
}





