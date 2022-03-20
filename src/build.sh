gcc -Wall -Werror -I../lib ../lib/neoviki_socket.c ../lib/neoviki_printf.c msg_server.c -o msg_server
gcc -Wall -Werror -I../lib ../lib/neoviki_socket.c ../lib/neoviki_printf.c msg_client.c -o msg_client
