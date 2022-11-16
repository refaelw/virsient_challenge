
#include <fstream>
#include <array>
#include <random>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

// Test the possible functions.
#include <catch2/catch_test_macros.hpp>

#include "client.h"
#include "linked_list.h"

DWORD WINAPI thread_sleep(LPVOID lpParam)
{
    uint32_t *sleep_time;
    sleep_time = (uint32_t *)lpParam;
    Sleep(*sleep_time);
    return 0;
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