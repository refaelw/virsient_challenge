#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdint.h>
#include "crossplatform.h"

#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
  // used by C++ source code
#endif

#define MAGIC_NUMBER 0x184F189A

// The codes to send back to the client.
#define TX_FILENAME 0x01
#define TX_BUFFER 0x02
  struct packet1
  {
    uint32_t magic_number;
    uint32_t file_size;
    uint32_t filename_size;
    uint32_t checksum;
  };

  EXTERN_DLL_EXPORT int transmit_file(SOCKET *connected_socket, const char *filename);
  EXTERN_DLL_EXPORT int start_client(const char *hostname, uint16_t port, SOCKET *connected_socket);
  EXTERN_DLL_EXPORT void stop_client(SOCKET *connected_socket);

#ifdef __cplusplus
}
#endif

#endif