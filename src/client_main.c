
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "client.h"
#include "file_io.h"

int main(int argc, char *argv[])
{
    // We have a file to transfer.
    if (argc != 3)
    {
        // The usage is
        printf("%s filename hostname\n", argv[0]);
        return -1;
    }

    // So find the server and connect.
    int ret_val;
    const char *filename = argv[1];
    const char *hostname = argv[2];

    // Send the file.
    SOCKET socket;
    uint16_t port = 54821;

    ret_val = start_client(hostname, port, &socket);
    if (ret_val != 0)
    {
        printf("Failed to start the client and connect to the server!\n");
        return -1;
    }
    ret_val = transmit_file(&socket, filename);
    if (ret_val != 0)
    {
        printf("Failed to buffer and transmit the file!\n");
        return -1;
    }

    return 0;
}