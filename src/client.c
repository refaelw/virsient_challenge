
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

//#include <unistd.h>
// The boost libraries are pretty good for this stuff.
#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "client.h"

struct packet1
{
    uint32_t magic_number;
    uint32_t file_size;
    uint32_t filename_size;
    uint32_t checksum;
};

// Lets start the client
int transmit_file()
{
    // Send the first part of the message.

    // Get response

    // Send filename (because variable length)

    // Get the okay message back

    // Send the entire buffer.
}

int start_client(const char *hostname, uint16_t port, SOCKET *connected_socket)
{
    int ret_val;
    struct addrinfo hints, *result, *ptr;
    WSADATA wsa_data;
    *connected_socket = INVALID_SOCKET;

    int32_t max_retries = 25;
    int32_t retry_count = 0;

    ret_val = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret_val != 0)
    {
        perror("start_client() - WSAStartup() failed!");
        return -1;
    }
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    ret_val = getaddrinfo(hostname, port, &hints, &result);
    if (ret_val != 0)
    {
        perror("start_client() - getaddrinfo failed!");
        WSACleanup();
        return -1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
    {
        *connected_socket = socket(ptr->ai_family, ptr->ai_socktype,
                                   ptr->ai_protocol);
        if (*connected_socket == INVALID_SOCKET)
        {
            perror("start_client() - socket failed");
            WSACleanup();
            return -1;
        }

        ret_val = connect(*connected_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (ret_val == SOCKET_ERROR)
        {
            closesocket(*connected_socket);
            *connected_socket = INVALID_SOCKET;
            retry_count++;
            if (retry_count > max_retries)
            {
                break;
            }
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (*connected_socket == INVALID_SOCKET)
    {
        perror("start_client() - Unable to connect to server!");
        WSACleanup();
        return -1;
    }
    // We have a valid socket.
    return 0;
}

/*
int rx_header()
{
    // We wait on the connection until

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // We spin up a thread now.
    // Receive until the peer shuts down the connection
    do
    {

        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);

            // Echo the buffer back to the sender
            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
            if (iSendResult == SOCKET_ERROR)
            {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else
        {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (iResult > 0);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }
}

int stop_server()
{
    // No longer need server socket
    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}
* /

    int start_server(uint16_t port)
{
    // FTP uses port 21,
    // uint16_t port = 1778;
    WSADATA wsa_data;
    int ret_val, err_code;
    SOCKET listen_socket;
    struct addrinfo *result, hints;

    int max_retrys = 50;
    struct timeval time_out;
    time_out.tv_sec = 10;
    time_out.tv_usec = 0;

    ret_val = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret_val != 0)
    {
        perror("start_server() - Failed WSAStartup");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));
    // We are going to try IPv6 to start with.
    hints.ai_addr = AF_INET6;
    // We are going to say use TCP for reliable transfer.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // Accept any connections.
    hints.ai_flags = AI_PASSIVE;

    ret_val = getaddrinfo(NULL, port, &hints, &result);
    if (ret_val != 0)
    {
        if (ret_val == EAI_AGAIN)
        {
            // Temporay failure, sleep and try again.
            Sleep(1000);
            ret_val = getaddrinfo(NULL, port, &hints, &result);
            if (ret_val != 0)
            {
                // Things are bad.
                WSACleanup();
                return -1;
            }
        }
        else
        {
            // Things went bad.
            WSACleanup();
            return -1;
        }
    }
    // Create the socket
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET)
    {
        // Clean up.
        err_code = WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    // Configure the socket
    setsockopt(listen_socket, IPPROTO_TCP, TCP_MAXRT, &max_retrys, sizeof(max_retrys));
    setsockopt(listen_socket, IPPROTO_TCP, SO_RCVTIMEO, &time_out, sizeof(time_out));
    // bind
    ret_val = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (ret_val == SOCKET_ERROR)
    {
        // Something went wrong ...
        err_code = WSAGetLastError();
        // Cleanup
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(result);

    // Now listen. Use SO MAX CONN to support as many connections as possible.
    ret_val = listen(listen_socket, SOMAXCONN);
    if (ret_val == SOCKET_ERROR)
    {
        // Something went wrong ...
        err_code = WSAGetLastError();
        // Cleanup
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    // The accept() is blocking so should start in another thread.
    return;

    // This is where we have to be careful, as the client socket is what we communicate on.
    // client_socket

    // Setup the socket options due to the channel conditions
    // We are going to increase the timeout because the channel is unreliable
}

/*
int
sha_compute(uint8_t buffer, size_t buffer_size)
{
    // Get the buffers checksum.
}
int npackets(size_t buffer_size, size_t pkt_size)
{
    // Calculate the number of packets to transmit.
}

int start_transfer()
{
    // So we send the file header information.

    // If the
}

int transmit_buffer()
{
    // So run through the buffer and send the packets.
}
*/
