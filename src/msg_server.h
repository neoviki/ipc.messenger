#ifndef NEOVIKI_MSG_SERVER_H
#define NEOVIKI_MSG_SERVER_H
int msg_server_loop(char *key, int buff_size);

/*called after data received by the process*/
int msg_server_incoming_cb(char *in);

/*called before sending data out of the process*/
int msg_server_outgoing_cb(char *out, int out_size);
#endif
