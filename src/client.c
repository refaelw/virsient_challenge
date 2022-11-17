
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
#include "crossplatform.h"

int transmit_buffer(SOCKET *connected_socket, struct packet1 *pkt1, uint8_t *buffer, const char *filename)
{
    // Because if client and server the same, want to test in the same part
    // Send the packet information
    uint32_t rx_codes;
    int ret_val;

    ret_val = send(*connected_socket, pkt1, sizeof(struct packet1), 0);
    if (ret_val == SOCKET_ERROR)
    {
        // Error out
        perror("transmit_buffer() - Failed to send pkt\n");
        return -1;
    }

    // Get response
    ret_val = recv(*connected_socket, &rx_codes, sizeof(rx_codes), 0);
    if (ret_val != sizeof(rx_codes))
    {
        // Error out
        perror("transmit_buffer() - Failed recv the tx filename code\n");
        return -1;
    }

    if (rx_codes == TX_FILENAME)
    {
        // Send filename (because variable length)
        ret_val = send(*connected_socket, filename, pkt1->filename_size, 0);
        if (ret_val == SOCKET_ERROR)
        {
            // Error out
            perror("transmit_buffer() - failed to send filename\n");
            return -1;
        }

        // Get the okay message back
        ret_val = recv(*connected_socket, &rx_codes, sizeof(rx_codes), 0);
        if (ret_val != sizeof(rx_codes))
        {
            // Error out
            perror("transmit_buffer() - failed to recv tx buffer code\n");
            return -1;
        }

        // Check the rx code to make sure we should transmit the buffer
        if (rx_codes == TX_BUFFER)
        {
            // Send the entire buffer.
            ret_val = send(*connected_socket, buffer, pkt1->file_size, 0);
            if (ret_val == SOCKET_ERROR)
            {
                // Error out
                perror("transmit_buffer() - failed to send buffer\n");
                return -1;
            }
        }
        else
        {
            perror("transmit_buffer() - did not recieve the correct code, expecting TX buffer!");
            return -1;
        }
    }
    else
    {
        perror("transmit_buffer() - did not recieve the correct code, expected TX FILENAME");
        return -1;
    }

    // close down the socket
    // XXX : do we want to that here.
    stop_client(connected_socket);
    return 0;
}

// Lets start the client
int transmit_file(SOCKET *connected_socket, const char *filename)
{
    // Send the first part of the message.
    uint8_t *buffer;
    int buffer_size;
    int ret_val;

    buffer_size = buffer_file(filename, &buffer);
    if (buffer_size < 1)
    {
        // Error out
    }

    struct packet1 pkt1;

    // Getting ready to transmit
    pkt1.file_size = buffer_size;
    pkt1.checksum = compute_checksum(buffer, pkt1.file_size);
    pkt1.filename_size = strlen(filename) + 1;
    pkt1.magic_number = MAGIC_NUMBER;

    return transmit_buffer(connected_socket, &pkt1, buffer, filename);
}

void stop_client(SOCKET *connected_socket)
{
    closesocket(*connected_socket);
    WSACleanup();
}

int start_client(const char *hostname, uint16_t port, SOCKET *connected_socket)
{
    int ret_val;
    struct addrinfo hints, *result, *ptr;
    WSADATA wsa_data;
    // 65500
    char port_str[7];
    sprintf(port_str, "%d", port);

    *connected_socket = INVALID_SOCKET;
    result = NULL;
    ptr = NULL;

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

    ret_val = getaddrinfo(hostname, port_str, &hints, &result);
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
