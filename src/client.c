
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "client.h"

struct file_info
{
    char *name;
    uint32_t size; // Normally I would use size_t, but because that changes between hardware architectures (ARM32 to ARM64) using more defined variable types
    uint8_t *buffer;
    uint32_t checksum;
    // filename
    // size
    // buffer of file
    // posistion of transmission.
    // sha key
};
/*
int
sha_compute(uint8_t buffer, size_t buffer_size)
{
    // Get the buffers checksum.
}

int get_filesize(const char *filename)
{
    // Verify the file exists on the file system.
    FILE *fp;
    long int file_size;
    uint8_t *buffer;
    size_t read_bytes;

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        // We have a problem, return an error code
    }
    // Now get the size of the file
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    if (file_size < 1)
    {
        // If the file is zero why are we transmitting it?
    }
    // Go back to start
    rewind(fp);
    // Now create the buffer and read in
    buffer = (uint8_t *)malloc(file_size);
    if (buffer == NULL)
    {
        // We run out of memory, oh fuck.
    }
    read_bytes = fread(buffer, 1, file_size, fp);
    if (read_bytes != file_size)
    {
        // Something interesting has happened.
    }
    //
    fclose(fp);

    // Things went well.
    return read_bytes;
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
int main(int argc, char *argv[])
{
    printf("Hello World\n");
    // We have a file to transfer.

    // So find the server and connect.

    // Send the file.
}