#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "neoviki_socket.h"
#include "neoviki_printf.h"
#include "msg_client.h"
#include "msg_server.h"
#define KEY "/tmp/test.app"


static server_t *srv = NULL;
static int fresh_connect = 1;

static void msg_server_signal_handler(int sig)
{
    if(sig == SIGPIPE){
        neoviki_printf(debug, "signal ( SIGPIPE : %d )\n", sig);
        if(srv){
            srv->connected = 0;
            server_close(srv);
            srv = NULL;
        }
    }else if(sig == SIGINT){
        neoviki_printf(debug, "signal ( SIGINT  : %d )\n", sig);
        exit(0);
    }
}

int msg_server_outgoing_cb(char *out, int out_size)
{
    static int i;
    i=(i+1)%100;
    if(!out) return -1;
    snprintf(out, out_size, "data ( %d )", i);
    printf("server ( out ) : %s\n", out);
    return 0;
}

int msg_server_incoming_cb(char *in)
{
    if(!in) return -1;
    printf("server ( in  ) : %s\n", in);
    return 0;
}

int msg_server_loop(char *key, int buff_size)
{
    CLEAR_SCREEN();
    int ret;
    char srv_in[buff_size], srv_out[buff_size];

    if(!key){
        neoviki_printf(error, "key = null\n");
        return -1;
    }

    signal(SIGPIPE, msg_server_signal_handler);
    signal(SIGINT,  msg_server_signal_handler);

    while(1) {
        if(!srv){
            srv = server_open(key, buff_size);
            sleep(1);
            continue;
        }

        if(srv->restart){
            printf("Client connection interrupted ( %s )\n", key);
            srv = server_reopen(srv);
            if(!srv) continue;
        }

        if(!srv->connected){
            if(fresh_connect){
                printf("Connecting to client ( %s )\n", key);
            }else{
                printf("Reconnecting to client ( %s )\n", key);
            }
 
            server_connect(srv);
            sleep(1);
            continue;
        } 
        
        fresh_connect = 0;
        
        ret = server_read(srv);
       
        if(ret == -1 ){
            continue;
        }

        strncpy(srv_in, srv->buff, sizeof(srv_in)), 
        neoviki_printf(debug, "server ( in  ) : %s\n", srv_in);
        msg_server_incoming_cb(srv_in);
        
        if(strcmp(srv_in, NEOVIKI_REQUEST) == 0){
            ret = msg_server_outgoing_cb(srv_out, sizeof(srv_out));

            if(ret == -1) continue;
            
            neoviki_printf(debug, "server ( out ) : %s\n", srv_out);

            server_write(srv, srv_out);
        }

        sleep(1);
    }
    
    server_close(srv);

    return 0;
}

int main()
{
    neoviki_debug(0);
    if(msg_server_loop(KEY, NEOVIKI_BUFF_SIZE) < 0) return -1;
    return 0;
}
