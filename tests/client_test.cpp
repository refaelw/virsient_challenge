
#include <fstream>
#include <array>
#include <random>
#include <iostream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#endif

// Test the possible functions.
#include <catch2/catch_test_macros.hpp>

#include "server.h"
#include "client.h"
#include "crossplatform.h"
#include "linked_list.h"
#include "file_io.h"

DWORD WINAPI thread_sleep(LPVOID lpParam)
{
    uint32_t *sleep_time;
    sleep_time = (uint32_t *)lpParam;
    Sleep(*sleep_time);
    return 0;
}

void client_thread(std::string hostname, uint16_t port, std::string filename, size_t file_size)
{
    // Designed to be a thread so can boot multiple
    // TX file transfers at once.
    SOCKET connected_socket;
    int ret_val;
    std::vector<uint8_t> buf;
    buf.resize(file_size);

    struct packet1 pkt1;
    pkt1.magic_number = MAGIC_NUMBER;
    pkt1.file_size = file_size;
    pkt1.filename_size = strlen(filename.c_str()) + 1;
    pkt1.checksum = compute_checksum(buf.data(), buf.size());

    ret_val = start_client(hostname.c_str(), port, &connected_socket);
    REQUIRE(ret_val == 0);

    ret_val = transmit_buffer(&connected_socket, &pkt1, buf.data(), filename.c_str());
    CHECK(ret_val == 0);
    std::cout << "Have transmitted file " << filename << std::endl;
    return;
}

void start_server_loop(bool &run_loop, uint16_t port)
{
    //
    SOCKET listen_socket;
    int ret_val;
    ret_val = start_server(port, &listen_socket);
    REQUIRE(ret_val == 0);
    ret_val = server_loop(&listen_socket, &run_loop);
    return;
}

void start_server_thread(uint16_t port)
{
    // We start the server
    int ret_val;
    std::array<char, 10> rx_bytes;

    SOCKET listen_socket, client_socket;

    ret_val = start_server(port, &listen_socket);
    REQUIRE(ret_val == 0);
    // This is a blocking function
    client_socket = accept(listen_socket, NULL, NULL);
    REQUIRE(client_socket != INVALID_SOCKET);
    // We only need to listen once.
    closesocket(listen_socket);
    ret_val = recv(client_socket, rx_bytes.data(), rx_bytes.size(), 0);
    if (ret_val < 0)
    {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }
    CHECK(ret_val == rx_bytes.size());
    for (size_t n = 0; n < rx_bytes.size(); n++)
    {
        CHECK(rx_bytes[n] == n);
    }

    // This need to happen on the client socket.
    ret_val = shutdown(client_socket, SD_SEND);
    closesocket(client_socket);

    WSACleanup();
    return;
}

TEST_CASE("TransmitFiles")
{
    // Finally getting the unit tests together.
    uint16_t port = 47181;
    std::string hostname = "LocalHost";

    bool run_server = true;
    std::thread server_thread(start_server_loop, std::ref(run_server), port);

    // We spin up the client threads
    std::array<std::thread, 10> client_threads;
    for (size_t n = 0; n < client_threads.size(); n++)
    {
        std::string filename = "file_number_" + std::to_string(n) + ".bin";
        size_t filesize = (n + 1) * 200;

        client_threads.at(n) = std::thread(client_thread, hostname, port, filename, filesize);
    }

    // Now wait for all the threads to joni back up.
    for (size_t n = 0; n < client_threads.size(); n++)
    {
        client_threads.at(n).join();
    }

    run_server = false;
    server_thread.join();

    return;
}

TEST_CASE("StartServer")
{
    int ret_val;
    uint16_t port = 27015;
    SOCKET listen_socket;

    ret_val = start_server(port, &listen_socket);
    CHECK(ret_val == 0);
    stop_server(&listen_socket);
    return;
}

