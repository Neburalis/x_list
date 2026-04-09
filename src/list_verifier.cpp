#include <stdlib.h>

#include "list.h"
#include "base.h"

namespace x_list {

bool verifier(list_t *list) {
    if (list == NULL)           return false;

    list_element_t *elements = list->elements; // "кешируем" массив, чтобы каждый раз не ходить по указателю на list
    if (elements == NULL) {
        list->errno = LIST_NULL_POINTER;
        return false;
    }

    if (list->errno != LIST_NO_PROBLEM)
        return false;

    if (list->capacity == 0 || list->size == 0 || list->size > list->capacity) {
        list->errno = LIST_SIZE_MISMATCH;
        return false;
    }

    unsigned char *visited = (unsigned char *) calloc(list->capacity, sizeof(unsigned char));
    if (visited == NULL) {
        list->errno = LIST_CANNOT_ALLOC_MEMORY;
        return false;
    }

    size_t count = 0;
    size_t prev_idx = 0;
    size_t curr = elements[0].next; // head
    size_t last = 0;

    while (curr != 0) {
        if (curr >= list->capacity) {
            list->errno = LIST_INVALID_INDEX;
            free(visited);
            return false;
        }
        if (visited[curr]) {
            // revisited a non-zero node -> cycle not passing through 0
            list->errno = LIST_LOOP_IN_NEXT;
            free(visited);
            return false;
        }
        if (elements[curr].prev != prev_idx) {
            list->errno = LIST_LOOP_IN_PREV;
            free(visited);
            return false;
        }
        visited[curr] = 1;
        last = curr;
        prev_idx = curr;
        curr = elements[curr].next;
        ++count;
        if (count > list->capacity) {
            list->errno = LIST_LOOP_IN_NEXT;
            free(visited);
            return false;
        }
    }

    size_t free_curr = list->free_idx;
    size_t free_count = 0;
    // printf("verify free idx in list\n");
    while (free_curr != 0) {
        // eval_print(free_curr);
        if (free_curr >= list->capacity) {
            // printf("if 1\n");
            list->errno = LIST_INVALID_INDEX;
            free(visited);
            return false;
        }
        if (visited[free_curr] == 2) {
            // уже встречали этот элемент в free-list -> цикл
            // printf("if 2\n");
            list->errno = LIST_LOOP_IN_FREE;
            // printf("MEOW\n");
            free(visited);
            // printf("MEOW\n");
            return false;
        }
        if (visited[free_curr] == 1) {
            // printf("if 3\n");
            // элемент, который помечен как активный, находится в free-list -> повреждение
            list->errno = LIST_FREE_LIST_CORRUPT;
            free(visited);
            return false;
        }
        visited[free_curr] = 2;
        free_curr = elements[free_curr].next;
        ++free_count;
        if (free_count > list->capacity) {
            list->errno = LIST_FREE_LIST_CORRUPT;
            free(visited);
            return false;
        }
    }
    // printf("\n\n\n");

    if (elements[0].prev != last) {
        list->errno = LIST_LOOP_IN_PREV;
        free(visited);
        return false;
    }

    if (list->size != count + 1) {
        list->errno = LIST_SIZE_MISMATCH;
        free(visited);
        return false;
    }

    free(visited);
    return true;
}

}