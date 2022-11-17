#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "server.h"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        // The usage is
        printf("%s filename hostname\n", argv[0]);
        return -1;
    }

    uint16_t port = 54821;
    SOCKET socket;
    int ret_val;

    ret_val = start_server(port, &socket);
    if (ret_val != 0)
    {
        printf("Failed to start the server!\n");
    }

    bool run_loop = true;
    server_loop(&socket, &run_loop);

    // We will never get here
    stop_server(&socket);
    return 0;
}