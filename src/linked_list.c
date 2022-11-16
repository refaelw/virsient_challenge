
#include <windows.h>
#include <stdlib.h>

#include "linked_list.h"

void free_element(struct thread_elem **active_threads, struct thread_elem **current)
{
    if ((*current == NULL) || (*active_threads == NULL))
    {
        return;
    }

    if ((*current)->prev == NULL)
    {
        // At the start, so the list so deleting entire list.
        if ((*current)->next == NULL)
        {
            // No more elements
            free(*current);
            *active_threads = NULL;
        }
        else
        {
            *active_threads = (*current)->next;
            (*active_threads)->prev = NULL;
            free(*current);
        }
    }
    else if ((*current)->next == NULL)
    {
        // At the end of the list
        (*current)->prev->next = NULL;
        free(*current);
    }
    else
    {
        // Freeing the current element.
        (*current)->prev->next = (*current)->next;
        (*current)->next->prev = (*current)->prev;
        free(current);
    }
    return;
}

int list_length(struct thread_elem **active_threads)
{
    int length = 0;
    struct thread_elem *current;
    for (current = *active_threads; current != NULL; current = current->next)
    {
        length++;
    }
    return length;
}

// Add an element to the list.
struct thread_elem *add_thread(struct thread_elem **active_threads)
{
    struct thread_elem *current;

    if (*active_threads == NULL)
    {
        // We have no active threads, create one.
        *active_threads = malloc(sizeof(struct thread_elem));
        (*active_threads)->next = NULL;
        (*active_threads)->prev = NULL;
        current = *active_threads;
    }
    else
    {
        // We add to the end of the list.
        current = *active_threads;

        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = malloc(sizeof(struct thread_elem));
        // Setup the next element.
        current->next->prev = current;
        current->next->next = NULL;
        current = current->next;
    }

    current->thread = NULL;
    return current;
}

void check_threads(struct thread_elem **active_threads)
{
    struct thread_elem *current;
    for (current = *active_threads; current != NULL; current = current->next)
    {
        if ((WaitForSingleObject(current->thread, 0) == WAIT_OBJECT_0) || (current->thread == NULL))
        {
            free_element(active_threads, &current);
        }
    }
}