#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

int write_buffer(const char *filename, uint8_t *buf, size_t buf_size)
{
    // We write the file to disk.
    FILE *fp = NULL;
    int ret_val;
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("write_buffer() - fopen() failed");
        return -1;
    }
    size_t bytes_written = fwrite(buf, 1, buf_size, fp);
    ret_val = fclose(fp);

    if (bytes_written != buf_size)
    {
        perror("write_buffer() - fwrite(), not all the buffer was written to disk!");
        return -1;
    }
    if (ret_val != 0)
    {
        perror("write_buffer() - fclose(), failed to close file stream");
        return -1;
    }
    return 0;
}

int buffer_file(const char *filename, uint8_t **buf)
{
    // We are passing by copy, not by reference.
    // classic dumb C programming mistake ...
    // Verify the file exists on the file system.
    FILE *fp = NULL; //
    long int file_size;
    size_t read_bytes;
    int ret_val;
    errno_t err_code;

    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        // We have a problem, return an error code
        perror("get_filesize() - fopen failed on file!");
        return -1;
    }
    // Now get the size of the file
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    if (file_size < 1)
    {
        // If the file is zero why are we transmitting it?
        perror("get_filesize() - Input file has 0 size!");
        return -1;
    }
    // Go back to start
    rewind(fp);
    // Now create the buffer and read in
    *buf = (uint8_t *)malloc(file_size);
    if (*buf == NULL)
    {
        // We run out of memory, oh fuck.
        perror("get_filesize() - malloc() failed!");
        return -1;
    }

    read_bytes = fread(*buf, 1, file_size, fp);
    if (read_bytes != file_size)
    {
        // Something interesting has happened.
        perror("get_filesize() - Number or read bytes doesn't match file size!");
        // I'm going to assume this is an error, something like a file contention
        free((void *)*buf);
        return -1;
    }

    ret_val = fclose(fp);
    if (ret_val != 0)
    {
        // Things have gotten werid ...
        perror("get_filesize() - fclose failed!");
        free((void *)*buf);
        return -1;
    }

    // Things went well.
    return read_bytes;
}

uint32_t compute_checksum(uint8_t *buffer, size_t buf_size)
{
    // XXX : Currently not implemented.
    return 0;
}