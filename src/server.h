#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdint.h>
#include "crossplatform.h"

#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
  // used by C++ source code
#endif

    EXTERN_DLL_EXPORT int start_server(uint16_t port, SOCKET *listen_socket);
    EXTERN_DLL_EXPORT void stop_server(SOCKET *listen_socket);

#ifdef __cplusplus
}
#endif

#endif