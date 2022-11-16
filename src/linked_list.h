#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#include <stdint.h>
#include "client.h"

#ifdef __cplusplus
extern "C"
{ // only need to export C interface if
  // used by C++ source code
#endif

    struct thread_elem
    {
        // We do need a linked list when we delete something.
        struct thread_elem *prev;
        struct thread_elem *next;
        void *thread; // The thread value.
        SOCKET client_socket;
        DWORD dwThreadIdArray;
    };

    EXTERN_DLL_EXPORT void free_element(struct thread_elem **active_threads, struct thread_elem **current);
    EXTERN_DLL_EXPORT int list_length(struct thread_elem **active_threads);
    EXTERN_DLL_EXPORT struct thread_elem *add_thread(struct thread_elem **active_threads);
    EXTERN_DLL_EXPORT void check_threads(struct thread_elem **active_threads);

#ifdef __cplusplus
}
#endif

#endif