TEST_CASE("ClientServer")
{
    using namespace std::chrono_literals;
    int ret_val;
    uint16_t port = 27015;
    SOCKET connected_socket;

    // We test that we can start and stop the client and
    // servers.
    std::string hostname = "LocalHost";
    std::thread server_thread = std::thread(start_server_thread, port);

    // Make sure the sever is actually up and running
    std::this_thread::sleep_for(1ms);

    // Start the client.
    ret_val = start_client(hostname.c_str(), port, &connected_socket);
    REQUIRE(ret_val == 0);
    // Send some data.
    std::array<char, 10> bytes{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    ret_val = send(connected_socket, bytes.data(), bytes.size(), 0);
    CHECK(ret_val == 10);

    stop_client(&connected_socket);

    server_thread.join();
    // Start the client.
}

TEST_CASE("StartThreads")
{
    std::array<uint32_t, 8> sleep_times{5000, 2000, 3000, 1000, 6000, 300, 20, 10000};
    // HANDLE hThreadArray[10];
    struct thread_elem *active_threads;
    // Add a value to the list.
    // Because I've never used windows threads before
    for (size_t n = 0; n < sleep_times.size(); n++)
    {
        struct thread_elem *elem = add_thread(&active_threads);

        elem->thread = CreateThread(
            NULL,                    // default security attributes
            0,                       // use default stack size
            thread_sleep,            // thread function name
            &sleep_times[n],         // argument to thread function
            0,                       // use default creation flags
            &elem->dwThreadIdArray); // returns the thread identifier

        if (elem->thread == NULL)
        {
            // We free this element and error out
            free_element(&active_threads, &elem);
            std::cout << "Failed to create thread!" << std::endl;
            break;
        }
    }
    CHECK(list_length(&active_threads) == sleep_times.size());

    int check_count = 0;
    while (list_length(&active_threads) > 0)
    {
        check_threads(&active_threads);
        check_count++;
        Sleep(10);
    }
    CHECK(check_count > 10);
    return;
}

TEST_CASE("LinkedListCleanUp")
{
    struct thread_elem *active_threads = NULL;
    struct thread_elem *new_elem = add_thread(&active_threads);
    CHECK(list_length(&active_threads) == 1);
    // This will cause issues.
    check_threads(&active_threads);
    CHECK(list_length(&active_threads) == 0);
    return;
}

TEST_CASE("LinkedList")
{
    //
    struct thread_elem *active_threads = NULL;
    int nthreads;
    nthreads = list_length(&active_threads);
    REQUIRE(nthreads == 0);

    // Add a thread element
    struct thread_elem *new_elem = add_thread(&active_threads);
    REQUIRE(list_length(&active_threads) == 1);

    struct thread_elem *new_elem2 = add_thread(&active_threads);
    REQUIRE(list_length(&active_threads) == 2);

    //
    check_threads(&active_threads);

    // We delete the first thread ...
    free_element(&active_threads, &active_threads);
    REQUIRE(list_length(&active_threads) == 1);
    CHECK(active_threads == new_elem2);
    CHECK(active_threads->next == NULL);
    CHECK(active_threads->prev == NULL);

    free_element(&active_threads, &new_elem2);
    REQUIRE(list_length(&active_threads) == 0);

    // Make sure doesn't segfault.
    free_element(&active_threads, &active_threads);
}

TEST_CASE("FileBuffer")
{
    std::array<char, 101> binary_data;
    std::string testFilename = "test.bin";

    // For generating random numbers for testing files
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1, 255);

    std::ofstream testFile(testFilename, std::ofstream::binary);
    for (auto &b : binary_data)
    {
        b = dist6(rng);
    }

    testFile.write(binary_data.data(), binary_data.size() * sizeof(uint8_t));
    testFile.close();

    int filesize;
    uint8_t *buffer = nullptr;

    filesize = buffer_file("fake.bin", &buffer);
    CHECK(filesize == -1);
    CHECK(buffer == nullptr);

    filesize = buffer_file(testFilename.c_str(), &buffer);

    REQUIRE(filesize == 101);
    REQUIRE(buffer != nullptr);

    // Make sure the contents are buffered correclty.
    for (size_t n = 0; n < binary_data.size(); n++)
    {
        CHECK((uint8_t)binary_data.at(n) == *(buffer + n));
    }
    // Clean up properly
    free(buffer);
}