#ifndef _FILE_IO_H_
#define _FILE_IO_H_

#include <stdint.h>

#include "crossplatform.h"

#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
  // used by C++ source code
#endif

  struct file_info
  {
    char *name;        /**< The filename of the file */
    uint32_t size;     /**< Normally I would use size_t, but because that changes between hardware architectures (ARM32 to ARM64) using more defined variable types */
    uint8_t *buffer;   /**< The buffer where the file is stored */
    uint32_t checksum; /**< Checksum calculated on the buffer */
  };

  /**
   * Load an input file into memory.
   * @param filename The input file to load into RAM
   * @param buffer The pointer the file is loaded into.
   * @returns The size of buffer, or -1 on failure.
   */
  EXTERN_DLL_EXPORT int buffer_file(const char *filename, uint8_t **buf);
  EXTERN_DLL_EXPORT int write_buffer(const char *filename, uint8_t *buf, size_t buf_size);
  EXTERN_DLL_EXPORT uint32_t compute_checksum(uint8_t *buffer, size_t buf_size);
#ifdef __cplusplus
}
#endif

#endif