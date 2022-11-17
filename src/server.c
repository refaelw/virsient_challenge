#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>

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

#include "server.h"
#include "client.h"
#include "crossplatform.h"
#include "linked_list.h"
#include "file_io.h"

DWORD WINAPI rx_thread(LPVOID lpParam)
{
    // This is where we do the message receiving
    struct thread_elem *elem;
    struct packet1 pkt1;
    char *filename;
    uint8_t *buffer;
    int ret_val;
    int rx_bytes;
    uint32_t send_code;
    elem = (struct thread_elem *)lpParam;
    // RX the header packet
    rx_bytes = recv(elem->client_socket, &pkt1, sizeof(pkt1), 0);
    if (rx_bytes != sizeof(pkt1))
    {
        // Something failed ...
        perror("rx_thread() - Failed to recv full packet size!");
        return -1;
    }
    if (pkt1.magic_number != MAGIC_NUMBER)
    {
        // This message is not what we think it is
        perror("rx_thread() - magic number not recived");
        return -1;
    }

    filename = malloc(pkt1.filename_size);
    buffer = malloc(pkt1.file_size);

    send_code = TX_FILENAME;
    ret_val = send(elem->client_socket, &send_code, sizeof(send_code), 0);
    if (ret_val < 0)
    {
        perror("rx_thread() - send failed!\n");
        // Error out
        free(filename);
        free(buffer);
        return -1;
    }
    // RX the filename
    rx_bytes = recv(elem->client_socket, filename, pkt1.filename_size, 0);
    if (rx_bytes != pkt1.filename_size)
    {
        // Error out
        perror("rx_thread() - recv filename failed!\n");
        free(filename);
        free(buffer);
        return -1;
    }

    // Check the filesystem
    send_code = TX_BUFFER;
    ret_val = send(elem->client_socket, &send_code, sizeof(send_code), 0);
    if (ret_val < 0)
    {
        // Error out
        perror("rx_thread() - send tx buffer failed!\n");

        free(filename);
        free(buffer);
        return -1;
    }
    // RX the buffer
    rx_bytes = recv(elem->client_socket, buffer, pkt1.file_size, 0);
    if (rx_bytes != pkt1.file_size)
    {
        perror("rx_thread() - recv buffer failed!\n");
        free(filename);
        free(buffer);
        return -1;
    }
    // close the connection
    ret_val = shutdown(elem->client_socket, SD_SEND);
    closesocket(elem->client_socket);

    //
    uint32_t checksum = compute_checksum(buffer, pkt1.file_size);
    if (checksum != pkt1.checksum)
    {
        // Something went wrong ...
        free(filename);
        free(buffer);
        return -1;
    }

    // write the file
    ret_val = write_buffer(filename, buffer, pkt1.file_size);
    if (ret_val != 0)
    {
        // Failed to write
        free(filename);
        free(buffer);
        return -1;
    }
    // free everything
    free(filename);
    free(buffer);
    return 0;
}

int server_loop(SOCKET *listen_socket, bool *keep_looping)
{
    //
    struct thread_elem *active_threads;
    // This is a constant so we dont get killed by too much memory
    int max_threads = 25;

    // This way we can kill the sever from another
    // XXX : I know its not atomic, but C doesn't have standard atomic types.
    while (*keep_looping == true)
    {
        if (list_length(&active_threads) < max_threads)
        {
            struct thread_elem *elem = add_thread(&active_threads);

            // This is blocking so will wait until forever.
            // Oh fuck, this blocks, so will never exit.
            // But this function call is non-blocking on send and other socket function calls.
            // ioctlsocket(sock, FIONBIO, 1);

            elem->client_socket = accept(*listen_socket, NULL, NULL);
            if (elem->client_socket == INVALID_SOCKET)
            {
                perror("server_loop() - Failed to accept!");
                // I think we just try again, because we are the server ...
                // XXX
                closesocket(*listen_socket);
                WSACleanup();
                return -1;
            }

            // Start the threads
            elem->thread = CreateThread(
                NULL,
                0,
                rx_thread,
                elem,
                0,
                &elem->dwThreadIdArray);
        }
        // Cleanup any threads that are done.
        check_threads(&active_threads);
    }
    // Need to kill all threads if not done so already.
    while (list_length(&active_threads) > 0)
    {
        check_threads(&active_threads);
    }

    stop_server(listen_socket);
    return 0;
}

void stop_server(SOCKET *socket)
{
    closesocket(*socket);
    WSACleanup();
}

int start_server(uint16_t port, SOCKET *listen_socket)
{
    WSADATA wsa_data;
    int ret_val, err_code;
    struct addrinfo hints;
    struct addrinfo *result;

    int max_retrys = 50;
    struct timeval time_out;
    time_out.tv_sec = 10;
    time_out.tv_usec = 0;

    result = NULL;

    ret_val = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (ret_val != 0)
    {
        perror("start_server() - Failed WSAStartup");
        return -1;
    }

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    // We are going to say use TCP for reliable transfer.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    // Accept any connections.
    hints.ai_flags = AI_PASSIVE;

    char port_str[7];
    sprintf(port_str, "%d", port);

    ret_val = getaddrinfo(NULL, port_str, &hints, &result);
    if (ret_val != 0)
    {
        if (ret_val == EAI_AGAIN)
        {
            // Temporay failure, sleep and try again.
            Sleep(1000);
            ret_val = getaddrinfo(NULL, port_str, &hints, &result);
            if (ret_val != 0)
            {
                // Things are bad.
                perror("start_server() - getaddrinfo failed!");
                WSACleanup();
                return -1;
            }
        }
        else
        {
            // Things went bad.
            perror("start_server() - getaddrinfo failed, with a bad error code!");
            WSACleanup();
            return -1;
        }
    }
    // Create the socket
    *listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (*listen_socket == INVALID_SOCKET)
    {
        // Clean up.
        perror("start_server() - socket failed!");
        err_code = WSAGetLastError();
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    // Configure the socket
    setsockopt(*listen_socket, IPPROTO_TCP, TCP_MAXRT, &max_retrys, sizeof(max_retrys));
    setsockopt(*listen_socket, IPPROTO_TCP, SO_RCVTIMEO, &time_out, sizeof(time_out));

    // bind
    ret_val = bind(*listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (ret_val == SOCKET_ERROR)
    {
        perror("start_server() - bind failed!");
        // Something went wrong ...
        err_code = WSAGetLastError();
        // Cleanup
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    freeaddrinfo(result);

    // Now listen. Use SO MAX CONN to support as many connections as possible.
    ret_val = listen(*listen_socket, SOMAXCONN);
    if (ret_val == SOCKET_ERROR)
    {
        perror("start_server() - listen failed!");
        // Something went wrong ...
        err_code = WSAGetLastError();
        // Cleanup
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }

    // We are ready for accept.
    return 0;
}