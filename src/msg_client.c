#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "neoviki_socket.h"
#include "neoviki_printf.h"
#include "msg_client.h"

#define KEY "/tmp/test.app"

static client_t *cli = NULL;

static int fresh_connect = 1;

static void msg_client_signal_handler(int sig)
{
    if(sig == SIGPIPE){
        neoviki_printf(debug, "signal ( SIGPIPE : %d )\n", sig);
        if(cli){
            cli->connected = 0;
            client_close(cli);
            cli = NULL;
        }
    }else if(sig == SIGINT){
        neoviki_printf(debug, "signal ( SIGINT  : %d )\n", sig);
        exit(0);
    }
}

/*called after data received by the process*/
int msg_client_incoming_cb(char *in)
{
    if(!in) return -1;
    printf ("client ( in  ) : %s\n", in);
    return 0;
}

/*called before sending data out of the process*/
int msg_client_outgoing_cb(char *out, int out_size)
{
    if(!out) return -1;
    snprintf(out, out_size, "%s", NEOVIKI_REQUEST);
    printf ("client ( out ) : %s\n", out);
    return 0;
}


int msg_client_loop(char *key, int buff_size) 
{
    CLEAR_SCREEN();
    char cli_out[buff_size], cli_in[buff_size];
    int ret;

    if(!key){
        neoviki_printf(error, "key = null\n");
        return -1;
    };

    signal(SIGPIPE, msg_client_signal_handler);
    signal(SIGINT,  msg_client_signal_handler);

    
    while (1){
        
        if(!cli){ 
            cli = client_open(key, buff_size);
            sleep(1);
            continue;
        }

        if(cli->restart){
            printf("Server connection interrupted ( %s )\n", key);
            cli = client_reopen(cli);
            if(!cli) continue;
        }

        if(!cli->connected){
            if(fresh_connect){
                printf("Connecting to server ( %s )\n", key);
            }else{
                printf("Reconnecting to server ( %s )\n", key);
            }
            client_connect(cli);
            sleep(1);
            continue;
        }
        
        fresh_connect = 0;

        ret = msg_client_outgoing_cb(cli_out, sizeof(cli_out));
        if(ret == -1 ) continue;

        ret = client_write(cli, cli_out);
        if(ret == -1 ) continue;


        ret = client_read(cli);
        if(ret == -1 ) continue;

        strncpy(cli_in, cli->buff, sizeof(cli_in)), 
        msg_client_incoming_cb(cli_in);

        if (!strcmp (cli_in, NEOVIKI_CLIENT_EXIT)) {
            break;
        }
        
        sleep(1);
    }
    
    client_close(cli);
    
    return 0;
}

int main (void)
{
    if(msg_client_loop(KEY, NEOVIKI_BUFF_SIZE) < 0) return -1;
    return 0;
}
