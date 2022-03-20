#ifndef MSG_CLIENT_H
#define MSG_CLIENT_H

#define NEOVIKI_BUFF_SIZE 1024

#define NEOVIKI_REQUEST         "get_reading"
#define NEOVIKI_CLIENT_EXIT     "quit()"

int msg_client_loop(char *key, int buff_size);

/*called after data received by the process*/
int msg_client_incoming_cb(char *in);

/*called before sending data out of the process*/
int msg_client_outgoing_cb(char *out, int out_size);

#endif

