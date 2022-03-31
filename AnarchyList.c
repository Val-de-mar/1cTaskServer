
#include "AnarchyList.h"
#include <stdlib.h>
#include <stdio.h>

AnarchyList *emplaceAnarchyAfter(AnarchyList *prev, void *value) {
    AnarchyList *new_node = malloc(sizeof(AnarchyList));
    AnarchyList *next = prev->next;
    if (next != NULL) {
        next->prev = new_node;
    }
    prev->next = new_node;
    new_node->next = next;
    new_node->prev = prev;
    new_node->value = value;
    return new_node;
}

void destroyHeapAnarchyList(AnarchyList *node) {
    AnarchyList *prev = node->prev;
    AnarchyList *next = node->next;
    if (next != NULL) {
        next->prev = prev;
    }
    if (prev != NULL) {
        prev->next = next;
    }
    free(node);
}
