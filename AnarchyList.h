//
// Created by val-de-mar on 31.03.2022.
//

#ifndef TASK_ANARCHYLIST_H
#define TASK_ANARCHYLIST_H


typedef struct AnarchyListStruct {
    void *value;
    struct AnarchyListStruct *prev;
    struct AnarchyListStruct *next;
} AnarchyList;

AnarchyList *emplaceAnarchyAfter(AnarchyList *prev, void *value);

void destroyHeapAnarchyList(AnarchyList *node);


#endif //TASK_ANARCHYLIST_H
